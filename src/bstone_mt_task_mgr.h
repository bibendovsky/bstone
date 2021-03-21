/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
protected:
	MtTask();

public:
	virtual ~MtTask();


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
protected:
	MtTaskMgr();


public:
	virtual ~MtTaskMgr();


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
