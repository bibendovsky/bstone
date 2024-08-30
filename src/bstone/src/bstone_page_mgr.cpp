/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// Page (VSWAP) manager.
//


#include "bstone_page_mgr.h"

#include <string>
#include <vector>

#include "id_ca.h"

#include "bstone_endian.h"
#include "bstone_exception.h"
#include "bstone_file_stream.h"


namespace bstone
{

PageMgr::PageMgr() noexcept = default;

PageMgr::~PageMgr() = default;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

class PageMgrImpl final :
	public PageMgr
{
public:
	PageMgrImpl();
	~PageMgrImpl() override;


	// =========================================================================
	// PageMgr

	int get_count() const noexcept override;

	int get_wall_count() const noexcept override;

	int get_sprite_count() const noexcept override;


	const std::uint8_t* get(
		int index) const override;

	const std::uint8_t* get_audio(
		int audio_index) const override;

	const std::uint8_t* get_last_audio() const override;

	const std::uint8_t* get_sprite(
		int sprite_index) const override;

	// PageMgr
	// =========================================================================


private:
	using Pages = std::vector<std::uint8_t>;
	using PagePtrs = std::vector<const std::uint8_t*>;


	int count_{};
	int wall_count_{};
	int sprite_count_{};
	int audio_count_{};

	int sprite_base_index_{};
	int audio_base_index_{};

	Pages pages_{};
	PagePtrs pages_ptrs_{};


	void load_vswap();
}; // PageMgrImpl

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

PageMgrImpl::PageMgrImpl()
{
	load_vswap();
}

PageMgrImpl::~PageMgrImpl() = default;

int PageMgrImpl::get_count() const noexcept
{
	return count_;
}

int PageMgrImpl::get_wall_count() const noexcept
{
	return wall_count_;
}

int PageMgrImpl::get_sprite_count() const noexcept
{
	return sprite_count_;
}

const std::uint8_t* PageMgrImpl::get(
	int index) const
{
	if (index < 0 || index >= count_)
	{
		BSTONE_THROW_STATIC_SOURCE("Page index out of range.");
	}

	const auto page_ptr = pages_ptrs_[index];

	if (!page_ptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Sparse page.");
	}

	return page_ptr;
}

const std::uint8_t* PageMgrImpl::get_audio(
	int audio_index) const
{
	if (audio_index < 0 || audio_index >= audio_count_)
	{
		BSTONE_THROW_STATIC_SOURCE("Audio page index out of range.");
	}

	return get(audio_base_index_ + audio_index);
}

const std::uint8_t* PageMgrImpl::get_last_audio() const
{
	return get(count_ - 1);
}

const std::uint8_t* PageMgrImpl::get_sprite(
	int sprite_index) const
{
	if (sprite_index < 0 || sprite_index >= sprite_count_)
	{
		BSTONE_THROW_STATIC_SOURCE("Sprite page index out of range.");
	}

	return get(sprite_base_index_ + sprite_index);
}

void PageMgrImpl::load_vswap()
{
	constexpr auto max_vswap_size = 4 * 1'024 * 1'024;
	constexpr auto page_size = 4'096;

	auto vswap_file = FileStream{};

	ca_open_resource(AssetsResourceType::vswap, vswap_file);

	if (!vswap_file.is_open())
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to open VSWAP.");
	}

	const auto file_size = vswap_file.get_size();

	if (file_size < page_size || file_size > max_vswap_size)
	{
		BSTONE_THROW_STATIC_SOURCE("File size out of range.");
	}

	const auto vswap_size = static_cast<int>(file_size);
	const auto pages_size = vswap_size + page_size;

	pages_.resize(pages_size);

	auto pages_bytes = pages_.data();
	vswap_file.read_exactly(pages_bytes, vswap_size);

	const auto u16_elements = reinterpret_cast<const std::uint16_t*>(pages_bytes);

	count_ = bstone::endian::to_little(u16_elements[0]);
	sprite_base_index_ = bstone::endian::to_little(u16_elements[1]);
	audio_base_index_ = bstone::endian::to_little(u16_elements[2]);

	pages_ptrs_.resize(count_ + 1);

	const auto page_offsets = reinterpret_cast<const std::uint32_t*>(&pages_bytes[6]);

	for (auto i = 0; i <= count_; ++i)
	{
		const auto page_offset = bstone::endian::to_little(page_offsets[i]);

		if (page_offset > 0)
		{
			pages_ptrs_[i] = pages_bytes + page_offset;
		}
	}

	wall_count_ = sprite_base_index_;
	sprite_count_ = audio_base_index_ - sprite_base_index_;
	audio_count_ = count_ - audio_base_index_;
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

PageMgrUPtr make_page_mgr()
{
	return std::make_unique<PageMgrImpl>();
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // bstone
