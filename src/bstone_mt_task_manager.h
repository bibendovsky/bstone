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


#ifndef BSTONE_MT_TASK_MANAGER_INCLUDED
#define BSTONE_MT_TASK_MANAGER_INCLUDED


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
		const std::exception_ptr exception_ptr) = 0;
}; // MtTask

using MtTaskPtr = MtTask*;
using MtTaskUPtr = std::unique_ptr<MtTask>;

//
// MtTask
// ==========================================================================


// ==========================================================================
// MtTaskManager
//

class MtTaskManager
{
protected:
	MtTaskManager();


public:
	virtual ~MtTaskManager();


	virtual int concurrency_get_max() const noexcept = 0;

	virtual int concurrency_get() const noexcept = 0;

	virtual bool has_concurrency() const noexcept = 0;


	virtual void add_tasks_and_wait_for_added(
		MtTaskPtr* const mt_tasks,
		const int mt_task_count) = 0;
}; // MtTaskManager

using MtTaskManagerPtr = MtTaskManager*;
using MtTaskManagerUPtr = std::unique_ptr<MtTaskManager>;

//
// MtTaskManager
// ==========================================================================


// ==========================================================================
// MtTaskManagerFactory

struct MtTaskManagerFactory final
{
	static MtTaskManagerUPtr create(
		const int concurrency_reserve,
		const int max_task_count);
}; // MtTaskManagerFactory

// MtTaskManagerFactory
// ==========================================================================


} // bstone


#endif // !BSTONE_MT_TASK_MANAGER_INCLUDED
