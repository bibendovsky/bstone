/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_vk_r3r_observer.h"
#include "bstone_exception.h"
#include <algorithm>

VkR3rPostPresentSubject::VkR3rPostPresentSubject()
{
	observers_.reserve(64);
	observers_copy_.reserve(64);
}

void VkR3rPostPresentSubject::do_attach(Observer& observer)
try {
	if (!observers_.emplace(&observer).second)
	{
		BSTONE_THROW_STATIC_SOURCE("Duplicate observer.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void VkR3rPostPresentSubject::do_detach(Observer& observer)
try {
	if (observers_.erase(&observer) == 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Detached observer.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void VkR3rPostPresentSubject::do_notify()
{
	observers_copy_ = observers_;
	for (Observer* observer : observers_copy_)
	{
		observer->update();
	}
}
