/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2019 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
#include "bstone_mt_task_manager.h"

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
// MtFlag
//

class MtFlag
{
public:
	static constexpr int default_spin_count = 4096;


	explicit MtFlag(
		const int spin_count = default_spin_count);

	MtFlag(
		const MtFlag& that) = delete;

	MtFlag& operator=(
		const MtFlag& that) = delete;

	~MtFlag();


	void lock() noexcept;

	void unlock() noexcept;


private:
	using Flag = std::atomic_flag;


	volatile int spin_count_;
	Flag flag_;
}; // MtFlag

using MtFlagLock = std::unique_lock<MtFlag>;

//
// MtFlag
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
	Items items_;
	MtFlag mt_flag_;
}; // MtTaskQueue

//
// MtTaskQueue
// ==========================================================================


} // detail


// ==========================================================================
// GenericMtTaskManager
//

class GenericMtTaskManager :
	public MtTaskManager
{
public:
	GenericMtTaskManager(
		const int concurrency_reserve,
		const int max_task_count);

	~GenericMtTaskManager() override;


	int concurrency_get_max() const noexcept override;

	int concurrency_get() const noexcept override;

	bool has_concurrency() const noexcept override;


	void add_tasks_and_wait_for_added(
		MtTaskPtr* const mt_tasks,
		const int mt_task_count) override;


private:
	using MtThreads = std::vector<std::thread>;


	int concurrency_max_;
	int concurrency_;
	int concurrency_reserve_;

	bool mt_is_quit_;

	detail::MtTaskQueue mt_task_queue_;
	MtThreads mt_threads_;


	void initialize_concurrency();

	void initialize_threads();

	void initialize();

	void uninitialize();

	void mt_thread_func();
}; // MtTaskManager

using GenericMtTaskManagerPtr = GenericMtTaskManager*;
using GenericMtTaskManagerUPtr = std::unique_ptr<GenericMtTaskManager>;

//
// GenericMtTaskManager
// ==========================================================================


namespace detail
{


// ==========================================================================
// MtFlag
//

MtFlag::MtFlag(
	const int spin_count)
	:
	spin_count_{spin_count},
	flag_{ATOMIC_FLAG_INIT}
{
	if (spin_count_ < 0)
	{
		throw Exception{"Spin count out of range."};
	}
}

MtFlag::~MtFlag() = default;

void MtFlag::lock() noexcept
{
	while (flag_.test_and_set(std::memory_order_acquire))
	{
		for (int i = 0; i < spin_count_; ++i)
		{
		}
	}
}

void MtFlag::unlock() noexcept
{
	flag_.clear(std::memory_order::memory_order_release);
}

//
// MtFlag
// ==========================================================================


// ==========================================================================
// MtTaskQueue
//

MtTaskQueue::MtTaskQueue(
	const int size)
	:
	size_{},
	mt_read_index_{ATOMIC_VAR_INIT(0)},
	mt_write_index_{ATOMIC_VAR_INIT(0)},
	items_{},
	mt_flag_{}
{
	if (size <= 0)
	{
		throw Exception{"Max size out of range."};
	}

	size_ = static_cast<Index>(size);
	items_.resize(size_);
}

void MtTaskQueue::push(
	const MtTaskPtr mt_task)
{
	if (!mt_task)
	{
		throw Exception{"Null task."};
	}

	MtFlagLock flag_lock{mt_flag_};

	const auto read_index = mt_read_index_.load(std::memory_order_acquire);
	const auto write_index = mt_write_index_.load(std::memory_order_acquire);

	const auto new_write_index = (write_index + 1) % size_;

	if (read_index == new_write_index)
	{
		throw Exception{"Queue overflow."};
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
		throw Exception{"Null task list."};
	}

	if (mt_task_count <= 0)
	{
		throw Exception{"Task count out of range."};
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
		throw Exception{"Null task."};
	}

	MtFlagLock flag_lock{mt_flag_};

	const auto read_index = mt_read_index_.load(std::memory_order_acquire);

	auto write_index = mt_write_index_.load(std::memory_order_acquire);
	auto new_write_index = write_index;

	for (int i = 0; i < mt_task_count; ++i)
	{
		new_write_index += 1;
		new_write_index %= size_;

		if (read_index == new_write_index)
		{
			throw Exception{"Queue overflow."};
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
	MtFlagLock flag_lock{mt_flag_};

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
// MtTaskManager
//

MtTaskManager::MtTaskManager() = default;

MtTaskManager::~MtTaskManager() = default;

//
// MtTaskManager
// ==========================================================================


// ==========================================================================
// GenericMtTaskManager
//

GenericMtTaskManager::GenericMtTaskManager(
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

GenericMtTaskManager::~GenericMtTaskManager()
{
	uninitialize();
}

int GenericMtTaskManager::concurrency_get_max() const noexcept
{
	return concurrency_max_;
}

int GenericMtTaskManager::concurrency_get() const noexcept
{
	return concurrency_;
}

bool GenericMtTaskManager::has_concurrency() const noexcept
{
	return concurrency_ > 1;
}

void GenericMtTaskManager::add_tasks_and_wait_for_added(
	MtTaskPtr* const mt_tasks,
	const int mt_task_count)
{
	mt_task_queue_.push(mt_tasks, mt_task_count);

	auto is_completed = false;

	while (!is_completed)
	{
		is_completed = std::all_of(
			mt_tasks,
			mt_tasks + mt_task_count,
			[](const auto item)
			{
				return item->is_completed();
			}
		);
	}
}

void GenericMtTaskManager::initialize_concurrency()
{
	concurrency_max_ = static_cast<int>(std::thread::hardware_concurrency());

	if (concurrency_reserve_ < 0)
	{
		throw Exception{"Concurrency reserve out of range."};
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

void GenericMtTaskManager::initialize_threads()
{
	if (!has_concurrency())
	{
		return;
	}

	mt_threads_.resize(concurrency_);

	for (auto& mt_thread : mt_threads_)
	{
		mt_thread = std::move(std::thread{&GenericMtTaskManager::mt_thread_func, this});
	}
}

void GenericMtTaskManager::initialize()
{
	initialize_concurrency();
	initialize_threads();
}

void GenericMtTaskManager::uninitialize()
{
	mt_is_quit_ = true;

	if (has_concurrency())
	{
		for (auto& mt_thread : mt_threads_)
		{
			if (mt_thread.joinable())
			{
				mt_thread.join();
			}
		}
	}
}

void GenericMtTaskManager::mt_thread_func()
{
	const auto sleep_duration_ms = std::chrono::milliseconds{1};

	while (!mt_is_quit_)
	{
		auto task = MtTaskPtr{};

		if (mt_task_queue_.pop(task))
		{
			(*task)();

			task->set_is_completed(true);
		}
		else
		{
			std::this_thread::sleep_for(sleep_duration_ms);
		}
	}
}

//
// GenericMtTaskManager
// ==========================================================================


// ==========================================================================
// MtTaskManagerFactory

MtTaskManagerUPtr MtTaskManagerFactory::create(
	const int concurrency_reserve,
	const int max_task_count)
{
	return std::make_unique<GenericMtTaskManager>(concurrency_reserve, max_task_count);
}

// MtTaskManagerFactory
// ==========================================================================


} // bstone
