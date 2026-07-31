/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// Warning!
//
// All functions in this source file are designated usable by the memory
// manager after program initialization.
//


#include <cmath>

#include <algorithm>
#include <iostream>

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

#include "bstone_exception_utils.h"
#include "bstone_globals.h"
#include "bstone_logger.h"
#include "bstone_version.h"
#include "bstone_content_path.h"
#include "bstone_fs_utils.h"
#include "bstone_sha1.h"
#include "bstone_sys_message_box.h"
#include "bstone_vfs.h"
#include "bstone_game_art.h"
#include "bstone_game_source.h"
#include "bstone_sys_folder_dialog.h"
#include "bstone_sys_launcher.h"

void content_remember_dir(const std::string& dir);


extern SpanStart spanstart;
extern BaseDist basedist;
extern PlaneYLookup planeylookup;
extern MirrorOfs mirrorofs;


void CA_CannotOpen(
	const std::string& string);

void CAL_GetGrChunkLength(
	std::int16_t chunk);

void CA_CacheScreen(
	std::int16_t chunk);

void IN_StartAck();
bool IN_CheckAck();
void OpenMapFile();
void CloseMapFile();
void ClearMemory();
void ShutdownId();
void InitRedShifts();

void ReadConfig();
void read_high_scores();

void initialize_sprites();
void initialize_states();
void initialize_audio_constants();
void initialize_songs();
void initialize_gfxv_contants();
void initialize_tp_shape_table();
void initialize_tp_animation_table();
void initialize_static_info_constants();
void initialize_weapon_constants();
void initialize_grenade_shape_constants();
void initialize_static_health_table();
void initialize_hit_point_table();
void initialize_boss_constants();
void initialize_messages();
void initialize_ca_constants();

void InitPlaytemp();


extern const std::uint8_t colormap[16896];
const std::uint8_t* lightsource;


const std::string& get_message_box_title()
{
	static const auto result = std::string{} + "BStone v" + bstone::get_version().string_short;

	return result;
}


namespace {

struct AssetBundle
{
	const bstone::VfsSearchPath* vfs_search_path;
	AssetsVersion assets_version;

	bool is_empty() const
	{
		return vfs_search_path == nullptr || assets_version == AssetsVersion::none;
	}
};

class AssetBundleMgr
{
public:
	AssetBundleMgr(bstone::Logger& logger, bstone::Vfs& vfs)
		:
		logger_{logger},
		vfs_{vfs}
	{
		logger_.log_information("Find the asset bundles.");
		initialize_versions();
		initialize_filenames();
		initialize_versions_hashes();
		probe();
	}

	AssetBundleMgr(const AssetBundleMgr& that) = delete;

	const AssetBundle& get_aog_sw_asset_bundle() const
	{
		return aog_sw_asset_bundle_;
	}

	const AssetBundle& get_aog_asset_bundle() const
	{
		return aog_asset_bundle_;
	}

	const AssetBundle& get_ps_asset_bundle() const
	{
		return ps_asset_bundle_;
	}

	const std::vector<AssetBundle>& get_found_asset_bundles() const
	{
		return found_asset_bundles_;
	}

	static const char* get_bundle_version_string(AssetsVersion bundle_version)
	{
		switch (bundle_version)
		{
			case AssetsVersion::aog_sw_v1_0: return "Blake Stone: Aliens of Gold v1.0 (shareware)";
			case AssetsVersion::aog_sw_v2_0: return "Blake Stone: Aliens of Gold v2.0 (shareware)";
			case AssetsVersion::aog_sw_v2_1: return "Blake Stone: Aliens of Gold v2.1 (shareware)";
			case AssetsVersion::aog_sw_v3_0: return "Blake Stone: Aliens of Gold v3.0 (shareware)";
			case AssetsVersion::aog_full_v1_0: return "Blake Stone: Aliens of Gold v1.0";
			case AssetsVersion::aog_full_v2_0: return "Blake Stone: Aliens of Gold v2.0";
			case AssetsVersion::aog_full_v2_1: return "Blake Stone: Aliens of Gold v2.1";
			case AssetsVersion::aog_full_v3_0: return "Blake Stone: Aliens of Gold v3.0";
			case AssetsVersion::ps: return "Blake Stone: Planet Strike";
			default: return "Unknown Blake Stone";
		}
	}

private:
	using Versions = std::vector<AssetsVersion>;
	using Filenames = std::vector<const char*>;
	using Hashes = std::vector<bstone::Sha1Digest>;
	using VersionsHashes = std::vector<Hashes>;
	using AssetBundles = std::vector<AssetBundle>;
	using Sha1DigestBuffer = std::vector<bstone::Sha1Digest>;

	bstone::Logger& logger_;
	bstone::Vfs& vfs_;
	Versions aog_sw_versions_{};
	Versions aog_versions_{};
	Versions ps_versions_{};
	Filenames aog_sw_filenames_{};
	Filenames aog_filenames_{};
	Filenames ps_filenames_{};
	VersionsHashes aog_sw_versions_hashes_{};
	VersionsHashes aog_versions_hashes_{};
	VersionsHashes ps_versions_hashes_{};
	AssetBundle aog_sw_asset_bundle_{};
	AssetBundle aog_asset_bundle_{};
	AssetBundle ps_asset_bundle_{};
	// Every copy found. Two stores can each hold the same game, at different
	// versions, and both are the user's to choose from.
	std::vector<AssetBundle> found_asset_bundles_{};

