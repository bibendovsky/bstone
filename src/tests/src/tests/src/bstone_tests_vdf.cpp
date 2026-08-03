#include <string>
#include "bstone_vdf.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// bool parse_vdf(std::string_view, VdfNode&)
// Empty text.
void test_c6924ee2i0oiegx3()
{
	auto root = bstone::VdfNode{};
	const auto is_parsed = bstone::parse_vdf("", root);
	tester.check(is_parsed && root.children.empty());
}

// bool parse_vdf(std::string_view, VdfNode&)
// A single key/value pair.
void test_38rta6pubumw927k()
{
	auto root = bstone::VdfNode{};
	const auto is_parsed = bstone::parse_vdf("\"key\" \"value\"", root);
	tester.check(
		is_parsed &&
		root.children.size() == 1 &&
		root.children[0].name == "key" &&
		root.children[0].value == "value" &&
		!root.children[0].is_object());
}

// bool parse_vdf(std::string_view, VdfNode&)
// A nested object.
void test_mtdqyztui42hbkva()
{
	constexpr auto text =
		"\"outer\"\n"
		"{\n"
		"\t\"inner\"\t\"1\"\n"
		"}\n";
	auto root = bstone::VdfNode{};
	const auto is_parsed = bstone::parse_vdf(text, root);
	const auto* const outer = root.find_child("outer");
	tester.check(
		is_parsed &&
		outer != nullptr &&
		outer->is_object() &&
		outer->find_value("inner") == "1");
}

// bool parse_vdf(std::string_view, VdfNode&)
// Current libraryfolders.vdf: numeric keys map to objects carrying "path".
void test_10iqosiccebjqz79()
{
	constexpr auto text =
		"\"libraryfolders\"\n"
		"{\n"
		"\t\"0\"\n"
		"\t{\n"
		"\t\t\"path\"\t\t\"/home/user/.local/share/Steam\"\n"
		"\t\t\"label\"\t\t\"\"\n"
		"\t\t\"apps\"\n"
		"\t\t{\n"
		"\t\t\t\"358190\"\t\t\"123456\"\n"
		"\t\t}\n"
		"\t}\n"
		"\t\"1\"\n"
		"\t{\n"
		"\t\t\"path\"\t\t\"/mnt/games/SteamLibrary\"\n"
		"\t}\n"
		"}\n";
	auto root = bstone::VdfNode{};
	const auto is_parsed = bstone::parse_vdf(text, root);
	const auto* const folders = root.find_child("libraryfolders");
	const auto* const first = folders != nullptr ? folders->find_child("0") : nullptr;
	const auto* const second = folders != nullptr ? folders->find_child("1") : nullptr;
	const auto* const apps = first != nullptr ? first->find_child("apps") : nullptr;
	tester.check(
		is_parsed &&
		folders != nullptr &&
		folders->children.size() == 2 &&
		first != nullptr &&
		first->find_value("path") == "/home/user/.local/share/Steam" &&
		first->find_value("label").empty() &&
		second != nullptr &&
		second->find_value("path") == "/mnt/games/SteamLibrary" &&
		apps != nullptr &&
		apps->find_value("358190") == "123456");
}

// bool parse_vdf(std::string_view, VdfNode&)
// Legacy libraryfolders.vdf: numeric keys map straight to a path string, and the
// root key was capitalised differently.
void test_z9aotytq2nxntmts()
{
	constexpr auto text =
		"\"LibraryFolders\"\n"
		"{\n"
		"\t\"TimeNextStatsReport\"\t\t\"1600000000\"\n"
		"\t\"1\"\t\t\"D:\\\\SteamLibrary\"\n"
		"}\n";
	auto root = bstone::VdfNode{};
	const auto is_parsed = bstone::parse_vdf(text, root);
	// Lookup is case-insensitive, so the same call finds either spelling.
	const auto* const folders = root.find_child("libraryfolders");
	tester.check(
		is_parsed &&
		folders != nullptr &&
		folders->find_value("1") == "D:\\SteamLibrary");
}

