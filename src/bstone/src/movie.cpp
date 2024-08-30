/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


#include "an_codes.h"
#include "id_ca.h"
#include "id_heads.h"
#include "id_in.h"
#include "id_sd.h"
#include "id_vh.h"
#include "id_vl.h"
#include "jm_vl.h"
#include "movie.h"
#include "bstone_archiver.h"
#include "bstone_endian.h"


class Movie
{
public:
	//
	// Playes an animation.
	//
	// Returns:
	//    - True if movie file was found and "played".
	//    - False otherwise.
	//
	bool play(
		const MovieId movie_id,
		const std::uint8_t* const palette);


private:
	struct Descriptor
	{
		AssetsResourceType assets_resource_type;

		std::int8_t repeat_count_;
		std::int8_t tick_delay_;
	}; // descriptor


	struct AnimFrame
	{
		static constexpr auto class_size = 12;


		std::uint16_t code;
		std::int32_t block_num;
		std::int32_t recsize;
	}; // AnimFrame

	static_assert(AnimFrame::class_size == sizeof(AnimFrame), "Class size mismatch.");


	struct AnimChunk
	{
		static constexpr auto class_size = 6;


		std::uint16_t opt;
		std::uint16_t offset;
		std::uint16_t length;


		void endian()
		{
			opt = bstone::endian::to_little(opt);
			offset = bstone::endian::to_little(offset);
			length = bstone::endian::to_little(length);
		}
	}; // AnimChunk

	static_assert(AnimChunk::class_size == sizeof(AnimChunk), "Class size mismatch.");


	static constexpr auto max_movies = 4;

	using Descriptors = std::array<Descriptor, max_movies>;


	enum class Flag
	{
		none,
		fill,
		skip,
	}; // Flag


	static constexpr auto max_buffer_size = 65'536;


	using Buffer = std::vector<char>;

	bstone::FileStream file_stream_;

	Buffer buffer_;
	int buffer_offset_{}; // Len of data loaded into buffer_
	char* buffer_ptr_{}; // Ptr to next frame in buffer_
	char* next_ptr_{}; // Ptr Ofs to next frame after BufferOfs

	bool has_more_pages_{}; // More Pages avail on disk?

	Flag flag_{};
	bool is_exit_{};
	bool is_ever_faded_{};
	int repeat_count_{};
	ControlInfo control_info_{};
	const std::uint8_t* palette_{};

	bstone::Archiver archiver_;


	const Descriptor& get_descriptor(
		const MovieId movie_id);

	//
	// Inits all the internal routines for the Movie Presenter
	//
	void initialize(
		const Descriptor& MovieDescriptor,
		const std::uint8_t* const palette);

	void uninitialize();

	//
	// Draws a block of image.
	//
	// Parameters:
	//    - byte_offset - offset for the image to be drawn.
	//
	//    - source - source image of graphic to be blasted to latch memory.
	//               This pic is NOT 'munged'.
	//
	//    - length = length of the source image in bytes
	//
	void jm_draw_block(
		const int byte_offset,
		const char* const source,
		const int length);

	//
	// Shows an animation frame
	//
	// Parameters:
	//    - inpic - pointer to animpic.
	//
	void show_frame(
		char* inpic);

	//
	// Loads the RAM Buffer full of graphics.
	//
	// Returns:
	//    - True if more pages available.
	//    - False otherwise.
	//
	bool load_buffer();

	//
	// Returns pointer to next Block/Screen of animation
	//
	// This function "buffers" the movie presentation.
	// It loads and buffers incomming frames of animation.
	//
	// Returns:
	//    - True on success.
	//    - False otherwise.
	//
	bool get_frame();

	//
	// This handles the current page of data from the ram buffer.
	//
	void handle_page(
		const Descriptor& descriptor);
}; // Movie


const Movie::Descriptor& Movie::get_descriptor(
	const MovieId movie_id)
{
	static const auto descriptors = Descriptors
	{{
		{AssetsResourceType::ianim, 1, 3}, // intro
		{AssetsResourceType::eanim, 1, 3}, // final
		{AssetsResourceType::sanim, 1, 3}, // final_2
		{AssetsResourceType::ganim, 1, 3}, // final_3
	}}; // descriptors

	return descriptors[movie_id];
}

void Movie::initialize(
	const Descriptor& descriptor,
	const std::uint8_t* const palette)
{
	repeat_count_ = descriptor.repeat_count_;
	flag_ = Flag::fill;
	buffer_offset_ = 0;
	has_more_pages_ = true;
	is_exit_ = false;
	is_ever_faded_ = false;

	palette_ = palette;

	JM_VGALinearFill(0, vga_ref_width * vga_ref_height, 0);

	VL_FillPalette(0, 0, 0);

	// Find out how much memory we have to work with.
	buffer_.resize(max_buffer_size);

	archiver_.close();

	IN_ClearKeysDown();
}

void Movie::uninitialize()
{
	archiver_.close();
	buffer_.clear();
	file_stream_.close();
}

void Movie::jm_draw_block(
	const int byte_offset,
	const char* const source,
	const int length)
{
	auto x = byte_offset % vga_ref_width;
	auto y = byte_offset / vga_ref_width;

	for (int i = 0; i < length; ++i)
	{
		VL_Plot(x, y, static_cast<std::uint8_t>(source[i]));

		++x;

		if (x == vga_ref_width)
		{
			x = 0;
			++y;
		}
	}
}

void Movie::show_frame(
	char* inpic)
{
	if (!inpic)
	{
		return;
	}

	while (true)
	{
		auto& ah = *reinterpret_cast<AnimChunk*>(inpic);

		if (ah.opt == 0)
		{
			break;
		}

		ah.endian();

		inpic += AnimChunk::class_size;

		jm_draw_block(ah.offset, inpic, ah.length);

		inpic += ah.length;
	}
}