	void initialize_versions()
	{
		aog_sw_versions_ = {
			AssetsVersion::aog_sw_v1_0,
			AssetsVersion::aog_sw_v2_0,
			AssetsVersion::aog_sw_v2_1,
			AssetsVersion::aog_sw_v3_0};
		aog_versions_ = {
			AssetsVersion::aog_full_v1_0,
			AssetsVersion::aog_full_v2_0,
			AssetsVersion::aog_full_v2_1,
			AssetsVersion::aog_full_v3_0};
		ps_versions_ = {AssetsVersion::ps};
	}

	void initialize_filenames(const AssetsResources& assets_resources, Filenames& filenames)
	{
		filenames.clear();
		filenames.reserve(assets_resources.size());
		for (const AssetsResource& assets_resource : assets_resources)
			filenames.emplace_back(assets_resource.file_name);
	}

	void initialize_filenames()
	{
		initialize_filenames(Assets::get_aog_sw_v1_0_resources(), aog_sw_filenames_);
		initialize_filenames(Assets::get_aog_full_v1_0_resources(), aog_filenames_);
		initialize_filenames(Assets::get_ps_resources(), ps_filenames_);
	}

	void initialize_versions_hashes(std::span<const AssetsResources*> versions_assets_resources, VersionsHashes& versions_hashes)
	{
		versions_hashes.clear();
		versions_hashes.reserve(versions_assets_resources.size());
		for (const AssetsResources* assets_resources : versions_assets_resources)
		{
			Hashes& hashes = versions_hashes.emplace_back();
			hashes.reserve(assets_resources->size());
			for (const AssetsResource& assets_resource : *assets_resources)
				hashes.emplace_back(bstone::make_sha1_digest(assets_resource.hash_string));
		}
	}

	void initialize_aog_sw_version_hashes()
	{
		const AssetsResources* versions_assets_resources[] = {
			&Assets::get_aog_sw_v1_0_resources(),
			&Assets::get_aog_sw_v2_0_resources(),
			&Assets::get_aog_sw_v2_1_resources(),
			&Assets::get_aog_sw_v3_0_resources()};
		std::span<const AssetsResources*> versions_assets_resources_span{versions_assets_resources};
		initialize_versions_hashes(versions_assets_resources_span, aog_sw_versions_hashes_);
	}

	void initialize_aog_version_hashes()
	{
		const AssetsResources* versions_assets_resources[] = {
			&Assets::get_aog_full_v1_0_resources(),
			&Assets::get_aog_full_v2_0_resources(),
			&Assets::get_aog_full_v2_1_resources(),
			&Assets::get_aog_full_v3_0_resources()};
		std::span<const AssetsResources*> versions_assets_resources_span{versions_assets_resources};
		initialize_versions_hashes(versions_assets_resources_span, aog_versions_hashes_);
	}

	void initialize_ps_version_hashes()
	{
		const AssetsResources* versions_assets_resources[] = {&Assets::get_ps_resources()};
		std::span<const AssetsResources*> versions_assets_resources_span{versions_assets_resources};
		initialize_versions_hashes(versions_assets_resources_span, ps_versions_hashes_);
	}

	void initialize_versions_hashes()
	{
		initialize_aog_sw_version_hashes();
		initialize_aog_version_hashes();
		initialize_ps_version_hashes();
	}

	bool calculate_sha1_digest(bstone::VfsInputStream* vfs_stream, bstone::Sha1Digest& sha1_digest)
	{
		if (!vfs_stream->rewind())
			return false;
		const int file_size = vfs_stream->get_size();
		if (file_size < 0)
			return false;
		constexpr int buffer_size = 1024;
		unsigned char buffer[buffer_size];
		bstone::Sha1 sha1{};
		int file_offset = 0;
		for (;;)
		{
			const int read_size = vfs_stream->read(buffer, buffer_size);
			if (read_size < 0)
				return false;
			if (read_size == 0)
				break;
			sha1.process(buffer, read_size);
			file_offset += read_size;
		}
		if (file_offset != file_size)
			return false;
		sha1.finish();
		sha1_digest = sha1.get_digest();
		return true;
	}

