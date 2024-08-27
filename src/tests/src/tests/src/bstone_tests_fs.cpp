#include <string>

#include "bstone_file.h"
#include "bstone_fs.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

#ifdef _WIN32
	#define BSTONE_TESTS_PATH_SEPARATOR_STRING "\\"
#else
	#define BSTONE_TESTS_PATH_SEPARATOR_STRING "/"
#endif

// ==========================================================================

// std::intptr_t get_working_directory(char*, std::intptr_t)
void test_oe4nc54c2sipndrz()
{
	constexpr auto max_size = 4096;
	auto tag_path = std::string{};
	tag_path.reserve(max_size + 128);
	tag_path.resize(max_size);
	const auto written_size = bstone::fs::get_working_directory(&tag_path.front(), max_size);
	tag_path.resize(written_size);
	tag_path += bstone::fs::native_separator;
	tag_path += "data";
	tag_path += bstone::fs::native_separator;
	tag_path += "bstone_tests_tag.txt";
	const auto tag_file = bstone::File{tag_path.c_str()};
}

// ==========================================================================

// void set_working_directory(const char*)
void test_bw6ltnfefy02ekxt()
{
	constexpr auto max_size = 4096;
	auto tag_path = std::string{};
	tag_path.reserve(max_size + 128);
	tag_path.resize(max_size);
	const auto written_size = bstone::fs::get_working_directory(&tag_path.front(), max_size);
	tag_path.resize(written_size);
	bstone::fs::set_working_directory(tag_path.c_str());
	tag_path += bstone::fs::native_separator;
	tag_path += "data";
	tag_path += bstone::fs::native_separator;
	tag_path += "bstone_tests_tag.txt";
	const auto tag_file = bstone::File{tag_path.c_str()};
}

// ==========================================================================

// void create_directory(const char*)
void test_tehkg642yrgm2ykz()
{
	constexpr auto max_size = 4096;
	auto current_path = std::string{};
	current_path.reserve(max_size + 128);
	current_path.resize(max_size);
	const auto written_size = bstone::fs::get_working_directory(&current_path.front(), max_size);
	current_path.resize(written_size);

	auto dir_path = std::string{};
	dir_path.reserve(current_path.size() + 128);

	dir_path.clear();
	dir_path += current_path;
	dir_path += bstone::fs::native_separator;
	dir_path += "data";
	dir_path += bstone::fs::native_separator;
	dir_path += "fs_create_directory";
	bstone::fs::remove_if_exists(dir_path.c_str());

	bstone::fs::create_directory(dir_path.c_str());
}

// ==========================================================================

// void create_directories(const char*)
void test_si3ofxkurvgv5v7g()
{
	constexpr auto max_size = 4096;
	auto current_path = std::string{};
	current_path.reserve(max_size + 128);
	current_path.resize(max_size);
	const auto written_size = bstone::fs::get_working_directory(&current_path.front(), max_size);
	current_path.resize(written_size);

	auto dir_path = std::string{};
	dir_path.reserve(current_path.size() + 128);

	dir_path.clear();
	dir_path += current_path;
	dir_path += bstone::fs::native_separator;
	dir_path += "data";
	dir_path += bstone::fs::native_separator;
	dir_path += "fs_create_directories";
	dir_path += bstone::fs::native_separator;
	dir_path += "1";
	bstone::fs::remove_if_exists(dir_path.c_str());

	dir_path.clear();
	dir_path += current_path;
	dir_path += bstone::fs::native_separator;
	dir_path += "data";
	dir_path += bstone::fs::native_separator;
	dir_path += "fs_create_directories";
	bstone::fs::remove_if_exists(dir_path.c_str());

	dir_path += bstone::fs::native_separator;
	dir_path += "1";
	bstone::fs::create_directories(dir_path.c_str());
}

// ==========================================================================

// void rename(const char*, const char*)
// Directory.
void test_vev33rwqzayizqj9()
{
	constexpr auto max_size = 4096;
	auto current_path = std::string{};
	current_path.reserve(max_size + 128);
	current_path.resize(max_size);
	const auto written_size = bstone::fs::get_working_directory(&current_path.front(), max_size);
	current_path.resize(written_size);

	auto dir_path = std::string{};
	dir_path.reserve(current_path.size() + 128);

	dir_path.clear();
	dir_path += current_path;
	dir_path += bstone::fs::native_separator;
	dir_path += "data";
	dir_path += bstone::fs::native_separator;
	dir_path += "fs_rename_directory";
	bstone::fs::remove_if_exists(dir_path.c_str());

	dir_path += '1';
	bstone::fs::remove_if_exists(dir_path.c_str());

	dir_path.clear();
	dir_path += current_path;
	dir_path += bstone::fs::native_separator;
	dir_path += "data";
	dir_path += bstone::fs::native_separator;
	dir_path += "fs_rename_directory";
	const auto new_dir_path = dir_path;
	dir_path += '1';
	bstone::fs::create_directories(dir_path.c_str());

	bstone::fs::rename(dir_path.c_str(), new_dir_path.c_str());
}

// ==========================================================================

