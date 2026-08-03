/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Launcher panel (SDL)

#include "bstone_sys_launcher.h"

#include "bstone_ascii.h"
#include "bstone_image_decoder.h"
#include "bstone_launcher_icon.h"
#include "bstone_process.h"
#include "bstone_rgb8.h"

#include <cstdint>
#include <string>
#include <vector>

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_pixels.h"
#include "SDL3/SDL_surface.h"
#include "SDL3/SDL_video.h"

namespace bstone::sys {

namespace {

// The panel's own five-by-seven pixel font, so it can draw before any game
// files exist. Uppercase only; lowercase input is raised before lookup.
struct LauncherGlyph
{
	char ch;
	std::uint8_t rows[7];
};

constexpr LauncherGlyph launcher_glyphs[] =
{
	{'A', {0b01110000, 0b10001000, 0b10001000, 0b11111000, 0b10001000, 0b10001000, 0b10001000}},
	{'B', {0b11110000, 0b10001000, 0b11110000, 0b10001000, 0b10001000, 0b10001000, 0b11110000}},
	{'C', {0b01110000, 0b10001000, 0b10000000, 0b10000000, 0b10000000, 0b10001000, 0b01110000}},
	{'D', {0b11100000, 0b10010000, 0b10001000, 0b10001000, 0b10001000, 0b10010000, 0b11100000}},
	{'E', {0b11111000, 0b10000000, 0b11110000, 0b10000000, 0b10000000, 0b10000000, 0b11111000}},
	{'F', {0b11111000, 0b10000000, 0b11110000, 0b10000000, 0b10000000, 0b10000000, 0b10000000}},
	{'G', {0b01110000, 0b10001000, 0b10000000, 0b10111000, 0b10001000, 0b10001000, 0b01111000}},
	{'H', {0b10001000, 0b10001000, 0b11111000, 0b10001000, 0b10001000, 0b10001000, 0b10001000}},
	{'I', {0b01110000, 0b00100000, 0b00100000, 0b00100000, 0b00100000, 0b00100000, 0b01110000}},
	{'J', {0b00111000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b10010000, 0b01100000}},
	{'K', {0b10001000, 0b10010000, 0b10100000, 0b11000000, 0b10100000, 0b10010000, 0b10001000}},
	{'L', {0b10000000, 0b10000000, 0b10000000, 0b10000000, 0b10000000, 0b10000000, 0b11111000}},
	{'M', {0b10001000, 0b11011000, 0b10101000, 0b10101000, 0b10001000, 0b10001000, 0b10001000}},
	{'N', {0b10001000, 0b11001000, 0b10101000, 0b10011000, 0b10001000, 0b10001000, 0b10001000}},
	{'O', {0b01110000, 0b10001000, 0b10001000, 0b10001000, 0b10001000, 0b10001000, 0b01110000}},
	{'P', {0b11110000, 0b10001000, 0b10001000, 0b11110000, 0b10000000, 0b10000000, 0b10000000}},
	{'Q', {0b01110000, 0b10001000, 0b10001000, 0b10001000, 0b10101000, 0b10010000, 0b01101000}},
	{'R', {0b11110000, 0b10001000, 0b10001000, 0b11110000, 0b10100000, 0b10010000, 0b10001000}},
	{'S', {0b01111000, 0b10000000, 0b10000000, 0b01110000, 0b00001000, 0b00001000, 0b11110000}},
	{'T', {0b11111000, 0b00100000, 0b00100000, 0b00100000, 0b00100000, 0b00100000, 0b00100000}},
	{'U', {0b10001000, 0b10001000, 0b10001000, 0b10001000, 0b10001000, 0b10001000, 0b01110000}},
	{'V', {0b10001000, 0b10001000, 0b10001000, 0b10001000, 0b01010000, 0b01010000, 0b00100000}},
	{'W', {0b10001000, 0b10001000, 0b10101000, 0b10101000, 0b10101000, 0b11011000, 0b10001000}},
	{'X', {0b10001000, 0b01010000, 0b00100000, 0b00100000, 0b00100000, 0b01010000, 0b10001000}},
	{'Y', {0b10001000, 0b01010000, 0b00100000, 0b00100000, 0b00100000, 0b00100000, 0b00100000}},
	{'Z', {0b11111000, 0b00001000, 0b00010000, 0b00100000, 0b01000000, 0b10000000, 0b11111000}},
	{'0', {0b01110000, 0b10001000, 0b10011000, 0b10101000, 0b11001000, 0b10001000, 0b01110000}},
	{'1', {0b00100000, 0b01100000, 0b00100000, 0b00100000, 0b00100000, 0b00100000, 0b01110000}},
	{'2', {0b01110000, 0b10001000, 0b00001000, 0b00110000, 0b01000000, 0b10000000, 0b11111000}},
	{'3', {0b11110000, 0b00001000, 0b00001000, 0b01110000, 0b00001000, 0b00001000, 0b11110000}},
	{'4', {0b00010000, 0b00110000, 0b01010000, 0b10010000, 0b11111000, 0b00010000, 0b00010000}},
	{'5', {0b11111000, 0b10000000, 0b11110000, 0b00001000, 0b00001000, 0b10001000, 0b01110000}},
	{'6', {0b01110000, 0b10000000, 0b10000000, 0b11110000, 0b10001000, 0b10001000, 0b01110000}},
	{'7', {0b11111000, 0b00001000, 0b00010000, 0b00100000, 0b01000000, 0b01000000, 0b01000000}},
	{'8', {0b01110000, 0b10001000, 0b10001000, 0b01110000, 0b10001000, 0b10001000, 0b01110000}},
	{'9', {0b01110000, 0b10001000, 0b10001000, 0b01111000, 0b00001000, 0b00001000, 0b01110000}},
	{'.', {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b01100000, 0b01100000}},
	{',', {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b01100000, 0b01100000, 0b01000000}},
	{':', {0b00000000, 0b01100000, 0b01100000, 0b00000000, 0b01100000, 0b01100000, 0b00000000}},
	{'-', {0b00000000, 0b00000000, 0b00000000, 0b11111000, 0b00000000, 0b00000000, 0b00000000}},
	{'_', {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b11111000}},
	{'/', {0b00001000, 0b00010000, 0b00010000, 0b00100000, 0b01000000, 0b01000000, 0b10000000}},
	{'(', {0b00010000, 0b00100000, 0b01000000, 0b01000000, 0b01000000, 0b00100000, 0b00010000}},
	{')', {0b01000000, 0b00100000, 0b00010000, 0b00010000, 0b00010000, 0b00100000, 0b01000000}},
	{'\'', {0b00100000, 0b00100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000}},
	{'!', {0b00100000, 0b00100000, 0b00100000, 0b00100000, 0b00100000, 0b00000000, 0b00100000}},
	{'?', {0b01110000, 0b10001000, 0b00001000, 0b00110000, 0b00100000, 0b00000000, 0b00100000}},
	{'&', {0b01100000, 0b10010000, 0b10100000, 0b01000000, 0b10101000, 0b10010000, 0b01101000}},
	{'+', {0b00000000, 0b00100000, 0b00100000, 0b11111000, 0b00100000, 0b00100000, 0b00000000}},
};

struct LauncherColor
{
	std::uint8_t r;
	std::uint8_t g;
	std::uint8_t b;
};

constexpr LauncherColor color_background{22, 24, 29};
constexpr LauncherColor color_card{34, 38, 46};
constexpr LauncherColor color_card_focus{43, 49, 59};
constexpr LauncherColor color_accent{232, 163, 61};
constexpr LauncherColor color_title{232, 230, 227};
constexpr LauncherColor color_detail{138, 144, 153};
constexpr LauncherColor color_border{58, 65, 77};

constexpr int window_width = 820;
constexpr int window_height = 720;
constexpr int pad = 32;
constexpr float tile_height = 256.0F;
constexpr float tile_gap = 64.0F;
constexpr float tile_art_size = 190.0F;
constexpr float tile_pad = 18.0F;
constexpr float chip_width = 58.0F;
constexpr float chip_height = 50.0F;
constexpr float chip_gap = 7.0F;
constexpr int chips_per_row = 4;
// A tile is as wide as the row of chips under it, so the two line up.
constexpr float tile_width = chips_per_row * chip_width + (chips_per_row - 1) * chip_gap;
constexpr float chip_row_gap = 7.0F;
constexpr float chip_top_gap = 9.0F;
constexpr float caption_top_gap = 9.0F;
constexpr float caption_height = 14.0F;
constexpr float path_top_gap = 22.0F;
constexpr float scroll_step = 45.0F;
constexpr float button_height = 44.0F;
constexpr float list_top = 152.0F;
// The list ends above the buttons rather than behind them, so a long list is
// cut off at a place the user can see and scroll from.
constexpr float list_bottom = window_height - pad - button_height - 16.0F;
constexpr float scrollbar_width = 6.0F;

const std::uint8_t* find_glyph_rows(char ch)
{
	ch = ascii::to_upper(ch);

	for (const LauncherGlyph& glyph : launcher_glyphs)
	{
		if (glyph.ch == ch)
		{
			return glyph.rows;
		}
	}

	return nullptr;
}

void set_color(SDL_Renderer* renderer, LauncherColor color)
{
	SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
}

void fill_rect(SDL_Renderer* renderer, float x, float y, float w, float h)
{
	const SDL_FRect rect{x, y, w, h};
	SDL_RenderFillRect(renderer, &rect);
}

void draw_border(SDL_Renderer* renderer, const SDL_FRect& rect, LauncherColor color)
{
	set_color(renderer, color);
	fill_rect(renderer, rect.x, rect.y, rect.w, 1.0F);
	fill_rect(renderer, rect.x, rect.y + rect.h - 1.0F, rect.w, 1.0F);
	fill_rect(renderer, rect.x, rect.y, 1.0F, rect.h);
	fill_rect(renderer, rect.x + rect.w - 1.0F, rect.y, 1.0F, rect.h);
}

void draw_text(SDL_Renderer* renderer, float x, float y, float scale, LauncherColor color, const std::string& text)
{
	set_color(renderer, color);
	float pen_x = x;

	for (const char ch : text)
	{
		const std::uint8_t* const rows = find_glyph_rows(ch);

		if (rows != nullptr)
		{
			for (int row = 0; row < 7; ++row)
			{
				for (int bit = 0; bit < 5; ++bit)
				{
					if ((rows[row] & (0b10000000 >> bit)) != 0)
					{
						fill_rect(
							renderer,
							pen_x + static_cast<float>(bit) * scale,
							y + static_cast<float>(row) * scale,
							scale,
							scale);
					}
				}
			}
		}

		pen_x += 6.0F * scale;
	}
}

float measure_text(float scale, const std::string& text)
{
	return static_cast<float>(text.size()) * 6.0F * scale;
}

// Long paths say most at their two ends, so drop the middle to fit.
std::string elide_middle(const std::string& text, float scale, float max_width)
{
	if (measure_text(scale, text) <= max_width)
	{
		return text;
	}

	const auto max_chars = static_cast<std::size_t>(max_width / (6.0F * scale));

	if (max_chars <= 5)
	{
		return text.substr(0, max_chars);
	}

	const std::size_t keep = max_chars - 3;
	const std::size_t head = keep / 2;
	const std::size_t tail = keep - head;
	return text.substr(0, head) + "..." + text.substr(text.size() - tail);
}

// Splits into lines that fit, breaking on spaces.
std::vector<std::string> wrap_text(const std::string& text, float scale, float max_width)
{
	std::vector<std::string> lines{};
	std::string line{};
	std::string word{};

	const auto flush_word = [&]()
	{
		if (word.empty())
		{
			return;
		}

		const std::string candidate = line.empty() ? word : line + " " + word;

		if (measure_text(scale, candidate) <= max_width)
		{
			line = candidate;
		}
		else
		{
			if (!line.empty())
			{
				lines.emplace_back(line);
			}

			line = word;
		}

		word.clear();
	};

	for (const char ch : text)
	{
		if (ch == ' ')
		{
			flush_word();
		}
		else if (ch == '\n')
		{
			flush_word();
			lines.emplace_back(line);
			line.clear();
		}
		else
		{
			word += ch;
		}
	}

	flush_word();

	if (!line.empty())
	{
		lines.emplace_back(line);
	}

	return lines;
}

// A chip chooses which release of a game to play; a tile plays the one chosen.
// A game with a single release has no chips, so the tile is the whole control.
enum class LauncherWidgetKind
{
	tile,
	chip,
	button,
};

struct LauncherWidget
{
	SDL_FRect rect;
	SDL_Texture* art;
	int art_width;
	int art_height;
	LauncherWidgetKind kind;
	LauncherAction action;
	int game_index;
	int release_index;
	std::string title;
	std::string qualifier;
};

struct LauncherLayout
{
	std::vector<LauncherWidget> widgets;
	int focus_index;
	// Which release each game will play, one entry per game.
	std::vector<int> chosen_releases;
	// Where the games end, so the path line can sit under all of them.
	float content_bottom;
	// How far the games have been scrolled, and the most they can be. The
	// buttons below them stay put.
	float scroll;
	float scroll_max;
};

bool is_scrollable(const LauncherWidget& widget)
{
	return widget.kind != LauncherWidgetKind::button;
}

// A release with no copy is shown but cannot be picked, so it takes neither
// the focus nor a click.
bool is_available(const LauncherItem& item, int release_index)
{
	return item.releases[static_cast<std::size_t>(release_index)].item_index >= 0;
}

bool is_selectable(std::span<const LauncherItem> items, const LauncherWidget& widget)
{
	return widget.kind != LauncherWidgetKind::chip ||
		is_available(items[static_cast<std::size_t>(widget.game_index)], widget.release_index);
}

// The full releases take a row and the shareware ones another. A game with a
// single release has no chips at all.
int count_chip_rows(const LauncherItem& item)
{
	if (item.releases.size() <= 1)
	{
		return 0;
	}

	auto has_full = false;
	auto has_qualified = false;

	for (const LauncherRelease& release : item.releases)
	{
		if (release.qualifier.empty())
		{
			has_full = true;
		}
		else
		{
			has_qualified = true;
		}
	}

	return (has_full ? 1 : 0) + (has_qualified ? 1 : 0);
}

SDL_FRect screen_rect(const LauncherWidget& widget, float scroll)
{
	SDL_FRect rect = widget.rect;

	if (is_scrollable(widget))
	{
		rect.y -= scroll;
	}

	return rect;
}

void clamp_scroll(LauncherLayout& layout)
{
	if (layout.scroll > layout.scroll_max)
	{
		layout.scroll = layout.scroll_max;
	}

	if (layout.scroll < 0.0F)
	{
		layout.scroll = 0.0F;
	}
}

// Keeps the focused card within the visible part of the list, so moving the
// focus with the keyboard scrolls rather than walking off the edge.
void scroll_focus_into_view(LauncherLayout& layout)
{
	if (layout.focus_index < 0 || layout.focus_index >= static_cast<int>(layout.widgets.size()))
	{
		return;
	}

	const LauncherWidget& widget = layout.widgets[layout.focus_index];

	if (!is_scrollable(widget))
	{
		return;
	}

	if (widget.rect.y - layout.scroll < list_top)
	{
		layout.scroll = widget.rect.y - list_top;
	}
	else if (widget.rect.y + widget.rect.h - layout.scroll > list_bottom)
	{
		layout.scroll = widget.rect.y + widget.rect.h - list_bottom;
	}

	clamp_scroll(layout);
}

void build_layout(std::span<const LauncherItem> items, const char* shareware_url, LauncherLayout& layout)
{
	layout.widgets.clear();
	layout.chosen_releases.assign(items.size(), 0);

	// Start on the release preferred among those the user actually has.
	for (std::size_t i = 0; i < items.size(); ++i)
	{
		auto best = -1;

		for (std::size_t j = 0; j < items[i].releases.size(); ++j)
		{
			const LauncherRelease& release = items[i].releases[j];

			if (release.item_index < 0)
			{
				continue;
			}

			if (best < 0 || release.preference < items[i].releases[static_cast<std::size_t>(best)].preference)
			{
				best = static_cast<int>(j);
			}
		}

		layout.chosen_releases[i] = best < 0 ? 0 : best;
	}

	const auto game_count = static_cast<int>(items.size());
	const float row_width = game_count * tile_width + (game_count > 0 ? (game_count - 1) * tile_gap : 0.0F);
	float x = (window_width - row_width) / 2.0F;
	float content_bottom = list_top;

	for (int i = 0; i < game_count; ++i)
	{
		const LauncherItem& item = items[i];

		auto& tile = layout.widgets.emplace_back();
		tile.rect = SDL_FRect{x, list_top, tile_width, tile_height};
		tile.kind = LauncherWidgetKind::tile;
		tile.action = LauncherAction::play;
		tile.game_index = i;
		tile.release_index = -1;
		tile.title = item.title;

		// One release is no choice, so it is left to the tile.
		const int row_count = count_chip_rows(item);
		float row_y = list_top + tile_height + chip_top_gap;

		for (int row = 0; row < row_count; ++row)
		{
			const bool wants_qualified = (row_count == 2 && row == 1) ||
				(row_count == 1 && !item.releases.front().qualifier.empty());
			auto row_size = 0;

			for (const LauncherRelease& release : item.releases)
			{
				if (release.qualifier.empty() != wants_qualified)
				{
					++row_size;
				}
			}

			float chip_x = x + (tile_width - (row_size * chip_width + (row_size - 1) * chip_gap)) / 2.0F;

			for (int j = 0; j < static_cast<int>(item.releases.size()); ++j)
			{
				if (item.releases[static_cast<std::size_t>(j)].qualifier.empty() == wants_qualified)
				{
					continue;
				}

				auto& chip = layout.widgets.emplace_back();
				chip.rect = SDL_FRect{chip_x, row_y, chip_width, chip_height};
				chip.kind = LauncherWidgetKind::chip;
				chip.action = LauncherAction::play;
				chip.game_index = i;
				chip.release_index = j;
				chip.title = item.releases[static_cast<std::size_t>(j)].label;
				chip.qualifier = item.releases[static_cast<std::size_t>(j)].qualifier;
				chip_x += chip_width + chip_gap;
			}

			row_y += chip_height + chip_row_gap;
		}

		const float caption_y = row_count > 0 ?
			row_y - chip_row_gap + caption_top_gap :
			list_top + tile_height + chip_top_gap;
		content_bottom = std::max(content_bottom, caption_y + caption_height);

		x += tile_width + tile_gap;
	}

	layout.content_bottom = content_bottom;
	const float content_height = game_count > 0 ? content_bottom - list_top : 0.0F;
	const float viewport_height = list_bottom - list_top;
	layout.scroll_max = content_height > viewport_height ? content_height - viewport_height : 0.0F;
	clamp_scroll(layout);

	const float button_y = window_height - pad - button_height;

	{
		auto& widget = layout.widgets.emplace_back();
		widget.rect = SDL_FRect{pad, button_y, 260.0F, button_height};
		widget.kind = LauncherWidgetKind::button;
		widget.action = LauncherAction::add_source;
		widget.game_index = -1;
		widget.release_index = -1;
		widget.title = "Add game source";
	}

	if (shareware_url != nullptr)
	{
		auto& widget = layout.widgets.emplace_back();
		widget.rect = SDL_FRect{pad + 272.0F, button_y, 236.0F, button_height};
		widget.kind = LauncherWidgetKind::button;
		widget.action = LauncherAction::get_shareware;
		widget.game_index = -1;
		widget.release_index = -1;
		widget.title = "Get shareware";
	}

	{
		auto& widget = layout.widgets.emplace_back();
		widget.rect = SDL_FRect{window_width - pad - 120.0F, button_y, 120.0F, button_height};
		widget.kind = LauncherWidgetKind::button;
		widget.action = LauncherAction::quit;
		widget.game_index = -1;
		widget.release_index = -1;
		widget.title = "Quit";
	}
}

void draw_panel(
	SDL_Renderer* renderer,
	std::span<const LauncherItem> items,
	const char* empty_message,
	const char* add_source_note,
	const LauncherLayout& layout)
{
	set_color(renderer, color_background);
	SDL_RenderClear(renderer);

	static const std::string wordmark = "BSTONE";
	constexpr float wordmark_scale = 4.0F;
	draw_text(renderer, pad, pad, wordmark_scale, color_title, wordmark);
	set_color(renderer, color_accent);
	// The advance after the last glyph is spacing, not part of the word.
	const float wordmark_width = measure_text(wordmark_scale, wordmark) - wordmark_scale;
	fill_rect(renderer, pad, pad + 36.0F, wordmark_width, 3.0F);
	draw_text(
		renderer,
		pad,
		pad + 54.0F,
		2.0F,
		color_detail,
		items.empty() ? "No games found yet" : "Choose a game");

	if (items.empty())
	{
		float y = 142.0F;

		for (const std::string& line : wrap_text(empty_message, 2.0F, window_width - 2.0F * pad))
		{
			draw_text(renderer, pad, y, 2.0F, color_detail, line);
			y += 22.0F;
		}
	}

	// Cards are held inside the list so a partly scrolled one is cut off at the
	// edge of the list rather than drawn over the header or the buttons.
	const SDL_Rect list_clip{
		0,
		static_cast<int>(list_top),
		window_width,
		static_cast<int>(list_bottom - list_top)};

	for (int i = 0; i < static_cast<int>(layout.widgets.size()); ++i)
	{
		const LauncherWidget& widget = layout.widgets[i];
		const bool has_focus = (i == layout.focus_index);
		const SDL_FRect rect = screen_rect(widget, layout.scroll);

		if (widget.kind == LauncherWidgetKind::tile)
		{
			if (rect.y + rect.h <= list_top || rect.y >= list_bottom)
			{
				continue;
			}

			SDL_SetRenderClipRect(renderer, &list_clip);
			set_color(renderer, has_focus ? color_card_focus : color_card);
			fill_rect(renderer, rect.x, rect.y, rect.w, rect.h);
			draw_border(renderer, rect, has_focus ? color_accent : color_card);

			if (widget.art != nullptr)
			{
				// An icon is square but a store logo is wide, so fit the art
				// inside the square left for it and centre what is left over.
				float art_w = static_cast<float>(widget.art_width);
				float art_h = static_cast<float>(widget.art_height);
				const float scale = tile_art_size / (art_w > art_h ? art_w : art_h);
				art_w *= scale;
				art_h *= scale;
				const SDL_FRect art_rect{
					rect.x + (rect.w - art_w) / 2.0F,
					rect.y + tile_pad + (tile_art_size - art_h) / 2.0F,
					art_w,
					art_h};
				SDL_RenderTexture(renderer, widget.art, nullptr, &art_rect);
			}

			const float title_w = rect.w - 2.0F * tile_pad;
			const std::string title = elide_middle(widget.title, 2.0F, title_w);
			draw_text(
				renderer,
				rect.x + (rect.w - measure_text(2.0F, title)) / 2.0F,
				rect.y + tile_pad + tile_art_size + 16.0F,
				2.0F,
				color_title,
				title);

			const LauncherItem& item = items[static_cast<std::size_t>(widget.game_index)];
			const int chosen = layout.chosen_releases[static_cast<std::size_t>(widget.game_index)];

			if (!item.releases.empty())
			{
				const std::string& source = item.releases[static_cast<std::size_t>(chosen)].source;
				const int rows = count_chip_rows(item);
				const float caption_y = rect.y + rect.h + chip_top_gap +
					(rows > 0 ? rows * chip_height + (rows - 1) * chip_row_gap + caption_top_gap : 0.0F);
				const std::string caption = elide_middle(source, 1.0F, rect.w);
				draw_text(
					renderer,
					rect.x + (rect.w - measure_text(1.0F, caption)) / 2.0F,
					caption_y,
					1.0F,
					color_detail,
					caption);
			}

			SDL_SetRenderClipRect(renderer, nullptr);
		}
		else if (widget.kind == LauncherWidgetKind::chip)
		{
			if (rect.y + rect.h <= list_top || rect.y >= list_bottom)
			{
				continue;
			}

			const LauncherItem& chip_item = items[static_cast<std::size_t>(widget.game_index)];
			const bool is_owned = is_available(chip_item, widget.release_index);
			const bool is_chosen = is_owned &&
				layout.chosen_releases[static_cast<std::size_t>(widget.game_index)] == widget.release_index;

			SDL_SetRenderClipRect(renderer, &list_clip);
			set_color(renderer, has_focus || is_chosen ? color_card_focus : color_card);
			fill_rect(renderer, rect.x, rect.y, rect.w, rect.h);
			draw_border(renderer, rect, is_chosen || has_focus ? color_accent : color_border);

			const LauncherColor label_color = is_chosen ? color_title : (is_owned ? color_detail : color_border);
			const bool has_qualifier = !widget.qualifier.empty();
			const float label_y = rect.y + (has_qualifier ? 10.0F : 16.0F);
			draw_text(
				renderer,
				rect.x + (rect.w - measure_text(2.0F, widget.title)) / 2.0F,
				label_y,
				2.0F,
				label_color,
				widget.title);

			if (has_qualifier)
			{
				draw_text(
					renderer,
					rect.x + (rect.w - measure_text(1.0F, widget.qualifier)) / 2.0F,
					rect.y + 30.0F,
					1.0F,
					is_owned ? color_detail : color_border,
					widget.qualifier);
			}

			SDL_SetRenderClipRect(renderer, nullptr);
		}
		else
		{
			set_color(renderer, has_focus ? color_card_focus : color_background);
			fill_rect(renderer, rect.x, rect.y, rect.w, rect.h);
			draw_border(renderer, rect, has_focus ? color_accent : color_border);
			const float text_x = rect.x + (rect.w - measure_text(2.0F, widget.title)) / 2.0F;
			draw_text(renderer, text_x, rect.y + 15.0F, 2.0F, color_title, widget.title);
		}
	}

	// Where the release under the focus actually sits, spanning both games so
	// a long path has the width to be read.
	if (layout.focus_index >= 0 && layout.focus_index < static_cast<int>(layout.widgets.size()))
	{
		const LauncherWidget& focused = layout.widgets[static_cast<std::size_t>(layout.focus_index)];

		if (focused.game_index >= 0)
		{
			const LauncherItem& item = items[static_cast<std::size_t>(focused.game_index)];
			const int release_index = focused.kind == LauncherWidgetKind::chip ?
				focused.release_index :
				layout.chosen_releases[static_cast<std::size_t>(focused.game_index)];
			const LauncherRelease& release = item.releases[static_cast<std::size_t>(release_index)];

			if (release.item_index >= 0)
			{
				const float path_width = window_width - 2.0F * pad;
				const std::string path = elide_middle(release.path, 1.0F, path_width);
				draw_text(
					renderer,
					pad + (path_width - measure_text(1.0F, path)) / 2.0F,
					layout.content_bottom + path_top_gap,
					1.0F,
					color_detail,
					path);
			}
		}
	}

	if (layout.scroll_max > 0.0F)
	{
		const float track_height = list_bottom - list_top;
		const float track_x = window_width - pad - scrollbar_width;
		set_color(renderer, color_border);
		fill_rect(renderer, track_x, list_top, scrollbar_width, track_height);

		const float visible_fraction = track_height / (track_height + layout.scroll_max);
		const float thumb_height = track_height * visible_fraction;
		const float thumb_y = list_top + (track_height - thumb_height) * (layout.scroll / layout.scroll_max);
		set_color(renderer, color_accent);
		fill_rect(renderer, track_x, thumb_y, scrollbar_width, thumb_height);
	}

	if (add_source_note != nullptr && items.empty())
	{
		float y = window_height - pad - 104.0F;

		for (const std::string& line : wrap_text(add_source_note, 2.0F, window_width - 2.0F * pad))
		{
			draw_text(renderer, pad, y, 2.0F, color_detail, line);
			y += 22.0F;
		}
	}

	SDL_RenderPresent(renderer);
}

// A chip chooses a release and leaves the panel up; a tile plays whichever
// release its game is on. Returns whether the panel is finished with.
bool activate_widget(
	std::span<const LauncherItem> items,
	const char* shareware_url,
	LauncherLayout& layout,
	int index,
	LauncherResult& result)
{
	const LauncherWidget& widget = layout.widgets[static_cast<std::size_t>(index)];

	if (widget.kind == LauncherWidgetKind::chip)
	{
		if (is_selectable(items, widget))
		{
			layout.chosen_releases[static_cast<std::size_t>(widget.game_index)] = widget.release_index;
		}

		return false;
	}

	if (widget.kind == LauncherWidgetKind::tile)
	{
		const LauncherItem& item = items[static_cast<std::size_t>(widget.game_index)];

		if (item.releases.empty())
		{
			return false;
		}

		const int chosen = layout.chosen_releases[static_cast<std::size_t>(widget.game_index)];
		result = LauncherResult{LauncherAction::play, item.releases[static_cast<std::size_t>(chosen)].item_index};
		return true;
	}

	// The page is opened outside; the panel stays up for the user to come
	// back to with what they downloaded.
	if (widget.action == LauncherAction::get_shareware)
	{
		process::open_file_or_url(shareware_url);
		return false;
	}

	result = LauncherResult{widget.action, -1};
	return true;
}

void move_focus(std::span<const LauncherItem> items, LauncherLayout& layout, int step)
{
	const auto count = static_cast<int>(layout.widgets.size());

	for (int i = 0; i < count; ++i)
	{
		layout.focus_index = (layout.focus_index + step + count) % count;

		if (is_selectable(items, layout.widgets[static_cast<std::size_t>(layout.focus_index)]))
		{
			break;
		}
	}

	scroll_focus_into_view(layout);
}

int widget_at(std::span<const LauncherItem> items, const LauncherLayout& layout, float x, float y)
{
	for (int i = 0; i < static_cast<int>(layout.widgets.size()); ++i)
	{
		const LauncherWidget& widget = layout.widgets[i];

		// A card scrolled out of the list is not there to be clicked, even
		// though the place it would occupy is.
		if (is_scrollable(widget) && (y < list_top || y >= list_bottom))
		{
			continue;
		}

		if (!is_selectable(items, widget))
		{
			continue;
		}

		const SDL_FRect rect = screen_rect(widget, layout.scroll);

		if (x >= rect.x && x < rect.x + rect.w && y >= rect.y && y < rect.y + rect.h)
		{
			return i;
		}
	}

	return -1;
}

} // namespace

LauncherResult Launcher::run(
	std::span<const LauncherItem> items,
	const char* empty_message,
	const char* add_source_note,
	const char* shareware_url,
	LauncherAddSourceFunc add_source_func,
	void* user_data)
{
	auto result = LauncherResult{LauncherAction::quit, -1};

	SDL_Window* const window = SDL_CreateWindow("BStone", window_width, window_height, 0);

	if (window == nullptr)
	{
		return result;
	}

	// BStone's own icon, so the window is the launcher's rather than nameless.
	auto icon_pixels = Rgba8Buffer{};
	int icon_width = 0;
	int icon_height = 0;

	try
	{
		const ImageDecoderUPtr image_decoder = make_image_decoder(ImageDecoderType::png);
		const std::span<const unsigned char> icon_png = get_launcher_icon_png();
		image_decoder->decode(
			icon_png.data(),
			static_cast<int>(icon_png.size()),
			icon_width,
			icon_height,
			icon_pixels);

		SDL_Surface* const icon_surface = SDL_CreateSurfaceFrom(
			icon_width,
			icon_height,
			SDL_PIXELFORMAT_RGBA32,
			icon_pixels.data(),
			icon_width * 4);

		if (icon_surface != nullptr)
		{
			SDL_SetWindowIcon(window, icon_surface);
			SDL_DestroySurface(icon_surface);
		}
	}
	catch (...)
	{
	}

	SDL_Renderer* const renderer = SDL_CreateRenderer(window, nullptr);

	if (renderer == nullptr)
	{
		SDL_DestroyWindow(window);
		return result;
	}

	auto layout = LauncherLayout{};
	layout.focus_index = 0;
	build_layout(items, shareware_url, layout);

	for (LauncherWidget& widget : layout.widgets)
	{
		if (widget.kind != LauncherWidgetKind::tile)
		{
			continue;
		}

		const LauncherItem& item = items[static_cast<std::size_t>(widget.game_index)];

		if (item.art_pixels == nullptr || item.art_width <= 0 || item.art_height <= 0)
		{
			continue;
		}

		SDL_Surface* const surface = SDL_CreateSurfaceFrom(
			item.art_width,
			item.art_height,
			SDL_PIXELFORMAT_RGBA32,
			const_cast<void*>(item.art_pixels),
			item.art_width * 4);

		if (surface == nullptr)
		{
			continue;
		}

		widget.art = SDL_CreateTextureFromSurface(renderer, surface);
		SDL_DestroySurface(surface);

		if (widget.art != nullptr)
		{
			widget.art_width = item.art_width;
			widget.art_height = item.art_height;
			SDL_SetTextureScaleMode(widget.art, SDL_SCALEMODE_LINEAR);
		}
	}

	draw_panel(renderer, items, empty_message, add_source_note, layout);

	bool is_done = false;

	while (!is_done)
	{
		SDL_Event e;

		if (!SDL_WaitEvent(&e))
		{
			break;
		}

		switch (e.type)
		{
			case SDL_EVENT_QUIT:
				result = LauncherResult{LauncherAction::quit, -1};
				is_done = true;
				break;

			case SDL_EVENT_MOUSE_MOTION:
			{
				const int index = widget_at(items, layout, e.motion.x, e.motion.y);

				if (index >= 0)
				{
					layout.focus_index = index;
				}

				break;
			}

			case SDL_EVENT_MOUSE_WHEEL:
				layout.scroll -= e.wheel.y * scroll_step;
				clamp_scroll(layout);
				break;

			case SDL_EVENT_MOUSE_BUTTON_DOWN:
			{
				const int index = widget_at(items, layout, e.button.x, e.button.y);

				if (index >= 0)
				{
					if (layout.widgets[index].action == LauncherAction::add_source)
					{
						// Keep the panel up: the dialog belongs to this window.
						if (add_source_func != nullptr && add_source_func(user_data, window))
						{
							result = LauncherResult{LauncherAction::add_source, -1};
							is_done = true;
						}
					}
					else
					{
						is_done = activate_widget(items, shareware_url, layout, index, result);
					}
				}

				break;
			}

			case SDL_EVENT_KEY_DOWN:
				switch (e.key.key)
				{
					case SDLK_UP:
						move_focus(items, layout, -1);
						break;

					case SDLK_DOWN:
					case SDLK_TAB:
						move_focus(items, layout, 1);
						break;

					case SDLK_RETURN:
					case SDLK_KP_ENTER:
					{
						if (layout.widgets[layout.focus_index].action == LauncherAction::add_source)
						{
							if (add_source_func != nullptr && add_source_func(user_data, window))
							{
								result = LauncherResult{LauncherAction::add_source, -1};
								is_done = true;
							}
						}
						else
						{
							is_done = activate_widget(items, shareware_url, layout, layout.focus_index, result);
						}

						break;
					}

					case SDLK_ESCAPE:
						result = LauncherResult{LauncherAction::quit, -1};
						is_done = true;
						break;

					default:
						break;
				}

				break;

			default:
				break;
		}

		if (!is_done)
		{
			draw_panel(renderer, items, empty_message, add_source_note, layout);
		}
	}

	for (LauncherWidget& widget : layout.widgets)
	{
		SDL_DestroyTexture(widget.art);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	return result;
}

} // namespace bstone::sys