// bool parse_vdf(std::string_view, VdfNode&)
// appmanifest_*.acf.
void test_lo89hdqsgqjdxx40()
{
	constexpr auto text =
		"\"AppState\"\n"
		"{\n"
		"\t\"appid\"\t\t\"358190\"\n"
		"\t\"name\"\t\t\"Blake Stone: Aliens of Gold\"\n"
		"\t\"StateFlags\"\t\t\"4\"\n"
		"\t\"installdir\"\t\t\"Blake Stone - Aliens of Gold\"\n"
		"\t\"InstalledDepots\"\n"
		"\t{\n"
		"\t\t\"358191\"\n"
		"\t\t{\n"
		"\t\t\t\"manifest\"\t\t\"123\"\n"
		"\t\t}\n"
		"\t}\n"
		"}\n";
	auto root = bstone::VdfNode{};
	const auto is_parsed = bstone::parse_vdf(text, root);
	const auto* const app_state = root.find_child("AppState");
	tester.check(
		is_parsed &&
		app_state != nullptr &&
		app_state->find_value("appid") == "358190" &&
		app_state->find_value("StateFlags") == "4" &&
		app_state->find_value("installdir") == "Blake Stone - Aliens of Gold");
}

// bool parse_vdf(std::string_view, VdfNode&)
// Escapes: Steam writes Windows paths with doubled separators.
void test_u8vx20e1ptkzq6px()
{
	auto root = bstone::VdfNode{};
	const auto is_parsed = bstone::parse_vdf("\"path\" \"C:\\\\Program Files (x86)\\\\Steam\"", root);
	tester.check(is_parsed && root.find_value("path") == "C:\\Program Files (x86)\\Steam");
}

// bool parse_vdf(std::string_view, VdfNode&)
// Escapes: quote, newline and tab.
void test_xga7pncvvw9taohu()
{
	auto root = bstone::VdfNode{};
	const auto is_parsed = bstone::parse_vdf("\"k\" \"a\\\"b\\nc\\td\"", root);
	tester.check(is_parsed && root.find_value("k") == "a\"b\nc\td");
}

// bool parse_vdf(std::string_view, VdfNode&)
// An unknown escape keeps its literal character.
void test_oromby1njuckga1u()
{
	auto root = bstone::VdfNode{};
	const auto is_parsed = bstone::parse_vdf("\"k\" \"a\\qb\"", root);
	tester.check(is_parsed && root.find_value("k") == "aqb");
}

// bool parse_vdf(std::string_view, VdfNode&)
// Line comments are ignored.
void test_maicoqzy0n4k78oc()
{
	constexpr auto text =
		"// leading comment\n"
		"\"outer\"\n"
		"{\n"
		"\t// inner comment\n"
		"\t\"a\"\t\"1\" // trailing comment\n"
		"}\n";
	auto root = bstone::VdfNode{};
	const auto is_parsed = bstone::parse_vdf(text, root);
	const auto* const outer = root.find_child("outer");
	tester.check(is_parsed && outer != nullptr && outer->find_value("a") == "1");
}

// bool parse_vdf(std::string_view, VdfNode&)
// Bare (unquoted) tokens.
void test_sil9xbvqosqpuelr()
{
	auto root = bstone::VdfNode{};
	const auto is_parsed = bstone::parse_vdf("outer { key value }", root);
	const auto* const outer = root.find_child("outer");
	tester.check(is_parsed && outer != nullptr && outer->find_value("key") == "value");
}

// bool parse_vdf(std::string_view, VdfNode&)
// A UTF-8 byte-order mark is skipped.
void test_8vg16na7z6o5c88f()
{
	auto root = bstone::VdfNode{};
	const auto is_parsed = bstone::parse_vdf("\xEF\xBB\xBF\"key\" \"value\"", root);
	tester.check(is_parsed && root.find_value("key") == "value");
}

// bool parse_vdf(std::string_view, VdfNode&)
// CRLF line endings.
void test_3mu0khfb2i9jxb1k()
{
	auto root = bstone::VdfNode{};
	const auto is_parsed = bstone::parse_vdf("\"outer\"\r\n{\r\n\t\"a\"\t\"1\"\r\n}\r\n", root);
	const auto* const outer = root.find_child("outer");
	tester.check(is_parsed && outer != nullptr && outer->find_value("a") == "1");
}

// ==========================================================================

// const VdfNode* VdfNode::find_child(std::string_view) const
// Lookup is case-insensitive.
void test_c29k06zz2kyho37k()
{
	auto root = bstone::VdfNode{};
	bstone::parse_vdf("\"MiXeD\" \"1\"", root);
	tester.check(
		root.find_child("mixed") != nullptr &&
		root.find_child("MIXED") != nullptr &&
		root.find_child("MiXeD") != nullptr);
}

// const VdfNode* VdfNode::find_child(std::string_view) const
// A missing child yields null.
void test_5lariqpxs8t1lb69()
{
	auto root = bstone::VdfNode{};
	bstone::parse_vdf("\"key\" \"value\"", root);
	tester.check(root.find_child("absent") == nullptr);
}

