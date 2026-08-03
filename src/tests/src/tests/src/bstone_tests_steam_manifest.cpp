#include <string>
#include "bstone_steam_manifest.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// std::vector<std::string> SteamManifest::parse_library_paths(std::string_view)
// Current layout: a numbered key maps to an object carrying "path".
void test_zj8i5m1vqbkzu9ok()
{
	constexpr auto text =
		"\"libraryfolders\"\n"
		"{\n"
		"\t\"0\"\n"
		"\t{\n"
		"\t\t\"path\"\t\t\"/home/user/.local/share/Steam\"\n"
		"\t}\n"
		"\t\"1\"\n"
		"\t{\n"
		"\t\t\"path\"\t\t\"/mnt/games/SteamLibrary\"\n"
		"\t}\n"
		"}\n";
	const auto paths = bstone::SteamManifest::parse_library_paths(text);
	tester.check(
		paths.size() == 2 &&
		paths[0] == "/home/user/.local/share/Steam" &&
		paths[1] == "/mnt/games/SteamLibrary");
}

// std::vector<std::string> SteamManifest::parse_library_paths(std::string_view)
// Legacy layout: a numbered key maps straight to the path, the root key is
// spelled differently, and numbering starts at one.
void test_a4qz0m9ynct1lpbc()
{
	constexpr auto text =
		"\"LibraryFolders\"\n"
		"{\n"
		"\t\"TimeNextStatsReport\"\t\t\"1600000000\"\n"
		"\t\"ContentStatsID\"\t\t\"123\"\n"
		"\t\"1\"\t\t\"D:\\\\SteamLibrary\"\n"
		"\t\"2\"\t\t\"E:\\\\Games\"\n"
		"}\n";
	const auto paths = bstone::SteamManifest::parse_library_paths(text);
	tester.check(
		paths.size() == 2 &&
		paths[0] == "D:\\SteamLibrary" &&
		paths[1] == "E:\\Games");
}

// std::vector<std::string> SteamManifest::parse_library_paths(std::string_view)
// Book-keeping keys sitting beside the numbered ones are ignored.
void test_wq2ncvf5xd8g0nkh()
{
	constexpr auto text =
		"\"libraryfolders\"\n"
		"{\n"
		"\t\"contentstatsid\"\t\t\"987654321\"\n"
		"\t\"0\"\n"
		"\t{\n"
		"\t\t\"path\"\t\t\"/steam\"\n"
		"\t\t\"label\"\t\t\"\"\n"
		"\t\t\"apps\"\n"
		"\t\t{\n"
		"\t\t}\n"
		"\t}\n"
		"}\n";
	const auto paths = bstone::SteamManifest::parse_library_paths(text);
	tester.check(paths.size() == 1 && paths[0] == "/steam");
}

// std::vector<std::string> SteamManifest::parse_library_paths(std::string_view)
// An entry without a path contributes nothing.
void test_p6bkq0dtnf3zj7la()
{
	constexpr auto text =
		"\"libraryfolders\"\n"
		"{\n"
		"\t\"0\"\n"
		"\t{\n"
		"\t\t\"label\"\t\t\"\"\n"
		"\t}\n"
		"}\n";
	tester.check(bstone::SteamManifest::parse_library_paths(text).empty());
}

// std::vector<std::string> SteamManifest::parse_library_paths(std::string_view)
// Unrelated or malformed text yields nothing.
void test_g5wmv2xhqe0sbrz3()
{
	tester.check(
		bstone::SteamManifest::parse_library_paths("").empty() &&
		bstone::SteamManifest::parse_library_paths("\"AppState\" { \"appid\" \"1\" }").empty() &&
		bstone::SteamManifest::parse_library_paths("\"libraryfolders\" { \"0\" { ").empty());
}

// std::vector<std::string> SteamManifest::parse_library_paths(std::string_view)
// The entry count is bounded, so a file crafted to hold a huge number of them
// cannot hand the caller an unbounded amount of work during start-up.
void test_m4hgo7wct2zbrn5v()
{
	auto text = std::string{"\"libraryfolders\"\n{\n"};
	for (auto i = 0; i < 5000; ++i)
	{
		text += "\t\"" + std::to_string(i) + "\"\t\"/lib" + std::to_string(i) + "\"\n";
	}
	text += "}\n";
	const auto paths = bstone::SteamManifest::parse_library_paths(text);
	tester.check(!paths.empty() && paths.size() <= 64 && paths[0] == "/lib0");
}

// ==========================================================================

// bool SteamManifest::parse_install_dir(std::string_view, std::string&)
// A fully installed application.
void test_d0vrqk8pnf1yhcxo()
{
	constexpr auto text =
		"\"AppState\"\n"
		"{\n"
		"\t\"appid\"\t\t\"358190\"\n"
		"\t\"StateFlags\"\t\t\"4\"\n"
		"\t\"installdir\"\t\t\"Blake Stone Aliens of Gold\"\n"
		"}\n";
	auto install_dir = std::string{};
	tester.check(
		bstone::SteamManifest::parse_install_dir(text, install_dir) &&
		install_dir == "Blake Stone Aliens of Gold");
}

