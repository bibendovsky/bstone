/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2015 Boris I. Bendovsky (bibendovsky@hotmail.com)

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


struct MovieStuff
{
	AssetsCRefString file_base_name_;

	std::int8_t repeat_count_;
	std::int8_t tick_delay_;
}; // MovieStuff


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
		if (bstone::Endian::is_little())
		{
			return;
		}

		bstone::Endian::little_i(opt);
		bstone::Endian::little_i(offset);
		bstone::Endian::little_i(length);
	}
}; // AnimChunk

static_assert(AnimChunk::class_size == sizeof(AnimChunk), "Class size mismatch.");


constexpr auto max_movies = 4;

using Movies = std::array<MovieStuff, max_movies>;


enum class MovieFlag
{
	none,
	fill,
	skip,
}; // MovieFlag


constexpr auto max_buffer_size = 65'536;


using MovieBuffer = std::vector<char>;

// Movie File variables
bstone::FileStream movie_stream_;

// movie_get_frame & movie_load_buffer variables
MovieBuffer movie_buffer_; // Ptr to Allocated Memory for Buffer
int movie_buffer_offset_; // Len of data loaded into movie_buffer_
char* movie_buffer_ptr_; // Ptr to next frame in movie_buffer_
char* movie_next_ptr_; // Ptr Ofs to next frame after BufferOfs

bool movie_has_more_pages_; // More Pages avail on disk?

MovieFlag movie_flag_;
bool movie_is_exit_;
bool movie_is_ever_faded_;
int movie_repeat_count_;
ControlInfo control_info_;
const std::uint8_t* movie_palette_;

bstone::ArchiverUPtr movie_archiver_;


//
// MOVIE Definations for external movies
//
// NOTE: This list is ordered according to mv_???? enum list.
//

const auto movies = Movies
{{
	{Assets::get_intro_fmv_base_name(), 1, 3}, // intro
	{Assets::get_episode_6_fmv_base_name(), 1, 3}, // final
	{Assets::get_episode_2_4_fmv_base_name(), 1, 3}, // final_2
	{Assets::get_episode_3_5_fmv_base_name(), 1, 3}, // final_3
}}; // movies


// Inits all the internal routines for the Movie Presenter
static void movie_setup(
	const MovieStuff& MovieStuff,
	const std::uint8_t* const palette)
{
	movie_repeat_count_ = MovieStuff.repeat_count_;
	movie_flag_ = MovieFlag::fill;
	movie_buffer_offset_ = 0;
	movie_has_more_pages_ = true;
	movie_is_exit_ = false;
	movie_is_ever_faded_ = false;

	movie_palette_ = palette;

	::JM_VGALinearFill(0, ::vga_ref_width * ::vga_ref_height, 0);

	::VL_FillPalette(0, 0, 0);

	// Find out how much memory we have to work with.
	movie_buffer_.resize(max_buffer_size);

	movie_archiver_ = bstone::ArchiverFactory::create();

	::IN_ClearKeysDown();
}

static void movie_shutdown()
{
	movie_archiver_ = nullptr;
	movie_buffer_.clear();
	movie_stream_.close();
}

// ---------------------------------------------------------------------------
//
// dest_offset = Correct offset value for memory location for Paged/Latch mem
//
// byte_offset = Offset for the image to be drawn - This address is NOT
//               a calculated Paged/Latch value but a byte offset in
//               conventional memory.
//
// source = Source image of graphic to be blasted to latch memory.
//          This pic is NOT 'munged'.
//
// length = length of the source image in bytes
// ---------------------------------------------------------------------------
static void movie_jm_draw_block(
	const int byte_offset,
	const char* const source,
	const int length)
{
	auto x = byte_offset % ::vga_ref_width;
	auto y = byte_offset / ::vga_ref_width;

	for (int i = 0; i < length; ++i)
	{
		::VL_Plot(x, y, static_cast<std::uint8_t>(source[i]));

		++x;

		if (x == ::vga_ref_width)
		{
			x = 0;
			++y;
		}
	}
}

// ---------------------------------------------------------------------------
// Shows an animation frame
//
// PARAMETERS: pointer to animpic
// ---------------------------------------------------------------------------
static void movie_show_frame(
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

		movie_jm_draw_block(ah.offset, inpic, ah.length);

		inpic += ah.length;
	}
}

// ---------------------------------------------------------------------------
// Loads the RAM Buffer full of graphics...
//
// RETURNS:  true - MORE Pages avail on disk..
//          false   - LAST Pages from disk..
//
// movie_buffer_offset_ = Length of data loaded into buffer
//
// ---------------------------------------------------------------------------
static bool movie_load_buffer()
{
	auto frame = movie_buffer_.data();
	auto free_space = max_buffer_size;

	movie_next_ptr_ = frame;
	movie_buffer_ptr_ = frame;

	AnimFrame blk;

	while (free_space)
	{
		const auto chunkstart = movie_stream_.get_position();

		blk.code = movie_archiver_->read_uint16();
		blk.block_num = movie_archiver_->read_int32();
		blk.recsize = movie_archiver_->read_int32();

		if (blk.code == AN_END_OF_ANIM)
		{
			return false;
		}

		if (free_space >= (blk.recsize + AnimFrame::class_size))
		{
			*reinterpret_cast<AnimFrame*>(frame) = blk;

			free_space -= AnimFrame::class_size;
			frame += AnimFrame::class_size;
			movie_buffer_offset_ += AnimFrame::class_size;

			if (blk.recsize > 0)
			{
				movie_archiver_->read_char_array(frame, blk.recsize);
			}

			free_space -= blk.recsize;
			frame += blk.recsize;
			movie_buffer_offset_ += blk.recsize;
		}
		else
		{
			movie_stream_.set_position(chunkstart);
			free_space = 0;
		}
	}

	return true;
}

