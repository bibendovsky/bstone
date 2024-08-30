/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


#include <cstring>

#include "audio.h"
#include "id_ca.h"
#include "id_heads.h"
#include "id_in.h"
#include "id_sd.h"
#include "id_us.h"
#include "id_vh.h"
#include "id_vl.h"
#include "3d_menu.h"
#include "gfxv.h"


void CA_CacheScreen(
	std::int16_t chunk);


// ==========================================================================
//
// LOCAL VARABLES
//
// ==========================================================================

void ClearSplitVWB()
{
	update = Update{};
	WindowX = 0;
	WindowY = 0;
	WindowW = 320;
	WindowH = 152;
}

bool Breifing(
	breifing_type BreifingType,
	std::uint16_t episode)
{
	HelpPresenter(
		nullptr,
		true,
		static_cast<std::uint16_t>(BRIEF_W1 + (episode * 2) + BreifingType - 1),
		false);

	return EscPressed;
}

void ShPrint(
	const char* text,
	std::int8_t shadow_color,
	bool single_char)
{
	std::uint16_t old_color = fontcolor, old_x = px, old_y = py;
	const char* string;
	char buf[2] = {0, 0};

	if (single_char)
	{
		string = buf;
		buf[0] = *(char*)text;
	}
	else
	{
		string = text;
	}

	fontcolor = shadow_color;
	py++;
	px++;
	USL_DrawString(string); // JTR - This marks blocks!

	fontcolor = static_cast<std::uint8_t>(old_color);
	py = old_y;
	px = old_x;
	USL_DrawString(string); // JTR - This marks blocks!
}

void PreloadUpdate(
	std::uint16_t current,
	std::uint16_t total)
{
	// BBi No progress bars
#if 0
	std::uint16_t w = WindowW - 10;

	if (current > total)
	{
		current = total;
	}

	w = ((std::int32_t)w * current) / total;
	if (w)
	{
		VWB_Bar(WindowX, WindowY, w - 1, 1, BORDER_TEXT_COLOR);
	}

	VW_UpdateScreen();
#else
	static_cast<void>(current);
	static_cast<void>(total);
#endif
}

char prep_msg[] = "^ST1^CEGet Ready, Blake!\r^XX";


void DisplayPrepingMsg(
	const char* text)
{
	vid_set_ui_mask_3d(false);

	// Cache-in font
	//
	fontnumber = 1;
	CA_CacheGrChunk(STARTFONT + 1);
	BMAmsg(text);
	UNCACHEGRCHUNK(STARTFONT + 1);

	// BBi No progress bars
#if 0
// Set thermometer boundaries
//
	WindowX = 36;
	WindowY = 188;
	WindowW = 260;
	WindowH = 32;

	// Init MAP and GFX thermometer areas
	//
	VWB_Bar(WindowX, WindowY - 7, WindowW - 10, 2, BORDER_LO_COLOR);
	VWB_Bar(WindowX, WindowY - 7, WindowW - 11, 1, BORDER_TEXT_COLOR - 15);
	VWB_Bar(WindowX, WindowY, WindowW - 10, 2, BORDER_LO_COLOR);
	VWB_Bar(WindowX, WindowY, WindowW - 11, 1, BORDER_TEXT_COLOR - 15);
#endif

	// Update screen and fade in
	//
	const auto old_vid_is_hud = vid_is_hud;

	vid_is_hud = false;

	VW_UpdateScreen();
	if (screenfaded)
	{
		VW_FadeIn();
	}

	vid_is_hud = old_vid_is_hud;
}

void PreloadGraphics()
{
	WindowY = 188;

	const auto old_vid_is_hud = vid_is_hud;

	if (playstate != ex_transported)
	{
		vid_is_hud = true;

		vid_set_ui_mask_3d(false);
	}
	else
	{
		VWB_Bar(
			0,
			ref_3d_view_top_y - ref_3d_margin,
			vga_ref_width,
			ref_3d_view_height + 2 * ref_3d_margin,
			0
		);
	}

	vid_is_hud = old_vid_is_hud;
}


static const std::int16_t SCORE_Y_SPACING = 7;