// std::string_view VdfNode::find_value(std::string_view) const
// A missing child yields an empty value.
void test_lcle5r4sogsq8xyb()
{
	auto root = bstone::VdfNode{};
	bstone::parse_vdf("\"key\" \"value\"", root);
	tester.check(root.find_value("absent").empty());
}

// bool VdfNode::is_object() const
void test_e18zvbckv4wvf97j()
{
	auto root = bstone::VdfNode{};
	bstone::parse_vdf("\"leaf\" \"1\" \"obj\" { \"a\" \"2\" }", root);
	const auto* const leaf = root.find_child("leaf");
	const auto* const obj = root.find_child("obj");
	tester.check(
		leaf != nullptr && !leaf->is_object() &&
		obj != nullptr && obj->is_object());
}

// bool VdfNode::is_object() const
// An empty block is still an object. Steam writes `"apps" {}` for a library
// folder with nothing installed in it.
void test_pw1tqk4vsyn8j2ba()
{
	auto root = bstone::VdfNode{};
	const auto is_parsed = bstone::parse_vdf("\"apps\"\n{\n}\n\"scalar\" \"\"", root);
	const auto* const apps = root.find_child("apps");
	const auto* const scalar = root.find_child("scalar");
	tester.check(
		is_parsed &&
		apps != nullptr && apps->is_object() && apps->children.empty() &&
		scalar != nullptr && !scalar->is_object());
}

// bool parse_vdf(std::string_view, VdfNode&)
// Brackets inside a quoted value are literal, not a platform conditional.
void test_74n2wcbzsp2hbcyc()
{
	auto root = bstone::VdfNode{};
	const auto is_parsed = bstone::parse_vdf("\"path\" \"D:\\\\Games\\\\[Steam]\"", root);
	tester.check(is_parsed && root.find_value("path") == "D:\\Games\\[Steam]");
}

// bool parse_vdf(std::string_view, VdfNode&)
// A UNC library path only survives if escapes are processed.
void test_bnbx7yrpc5f5owsc()
{
	auto root = bstone::VdfNode{};
	const auto is_parsed = bstone::parse_vdf("\"path\" \"\\\\\\\\NAS\\\\Share\"", root);
	tester.check(is_parsed && root.find_value("path") == "\\\\NAS\\Share");
}

// bool parse_vdf(std::string_view, VdfNode&)
// Scalar siblings sit alongside the numbered library entries, so a caller must be
// able to tell them apart.
void test_yqbm5f8hbnjy1mjb()
{
	constexpr auto text =
		"\"libraryfolders\"\n"
		"{\n"
		"\t\"contentstatsid\"\t\t\"123456789\"\n"
		"\t\"0\"\n"
		"\t{\n"
		"\t\t\"path\"\t\t\"/steam\"\n"
		"\t}\n"
		"}\n";
	auto root = bstone::VdfNode{};
	const auto is_parsed = bstone::parse_vdf(text, root);
	const auto* const folders = root.find_child("libraryfolders");
	const auto* const stats = folders != nullptr ? folders->find_child("contentstatsid") : nullptr;
	const auto* const entry = folders != nullptr ? folders->find_child("0") : nullptr;
	tester.check(
		is_parsed &&
		folders != nullptr && folders->children.size() == 2 &&
		stats != nullptr && !stats->is_object() &&
		entry != nullptr && entry->is_object() &&
		entry->find_value("path") == "/steam");
}

// const VdfNode* VdfNode::find_child(std::string_view) const
// Duplicate keys are kept, and the first one wins, as KeyValues does.
void test_h1i2pvjnz6ftwgg2()
{
	auto root = bstone::VdfNode{};
	const auto is_parsed = bstone::parse_vdf("\"k\" \"first\" \"k\" \"second\"", root);
	tester.check(is_parsed && root.children.size() == 2 && root.find_value("k") == "first");
}

// ==========================================================================

// bool parse_vdf(std::string_view, VdfNode&)
// Malformed: unterminated quoted string.
void test_gty4pdk0fdvl7b2x()
{
	auto root = bstone::VdfNode{};
	const auto is_parsed = bstone::parse_vdf("\"key\" \"unterminated", root);
	tester.check(!is_parsed && root.children.empty());
}

// bool parse_vdf(std::string_view, VdfNode&)
// Malformed: unterminated block.
void test_ko3ztf6uag1gfbip()
{
	auto root = bstone::VdfNode{};
	const auto is_parsed = bstone::parse_vdf("\"outer\"\n{\n\t\"a\"\t\"1\"\n", root);
	tester.check(!is_parsed && root.children.empty());
}

