/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// A logger.

#include "bstone_logger.h"

#include <cstddef>
#include <cstdint>

#include <algorithm>
#include <array>
#include <condition_variable>
#include <exception>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "bstone_exception.h"
#include "bstone_file_stream.h"
#include "bstone_memory_resource.h"
#include "bstone_single_pool_resource.h"

namespace bstone {

Logger::Logger() noexcept = default;

Logger::~Logger() = default;

void Logger::log(LoggerMessageType message_type, StringView message_sv) noexcept
{
	do_log(message_type, message_sv);
}

void Logger::log_information() noexcept
{
	do_log(LoggerMessageType::information, StringView{});
}

void Logger::log_information(StringView message_sv) noexcept
{
	do_log(LoggerMessageType::information, message_sv);
}

void Logger::log_warning(StringView message_sv) noexcept
{
	do_log(LoggerMessageType::warning, message_sv);
}

void Logger::log_error(StringView message_sv) noexcept
{
	do_log(LoggerMessageType::error, message_sv);
}

void Logger::log_exception(std::exception_ptr exception_ptr) noexcept
{
	try
	{
		auto message_buffer = std::string{};
		message_buffer.reserve(2048);
		log_exception_internal(exception_ptr, message_buffer);
		log_error(StringView{message_buffer.c_str(), static_cast<std::intptr_t>(message_buffer.size())});
	}
	catch (...)
	{
		auto error_message = "Generic failure.";

		try
		{
			std::rethrow_exception(std::current_exception());
		}
		catch (const std::exception& exception)
		{
			error_message = exception.what();
		}
		catch (...) {}

		log_error(__func__);
		log_error(error_message);
	}
}

void Logger::log_current_exception() noexcept
{
	log_exception(std::current_exception());
}

void Logger::flush() noexcept
{
	do_flush();
}

void Logger::log_exception_internal(std::exception_ptr exception_ptr, std::string& message_buffer)
{
	try
	{
		std::rethrow_exception(exception_ptr);
	}
	catch (const std::exception& exception)
	{
		const auto nested_exception = dynamic_cast<const std::nested_exception*>(&exception);

		if (nested_exception != nullptr && nested_exception->nested_ptr() != nullptr)
		{
			try
			{
				nested_exception->rethrow_nested();
			}
			catch (...)
			{
				log_exception_internal(std::current_exception(), message_buffer);
			}
		}

		if (!message_buffer.empty())
		{
			message_buffer += '\n';
		}

		message_buffer += exception.what();
	}
	catch (...)
	{
		if (!message_buffer.empty())
		{
			message_buffer += '\n';
		}

		message_buffer += "Generic failure.";
	}
}

namespace {

// ==========================================================================

class LoggerImplQueue
{
public:
	LoggerImplQueue();

	void set_block_size(std::intptr_t block_size);