bool Movie::load_buffer()
{
	auto frame = buffer_.data();
	auto free_space = max_buffer_size;

	next_ptr_ = frame;
	buffer_ptr_ = frame;

	auto blk = AnimFrame{};

	while (free_space)
	{
		const auto chunkstart = file_stream_.get_position();

		blk.code = archiver_.read_uint16();
		blk.block_num = archiver_.read_int32();
		blk.recsize = archiver_.read_int32();

		if (blk.code == AN_END_OF_ANIM)
		{
			return false;
		}

		if (free_space >= (blk.recsize + AnimFrame::class_size))
		{
			*reinterpret_cast<AnimFrame*>(frame) = blk;

			free_space -= AnimFrame::class_size;
			frame += AnimFrame::class_size;
			buffer_offset_ += AnimFrame::class_size;

			if (blk.recsize > 0)
			{
				archiver_.read_char_array(frame, blk.recsize);
			}

			free_space -= blk.recsize;
			frame += blk.recsize;
			buffer_offset_ += blk.recsize;
		}
		else
		{
			file_stream_.set_position(chunkstart);
			free_space = 0;
		}
	}

	return true;
}

bool Movie::get_frame()
{
	if (buffer_offset_ == 0)
	{
		if (has_more_pages_)
		{
			has_more_pages_ = load_buffer();
		}
		else
		{
			return false;
		}
	}

	buffer_ptr_ = next_ptr_;

	const auto& blk = *reinterpret_cast<const AnimFrame*>(buffer_ptr_);

	buffer_offset_ -= AnimFrame::class_size;
	buffer_offset_ -= blk.recsize;
	next_ptr_ = buffer_ptr_ + AnimFrame::class_size + blk.recsize;

	return true;
}

void Movie::handle_page(
	const Descriptor& descriptor)
{
	const auto& blk = *reinterpret_cast<const AnimFrame*>(buffer_ptr_);

	buffer_ptr_ += AnimFrame::class_size;

	auto frame = buffer_ptr_;

	IN_ReadControl(0, &control_info_);

	switch (blk.code)
	{
	case AN_SOUND:
	{
		// Sound Chunk
		//

		const auto sound_chunk = bstone::endian::to_little(*reinterpret_cast<const std::uint16_t*>(frame));
		sd_play_player_item_sound(sound_chunk);
		buffer_ptr_ += blk.recsize;
	}
	break;

	case AN_FADE_IN_FRAME:
		// Fade In Page
		//

		VL_FadeIn(0, 255, palette_, 30);
		is_ever_faded_ = true;
		screenfaded = false;
		break;

	case AN_FADE_OUT_FRAME:
		// Fade Out Page
		//

		VW_FadeOut();
		screenfaded = true;
		break;

	case AN_PAUSE: // Pause
	{
		const auto vbls = bstone::endian::to_little(*reinterpret_cast<const std::uint16_t*>(frame));

		IN_UserInput(vbls);

		buffer_ptr_ += blk.recsize;

		// BBi
		IN_ClearKeysDown();
		control_info_ = {};
		// BBi
	}
	break;

	// Graphics Chunk
	case AN_PAGE:
	{
		if (flag_ == Flag::fill)
		{
			// First page coming in. Fill screen with fill color...
			//

			// Set READ flag to skip the first frame on an anim repeat
			flag_ = Flag::none;

			JM_VGALinearFill(0, vga_ref_width * vga_ref_height, *frame);

			++frame;
		}

		show_frame(frame);

		VL_RefreshScreen();

		if (TimeCount < descriptor.tick_delay_)
		{
			const auto min_wait_time = 0;
			const auto max_wait_time = 2 * TickBase; // 2 seconds

			auto wait_time = descriptor.tick_delay_ - TimeCount;

			if (wait_time < min_wait_time)
			{
				wait_time = min_wait_time;
			}

			if (wait_time > max_wait_time)
			{
				wait_time = max_wait_time;
			}

			if (wait_time > 0)
			{
				wait_time *= 1000;
				wait_time /= TickBase;

				sys_sleep_for(wait_time);
			}
		}
		else
		{
			sys_sleep_for(1000 / TickBase);
		}

		TimeCount = 0;

		if (!screenfaded &&
			(control_info_.button0 || control_info_.button1 || LastScan != ScanCode::sc_none))
		{
			is_exit_ = true;

			if (is_ever_faded_)
			{
				// This needs to be a passed flag...

				VW_FadeOut();
				screenfaded = true;
			}
		}
		break;
	}

	case AN_END_OF_ANIM:
		is_exit_ = true;
		break;

	default:
		BSTONE_THROW_STATIC_SOURCE("Unrecognized anim code.");
	}
}

bool Movie::play(
	const MovieId movie_id,
	const std::uint8_t* const palette)
{
	const auto& descriptor = get_descriptor(movie_id);

	// Init our Movie Stuff...
	//

	initialize(descriptor, palette);

	// Start the anim process
	//

	ca_open_resource(descriptor.assets_resource_type, file_stream_);

	if (!file_stream_.is_open())
	{
		return false;
	}

	archiver_.open(file_stream_);

	while (repeat_count_ && !is_exit_)
	{
		while (!is_exit_)
		{
			if (!get_frame())
			{
				break;
			}

			handle_page(descriptor);
		}

		--repeat_count_;

		flag_ = Flag::skip;
	}

	uninitialize();

	return true;
}


bool movie_play(
	const MovieId movie_id,
	const std::uint8_t* const palette)
{
	static auto movie = Movie{};

	return movie.play(movie_id, palette);
}
