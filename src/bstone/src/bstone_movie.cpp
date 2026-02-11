/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "id_ca.h"
#include "id_heads.h"
#include "id_in.h"
#include "id_sd.h"
#include "id_vh.h"
#include "id_vl.h"
#include "jm_vl.h"
#include "bstone_movie.h"
#include "bstone_memory_binary_reader.h"
#include <algorithm>
#include <memory>

namespace bstone {

namespace {

/*
ANIM CODES - DOCS
-----------------
FI - Fade In the current frame (Last Frame grabbed)
FO - Fade Out the current frame (Last Frame grabbed)
FB - Fade In with rate (a numeral value should follow in the script)
     ** MUST be a divisor of 64
FE - Fade Out with rate (a numeral value should follow in the script)
     ** MUST be a divisor of 64
SD - Play sounds (a numeral value should follow in the script)
GR - Graphic Page (full screen)

PA - Pause/Delay 'xxxxxx' number of VBLs
*/

consteval int MV_CNVT_CODE(char c1, char c2)
{
	return static_cast<unsigned char>(c1) | (static_cast<unsigned char>(c2) << 8);
}

constexpr int AN_PAUSE = MV_CNVT_CODE('P', 'A');
constexpr int AN_SOUND = MV_CNVT_CODE('S', 'D');
constexpr int AN_MUSIC = MV_CNVT_CODE('M', 'U');
constexpr int AN_PAGE = MV_CNVT_CODE('G', 'R');
constexpr int AN_FADE_IN_FRAME = MV_CNVT_CODE('F', 'I');
constexpr int AN_FADE_OUT_FRAME = MV_CNVT_CODE('F', 'O');
constexpr int AN_FADE_IN = MV_CNVT_CODE('F', 'B');
constexpr int AN_FADE_OUT = MV_CNVT_CODE('F', 'E');
constexpr int AN_PALETTE = MV_CNVT_CODE('P', 'L');
constexpr int AN_PRELOAD_BEGIN = MV_CNVT_CODE('L', 'B');
constexpr int AN_PRELOAD_END = MV_CNVT_CODE('L', 'E');
constexpr int AN_END_OF_ANIM = MV_CNVT_CODE('X', 'X');

// =====================================

class Movie
{
public:
	//
	// Plays an animation.
	//
	// Returns:
	//    - True if movie file was found and "played".
	//    - False otherwise.
	//
	bool play(MovieId movie_id, const std::uint8_t* palette);

private:
	static constexpr int max_file_size = 400'000;

	struct DataDeleter
	{
		void operator()(void* pointer) const
		{
			::operator delete(pointer);
		}
	};

	using Data = std::unique_ptr<std::uint8_t[], DataDeleter>;

	struct Descriptor
	{
		AssetsResourceType assets_resource_type;
		int tick_delay;
	};

	static constexpr int max_movies = 4;

	enum class Flag
	{
		none,
		fill,
		skip,
	};

	Data data_{};
	MemoryBinaryReader binary_reader_{};
	int anim_frame_code_{};
	int anim_frame_block_num_{};
	int anim_frame_recsize_{};
	Flag flag_{};
	bool is_exit_{};
	bool is_ever_faded_{};
	ControlInfo control_info_{};
	const std::uint8_t* palette_{};
	long long page_last_time_ns_{};

	const Descriptor& get_descriptor(MovieId movie_id) const;

	/*
	Draws a block of image.
	Parameters:
	    - byte_offset - offset for the image to be drawn.
	    - source - source image of graphic to be blasted to latch memory.
	               This pic is NOT 'munged'.
	    - length = length of the source image in bytes
	*/
	void jm_draw_block(int byte_offset, const std::uint8_t* source, int length);

	/*
	Shows the animation frame.
	Parameters:
	    - inpic - pointer to animpic.
	*/
	void show_frame();

	/*
	Advanced to the next frame.
	Returns:
	    - True on success or false otherwise.
	*/
	bool get_frame();