	void clear() noexcept;
	void enqueue(LoggerMessageType message_type, const char* message, std::intptr_t message_length);
	bool dequeue(LoggerMessageType& message_type, const char*& message, std::intptr_t& message_length);

private:
	static constexpr auto header_size = static_cast<std::intptr_t>(sizeof(std::intptr_t));

private:
	using Queue = std::vector<char>;

private:
	std::intptr_t block_size_{};
	std::intptr_t size_{};
	std::intptr_t index_{};
	Queue queue_{};

private:
	static std::intptr_t align_value(std::intptr_t value, std::intptr_t alignment) noexcept;
};

// --------------------------------------------------------------------------

LoggerImplQueue::LoggerImplQueue() = default;

void LoggerImplQueue::set_block_size(std::intptr_t block_size)
{
	if (block_size <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Block size out of range.");
	}

	block_size_ = align_value(block_size, header_size);

	if (static_cast<std::intptr_t>(queue_.size()) < block_size)
	{
		queue_.resize(block_size_);
	}
}

void LoggerImplQueue::clear() noexcept
{
	size_ = 0;
	index_ = 0;
}

void LoggerImplQueue::enqueue(LoggerMessageType message_type, const char* message, std::intptr_t message_length)
{
	const auto aligned_message_length = align_value(message_length, header_size);
	const auto new_size = size_ + header_size + aligned_message_length;
	const auto capacity = static_cast<std::intptr_t>(queue_.size());

	if (capacity < new_size)
	{
		const auto new_capacity = align_value(new_size, block_size_);
		queue_.resize(new_capacity);
	}

	const auto message_type_and_length = (static_cast<std::intptr_t>(message_type) << 24) | message_length;
	reinterpret_cast<std::intptr_t&>(queue_[size_]) = message_type_and_length;
	std::copy_n(message, message_length, queue_.begin() + size_ + header_size);
	size_ += header_size + aligned_message_length;
}

bool LoggerImplQueue::dequeue(LoggerMessageType& message_type, const char*& message, std::intptr_t& message_length)
{
	if (index_ == size_)
	{
		message_type = LoggerMessageType::information;
		message = nullptr;
		message_length = 0;
		return false;
	}

	const auto message_type_and_length = reinterpret_cast<const std::intptr_t&>(queue_[index_]);
	message_type = static_cast<LoggerMessageType>(message_type_and_length >> 24);
	message_length = message_type_and_length & 0xFFFFFF;
	message = &queue_[index_ + header_size];
	const auto aligned_message_length = align_value(message_length, header_size);
	index_ += header_size + aligned_message_length;
	return true;
}

std::intptr_t LoggerImplQueue::align_value(std::intptr_t value, std::intptr_t alignment) noexcept
{
	return ((value + alignment - 1) / alignment) * alignment;
}

// ==========================================================================

class LoggerImpl final : public Logger
{
public:
	LoggerImpl(const LoggerOpenParam& param);
	~LoggerImpl() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr) noexcept;

private:
	static const StringView empty_sv;
	static const StringView error_prefix_sv;
	static const StringView warning_prefix_sv;

private:
	using Mutex = std::mutex;
	using LockGuard = std::lock_guard<Mutex>;
	using UniqueLock = std::unique_lock<Mutex>;
	using Queues = std::array<LoggerImplQueue, 2>;
	using LogFunc = void (LoggerImpl::*)(LoggerMessageType, StringView);
	using FlushFunc = void (LoggerImpl::*)();

private:
	bool is_synchronous_{};
	bool is_cancellation_requested_{};
	bool is_flush_requested_{};
	bool is_flush_acknowledged_{};
	bool has_messages_{};
	bool is_file_open_{};
	bool is_file_open_at_least_once_{};
	bool is_thread_failed_{};
	LoggerFlushPolicy flush_policy_{};
	std::intptr_t consumer_queue_index_{};
	std::intptr_t producer_queue_index_{};
	LogFunc log_func_{};
	FlushFunc flush_func_{};
	std::exception_ptr exception_ptr_{};
	std::thread thread_{};
	FileStream file_stream_{};
	std::string file_path_{};
	std::string line_{};
	std::condition_variable cv_{};
	std::condition_variable cv_flush_{};
	Mutex cv_mutex_{};
	Mutex cv_flush_mutex_{};
	Queues queues_{};

private:
	void do_log(LoggerMessageType message_type, StringView message_sv) noexcept override;
	void do_flush() noexcept override;

private:
	static MemoryResource& get_memory_resource();

