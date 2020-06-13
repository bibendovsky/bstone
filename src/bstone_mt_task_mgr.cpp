/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2020 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


//
// Multi-threaded task manager.
//

#include "bstone_precompiled.h"

#include "bstone_mt_task_mgr.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <limits>
#include <mutex>
#include <thread>
#include <vector>

#include "bstone_exception.h"


namespace bstone
{


namespace detail
{


// ==========================================================================
// MtSpinFlag
//

class MtSpinFlag
{
public:
	static constexpr int default_spin_count = 4096;


	explicit MtSpinFlag(
		const int spin_count = default_spin_count);

	MtSpinFlag(
		const MtSpinFlag& that) = delete;

	MtSpinFlag& operator=(
		const MtSpinFlag& that) = delete;

	~MtSpinFlag();


	void lock() noexcept;

	void unlock() noexcept;


private:
	using Flag = std::atomic_flag;


	volatile int spin_count_;
	Flag flag_;
}; // MtSpinFlag

using MtSpinFlagLock = std::unique_lock<MtSpinFlag>;

//
// MtSpinFlag
// ==========================================================================


// ==========================================================================
// MtTaskQueue
//

class MtTaskQueue
{
public:
	explicit MtTaskQueue(
		const int size);

	MtTaskQueue(
		const MtTaskQueue& that) = delete;

	MtTaskQueue& operator=(
		const MtTaskQueue& that) = delete;


	void push(
		const MtTaskPtr mt_task);

	void push(
		MtTaskPtr* const mt_tasks,
		const int mt_task_count);

	bool pop(
		MtTaskPtr& mt_task) noexcept;


private:
	using Items = std::vector<MtTaskPtr>;
	using Index = unsigned int;
	using MtIndex = std::atomic<Index>;