	// This handles the current page of data from the ram buffer.
	void handle_page(const Descriptor& descriptor);
};

// -------------------------------------

const Movie::Descriptor& Movie::get_descriptor(MovieId movie_id) const
{
	constinit static const Descriptor descriptors[max_movies] =
	{
		{.assets_resource_type = AssetsResourceType::ianim, .tick_delay = 3}, // intro
		{.assets_resource_type = AssetsResourceType::eanim, .tick_delay = 3}, // final
		{.assets_resource_type = AssetsResourceType::sanim, .tick_delay = 3}, // final_2
		{.assets_resource_type = AssetsResourceType::ganim, .tick_delay = 3}, // final_3
	};
	return descriptors[static_cast<int>(movie_id)];
}

void Movie::jm_draw_block(int byte_offset, const std::uint8_t* source, int length)
{
	int x = byte_offset % vga_ref_width;
	int y = byte_offset / vga_ref_width;
	for (int i = 0; i < length; ++i)
	{
		VL_Plot(x, y, source[i]);
		++x;
		if (x == vga_ref_width)
		{
			x = 0;
			++y;
		}
	}
}

void Movie::show_frame()
{
	MemoryBinaryReader reader{binary_reader_.get_current_data(), anim_frame_recsize_};
	if (flag_ == Flag::fill)
	{
		// First page coming in. Fill screen with fill color...
		// Set READ flag to skip the first frame on an anim repeat
		flag_ = Flag::none;
		if (!reader.can_read_x8())
		{
			BSTONE_THROW_STATIC_SOURCE("No filler value.");
		}
		JM_VGALinearFill(0, vga_ref_width * vga_ref_height, reader.read_u8());
	}
	for (;;)
	{
		if (!reader.can_read_n(6))
		{
			break;
		}
		const int anim_chunk_opt = reader.read_u16_le();
		const int anim_chunk_offset = reader.read_u16_le();
		const int anim_chunk_length = reader.read_u16_le();
		if (anim_chunk_opt == 0)
		{
			break;
		}
		if (!reader.can_read_n(anim_chunk_length))
		{
			BSTONE_THROW_STATIC_SOURCE("Truncated draw data.");
		}
		jm_draw_block(anim_chunk_offset, static_cast<const std::uint8_t*>(reader.get_current_data()), anim_chunk_length);
		reader.skip(anim_chunk_length);
	}
	binary_reader_.skip(anim_frame_recsize_);
}

bool Movie::get_frame()
{
	if (!binary_reader_.can_read_n(12))
	{
		return false;
	}
	anim_frame_code_ = binary_reader_.read_u16_le();
	anim_frame_block_num_ = binary_reader_.read_s32_le();
	anim_frame_recsize_ = binary_reader_.read_s32_le();
	if (anim_frame_code_ == AN_END_OF_ANIM)
	{
		binary_reader_.set_position(binary_reader_.get_size());
		return false;
	}
	return true;
}

void Movie::handle_page(const Descriptor& descriptor)
{
	IN_ReadControl(0, &control_info_);
	switch (anim_frame_code_)
	{
		case AN_SOUND:
			// Sound Chunk
			if (!binary_reader_.can_read_x16())
			{
				BSTONE_THROW_STATIC_SOURCE("No sound index.");
			}
			sd_play_player_item_sound(binary_reader_.read_u16_le());
			binary_reader_.skip(anim_frame_recsize_ - 2);
			break;
		case AN_FADE_IN_FRAME:
			// Fade In Page
			VL_FadeIn(0, 255, palette_, 30);
			is_ever_faded_ = true;
			screenfaded = false;
			break;
		case AN_FADE_OUT_FRAME:
			// Fade Out Page
			VW_FadeOut();
			screenfaded = true;
			break;
		case AN_PAUSE:
			// Pause
			if (!binary_reader_.can_read_x16())
			{
				BSTONE_THROW_STATIC_SOURCE("No pause ticks.");
			}
			IN_UserInput(binary_reader_.read_u16_le());
			binary_reader_.skip(anim_frame_recsize_ - 2);
			// BBi
			IN_ClearKeysDown();
			control_info_ = {};
			// BBi
			break;
		case AN_PAGE:
			// Graphics Chunk
			show_frame();
			VL_RefreshScreen();
			{
				const long long one_second_ns = 1'000'000'000;
				const long long two_seconds_ns = 2 * one_second_ns;
				const long long delay_ns = (one_second_ns * descriptor.tick_delay) / TickBase;
				const long long diff_ns = sys_get_time_ns() - page_last_time_ns_;
				if (diff_ns < delay_ns)
				{
					const long long sleep_for_ns = std::clamp(delay_ns - diff_ns, 0LL, two_seconds_ns);
					sys_sleep_for_ns(sleep_for_ns);
				}
				else
				{
					sys_sleep_for(1000 / TickBase);
				}
			}
			page_last_time_ns_ = sys_get_time_ns();
			if (!screenfaded && (control_info_.button0 || control_info_.button1 || LastScan != ScanCode::sc_none))
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
		case AN_END_OF_ANIM:
			is_exit_ = true;
			break;
		default:
			BSTONE_THROW_STATIC_SOURCE("Unknown anim code.");
	}
}

bool Movie::play(MovieId movie_id, const std::uint8_t* palette)
{
	// Init our Movie Stuff...
	flag_ = Flag::fill;
	palette_ = palette;
	JM_VGALinearFill(0, vga_ref_width * vga_ref_height, 0);
	VL_FillPalette(0, 0, 0);
	IN_ClearKeysDown();
	page_last_time_ns_ = sys_get_time_ns();
	// Start the anim process
	FileStream file_stream{};
	const Descriptor& descriptor = get_descriptor(movie_id);
	ca_open_resource(descriptor.assets_resource_type, file_stream);
	const std::int64_t file_size = file_stream.get_size();
	if (file_size < 0 || file_size > max_file_size)
	{
		return false;
	}
	const int data_size = static_cast<int>(file_size);
	data_.reset(static_cast<std::uint8_t*>(::operator new(data_size)));
	if (file_stream.read(data_.get(), data_size) != data_size)
	{
		return false;
	}
	binary_reader_ = MemoryBinaryReader{data_.get(), data_size};
	while (!is_exit_)
	{
		if (!get_frame())
		{
			break;
		}
		handle_page(descriptor);
	}
	return true;
}

} // namespace

bool movie_play(MovieId movie_id, const std::uint8_t* palette, Logger& logger)
try
{
	Movie movie{};
	return movie.play(movie_id, palette);
}
catch (...)
{
	logger.log_current_exception();
	return false;
}

} // namespace bstone