	void log_logger_current_exception() noexcept;
	void try_open_file() noexcept;
	void write_internal(LoggerMessageType message_type, StringView message_sv);
	void write_sync(LoggerMessageType message_type, StringView message_sv);
	void write_async(LoggerMessageType message_type, StringView message_sv);
	void flush_internal();
	void acknowledge_flush();
	void flush_sync();
	void flush_async();
	void thread_main_proxy() noexcept;
	void thread_main();
};

// --------------------------------------------------------------------------

const StringView LoggerImpl::empty_sv = StringView{};
const StringView LoggerImpl::error_prefix_sv = StringView{"[ERROR] "};
const StringView LoggerImpl::warning_prefix_sv = StringView{"[WARNING] "};

LoggerImpl::LoggerImpl(const LoggerOpenParam& param)
{
	is_synchronous_ = param.is_synchronous;
	flush_policy_ = param.flush_policy;
	file_path_ = param.file_path;

	line_.reserve(2048);

	if (is_synchronous_)
	{
		log_func_ = &LoggerImpl::write_sync;
		flush_func_ = &LoggerImpl::flush_sync;
	}
	else
	{
		log_func_ = &LoggerImpl::write_async;
		flush_func_ = &LoggerImpl::flush_async;

		consumer_queue_index_ = 0;
		producer_queue_index_ = 1;

		for (auto& queue : queues_)
		{
			queue.set_block_size(65536);
		}

		thread_ = std::thread{&LoggerImpl::thread_main_proxy, this};
	}
}

LoggerImpl::~LoggerImpl()
{
	if (!thread_.joinable())
	{
		return;
	}

	{
		LockGuard lock_guard{cv_mutex_};
		is_cancellation_requested_ = true;
		is_flush_requested_ = true;
		has_messages_ = true;
	}

	cv_.notify_one();
	thread_.join();
}

void* LoggerImpl::operator new(std::size_t size)
{
	return get_memory_resource().allocate(size);
}

void LoggerImpl::operator delete(void* ptr) noexcept
{
	get_memory_resource().deallocate(ptr);
}

void LoggerImpl::do_log(LoggerMessageType message_type, StringView message_sv) noexcept
{
	try
	{
		(this->*log_func_)(message_type, message_sv);
	}
	catch (...)
	{
		log_logger_current_exception();
	}
}

void LoggerImpl::do_flush() noexcept
{
	try
	{
		(this->*flush_func_)();
	}
	catch (...)
	{
		log_logger_current_exception();
	}
}

MemoryResource& LoggerImpl::get_memory_resource()
{
	static SinglePoolResource<LoggerImpl> memory_resource{};
	return memory_resource;
}

void LoggerImpl::log_logger_current_exception() noexcept
{
	std::cerr << error_prefix_sv.get_data() << " Logger failed. ";

	try
	{
		std::rethrow_exception(std::current_exception());
	}
	catch (const std::exception& exception)
	{
		std::cerr << exception.what();
	}
	catch (...)
	{
		std::cerr << "Generic failure.";
	}

	std::cerr << std::endl;
}

void LoggerImpl::try_open_file() noexcept
{
	if (is_file_open_)
	{
		return;
	}

	auto flags = FileOpenFlags::create | FileOpenFlags::read_write;

	if (!is_file_open_at_least_once_)
	{
		flags |= FileOpenFlags::truncate;
	}

	if (file_stream_.try_open(file_path_.c_str(), flags))
	{
		is_file_open_ = true;
		is_file_open_at_least_once_ = true;
	}
}

void LoggerImpl::write_internal(LoggerMessageType message_type, StringView message_sv)
{
	const auto is_error = message_type == LoggerMessageType::error;
	const auto is_warning = message_type == LoggerMessageType::warning;
	const auto prefix = is_error ? error_prefix_sv : (is_warning ? warning_prefix_sv : empty_sv);

	line_.clear();
	line_.append(prefix.get_data(), static_cast<std::size_t>(prefix.get_size()));
	line_.append(message_sv.get_data(), static_cast<std::size_t>(message_sv.get_size()));
	line_ += '\n';

	auto& std_stream = is_error ? std::cerr : std::cout;
	std_stream << line_;

	if (flush_policy_ == LoggerFlushPolicy::every_message)
	{
		std_stream.flush();
	}

	if (is_file_open_)
	{
		try
		{
			file_stream_.seek(0, StreamOrigin::end);
			file_stream_.write_exactly(line_.data(), static_cast<std::intptr_t>(line_.size()));

			if (flush_policy_ == LoggerFlushPolicy::every_message)
			{
				file_stream_.flush();
			}
		}
		catch (...)
		{
			is_file_open_ = false;
		}
	}
}

void LoggerImpl::write_sync(LoggerMessageType message_type, StringView message_sv)
{
	try_open_file();
	write_internal(message_type, message_sv);
}

void LoggerImpl::write_async(LoggerMessageType message_type, StringView message_sv)
{
	{
		LockGuard lock_guard{cv_mutex_};

		if (is_thread_failed_)
		{
			return;
		}

		if (exception_ptr_ != nullptr)
		{
			is_thread_failed_ = true;
			std::rethrow_exception(exception_ptr_);
		}

		auto& queue = queues_[producer_queue_index_];
		queue.enqueue(message_type, message_sv.get_data(), message_sv.get_size());

		has_messages_ = true;
	}

	cv_.notify_one();
}

void LoggerImpl::flush_internal()
{
	if (is_file_open_)
	{
		file_stream_.flush();
	}

	std::cerr.flush();
	std::cout.flush();
}

void LoggerImpl::acknowledge_flush()
{
	{
		LockGuard lock_guard{cv_flush_mutex_};
		is_flush_acknowledged_ = true;
	}

	cv_flush_.notify_one();
}

void LoggerImpl::flush_sync()
{
	flush_internal();
}

void LoggerImpl::flush_async()
{
	{
		LockGuard lock_guard{cv_flush_mutex_};
		is_flush_acknowledged_ = false;
	}

	{
		LockGuard lock_guard{cv_mutex_};
		is_flush_requested_ = true;

		if (is_thread_failed_)
		{
			return;
		}

		if (exception_ptr_ != nullptr)
		{
			is_thread_failed_ = true;
			std::rethrow_exception(exception_ptr_);
		}
	}

	cv_.notify_one();

	{
		UniqueLock cv_lock{cv_flush_mutex_};
		cv_flush_.wait(cv_lock, [this]() { return is_flush_acknowledged_; });
	}
}

void LoggerImpl::thread_main_proxy() noexcept
{
	try
	{
		thread_main();
	}
	catch (...)
	{
		{
			LockGuard lock_guard{cv_mutex_};
			exception_ptr_ = std::current_exception();
		}

		{
			LockGuard lock_guard{cv_flush_mutex_};
			is_flush_acknowledged_ = true;
		}
	}
}

void LoggerImpl::thread_main()
{
	using QueueIndices = std::vector<std::intptr_t>;

	auto is_cancellation_requested = false;
	auto is_flush_requested = false;
	auto message_type = LoggerMessageType{};
	auto message = static_cast<const char*>(nullptr);
	auto message_length = std::intptr_t{};

	auto queue_indices = QueueIndices{};
	queue_indices.reserve(2);

	while (!is_cancellation_requested)
	{
		{
			UniqueLock cv_lock{cv_mutex_};

			cv_.wait(
				cv_lock,
				[this]()
				{
					return has_messages_ || is_cancellation_requested_ || is_flush_requested_;
				});

			is_cancellation_requested |= is_cancellation_requested_;

			is_flush_requested |= is_flush_requested_;
			is_flush_requested_ = false;

			has_messages_ = false;

			std::swap(consumer_queue_index_, producer_queue_index_);
		}

		try_open_file();

		queue_indices = {consumer_queue_index_};

		if (is_cancellation_requested)
		{
			queue_indices.emplace_back(producer_queue_index_);
		}

		for (const auto& queue_index : queue_indices)
		{
			auto& queue = queues_[queue_index];

			while (queue.dequeue(message_type, message, message_length))
			{
				write_internal(message_type, StringView{message, message_length});
			}

			queue.clear();
		}

		if (is_flush_requested)
		{
			is_flush_requested = false;
			flush_internal();
			acknowledge_flush();
		}
	}
}

} // namespace

// ==========================================================================

LoggerUPtr make_logger(const LoggerOpenParam& param)
{
	return std::make_unique<LoggerImpl>(param);
}

} // namespace bstone