// ---------------------------------------------------------------------------
// Returns pointer to next Block/Screen of animation
//
// PURPOSE: This function "Buffers" the movie presentation from allocated
//      ram.  It loads and buffers incomming frames of animation..
//
// RETURNS:  true - Ok
//           false - End Of File
// ---------------------------------------------------------------------------
static bool movie_get_frame()
{
	if (movie_buffer_offset_ == 0)
	{
		if (movie_has_more_pages_)
		{
			movie_has_more_pages_ = movie_load_buffer();
		}
		else
		{
			return false;
		}
	}

	movie_buffer_ptr_ = movie_next_ptr_;

	const auto& blk = *reinterpret_cast<const AnimFrame*>(movie_buffer_ptr_);

	movie_buffer_offset_ -= AnimFrame::class_size;
	movie_buffer_offset_ -= blk.recsize;
	movie_next_ptr_ = movie_buffer_ptr_ + AnimFrame::class_size + blk.recsize;

	return true;
}

// ---------------------------------------------------------------------------
// This handles the current page of data from the ram buffer...
//
// PURPOSE: Process current Page of anim.
//
// ---------------------------------------------------------------------------
static void movie_handle_page(
	const MovieStuff& MovieStuff)
{
	const auto& blk = *reinterpret_cast<const AnimFrame*>(movie_buffer_ptr_);

	movie_buffer_ptr_ += AnimFrame::class_size;

	auto frame = movie_buffer_ptr_;

	::IN_ReadControl(0, &control_info_);

	switch (blk.code)
	{
	case AN_SOUND:
	{
		// Sound Chunk
		//

		const auto sound_chunk = bstone::Endian::little(*reinterpret_cast<const std::uint16_t*>(frame));

		::sd_play_player_sound(sound_chunk, bstone::ActorChannel::item);

		movie_buffer_ptr_ += blk.recsize;
	}
	break;

	case AN_FADE_IN_FRAME:
		// Fade In Page
		//

		::VL_FadeIn(0, 255, movie_palette_, 30);
		movie_is_ever_faded_ = true;
		::screenfaded = false;
		break;

	case AN_FADE_OUT_FRAME:
		// Fade Out Page
		//

		VW_FadeOut();
		::screenfaded = true;
		break;

	case AN_PAUSE: // Pause
	{
		const auto vbls = bstone::Endian::little(*reinterpret_cast<const std::uint16_t*>(frame));

		::IN_UserInput(vbls);

		movie_buffer_ptr_ += blk.recsize;

		// BBi
		::IN_ClearKeysDown();
		control_info_ = {};
		// BBi
	}
	break;

	// Graphics Chunk
	case AN_PAGE:
	{
		if (movie_flag_ == MovieFlag::fill)
		{
			// First page comming in. Fill screen with fill color...
			//

			// Set READ flag to skip the first frame on an anim repeat
			movie_flag_ = MovieFlag::none;

			::JM_VGALinearFill(0, ::vga_ref_width * ::vga_ref_height, *frame);

			++frame;
		}

		movie_show_frame(frame);

		::VL_RefreshScreen();

		if (TimeCount < static_cast<std::uint32_t>(MovieStuff.tick_delay_))
		{
			const auto min_wait_time = 0;
			const auto max_wait_time = 2 * TickBase; // 2 seconds

			auto wait_time = MovieStuff.tick_delay_ - static_cast<int>(TimeCount);

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

				::sys_sleep_for(wait_time);
			}
		}
		else
		{
			::sys_sleep_for(1000 / TickBase);
		}

		::TimeCount = 0;

		if (!::screenfaded &&
			(control_info_.button0 || control_info_.button1 || ::LastScan != ScanCode::sc_none))
		{
			movie_is_exit_ = true;

			if (movie_is_ever_faded_)
			{
				// This needs to be a passed flag...

				VW_FadeOut();
				::screenfaded = true;
			}
		}
		break;
	}

	case AN_END_OF_ANIM:
		movie_is_exit_ = true;
		break;

	default:
		::Quit("Unrecognized anim code.");
		break;
	}
}

// ---------------------------------------------------------------------------
// movie_play() - Playes an Animation
//
// RETURNS: true  - Movie File was found and "played"
//      false - Movie file was NOT found!
// ---------------------------------------------------------------------------
bool movie_play(
	const MovieId movie_id,
	const std::uint8_t* const palette)
{
	const auto& MovieStuff = movies[movie_id];

	// Init our Movie Stuff...
	//

	movie_setup(MovieStuff, palette);

	// Start the anim process
	//

	::ca_open_resource(MovieStuff.file_base_name_, movie_stream_);

	if (!movie_stream_.is_open())
	{
		return false;
	}

	try
	{
		movie_archiver_->initialize(&movie_stream_);

		while (movie_repeat_count_ && !movie_is_exit_)
		{
			while (!movie_is_exit_)
			{
				if (!movie_get_frame())
				{
					break;
				}

				movie_handle_page(MovieStuff);
			}

			--movie_repeat_count_;

			movie_flag_ = MovieFlag::skip;
		}
	}
	catch (const bstone::ArchiverException& ex)
	{
		::Quit(ex.get_message());
	}

	movie_shutdown();

	return true;
}
