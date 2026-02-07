/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
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
	MtTask() = default;

	virtual ~MtTask() = default;


	virtual void execute() = 0;


	virtual bool is_completed() const = 0;

	virtual void set_completed() = 0;


	virtual bool is_failed() const = 0;

	virtual std::exception_ptr get_exception_ptr() const = 0;

	virtual void set_failed(
		std::exception_ptr exception_ptr) = 0;
}; // MtTask

//
// MtTask
// ==========================================================================


// ==========================================================================
// MtTaskMgr
//

class MtTaskMgr
{
public:
	MtTaskMgr() = default;

	virtual ~MtTaskMgr() = default;


	virtual int get_max_threads() const = 0;

	virtual int get_thread_count() const = 0;


	virtual void add_tasks(
		MtTask** tasks,
		int task_count) = 0;

	virtual void add_tasks_and_wait_for_added(
		MtTask** tasks,
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
