/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// Page (VSWAP) manager.
//


#ifndef BSTONE_PAGE_MANAGER_INCLUDED
#define BSTONE_PAGE_MANAGER_INCLUDED


#include <cstdint>

#include <memory>


namespace bstone
{


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//
// Page (VSWAP) manager.
//
class PageMgr
{
public:
	PageMgr() noexcept;

	virtual ~PageMgr();


	virtual int get_count() const noexcept = 0;

	virtual int get_wall_count() const noexcept = 0;

	virtual int get_sprite_count() const noexcept = 0;


	virtual const std::uint8_t* get(
		int index) const = 0;

	virtual const std::uint8_t* get_audio(
		int audio_index) const = 0;

	virtual const std::uint8_t* get_last_audio() const = 0;

	virtual const std::uint8_t* get_sprite(
		int sprite_index) const = 0;
}; // PageMgr

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

using PageMgrUPtr = std::unique_ptr<PageMgr>;

PageMgrUPtr make_page_mgr();

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // bstone


#endif // !BSTONE_PAGE_MANAGER_INCLUDED