	void probe(
		const bstone::VfsSearchPath& vfs_search_path,
		const Filenames& file_names,
		const Versions& bundle_versions,
		const VersionsHashes& bundle_sha1_digests,
		Sha1DigestBuffer& sha1_digest_buffer,
		AssetBundle& found_asset_bundle)
	{
		found_asset_bundle = AssetBundle{
			.vfs_search_path = nullptr,
			.assets_version = AssetsVersion::none};
		const int asset_count = static_cast<int>(file_names.size());
		for (int i_asset = 0; i_asset < asset_count; ++i_asset)
		{
			const char* file_name = file_names[i_asset];
			auto vfs_stream = vfs_.open_file(vfs_search_path, file_name);
			if (vfs_stream == nullptr)
			{
				if (i_asset > 0)
					logger_.log_error("Missing file. (filename={})", file_name);
				return;
			}
			bstone::Sha1Digest& sha1_digest = sha1_digest_buffer[i_asset];
			if (!calculate_sha1_digest(vfs_stream.get(), sha1_digest))
			{
				logger_.log_error("Failed to hash. (filename={})", file_name);
				return;
			}
		}
		const int version_count = static_cast<int>(bundle_versions.size());
		for (int i_version = 0; i_version < version_count; ++i_version)
		{
			bool is_found = true;
			for (int i_asset = 0; i_asset < asset_count; ++i_asset)
			{
				if (bundle_sha1_digests[i_version][i_asset] != sha1_digest_buffer[i_asset])
				{
					is_found = false;
					break;
				}
			}
			if (is_found)
			{
				found_asset_bundle = AssetBundle{
					.vfs_search_path = &vfs_search_path,
					.assets_version = bundle_versions[i_version]};
				return;
			}
		}
		logger_.log_error("Unknown asset bundle version.");
	}

	void probe_aog_sw(const bstone::VfsSearchPath& vfs_search_path, Sha1DigestBuffer& sha1_digest_buffer)
	{
		AssetBundle aog_sw_found_bundle;
		probe(
			vfs_search_path,
			aog_sw_filenames_,
			aog_sw_versions_,
			aog_sw_versions_hashes_,
			sha1_digest_buffer,
			aog_sw_found_bundle);
		if (aog_sw_found_bundle.vfs_search_path != nullptr && aog_sw_found_bundle.assets_version != AssetsVersion::none)
		{
			logger_.log_information("Found {}.", get_bundle_version_string(aog_sw_found_bundle.assets_version));
			found_asset_bundles_.emplace_back(aog_sw_found_bundle);
			if (aog_sw_found_bundle.assets_version > aog_sw_asset_bundle_.assets_version)
				aog_sw_asset_bundle_ = aog_sw_found_bundle;
			if (aog_sw_found_bundle.assets_version == AssetsVersion::aog_sw_v2_1)
				aog_sw_asset_bundle_ = aog_sw_found_bundle; // Always prefer v2.1.
		}
	}

	void probe_aog(const bstone::VfsSearchPath& vfs_search_path, Sha1DigestBuffer& sha1_digest_buffer)
	{
		AssetBundle aog_found_bundle;
		probe(
			vfs_search_path,
			aog_filenames_,
			aog_versions_,
			aog_versions_hashes_,
			sha1_digest_buffer,
			aog_found_bundle);
		if (aog_found_bundle.vfs_search_path != nullptr && aog_found_bundle.assets_version != AssetsVersion::none)
		{
			logger_.log_information("Found {}.", get_bundle_version_string(aog_found_bundle.assets_version));
			found_asset_bundles_.emplace_back(aog_found_bundle);
			if (aog_found_bundle.assets_version > aog_asset_bundle_.assets_version)
				aog_asset_bundle_ = aog_found_bundle;
			if (aog_found_bundle.assets_version == AssetsVersion::aog_full_v2_1)
				aog_asset_bundle_ = aog_found_bundle; // Always prefer v2.1.
		}
	}

	void probe_ps(const bstone::VfsSearchPath& vfs_search_path, Sha1DigestBuffer& sha1_digest_buffer)
	{
		AssetBundle ps_found_bundle;
		probe(
			vfs_search_path,
			ps_filenames_,
			ps_versions_,
			ps_versions_hashes_,
			sha1_digest_buffer,
			ps_found_bundle);
		if (ps_found_bundle.vfs_search_path != nullptr && ps_found_bundle.assets_version != AssetsVersion::none)
		{
			logger_.log_information("Found {}.", get_bundle_version_string(ps_found_bundle.assets_version));
			found_asset_bundles_.emplace_back(ps_found_bundle);
			if (ps_found_bundle.assets_version > ps_asset_bundle_.assets_version)
				ps_asset_bundle_ = ps_found_bundle;
		}
	}

	void probe(const bstone::VfsSearchPath& vfs_search_path, Sha1DigestBuffer& sha1_digest_buffer)
	{
		probe_aog_sw(vfs_search_path, sha1_digest_buffer);
		probe_aog(vfs_search_path, sha1_digest_buffer);
		probe_ps(vfs_search_path, sha1_digest_buffer);
	}