// bool parse_vdf(std::string_view, VdfNode&)
// Malformed: a stray closing brace.
void test_z3lwoanvegbfdmvi()
{
	auto root = bstone::VdfNode{};
	const auto is_parsed = bstone::parse_vdf("}", root);
	tester.check(!is_parsed && root.children.empty());
}

// bool parse_vdf(std::string_view, VdfNode&)
// Malformed: a key with neither a value nor a block.
void test_ifv09e68sbis1bcm()
{
	auto root = bstone::VdfNode{};
	const auto is_parsed = bstone::parse_vdf("\"lonely\"", root);
	tester.check(!is_parsed && root.children.empty());
}

// bool parse_vdf(std::string_view, VdfNode&)
// Malformed: nesting deeper than the parser accepts is rejected rather than
// overflowing the stack.
void test_0lwm7ztdn1hypbr1()
{
	auto text = std::string{};
	constexpr auto depth = 200;
	for (auto i = 0; i < depth; ++i)
	{
		text += "\"a\"\n{\n";
	}
	for (auto i = 0; i < depth; ++i)
	{
		text += "}\n";
	}
	auto root = bstone::VdfNode{};
	const auto is_parsed = bstone::parse_vdf(text, root);
	tester.check(!is_parsed && root.children.empty());
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_parse();
		register_lookup();
		register_malformed();
	}

private:
	void register_parse()
	{
		tester.register_test("Vdf#c6924ee2i0oiegx3", test_c6924ee2i0oiegx3);
		tester.register_test("Vdf#38rta6pubumw927k", test_38rta6pubumw927k);
		tester.register_test("Vdf#mtdqyztui42hbkva", test_mtdqyztui42hbkva);
		tester.register_test("Vdf#10iqosiccebjqz79", test_10iqosiccebjqz79);
		tester.register_test("Vdf#z9aotytq2nxntmts", test_z9aotytq2nxntmts);
		tester.register_test("Vdf#lo89hdqsgqjdxx40", test_lo89hdqsgqjdxx40);
		tester.register_test("Vdf#u8vx20e1ptkzq6px", test_u8vx20e1ptkzq6px);
		tester.register_test("Vdf#xga7pncvvw9taohu", test_xga7pncvvw9taohu);
		tester.register_test("Vdf#oromby1njuckga1u", test_oromby1njuckga1u);
		tester.register_test("Vdf#maicoqzy0n4k78oc", test_maicoqzy0n4k78oc);
		tester.register_test("Vdf#sil9xbvqosqpuelr", test_sil9xbvqosqpuelr);
		tester.register_test("Vdf#8vg16na7z6o5c88f", test_8vg16na7z6o5c88f);
		tester.register_test("Vdf#3mu0khfb2i9jxb1k", test_3mu0khfb2i9jxb1k);
	}

	void register_lookup()
	{
		tester.register_test("Vdf#c29k06zz2kyho37k", test_c29k06zz2kyho37k);
		tester.register_test("Vdf#5lariqpxs8t1lb69", test_5lariqpxs8t1lb69);
		tester.register_test("Vdf#lcle5r4sogsq8xyb", test_lcle5r4sogsq8xyb);
		tester.register_test("Vdf#e18zvbckv4wvf97j", test_e18zvbckv4wvf97j);
		tester.register_test("Vdf#pw1tqk4vsyn8j2ba", test_pw1tqk4vsyn8j2ba);
		tester.register_test("Vdf#74n2wcbzsp2hbcyc", test_74n2wcbzsp2hbcyc);
		tester.register_test("Vdf#bnbx7yrpc5f5owsc", test_bnbx7yrpc5f5owsc);
		tester.register_test("Vdf#yqbm5f8hbnjy1mjb", test_yqbm5f8hbnjy1mjb);
		tester.register_test("Vdf#h1i2pvjnz6ftwgg2", test_h1i2pvjnz6ftwgg2);
	}

	void register_malformed()
	{
		tester.register_test("Vdf#gty4pdk0fdvl7b2x", test_gty4pdk0fdvl7b2x);
		tester.register_test("Vdf#ko3ztf6uag1gfbip", test_ko3ztf6uag1gfbip);
		tester.register_test("Vdf#z3lwoanvegbfdmvi", test_z3lwoanvegbfdmvi);
		tester.register_test("Vdf#ifv09e68sbis1bcm", test_ifv09e68sbis1bcm);
		tester.register_test("Vdf#0lwm7ztdn1hypbr1", test_0lwm7ztdn1hypbr1);
	}
};

auto registrator = Registrator{};

} // namespace