	Index size_;
	MtIndex mt_read_index_;
	MtIndex mt_write_index_;
	MtSpinFlag mt_spin_flag_;
	Items items_;
}; // MtTaskQueue

//
// MtTaskQueue
// ==========================================================================


} // detail


// ==========================================================================
// MtTaskMgrImplException
//

class MtTaskMgrImplException :
	public Exception
{
public:
	explicit MtTaskMgrImplException(
		const char* const message)
		:
		Exception{message}
	{
	}
}; // MtTaskMgrImplException

//
// MtTaskMgrImplException
// ==========================================================================


// ==========================================================================
// MtTaskMgrImpl
//

class MtTaskMgrImpl :
	public MtTaskMgr
{
public:
	MtTaskMgrImpl(
		const int concurrency_reserve,
		const int max_task_count);

	~MtTaskMgrImpl() override;


	int get_max_concurrency() const noexcept override;

	int get_concurrency() const noexcept override;

	bool has_concurrency() const noexcept override;


	void add_tasks_and_wait_for_added(
		MtTaskPtr* const mt_tasks,
		const int mt_task_count) override;


private:
	struct MtThread
	{
		bool is_failed_;
		std::exception_ptr exception_;

		std::thread thread_;
	}; // MtThread

	using MtThreadPtr = MtThread*;

	using MtThreads = std::vector<MtThread>;


	int concurrency_max_;
	int concurrency_;
	int concurrency_reserve_;

	std::atomic_bool mt_is_quit_;

	detail::MtTaskQueue mt_task_queue_;
	MtThreads mt_threads_;


	void initialize_concurrency();

	void initialize_threads();

	void initialize();

	void uninitialize();

	bool mt_is_quit();

	//
	// Returns:
	//    - "true" if task was picked and executed.
	//    - "false" if there are no more tasks.
	//
	bool try_pick_and_execute();

	void mt_thread_func(
		MtThread* mt_thread);
}; // MtTaskMgr

using MtTaskMgrImplPtr = MtTaskMgrImpl*;
using MtTaskMgrImplUPtr = std::unique_ptr<MtTaskMgrImpl>;

//
// MtTaskMgrImpl
// ==========================================================================


namespace detail
{


// ==========================================================================
// MtSpinFlag
//

MtSpinFlag::MtSpinFlag(
	const int spin_count)
	:
	spin_count_{spin_count},
	flag_{}
{
	if (spin_count_ < 0)
	{
		throw MtTaskMgrImplException{"Spin count out of range."};
	}
}

MtSpinFlag::~MtSpinFlag() = default;

void MtSpinFlag::lock() noexcept
{
	while (flag_.test_and_set(std::memory_order_acquire))
	{
		for (int i = 0; i < spin_count_; ++i)
		{
		}
	}
}

void MtSpinFlag::unlock() noexcept
{
	flag_.clear(std::memory_order_release);
}

//
// MtSpinFlag
// ==========================================================================


// ==========================================================================
// MtTaskQueue
//

MtTaskQueue::MtTaskQueue(
	const int size)
	:
	size_{},
	mt_read_index_{},
	mt_write_index_{},
	mt_spin_flag_{},
	items_{}
{
	if (size <= 0)
	{
		throw MtTaskMgrImplException{"Max size out of range."};
	}

	size_ = static_cast<Index>(size);
	items_.resize(size_);
}

void MtTaskQueue::push(
	const MtTaskPtr mt_task)
{
	if (!mt_task)
	{
		throw MtTaskMgrImplException{"Null task."};
	}

	MtSpinFlagLock flag_lock{mt_spin_flag_};

	const auto read_index = mt_read_index_.load(std::memory_order_acquire);
	const auto write_index = mt_write_index_.load(std::memory_order_acquire);

	const auto new_write_index = (write_index + 1) % size_;

	if (read_index == new_write_index)
	{
		throw MtTaskMgrImplException{"Queue overflow."};
	}

	items_[write_index] = mt_task;

	mt_write_index_.store(new_write_index, std::memory_order_release);
}

void MtTaskQueue::push(
	MtTaskPtr* const mt_tasks,
	const int mt_task_count)
{
	if (!mt_tasks)
	{
		throw MtTaskMgrImplException{"Null task list."};
	}

	if (mt_task_count <= 0)
	{
		throw MtTaskMgrImplException{"Task count out of range."};
	}

	const auto has_null = std::any_of(
		mt_tasks,
		mt_tasks + mt_task_count,
		[](const auto item)
		{
			return !item;
		}
	);

	if (has_null)
	{
		throw MtTaskMgrImplException{"Null task."};
	}

	MtSpinFlagLock flag_lock{mt_spin_flag_};

	const auto read_index = mt_read_index_.load(std::memory_order_acquire);

	auto write_index = mt_write_index_.load(std::memory_order_acquire);
	auto new_write_index = write_index;

	for (int i = 0; i < mt_task_count; ++i)
	{
		new_write_index += 1;
		new_write_index %= size_;

		if (read_index == new_write_index)
		{
			throw MtTaskMgrImplException{"Queue overflow."};
		}

		items_[write_index] = mt_tasks[i];

		write_index += 1;
		write_index %= size_;
	}

	mt_write_index_.store(new_write_index, std::memory_order_release);
}

bool MtTaskQueue::pop(
	MtTaskPtr& mt_task) noexcept
{
	MtSpinFlagLock flag_lock{mt_spin_flag_};

	const auto read_index = mt_read_index_.load(std::memory_order_acquire);
	const auto write_index = mt_write_index_.load(std::memory_order_acquire);

	if (read_index == write_index)
	{
		return false;
	}

	const auto new_read_index = (read_index + 1) % size_;

	mt_task = items_[read_index];

	mt_read_index_.store(new_read_index, std::memory_order_release);

	return true;
}

//
// MtTaskQueue
// ==========================================================================


} // detail


// ==========================================================================
// MtTask
//

MtTask::MtTask() = default;

MtTask::~MtTask() = default;

//
// MtTask
// ==========================================================================


// ==========================================================================
// MtTaskMgr
//

MtTaskMgr::MtTaskMgr() = default;

MtTaskMgr::~MtTaskMgr() = default;

//
// MtTaskMgr
// ==========================================================================


// ==========================================================================
// MtTaskMgrImpl
//

MtTaskMgrImpl::MtTaskMgrImpl(
	const int concurrency_reserve,
	const int max_task_count)
	:
	concurrency_max_{},
	concurrency_{},
	concurrency_reserve_{concurrency_reserve},
	mt_is_quit_{},
	mt_task_queue_{max_task_count},
	mt_threads_{}
{
	initialize();
}

MtTaskMgrImpl::~MtTaskMgrImpl()
{
	uninitialize();
}

int MtTaskMgrImpl::get_max_concurrency() const noexcept
{
	return concurrency_max_;
}

int MtTaskMgrImpl::get_concurrency() const noexcept
{
	return concurrency_;
}

bool MtTaskMgrImpl::has_concurrency() const noexcept
{
	return concurrency_ > 1;
}

void MtTaskMgrImpl::add_tasks_and_wait_for_added(
	MtTaskPtr* const mt_tasks,
	const int mt_task_count)
{
	mt_task_queue_.push(mt_tasks, mt_task_count);

	auto is_quit = false;
	auto is_completed = false;

	while (!is_quit && !is_completed)
	{
		try_pick_and_execute();

		is_completed = std::all_of(
			mt_tasks,
			mt_tasks + mt_task_count,
			[](const auto item)
			{
				return item->is_completed();
			}
		);

		is_quit = mt_is_quit();
	}

	for (auto& mt_thread : mt_threads_)
	{
		if (mt_thread.is_failed_)
		{
			std::rethrow_exception(mt_thread.exception_);
		}
	}

	for (int i = 0; i < mt_task_count; ++i)
	{
		const auto& mt_task = mt_tasks[i];

		if (mt_task->is_failed())
		{
			std::rethrow_exception(mt_task->get_exception_ptr());
		}
	}
}

void MtTaskMgrImpl::initialize_concurrency()
{
	concurrency_max_ = static_cast<int>(std::thread::hardware_concurrency());

	if (concurrency_reserve_ < 0)
	{
		throw MtTaskMgrImplException{"Concurrency reserve out of range."};
	}

	concurrency_ = concurrency_max_ - concurrency_reserve_;

	if (concurrency_ <= 0)
	{
		concurrency_ = 1;
	}

	if (concurrency_ > concurrency_max_)
	{
		concurrency_ = concurrency_max_;
	}
}

void MtTaskMgrImpl::initialize_threads()
{
	if (!has_concurrency())
	{
		return;
	}

	mt_threads_.resize(concurrency_);

	for (auto& mt_thread : mt_threads_)
	{
		mt_thread.is_failed_ = false;
		mt_thread.exception_ = nullptr;

		mt_thread.thread_ = std::move(std::thread{&MtTaskMgrImpl::mt_thread_func, this, &mt_thread});
	}
}

void MtTaskMgrImpl::initialize()
{
	initialize_concurrency();
	initialize_threads();
}

void MtTaskMgrImpl::uninitialize()
{
	mt_is_quit_.store(true, std::memory_order_release);

	if (has_concurrency())
	{
		for (auto& mt_thread : mt_threads_)
		{
			if (mt_thread.thread_.joinable())
			{
				mt_thread.thread_.join();
			}
		}
	}
}

bool MtTaskMgrImpl::mt_is_quit()
{
	return mt_is_quit_.load(std::memory_order_acquire);
}

bool MtTaskMgrImpl::try_pick_and_execute()
{
	auto mt_task = MtTaskPtr{};

	if (!mt_task_queue_.pop(mt_task))
	{
		return false;
	}

	try
	{
		mt_task->execute();
		mt_task->set_completed();
	}
	catch (...)
	{
		mt_task->set_failed(std::current_exception());
	}

	return true;
}

void MtTaskMgrImpl::mt_thread_func(
	MtThread* mt_thread)
{
	const auto sleep_duration_ms = std::chrono::milliseconds{1};

	try
	{
		while (!mt_is_quit())
		{
			if (!try_pick_and_execute())
			{
				std::this_thread::sleep_for(sleep_duration_ms);
			}
		}
	}
	catch (...)
	{
		mt_thread->is_failed_ = true;
		mt_thread->exception_ = std::current_exception();
	}
}

//
// MtTaskMgrImpl
// ==========================================================================


MtTaskMgrUPtr make_mt_task_manager(
	const int concurrency_reserve,
	const int max_task_count)
{
	return std::make_unique<MtTaskMgrImpl>(concurrency_reserve, max_task_count);
}


} // bstone