	void probe()
	{
		Sha1DigestBuffer sha1_digest_buffer{};
		sha1_digest_buffer.resize(std::max({aog_sw_filenames_.size(), aog_filenames_.size(), ps_filenames_.size()}));
		int vfs_search_path_count = vfs_.get_search_path_count();
		for (int i = 0; i < vfs_search_path_count; ++i)
		{
			const bstone::VfsSearchPath& vfs_search_path = vfs_.get_search_path(i);
			logger_.log_information("Search path \"{}\".", vfs_search_path.path);
			probe(vfs_search_path, sha1_digest_buffer);
		}
	}
};


// The virtual file system keeps the pointers it is given, so the strings behind
// them have to outlive every reopening.
std::vector<std::string> extra_search_paths{};

bool add_game_source(bstone::Logger& logger, bstone::Vfs& vfs, void* parent_window)
{
	const std::string picked_path = bstone::sys::FolderDialog::show(
		"Choose a folder holding Blake Stone",
		nullptr,
		parent_window);
	logger.log_information("Game source dialog returned \"{}\".", picked_path);

	if (picked_path.empty())
	{
		return false;
	}

	const bstone::GameSourcePaths found_paths = bstone::find_game_sources(picked_path);
	logger.log_information("Found {} game folder(s) under {}.", static_cast<int>(found_paths.size()), picked_path);

	if (found_paths.empty())
	{
		bstone::sys::MessageBox::show_simple(
			get_message_box_title().c_str(),
			"No game was found under that folder.",
			bstone::sys::MessageBoxType::warning);
		return false;
	}

	content_remember_dir(picked_path);

	// The file system points into the very list being grown here, so take a
	// copy of what it has before touching that list.
	std::vector<std::string> current_paths{};
	current_paths.reserve(static_cast<std::size_t>(vfs.get_search_path_count()));

	for (int i = 0; i < vfs.get_search_path_count(); ++i)
	{
		current_paths.emplace_back(vfs.get_search_path(i).path);
	}

	current_paths.insert(current_paths.end(), found_paths.cbegin(), found_paths.cend());
	extra_search_paths.clear();

	for (const std::string& path : current_paths)
	{
		if (std::find(extra_search_paths.cbegin(), extra_search_paths.cend(), path) == extra_search_paths.cend())
		{
			extra_search_paths.emplace_back(path);
		}
	}

	std::vector<const char*> search_path_ptrs{};
	search_path_ptrs.reserve(extra_search_paths.size());

	for (const std::string& search_path : extra_search_paths)
	{
		search_path_ptrs.emplace_back(search_path.c_str());
	}

	const bstone::VfsInitParam vfs_init_param{
		.logger = &logger,
		.search_paths = std::span{search_path_ptrs.data(), search_path_ptrs.size()}};

	return vfs.initialize(vfs_init_param);
}

bool find_contents_once()
{
	bstone::Logger& logger = *bstone::globals::logger;
	bstone::Vfs& vfs = *bstone::globals::vfs;
	AssetBundleMgr asset_bundle_mgr{logger, vfs};
	const bool force_aog_sw = g_args.has_option("aog_sw");
	const bool force_aog = g_args.has_option("aog");
	const bool force_ps = g_args.has_option("ps");
	if (force_aog_sw + force_aog + force_ps > 1)
		BSTONE_THROW_STATIC_SOURCE("Forced multiple products.");
	std::vector<const AssetBundle*> products_to_choose{};
	products_to_choose.reserve(3);
	const AssetBundle& aog_sw_asset_bundle = asset_bundle_mgr.get_aog_sw_asset_bundle();
	const AssetBundle& aog_asset_bundle = asset_bundle_mgr.get_aog_asset_bundle();
	const AssetBundle& ps_asset_bundle = asset_bundle_mgr.get_ps_asset_bundle();
	if (force_aog_sw || force_aog || force_ps)
	{
		if (force_aog_sw && !aog_sw_asset_bundle.is_empty())
			products_to_choose.emplace_back(&aog_sw_asset_bundle);
		if (force_aog && !aog_asset_bundle.is_empty())
			products_to_choose.emplace_back(&aog_asset_bundle);
		if (force_ps && !ps_asset_bundle.is_empty())
			products_to_choose.emplace_back(&ps_asset_bundle);
	}
	else
	{
		for (const AssetBundle& found_bundle : asset_bundle_mgr.get_found_asset_bundles())
		{
			products_to_choose.emplace_back(&found_bundle);
		}
	}
	const AssetBundle* choosen_bundle = nullptr;

	if (force_aog_sw || force_aog || force_ps)
	{
		if (products_to_choose.empty())
		{
			BSTONE_THROW_STATIC_SOURCE("Compatible product not found.");
		}

		choosen_bundle = products_to_choose.front();
		const char* const version_string = AssetBundleMgr::get_bundle_version_string(choosen_bundle->assets_version);
		logger.log_information("Chose {} at {}.", version_string, choosen_bundle->vfs_search_path->path);
	}
	else
	{
		std::vector<bstone::sys::LauncherItem> launcher_items{};
		launcher_items.reserve(products_to_choose.size());
		// The artwork belongs to the copy of the game it was read from, and
		// lives only as long as the launcher shows it.
		// The card art is drawn small; ask for it at twice that, so a dense
		// display has pixels to use and the rest is a gentle shrink.
		constexpr int launcher_art_size = 128;
		std::vector<bstone::GameArt> launcher_arts{};
		launcher_arts.reserve(products_to_choose.size());

		for (const AssetBundle* const product_to_choose : products_to_choose)
		{
			const std::string game_path = product_to_choose->vfs_search_path->path;
			const bstone::GameArt& art = launcher_arts.emplace_back(bstone::find_game_art(game_path, launcher_art_size));
			auto& launcher_item = launcher_items.emplace_back();
			launcher_item.title = AssetBundleMgr::get_bundle_version_string(product_to_choose->assets_version);
			launcher_item.detail =
				std::string{bstone::get_game_source_label(game_path)} + "  -  " +
				bstone::make_display_path(game_path);
			launcher_item.art_pixels = art.is_empty() ? nullptr : art.pixels.data();
			launcher_item.art_width = art.width;
			launcher_item.art_height = art.height;
		}

		struct AddSourceContext
		{
			bstone::Logger* logger;
			bstone::Vfs* vfs;
		};

		auto add_source_context = AddSourceContext{&logger, &vfs};
		const bstone::sys::LauncherAddSourceFunc add_source_func =
			[](void* user_data, void* parent_window) -> bool
			{
				auto& context = *static_cast<AddSourceContext*>(user_data);
				return add_game_source(*context.logger, *context.vfs, parent_window);
			};
		const bstone::sys::LauncherResult launcher_result = bstone::sys::Launcher::run(
			std::span{launcher_items.data(), launcher_items.size()},
			bstone::get_game_source_prompt(),
			bstone::get_game_source_dialog_note(),
			add_source_func,
			&add_source_context);

		switch (launcher_result.action)
		{
			case bstone::sys::LauncherAction::play:
				choosen_bundle = products_to_choose[launcher_result.item_index];
				logger.log_information(
					"User chose {}.",
					AssetBundleMgr::get_bundle_version_string(choosen_bundle->assets_version));
				break;

			case bstone::sys::LauncherAction::add_source:
				// The launcher already added it; probe again with the new paths.
				return false;

			default:
				logger.log_information("Cancelled by user.");
				Quit();
				return true;
		}
	}

	// Two copies of the same game hold the same file names, and the file
	// system answers from the last path that has one. Drop the copies that
	// were not chosen, so only the chosen one can answer for them.
	{
		std::vector<std::string> game_paths{};

		for (const AssetBundle& found_bundle : asset_bundle_mgr.get_found_asset_bundles())
		{
			game_paths.emplace_back(found_bundle.vfs_search_path->path);
		}

		const std::string chosen_path = choosen_bundle->vfs_search_path->path;
		std::vector<std::string> kept_paths{};
		kept_paths.reserve(static_cast<std::size_t>(vfs.get_search_path_count()));
		kept_paths.emplace_back(chosen_path);

		for (int i = 0; i < vfs.get_search_path_count(); ++i)
		{
			const std::string path = vfs.get_search_path(i).path;
			const bool is_another_copy = path != chosen_path &&
				std::find(game_paths.cbegin(), game_paths.cend(), path) != game_paths.cend();

			if (!is_another_copy && path != chosen_path)
			{
				kept_paths.emplace_back(path);
			}
		}

		extra_search_paths = std::move(kept_paths);
		std::vector<const char*> search_path_ptrs{};
		search_path_ptrs.reserve(extra_search_paths.size());

		for (const std::string& path : extra_search_paths)
		{
			search_path_ptrs.emplace_back(path.c_str());
		}

		const bstone::VfsInitParam vfs_init_param{
			.logger = &logger,
			.search_paths = std::span{search_path_ptrs.data(), search_path_ptrs.size()}};

		if (!vfs.initialize(vfs_init_param))
		{
			BSTONE_THROW_STATIC_SOURCE("Failed to reopen the virtual file system.");
		}
	}

	AssetsInfo& assets_info = get_assets_info();
	switch (choosen_bundle->assets_version)
	{
		case AssetsVersion::aog_sw_v1_0:
			assets_info.set_resources(Assets::get_aog_sw_v1_0_resources());
			break;
		case AssetsVersion::aog_sw_v2_0:
			assets_info.set_resources(Assets::get_aog_sw_v2_0_resources());
			break;
		case AssetsVersion::aog_sw_v2_1:
			assets_info.set_resources(Assets::get_aog_sw_v2_1_resources());
			break;
		case AssetsVersion::aog_sw_v3_0:
			assets_info.set_resources(Assets::get_aog_sw_v3_0_resources());
			break;
		case AssetsVersion::aog_full_v1_0:
			assets_info.set_resources(Assets::get_aog_full_v1_0_resources());
			break;
		case AssetsVersion::aog_full_v2_0:
			assets_info.set_resources(Assets::get_aog_full_v2_0_resources());
			break;
		case AssetsVersion::aog_full_v2_1:
			assets_info.set_resources(Assets::get_aog_full_v2_1_resources());
			break;
		case AssetsVersion::aog_full_v3_0:
			assets_info.set_resources(Assets::get_aog_full_v3_0_resources());
			break;
		case AssetsVersion::ps:
			assets_info.set_resources(Assets::get_ps_resources());
			break;
		default:
			BSTONE_THROW_STATIC_SOURCE("Unknown version of the asset bundle.");
	}
	assets_info.set_version(choosen_bundle->assets_version);
	return true;
}


void find_contents()
{
	// Adding a source reopens the file system and asks again, so keep going
	// until a game is chosen or the user leaves.
	while (!find_contents_once())
	{
	}
}

} // namespace


