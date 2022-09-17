/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// Multi-threaded task manager.
//


#ifndef BSTONE_MT_TASK_MGR_INCLUDED
#define BSTONE_MT_TASK_MGR_INCLUDED


#include <memory>


namespace bstone
{


// ==========================================================================
// MtTask
//

class MtTask
{
public:
	MtTask() noexcept = default;

	virtual ~MtTask() = default;


	virtual void execute() = 0;


	virtual bool is_completed() const noexcept = 0;

	virtual void set_completed() = 0;


	virtual bool is_failed() const noexcept = 0;

	virtual std::exception_ptr get_exception_ptr() const noexcept = 0;

	virtual void set_failed(
		std::exception_ptr exception_ptr) = 0;
}; // MtTask

using MtTaskPtr = MtTask*;

//
// MtTask
// ==========================================================================


// ==========================================================================
// MtTaskMgr
//

class MtTaskMgr
{
public:
	MtTaskMgr() noexcept = default;

	virtual ~MtTaskMgr() = default;


	virtual int get_max_threads() const noexcept = 0;

	virtual int get_thread_count() const noexcept = 0;


	virtual void add_tasks(
		MtTaskPtr* tasks,
		int task_count) = 0;

	virtual void add_tasks_and_wait_for_added(
		MtTaskPtr* tasks,
		int task_count) = 0;
}; // MtTaskMgr


using MtTaskMgrUPtr = std::unique_ptr<MtTaskMgr>;

//
// MtTaskMgr
// ==========================================================================


MtTaskMgrUPtr make_mt_task_manager(
	int concurrency_reserve,
	int max_task_count);


} // bstone


#endif // !BSTONE_MT_TASK_MGR_INCLUDED