// bool SteamManifest::parse_install_dir(std::string_view, std::string&)
// Installed with an update pending, running or paused. The files are present, so
// these must not be rejected.
void test_t7yjs3q1murvb5ez()
{
	auto install_dir = std::string{};
	auto is_valid = true;
	for (const auto* const state_flags : {"6", "262", "518"})
	{
		const auto text =
			std::string{"\"AppState\" { \"StateFlags\" \""} + state_flags +
			"\" \"installdir\" \"Game\" }";
		is_valid = is_valid &&
			bstone::SteamManifest::parse_install_dir(text, install_dir) &&
			install_dir == "Game";
	}
	tester.check(is_valid);
}

// bool SteamManifest::parse_install_dir(std::string_view, std::string&)
// A queued download writes a manifest before any files exist.
void test_n8cxwl4gz0iqty2f()
{
	auto install_dir = std::string{};
	auto is_rejected = true;
	// 1026 = update required and started, 2 = update required, 0 = nothing.
	for (const auto* const state_flags : {"1026", "2", "0"})
	{
		const auto text =
			std::string{"\"AppState\" { \"StateFlags\" \""} + state_flags +
			"\" \"installdir\" \"Game\" }";
		is_rejected = is_rejected &&
			!bstone::SteamManifest::parse_install_dir(text, install_dir) &&
			install_dir.empty();
	}
	tester.check(is_rejected);
}

// bool SteamManifest::parse_install_dir(std::string_view, std::string&)
// Key lookup is case-insensitive; real manifests disagree between clients.
void test_r1ekbj9ofsw6h4mu()
{
	constexpr auto text = "\"appstate\" { \"stateflags\" \"4\" \"INSTALLDIR\" \"Game\" }";
	auto install_dir = std::string{};
	tester.check(bstone::SteamManifest::parse_install_dir(text, install_dir) && install_dir == "Game");
}

// bool SteamManifest::parse_install_dir(std::string_view, std::string&)
// An install directory that is not a bare name would point outside the library.
void test_k3pmzf7txv0dc8ln()
{
	auto install_dir = std::string{};
	auto is_rejected = true;
	for (const auto* const value : {"", "..", ".", "../../etc", "/absolute", "C:\\Windows", "sub/dir"})
	{
		const auto text =
			std::string{"\"AppState\" { \"StateFlags\" \"4\" \"installdir\" \""} + value + "\" }";
		is_rejected = is_rejected && !bstone::SteamManifest::parse_install_dir(text, install_dir);
	}
	tester.check(is_rejected);
}

// bool SteamManifest::parse_install_dir(std::string_view, std::string&)
// A missing or non-numeric state, and text that is not a manifest at all.
void test_v9lhq2u5wdbtoe1c()
{
	auto install_dir = std::string{};
	tester.check(
		!bstone::SteamManifest::parse_install_dir("\"AppState\" { \"installdir\" \"Game\" }", install_dir) &&
		!bstone::SteamManifest::parse_install_dir("\"AppState\" { \"StateFlags\" \"x4\" \"installdir\" \"G\" }", install_dir) &&
		!bstone::SteamManifest::parse_install_dir("\"libraryfolders\" { }", install_dir) &&
		!bstone::SteamManifest::parse_install_dir("", install_dir));
}

// bool SteamManifest::parse_install_dir(std::string_view, std::string&)
// The directory name keeps its case exactly; some are all lower case.
void test_f2sdynm8ck6bqwjr()
{
	constexpr auto text = "\"AppState\" { \"StateFlags\" \"4\" \"installdir\" \"dota 2 beta\" }";
	auto install_dir = std::string{};
	tester.check(bstone::SteamManifest::parse_install_dir(text, install_dir) && install_dir == "dota 2 beta");
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_library_paths();
		register_install_dir();
	}

private:
	void register_library_paths()
	{
		tester.register_test("SteamManifest#zj8i5m1vqbkzu9ok", test_zj8i5m1vqbkzu9ok);
		tester.register_test("SteamManifest#a4qz0m9ynct1lpbc", test_a4qz0m9ynct1lpbc);
		tester.register_test("SteamManifest#wq2ncvf5xd8g0nkh", test_wq2ncvf5xd8g0nkh);
		tester.register_test("SteamManifest#p6bkq0dtnf3zj7la", test_p6bkq0dtnf3zj7la);
		tester.register_test("SteamManifest#g5wmv2xhqe0sbrz3", test_g5wmv2xhqe0sbrz3);
		tester.register_test("SteamManifest#m4hgo7wct2zbrn5v", test_m4hgo7wct2zbrn5v);
	}

	void register_install_dir()
	{
		tester.register_test("SteamManifest#d0vrqk8pnf1yhcxo", test_d0vrqk8pnf1yhcxo);
		tester.register_test("SteamManifest#t7yjs3q1murvb5ez", test_t7yjs3q1murvb5ez);
		tester.register_test("SteamManifest#n8cxwl4gz0iqty2f", test_n8cxwl4gz0iqty2f);
		tester.register_test("SteamManifest#r1ekbj9ofsw6h4mu", test_r1ekbj9ofsw6h4mu);
		tester.register_test("SteamManifest#k3pmzf7txv0dc8ln", test_k3pmzf7txv0dc8ln);
		tester.register_test("SteamManifest#v9lhq2u5wdbtoe1c", test_v9lhq2u5wdbtoe1c);
		tester.register_test("SteamManifest#f2sdynm8ck6bqwjr", test_f2sdynm8ck6bqwjr);
	}
};

auto registrator = Registrator{};

} // namespace