// void rename(const char*, const char*)
// File.
void test_kya8vcige6hhxfpm()
{
	constexpr auto max_size = 4096;
	auto current_path = std::string{};
	current_path.reserve(max_size + 128);
	current_path.resize(max_size);
	const auto written_size = bstone::fs::get_working_directory(&current_path.front(), max_size);
	current_path.resize(written_size);

	auto dir_path = std::string{};
	dir_path.reserve(current_path.size() + 128);

	dir_path.clear();
	dir_path += current_path;
	dir_path += bstone::fs::native_separator;
	dir_path += "data";
	dir_path += bstone::fs::native_separator;
	dir_path += "fs_rename_file";
	dir_path += bstone::fs::native_separator;
	dir_path += "1.txt";
	bstone::fs::remove_if_exists(dir_path.c_str());

	dir_path.clear();
	dir_path += current_path;
	dir_path += bstone::fs::native_separator;
	dir_path += "data";
	dir_path += bstone::fs::native_separator;
	dir_path += "fs_rename_file";
	dir_path += bstone::fs::native_separator;
	dir_path += "1_.txt";
	bstone::fs::remove_if_exists(dir_path.c_str());

	dir_path.clear();
	dir_path += current_path;
	dir_path += bstone::fs::native_separator;
	dir_path += "data";
	dir_path += bstone::fs::native_separator;
	dir_path += "fs_rename_file";
	bstone::fs::remove_if_exists(dir_path.c_str());

	dir_path.clear();
	dir_path += current_path;
	dir_path += bstone::fs::native_separator;
	dir_path += "data";
	dir_path += bstone::fs::native_separator;
	dir_path += "fs_rename_file";
	bstone::fs::create_directories(dir_path.c_str());

	dir_path += bstone::fs::native_separator;
	dir_path += "1_.txt";
	{
		const auto file = bstone::File{dir_path.c_str(), bstone::FileOpenFlags::create, bstone::FileShareMode::exclusive};
	}

	auto new_dir_path = std::string{};
	new_dir_path.reserve(dir_path.size());
	new_dir_path += current_path;
	new_dir_path += bstone::fs::native_separator;
	new_dir_path += "data";
	new_dir_path += bstone::fs::native_separator;
	new_dir_path += "fs_rename_file";
	new_dir_path += bstone::fs::native_separator;
	new_dir_path += "1.txt";

	bstone::fs::rename(dir_path.c_str(), new_dir_path.c_str());
}

// ==========================================================================

// void remove_if_exists(const char*)
// Directory.
void test_eqw97jgos6libcl8()
{
	constexpr auto max_size = 4096;
	auto current_path = std::string{};
	current_path.reserve(max_size + 128);
	current_path.resize(max_size);
	const auto written_size = bstone::fs::get_working_directory(&current_path.front(), max_size);
	current_path.resize(written_size);

	auto dir_path = std::string{};
	dir_path.reserve(current_path.size() + 128);

	dir_path.clear();
	dir_path += current_path;
	dir_path += bstone::fs::native_separator;
	dir_path += "data";
	dir_path += bstone::fs::native_separator;
	dir_path += "remove_if_exists_directory";
	bstone::fs::remove_if_exists(dir_path.c_str());
	bstone::fs::remove_if_exists(dir_path.c_str());

	bstone::fs::create_directory(dir_path.c_str());
	bstone::fs::remove_if_exists(dir_path.c_str());
	bstone::fs::remove_if_exists(dir_path.c_str());
}

// ==========================================================================

// void remove_if_exists(const char*)
// File.
void test_fe5kz9xekiti9zof()
{
	constexpr auto max_size = 4096;
	auto current_path = std::string{};
	current_path.reserve(max_size + 128);
	current_path.resize(max_size);
	const auto written_size = bstone::fs::get_working_directory(&current_path.front(), max_size);
	current_path.resize(written_size);

	auto dir_path = std::string{};
	dir_path.reserve(current_path.size() + 128);

	dir_path.clear();
	dir_path += current_path;
	dir_path += bstone::fs::native_separator;
	dir_path += "data";
	dir_path += bstone::fs::native_separator;
	dir_path += "remove_if_exists_file";
	dir_path += bstone::fs::native_separator;
	dir_path += "1.txt";
	bstone::fs::remove_if_exists(dir_path.c_str());
	bstone::fs::remove_if_exists(dir_path.c_str());

	dir_path.clear();
	dir_path += current_path;
	dir_path += bstone::fs::native_separator;
	dir_path += "data";
	dir_path += bstone::fs::native_separator;
	dir_path += "remove_if_exists_file";
	bstone::fs::remove_if_exists(dir_path.c_str());

	bstone::fs::create_directory(dir_path.c_str());

	dir_path += bstone::fs::native_separator;
	dir_path += "1.txt";
	{
		const auto file = bstone::File{dir_path.c_str(), bstone::FileOpenFlags::create, bstone::FileShareMode::exclusive};
	}

	bstone::fs::remove_if_exists(dir_path.c_str());
	bstone::fs::remove_if_exists(dir_path.c_str());
}

