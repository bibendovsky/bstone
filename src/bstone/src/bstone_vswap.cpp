/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// VSWAP file

/*
File format:
u16 chunk_count
u16 sprite_start
u16 sound_start
u32 chunk_offsets[chunk_count]
u16 chunk_sizes[chunk_count]
u8  data[...]
*/

#include "bstone_vswap.h"
#include "bstone_endian.h"
#include "bstone_exception.h"
#include "bstone_file_stream.h"
#include "id_ca.h"

namespace bstone {

namespace {

class VswapImpl final : public Vswap
{
public:
	VswapImpl();
	~VswapImpl() override = default;

	int get_chunk_count() const override;
	int get_wall_count() const override;
	int get_sprite_count() const override;
	int get_audio_count() const override;
	int get_chunk_size(int index) const override;
	int get_wall_size(int index) const override;
	int get_sprite_size(int index) const override;
	int get_audio_size(int index) const override;
	const std::uint8_t* get_chunk_data(int index) const override;
	const std::uint8_t* get_wall_data(int index) const override;
	const std::uint8_t* get_sprite_data(int index) const override;
	const std::uint8_t* get_audio_data(int index) const override;

private:
	static constexpr int min_file_size = 6;
	static constexpr int max_file_size = 4'000'000;
	static constexpr int min_chunks = 0;
	static constexpr int max_chunks = 1400;

	struct BytesDeleter
	{
		void operator()(void* pointer)
		{
			::operator delete(pointer);
		}
	};

	using Bytes = std::unique_ptr<std::uint8_t[], BytesDeleter>;

	Bytes bytes_{};

	int get_sprite_start() const;
	int get_sound_start() const;
	int get_chunk_offset(int index) const;
	int impl_get_chunk_count() const;
	int impl_get_chunk_size(int index) const;
};

// -------------------------------------

VswapImpl::VswapImpl()
{
	FileStream vswap_file{};
	ca_open_resource(AssetsResourceType::vswap, vswap_file);
	const std::int64_t file_size_s64 = vswap_file.get_size();
	if (file_size_s64 < min_file_size || file_size_s64 > max_file_size)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid file size.");
	}
	const int file_size = static_cast<int>(file_size_s64);
	bytes_.reset(static_cast<unsigned char*>(::operator new(file_size)));
	vswap_file.read_exactly(bytes_.get(), file_size);
	const int chunk_count = impl_get_chunk_count();
	if (chunk_count < min_chunks || chunk_count > max_chunks)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid chunk count.");
	}
	const int sprite_start = get_sprite_start();
	const int sound_start = get_sound_start();
	const int min_sprite_start = min_chunks;
	const int max_sprite_start = sound_start;
	if (sprite_start < min_sprite_start || sprite_start > max_sprite_start)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid sprite start.");
	}
	const int min_sound_start = sprite_start;
	const int max_sound_start = chunk_count;
	if (sound_start < min_sound_start || sound_start > max_sound_start)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid sound start.");
	}
	const int chunk_offsets_size = chunk_count * 4;
	const int chunk_sizes_size = chunk_count * 2;
	const int data_start = 6 + chunk_offsets_size + chunk_sizes_size;
	if (data_start > file_size)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid data start.");
	}
	for (int i = 0; i < chunk_count; ++i)
	{
		const int chunk_offset = get_chunk_offset(i);
		const int chunk_size = impl_get_chunk_size(i);
		if (chunk_offset < data_start || chunk_offset > file_size || chunk_offset + chunk_size > file_size)
		{
			BSTONE_THROW_STATIC_SOURCE("Invalid chunk boundaries.");
		}
	}
}

int VswapImpl::get_chunk_count() const
{
	return impl_get_chunk_count();
}

int VswapImpl::get_wall_count() const
{
	return get_sprite_start();
}

int VswapImpl::get_sprite_count() const
{
	return get_sound_start() - get_sprite_start();
}

int VswapImpl::get_audio_count() const
{
	return get_chunk_count() - get_sound_start();
}

int VswapImpl::get_chunk_size(int index) const
{
	return impl_get_chunk_size(index);
}

int VswapImpl::get_wall_size(int index) const
{
	return get_chunk_size(index);
}

int VswapImpl::get_sprite_size(int index) const
{
	return get_chunk_size(get_sprite_start() + index);
}

int VswapImpl::get_audio_size(int index) const
{
	return get_chunk_size(get_sound_start() + index);
}

const std::uint8_t* VswapImpl::get_chunk_data(int index) const
{
	return bytes_.get() + get_chunk_offset(index);
}

const std::uint8_t* VswapImpl::get_wall_data(int index) const
{
	return get_chunk_data(index);
}

const std::uint8_t* VswapImpl::get_sprite_data(int index) const
{
	return get_chunk_data(get_sprite_start() + index);
}

const std::uint8_t* VswapImpl::get_audio_data(int index) const
{
	return get_chunk_data(get_sound_start() + index);
}

int VswapImpl::get_sprite_start() const
{
	return endian::read_u16_le(bytes_.get() + 2);
}

int VswapImpl::get_sound_start() const
{
	return endian::read_u16_le(bytes_.get() + 4);
}

int VswapImpl::get_chunk_offset(int index) const
{
	return endian::read_s32_le(bytes_.get() + 6 + index * 4);
}

int VswapImpl::impl_get_chunk_count() const
{
	return endian::read_u16_le(bytes_.get());
}

int VswapImpl::impl_get_chunk_size(int index) const
{
	return endian::read_u16_le(bytes_.get() + 6 + impl_get_chunk_count() * 4 + index * 2);
}

} // namespace

// =====================================

VswapUPtr make_vswap()
{
	return std::make_unique<VswapImpl>();
}

} // namespace bstone