// ------------------ ID Software 'startup' functions ---------------------


/*
==================
=
= BuildTables
=
= Calculates:
=
= scale                 projection constant
= sintable/costable     overlapping fractional tables
=
==================
*/
void BuildTables()
{
	//
	// calculate fine tangents
	//

	for (auto i = 0; i < (FINEANGLES / 8); ++i)
	{
		const auto tang = std::tan((i + 0.5) / radtoint);

		finetangent[i] = tang;
		finetangent[(FINEANGLES / 4) - 1 - i] = 1.0 / tang;
	}

	//
	// costable overlays sintable with a quarter phase shift
	// ANGLES is assumed to be divisable by four
	//
	// The low word of the value is the fraction, the high bit is the sign bit,
	// bits 16-30 should be 0
	//

	auto angle = 0.0;
	auto anglestep = bstone::math::pi() / 2.0 / ANGLEQUAD;

	for (auto i = 0; i <= ANGLEQUAD; ++i)
	{
		const auto value = std::sin(angle);

		sintable[i] = value;
		sintable[i + ANGLES] = value;
		sintable[ANGLES / 2 - i] = value;

		sintable[ANGLES - i] = -value;
		sintable[ANGLES / 2 + i] = -value;

		angle += anglestep;
	}

	lightsource = colormap;
}