// ==========================================================================

// void remove(const char*)
// Directory.
void test_2lduj52sj26u8ues()
{
	constexpr auto max_size = 4096;
	auto current_path = std::string{};
	current_path.reserve(max_size + 128);
	current_path.resize(max_size);
	const auto written_size = bstone::fs::get_working_directory(&current_path.front(), max_size);
	current_path.resize(written_size);

	auto dir_path = std::string{};
	dir_path.reserve(current_path.size() + 128);

	dir_path.clear();
	dir_path += current_path;
	dir_path += bstone::fs::native_separator;
	dir_path += "data";
	dir_path += bstone::fs::native_separator;
	dir_path += "remove_directory";
	bstone::fs::remove_if_exists(dir_path.c_str());

	bstone::fs::create_directory(dir_path.c_str());
	bstone::fs::remove(dir_path.c_str());
}

// ==========================================================================

// void remove(const char*)
// File.
void test_pb7luss22on0qt4t()
{
	constexpr auto max_size = 4096;
	auto current_path = std::string{};
	current_path.reserve(max_size + 128);
	current_path.resize(max_size);
	const auto written_size = bstone::fs::get_working_directory(&current_path.front(), max_size);
	current_path.resize(written_size);

	auto dir_path = std::string{};
	dir_path.reserve(current_path.size() + 128);

	dir_path.clear();
	dir_path += current_path;
	dir_path += bstone::fs::native_separator;
	dir_path += "data";
	dir_path += bstone::fs::native_separator;
	dir_path += "remove_file";
	dir_path += bstone::fs::native_separator;
	dir_path += "1.txt";
	bstone::fs::remove_if_exists(dir_path.c_str());

	dir_path.clear();
	dir_path += current_path;
	dir_path += bstone::fs::native_separator;
	dir_path += "data";
	dir_path += bstone::fs::native_separator;
	dir_path += "remove_file";
	bstone::fs::remove_if_exists(dir_path.c_str());

	bstone::fs::create_directory(dir_path.c_str());

	dir_path += bstone::fs::native_separator;
	dir_path += "1.txt";
	{
		const auto file = bstone::File{dir_path.c_str(), bstone::FileOpenFlags::create, bstone::FileShareMode::exclusive};
	}

	bstone::fs::remove(dir_path.c_str());
}

// ==========================================================================

// bool is_directory_exists(const char* path)
void test_yjjwwi33w89bdb89()
{
	constexpr auto valid_dir_path = "data";
	constexpr auto invalid_dir_path = "/:/";

	tester.check(bstone::fs::is_directory_exists(valid_dir_path));
	tester.check(!bstone::fs::is_directory_exists(invalid_dir_path));
}

// ==========================================================================

// bool is_regular_file_exists(const char* path)
void test_4iveypn2vd69u4w6()
{
	constexpr auto valid_file_path = "data" BSTONE_TESTS_PATH_SEPARATOR_STRING "bstone_tests_tag.txt";
	constexpr auto invalid_file_path = "/:/";

	tester.check(bstone::fs::is_regular_file_exists(valid_file_path));
	tester.check(!bstone::fs::is_regular_file_exists(invalid_file_path));
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_get_current_directory();
		register_set_current_directory();
		register_create_directory();
		register_create_directories();
		register_rename();
		register_remove_if_exists();
		register_remove();
		register_is_directory_exists();
		register_is_regular_file_exists();
	}

private:
	void register_get_current_directory()
	{
		tester.register_test("fs#oe4nc54c2sipndrz", test_oe4nc54c2sipndrz);
	}

	void register_set_current_directory()
	{
		tester.register_test("fs#bw6ltnfefy02ekxt", test_bw6ltnfefy02ekxt);
	}
	
	void register_create_directory()
	{
		tester.register_test("fs#tehkg642yrgm2ykz", test_tehkg642yrgm2ykz);
	}

	void register_create_directories()
	{
		tester.register_test("fs#si3ofxkurvgv5v7g", test_si3ofxkurvgv5v7g);
	}

	void register_rename()
	{
		tester.register_test("fs#vev33rwqzayizqj9", test_vev33rwqzayizqj9);
		tester.register_test("fs#kya8vcige6hhxfpm", test_kya8vcige6hhxfpm);
	}

	void register_remove_if_exists()
	{
		tester.register_test("fs#eqw97jgos6libcl8", test_eqw97jgos6libcl8);
		tester.register_test("fs#fe5kz9xekiti9zof", test_fe5kz9xekiti9zof);
	}

	void register_remove()
	{
		tester.register_test("fs#2lduj52sj26u8ues", test_2lduj52sj26u8ues);
		tester.register_test("fs#pb7luss22on0qt4t", test_pb7luss22on0qt4t);
	}

	void register_is_directory_exists()
	{
		tester.register_test("fs#yjjwwi33w89bdb89", test_yjjwwi33w89bdb89);
	}

	void register_is_regular_file_exists()
	{
		tester.register_test("fs#4iveypn2vd69u4w6", test_4iveypn2vd69u4w6);
	}
};

auto registrator = Registrator{};

} // namespace