void DrawHighScores()
{
	int i;
	int w;
	int h;
	HighScore* s;

	ClearMScreen();
	CA_CacheScreen(BACKGROUND_SCREENPIC);
	DrawMenuTitle("HIGH SCORES");

	if (playstate != ex_title)
	{
		DrawInstructions(IT_HIGHSCORES);
	}

	fontnumber = 2;
	SETFONTCOLOR(ENABLED_TEXT_COLOR, TERM_BACK_COLOR);

	ShadowPrint("NAME", 86, 60);
	ShadowPrint("SCORE", 175, 60);
	ShadowPrint("MISSION", 247, 53);
	ShadowPrint("RATIO", 254, 60);

	for (i = 0, s = Scores.data(); i < MaxScores; i++, s++)
	{
		SETFONTCOLOR(HIGHLIGHT_TEXT_COLOR - 1, TERM_BACK_COLOR);
		//
		// name
		//
		if (*s->name)
		{
			ShadowPrint(s->name, 45, static_cast<std::int16_t>(68 + (SCORE_Y_SPACING * i)));
		}

		//
		// score
		//

		std::string buffer;

		if (s->score > 9999999)
		{
			SETFONTCOLOR(HIGHLIGHT_TEXT_COLOR + 1, TERM_BACK_COLOR);
		}

		buffer = std::to_string(s->score);
		USL_MeasureString(buffer.c_str(), &w, &h);
		ShadowPrint(buffer.c_str(), static_cast<std::int16_t>(205 - w), static_cast<std::int16_t>(68 + (SCORE_Y_SPACING * i))); // 235

		//
		// mission ratio
		//
		buffer = std::to_string(s->ratio);
		USL_MeasureString(buffer.c_str(), &w, &h);
		ShadowPrint(buffer.c_str(), static_cast<std::int16_t>(272 - w), static_cast<std::int16_t>(68 + (SCORE_Y_SPACING * i)));
	}

	VW_UpdateScreen();
}

void CheckHighScore(
	std::int32_t score,
	std::uint16_t other)
{
	std::uint16_t i, j;
	std::int16_t n;
	auto myscore = HighScore{};
	US_CursorStruct TermCursor = {'@', 0, HIGHLIGHT_TEXT_COLOR, 2};


	// Check for cheaters
#ifdef NDEBUG
	if (DebugOk)
	{
		sd_play_player_no_way_sound(NOWAYSND);
		return;
	}
#endif // NDEBUG

	strcpy(myscore.name, "");
	myscore.score = score;
	myscore.episode = gamestate.episode;
	myscore.completed = other;
	myscore.ratio = ShowStats(0, 0, ss_justcalc, &gamestuff.level[gamestate.mapon].stats);

	for (i = 0, n = -1; i < MaxScores; i++)
	{
		if ((myscore.score > Scores[i].score) || ((myscore.score == Scores[i].score)
			&& (myscore.completed > Scores[i].completed)))
		{
			for (j = MaxScores; --j > i; )
			{
				Scores[j] = Scores[j - 1];
			}
			Scores[i] = myscore;
			n = i;
			break;
		}
	}

	StartCPMusic(static_cast<std::int16_t>(ROSTER_MUS));

	DrawHighScores();

	VW_FadeIn();

	if (n != -1)
	{
		//
		// got a high score
		//

		DrawInstructions(IT_ENTER_HIGHSCORE);
		SETFONTCOLOR(HIGHLIGHT_TEXT_COLOR, TERM_BACK_COLOR);
		PrintY = 68 + (SCORE_Y_SPACING * n);
		PrintX = 45;
		use_custom_cursor = true;
		allcaps = true;
		US_CustomCursor = TermCursor;
		US_LineInput(PrintX, PrintY, Scores[n].name, nullptr, true, MaxHighName, 100);
	}
	else
	{
		IN_ClearKeysDown();
		IN_UserInput(500);
	}

	StopMusic();
	use_custom_cursor = false;
}

std::uint16_t Random(
	std::uint16_t Max)
{
	std::uint16_t returnval;

	if (Max != 0)
	{
		if (Max > 255)
		{
			returnval = static_cast<std::uint16_t>((US_RndT() << 8) + US_RndT());
		}
		else
		{
			returnval = static_cast<std::uint16_t>(US_RndT());
		}

		return returnval % Max;
	}
	else
	{
		return 0;
	}
}