// Map tile values to scaled pics
void SetupWalls()
{
	//
	// Hey! Yea You! This is where you can VERY easly setup to use a
	// specific 'BANK' of wall graphics.... JTR
	//

	for (int i = 1; i < MAXWALLTILES; ++i)
	{
		horizwall[i] = static_cast<std::int16_t>((i - 1) * 2);
		vertwall[i] = horizwall[i] + 1;
	}

	WallHeight().swap(wallheight);
	wallheight.resize(vga_width);


	const int k_half_height = vga_height / 2;

	SpanStart().swap(spanstart);
	spanstart.resize(k_half_height);

	BaseDist().swap(basedist);
	basedist.resize(k_half_height);

	PlaneYLookup().swap(planeylookup);
	planeylookup.resize(k_half_height);

	MirrorOfs().swap(mirrorofs);
	mirrorofs.resize(k_half_height);
}

// --------------------- Other general functions ------------------------

void CheckForEpisodes()
{
	const auto& assets_info = get_assets_info();

	if (assets_info.is_aog_full())
	{
		menu_enable_all_episodes();
	}
}


extern const char* MainStrs[];
extern char bc_buffer[];


void PreDemo()
{
	if (gp_no_intro_outro() || gp_no_screens())
	{
		return;
	}

	vid_is_movie = true;

	VL_SetPaletteIntensity(vgapal, 0);

	const auto& assets_info = get_assets_info();

	if (assets_info.is_aog_full())
	{
		// ---------------------
		// Anti-piracy screen
		// ---------------------

		// Cache pic
		//
		CA_CacheScreen(PIRACYPIC);

		// Cache and set palette.  AND  Fade it in!
		//
		CA_CacheGrChunk(PIRACYPALETTE);
		VL_SetPalette(0, 256, grsegs[PIRACYPALETTE].data());
		VL_SetPaletteIntensity(grsegs[PIRACYPALETTE].data(), 0);
		VW_UpdateScreen();

		VL_FadeOut(0, 255, 0, 0, 25, 20);
		VL_FadeIn(0, 255, grsegs[PIRACYPALETTE].data(), 30);

		// Wait a little
		//
		IN_UserInput(TickBase * 20);

		// Free palette
		//
		UNCACHEGRCHUNK(PIRACYPALETTE);

		VL_FadeOut(0, 255, 0, 0, 25, 20);
		VW_FadeOut();

		// Cleanup screen for upcoming SetPalette call
		//
		VL_Bar(0, 0, 320, 200, 0);
	}

	// ---------------------
	// Apogee presents
	// ---------------------

	// Cache pic
	//
	CA_CacheScreen(APOGEEPIC);

	sd_start_music(APOGFNFM_MUS, false);

	// Cache and set palette.  AND  Fade it in!
	//
	CA_CacheGrChunk(APOGEEPALETTE);
	VL_SetPalette(0, 256, grsegs[APOGEEPALETTE].data());
	VL_SetPaletteIntensity(grsegs[APOGEEPALETTE].data(), 0);
	VW_UpdateScreen();
	if (assets_info.is_aog())
	{
		VL_FadeOut(0, 255, 0, 0, 0, 20);
	}
	else
	{
		VL_FadeOut(0, 255, 25, 29, 53, 20);
	}
	VL_FadeIn(0, 255, grsegs[APOGEEPALETTE].data(), 30);

	// Wait for end of fanfare
	//
	if (sd_is_music_enabled())
	{
		IN_StartAck();
		while (sd_is_music_playing() && (!IN_CheckAck()))
		{
		}
	}
	else
	{
		IN_UserInput(TickBase * 6);
	}

	IN_ClearKeysDown();

	sd_music_off();

	// Free palette and music.  AND  Restore palette
	//
	UNCACHEGRCHUNK(APOGEEPALETTE);

	if (assets_info.is_ps())
	{
		// Do A Blue Flash!
		VL_FadeOut(0, 255, 25, 29, 53, 20);
	}
	else
	{
		VL_FadeOut(0, 255, 0, 0, 0, 30);
	}

	// ---------------------
	// JAM logo intro
	// ---------------------

	// Load and start music
	//
	sd_start_music(TITLE_LOOP_MUSIC);

	IN_ClearKeysDown();

	// Show JAM logo
	//
	if (!DoMovie(bstone::MovieId::intro))
	{
		BSTONE_THROW_STATIC_SOURCE("JAM animation (IANIM.xxx) does not exist.");
	}

	// ---------------------
	// PC-13
	// ---------------------
	VL_Bar(0, 0, 320, 200, 0x14);
	CacheDrawPic(0, 64, PC13PIC);
	VW_UpdateScreen();
	VW_FadeIn();
	IN_UserInput(TickBase * 2);

	// Do A Red Flash!

	if (assets_info.is_aog())
	{
		VL_FadeOut(0, 255, 39, 0, 0, 20);
	}
	else
	{
		VL_FadeOut(0, 255, 0, 0, 0, 20);
	}

	VW_FadeOut();

	vid_is_movie = false;
}

