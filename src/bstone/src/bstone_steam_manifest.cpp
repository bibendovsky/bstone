/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Interpretation of Steam's bookkeeping files.

#include "bstone_steam_manifest.h"
#include "bstone_vdf.h"
#include <cstdint>

namespace bstone {

namespace {

// Set while Steam believes the application's files are on disk. It is the only
// bit whose meaning every version of Steam agrees on, and it stays set while an
// update is pending or running, which is still playable.
constexpr std::uint64_t app_state_fully_installed = 4;

// Parses an unsigned decimal, rejecting anything else.
bool parse_uint(std::string_view string, std::uint64_t& value) noexcept
{
	if (string.empty())
		return false;
	value = 0;
	for (const char ch : string)
	{
		if (ch < '0' || ch > '9')
			return false;
		if (value > (UINT64_MAX - 9) / 10)
			return false; // Overflow.
		value = (value * 10) + static_cast<std::uint64_t>(ch - '0');
	}
	return true;
}

} // namespace

std::vector<std::string> parse_steam_library_paths(std::string_view vdf_text)
{
	auto library_paths = std::vector<std::string>{};
	auto root = VdfNode{};
	if (!parse_vdf(vdf_text, root))
		return library_paths;
	const VdfNode* const folders = root.find_child("libraryfolders");
	if (folders == nullptr)
		return library_paths;
	for (const VdfNode& entry : folders->children)
	{
		// Book-keeping keys such as "contentstatsid" sit beside the numbered ones,
		// and the numbering is neither contiguous nor consistently based, so the
		// entries are selected by name rather than counted through.
		auto index = std::uint64_t{};
		if (!parse_uint(entry.name, index))
			continue;
		const auto path = entry.is_object() ?
			entry.find_value("path") :
			std::string_view{entry.value};
		if (!path.empty())
			library_paths.emplace_back(path);
	}
	return library_paths;
}

bool parse_steam_install_dir(std::string_view acf_text, std::string& install_dir)
{
	install_dir.clear();
	auto root = VdfNode{};
	if (!parse_vdf(acf_text, root))
		return false;
	const VdfNode* const app_state = root.find_child("AppState");
	if (app_state == nullptr)
		return false;
	// Steam writes the manifest as soon as a download is queued, so its presence
	// alone says nothing about whether the files exist.
	auto state_flags = std::uint64_t{};
	if (!parse_uint(app_state->find_value("StateFlags"), state_flags))
		return false;
	if ((state_flags & app_state_fully_installed) == 0)
		return false;
	const auto value = app_state->find_value("installdir");
	// The recorded directory is a bare name; a separator, a drive or a parent
	// reference would let a corrupt manifest escape the library.
	if (value.empty() || value == "." || value == "..")
		return false;
	if (value.find_first_of("/\\:") != std::string_view::npos)
		return false;
	install_dir.assign(value);
	return true;
}

} // namespace bstone