void check_for_extract_options()
{
	{
		constexpr auto extract_all_option_name_sv = std::string_view{"extract_all"};

		if (g_args.has_option(extract_all_option_name_sv))
		{
			const auto dst_dir_sv = g_args.get_option_value(extract_all_option_name_sv);
			const auto dst_dir = std::string{dst_dir_sv.cbegin(), dst_dir_sv.cend()};
			ca_extract_all(dst_dir);
			Quit();
		}
	}

	{
		constexpr auto extract_vga_palette_option_name_sv = std::string_view{"extract_vga_palette"};

		if (g_args.has_option(extract_vga_palette_option_name_sv))
		{
			const auto dst_dir_sv = g_args.get_option_value(extract_vga_palette_option_name_sv);
			const auto dst_dir = std::string{dst_dir_sv.cbegin(), dst_dir_sv.cend()};
			ca_extract_vga_palette(dst_dir);
			Quit();
		}
	}

	{
		constexpr auto extract_walls_option_name_sv = std::string_view{"extract_walls"};

		if (g_args.has_option(extract_walls_option_name_sv))
		{
			const auto dst_dir_sv = g_args.get_option_value(extract_walls_option_name_sv);
			const auto dst_dir = std::string{dst_dir_sv.cbegin(), dst_dir_sv.cend()};
			ca_extract_walls(dst_dir);
			Quit();
		}
	}

	{
		constexpr auto extract_sprites_option_name_sv = std::string_view{"extract_sprites"};

		if (g_args.has_option(extract_sprites_option_name_sv))
		{
			const auto dst_dir_sv = g_args.get_option_value(extract_sprites_option_name_sv);
			const auto dst_dir = std::string{dst_dir_sv.cbegin(), dst_dir_sv.cend()};
			ca_extract_sprites(dst_dir);
			Quit();
		}
	}

	{
		constexpr auto extract_musics_option_name_sv = std::string_view{"extract_music"};

		if (g_args.has_option(extract_musics_option_name_sv))
		{
			const auto dst_dir_sv = g_args.get_option_value(extract_musics_option_name_sv);
			const auto dst_dir = std::string{dst_dir_sv.cbegin(), dst_dir_sv.cend()};
			ca_extract_music(dst_dir);
			Quit();
		}
	}

	{
		constexpr auto extract_sfx_option_name_sv = std::string_view{"extract_sfx"};

		if (g_args.has_option(extract_sfx_option_name_sv))
		{
			const auto extract_dir_sv = g_args.get_option_value(extract_sfx_option_name_sv);
			const auto extract_dir = std::string{extract_dir_sv.cbegin(), extract_dir_sv.cend()};
			ca_extract_sfx(extract_dir);
			Quit();
		}
	}

	{
		constexpr auto extract_texts_option_name_sv = std::string_view{"extract_texts"};

		if (g_args.has_option(extract_texts_option_name_sv))
		{
			const auto dst_dir_sv = g_args.get_option_value(extract_texts_option_name_sv);
			const auto dst_dir = std::string{dst_dir_sv.cbegin(), dst_dir_sv.cend()};
			ca_extract_texts(dst_dir);
			Quit();
		}
	}

	{
		constexpr auto extract_levels_option_name_sv = std::string_view{"extract_levels"};

		if (g_args.has_option(extract_levels_option_name_sv))
		{
			const auto dst_dir_sv = g_args.get_option_value(extract_levels_option_name_sv);
			const auto dst_dir = std::string{dst_dir_sv.cbegin(), dst_dir_sv.cend()};
			ca_extract_levels(dst_dir);
			Quit();
		}
	}
}

void InitGame()
{
	vid_is_movie = true;

	std::int16_t i;
	std::int16_t x;
	std::int16_t y;
	std::uint16_t* blockstart;

	CA_Startup();
	bstone::globals::vswap = bstone::make_vswap();

	check_for_extract_options();
	read_high_scores();
	InitPlaytemp();

	sd_startup();
	IN_Startup();
	VW_Startup();
	US_Startup();

	VL_SetPalette(0, 256, vgapal);

	//
	// build some tables
	//

	for (i = 0; i < MAPSIZE; i++)
	{
		nearmapylookup[i] = &tilemap[0][0] + MAPSIZE * i;
		farmapylookup[i] = i * 64;
	}

	for (i = 0; i < PORTTILESHIGH; i++)
	{
		uwidthtable[i] = UPDATEWIDE * i;
	}

	blockstart = &blockstarts[0];
	for (y = 0; y < UPDATEHIGH; y++)
	{
		for (x = 0; x < UPDATEWIDE; x++)
		{
			*blockstart++ = SCREENWIDTH * 16 * y + x * TILEWIDTH;
		}
	}

	updateptr = &update[0];

	bufferofs = 0;


	//
	// load in and lock down some basic chunks
	//

	LoadFonts();

	LoadLatchMem();
	BuildTables(); // trig tables
	SetupWalls();
	NewViewSize();

	//
	// initialize variables
	//

	InitRedShifts();

	vid_is_movie = false;

	in_grab_mouse(true);
}

std::uint16_t scan_atoi(
	const char* s)
{
	while (*s && (!isdigit(*s)))
	{ // First scans for a digit...
		s++;
	}

	return static_cast<std::uint16_t>(atoi(s)); // Then converts to integer...
}


extern const char* MainStrs[];
extern std::int16_t starting_episode, starting_level, starting_difficulty;


static void output_version()
{
	const auto version_string = bstone::get_version().string;
	const auto message = std::string{} + "BStone v" + version_string + '.';

	// Standard output.
	//
	std::cout << message << '\n';

	// Message box.
	//
	try
	{
		bstone::sys::MessageBox::show_simple(
			get_message_box_title().c_str(),
			message.c_str(),
			bstone::sys::MessageBoxType::information);
	}
	catch (...)
	{
		const auto error_message = bstone::get_nested_message();
		bstone::globals::logger->log_error(error_message.c_str());
	}
}

namespace {

void deserialize_cvars_from_cli(const bstone::Cl& args, bstone::CVarMgr& cvar_mgr)
{
	for (const auto& option : args.get_options())
	{
		const auto cvar = cvar_mgr.find(option.name);

		if (cvar == nullptr)
		{
			continue;
		}

		if (option.args.size() != 1)
		{
			bstone::globals::logger->log_warning("Expected one argument for option {}.", option.name);
			continue;
		}

		cvar->set_string(option.args.front());
	}
}

} // namespace

void freed_main()
{
	if (g_args.has_option("version"))
	{
		output_version();
		Quit();
	}

	vid_initialize_cvars(*bstone::globals::cvar_mgr);
	sd_initialize_cvars(*bstone::globals::cvar_mgr);
	gp_initialize_cvars(*bstone::globals::cvar_mgr);
	am_initialize_cvars(*bstone::globals::cvar_mgr);
	in_initialize_cvars(*bstone::globals::cvar_mgr);
	in_initialize_ccmds(*bstone::globals::ccmd_mgr);

	// Setup for APOGEECD thingie.
	//
	InitDestPath();
	find_contents();

	bstone::globals::logger->log_information();
	bstone::globals::logger->log_information("Profile path: {}", get_profile_dir());
	bstone::globals::logger->log_information("Screenshot path: {}", get_screenshot_dir());

	// BBi
	{
		if (g_args.has_option("calculate_hashes"))
		{
			ca_calculate_hashes();
			Quit();
		}
	}

	// Make sure there's room to play the game
	//
	CheckDiskSpace(DISK_SPACE_NEEDED, CANT_PLAY_TXT, cds_dos_print);

	// Which version is this? (SHAREWARE? 1-3? 1-6?)
	//
	CheckForEpisodes();

	// BBi
	ReadConfig();
	deserialize_cvars_from_cli(g_args, *bstone::globals::cvar_mgr);
	sd_handle_command_line(g_args);

	initialize_sprites();
	initialize_gfxv_contants();
	initialize_states();
	initialize_tp_shape_table();
	initialize_tp_animation_table();
	initialize_audio_constants();
	initialize_songs();
	initialize_static_info_constants();
	initialize_weapon_constants();
	initialize_grenade_shape_constants();
	initialize_static_health_table();
	initialize_boss_constants();
	initialize_hit_point_table();
	initialize_messages();
	initialize_ca_constants();

	gamestuff.initialize();
	old_gamestuff = gamestuff;

	gamestate.initialize();
	old_gamestate = gamestate;

	if (g_args.has_option("cheats"))
	{
		DebugOk = true;
	}

	InitGame();

	PreDemo();
}
