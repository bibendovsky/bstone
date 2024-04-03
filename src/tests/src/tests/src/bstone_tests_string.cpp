#include <cstdint>

#include <algorithm>
#include <list>
#include <string>
#include <memory>
#include <utility>

#include "bstone_memory_resource.h"
#include "bstone_string.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

constexpr auto memory_fill_value = 'H';

using String = bstone::BasicString<char>;

class MemoryResource final : public bstone::MemoryResource
{
public:
	MemoryResource() = default;
	~MemoryResource() override = default;

private:
	void* do_allocate(std::intptr_t size) override
	{
		const auto chars = bstone::get_default_memory_resource().allocate<char>(size);
		std::fill_n(chars, size, memory_fill_value);
		return chars;
	}

	void do_deallocate(void* ptr) noexcept override
	{
		bstone::get_default_memory_resource().deallocate(ptr);
	}
};

bstone::MemoryResource& get_memory_resource()
{
	static auto memory_resource = MemoryResource{};

	return memory_resource;
}

template<typename TDstChar>
std::list<TDstChar> make_char_list(const char* src_string)
{
	const auto size = std::char_traits<char>::length(src_string);
	auto dst_list = std::list<TDstChar>{src_string, src_string + size};
	return dst_list;
}

template<typename TDstChar>
std::basic_string<TDstChar> make_string(const char* src_string)
{
	const auto size = std::char_traits<char>::length(src_string);
	auto dst_string = std::basic_string<TDstChar>{};
	dst_string.resize(size);
	std::copy(src_string, src_string + size, dst_string.begin());
	return dst_string;
}

template<typename TChar>
bool are_equal(const char* a, const TChar* b, std::intptr_t b_size)
{
	const auto a_size = std::char_traits<char>::length(a);
	return std::equal(a, a + a_size, b, b + b_size);
}

// ==========================================================================

// BasicString()
template<typename TChar>
void test_l46dsv6fbj3zravq()
{
	const auto string = bstone::BasicString<TChar>{};

	const auto data = string.get_data();
	const auto has_data = data != nullptr;
	const auto capacity = string.get_capacity();
	const auto size = string.get_size();
	const auto is_first_null = has_data && *data == '\0';

	tester.check(has_data && capacity >= 0 && size == 0 && is_first_null);
}

void test_l46dsv6fbj3zravq_char()
{
	test_l46dsv6fbj3zravq<char>();
}

void test_l46dsv6fbj3zravq_wchar_t()
{
	test_l46dsv6fbj3zravq<wchar_t>();
}

// ==========================================================================

// BasicString(MemoryResource&)
template<typename TChar>
void test_8gl9zxe1fn4o78da()
{
	const auto string = bstone::BasicString<TChar>{get_memory_resource()};
	const auto data = string.get_data();
	const auto has_data = data != nullptr;
	const auto capacity = string.get_capacity();
	const auto size = string.get_size();
	const auto is_first_null = has_data && *data == '\0';
	tester.check(has_data && capacity >= 0 && size == 0 && is_first_null);
}

void test_8gl9zxe1fn4o78da_char()
{
	test_8gl9zxe1fn4o78da<char>();
}

void test_8gl9zxe1fn4o78da_wchar_t()
{
	test_8gl9zxe1fn4o78da<wchar_t>();
}

// ==========================================================================

// BasicString(std::intptr_t, StringCapacityTag)
template<typename TChar>
void test_i3gssmuub094jhxi()
{
	constexpr auto ref_capacity = 3;
	const auto string = bstone::BasicString<TChar>{ref_capacity, bstone::StringCapacityTag{}};
	const auto data = string.get_data();
	const auto capacity = string.get_capacity();
	const auto size = string.get_size();

	const auto is_valid_1 = size == 0;
	const auto is_valid_2 = capacity == ref_capacity;
	const auto is_valid_3 = data[0] == '\0';

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_i3gssmuub094jhxi_char()
{
	test_i3gssmuub094jhxi<char>();
}

void test_i3gssmuub094jhxi_wchar_t()
{
	test_i3gssmuub094jhxi<wchar_t>();
}

// ==========================================================================

// BasicString(std::intptr_t, StringCapacityTag, MemoryResource&)
template<typename TChar>
void test_f6nsg7ws4in9ypvw()
{
	constexpr auto ref_capacity = 3;
	const auto string = bstone::BasicString<TChar>{
		ref_capacity, bstone::StringCapacityTag{}, get_memory_resource()};
	const auto data = string.get_data();
	const auto capacity = string.get_capacity();
	const auto size = string.get_size();

	const auto is_valid_1 = size == 0;
	const auto is_valid_2 = capacity >= ref_capacity;
	const auto is_valid_3 = data != nullptr;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_f6nsg7ws4in9ypvw_char()
{
	test_f6nsg7ws4in9ypvw<char>();
}

void test_f6nsg7ws4in9ypvw_wchar_t()
{
	test_f6nsg7ws4in9ypvw<wchar_t>();
}

// ==========================================================================

// BasicString(TChar, std::intptr_t, StringFillTag)
template<typename TChar>
void test_exn9k1icofwfnqm3()
{
	constexpr auto ref_count = 3;
	const auto string = bstone::BasicString<TChar>{TChar{'1'}, ref_count, bstone::StringFillTag{}};
	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == ref_count &&
		data[0] == '1' &&
		data[1] == '1' &&
		data[2] == '1' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_exn9k1icofwfnqm3_char()
{
	test_exn9k1icofwfnqm3<char>();
}

void test_exn9k1icofwfnqm3_wchar_t()
{
	test_exn9k1icofwfnqm3<wchar_t>();
}

// ==========================================================================

// BasicString(TChar, std::intptr_t, MemoryResource&)
template<typename TChar>
void test_3wd7p3xrpf24pbpm()
{
	constexpr auto ref_count = 3;

	const auto string = bstone::BasicString<TChar>
	{
		TChar{'1'},
		std::intptr_t{ref_count},
		get_memory_resource()
	};

	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == ref_count &&
		data[0] == '1' &&
		data[1] == '1' &&
		data[2] == '1' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_3wd7p3xrpf24pbpm_char()
{
	test_3wd7p3xrpf24pbpm<char>();
}

void test_3wd7p3xrpf24pbpm_wchar_t()
{
	test_3wd7p3xrpf24pbpm<wchar_t>();
}

// ==========================================================================

// BasicString(const TChar*)
template<typename TChar>
void test_gxlnmigx4uzpsxaw()
{
	constexpr auto ref_string = "string";
	const auto src_string = make_string<TChar>(ref_string);

	const auto string = bstone::BasicString<TChar>{src_string.c_str()};

	const auto data = string.get_data();
	const auto has_data = data != nullptr;
	const auto capacity = string.get_capacity();
	const auto size = string.get_size();
	const auto is_equal = has_data && are_equal(ref_string, data, size);
	const auto has_null = has_data && data[src_string.size()] == '\0';

	tester.check(
		has_data &&
		capacity == size &&
		size == static_cast<std::intptr_t>(src_string.size()) &&
		is_equal &&
		has_null);
}

void test_gxlnmigx4uzpsxaw_char()
{
	test_gxlnmigx4uzpsxaw<char>();
}

void test_gxlnmigx4uzpsxaw_wchar_t()
{
	test_gxlnmigx4uzpsxaw<wchar_t>();
}

// ==========================================================================

// BasicString(const TChar*, MemoryResource&)
template<typename TChar>
void test_hv6wr0cirp1u2r2d()
{
	constexpr auto ref_string = "string";
	const auto src_string = make_string<TChar>(ref_string);
	const auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto data = string.get_data();
	const auto has_data = data != nullptr;
	const auto capacity = string.get_capacity();
	const auto size = string.get_size();
	const auto is_equal = has_data && are_equal(ref_string, data, size);
	const auto has_null = has_data && data[src_string.size()] == '\0';

	tester.check(
		has_data &&
		capacity == size &&
		size == static_cast<std::intptr_t>(src_string.size()) &&
		is_equal &&
		has_null);
}

void test_hv6wr0cirp1u2r2d_char()
{
	test_hv6wr0cirp1u2r2d<char>();
}

void test_hv6wr0cirp1u2r2d_wchar_t()
{
	test_hv6wr0cirp1u2r2d<wchar_t>();
}

// ==========================================================================

// BasicString(const TChar*, std::intptr_t)
template<typename TChar>
void test_sn0okd13g7zxzrhu()
{
	constexpr auto ref_string = "string";
	const auto src_string = make_string<TChar>(ref_string);

	const auto string = bstone::BasicString<TChar>{
		src_string.c_str(),
		static_cast<std::intptr_t>(src_string.size())
	};

	const auto data = string.get_data();
	const auto has_data = data != nullptr;
	const auto capacity = string.get_capacity();
	const auto size = string.get_size();
	const auto is_equal = has_data && are_equal(ref_string, data, size);
	const auto has_null = has_data && data[src_string.size()] == '\0';

	tester.check(
		has_data &&
		capacity == size &&
		size == static_cast<std::intptr_t>(src_string.size()) &&
		is_equal &&
		has_null);
}

void test_sn0okd13g7zxzrhu_char()
{
	test_sn0okd13g7zxzrhu<char>();
}

void test_sn0okd13g7zxzrhu_wchar_t()
{
	test_sn0okd13g7zxzrhu<wchar_t>();
}

// ==========================================================================

// BasicString(const TChar*, std::intptr_t, MemoryResource&)
template<typename TChar>
void test_q19fgblkse5v8wzx()
{
	constexpr auto ref_string = "string";
	const auto src_string = make_string<TChar>(ref_string);

	const auto string = bstone::BasicString<TChar>
	{
		src_string.c_str(),
		static_cast<std::intptr_t>(src_string.size()),
		get_memory_resource()
	};

	const auto data = string.get_data();
	const auto has_data = data != nullptr;
	const auto capacity = string.get_capacity();
	const auto size = string.get_size();
	const auto is_equal = has_data && are_equal(ref_string, data, size);
	const auto has_null = has_data && data[src_string.size()] == '\0';

	tester.check(
		has_data &&
		capacity == size &&
		size == static_cast<std::intptr_t>(src_string.size()) &&
		is_equal &&
		has_null);
}

void test_q19fgblkse5v8wzx_char()
{
	test_q19fgblkse5v8wzx<char>();
}

void test_q19fgblkse5v8wzx_wchar_t()
{
	test_q19fgblkse5v8wzx<wchar_t>();
}

// ==========================================================================

// BasicString(BasicStringView<Char>)
template<typename TChar>
void test_8t2m6w3w79fic3th()
{
	constexpr auto ref_string = "string";
	const auto src_string = make_string<TChar>(ref_string);

	const auto string = bstone::BasicString<TChar>{bstone::BasicStringView<TChar>{
		src_string.c_str(),
		static_cast<std::intptr_t>(src_string.size())}};

	const auto data = string.get_data();
	const auto has_data = data != nullptr;
	const auto capacity = string.get_capacity();
	const auto size = string.get_size();
	const auto is_equal = has_data && are_equal(ref_string, data, size);
	const auto has_null = has_data && data[src_string.size()] == '\0';

	tester.check(
		has_data &&
		capacity == size &&
		size == static_cast<std::intptr_t>(src_string.size()) &&
		is_equal &&
		has_null);
}

void test_8t2m6w3w79fic3th_char()
{
	test_8t2m6w3w79fic3th<char>();
}

void test_8t2m6w3w79fic3th_wchar_t()
{
	test_8t2m6w3w79fic3th<wchar_t>();
}

// ==========================================================================

// BasicString(BasicStringView<Char>, MemoryResource&)
template<typename TChar>
void test_asingpo2nvazj5yd()
{
	constexpr auto ref_string = "string";
	const auto src_string = make_string<TChar>(ref_string);

	const auto string = bstone::BasicString<TChar>{
		bstone::BasicStringView<TChar>{
			src_string.c_str(),
			static_cast<std::intptr_t>(src_string.size())},
		get_memory_resource()};

	const auto data = string.get_data();
	const auto has_data = data != nullptr;
	const auto capacity = string.get_capacity();
	const auto size = string.get_size();
	const auto is_equal = has_data && are_equal(ref_string, data, size);
	const auto has_null = has_data && data[src_string.size()] == '\0';

	tester.check(
		has_data &&
		capacity == size &&
		size == static_cast<std::intptr_t>(src_string.size()) &&
		is_equal &&
		has_null);
}

void test_asingpo2nvazj5yd_char()
{
	test_asingpo2nvazj5yd<char>();
}

void test_asingpo2nvazj5yd_wchar_t()
{
	test_asingpo2nvazj5yd<wchar_t>();
}

// ==========================================================================

// BasicString(const BasicString&)
template<typename TChar>
void test_q3byhe2g3l5j9x6c()
{
	constexpr auto ref_string = "string";
	const auto src_string = make_string<TChar>(ref_string);

	const auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto string = string_1;

	const auto data = string.get_data();
	const auto has_data = data != nullptr;
	const auto capacity = string.get_capacity();
	const auto size = string.get_size();
	const auto is_equal = has_data && are_equal(ref_string, data, size);
	const auto has_null = has_data && data[src_string.size()] == '\0';

	tester.check(
		has_data &&
		capacity == size &&
		size == static_cast<std::intptr_t>(src_string.size()) &&
		is_equal &&
		has_null);
}

void test_q3byhe2g3l5j9x6c_char()
{
	test_q3byhe2g3l5j9x6c<char>();
}

void test_q3byhe2g3l5j9x6c_wchar_t()
{
	test_q3byhe2g3l5j9x6c<wchar_t>();
}

// ==========================================================================

// BasicString(BasicString&&) noexcept
template<typename TChar>
void test_in9b2l1ltmse0oph()
{
	constexpr auto ref_string = "string";
	const auto src_string = make_string<TChar>(ref_string);

	auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto string = std::move(string_1);

	const auto data = string.get_data();
	const auto has_data = data != nullptr;
	const auto capacity = string.get_capacity();
	const auto size = string.get_size();
	const auto is_equal = has_data && are_equal(ref_string, data, size);
	const auto has_null = has_data && data[src_string.size()] == '\0';

	tester.check(
		has_data &&
		capacity == size &&
		size == static_cast<std::intptr_t>(src_string.size()) &&
		is_equal &&
		has_null);
}

void test_in9b2l1ltmse0oph_char()
{
	test_in9b2l1ltmse0oph<char>();
}

void test_in9b2l1ltmse0oph_wchar_t()
{
	test_in9b2l1ltmse0oph<wchar_t>();
}

// ==========================================================================

// BasicString& operator=(const BasicString&)
template<typename TChar>
void test_nobp510mnagq6ywj()
{
	constexpr auto ref_string = "string";
	const auto src_string = make_string<TChar>(ref_string);

	const auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	auto string = bstone::BasicString<TChar>{get_memory_resource()};

	string = string_1;

	const auto data = string.get_data();
	const auto has_data = data != nullptr;
	const auto capacity = string.get_capacity();
	const auto size = string.get_size();
	const auto is_equal = has_data && are_equal(ref_string, data, size);
	const auto has_null = has_data && data[src_string.size()] == '\0';

	tester.check(
		has_data &&
		capacity == size &&
		size == static_cast<std::intptr_t>(src_string.size()) &&
		is_equal &&
		has_null);
}

void test_nobp510mnagq6ywj_char()
{
	test_nobp510mnagq6ywj<char>();
}

void test_nobp510mnagq6ywj_wchar_t()
{
	test_nobp510mnagq6ywj<wchar_t>();
}

// ==========================================================================

// BasicString& operator=(BasicString&&)
template<typename TChar>
void test_621cu7goqwtwyv1o()
{
	constexpr auto ref_string = "string";
	const auto src_string = make_string<TChar>(ref_string);

	auto string = bstone::BasicString<TChar>{get_memory_resource()};

	{
		auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
		string = std::move(string_1);
	}

	const auto data = string.get_data();
	const auto has_data = data != nullptr;
	const auto capacity = string.get_capacity();
	const auto size = string.get_size();
	const auto is_equal = has_data && are_equal(ref_string, data, size);
	const auto has_null = has_data && data[src_string.size()] == '\0';

	tester.check(
		has_data &&
		capacity == size &&
		size == static_cast<std::intptr_t>(src_string.size()) &&
		is_equal &&
		has_null);
}

void test_621cu7goqwtwyv1o_char()
{
	test_621cu7goqwtwyv1o<char>();
}

void test_621cu7goqwtwyv1o_wchar_t()
{
	test_621cu7goqwtwyv1o<wchar_t>();
}

// ==========================================================================

// const Char* get_data() const noexcept
template<typename TChar>
void test_xfpsa3d6zmsisrc1()
{
	constexpr auto ref_string = "str";
	const auto src_string = make_string<TChar>(ref_string);

	auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto data = string.get_data();
	const auto& char_1 = data[0];
	const auto& char_2 = data[1];
	const auto& char_3 = data[2];
	const auto& char_4 = data[3];

	const auto is_valid =
		char_1 == 's' &&
		char_2 == 't' &&
		char_3 == 'r' &&
		char_4 == '\0';

	tester.check(is_valid);
}

void test_xfpsa3d6zmsisrc1_char()
{
	test_xfpsa3d6zmsisrc1<char>();
}

void test_xfpsa3d6zmsisrc1_wchar_t()
{
	test_xfpsa3d6zmsisrc1<wchar_t>();
}

// --------------------------------------------------------------------------

// Char* get_data() noexcept
template<typename TChar>
void test_xxz5fefaennocbmn()
{
	constexpr auto ref_string = "str";
	const auto src_string = make_string<TChar>(ref_string);

	auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto data = string.get_data();
	data[0] += 0;
	data[1] += 0;
	data[2] += 0;
	data[3] += 0;
	const auto& char_1 = data[0];
	const auto& char_2 = data[1];
	const auto& char_3 = data[2];
	const auto& char_4 = data[3];

	const auto is_valid =
		char_1 == 's' &&
		char_2 == 't' &&
		char_3 == 'r' &&
		char_4 == '\0';

	tester.check(is_valid);
}

void test_xxz5fefaennocbmn_char()
{
	test_xxz5fefaennocbmn<char>();
}

void test_xxz5fefaennocbmn_wchar_t()
{
	test_xxz5fefaennocbmn<wchar_t>();
}

// ==========================================================================

// std::intptr_t get_capacity() const noexcept
template<typename TChar>
void test_z1d46bdmylazmr5d()
{
	constexpr auto ref_string = "str";
	const auto src_string = make_string<TChar>(ref_string);

	const auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto is_valid = string.get_capacity() == 3;

	tester.check(is_valid);
}

void test_z1d46bdmylazmr5d_char()
{
	test_z1d46bdmylazmr5d<char>();
}

void test_z1d46bdmylazmr5d_wchar_t()
{
	test_z1d46bdmylazmr5d<wchar_t>();
}

// ==========================================================================

// std::intptr_t get_size() const noexcept
template<typename TChar>
void test_9b5doqxmcsngrghn()
{
	constexpr auto ref_string = "str";
	const auto src_string = make_string<TChar>(ref_string);

	const auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto is_valid = string.get_size() == 3;

	tester.check(is_valid);
}

void test_9b5doqxmcsngrghn_char()
{
	test_9b5doqxmcsngrghn<char>();
}

void test_9b5doqxmcsngrghn_wchar_t()
{
	test_9b5doqxmcsngrghn<wchar_t>();
}

// ==========================================================================

// bool is_empty() const noexcept
template<typename TChar>
void test_9nzsk1kjog5jxvy9()
{
	constexpr auto ref_string = "str";
	const auto src_string = make_string<TChar>(ref_string);

	const auto string_1 = bstone::BasicString<TChar>{get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto is_valid_1 = string_1.is_empty();
	const auto is_valid_2 = !string_2.is_empty();

	tester.check(is_valid_1 && is_valid_2);
}

void test_9nzsk1kjog5jxvy9_char()
{
	test_9nzsk1kjog5jxvy9<char>();
}

void test_9nzsk1kjog5jxvy9_wchar_t()
{
	test_9nzsk1kjog5jxvy9<wchar_t>();
}

// ==========================================================================

// const TChar* begin() const noexcept
template<typename TChar>
void test_194mgikj8lq17j5y()
{
	constexpr auto ref_string = "string";
	const auto src_string = make_string<TChar>(ref_string);

	const auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	tester.check(string.begin() == string.get_data());
}

void test_194mgikj8lq17j5y_char()
{
	test_194mgikj8lq17j5y<char>();
}

void test_194mgikj8lq17j5y_wchar_t()
{
	test_194mgikj8lq17j5y<wchar_t>();
}

// --------------------------------------------------------------------------

// TChar* begin() noexcept
template<typename TChar>
void test_f4iwc1n12d4sj1qg()
{
	constexpr auto ref_string = "string";
	const auto src_string = make_string<TChar>(ref_string);

	auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	tester.check(string.begin() == string.get_data());
}

void test_f4iwc1n12d4sj1qg_char()
{
	test_f4iwc1n12d4sj1qg<char>();
}

void test_f4iwc1n12d4sj1qg_wchar_t()
{
	test_f4iwc1n12d4sj1qg<wchar_t>();
}

// ==========================================================================

// const TChar* end() const noexcept
template<typename TChar>
void test_fm33lb2a6ar3j1j1()
{
	constexpr auto ref_string = "string";
	const auto src_string = make_string<TChar>(ref_string);

	const auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	tester.check(string.end() == string.get_data() + src_string.size());
}

void test_fm33lb2a6ar3j1j1_char()
{
	test_fm33lb2a6ar3j1j1<char>();
}

void test_fm33lb2a6ar3j1j1_wchar_t()
{
	test_fm33lb2a6ar3j1j1<wchar_t>();
}

// --------------------------------------------------------------------------

// TChar* end() noexcept
template<typename TChar>
void test_dpxsbiqqf3y6crin()
{
	constexpr auto ref_string = "string";
	const auto src_string = make_string<TChar>(ref_string);

	auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	tester.check(string.end() == string.get_data() + src_string.size());
}

void test_dpxsbiqqf3y6crin_char()
{
	test_dpxsbiqqf3y6crin<char>();
}

void test_dpxsbiqqf3y6crin_wchar_t()
{
	test_dpxsbiqqf3y6crin<wchar_t>();
}

// ==========================================================================

// const TChar* cbegin() const noexcept
template<typename TChar>
void test_pohekjyqfujtdeg0()
{
	constexpr auto ref_string = "string";
	const auto src_string = make_string<TChar>(ref_string);

	const auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	tester.check(string.cbegin() == string.get_data());
}

void test_pohekjyqfujtdeg0_char()
{
	test_pohekjyqfujtdeg0<char>();
}

void test_pohekjyqfujtdeg0_wchar_t()
{
	test_pohekjyqfujtdeg0<wchar_t>();
}

// ==========================================================================

// const TChar* cend() const noexcept
template<typename TChar>
void test_pk0bpgspw292sgor()
{
	constexpr auto ref_string = "string";
	const auto src_string = make_string<TChar>(ref_string);

	const auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	tester.check(string.cend() == string.get_data() + src_string.size());
}

void test_pk0bpgspw292sgor_char()
{
	test_pk0bpgspw292sgor<char>();
}

void test_pk0bpgspw292sgor_wchar_t()
{
	test_pk0bpgspw292sgor<wchar_t>();
}

// ==========================================================================

// const TChar* cend() const noexcept
template<typename TChar>
void test_ob92iyvz3a4bxawv()
{
	constexpr auto capacity = std::intptr_t{100};

	auto string = bstone::BasicString<TChar>{get_memory_resource()};

	const auto capacity_1 = string.get_capacity();
	const auto data_1 = string.get_data();

	string.reserve(capacity);

	const auto capacity_2 = string.get_capacity();
	const auto data_2 = string.get_data();

	tester.check(
		capacity_1 == 0 &&
		capacity_2 == capacity &&
		data_1 != nullptr &&
		data_2 != nullptr &&
		data_1 != data_2);
}

void test_ob92iyvz3a4bxawv_char()
{
	test_ob92iyvz3a4bxawv<char>();
}

void test_ob92iyvz3a4bxawv_wchar_t()
{
	test_ob92iyvz3a4bxawv<wchar_t>();
}

// ==========================================================================

// void resize(std::intptr_t)
template<typename TChar>
void test_7dx09kya8th4p717()
{
	const auto src_string = make_string<TChar>("s");

	auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto data_1 = string.get_data();
	const auto size_1 = string.get_size();

	const auto is_valid_1 =
		size_1 == 1 &&
		data_1[0] == 's' &&
		data_1[1] == '\0';

	string.resize(3);

	const auto data_2 = string.get_data();
	const auto size_2 = string.get_size();

	const auto is_valid_2 =
		size_2 == 3 &&
		data_2[0] == 's' &&
		data_2[1] == '\0' &&
		data_2[2] == '\0' &&
		data_2[3] == '\0';

	data_2[1] = 't';
	data_2[2] = 'r';

	string.resize(2);

	const auto data_3 = string.get_data();
	const auto size_3 = string.get_size();

	const auto is_valid_3 =
		size_3 == 2 &&
		data_3[0] == 's' &&
		data_3[1] == 't' &&
		data_3[2] == '\0' &&
		data_3[3] == '\0';

	const auto is_valid_4 = data_2 == data_3;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_7dx09kya8th4p717_char()
{
	test_7dx09kya8th4p717<char>();
}

void test_7dx09kya8th4p717_wchar_t()
{
	test_7dx09kya8th4p717<wchar_t>();
}

// --------------------------------------------------------------------------

// void resize(std::intptr_t, Char)
template<typename TChar>
void test_wdj6n73o7vm8x9to()
{
	const auto src_string = make_string<TChar>("s");

	auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto data_1 = string.get_data();
	const auto size_1 = string.get_size();

	const auto is_valid_1 =
		size_1 == 1 &&
		data_1[0] == 's' &&
		data_1[1] == '\0';

	string.resize(3, 'X');

	const auto data_2 = string.get_data();
	const auto size_2 = string.get_size();

	const auto is_valid_2 =
		size_2 == 3 &&
		data_2[0] == 's' &&
		data_2[1] == 'X' &&
		data_2[2] == 'X' &&
		data_2[3] == '\0';

	data_2[1] = 't';
	data_2[2] = 'r';

	string.resize(2);

	const auto data_3 = string.get_data();
	const auto size_3 = string.get_size();

	const auto is_valid_3 =
		size_3 == 2 &&
		data_3[0] == 's' &&
		data_3[1] == 't' &&
		data_3[2] == '\0' &&
		data_3[3] == '\0';

	const auto is_valid_4 = data_2 == data_3;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_wdj6n73o7vm8x9to_char()
{
	test_wdj6n73o7vm8x9to<char>();
}

void test_wdj6n73o7vm8x9to_wchar_t()
{
	test_wdj6n73o7vm8x9to<wchar_t>();
}

// ==========================================================================

// const Char& operator[](std::intptr_t) const
template<typename TChar>
void test_il9qs68frle53apy()
{
	const auto src_string = make_string<TChar>("s");

	const auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto& char_1 = string[0];
	const auto& char_2 = string[1];

	tester.check(char_1 == 's' && char_2 == '\0');
}

void test_il9qs68frle53apy_char()
{
	test_il9qs68frle53apy<char>();
}

void test_il9qs68frle53apy_wchar_t()
{
	test_il9qs68frle53apy<wchar_t>();
}

// --------------------------------------------------------------------------

// Char& operator[](std::intptr_t)
template<typename TChar>
void test_p5ci6kxdkiyl9h62()
{
	const auto src_string = make_string<TChar>("s");

	auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	auto& char_1 = string[0];
	char_1 += 0;
	auto& char_2 = string[1];
	char_2 += 0;

	tester.check(char_1 == 's' && char_2 == '\0');
}

void test_p5ci6kxdkiyl9h62_char()
{
	test_p5ci6kxdkiyl9h62<char>();
}

void test_p5ci6kxdkiyl9h62_wchar_t()
{
	test_p5ci6kxdkiyl9h62<wchar_t>();
}

// ==========================================================================

// const Char& get_front(std::intptr_t) const
template<typename TChar>
void test_bobijkv4mva6a6iw()
{
	const auto src_string = make_string<TChar>("str");

	const auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto& ch = string.get_front();

	tester.check(ch == 's');
}

void test_bobijkv4mva6a6iw_char()
{
	test_bobijkv4mva6a6iw<char>();
}

void test_bobijkv4mva6a6iw_wchar_t()
{
	test_bobijkv4mva6a6iw<wchar_t>();
}

// --------------------------------------------------------------------------

// Char& get_front(std::intptr_t)
template<typename TChar>
void test_6ijbe3eea96r10tb()
{
	const auto src_string = make_string<TChar>("str");

	auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	auto& ch = string.get_front();
	ch += 0;

	tester.check(ch == 's');
}

void test_6ijbe3eea96r10tb_char()
{
	test_6ijbe3eea96r10tb<char>();
}

void test_6ijbe3eea96r10tb_wchar_t()
{
	test_6ijbe3eea96r10tb<wchar_t>();
}

// ==========================================================================

// const Char& get_back(std::intptr_t) const
template<typename TChar>
void test_c7wlkcpbpyxnryt3()
{
	const auto src_string = make_string<TChar>("str");

	const auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto& ch = string.get_back();

	tester.check(ch == 'r');
}

void test_c7wlkcpbpyxnryt3_char()
{
	test_c7wlkcpbpyxnryt3<char>();
}

void test_c7wlkcpbpyxnryt3_wchar_t()
{
	test_c7wlkcpbpyxnryt3<wchar_t>();
}

// --------------------------------------------------------------------------

// Char& get_back(std::intptr_t)
template<typename TChar>
void test_e33fwe8myadjd3ye()
{
	const auto src_string = make_string<TChar>("str");

	auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	auto& ch = string.get_back();
	ch += 0;

	tester.check(ch == 'r');
}

void test_e33fwe8myadjd3ye_char()
{
	test_e33fwe8myadjd3ye<char>();
}

void test_e33fwe8myadjd3ye_wchar_t()
{
	test_e33fwe8myadjd3ye<wchar_t>();
}

// ==========================================================================

// void clear()
template<typename TChar>
void test_m15ygkaar58jzweu()
{
	const auto src_string = make_string<TChar>("str");

	auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	string.clear();

	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == 0 &&
		data[0] == '\0' &&
		data[1] == 't' &&
		data[2] == 'r' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_m15ygkaar58jzweu_char()
{
	test_m15ygkaar58jzweu<char>();
}

void test_m15ygkaar58jzweu_wchar_t()
{
	test_m15ygkaar58jzweu<wchar_t>();
}

// ==========================================================================

// void swap(BasicString&) noexcept
template<typename TChar>
void test_0p5uwwerzn5wsrh9()
{
	const auto src_string = make_string<TChar>("s");

	auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	auto string_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto data_1 = string_1.get_data();
	const auto size_1 = string_1.get_size();
	const auto is_valid_1 = data_1[0] == 's' && data_1[1] == '\0';

	const auto data_2 = string_2.get_data();
	const auto size_2 = string_2.get_size();
	const auto is_valid_2 = data_2[0] == '\0';

	string_1.swap(string_2);

	const auto data_3 = string_1.get_data();
	const auto size_3 = string_1.get_size();
	const auto is_valid_3 = data_3[0] == '\0';

	const auto data_4 = string_2.get_data();
	const auto size_4 = string_2.get_size();
	const auto is_valid_4 = data_4[0] == 's' && data_4[1] == '\0';

	const auto is_valid_5 = size_1 == 1;
	const auto is_valid_6 = size_2 == 0;
	const auto is_valid_7 = size_3 == 0;
	const auto is_valid_8 = size_4 == 1;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		is_valid_5 &&
		is_valid_6 &&
		is_valid_7 &&
		is_valid_8 &&
		true;

	tester.check(is_valid);
}

void test_0p5uwwerzn5wsrh9_char()
{
	test_0p5uwwerzn5wsrh9<char>();
}

void test_0p5uwwerzn5wsrh9_wchar_t()
{
	test_0p5uwwerzn5wsrh9<wchar_t>();
}

// ==========================================================================

// BasicString& assign(Char, std::intptr_t)
template<typename TChar>
void test_g2763fzzb2efirox()
{
	const auto src_string = make_string<TChar>("##############################");

	auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto& string = string_1.assign(TChar{'r'}, 4);

	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == 4 &&
		data[0] == 'r' &&
		data[1] == 'r' &&
		data[2] == 'r' &&
		data[3] == 'r' &&
		data[4] == '\0';

	tester.check(is_valid);
}

void test_g2763fzzb2efirox_char()
{
	test_g2763fzzb2efirox<char>();
}

void test_g2763fzzb2efirox_wchar_t()
{
	test_g2763fzzb2efirox<wchar_t>();
}

// ==========================================================================

// BasicString& assign(const Char*)
template<typename TChar>
void test_9kcssb2px5eqysbo()
{
	const auto src_string_1 = make_string<TChar>("##############################");
	const auto src_string_2 = make_string<TChar>("str");

	auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto& string = string_1.assign(src_string_2.c_str());

	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == 3 &&
		data[0] == 's' &&
		data[1] == 't' &&
		data[2] == 'r' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_9kcssb2px5eqysbo_char()
{
	test_9kcssb2px5eqysbo<char>();
}

void test_9kcssb2px5eqysbo_wchar_t()
{
	test_9kcssb2px5eqysbo<wchar_t>();
}

// ==========================================================================

// BasicString& assign(const Char*, std::intptr_t)
template<typename TChar>
void test_538hehakfc0ftpzi()
{
	const auto src_string_1 = make_string<TChar>("##############################");
	const auto src_string_2 = make_string<TChar>("!!str!!");

	auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto& string = string_1.assign(src_string_2.c_str() + 2, 3);

	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == 3 &&
		data[0] == 's' &&
		data[1] == 't' &&
		data[2] == 'r' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_538hehakfc0ftpzi_char()
{
	test_538hehakfc0ftpzi<char>();
}

void test_538hehakfc0ftpzi_wchar_t()
{
	test_538hehakfc0ftpzi<wchar_t>();
}

// ==========================================================================

// BasicString& assign(const BasicString&)
template<typename TChar>
void test_dle1ktr6lapaeu7c()
{
	const auto src_string_1 = make_string<TChar>("##############################");
	const auto src_string_2 = make_string<TChar>("str");

	auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};

	const auto& string = string_1.assign(string_2);

	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == 3 &&
		data[0] == 's' &&
		data[1] == 't' &&
		data[2] == 'r' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_dle1ktr6lapaeu7c_char()
{
	test_dle1ktr6lapaeu7c<char>();
}

void test_dle1ktr6lapaeu7c_wchar_t()
{
	test_dle1ktr6lapaeu7c<wchar_t>();
}

// ==========================================================================

// BasicString& assign(const BasicString&, std::intptr_t)
template<typename TChar>
void test_w9mioe7ry1g2i75g()
{
	const auto src_string_1 = make_string<TChar>("##############################");
	const auto src_string_2 = make_string<TChar>("!!str");

	auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};

	const auto& string = string_1.assign(string_2, 2);

	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == 3 &&
		data[0] == 's' &&
		data[1] == 't' &&
		data[2] == 'r' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_w9mioe7ry1g2i75g_char()
{
	test_w9mioe7ry1g2i75g<char>();
}

void test_w9mioe7ry1g2i75g_wchar_t()
{
	test_w9mioe7ry1g2i75g<wchar_t>();
}

// ==========================================================================

// BasicString& assign(const BasicString& rhs, std::intptr_t, std::intptr_t)
template<typename TChar>
void test_itc6c77uw485667y()
{
	const auto src_string_1 = make_string<TChar>("##############################");
	const auto src_string_2 = make_string<TChar>("!!str!!");

	auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};

	const auto& string = string_1.assign(string_2, 2, 3);

	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == 3 &&
		data[0] == 's' &&
		data[1] == 't' &&
		data[2] == 'r' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_itc6c77uw485667y_char()
{
	test_itc6c77uw485667y<char>();
}

void test_itc6c77uw485667y_wchar_t()
{
	test_itc6c77uw485667y<wchar_t>();
}

// ==========================================================================

// BasicString& assign(BasicString&&) noexcept
template<typename TChar>
void test_r4si1rv5vyk18sqw()
{
	const auto src_string_1 = make_string<TChar>("##############################");
	const auto src_string_2 = make_string<TChar>("str");

	auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	auto string_2 = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};

	const auto& string = string_1.assign(std::move(string_2));

	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == 3 &&
		data[0] == 's' &&
		data[1] == 't' &&
		data[2] == 'r' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_r4si1rv5vyk18sqw_char()
{
	test_r4si1rv5vyk18sqw<char>();
}

void test_r4si1rv5vyk18sqw_wchar_t()
{
	test_r4si1rv5vyk18sqw<wchar_t>();
}

// ==========================================================================

// BasicString& assign(BasicStringView<Char>)
template<typename TChar>
void test_jdc52wzr9lwfglpn()
{
	const auto src_string_1 = make_string<TChar>("##############################");
	const auto src_string_2 = make_string<TChar>("str");

	auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};

	const auto string_view = bstone::BasicStringView<TChar>{
		src_string_2.c_str(),
		static_cast<std::intptr_t>(src_string_2.size())};

	const auto& string = string_1.assign(string_view);

	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == 3 &&
		data[0] == 's' &&
		data[1] == 't' &&
		data[2] == 'r' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_jdc52wzr9lwfglpn_char()
{
	test_jdc52wzr9lwfglpn<char>();
}

void test_jdc52wzr9lwfglpn_wchar_t()
{
	test_jdc52wzr9lwfglpn<wchar_t>();
}

// ==========================================================================

// BasicString& assign(TIter, TIter)
// Random access iterator.
template<typename TChar>
void test_b8b64tyi0ymosqh2()
{
	const auto src_string_1 = make_string<TChar>("##############################");
	const auto src_string_2 = make_string<TChar>("str");

	auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto& string = string_1.assign(src_string_2.cbegin(), src_string_2.cend());

	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == 3 &&
		data[0] == 's' &&
		data[1] == 't' &&
		data[2] == 'r' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_b8b64tyi0ymosqh2_char()
{
	test_b8b64tyi0ymosqh2<char>();
}

void test_b8b64tyi0ymosqh2_wchar_t()
{
	test_b8b64tyi0ymosqh2<wchar_t>();
}

// ==========================================================================

// BasicString& assign(TIter, TIter)
// Non random access iterator.
template<typename TChar>
void test_ms657yc4w46xd9hn()
{
	const auto src_string = make_string<TChar>("##############################");
	const auto src_list = make_char_list<TChar>("str");

	auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto& string = string_1.assign(src_list.cbegin(), src_list.cend());

	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == 3 &&
		data[0] == 's' &&
		data[1] == 't' &&
		data[2] == 'r' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_ms657yc4w46xd9hn_char()
{
	test_ms657yc4w46xd9hn<char>();
}

void test_ms657yc4w46xd9hn_wchar_t()
{
	test_ms657yc4w46xd9hn<wchar_t>();
}

// ==========================================================================

// BasicString& assign(std::initializer_list<Char>)
template<typename TChar>
void test_2uc5kzilw6hxwpap()
{
	const auto src_string = make_string<TChar>("##############################");

	auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto& string = string_1.assign({TChar{'s'}, TChar{'t'}, TChar{'r'}});

	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == 3 &&
		data[0] == 's' &&
		data[1] == 't' &&
		data[2] == 'r' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_2uc5kzilw6hxwpap_char()
{
	test_2uc5kzilw6hxwpap<char>();
}

void test_2uc5kzilw6hxwpap_wchar_t()
{
	test_2uc5kzilw6hxwpap<wchar_t>();
}

// ==========================================================================

// BasicString& append(Char)
template<typename TChar>
void test_zl84ogc7998w799u()
{
	const auto src_string = make_string<TChar>("st");

	auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto& string = string_1.append(TChar{'r'});

	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == 3 &&
		data[0] == 's' &&
		data[1] == 't' &&
		data[2] == 'r' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_zl84ogc7998w799u_char()
{
	test_zl84ogc7998w799u<char>();
}

void test_zl84ogc7998w799u_wchar_t()
{
	test_zl84ogc7998w799u<wchar_t>();
}

// ==========================================================================

// BasicString& append(Char, std::intptr_t)
template<typename TChar>
void test_cvp9cybqg2jg0ap2()
{
	const auto src_string = make_string<TChar>("st");

	auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto& string = string_1.append(TChar{'r'}, 2);

	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == 4 &&
		data[0] == 's' &&
		data[1] == 't' &&
		data[2] == 'r' &&
		data[3] == 'r' &&
		data[4] == '\0';

	tester.check(is_valid);
}

void test_cvp9cybqg2jg0ap2_char()
{
	test_cvp9cybqg2jg0ap2<char>();
}

void test_cvp9cybqg2jg0ap2_wchar_t()
{
	test_cvp9cybqg2jg0ap2<wchar_t>();
}

// ==========================================================================

// BasicString& append(const Char*)
template<typename TChar>
void test_koa3ubk2qqe5gi9l()
{
	const auto src_string_1 = make_string<TChar>("s");
	const auto src_string_2 = make_string<TChar>("tr");

	auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto& string = string_1.append(src_string_2.c_str());

	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == 3 &&
		data[0] == 's' &&
		data[1] == 't' &&
		data[2] == 'r' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_koa3ubk2qqe5gi9l_char()
{
	test_koa3ubk2qqe5gi9l<char>();
}

void test_koa3ubk2qqe5gi9l_wchar_t()
{
	test_koa3ubk2qqe5gi9l<wchar_t>();
}

// ==========================================================================

// BasicString& append(const Char*, std::intptr_t)
template<typename TChar>
void test_h696ba837cicyicx()
{
	const auto src_string_1 = make_string<TChar>("s");
	const auto src_string_2 = make_string<TChar>("string");

	auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto& string = string_1.append(src_string_2.c_str() + 1, 2);

	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == 3 &&
		data[0] == 's' &&
		data[1] == 't' &&
		data[2] == 'r' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_h696ba837cicyicx_char()
{
	test_h696ba837cicyicx<char>();
}

void test_h696ba837cicyicx_wchar_t()
{
	test_h696ba837cicyicx<wchar_t>();
}

// ==========================================================================

// BasicString& append(const BasicString&)
template<typename TChar>
void test_9d0v9x5hdpaz3u7c()
{
	const auto src_string_1 = make_string<TChar>("s");
	const auto src_string_2 = make_string<TChar>("tr");

	auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};

	const auto& string = string_1.append(string_2);

	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == 3 &&
		data[0] == 's' &&
		data[1] == 't' &&
		data[2] == 'r' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_9d0v9x5hdpaz3u7c_char()
{
	test_9d0v9x5hdpaz3u7c<char>();
}

void test_9d0v9x5hdpaz3u7c_wchar_t()
{
	test_9d0v9x5hdpaz3u7c<wchar_t>();
}

// ==========================================================================

// BasicString& append(const BasicString&, std::intptr_t)
template<typename TChar>
void test_6z9ek2tengpxfrgl()
{
	const auto src_string_1 = make_string<TChar>("s");
	const auto src_string_2 = make_string<TChar>("str");

	auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};

	const auto& string = string_1.append(string_2, 1);

	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == 3 &&
		data[0] == 's' &&
		data[1] == 't' &&
		data[2] == 'r' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_6z9ek2tengpxfrgl_char()
{
	test_6z9ek2tengpxfrgl<char>();
}

void test_6z9ek2tengpxfrgl_wchar_t()
{
	test_6z9ek2tengpxfrgl<wchar_t>();
}

// ==========================================================================

// BasicString& append(const BasicString& rhs, std::intptr_t, std::intptr_t)
template<typename TChar>
void test_0l93gfiuux94hbu6()
{
	const auto src_string_1 = make_string<TChar>("s");
	const auto src_string_2 = make_string<TChar>("string");

	auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};

	const auto& string = string_1.append(string_2, 1, 2);

	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == 3 &&
		data[0] == 's' &&
		data[1] == 't' &&
		data[2] == 'r' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_0l93gfiuux94hbu6_char()
{
	test_0l93gfiuux94hbu6<char>();
}

void test_0l93gfiuux94hbu6_wchar_t()
{
	test_0l93gfiuux94hbu6<wchar_t>();
}

// ==========================================================================

// BasicString& append(BasicStringView<Char>)
template<typename TChar>
void test_39zmhpxm33s54k6l()
{
	const auto src_string_1 = make_string<TChar>("s");
	const auto src_string_2 = make_string<TChar>("tr");

	auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};

	const auto string_view = bstone::BasicStringView<TChar>{
		src_string_2.c_str(),
		static_cast<std::intptr_t>(src_string_2.size())};

	const auto& string = string_1.append(string_view);

	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == 3 &&
		data[0] == 's' &&
		data[1] == 't' &&
		data[2] == 'r' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_39zmhpxm33s54k6l_char()
{
	test_39zmhpxm33s54k6l<char>();
}

void test_39zmhpxm33s54k6l_wchar_t()
{
	test_39zmhpxm33s54k6l<wchar_t>();
}

// ==========================================================================

// BasicString& append(TIter, TIter)
// Random access iterator.
template<typename TChar>
void test_35lli5zs2siizeqt()
{
	const auto src_string = make_string<TChar>("str");

	auto string_1 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto& string = string_1.append(src_string.cbegin(), src_string.cend());

	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == 3 &&
		data[0] == 's' &&
		data[1] == 't' &&
		data[2] == 'r' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_35lli5zs2siizeqt_char()
{
	test_35lli5zs2siizeqt<char>();
}

void test_35lli5zs2siizeqt_wchar_t()
{
	test_35lli5zs2siizeqt<wchar_t>();
}

// ==========================================================================

// BasicString& append(TIter, TIter)
// Non random access iterator.
template<typename TChar>
void test_62hho616a8ahj1yg()
{
	const auto src_list = make_char_list<TChar>("str");

	auto string_1 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto& string = string_1.append(src_list.cbegin(), src_list.cend());

	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == 3 &&
		data[0] == 's' &&
		data[1] == 't' &&
		data[2] == 'r' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_62hho616a8ahj1yg_char()
{
	test_62hho616a8ahj1yg<char>();
}

void test_62hho616a8ahj1yg_wchar_t()
{
	test_62hho616a8ahj1yg<wchar_t>();
}

// ==========================================================================

// BasicString& append(std::initializer_list<Char>)
template<typename TChar>
void test_vloq4kjrkvfb8i87()
{
	auto string_1 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto& string = string_1.append({TChar{'s'}, TChar{'t'}, TChar{'r'}});

	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == 3 &&
		data[0] == 's' &&
		data[1] == 't' &&
		data[2] == 'r' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_vloq4kjrkvfb8i87_char()
{
	test_vloq4kjrkvfb8i87<char>();
}

void test_vloq4kjrkvfb8i87_wchar_t()
{
	test_vloq4kjrkvfb8i87<wchar_t>();
}

// ==========================================================================

// BasicString& insert(std::intptr_t, Char);
template<typename TChar>
void test_5j7de12j4ul2qy4j()
{
	const auto src_string = make_string<TChar>("11");

	//
	auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto& string_1_1 = string_1.insert(0, TChar{'2'});
	const auto data_1 = string_1_1.get_data();
	const auto size_1 = string_1_1.get_size();

	const auto is_valid_1 =
		size_1 == 3 &&
		data_1[0] == '2' &&
		data_1[1] == '1' &&
		data_1[2] == '1' &&
		data_1[3] == '\0';

	//
	auto string_2 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto& string_2_1 = string_2.insert(1, TChar{'2'});
	const auto data_2 = string_2_1.get_data();
	const auto size_2 = string_2_1.get_size();

	const auto is_valid_2 =
		size_2 == 3 &&
		data_2[0] == '1' &&
		data_2[1] == '2' &&
		data_2[2] == '1' &&
		data_2[3] == '\0';

	//
	auto string_3 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto& string_3_1 = string_3.insert(2, TChar{'2'});
	const auto data_3 = string_3_1.get_data();
	const auto size_3 = string_3_1.get_size();

	const auto is_valid_3 =
		size_3 == 3 &&
		data_3[0] == '1' &&
		data_3[1] == '1' &&
		data_3[2] == '2' &&
		data_3[3] == '\0';

	//
	const auto is_valid = is_valid_1 && is_valid_2 && is_valid_3;

	tester.check(is_valid);
}

void test_5j7de12j4ul2qy4j_char()
{
	test_5j7de12j4ul2qy4j<char>();
}

void test_5j7de12j4ul2qy4j_wchar_t()
{
	test_5j7de12j4ul2qy4j<wchar_t>();
}

// ==========================================================================

// BasicString& insert(std::intptr_t, Char, std::intptr_t)
template<typename TChar>
void test_x701f3h1mn7dyshg()
{
	const auto src_string = make_string<TChar>("11");

	//
	auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto& string_1_1 = string_1.insert(0, TChar{'2'}, 2);
	const auto data_1 = string_1_1.get_data();
	const auto size_1 = string_1_1.get_size();

	const auto is_valid_1 =
		size_1 == 4 &&
		data_1[0] == '2' &&
		data_1[1] == '2' &&
		data_1[2] == '1' &&
		data_1[3] == '1' &&
		data_1[4] == '\0';

	//
	auto string_2 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto& string_2_1 = string_2.insert(1, TChar{'2'}, 2);
	const auto data_2 = string_2_1.get_data();
	const auto size_2 = string_2_1.get_size();

	const auto is_valid_2 =
		size_2 == 4 &&
		data_2[0] == '1' &&
		data_2[1] == '2' &&
		data_2[2] == '2' &&
		data_2[3] == '1' &&
		data_2[4] == '\0';

	//
	auto string_3 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto& string_3_1 = string_3.insert(2, TChar{'2'}, 2);
	const auto data_3 = string_3_1.get_data();
	const auto size_3 = string_3_1.get_size();

	const auto is_valid_3 =
		size_3 == 4 &&
		data_3[0] == '1' &&
		data_3[1] == '1' &&
		data_3[2] == '2' &&
		data_3[3] == '2' &&
		data_3[4] == '\0';

	//
	const auto is_valid = is_valid_1 && is_valid_2 && is_valid_3;

	tester.check(is_valid);
}

void test_x701f3h1mn7dyshg_char()
{
	test_x701f3h1mn7dyshg<char>();
}

void test_x701f3h1mn7dyshg_wchar_t()
{
	test_x701f3h1mn7dyshg<wchar_t>();
}

// ==========================================================================

// BasicString& insert(std::intptr_t, const Char*)
template<typename TChar>
void test_pn23tndg5952a41h()
{
	const auto src_string_1 = make_string<TChar>("11");
	const auto src_string_2 = make_string<TChar>("22");

	//
	auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto& string_1_1 = string_1.insert(0, src_string_2.c_str());
	const auto data_1 = string_1_1.get_data();
	const auto size_1 = string_1_1.get_size();

	const auto is_valid_1 =
		size_1 == 4 &&
		data_1[0] == '2' &&
		data_1[1] == '2' &&
		data_1[2] == '1' &&
		data_1[3] == '1' &&
		data_1[4] == '\0';

	//
	auto string_2 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto& string_2_1 = string_2.insert(1, src_string_2.c_str());
	const auto data_2 = string_2_1.get_data();
	const auto size_2 = string_2_1.get_size();

	const auto is_valid_2 =
		size_2 == 4 &&
		data_2[0] == '1' &&
		data_2[1] == '2' &&
		data_2[2] == '2' &&
		data_2[3] == '1' &&
		data_2[4] == '\0';

	//
	auto string_3 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto& string_3_1 = string_3.insert(2, src_string_2.c_str());
	const auto data_3 = string_3_1.get_data();
	const auto size_3 = string_3_1.get_size();

	const auto is_valid_3 =
		size_3 == 4 &&
		data_3[0] == '1' &&
		data_3[1] == '1' &&
		data_3[2] == '2' &&
		data_3[3] == '2' &&
		data_3[4] == '\0';

	//
	const auto is_valid = is_valid_1 && is_valid_2 && is_valid_3;

	tester.check(is_valid);
}

void test_pn23tndg5952a41h_char()
{
	test_pn23tndg5952a41h<char>();
}

void test_pn23tndg5952a41h_wchar_t()
{
	test_pn23tndg5952a41h<wchar_t>();
}

// ==========================================================================

// BasicString& insert(std::intptr_t, const Char*, std::intptr_t)
template<typename TChar>
void test_95sf0vox5maogpi6()
{
	const auto src_string_1 = make_string<TChar>("11");
	const auto src_string_2 = make_string<TChar>("222");

	//
	auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto& string_1_1 = string_1.insert(0, src_string_2.c_str(), 2);
	const auto data_1 = string_1_1.get_data();
	const auto size_1 = string_1_1.get_size();

	const auto is_valid_1 =
		size_1 == 4 &&
		data_1[0] == '2' &&
		data_1[1] == '2' &&
		data_1[2] == '1' &&
		data_1[3] == '1' &&
		data_1[4] == '\0';

	//
	auto string_2 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto& string_2_1 = string_2.insert(1, src_string_2.c_str(), 2);
	const auto data_2 = string_2_1.get_data();
	const auto size_2 = string_2_1.get_size();

	const auto is_valid_2 =
		size_2 == 4 &&
		data_2[0] == '1' &&
		data_2[1] == '2' &&
		data_2[2] == '2' &&
		data_2[3] == '1' &&
		data_2[4] == '\0';

	//
	auto string_3 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto& string_3_1 = string_3.insert(2, src_string_2.c_str(), 2);
	const auto data_3 = string_3_1.get_data();
	const auto size_3 = string_3_1.get_size();

	const auto is_valid_3 =
		size_3 == 4 &&
		data_3[0] == '1' &&
		data_3[1] == '1' &&
		data_3[2] == '2' &&
		data_3[3] == '2' &&
		data_3[4] == '\0';

	//
	const auto is_valid = is_valid_1 && is_valid_2 && is_valid_3;

	tester.check(is_valid);
}

void test_95sf0vox5maogpi6_char()
{
	test_95sf0vox5maogpi6<char>();
}

void test_95sf0vox5maogpi6_wchar_t()
{
	test_95sf0vox5maogpi6<wchar_t>();
}

// ==========================================================================

// BasicString& insert(std::intptr_t, const BasicString&)
template<typename TChar>
void test_k4en302xt2uxhq9v()
{
	const auto src_string_1 = make_string<TChar>("11");
	const auto src_string_2 = make_string<TChar>("22");

	//
	const auto substring = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};

	//
	auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto& string_1_1 = string_1.insert(0, substring);
	const auto data_1 = string_1_1.get_data();
	const auto size_1 = string_1_1.get_size();

	const auto is_valid_1 =
		size_1 == 4 &&
		data_1[0] == '2' &&
		data_1[1] == '2' &&
		data_1[2] == '1' &&
		data_1[3] == '1' &&
		data_1[4] == '\0';

	//
	auto string_2 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto& string_2_1 = string_2.insert(1, substring);
	const auto data_2 = string_2_1.get_data();
	const auto size_2 = string_2_1.get_size();

	const auto is_valid_2 =
		size_2 == 4 &&
		data_2[0] == '1' &&
		data_2[1] == '2' &&
		data_2[2] == '2' &&
		data_2[3] == '1' &&
		data_2[4] == '\0';

	//
	auto string_3 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto& string_3_1 = string_3.insert(2, substring);
	const auto data_3 = string_3_1.get_data();
	const auto size_3 = string_3_1.get_size();

	const auto is_valid_3 =
		size_3 == 4 &&
		data_3[0] == '1' &&
		data_3[1] == '1' &&
		data_3[2] == '2' &&
		data_3[3] == '2' &&
		data_3[4] == '\0';

	//
	const auto is_valid = is_valid_1 && is_valid_2 && is_valid_3;

	tester.check(is_valid);
}

void test_k4en302xt2uxhq9v_char()
{
	test_k4en302xt2uxhq9v<char>();
}

void test_k4en302xt2uxhq9v_wchar_t()
{
	test_k4en302xt2uxhq9v<wchar_t>();
}

// ==========================================================================

// BasicString& insert(std::intptr_t, const BasicString&, std::intptr_t)
template<typename TChar>
void test_e3fkzl3fw2b4d26p()
{
	const auto src_string_1 = make_string<TChar>("11");
	const auto src_string_2 = make_string<TChar>("###22");

	//
	const auto substring = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};

	//
	auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto& string_1_1 = string_1.insert(0, substring, 3);
	const auto data_1 = string_1_1.get_data();
	const auto size_1 = string_1_1.get_size();

	const auto is_valid_1 =
		size_1 == 4 &&
		data_1[0] == '2' &&
		data_1[1] == '2' &&
		data_1[2] == '1' &&
		data_1[3] == '1' &&
		data_1[4] == '\0';

	//
	auto string_2 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto& string_2_1 = string_2.insert(1, substring, 3);
	const auto data_2 = string_2_1.get_data();
	const auto size_2 = string_2_1.get_size();

	const auto is_valid_2 =
		size_2 == 4 &&
		data_2[0] == '1' &&
		data_2[1] == '2' &&
		data_2[2] == '2' &&
		data_2[3] == '1' &&
		data_2[4] == '\0';

	//
	auto string_3 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto& string_3_1 = string_3.insert(2, substring, 3);
	const auto data_3 = string_3_1.get_data();
	const auto size_3 = string_3_1.get_size();

	const auto is_valid_3 =
		size_3 == 4 &&
		data_3[0] == '1' &&
		data_3[1] == '1' &&
		data_3[2] == '2' &&
		data_3[3] == '2' &&
		data_3[4] == '\0';

	//
	const auto is_valid = is_valid_1 && is_valid_2 && is_valid_3;

	tester.check(is_valid);
}

void test_e3fkzl3fw2b4d26p_char()
{
	test_e3fkzl3fw2b4d26p<char>();
}

void test_e3fkzl3fw2b4d26p_wchar_t()
{
	test_e3fkzl3fw2b4d26p<wchar_t>();
}

// ==========================================================================

// BasicString& insert(std::intptr_t, const BasicString&, std::intptr_t, std::intptr_t)
template<typename TChar>
void test_hgcmuzkrgrih8bw6()
{
	const auto src_string_1 = make_string<TChar>("11");
	const auto src_string_2 = make_string<TChar>("###22***");

	//
	const auto substring = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};

	//
	auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto& string_1_1 = string_1.insert(0, substring, 3, 2);
	const auto data_1 = string_1_1.get_data();
	const auto size_1 = string_1_1.get_size();

	const auto is_valid_1 =
		size_1 == 4 &&
		data_1[0] == '2' &&
		data_1[1] == '2' &&
		data_1[2] == '1' &&
		data_1[3] == '1' &&
		data_1[4] == '\0';

	//
	auto string_2 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto& string_2_1 = string_2.insert(1, substring, 3, 2);
	const auto data_2 = string_2_1.get_data();
	const auto size_2 = string_2_1.get_size();

	const auto is_valid_2 =
		size_2 == 4 &&
		data_2[0] == '1' &&
		data_2[1] == '2' &&
		data_2[2] == '2' &&
		data_2[3] == '1' &&
		data_2[4] == '\0';

	//
	auto string_3 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto& string_3_1 = string_3.insert(2, substring, 3, 2);
	const auto data_3 = string_3_1.get_data();
	const auto size_3 = string_3_1.get_size();

	const auto is_valid_3 =
		size_3 == 4 &&
		data_3[0] == '1' &&
		data_3[1] == '1' &&
		data_3[2] == '2' &&
		data_3[3] == '2' &&
		data_3[4] == '\0';

	//
	const auto is_valid = is_valid_1 && is_valid_2 && is_valid_3;

	tester.check(is_valid);
}

void test_hgcmuzkrgrih8bw6_char()
{
	test_hgcmuzkrgrih8bw6<char>();
}

void test_hgcmuzkrgrih8bw6_wchar_t()
{
	test_hgcmuzkrgrih8bw6<wchar_t>();
}

// ==========================================================================

// BasicString& insert(std::intptr_t, BasicStringView<Char>)
template<typename TChar>
void test_1jhbg15jmysnvbnj()
{
	const auto src_string_1 = make_string<TChar>("11");
	const auto src_string_2 = make_string<TChar>("###22***");

	//
	const auto substring = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};
	const auto substring_view = bstone::BasicStringView<TChar>{substring.get_data() + 3, 2};

	//
	auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto& string_1_1 = string_1.insert(0, substring_view);
	const auto data_1 = string_1_1.get_data();
	const auto size_1 = string_1_1.get_size();

	const auto is_valid_1 =
		size_1 == 4 &&
		data_1[0] == '2' &&
		data_1[1] == '2' &&
		data_1[2] == '1' &&
		data_1[3] == '1' &&
		data_1[4] == '\0';

	//
	auto string_2 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto& string_2_1 = string_2.insert(1, substring, 3, 2);
	const auto data_2 = string_2_1.get_data();
	const auto size_2 = string_2_1.get_size();

	const auto is_valid_2 =
		size_2 == 4 &&
		data_2[0] == '1' &&
		data_2[1] == '2' &&
		data_2[2] == '2' &&
		data_2[3] == '1' &&
		data_2[4] == '\0';

	//
	auto string_3 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto& string_3_1 = string_3.insert(2, substring, 3, 2);
	const auto data_3 = string_3_1.get_data();
	const auto size_3 = string_3_1.get_size();

	const auto is_valid_3 =
		size_3 == 4 &&
		data_3[0] == '1' &&
		data_3[1] == '1' &&
		data_3[2] == '2' &&
		data_3[3] == '2' &&
		data_3[4] == '\0';

	//
	const auto is_valid = is_valid_1 && is_valid_2 && is_valid_3;

	tester.check(is_valid);
}

void test_1jhbg15jmysnvbnj_char()
{
	test_1jhbg15jmysnvbnj<char>();
}

void test_1jhbg15jmysnvbnj_wchar_t()
{
	test_1jhbg15jmysnvbnj<wchar_t>();
}

// ==========================================================================

// Char* insert(TDstIter, TSrcIter, TSrcIter)
// Random access iterator.
template<typename TChar>
void test_b9w1u8m6p7cg9c1k()
{
	const auto src_string_1 = make_string<TChar>("11");
	const auto src_string_2 = make_string<TChar>("22");

	//
	auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto result_1 = string_1.insert(string_1.begin(), src_string_2.cbegin(), src_string_2.end());
	const auto data_1 = string_1.get_data();
	const auto size_1 = string_1.get_size();
	const auto iter_1 = string_1.begin();

	const auto is_valid_1 =
		size_1 == 4 &&
		result_1 == iter_1 &&
		data_1[0] == '2' &&
		data_1[1] == '2' &&
		data_1[2] == '1' &&
		data_1[3] == '1' &&
		data_1[4] == '\0';

	//
	auto string_2 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto result_2 = string_2.insert(string_2.begin() + 1, src_string_2.cbegin(), src_string_2.end());
	const auto data_2 = string_2.get_data();
	const auto size_2 = string_2.get_size();
	const auto iter_2 = string_2.begin() + 1;

	const auto is_valid_2 =
		size_2 == 4 &&
		result_2 == iter_2 &&
		data_2[0] == '1' &&
		data_2[1] == '2' &&
		data_2[2] == '2' &&
		data_2[3] == '1' &&
		data_2[4] == '\0';

	//
	auto string_3 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto result_3 = string_3.insert(string_3.begin() + 2, src_string_2.cbegin(), src_string_2.end());
	const auto data_3 = string_3.get_data();
	const auto size_3 = string_3.get_size();
	const auto iter_3 = string_3.begin() + 2;

	const auto is_valid_3 =
		size_3 == 4 &&
		result_3 == iter_3 &&
		data_3[0] == '1' &&
		data_3[1] == '1' &&
		data_3[2] == '2' &&
		data_3[3] == '2' &&
		data_3[4] == '\0';

	//
	const auto is_valid = is_valid_1 && is_valid_2 && is_valid_3;

	tester.check(is_valid);
}

void test_b9w1u8m6p7cg9c1k_char()
{
	test_b9w1u8m6p7cg9c1k<char>();
}

void test_b9w1u8m6p7cg9c1k_wchar_t()
{
	test_b9w1u8m6p7cg9c1k<wchar_t>();
}

// ==========================================================================

// Char* insert(TDstIter, TSrcIter, TSrcIter)
// Not random access iterator.
template<typename TChar>
void test_alb7fc11vpuztxaj()
{
	const auto src_string = make_string<TChar>("11");
	const auto src_list = make_char_list<TChar>("22");

	//
	auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto result_1 = string_1.insert(string_1.begin(), src_list.cbegin(), src_list.end());
	const auto data_1 = string_1.get_data();
	const auto size_1 = string_1.get_size();
	const auto iter_1 = string_1.begin();

	const auto is_valid_1 =
		size_1 == 4 &&
		result_1 == iter_1 &&
		data_1[0] == '2' &&
		data_1[1] == '2' &&
		data_1[2] == '1' &&
		data_1[3] == '1' &&
		data_1[4] == '\0';

	//
	auto string_2 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto result_2 = string_2.insert(string_2.begin() + 1, src_list.cbegin(), src_list.end());
	const auto data_2 = string_2.get_data();
	const auto size_2 = string_2.get_size();
	const auto iter_2 = string_2.begin() + 1;

	const auto is_valid_2 =
		size_2 == 4 &&
		result_2 == iter_2 &&
		data_2[0] == '1' &&
		data_2[1] == '2' &&
		data_2[2] == '2' &&
		data_2[3] == '1' &&
		data_2[4] == '\0';

	//
	auto string_3 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto result_3 = string_3.insert(string_3.begin() + 2, src_list.cbegin(), src_list.end());
	const auto data_3 = string_3.get_data();
	const auto size_3 = string_3.get_size();
	const auto iter_3 = string_3.begin() + 2;

	const auto is_valid_3 =
		size_3 == 4 &&
		result_3 == iter_3 &&
		data_3[0] == '1' &&
		data_3[1] == '1' &&
		data_3[2] == '2' &&
		data_3[3] == '2' &&
		data_3[4] == '\0';

	//
	const auto is_valid = is_valid_1 && is_valid_2 && is_valid_3;

	tester.check(is_valid);
}

void test_alb7fc11vpuztxaj_char()
{
	test_alb7fc11vpuztxaj<char>();
}

void test_alb7fc11vpuztxaj_wchar_t()
{
	test_alb7fc11vpuztxaj<wchar_t>();
}

// ==========================================================================

// Char* insert(TDstIter, std::initializer_list<Char>);
template<typename TChar>
void test_hkk649lez4ahi368()
{
	const auto src_string_1 = make_string<TChar>("11");

	//
	auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto result_1 = string_1.insert(string_1.begin(), {TChar{'2'}, TChar{'2'}});
	const auto data_1 = string_1.get_data();
	const auto size_1 = string_1.get_size();
	const auto iter_1 = string_1.begin();

	const auto is_valid_1 =
		size_1 == 4 &&
		result_1 == iter_1 &&
		data_1[0] == '2' &&
		data_1[1] == '2' &&
		data_1[2] == '1' &&
		data_1[3] == '1' &&
		data_1[4] == '\0';

	//
	auto string_2 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto result_2 = string_2.insert(string_2.begin() + 1, {TChar{'2'}, TChar{'2'}});
	const auto data_2 = string_2.get_data();
	const auto size_2 = string_2.get_size();
	const auto iter_2 = string_2.begin() + 1;

	const auto is_valid_2 =
		size_2 == 4 &&
		result_2 == iter_2 &&
		data_2[0] == '1' &&
		data_2[1] == '2' &&
		data_2[2] == '2' &&
		data_2[3] == '1' &&
		data_2[4] == '\0';

	//
	auto string_3 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto result_3 = string_3.insert(string_3.begin() + 2, {TChar{'2'}, TChar{'2'}});
	const auto data_3 = string_3.get_data();
	const auto size_3 = string_3.get_size();
	const auto iter_3 = string_3.begin() + 2;

	const auto is_valid_3 =
		size_3 == 4 &&
		result_3 == iter_3 &&
		data_3[0] == '1' &&
		data_3[1] == '1' &&
		data_3[2] == '2' &&
		data_3[3] == '2' &&
		data_3[4] == '\0';

	//
	const auto is_valid = is_valid_1 && is_valid_2 && is_valid_3;

	tester.check(is_valid);
}

void test_hkk649lez4ahi368_char()
{
	test_hkk649lez4ahi368<char>();
}

void test_hkk649lez4ahi368_wchar_t()
{
	test_hkk649lez4ahi368<wchar_t>();
}

// ==========================================================================

// BasicString& erase(std::intptr_t)
template<typename TChar>
void test_13jut63zekqzr9xh()
{
	const auto src_string = make_string<TChar>("string");

	auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto& string = string_1.erase(3);
	const auto size = string.get_size();
	const auto data = string.get_data();

	const auto is_valid =
		size == 3 &&
		data[0] == 's' &&
		data[1] == 't' &&
		data[2] == 'r' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_13jut63zekqzr9xh_char()
{
	test_13jut63zekqzr9xh<char>();
}

void test_13jut63zekqzr9xh_wchar_t()
{
	test_13jut63zekqzr9xh<wchar_t>();
}

// ==========================================================================

// BasicString& erase(std::intptr_t, std::intptr_t)
template<typename TChar>
void test_o12sub2fiu7iosls()
{
	const auto src_string = make_string<TChar>("string");

	auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto& string = string_1.erase(1, 3);
	const auto size = string.get_size();
	const auto data = string.get_data();

	const auto is_valid =
		size == 3 &&
		data[0] == 's' &&
		data[1] == 'n' &&
		data[2] == 'g' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_o12sub2fiu7iosls_char()
{
	test_o12sub2fiu7iosls<char>();
}

void test_o12sub2fiu7iosls_wchar_t()
{
	test_o12sub2fiu7iosls<wchar_t>();
}

// ==========================================================================

// Char* erase(const Char*)
template<typename TChar>
void test_svff6q2nrw8nwy35()
{
	const auto src_string = make_string<TChar>("string");

	auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto result_iter = string.erase(string.begin() + 3);
	const auto size = string.get_size();
	const auto data = string.get_data();
	const auto iter = string.begin() + 3;

	const auto is_valid =
		size == 3 &&
		result_iter == iter &&
		data[0] == 's' &&
		data[1] == 't' &&
		data[2] == 'r' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_svff6q2nrw8nwy35_char()
{
	test_svff6q2nrw8nwy35<char>();
}

void test_svff6q2nrw8nwy35_wchar_t()
{
	test_svff6q2nrw8nwy35<wchar_t>();
}

// ==========================================================================

// Char* erase(const Char*, const Char*)
template<typename TChar>
void test_shz5467p4n3tugca()
{
	const auto src_string = make_string<TChar>("string");

	auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto result_iter = string.erase(string.begin() + 1, string.begin() + 4);
	const auto size = string.get_size();
	const auto data = string.get_data();
	const auto iter = string.begin() + 1;

	const auto is_valid =
		size == 3 &&
		result_iter == iter &&
		data[0] == 's' &&
		data[1] == 'n' &&
		data[2] == 'g' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_shz5467p4n3tugca_char()
{
	test_shz5467p4n3tugca<char>();
}

void test_shz5467p4n3tugca_wchar_t()
{
	test_shz5467p4n3tugca<wchar_t>();
}

// ==========================================================================

// void push_back(Char)
template<typename TChar>
void test_ltq0f177mhzv4bb9()
{
	const auto src_string = make_string<TChar>("st");

	auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	string.push_back(TChar{'r'});

	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == 3 &&
		data[0] == 's' &&
		data[1] == 't' &&
		data[2] == 'r' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_ltq0f177mhzv4bb9_char()
{
	test_ltq0f177mhzv4bb9<char>();
}

void test_ltq0f177mhzv4bb9_wchar_t()
{
	test_ltq0f177mhzv4bb9<wchar_t>();
}

// ==========================================================================

// Char pop_back()
template<typename TChar>
void test_eqdb9m29r80rt00i()
{
	const auto src_string = make_string<TChar>("str1");

	auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto ch = string.pop_back();
	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == 3 &&
		ch == '1' &&
		data[0] == 's' &&
		data[1] == 't' &&
		data[2] == 'r' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_eqdb9m29r80rt00i_char()
{
	test_eqdb9m29r80rt00i<char>();
}

void test_eqdb9m29r80rt00i_wchar_t()
{
	test_eqdb9m29r80rt00i<wchar_t>();
}

// ==========================================================================

// BasicString get_substring(std::intptr_t) const
template<typename TChar>
void test_r1lvuo2hj60v6aux()
{
	const auto src_string = make_string<TChar>("string");

	const auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto substring = string.get_substring(3);
	const auto data = substring.get_data();
	const auto size = substring.get_size();

	const auto is_valid =
		size == 3 &&
		data[0] == 'i' &&
		data[1] == 'n' &&
		data[2] == 'g' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_r1lvuo2hj60v6aux_char()
{
	test_r1lvuo2hj60v6aux<char>();
}

void test_r1lvuo2hj60v6aux_wchar_t()
{
	test_r1lvuo2hj60v6aux<wchar_t>();
}

// ==========================================================================

// BasicString get_substring(std::intptr_t, std::intptr_t) const
template<typename TChar>
void test_gysak99o3p50flj2()
{
	const auto src_string = make_string<TChar>("string");

	const auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto substring = string.get_substring(2, 3);
	const auto data = substring.get_data();
	const auto size = substring.get_size();

	const auto is_valid =
		size == 3 &&
		data[0] == 'r' &&
		data[1] == 'i' &&
		data[2] == 'n' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_gysak99o3p50flj2_char()
{
	test_gysak99o3p50flj2<char>();
}

void test_gysak99o3p50flj2_wchar_t()
{
	test_gysak99o3p50flj2<wchar_t>();
}

// ==========================================================================

// BasicStringView<Char> get_view() const noexcept
template<typename TChar>
void test_et6hqny7js6hgzhm()
{
	const auto src_string = make_string<TChar>("string");

	const auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto view = string.get_view().get_subview(3, 3);
	const auto data = view.get_data();
	const auto size = view.get_size();

	const auto is_valid =
		size == 3 &&
		data[0] == 'i' &&
		data[1] == 'n' &&
		data[2] == 'g' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_et6hqny7js6hgzhm_char()
{
	test_et6hqny7js6hgzhm<char>();
}

void test_et6hqny7js6hgzhm_wchar_t()
{
	test_et6hqny7js6hgzhm<wchar_t>();
}

// ==========================================================================

// BasicStringView<Char> get_view(std::intptr_t) const noexcept
template<typename TChar>
void test_34e1px4up6l6nbht()
{
	const auto src_string = make_string<TChar>("string");

	const auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto view = string.get_view().get_subview(3);
	const auto data = view.get_data();
	const auto size = view.get_size();

	const auto is_valid =
		size == 3 &&
		data[0] == 'i' &&
		data[1] == 'n' &&
		data[2] == 'g' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_34e1px4up6l6nbht_char()
{
	test_34e1px4up6l6nbht<char>();
}

void test_34e1px4up6l6nbht_wchar_t()
{
	test_34e1px4up6l6nbht<wchar_t>();
}

// ==========================================================================

// BasicStringView<Char> get_view(std::intptr_t, std::intptr_t) const noexcept
template<typename TChar>
void test_etkmnso9fu853ntt()
{
	const auto src_string = make_string<TChar>("string");

	const auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto view = string.get_view(3, 3);
	const auto data = view.get_data();
	const auto size = view.get_size();

	const auto is_valid =
		size == 3 &&
		data[0] == 'i' &&
		data[1] == 'n' &&
		data[2] == 'g' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_etkmnso9fu853ntt_char()
{
	test_etkmnso9fu853ntt<char>();
}

void test_etkmnso9fu853ntt_wchar_t()
{
	test_etkmnso9fu853ntt<wchar_t>();
}

// ==========================================================================

// operator BasicStringView<Char>() const noexcept
template<typename TChar>
void test_klf1ixu9tmzsl064()
{
	const auto src_string = make_string<TChar>("string");

	const auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto view = static_cast<bstone::BasicStringView<TChar>>(string).get_subview(3, 3);
	const auto data = view.get_data();
	const auto size = view.get_size();

	const auto is_valid =
		size == 3 &&
		data[0] == 'i' &&
		data[1] == 'n' &&
		data[2] == 'g' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_klf1ixu9tmzsl064_char()
{
	test_klf1ixu9tmzsl064<char>();
}

void test_klf1ixu9tmzsl064_wchar_t()
{
	test_klf1ixu9tmzsl064<wchar_t>();
}

// ==========================================================================

// int compare(const Char*) const
template<typename TChar>
void test_afjlv1efmzy4ti76()
{
	const auto src_string_1 = make_string<TChar>("string1");
	const auto src_string_2 = make_string<TChar>("ttring");
	const auto src_string_3 = make_string<TChar>("string11");

	const auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};
	const auto string_3 = bstone::BasicString<TChar>{src_string_3.c_str(), get_memory_resource()};

	const auto result_1 = string_1.compare(string_2.get_data()) < 0;
	const auto result_2 = string_2.compare(string_1.get_data()) > 0;
	const auto result_3 = string_1.compare(string_3.get_data()) < 0;
	const auto result_4 = string_3.compare(string_1.get_data()) > 0;
	const auto result_5 = string_1.compare(string_1.get_data()) == 0;

	const auto is_valid =
		result_1 &&
		result_2 &&
		result_3 &&
		result_4 &&
		result_5;

	tester.check(is_valid);
}

void test_afjlv1efmzy4ti76_char()
{
	test_afjlv1efmzy4ti76<char>();
}

void test_afjlv1efmzy4ti76_wchar_t()
{
	test_afjlv1efmzy4ti76<wchar_t>();
}

// ==========================================================================

// int compare(const Char*, std::intptr_t) const
template<typename TChar>
void test_rrp35kn244n393wo()
{
	const auto src_string_1 = make_string<TChar>("string1");
	const auto src_string_2 = make_string<TChar>("ttring");
	const auto src_string_3 = make_string<TChar>("string11");

	const auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};
	const auto string_3 = bstone::BasicString<TChar>{src_string_3.c_str(), get_memory_resource()};

	const auto result_1 = string_1.compare(string_2.get_data(), 1) < 0;
	const auto result_2 = string_2.compare(string_1.get_data(), 1) > 0;
	const auto result_3 = string_1.compare(string_3.get_data(), string_3.get_size()) < 0;
	const auto result_4 = string_3.compare(string_1.get_data(), string_1.get_size()) > 0;
	const auto result_5 = string_1.compare(string_1.get_data(), string_1.get_size()) == 0;

	const auto is_valid =
		result_1 &&
		result_2 &&
		result_3 &&
		result_4 &&
		result_5;

	tester.check(is_valid);
}

void test_rrp35kn244n393wo_char()
{
	test_rrp35kn244n393wo<char>();
}

void test_rrp35kn244n393wo_wchar_t()
{
	test_rrp35kn244n393wo<wchar_t>();
}

// ==========================================================================

// int compare(const BasicString&) const
template<typename TChar>
void test_6c11npvqkd8o2v3e()
{
	const auto src_string_1 = make_string<TChar>("string1");
	const auto src_string_2 = make_string<TChar>("ttring");
	const auto src_string_3 = make_string<TChar>("string11");

	const auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};
	const auto string_3 = bstone::BasicString<TChar>{src_string_3.c_str(), get_memory_resource()};

	const auto result_1 = string_1.compare(string_2) < 0;
	const auto result_2 = string_2.compare(string_1) > 0;
	const auto result_3 = string_1.compare(string_3) < 0;
	const auto result_4 = string_3.compare(string_1) > 0;
	const auto result_5 = string_1.compare(string_1) == 0;

	const auto is_valid =
		result_1 &&
		result_2 &&
		result_3 &&
		result_4 &&
		result_5;

	tester.check(is_valid);
}

void test_6c11npvqkd8o2v3e_char()
{
	test_6c11npvqkd8o2v3e<char>();
}

void test_6c11npvqkd8o2v3e_wchar_t()
{
	test_6c11npvqkd8o2v3e<wchar_t>();
}

// ==========================================================================

// int compare(BasicStringView<Char>) const noexcept
template<typename TChar>
void test_775yz1vluvocvro1()
{
	const auto src_string_1 = make_string<TChar>("string1");
	const auto src_string_2 = make_string<TChar>("ttring");
	const auto src_string_3 = make_string<TChar>("string11");

	const auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	const auto string_view_1 = string_1.get_view();

	const auto string_2 = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};
	const auto string_view_2 = string_2.get_view();

	const auto string_3 = bstone::BasicString<TChar>{src_string_3.c_str(), get_memory_resource()};
	const auto string_view_3 = string_3.get_view();

	const auto result_1 = string_1.compare(string_view_2) < 0;
	const auto result_2 = string_2.compare(string_view_1) > 0;
	const auto result_3 = string_1.compare(string_view_3) < 0;
	const auto result_4 = string_3.compare(string_view_1) > 0;
	const auto result_5 = string_1.compare(string_view_1) == 0;

	const auto is_valid =
		result_1 &&
		result_2 &&
		result_3 &&
		result_4 &&
		result_5;

	tester.check(is_valid);
}

void test_775yz1vluvocvro1_char()
{
	test_775yz1vluvocvro1<char>();
}

void test_775yz1vluvocvro1_wchar_t()
{
	test_775yz1vluvocvro1<wchar_t>();
}
// ==========================================================================

// std::intptr_t index_of(Char) const
template<typename TChar>
void test_7uq7rypocz7n0w3a()
{
	const auto src_string_1 = make_string<TChar>("string");

	const auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto result_1 = string_1.index_of(TChar{'i'}) == 3;
	const auto result_2 = string_1.index_of(TChar{'a'}) < 0;
	const auto result_3 = string_2.index_of(TChar{'a'}) < 0;

	const auto is_valid =
		result_1 &&
		result_2 &&
		result_3 &&
		true;

	tester.check(is_valid);
}

void test_7uq7rypocz7n0w3a_char()
{
	test_7uq7rypocz7n0w3a<char>();
}

void test_7uq7rypocz7n0w3a_wchar_t()
{
	test_7uq7rypocz7n0w3a<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_of(std::intptr_t, Char) const
template<typename TChar>
void test_z6br25ysjdr70112()
{
	const auto src_string = make_string<TChar>("string");

	const auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto result_1 = string_1.index_of(1, TChar{'i'}) == 3;
	const auto result_2 = string_1.index_of(4, TChar{'i'}) < 0;
	const auto result_3 = string_2.index_of(0, TChar{'i'}) < 0;

	const auto is_valid =
		result_1 &&
		result_2 &&
		result_3 &&
		true;

	tester.check(is_valid);
}

void test_z6br25ysjdr70112_char()
{
	test_z6br25ysjdr70112<char>();
}

void test_z6br25ysjdr70112_wchar_t()
{
	test_z6br25ysjdr70112<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_of(const Char*) const
template<typename TChar>
void test_kf0ahsbhxwtfbrzm()
{
	const auto src_string = make_string<TChar>("ABC ABCDAB ABCDABCDABDE");
	const auto src_substring_1 = make_string<TChar>("ABCDABD");
	const auto src_substring_2 = make_string<TChar>("ABCDABd");
	const auto src_substring_3 = make_string<TChar>("");
	const auto src_substring_4 = make_string<TChar>("ABC ABCDAB ABCDABCDABDE!");
	const auto src_substring_5 = make_string<TChar>("C");
	const auto src_substring_6 = make_string<TChar>("DA");

	const auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto result_1 = string_1.index_of(src_substring_1.c_str()) == 15;
	const auto result_2 = string_1.index_of(src_substring_2.c_str()) < 0;
	const auto result_3 = string_1.index_of(src_substring_3.c_str()) == 0;
	const auto result_4 = string_1.index_of(src_substring_4.c_str()) < 0;
	const auto result_5 = string_2.index_of(src_substring_1.c_str()) < 0;
	const auto result_6 = string_1.index_of(src_string.c_str()) == 0;
	const auto result_7 = string_1.index_of(src_substring_5.c_str()) == 2;
	const auto result_8 = string_1.index_of(src_substring_6.c_str()) == 7;

	const auto is_valid =
		result_1 &&
		result_2 &&
		result_3 &&
		result_4 &&
		result_5 &&
		result_6 &&
		result_7 &&
		result_8 &&
		true;

	tester.check(is_valid);
}

void test_kf0ahsbhxwtfbrzm_char()
{
	test_kf0ahsbhxwtfbrzm<char>();
}

void test_kf0ahsbhxwtfbrzm_wchar_t()
{
	test_kf0ahsbhxwtfbrzm<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_of(const Char*, std::intptr_t) const
template<typename TChar>
void test_qznvspfo1lllkomd()
{
	const auto src_string = make_string<TChar>("ABC ABCDAB ABCDABCDABDE");
	const auto src_substring_1 = make_string<TChar>("ABCDABD!");
	const auto src_substring_2 = make_string<TChar>("ABCDABd!");
	const auto src_substring_3 = make_string<TChar>("!");
	const auto src_substring_4 = make_string<TChar>("ABC ABCDAB ABCDABCDABDE!!");
	const auto src_substring_5 = make_string<TChar>("C!");
	const auto src_substring_6 = make_string<TChar>("DA!");

	const auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto result_1 = string_1.index_of(src_substring_1.c_str(), 7) == 15;
	const auto result_2 = string_1.index_of(src_substring_2.c_str(), 7) < 0;
	const auto result_3 = string_1.index_of(src_substring_3.c_str(), 0) == 0;
	const auto result_4 = string_1.index_of(src_substring_4.c_str(), 24) < 0;
	const auto result_5 = string_2.index_of(src_substring_1.c_str(), 7) < 0;
	const auto result_6 = string_1.index_of(
		src_string.c_str(), static_cast<std::intptr_t>(src_string.size())) == 0;
	const auto result_7 = string_1.index_of(src_substring_5.c_str(), 1) == 2;
	const auto result_8 = string_1.index_of(src_substring_6.c_str(), 2) == 7;

	const auto is_valid =
		result_1 &&
		result_2 &&
		result_3 &&
		result_4 &&
		result_5 &&
		result_6 &&
		result_7 &&
		result_8 &&
		true;

	tester.check(is_valid);
}

void test_qznvspfo1lllkomd_char()
{
	test_qznvspfo1lllkomd<char>();
}

void test_qznvspfo1lllkomd_wchar_t()
{
	test_qznvspfo1lllkomd<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_of(std::intptr_t, const Char*) const
template<typename TChar>
void test_ebs57ypu63vcvpr2()
{
	const auto src_string = make_string<TChar>("ABC ABCDAB ABCDABCDABDE");
	const auto src_substring_1 = make_string<TChar>("ABCDABD");
	const auto src_substring_2 = make_string<TChar>("ABCDABd");
	const auto src_substring_3 = make_string<TChar>("");
	const auto src_substring_4 = make_string<TChar>("ABC ABCDAB ABCDABCDABDE!");
	const auto src_substring_5 = make_string<TChar>("C");
	const auto src_substring_6 = make_string<TChar>("DA");

	const auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto result_1 = string_1.index_of(1, src_substring_1.c_str()) == 15;
	const auto result_2 = string_1.index_of(1, src_substring_2.c_str()) < 0;
	const auto result_3 = string_1.index_of(1, src_substring_3.c_str()) == 1;
	const auto result_4 = string_1.index_of(0, src_substring_4.c_str()) < 0;
	const auto result_5 = string_2.index_of(0, src_substring_1.c_str()) < 0;
	const auto result_6 = string_1.index_of(0, src_string.c_str()) == 0;
	const auto result_7 = string_1.index_of(1, src_substring_5.c_str(), 1) == 2;
	const auto result_8 = string_1.index_of(1, src_substring_6.c_str(), 2) == 7;

	const auto is_valid =
		result_1 &&
		result_2 &&
		result_3 &&
		result_4 &&
		result_5 &&
		result_6 &&
		result_7 &&
		result_8 &&
		true;

	tester.check(is_valid);
}

void test_ebs57ypu63vcvpr2_char()
{
	test_ebs57ypu63vcvpr2<char>();
}

void test_ebs57ypu63vcvpr2_wchar_t()
{
	test_ebs57ypu63vcvpr2<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_of(std::intptr_t offset, const Char*, std::intptr_t) const
template<typename TChar>
void test_c8lkq6z8kkib4dri()
{
	const auto src_string = make_string<TChar>("ABC ABCDAB ABCDABCDABDE");
	const auto src_substring_1 = make_string<TChar>("ABCDABD!");
	const auto src_substring_2 = make_string<TChar>("ABCDABd!");
	const auto src_substring_3 = make_string<TChar>("!");
	const auto src_substring_4 = make_string<TChar>("ABC ABCDAB ABCDABCDABDE!!");
	const auto src_substring_5 = make_string<TChar>("C!");
	const auto src_substring_6 = make_string<TChar>("DA!");

	const auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto result_1 = string_1.index_of(1, src_substring_1.c_str(), 7) == 15;
	const auto result_2 = string_1.index_of(1, src_substring_2.c_str(), 7) < 0;
	const auto result_3 = string_1.index_of(1, src_substring_3.c_str(), 0) == 1;
	const auto result_4 = string_1.index_of(0, src_substring_4.c_str(), 24) < 0;
	const auto result_5 = string_2.index_of(0, src_substring_1.c_str(), 7) < 0;
	const auto result_6 = string_1.index_of(
		0, src_string.c_str(), static_cast<std::intptr_t>(src_string.size())) == 0;
	const auto result_7 = string_1.index_of(1, src_substring_5.c_str(), 1) == 2;
	const auto result_8 = string_1.index_of(1, src_substring_6.c_str(), 2) == 7;

	const auto is_valid =
		result_1 &&
		result_2 &&
		result_3 &&
		result_4 &&
		result_5 &&
		result_6 &&
		result_7 &&
		result_8 &&
		true;

	tester.check(is_valid);
}

void test_c8lkq6z8kkib4dri_char()
{
	test_c8lkq6z8kkib4dri<char>();
}

void test_c8lkq6z8kkib4dri_wchar_t()
{
	test_c8lkq6z8kkib4dri<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_of(const BasicString&) const
template<typename TChar>
void test_uz1gprkmx7nymomo()
{
	const auto src_string = make_string<TChar>("ABC ABCDAB ABCDABCDABDE");
	const auto src_substring_1 = make_string<TChar>("ABCDABD");
	const auto src_substring_2 = make_string<TChar>("ABCDABd");
	const auto src_substring_3 = make_string<TChar>("");
	const auto src_substring_4 = make_string<TChar>("ABC ABCDAB ABCDABCDABDE!");
	const auto src_substring_5 = make_string<TChar>("C");
	const auto src_substring_6 = make_string<TChar>("DA");

	const auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto substring_1 = bstone::BasicString<TChar>{src_substring_1.c_str(), get_memory_resource()};
	const auto substring_2 = bstone::BasicString<TChar>{src_substring_2.c_str(), get_memory_resource()};
	const auto substring_3 = bstone::BasicString<TChar>{src_substring_3.c_str(), get_memory_resource()};
	const auto substring_4 = bstone::BasicString<TChar>{src_substring_4.c_str(), get_memory_resource()};
	const auto substring_5 = bstone::BasicString<TChar>{src_substring_5.c_str(), get_memory_resource()};
	const auto substring_6 = bstone::BasicString<TChar>{src_substring_6.c_str(), get_memory_resource()};

	const auto result_1 = string_1.index_of(substring_1) == 15;
	const auto result_2 = string_1.index_of(substring_2) < 0;
	const auto result_3 = string_1.index_of(substring_3) == 0;
	const auto result_4 = string_1.index_of(substring_4) < 0;
	const auto result_5 = string_2.index_of(substring_1) < 0;
	const auto result_6 = string_1.index_of(string_1) == 0;
	const auto result_7 = string_1.index_of(substring_5) == 2;
	const auto result_8 = string_1.index_of(substring_6) == 7;

	const auto is_valid =
		result_1 &&
		result_2 &&
		result_3 &&
		result_4 &&
		result_5 &&
		result_6 &&
		result_7 &&
		result_8 &&
		true;

	tester.check(is_valid);
}

void test_uz1gprkmx7nymomo_char()
{
	test_uz1gprkmx7nymomo<char>();
}

void test_uz1gprkmx7nymomo_wchar_t()
{
	test_uz1gprkmx7nymomo<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_of(std::intptr_t, const BasicString&) const
template<typename TChar>
void test_ju4s6jmtwbq5pbgo()
{
	const auto src_string = make_string<TChar>("ABC ABCDAB ABCDABCDABDE");
	const auto src_substring_1 = make_string<TChar>("ABCDABD");
	const auto src_substring_2 = make_string<TChar>("ABCDABd");
	const auto src_substring_3 = make_string<TChar>("");
	const auto src_substring_4 = make_string<TChar>("ABC ABCDAB ABCDABCDABDE!");
	const auto src_substring_5 = make_string<TChar>("C");
	const auto src_substring_6 = make_string<TChar>("DA");

	const auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto substring_1 = bstone::BasicString<TChar>{src_substring_1.c_str(), get_memory_resource()};
	const auto substring_2 = bstone::BasicString<TChar>{src_substring_2.c_str(), get_memory_resource()};
	const auto substring_3 = bstone::BasicString<TChar>{src_substring_3.c_str(), get_memory_resource()};
	const auto substring_4 = bstone::BasicString<TChar>{src_substring_4.c_str(), get_memory_resource()};
	const auto substring_5 = bstone::BasicString<TChar>{src_substring_5.c_str(), get_memory_resource()};
	const auto substring_6 = bstone::BasicString<TChar>{src_substring_6.c_str(), get_memory_resource()};

	const auto result_1 = string_1.index_of(1, substring_1) == 15;
	const auto result_2 = string_1.index_of(1, substring_2) < 0;
	const auto result_3 = string_1.index_of(1, substring_3) == 1;
	const auto result_4 = string_1.index_of(0, substring_4) < 0;
	const auto result_5 = string_2.index_of(0, substring_1) < 0;
	const auto result_6 = string_1.index_of(0, string_1) == 0;
	const auto result_7 = string_1.index_of(1, substring_5) == 2;
	const auto result_8 = string_1.index_of(1, substring_6) == 7;

	const auto is_valid =
		result_1 &&
		result_2 &&
		result_3 &&
		result_4 &&
		result_5 &&
		result_6 &&
		result_7 &&
		result_8 &&
		true;

	tester.check(is_valid);
}

void test_ju4s6jmtwbq5pbgo_char()
{
	test_ju4s6jmtwbq5pbgo<char>();
}

void test_ju4s6jmtwbq5pbgo_wchar_t()
{
	test_ju4s6jmtwbq5pbgo<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_of(const BasicStringView<Char>&) const noexcept
template<typename TChar>
void test_uddtqthzq34c7mn6()
{
	const auto src_string = make_string<TChar>("ABC ABCDAB ABCDABCDABDE");
	const auto src_substring_1 = make_string<TChar>("ABCDABD");
	const auto src_substring_2 = make_string<TChar>("ABCDABd");
	const auto src_substring_3 = make_string<TChar>("");
	const auto src_substring_4 = make_string<TChar>("ABC ABCDAB ABCDABCDABDE!");
	const auto src_substring_5 = make_string<TChar>("C");
	const auto src_substring_6 = make_string<TChar>("DA");

	const auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto view_1 = string_1.get_view();

	const auto string_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto substring_1 = bstone::BasicString<TChar>{src_substring_1.c_str(), get_memory_resource()};
	const auto subview_1 = substring_1.get_view();

	const auto substring_2 = bstone::BasicString<TChar>{src_substring_2.c_str(), get_memory_resource()};
	const auto subview_2 = substring_2.get_view();

	const auto substring_3 = bstone::BasicString<TChar>{src_substring_3.c_str(), get_memory_resource()};
	const auto subview_3 = substring_3.get_view();

	const auto substring_4 = bstone::BasicString<TChar>{src_substring_4.c_str(), get_memory_resource()};
	const auto subview_4 = substring_4.get_view();

	const auto substring_5 = bstone::BasicString<TChar>{src_substring_5.c_str(), get_memory_resource()};
	const auto subview_5 = substring_5.get_view();

	const auto substring_6 = bstone::BasicString<TChar>{src_substring_6.c_str(), get_memory_resource()};
	const auto subview_6 = substring_6.get_view();

	const auto result_1 = string_1.index_of(subview_1) == 15;
	const auto result_2 = string_1.index_of(subview_2) < 0;
	const auto result_3 = string_1.index_of(subview_3) == 0;
	const auto result_4 = string_1.index_of(subview_4) < 0;
	const auto result_5 = string_2.index_of(subview_1) < 0;
	const auto result_6 = string_1.index_of(view_1) == 0;
	const auto result_7 = string_1.index_of(subview_5) == 2;
	const auto result_8 = string_1.index_of(subview_6) == 7;

	const auto is_valid =
		result_1 &&
		result_2 &&
		result_3 &&
		result_4 &&
		result_5 &&
		result_6 &&
		result_7 &&
		result_8 &&
		true;

	tester.check(is_valid);
}

void test_uddtqthzq34c7mn6_char()
{
	test_uddtqthzq34c7mn6<char>();
}

void test_uddtqthzq34c7mn6_wchar_t()
{
	test_uddtqthzq34c7mn6<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_of(std::intptr_t, const BasicStringView<Char>&) const noexcept
template<typename TChar>
void test_bjpo2szsl95k7lh7()
{
	const auto src_string = make_string<TChar>("ABC ABCDAB ABCDABCDABDE");
	const auto src_substring_1 = make_string<TChar>("ABCDABD");
	const auto src_substring_2 = make_string<TChar>("ABCDABd");
	const auto src_substring_3 = make_string<TChar>("");
	const auto src_substring_4 = make_string<TChar>("ABC ABCDAB ABCDABCDABDE!");
	const auto src_substring_5 = make_string<TChar>("C");
	const auto src_substring_6 = make_string<TChar>("DA");

	const auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto view_1 = string_1.get_view();

	const auto string_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto substring_1 = bstone::BasicString<TChar>{src_substring_1.c_str(), get_memory_resource()};
	const auto subview_1 = substring_1.get_view();

	const auto substring_2 = bstone::BasicString<TChar>{src_substring_2.c_str(), get_memory_resource()};
	const auto subview_2 = substring_2.get_view();

	const auto substring_3 = bstone::BasicString<TChar>{src_substring_3.c_str(), get_memory_resource()};
	const auto subview_3 = substring_3.get_view();

	const auto substring_4 = bstone::BasicString<TChar>{src_substring_4.c_str(), get_memory_resource()};
	const auto subview_4 = substring_4.get_view();

	const auto substring_5 = bstone::BasicString<TChar>{src_substring_5.c_str(), get_memory_resource()};
	const auto subview_5 = substring_5.get_view();

	const auto substring_6 = bstone::BasicString<TChar>{src_substring_6.c_str(), get_memory_resource()};
	const auto subview_6 = substring_6.get_view();

	const auto is_valid_1 = string_1.index_of(1, subview_1) == 15;
	const auto is_valid_2 = string_1.index_of(1, subview_2) < 0;
	const auto is_valid_3 = string_1.index_of(1, subview_3) == 1;
	const auto is_valid_4 = string_1.index_of(0, subview_4) < 0;
	const auto is_valid_5 = string_2.index_of(0, subview_1) < 0;
	const auto is_valid_6 = string_1.index_of(0, view_1) == 0;
	const auto is_valid_7 = string_1.index_of(1, subview_5) == 2;
	const auto is_valid_8 = string_1.index_of(1, subview_6) == 7;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		is_valid_5 &&
		is_valid_6 &&
		is_valid_7 &&
		is_valid_8 &&
		true;

	tester.check(is_valid);
}

void test_bjpo2szsl95k7lh7_char()
{
	test_bjpo2szsl95k7lh7<char>();
}

void test_bjpo2szsl95k7lh7_wchar_t()
{
	test_bjpo2szsl95k7lh7<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_of(Char) const
template<typename TChar>
void test_46l6sy7wjx5xaffc()
{
	const auto src_haystack = make_string<TChar>("string");
	const auto needle_1 = TChar{'i'};
	const auto needle_2 = TChar{'a'};

	const auto haystack = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};

	const auto result_1 = haystack.last_index_of(needle_1);
	const auto is_valid_1 = result_1 == 3;

	const auto result_2 = haystack.last_index_of(needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		true;

	tester.check(is_valid);
}

void test_46l6sy7wjx5xaffc_char()
{
	test_46l6sy7wjx5xaffc<char>();
}

void test_46l6sy7wjx5xaffc_wchar_t()
{
	test_46l6sy7wjx5xaffc<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_of(std::intptr_t, Char) const
template<typename TChar>
void test_wzq7eemoi8hpqj06()
{
	const auto src_haystack = make_string<TChar>("string");
	const auto needle_1 = TChar{'i'};
	const auto needle_2 = TChar{'a'};

	const auto haystack = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};

	const auto result_1 = haystack.last_index_of(1, needle_1);
	const auto is_valid_1 = result_1 == 3;

	const auto result_2 = haystack.last_index_of(1, needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack.last_index_of(4, needle_1);
	const auto is_valid_3 = result_3 < 0;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_wzq7eemoi8hpqj06_char()
{
	test_wzq7eemoi8hpqj06<char>();
}

void test_wzq7eemoi8hpqj06_wchar_t()
{
	test_wzq7eemoi8hpqj06<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_of(const Char*) const
template<typename TChar>
void test_p0mdn7ac7o45r7gt()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");
	const auto needle_1 = make_string<TChar>("ABCDABD");
	const auto needle_2 = make_string<TChar>("ABCDABd");
	const auto needle_3 = make_string<TChar>("");
	const auto needle_4 = make_string<TChar>("ABCDABCDABDE ABC ABCDAB!");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto result_1 = haystack_1.last_index_of(needle_1.c_str());
	const auto is_valid_1 = result_1 == 4;

	const auto result_2 = haystack_1.last_index_of(needle_2.c_str());
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_of(needle_3.c_str());
	const auto is_valid_3 = result_3 == 0;

	const auto result_4 = haystack_1.last_index_of(needle_4.c_str());
	const auto is_valid_4 = result_4 < 0;

	const auto result_5 = haystack_2.last_index_of(needle_1.c_str());
	const auto is_valid_5 = result_5 < 0;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		is_valid_5 &&
		true;

	tester.check(is_valid);
}

void test_p0mdn7ac7o45r7gt_char()
{
	test_p0mdn7ac7o45r7gt<char>();
}

void test_p0mdn7ac7o45r7gt_wchar_t()
{
	test_p0mdn7ac7o45r7gt<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_of(const Char*, std::intptr_t) const
template<typename TChar>
void test_fkjdefgkjd9hbsmn()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");
	const auto needle_1 = make_string<TChar>("ABCDABD!");
	const auto needle_2 = make_string<TChar>("ABCDABd!");
	const auto needle_3 = make_string<TChar>("!");
	const auto needle_4 = make_string<TChar>("ABCDABCDABDE ABC ABCDAB!!");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto result_1 = haystack_1.last_index_of(needle_1.c_str(), 7);
	const auto is_valid_1 = result_1 == 4;

	const auto result_2 = haystack_1.last_index_of(needle_2.c_str(), 7);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_of(needle_3.c_str(), 0);
	const auto is_valid_3 = result_3 == 0;

	const auto result_4 = haystack_1.last_index_of(needle_4.c_str(), 24);
	const auto is_valid_4 = result_4 < 0;

	const auto result_5 = haystack_2.last_index_of(needle_1.c_str(), 7);
	const auto is_valid_5 = result_5 < 0;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		is_valid_5 &&
		true;

	tester.check(is_valid);
}

void test_fkjdefgkjd9hbsmn_char()
{
	test_fkjdefgkjd9hbsmn<char>();
}

void test_fkjdefgkjd9hbsmn_wchar_t()
{
	test_fkjdefgkjd9hbsmn<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_of(std::intptr_t, const Char*) const
template<typename TChar>
void test_3t442q45xlsdjpla()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");
	const auto needle_1 = make_string<TChar>("ABCDABD");
	const auto needle_2 = make_string<TChar>("ABCDABd");
	const auto needle_3 = make_string<TChar>("");
	const auto needle_4 = make_string<TChar>("ABCDABCDABDE ABC ABCDAB!");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto result_1 = haystack_1.last_index_of(1, needle_1.c_str());
	const auto is_valid_1 = result_1 == 4;

	const auto result_2 = haystack_1.last_index_of(10, needle_1.c_str());
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_of(1, needle_2.c_str());
	const auto is_valid_3 = result_3 < 0;

	const auto result_4 = haystack_1.last_index_of(1, needle_3.c_str());
	const auto is_valid_4 = result_4 == 1;

	const auto result_5 = haystack_1.last_index_of(0, needle_4.c_str());
	const auto is_valid_5 = result_5 < 0;

	const auto result_6 = haystack_2.last_index_of(0, needle_1.c_str());
	const auto is_valid_6 = result_6 < 0;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		is_valid_5 &&
		is_valid_6 &&
		true;

	tester.check(is_valid);
}

void test_3t442q45xlsdjpla_char()
{
	test_3t442q45xlsdjpla<char>();
}

void test_3t442q45xlsdjpla_wchar_t()
{
	test_3t442q45xlsdjpla<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_of(std::intptr_t, const Char*, std::intptr_t) const
template<typename TChar>
void test_d79qtwz97qgdwcx2()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");
	const auto needle_1 = make_string<TChar>("ABCDABD!");
	const auto needle_2 = make_string<TChar>("ABCDABd!");
	const auto needle_3 = make_string<TChar>("!");
	const auto needle_4 = make_string<TChar>("ABCDABCDABDE ABC ABCDAB!!");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto result_1 = haystack_1.last_index_of(1, needle_1.c_str(), 7);
	const auto is_valid_1 = result_1 == 4;

	const auto result_2 = haystack_1.last_index_of(10, needle_1.c_str(), 7);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_of(1, needle_2.c_str(), 7);
	const auto is_valid_3 = result_3 < 0;

	const auto result_4 = haystack_1.last_index_of(1, needle_3.c_str(), 0);
	const auto is_valid_4 = result_4 == 1;

	const auto result_5 = haystack_1.last_index_of(0, needle_4.c_str(), 24);
	const auto is_valid_5 = result_5 < 0;

	const auto result_6 = haystack_2.last_index_of(0, needle_1.c_str(), 7);
	const auto is_valid_6 = result_6 < 0;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		is_valid_5 &&
		is_valid_6 &&
		true;

	tester.check(is_valid);
}

void test_d79qtwz97qgdwcx2_char()
{
	test_d79qtwz97qgdwcx2<char>();
}

void test_d79qtwz97qgdwcx2_wchar_t()
{
	test_d79qtwz97qgdwcx2<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_of(const BasicString&) const
template<typename TChar>
void test_lngmpj4clauqry8d()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");
	const auto src_needle_1 = make_string<TChar>("ABCDABD");
	const auto src_needle_2 = make_string<TChar>("ABCDABd");
	const auto src_needle_3 = make_string<TChar>("");
	const auto src_needle_4 = make_string<TChar>("ABCDABCDABDE ABC ABCDAB!");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto needle_1 = bstone::BasicString<TChar>{src_needle_1.c_str(), get_memory_resource()};
	const auto needle_2 = bstone::BasicString<TChar>{src_needle_2.c_str(), get_memory_resource()};
	const auto needle_3 = bstone::BasicString<TChar>{src_needle_3.c_str(), get_memory_resource()};
	const auto needle_4 = bstone::BasicString<TChar>{src_needle_4.c_str(), get_memory_resource()};

	const auto result_1 = haystack_1.last_index_of(needle_1);
	const auto is_valid_1 = result_1 == 4;

	const auto result_2 = haystack_1.last_index_of(needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_of(needle_3);
	const auto is_valid_3 = result_3 == 0;

	const auto result_4 = haystack_1.last_index_of(needle_4);
	const auto is_valid_4 = result_4 < 0;

	const auto result_5 = haystack_2.last_index_of(needle_1);
	const auto is_valid_5 = result_5 < 0;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		is_valid_5 &&
		true;

	tester.check(is_valid);
}

void test_lngmpj4clauqry8d_char()
{
	test_lngmpj4clauqry8d<char>();
}

void test_lngmpj4clauqry8d_wchar_t()
{
	test_lngmpj4clauqry8d<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_of(std::intptr_t, const BasicString&) const
template<typename TChar>
void test_dg01g5oq4d1yrigu()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");
	const auto src_needle_1 = make_string<TChar>("ABCDABD");
	const auto src_needle_2 = make_string<TChar>("ABCDABd");
	const auto src_needle_3 = make_string<TChar>("");
	const auto src_needle_4 = make_string<TChar>("ABCDABCDABDE ABC ABCDAB!");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto needle_1 = bstone::BasicString<TChar>{src_needle_1.c_str(), get_memory_resource()};
	const auto needle_2 = bstone::BasicString<TChar>{src_needle_2.c_str(), get_memory_resource()};
	const auto needle_3 = bstone::BasicString<TChar>{src_needle_3.c_str(), get_memory_resource()};
	const auto needle_4 = bstone::BasicString<TChar>{src_needle_4.c_str(), get_memory_resource()};

	const auto result_1 = haystack_1.last_index_of(1, needle_1);
	const auto is_valid_1 = result_1 == 4;

	const auto result_2 = haystack_1.last_index_of(10, needle_1);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_of(1, needle_2);
	const auto is_valid_3 = result_3 < 0;

	const auto result_4 = haystack_1.last_index_of(1, needle_3);
	const auto is_valid_4 = result_4 == 1;

	const auto result_5 = haystack_1.last_index_of(0, needle_4);
	const auto is_valid_5 = result_5 < 0;

	const auto result_6 = haystack_2.last_index_of(0, needle_1);
	const auto is_valid_6 = result_6 < 0;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		is_valid_5 &&
		is_valid_6 &&
		true;

	tester.check(is_valid);
}

void test_dg01g5oq4d1yrigu_char()
{
	test_dg01g5oq4d1yrigu<char>();
}

void test_dg01g5oq4d1yrigu_wchar_t()
{
	test_dg01g5oq4d1yrigu<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_of(const BasicStringView<Char>&) const noexcept
template<typename TChar>
void test_wf6xgdnoipdh6mvu()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");
	const auto src_needle_1 = make_string<TChar>("ABCDABD");
	const auto src_needle_2 = make_string<TChar>("ABCDABd");
	const auto src_needle_3 = make_string<TChar>("");
	const auto src_needle_4 = make_string<TChar>("ABCDABCDABDE ABC ABCDAB!");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto needle_1 = bstone::BasicString<TChar>{src_needle_1.c_str(), get_memory_resource()};
	const auto needle_view_1 = needle_1.get_view();

	const auto needle_2 = bstone::BasicString<TChar>{src_needle_2.c_str(), get_memory_resource()};
	const auto needle_view_2 = needle_2.get_view();

	const auto needle_3 = bstone::BasicString<TChar>{src_needle_3.c_str(), get_memory_resource()};
	const auto needle_view_3 = needle_3.get_view();

	const auto needle_4 = bstone::BasicString<TChar>{src_needle_4.c_str(), get_memory_resource()};
	const auto needle_view_4 = needle_4.get_view();

	const auto result_1 = haystack_1.last_index_of(needle_view_1);
	const auto is_valid_1 = result_1 == 4;

	const auto result_2 = haystack_1.last_index_of(needle_view_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_of(needle_view_3);
	const auto is_valid_3 = result_3 == 0;

	const auto result_4 = haystack_1.last_index_of(needle_view_4);
	const auto is_valid_4 = result_4 < 0;

	const auto result_5 = haystack_2.last_index_of(needle_view_1);
	const auto is_valid_5 = result_5 < 0;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		is_valid_5 &&
		true;

	tester.check(is_valid);
}

void test_wf6xgdnoipdh6mvu_char()
{
	test_wf6xgdnoipdh6mvu<char>();
}

void test_wf6xgdnoipdh6mvu_wchar_t()
{
	test_wf6xgdnoipdh6mvu<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_of(std::intptr_t, const BasicStringView<Char>&) const noexcept
template<typename TChar>
void test_o49vbnlfhr0o8qgy()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");
	const auto src_needle_1 = make_string<TChar>("ABCDABD");
	const auto src_needle_2 = make_string<TChar>("ABCDABd");
	const auto src_needle_3 = make_string<TChar>("");
	const auto src_needle_4 = make_string<TChar>("ABCDABCDABDE ABC ABCDAB!");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto needle_1 = bstone::BasicString<TChar>{src_needle_1.c_str(), get_memory_resource()};
	const auto needle_view_1 = needle_1.get_view();

	const auto needle_2 = bstone::BasicString<TChar>{src_needle_2.c_str(), get_memory_resource()};
	const auto needle_view_2 = needle_2.get_view();

	const auto needle_3 = bstone::BasicString<TChar>{src_needle_3.c_str(), get_memory_resource()};
	const auto needle_view_3 = needle_3.get_view();

	const auto needle_4 = bstone::BasicString<TChar>{src_needle_4.c_str(), get_memory_resource()};
	const auto needle_view_4 = needle_4.get_view();

	const auto result_1 = haystack_1.last_index_of(1, needle_view_1);
	const auto is_valid_1 = result_1 == 4;

	const auto result_2 = haystack_1.last_index_of(10, needle_view_1);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_of(1, needle_view_2);
	const auto is_valid_3 = result_3 < 0;

	const auto result_4 = haystack_1.last_index_of(1, needle_view_3);
	const auto is_valid_4 = result_4 == 1;

	const auto result_5 = haystack_1.last_index_of(0, needle_view_4);
	const auto is_valid_5 = result_5 < 0;

	const auto result_6 = haystack_2.last_index_of(0, needle_view_1);
	const auto is_valid_6 = result_6 < 0;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		is_valid_5 &&
		is_valid_6 &&
		true;

	tester.check(is_valid);
}

void test_o49vbnlfhr0o8qgy_char()
{
	test_o49vbnlfhr0o8qgy<char>();
}

void test_o49vbnlfhr0o8qgy_wchar_t()
{
	test_o49vbnlfhr0o8qgy<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_of_any(Char) const noexcept
template<typename TChar>
void test_v1k94bqmiskcer9g()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto needle_1 = TChar{' '};
	const auto needle_2 = TChar{'X'};

	const auto result_1 = haystack_2.index_of_any(needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.index_of_any(needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.index_of_any(needle_1);
	const auto is_valid_3 = result_3 == 12;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_v1k94bqmiskcer9g_char()
{
	test_v1k94bqmiskcer9g<char>();
}

void test_v1k94bqmiskcer9g_wchar_t()
{
	test_v1k94bqmiskcer9g<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_of_any(const Char*) const noexcept
template<typename TChar>
void test_t0zm8e9yorlrmhyy()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto src_needle_1 = make_string<TChar>(" ");
	const auto src_needle_2 = make_string<TChar>("X");
	const auto src_needle_3 = make_string<TChar>("");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto needle_1 = src_needle_1.c_str();
	const auto needle_2 = src_needle_2.c_str();
	const auto needle_3 = src_needle_3.c_str();

	const auto result_1 = haystack_2.index_of_any(needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.index_of_any(needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_2.index_of_any(needle_3);
	const auto is_valid_3 = result_3 == 0;

	const auto result_4 = haystack_1.index_of_any(needle_1);
	const auto is_valid_4 = result_4 == 12;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_t0zm8e9yorlrmhyy_char()
{
	test_t0zm8e9yorlrmhyy<char>();
}

void test_t0zm8e9yorlrmhyy_wchar_t()
{
	test_t0zm8e9yorlrmhyy<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_of_any(const Char*, std::intptr_t) const noexcept
template<typename TChar>
void test_61c15k1dqjt9rfjs()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto src_needle_1 = make_string<TChar>(" A");
	const auto src_needle_2 = make_string<TChar>("XA");
	const auto src_needle_3 = make_string<TChar>("A");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto needle_1 = src_needle_1.c_str();
	const auto needle_2 = src_needle_2.c_str();
	const auto needle_3 = src_needle_3.c_str();

	const auto result_1 = haystack_2.index_of_any(needle_1, 1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.index_of_any(needle_2, 1);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_2.index_of_any(needle_3, 0);
	const auto is_valid_3 = result_3 == 0;

	const auto result_4 = haystack_1.index_of_any(needle_1, 1);
	const auto is_valid_4 = result_4 == 12;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_61c15k1dqjt9rfjs_char()
{
	test_61c15k1dqjt9rfjs<char>();
}

void test_61c15k1dqjt9rfjs_wchar_t()
{
	test_61c15k1dqjt9rfjs<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_of_any(const BasicString&) const noexcept
template<typename TChar>
void test_eu8d7kvdx79y5h18()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto src_needle_1 = make_string<TChar>(" ");
	const auto src_needle_2 = make_string<TChar>("X");
	const auto src_needle_3 = make_string<TChar>("");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto needle_1 = bstone::BasicString<TChar>{src_needle_1.c_str(), get_memory_resource()};
	const auto needle_2 = bstone::BasicString<TChar>{src_needle_2.c_str(), get_memory_resource()};
	const auto needle_3 = bstone::BasicString<TChar>{src_needle_3.c_str(), get_memory_resource()};

	const auto result_1 = haystack_2.index_of_any(needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.index_of_any(needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_2.index_of_any(needle_3);
	const auto is_valid_3 = result_3 == 0;

	const auto result_4 = haystack_1.index_of_any(needle_1);
	const auto is_valid_4 = result_4 == 12;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_eu8d7kvdx79y5h18_char()
{
	test_eu8d7kvdx79y5h18<char>();
}

void test_eu8d7kvdx79y5h18_wchar_t()
{
	test_eu8d7kvdx79y5h18<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_of_any(const BasicString&, std::intptr_t) const noexcept
template<typename TChar>
void test_ju2w7rm7607381x8()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto src_needle_1 = make_string<TChar>("! ");
	const auto src_needle_2 = make_string<TChar>("!X");
	const auto src_needle_3 = make_string<TChar>("!");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto needle_1 = bstone::BasicString<TChar>{src_needle_1.c_str(), get_memory_resource()};
	const auto needle_2 = bstone::BasicString<TChar>{src_needle_2.c_str(), get_memory_resource()};
	const auto needle_3 = bstone::BasicString<TChar>{src_needle_3.c_str(), get_memory_resource()};

	const auto result_1 = haystack_2.index_of_any(needle_1, 1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.index_of_any(needle_2, 1);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_2.index_of_any(needle_3, 1);
	const auto is_valid_3 = result_3 == 0;

	const auto result_4 = haystack_1.index_of_any(needle_1, 1);
	const auto is_valid_4 = result_4 == 12;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_ju2w7rm7607381x8_char()
{
	test_ju2w7rm7607381x8<char>();
}

void test_ju2w7rm7607381x8_wchar_t()
{
	test_ju2w7rm7607381x8<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_of_any(const BasicString&, std::intptr_t, std::intptr_t) const noexcept
template<typename TChar>
void test_pxeykhvr6h1xskox()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto src_needle_1 = make_string<TChar>("! !");
	const auto src_needle_2 = make_string<TChar>("!X!");
	const auto src_needle_3 = make_string<TChar>("!!");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto needle_1 = bstone::BasicString<TChar>{src_needle_1.c_str(), get_memory_resource()};
	const auto needle_2 = bstone::BasicString<TChar>{src_needle_2.c_str(), get_memory_resource()};
	const auto needle_3 = bstone::BasicString<TChar>{src_needle_3.c_str(), get_memory_resource()};

	const auto result_1 = haystack_2.index_of_any(needle_1, 1, 1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.index_of_any(needle_2, 1, 1);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_2.index_of_any(needle_3, 1, 0);
	const auto is_valid_3 = result_3 == 0;

	const auto result_4 = haystack_1.index_of_any(needle_1, 1, 1);
	const auto is_valid_4 = result_4 == 12;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_pxeykhvr6h1xskox_char()
{
	test_pxeykhvr6h1xskox<char>();
}

void test_pxeykhvr6h1xskox_wchar_t()
{
	test_pxeykhvr6h1xskox<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_of_any(BasicStringView<Char>) const noexcept
template<typename TChar>
void test_i1ijmtkfo9vjq72e()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto src_needle_1 = make_string<TChar>(" ");
	const auto src_needle_2 = make_string<TChar>("X");
	const auto src_needle_3 = make_string<TChar>("");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto needle_1 = bstone::BasicStringView<TChar>{src_needle_1.c_str()};
	const auto needle_2 = bstone::BasicStringView<TChar>{src_needle_2.c_str()};
	const auto needle_3 = bstone::BasicStringView<TChar>{src_needle_3.c_str()};

	const auto result_1 = haystack_2.index_of_any(needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.index_of_any(needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_2.index_of_any(needle_3);
	const auto is_valid_3 = result_3 == 0;

	const auto result_4 = haystack_1.index_of_any(needle_1);
	const auto is_valid_4 = result_4 == 12;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_i1ijmtkfo9vjq72e_char()
{
	test_i1ijmtkfo9vjq72e<char>();
}

void test_i1ijmtkfo9vjq72e_wchar_t()
{
	test_i1ijmtkfo9vjq72e<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_of_any(std::intptr_t, Char) const noexcept
template<typename TChar>
void test_o66c69um4ba9jehw()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto needle_1 = TChar{' '};
	const auto needle_2 = TChar{'X'};

	const auto result_1 = haystack_2.index_of_any(0, needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.index_of_any(10, needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.index_of_any(10, needle_1);
	const auto is_valid_3 = result_3 == 12;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_o66c69um4ba9jehw_char()
{
	test_o66c69um4ba9jehw<char>();
}

void test_o66c69um4ba9jehw_wchar_t()
{
	test_o66c69um4ba9jehw<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_of_any(std::intptr_t, const Char*) const noexcept
template<typename TChar>
void test_7rwjpeui0h6fyj9w()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto src_needle_1 = make_string<TChar>(" ");
	const auto src_needle_2 = make_string<TChar>("X");
	const auto src_needle_3 = make_string<TChar>("");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto needle_1 = src_needle_1.c_str();
	const auto needle_2 = src_needle_2.c_str();
	const auto needle_3 = src_needle_3.c_str();

	const auto result_1 = haystack_2.index_of_any(0, needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.index_of_any(10, needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_2.index_of_any(0, needle_3);
	const auto is_valid_3 = result_3 == 0;

	const auto result_4 = haystack_1.index_of_any(10, needle_1);
	const auto is_valid_4 = result_4 == 12;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_7rwjpeui0h6fyj9w_char()
{
	test_7rwjpeui0h6fyj9w<char>();
}

void test_7rwjpeui0h6fyj9w_wchar_t()
{
	test_7rwjpeui0h6fyj9w<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_of_any(std::intptr_t, const Char*, std::intptr_t) const noexcept
template<typename TChar>
void test_q7g4y4c0fte1k8mo()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto src_needle_1 = make_string<TChar>(" A");
	const auto src_needle_2 = make_string<TChar>("XA");
	const auto src_needle_3 = make_string<TChar>("A");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto needle_1 = src_needle_1.c_str();
	const auto needle_2 = src_needle_2.c_str();
	const auto needle_3 = src_needle_3.c_str();

	const auto result_1 = haystack_2.index_of_any(0, needle_1, 1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.index_of_any(10, needle_2, 1);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_2.index_of_any(0, needle_3, 0);
	const auto is_valid_3 = result_3 == 0;

	const auto result_4 = haystack_1.index_of_any(10, needle_1, 1);
	const auto is_valid_4 = result_4 == 12;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_q7g4y4c0fte1k8mo_char()
{
	test_q7g4y4c0fte1k8mo<char>();
}

void test_q7g4y4c0fte1k8mo_wchar_t()
{
	test_q7g4y4c0fte1k8mo<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_of_any(std::intptr_t, const BasicString&) const noexcept
template<typename TChar>
void test_j0g63voyqye98v48()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto src_needle_1 = make_string<TChar>(" ");
	const auto src_needle_2 = make_string<TChar>("X");
	const auto src_needle_3 = make_string<TChar>("");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto needle_1 = bstone::BasicString<TChar>{src_needle_1.c_str(), get_memory_resource()};
	const auto needle_2 = bstone::BasicString<TChar>{src_needle_2.c_str(), get_memory_resource()};
	const auto needle_3 = bstone::BasicString<TChar>{src_needle_3.c_str(), get_memory_resource()};

	const auto result_1 = haystack_2.index_of_any(0, needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.index_of_any(10, needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_2.index_of_any(0, needle_3);
	const auto is_valid_3 = result_3 == 0;

	const auto result_4 = haystack_1.index_of_any(10, needle_1);
	const auto is_valid_4 = result_4 == 12;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_j0g63voyqye98v48_char()
{
	test_j0g63voyqye98v48<char>();
}

void test_j0g63voyqye98v48_wchar_t()
{
	test_j0g63voyqye98v48<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_of_any(std::intptr_t, const BasicString&, std::intptr_t) const noexcept
template<typename TChar>
void test_5ixrmtnfdpn2zf99()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto src_needle_1 = make_string<TChar>("! ");
	const auto src_needle_2 = make_string<TChar>("!X");
	const auto src_needle_3 = make_string<TChar>("!");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto needle_1 = bstone::BasicString<TChar>{src_needle_1.c_str(), get_memory_resource()};
	const auto needle_2 = bstone::BasicString<TChar>{src_needle_2.c_str(), get_memory_resource()};
	const auto needle_3 = bstone::BasicString<TChar>{src_needle_3.c_str(), get_memory_resource()};

	const auto result_1 = haystack_2.index_of_any(0, needle_1, 1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.index_of_any(10, needle_2, 1);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_2.index_of_any(0, needle_3, 1);
	const auto is_valid_3 = result_3 == 0;

	const auto result_4 = haystack_1.index_of_any(10, needle_1, 1);
	const auto is_valid_4 = result_4 == 12;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_5ixrmtnfdpn2zf99_char()
{
	test_5ixrmtnfdpn2zf99<char>();
}

void test_5ixrmtnfdpn2zf99_wchar_t()
{
	test_5ixrmtnfdpn2zf99<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_of_any(std::intptr_t, const BasicString&, std::intptr_t, std::intptr_t) const noexcept
template<typename TChar>
void test_3pyycygs5y32ivp6()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto src_needle_1 = make_string<TChar>("! !");
	const auto src_needle_2 = make_string<TChar>("!X!");
	const auto src_needle_3 = make_string<TChar>("!!");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto needle_1 = bstone::BasicString<TChar>{src_needle_1.c_str(), get_memory_resource()};
	const auto needle_2 = bstone::BasicString<TChar>{src_needle_2.c_str(), get_memory_resource()};
	const auto needle_3 = bstone::BasicString<TChar>{src_needle_3.c_str(), get_memory_resource()};

	const auto result_1 = haystack_2.index_of_any(0, needle_1, 1, 1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.index_of_any(10, needle_2, 1, 1);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_2.index_of_any(0, needle_3, 1, 0);
	const auto is_valid_3 = result_3 == 0;

	const auto result_4 = haystack_1.index_of_any(10, needle_1, 1, 1);
	const auto is_valid_4 = result_4 == 12;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_3pyycygs5y32ivp6_char()
{
	test_3pyycygs5y32ivp6<char>();
}

void test_3pyycygs5y32ivp6_wchar_t()
{
	test_3pyycygs5y32ivp6<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_of_any(std::intptr_t, BasicStringView<Char>) const noexcept
template<typename TChar>
void test_8b1zzhrgsft08sll()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto src_needle_1 = make_string<TChar>(" ");
	const auto src_needle_2 = make_string<TChar>("X");
	const auto src_needle_3 = make_string<TChar>("");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto needle_1 = bstone::BasicStringView<TChar>{src_needle_1.c_str()};
	const auto needle_2 = bstone::BasicStringView<TChar>{src_needle_2.c_str()};
	const auto needle_3 = bstone::BasicStringView<TChar>{src_needle_3.c_str()};

	const auto result_1 = haystack_2.index_of_any(0, needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.index_of_any(10, needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_2.index_of_any(0, needle_3);
	const auto is_valid_3 = result_3 == 0;

	const auto result_4 = haystack_1.index_of_any(10, needle_1);
	const auto is_valid_4 = result_4 == 12;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_8b1zzhrgsft08sll_char()
{
	test_8b1zzhrgsft08sll<char>();
}

void test_8b1zzhrgsft08sll_wchar_t()
{
	test_8b1zzhrgsft08sll<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_not_of_any(Char) const noexcept
template<typename TChar>
void test_wb5bpimb7b8733m4()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto needle_1 = TChar{'A'};
	const auto needle_2 = TChar{'X'};

	const auto result_1 = haystack_2.index_not_of_any(needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.index_not_of_any(needle_2);
	const auto is_valid_2 = result_2 == 0;

	const auto result_3 = haystack_1.index_not_of_any(needle_1);
	const auto is_valid_3 = result_3 == 1;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_wb5bpimb7b8733m4_char()
{
	test_wb5bpimb7b8733m4<char>();
}

void test_wb5bpimb7b8733m4_wchar_t()
{
	test_wb5bpimb7b8733m4<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_not_of_any(const Char*) const noexcept
template<typename TChar>
void test_3dexxbngdoszc9ao()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("ABCD");
	const auto src_needle_2 = make_string<TChar>("ABCDE ");
	const auto src_needle_3 = make_string<TChar>("");

	const auto needle_1 = src_needle_1.c_str();
	const auto needle_2 = src_needle_2.c_str();
	const auto needle_3 = src_needle_3.c_str();

	const auto result_1 = haystack_2.index_not_of_any(needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.index_not_of_any(needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.index_not_of_any(needle_3);
	const auto is_valid_3 = result_3 == 0;

	const auto result_4 = haystack_1.index_not_of_any(needle_1);
	const auto is_valid_4 = result_4 == 11;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_3dexxbngdoszc9ao_char()
{
	test_3dexxbngdoszc9ao<char>();
}

void test_3dexxbngdoszc9ao_wchar_t()
{
	test_3dexxbngdoszc9ao<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_not_of_any(const Char*, std::intptr_t) const noexcept
template<typename TChar>
void test_2270z9jmj36w9ro6()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("ABCD!");
	const auto src_needle_2 = make_string<TChar>("ABCDE !");
	const auto src_needle_3 = make_string<TChar>("!");

	const auto needle_1 = src_needle_1.c_str();
	const auto needle_2 = src_needle_2.c_str();
	const auto needle_3 = src_needle_3.c_str();

	const auto result_1 = haystack_2.index_not_of_any(needle_1, 4);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.index_not_of_any(needle_2, 6);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.index_not_of_any(needle_3, 0);
	const auto is_valid_3 = result_3 == 0;

	const auto result_4 = haystack_1.index_not_of_any(needle_1, 4);
	const auto is_valid_4 = result_4 == 11;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_2270z9jmj36w9ro6_char()
{
	test_2270z9jmj36w9ro6<char>();
}

void test_2270z9jmj36w9ro6_wchar_t()
{
	test_2270z9jmj36w9ro6<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_not_of_any(const BasicString&) const noexcept
template<typename TChar>
void test_9l5w87x8rukm8vrw()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("ABCD");
	const auto src_needle_2 = make_string<TChar>("ABCDE ");
	const auto src_needle_3 = make_string<TChar>("");

	const auto needle_1 = bstone::BasicString<TChar>{src_needle_1.c_str(), get_memory_resource()};
	const auto needle_2 = bstone::BasicString<TChar>{src_needle_2.c_str(), get_memory_resource()};
	const auto needle_3 = bstone::BasicString<TChar>{src_needle_3.c_str(), get_memory_resource()};

	const auto result_1 = haystack_2.index_not_of_any(needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.index_not_of_any(needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.index_not_of_any(needle_3);
	const auto is_valid_3 = result_3 == 0;

	const auto result_4 = haystack_1.index_not_of_any(needle_1);
	const auto is_valid_4 = result_4 == 11;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_9l5w87x8rukm8vrw_char()
{
	test_9l5w87x8rukm8vrw<char>();
}

void test_9l5w87x8rukm8vrw_wchar_t()
{
	test_9l5w87x8rukm8vrw<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_not_of_any(const BasicString&, std::intptr_t) const noexcept
template<typename TChar>
void test_8hqhk9twgig3w6iu()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("!ABCD");
	const auto src_needle_2 = make_string<TChar>("!ABCDE ");
	const auto src_needle_3 = make_string<TChar>("!");

	const auto needle_1 = bstone::BasicString<TChar>{src_needle_1.c_str(), get_memory_resource()};
	const auto needle_2 = bstone::BasicString<TChar>{src_needle_2.c_str(), get_memory_resource()};
	const auto needle_3 = bstone::BasicString<TChar>{src_needle_3.c_str(), get_memory_resource()};

	const auto result_1 = haystack_2.index_not_of_any(needle_1, 1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.index_not_of_any(needle_2, 1);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.index_not_of_any(needle_3, 1);
	const auto is_valid_3 = result_3 == 0;

	const auto result_4 = haystack_1.index_not_of_any(needle_1, 1);
	const auto is_valid_4 = result_4 == 11;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_8hqhk9twgig3w6iu_char()
{
	test_8hqhk9twgig3w6iu<char>();
}

void test_8hqhk9twgig3w6iu_wchar_t()
{
	test_8hqhk9twgig3w6iu<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_not_of_any(const BasicString&, std::intptr_t, std::intptr_t) const noexcept
template<typename TChar>
void test_sp0pxeqmryuj3l2q()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("!ABCD!");
	const auto src_needle_2 = make_string<TChar>("!ABCDE !");
	const auto src_needle_3 = make_string<TChar>("!!");

	const auto needle_1 = bstone::BasicString<TChar>{src_needle_1.c_str(), get_memory_resource()};
	const auto needle_2 = bstone::BasicString<TChar>{src_needle_2.c_str(), get_memory_resource()};
	const auto needle_3 = bstone::BasicString<TChar>{src_needle_3.c_str(), get_memory_resource()};

	const auto result_1 = haystack_2.index_not_of_any(needle_1, 1, 4);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.index_not_of_any(needle_2, 1, 6);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.index_not_of_any(needle_3, 1, 0);
	const auto is_valid_3 = result_3 == 0;

	const auto result_4 = haystack_1.index_not_of_any(needle_1, 1, 4);
	const auto is_valid_4 = result_4 == 11;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_sp0pxeqmryuj3l2q_char()
{
	test_sp0pxeqmryuj3l2q<char>();
}

void test_sp0pxeqmryuj3l2q_wchar_t()
{
	test_sp0pxeqmryuj3l2q<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_not_of_any(BasicStringView<Char>) const noexcept
template<typename TChar>
void test_yfvg1t8326lbwkuq()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("ABCD");
	const auto src_needle_2 = make_string<TChar>("ABCDE ");
	const auto src_needle_3 = make_string<TChar>("");

	const auto needle_1 = bstone::BasicStringView<TChar>{src_needle_1.c_str()};
	const auto needle_2 = bstone::BasicStringView<TChar>{src_needle_2.c_str()};
	const auto needle_3 = bstone::BasicStringView<TChar>{src_needle_3.c_str()};

	const auto result_1 = haystack_2.index_not_of_any(needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.index_not_of_any(needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.index_not_of_any(needle_3);
	const auto is_valid_3 = result_3 == 0;

	const auto result_4 = haystack_1.index_not_of_any(needle_1);
	const auto is_valid_4 = result_4 == 11;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_yfvg1t8326lbwkuq_char()
{
	test_yfvg1t8326lbwkuq<char>();
}

void test_yfvg1t8326lbwkuq_wchar_t()
{
	test_yfvg1t8326lbwkuq<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_not_of_any(std::intptr_t, Char) const noexcept
template<typename TChar>
void test_xl1c3mek6i1vpuiw()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto needle_1 = TChar{'A'};
	const auto needle_2 = TChar{'X'};

	const auto result_1 = haystack_2.index_not_of_any(0, needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.index_not_of_any(4, needle_2);
	const auto is_valid_2 = result_2 == 4;

	const auto result_3 = haystack_1.index_not_of_any(4, needle_1);
	const auto is_valid_3 = result_3 == 5;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_xl1c3mek6i1vpuiw_char()
{
	test_xl1c3mek6i1vpuiw<char>();
}

void test_xl1c3mek6i1vpuiw_wchar_t()
{
	test_xl1c3mek6i1vpuiw<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_not_of_any(std::intptr_t, const Char*) const noexcept
template<typename TChar>
void test_h3chz1583d6rd9wo()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("ABCD");
	const auto src_needle_2 = make_string<TChar>("ABCDE ");
	const auto src_needle_3 = make_string<TChar>("");

	const auto needle_1 = src_needle_1.c_str();
	const auto needle_2 = src_needle_2.c_str();
	const auto needle_3 = src_needle_3.c_str();

	const auto result_1 = haystack_2.index_not_of_any(0, needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.index_not_of_any(4, needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.index_not_of_any(4, needle_3);
	const auto is_valid_3 = result_3 == 4;

	const auto result_4 = haystack_1.index_not_of_any(4, needle_1);
	const auto is_valid_4 = result_4 == 11;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_h3chz1583d6rd9wo_char()
{
	test_h3chz1583d6rd9wo<char>();
}

void test_h3chz1583d6rd9wo_wchar_t()
{
	test_h3chz1583d6rd9wo<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_not_of_any(std::intptr_t, const Char*, std::intptr_t) const noexcept
template<typename TChar>
void test_3fw7dhxj9pd8d4xt()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("ABCD!");
	const auto src_needle_2 = make_string<TChar>("ABCDE !");
	const auto src_needle_3 = make_string<TChar>("!");

	const auto needle_1 = src_needle_1.c_str();
	const auto needle_2 = src_needle_2.c_str();
	const auto needle_3 = src_needle_3.c_str();

	const auto result_1 = haystack_2.index_not_of_any(0, needle_1, 4);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.index_not_of_any(4, needle_2, 6);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.index_not_of_any(4, needle_3, 0);
	const auto is_valid_3 = result_3 == 4;

	const auto result_4 = haystack_1.index_not_of_any(4, needle_1, 4);
	const auto is_valid_4 = result_4 == 11;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_3fw7dhxj9pd8d4xt_char()
{
	test_3fw7dhxj9pd8d4xt<char>();
}

void test_3fw7dhxj9pd8d4xt_wchar_t()
{
	test_3fw7dhxj9pd8d4xt<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_not_of_any(std::intptr_t, const BasicString&) const noexcept
template<typename TChar>
void test_m5h2kycijbmewsgm()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("ABCD");
	const auto src_needle_2 = make_string<TChar>("ABCDE ");
	const auto src_needle_3 = make_string<TChar>("");

	const auto needle_1 = bstone::BasicString<TChar>{src_needle_1.c_str(), get_memory_resource()};
	const auto needle_2 = bstone::BasicString<TChar>{src_needle_2.c_str(), get_memory_resource()};
	const auto needle_3 = bstone::BasicString<TChar>{src_needle_3.c_str(), get_memory_resource()};

	const auto result_1 = haystack_2.index_not_of_any(0, needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.index_not_of_any(4, needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.index_not_of_any(4, needle_3);
	const auto is_valid_3 = result_3 == 4;

	const auto result_4 = haystack_1.index_not_of_any(needle_1);
	const auto is_valid_4 = result_4 == 11;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_m5h2kycijbmewsgm_char()
{
	test_m5h2kycijbmewsgm<char>();
}

void test_m5h2kycijbmewsgm_wchar_t()
{
	test_m5h2kycijbmewsgm<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_not_of_any(std::intptr_t, const BasicString&, std::intptr_t) const noexcept
template<typename TChar>
void test_6w4r6acup3ggob06()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("!ABCD");
	const auto src_needle_2 = make_string<TChar>("!ABCDE ");
	const auto src_needle_3 = make_string<TChar>("!");

	const auto needle_1 = bstone::BasicString<TChar>{src_needle_1.c_str(), get_memory_resource()};
	const auto needle_2 = bstone::BasicString<TChar>{src_needle_2.c_str(), get_memory_resource()};
	const auto needle_3 = bstone::BasicString<TChar>{src_needle_3.c_str(), get_memory_resource()};

	const auto result_1 = haystack_2.index_not_of_any(0, needle_1, 1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.index_not_of_any(4, needle_2, 1);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.index_not_of_any(4, needle_3, 1);
	const auto is_valid_3 = result_3 == 4;

	const auto result_4 = haystack_1.index_not_of_any(4, needle_1, 1);
	const auto is_valid_4 = result_4 == 11;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_6w4r6acup3ggob06_char()
{
	test_6w4r6acup3ggob06<char>();
}

void test_6w4r6acup3ggob06_wchar_t()
{
	test_6w4r6acup3ggob06<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_not_of_any(std::intptr_t, const BasicString&, std::intptr_t, std::intptr_t) const noexcept
template<typename TChar>
void test_nxyobgy48oymiht0()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("!ABCD!");
	const auto src_needle_2 = make_string<TChar>("!ABCDE !");
	const auto src_needle_3 = make_string<TChar>("!!");

	const auto needle_1 = bstone::BasicString<TChar>{src_needle_1.c_str(), get_memory_resource()};
	const auto needle_2 = bstone::BasicString<TChar>{src_needle_2.c_str(), get_memory_resource()};
	const auto needle_3 = bstone::BasicString<TChar>{src_needle_3.c_str(), get_memory_resource()};

	const auto result_1 = haystack_2.index_not_of_any(0, needle_1, 1, 4);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.index_not_of_any(4, needle_2, 1, 6);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.index_not_of_any(4, needle_3, 1, 0);
	const auto is_valid_3 = result_3 == 4;

	const auto result_4 = haystack_1.index_not_of_any(4, needle_1, 1, 4);
	const auto is_valid_4 = result_4 == 11;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_nxyobgy48oymiht0_char()
{
	test_nxyobgy48oymiht0<char>();
}

void test_nxyobgy48oymiht0_wchar_t()
{
	test_nxyobgy48oymiht0<wchar_t>();
}

// ==========================================================================

// std::intptr_t index_not_of_any(std::intptr_t, BasicStringView<Char>) const noexcept
template<typename TChar>
void test_sygb4b6uauyrn2mf()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("ABCD");
	const auto src_needle_2 = make_string<TChar>("ABCDE ");
	const auto src_needle_3 = make_string<TChar>("");

	const auto needle_1 = bstone::BasicStringView<TChar>{src_needle_1.c_str()};
	const auto needle_2 = bstone::BasicStringView<TChar>{src_needle_2.c_str()};
	const auto needle_3 = bstone::BasicStringView<TChar>{src_needle_3.c_str()};

	const auto result_1 = haystack_2.index_not_of_any(0, needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.index_not_of_any(4, needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.index_not_of_any(4, needle_3);
	const auto is_valid_3 = result_3 == 4;

	const auto result_4 = haystack_1.index_not_of_any(4, needle_1);
	const auto is_valid_4 = result_4 == 11;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_sygb4b6uauyrn2mf_char()
{
	test_sygb4b6uauyrn2mf<char>();
}

void test_sygb4b6uauyrn2mf_wchar_t()
{
	test_sygb4b6uauyrn2mf<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_of_any(Char) const noexcept
template<typename TChar>
void test_7uevpg98ot04vqev()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto needle_1 = TChar{'A'};
	const auto needle_2 = TChar{'X'};

	const auto result_1 = haystack_2.last_index_of_any(needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.last_index_of_any(needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_of_any(needle_1);
	const auto is_valid_3 = result_3 == 21;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_7uevpg98ot04vqev_char()
{
	test_7uevpg98ot04vqev<char>();
}

void test_7uevpg98ot04vqev_wchar_t()
{
	test_7uevpg98ot04vqev<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_of_any(const Char*) const noexcept
template<typename TChar>
void test_830axldq4tob123f()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("DC");
	const auto src_needle_2 = make_string<TChar>("XY");

	const auto needle_1 = src_needle_1.c_str();
	const auto needle_2 = src_needle_2.c_str();

	const auto result_1 = haystack_2.last_index_of_any(needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.last_index_of_any(needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_of_any(needle_1);
	const auto is_valid_3 = result_3 == 20;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_830axldq4tob123f_char()
{
	test_830axldq4tob123f<char>();
}

void test_830axldq4tob123f_wchar_t()
{
	test_830axldq4tob123f<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_of_any(const Char*, std::intptr_t) const noexcept
template<typename TChar>
void test_wakikixlvzuzklw1()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("DC!");
	const auto src_needle_2 = make_string<TChar>("XY!");

	const auto needle_1 = src_needle_1.c_str();
	const auto needle_2 = src_needle_2.c_str();

	const auto result_1 = haystack_2.last_index_of_any(needle_1, 2);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.last_index_of_any(needle_2, 2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_of_any(needle_1, 2);
	const auto is_valid_3 = result_3 == 20;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_wakikixlvzuzklw1_char()
{
	test_wakikixlvzuzklw1<char>();
}

void test_wakikixlvzuzklw1_wchar_t()
{
	test_wakikixlvzuzklw1<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_of_any(const BasicString&) const noexcept
template<typename TChar>
void test_9iw14ki41xa2l89s()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("DC");
	const auto src_needle_2 = make_string<TChar>("XY");

	const auto needle_1 = bstone::BasicString<TChar>{src_needle_1.c_str(), get_memory_resource()};
	const auto needle_2 = bstone::BasicString<TChar>{src_needle_2.c_str(), get_memory_resource()};

	const auto result_1 = haystack_2.last_index_of_any(needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.last_index_of_any(needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_of_any(needle_1);
	const auto is_valid_3 = result_3 == 20;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_9iw14ki41xa2l89s_char()
{
	test_9iw14ki41xa2l89s<char>();
}

void test_9iw14ki41xa2l89s_wchar_t()
{
	test_9iw14ki41xa2l89s<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_of_any(const BasicString&, std::intptr_t) const noexcept
template<typename TChar>
void test_02x7ueu9xlpj9cg7()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("!DC");
	const auto src_needle_2 = make_string<TChar>("!XY");

	const auto needle_1 = bstone::BasicString<TChar>{src_needle_1.c_str(), get_memory_resource()};
	const auto needle_2 = bstone::BasicString<TChar>{src_needle_2.c_str(), get_memory_resource()};

	const auto result_1 = haystack_2.last_index_of_any(needle_1, 1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.last_index_of_any(needle_2, 1);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_of_any(needle_1, 1);
	const auto is_valid_3 = result_3 == 20;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_02x7ueu9xlpj9cg7_char()
{
	test_02x7ueu9xlpj9cg7<char>();
}

void test_02x7ueu9xlpj9cg7_wchar_t()
{
	test_02x7ueu9xlpj9cg7<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_of_any(const BasicString&, std::intptr_t, std::intptr_t) const noexcept
template<typename TChar>
void test_mai1fbqg6yv52fcz()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("!DC!");
	const auto src_needle_2 = make_string<TChar>("!XY!");

	const auto needle_1 = bstone::BasicString<TChar>{src_needle_1.c_str(), get_memory_resource()};
	const auto needle_2 = bstone::BasicString<TChar>{src_needle_2.c_str(), get_memory_resource()};

	const auto result_1 = haystack_2.last_index_of_any(needle_1, 1, 2);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.last_index_of_any(needle_2, 1, 2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_of_any(needle_1, 1, 2);
	const auto is_valid_3 = result_3 == 20;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_mai1fbqg6yv52fcz_char()
{
	test_mai1fbqg6yv52fcz<char>();
}

void test_mai1fbqg6yv52fcz_wchar_t()
{
	test_mai1fbqg6yv52fcz<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_of_any(BasicStringView<Char>) const noexcept
template<typename TChar>
void test_nconmvc8c0yvhnl1()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("DC");
	const auto src_needle_2 = make_string<TChar>("XY");

	const auto needle_1 = bstone::BasicStringView<TChar>{src_needle_1.c_str()};
	const auto needle_2 = bstone::BasicStringView<TChar>{src_needle_2.c_str()};

	const auto result_1 = haystack_2.last_index_of_any(needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.last_index_of_any(needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_of_any(needle_1);
	const auto is_valid_3 = result_3 == 20;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_nconmvc8c0yvhnl1_char()
{
	test_nconmvc8c0yvhnl1<char>();
}

void test_nconmvc8c0yvhnl1_wchar_t()
{
	test_nconmvc8c0yvhnl1<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_of_any(std::intptr_t, Char) const noexcept
template<typename TChar>
void test_vzbjnbbsss9zukqy()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto needle_1 = TChar{'A'};
	const auto needle_2 = TChar{'X'};

	const auto result_1 = haystack_2.last_index_of_any(0, needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.last_index_of_any(10, needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_of_any(10, needle_1);
	const auto is_valid_3 = result_3 == 21;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_vzbjnbbsss9zukqy_char()
{
	test_vzbjnbbsss9zukqy<char>();
}

void test_vzbjnbbsss9zukqy_wchar_t()
{
	test_vzbjnbbsss9zukqy<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_of_any(std::intptr_t, const Char*) const noexcept
template<typename TChar>
void test_wr2qyobp1ksvuxc6()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("DC");
	const auto src_needle_2 = make_string<TChar>("XY");

	const auto needle_1 = src_needle_1.c_str();
	const auto needle_2 = src_needle_2.c_str();

	const auto result_1 = haystack_2.last_index_of_any(0, needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.last_index_of_any(10, needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_of_any(10, needle_1);
	const auto is_valid_3 = result_3 == 20;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_wr2qyobp1ksvuxc6_char()
{
	test_wr2qyobp1ksvuxc6<char>();
}

void test_wr2qyobp1ksvuxc6_wchar_t()
{
	test_wr2qyobp1ksvuxc6<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_of_any(std::intptr_t, const Char*, std::intptr_t) const noexcept
template<typename TChar>
void test_gl0ptrl9jfl3ytwh()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("DC!");
	const auto src_needle_2 = make_string<TChar>("XY!");

	const auto needle_1 = src_needle_1.c_str();
	const auto needle_2 = src_needle_2.c_str();

	const auto result_1 = haystack_2.last_index_of_any(0, needle_1, 2);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.last_index_of_any(10, needle_2, 2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_of_any(10, needle_1, 2);
	const auto is_valid_3 = result_3 == 20;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_gl0ptrl9jfl3ytwh_char()
{
	test_gl0ptrl9jfl3ytwh<char>();
}

void test_gl0ptrl9jfl3ytwh_wchar_t()
{
	test_gl0ptrl9jfl3ytwh<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_of_any(std::intptr_t, const BasicString&) const noexcept
template<typename TChar>
void test_l3fzvn44www26uo5()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("DC");
	const auto src_needle_2 = make_string<TChar>("XY");

	const auto needle_1 = bstone::BasicString<TChar>{src_needle_1.c_str(), get_memory_resource()};
	const auto needle_2 = bstone::BasicString<TChar>{src_needle_2.c_str(), get_memory_resource()};

	const auto result_1 = haystack_2.last_index_of_any(0, needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.last_index_of_any(10, needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_of_any(10, needle_1);
	const auto is_valid_3 = result_3 == 20;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_l3fzvn44www26uo5_char()
{
	test_l3fzvn44www26uo5<char>();
}

void test_l3fzvn44www26uo5_wchar_t()
{
	test_l3fzvn44www26uo5<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_of_any(std::intptr_t, const BasicString&, std::intptr_t) const noexcept
template<typename TChar>
void test_cq8st269j2yqdn5x()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("!DC");
	const auto src_needle_2 = make_string<TChar>("!XY");

	const auto needle_1 = bstone::BasicString<TChar>{src_needle_1.c_str(), get_memory_resource()};
	const auto needle_2 = bstone::BasicString<TChar>{src_needle_2.c_str(), get_memory_resource()};

	const auto result_1 = haystack_2.last_index_of_any(0, needle_1, 1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.last_index_of_any(10, needle_2, 1);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_of_any(10, needle_1, 1);
	const auto is_valid_3 = result_3 == 20;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_cq8st269j2yqdn5x_char()
{
	test_cq8st269j2yqdn5x<char>();
}

void test_cq8st269j2yqdn5x_wchar_t()
{
	test_cq8st269j2yqdn5x<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_of_any(std::intptr_t, const BasicString&, std::intptr_t, std::intptr_t) const noexcept
template<typename TChar>
void test_0l0cii3esc23tzfj()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("!DC!");
	const auto src_needle_2 = make_string<TChar>("!XY!");

	const auto needle_1 = bstone::BasicString<TChar>{src_needle_1.c_str(), get_memory_resource()};
	const auto needle_2 = bstone::BasicString<TChar>{src_needle_2.c_str(), get_memory_resource()};

	const auto result_1 = haystack_2.last_index_of_any(0, needle_1, 1, 2);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.last_index_of_any(10, needle_2, 1, 2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_of_any(10, needle_1, 1, 2);
	const auto is_valid_3 = result_3 == 20;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_0l0cii3esc23tzfj_char()
{
	test_0l0cii3esc23tzfj<char>();
}

void test_0l0cii3esc23tzfj_wchar_t()
{
	test_0l0cii3esc23tzfj<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_of_any(std::intptr_t, BasicStringView<Char>) const noexcept
template<typename TChar>
void test_c7hhypzqsdj5u5mh()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("DC");
	const auto src_needle_2 = make_string<TChar>("XY");

	const auto needle_1 = bstone::BasicStringView<TChar>{src_needle_1.c_str()};
	const auto needle_2 = bstone::BasicStringView<TChar>{src_needle_2.c_str()};

	const auto result_1 = haystack_2.last_index_of_any(0, needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.last_index_of_any(10, needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_of_any(10, needle_1);
	const auto is_valid_3 = result_3 == 20;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_c7hhypzqsdj5u5mh_char()
{
	test_c7hhypzqsdj5u5mh<char>();
}

void test_c7hhypzqsdj5u5mh_wchar_t()
{
	test_c7hhypzqsdj5u5mh<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_not_of_any(Char) const noexcept
template<typename TChar>
void test_vpv19kni7j13vud2()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto needle_1 = TChar{'B'};
	const auto needle_2 = TChar{'X'};

	const auto result_1 = haystack_2.last_index_not_of_any(needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.last_index_not_of_any(needle_2);
	const auto is_valid_2 = result_2 == 22;

	const auto result_3 = haystack_1.last_index_not_of_any(needle_1);
	const auto is_valid_3 = result_3 == 21;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_vpv19kni7j13vud2_char()
{
	test_vpv19kni7j13vud2<char>();
}

void test_vpv19kni7j13vud2_wchar_t()
{
	test_vpv19kni7j13vud2<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_not_of_any(const Char*) const noexcept
template<typename TChar>
void test_asx1po87jt9v3nw1()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("ABCD");
	const auto src_needle_2 = make_string<TChar>("ABCDE ");
	const auto src_needle_3 = make_string<TChar>("");

	const auto needle_1 = src_needle_1.c_str();
	const auto needle_2 = src_needle_2.c_str();
	const auto needle_3 = src_needle_3.c_str();

	const auto result_1 = haystack_2.last_index_not_of_any(needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.last_index_not_of_any(needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_not_of_any(needle_3);
	const auto is_valid_3 = result_3 == 0;

	const auto result_4 = haystack_1.last_index_not_of_any(needle_1);
	const auto is_valid_4 = result_4 == 16;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_asx1po87jt9v3nw1_char()
{
	test_asx1po87jt9v3nw1<char>();
}

void test_asx1po87jt9v3nw1_wchar_t()
{
	test_asx1po87jt9v3nw1<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_not_of_any(const Char*, std::intptr_t) const noexcept
template<typename TChar>
void test_j1kvhn51ol070iyl()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("ABCD!");
	const auto src_needle_2 = make_string<TChar>("ABCDE !");
	const auto src_needle_3 = make_string<TChar>("!");

	const auto needle_1 = src_needle_1.c_str();
	const auto needle_2 = src_needle_2.c_str();
	const auto needle_3 = src_needle_3.c_str();

	const auto result_1 = haystack_2.last_index_not_of_any(needle_1, 4);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.last_index_not_of_any(needle_2, 6);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_not_of_any(needle_3, 0);
	const auto is_valid_3 = result_3 == 0;

	const auto result_4 = haystack_1.last_index_not_of_any(needle_1, 4);
	const auto is_valid_4 = result_4 == 16;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_j1kvhn51ol070iyl_char()
{
	test_j1kvhn51ol070iyl<char>();
}

void test_j1kvhn51ol070iyl_wchar_t()
{
	test_j1kvhn51ol070iyl<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_not_of_any(const BasicString&) const noexcept
template<typename TChar>
void test_bsv4o4zni6nhoe5v()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("ABCD");
	const auto src_needle_2 = make_string<TChar>("ABCDE ");
	const auto src_needle_3 = make_string<TChar>("");

	const auto needle_1 = bstone::BasicString<TChar>{src_needle_1.c_str(), get_memory_resource()};
	const auto needle_2 = bstone::BasicString<TChar>{src_needle_2.c_str(), get_memory_resource()};
	const auto needle_3 = bstone::BasicString<TChar>{src_needle_3.c_str(), get_memory_resource()};

	const auto result_1 = haystack_2.last_index_not_of_any(needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.last_index_not_of_any(needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_not_of_any(needle_3);
	const auto is_valid_3 = result_3 == 0;

	const auto result_4 = haystack_1.last_index_not_of_any(needle_1);
	const auto is_valid_4 = result_4 == 16;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_bsv4o4zni6nhoe5v_char()
{
	test_bsv4o4zni6nhoe5v<char>();
}

void test_bsv4o4zni6nhoe5v_wchar_t()
{
	test_bsv4o4zni6nhoe5v<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_not_of_any(const BasicString&, std::intptr_t) const noexcept
template<typename TChar>
void test_ti8qeb6scihydez4()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("!ABCD");
	const auto src_needle_2 = make_string<TChar>("!ABCDE ");
	const auto src_needle_3 = make_string<TChar>("!");

	const auto needle_1 = bstone::BasicString<TChar>{src_needle_1.c_str(), get_memory_resource()};
	const auto needle_2 = bstone::BasicString<TChar>{src_needle_2.c_str(), get_memory_resource()};
	const auto needle_3 = bstone::BasicString<TChar>{src_needle_3.c_str(), get_memory_resource()};

	const auto result_1 = haystack_2.last_index_not_of_any(needle_1, 1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.last_index_not_of_any(needle_2, 1);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_not_of_any(needle_3, 1);
	const auto is_valid_3 = result_3 == 0;

	const auto result_4 = haystack_1.last_index_not_of_any(needle_1, 1);
	const auto is_valid_4 = result_4 == 16;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_ti8qeb6scihydez4_char()
{
	test_ti8qeb6scihydez4<char>();
}

void test_ti8qeb6scihydez4_wchar_t()
{
	test_ti8qeb6scihydez4<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_not_of_any(const BasicString&, std::intptr_t, std::intptr_t) const noexcept
template<typename TChar>
void test_tk9itcnxuo8vk6ve()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("!ABCD!");
	const auto src_needle_2 = make_string<TChar>("!ABCDE !");
	const auto src_needle_3 = make_string<TChar>("!!");

	const auto needle_1 = bstone::BasicString<TChar>{src_needle_1.c_str(), get_memory_resource()};
	const auto needle_2 = bstone::BasicString<TChar>{src_needle_2.c_str(), get_memory_resource()};
	const auto needle_3 = bstone::BasicString<TChar>{src_needle_3.c_str(), get_memory_resource()};

	const auto result_1 = haystack_2.last_index_not_of_any(needle_1, 1, 4);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.last_index_not_of_any(needle_2, 1, 6);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_not_of_any(needle_3, 1, 0);
	const auto is_valid_3 = result_3 == 0;

	const auto result_4 = haystack_1.last_index_not_of_any(needle_1, 1, 4);
	const auto is_valid_4 = result_4 == 16;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_tk9itcnxuo8vk6ve_char()
{
	test_tk9itcnxuo8vk6ve<char>();
}

void test_tk9itcnxuo8vk6ve_wchar_t()
{
	test_tk9itcnxuo8vk6ve<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_not_of_any(BasicStringView<Char>) const noexcept
template<typename TChar>
void test_glmots1o91mv3jon()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("ABCD");
	const auto src_needle_2 = make_string<TChar>("ABCDE ");
	const auto src_needle_3 = make_string<TChar>("");

	const auto needle_1 = bstone::BasicStringView<TChar>{src_needle_1.c_str()};
	const auto needle_2 = bstone::BasicStringView<TChar>{src_needle_2.c_str()};
	const auto needle_3 = bstone::BasicStringView<TChar>{src_needle_3.c_str()};

	const auto result_1 = haystack_2.last_index_not_of_any(needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.last_index_not_of_any(needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_not_of_any(needle_3);
	const auto is_valid_3 = result_3 == 0;

	const auto result_4 = haystack_1.last_index_not_of_any(needle_1);
	const auto is_valid_4 = result_4 == 16;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_glmots1o91mv3jon_char()
{
	test_glmots1o91mv3jon<char>();
}

void test_glmots1o91mv3jon_wchar_t()
{
	test_glmots1o91mv3jon<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_not_of_any(std::intptr_t, Char) const noexcept
template<typename TChar>
void test_g07lgdro09lxfqli()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto needle_1 = TChar{'B'};
	const auto needle_2 = TChar{'X'};

	const auto result_1 = haystack_2.last_index_not_of_any(0, needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.last_index_not_of_any(4, needle_2);
	const auto is_valid_2 = result_2 == 22;

	const auto result_3 = haystack_1.last_index_not_of_any(4, needle_1);
	const auto is_valid_3 = result_3 == 21;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_g07lgdro09lxfqli_char()
{
	test_g07lgdro09lxfqli<char>();
}

void test_g07lgdro09lxfqli_wchar_t()
{
	test_g07lgdro09lxfqli<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_not_of_any(std::intptr_t, const Char*) const noexcept
template<typename TChar>
void test_ckckg9r6rykqal7f()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("ABCD");
	const auto src_needle_2 = make_string<TChar>("ABCDE ");
	const auto src_needle_3 = make_string<TChar>("");

	const auto needle_1 = src_needle_1.c_str();
	const auto needle_2 = src_needle_2.c_str();
	const auto needle_3 = src_needle_3.c_str();

	const auto result_1 = haystack_2.last_index_not_of_any(0, needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.last_index_not_of_any(4, needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_not_of_any(4, needle_3);
	const auto is_valid_3 = result_3 == 4;

	const auto result_4 = haystack_1.last_index_not_of_any(4, needle_1);
	const auto is_valid_4 = result_4 == 16;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_ckckg9r6rykqal7f_char()
{
	test_ckckg9r6rykqal7f<char>();
}

void test_ckckg9r6rykqal7f_wchar_t()
{
	test_ckckg9r6rykqal7f<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_not_of_any(std::intptr_t, const Char*, std::intptr_t) const noexcept
template<typename TChar>
void test_0jn3yi06557p0hfd()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("ABCD!");
	const auto src_needle_2 = make_string<TChar>("ABCDE !");
	const auto src_needle_3 = make_string<TChar>("!");

	const auto needle_1 = src_needle_1.c_str();
	const auto needle_2 = src_needle_2.c_str();
	const auto needle_3 = src_needle_3.c_str();

	const auto result_1 = haystack_2.last_index_not_of_any(0, needle_1, 4);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.last_index_not_of_any(4, needle_2, 6);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_not_of_any(4, needle_3, 0);
	const auto is_valid_3 = result_3 == 4;

	const auto result_4 = haystack_1.last_index_not_of_any(4, needle_1, 4);
	const auto is_valid_4 = result_4 == 16;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_0jn3yi06557p0hfd_char()
{
	test_0jn3yi06557p0hfd<char>();
}

void test_0jn3yi06557p0hfd_wchar_t()
{
	test_0jn3yi06557p0hfd<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_not_of_any(std::intptr_t, const BasicString&) const noexcept
template<typename TChar>
void test_jgpmnp7moki886n4()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("ABCD");
	const auto src_needle_2 = make_string<TChar>("ABCDE ");
	const auto src_needle_3 = make_string<TChar>("");

	const auto needle_1 = bstone::BasicString<TChar>{src_needle_1.c_str(), get_memory_resource()};
	const auto needle_2 = bstone::BasicString<TChar>{src_needle_2.c_str(), get_memory_resource()};
	const auto needle_3 = bstone::BasicString<TChar>{src_needle_3.c_str(), get_memory_resource()};

	const auto result_1 = haystack_2.last_index_not_of_any(0, needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.last_index_not_of_any(4, needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_not_of_any(4, needle_3);
	const auto is_valid_3 = result_3 == 4;

	const auto result_4 = haystack_1.last_index_not_of_any(needle_1);
	const auto is_valid_4 = result_4 == 16;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_jgpmnp7moki886n4_char()
{
	test_jgpmnp7moki886n4<char>();
}

void test_jgpmnp7moki886n4_wchar_t()
{
	test_jgpmnp7moki886n4<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_not_of_any(std::intptr_t, const BasicString&, std::intptr_t) const noexcept
template<typename TChar>
void test_sv5ar3b3k2450vjn()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("!ABCD");
	const auto src_needle_2 = make_string<TChar>("!ABCDE ");
	const auto src_needle_3 = make_string<TChar>("!");

	const auto needle_1 = bstone::BasicString<TChar>{src_needle_1.c_str(), get_memory_resource()};
	const auto needle_2 = bstone::BasicString<TChar>{src_needle_2.c_str(), get_memory_resource()};
	const auto needle_3 = bstone::BasicString<TChar>{src_needle_3.c_str(), get_memory_resource()};

	const auto result_1 = haystack_2.last_index_not_of_any(0, needle_1, 1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.last_index_not_of_any(4, needle_2, 1);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_not_of_any(4, needle_3, 1);
	const auto is_valid_3 = result_3 == 4;

	const auto result_4 = haystack_1.last_index_not_of_any(4, needle_1, 1);
	const auto is_valid_4 = result_4 == 16;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_sv5ar3b3k2450vjn_char()
{
	test_sv5ar3b3k2450vjn<char>();
}

void test_sv5ar3b3k2450vjn_wchar_t()
{
	test_sv5ar3b3k2450vjn<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_not_of_any(std::intptr_t, const BasicString&, std::intptr_t, std::intptr_t) const noexcept
template<typename TChar>
void test_w108m1syb9hxjchd()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("!ABCD!");
	const auto src_needle_2 = make_string<TChar>("!ABCDE !");
	const auto src_needle_3 = make_string<TChar>("!!");

	const auto needle_1 = bstone::BasicString<TChar>{src_needle_1.c_str(), get_memory_resource()};
	const auto needle_2 = bstone::BasicString<TChar>{src_needle_2.c_str(), get_memory_resource()};
	const auto needle_3 = bstone::BasicString<TChar>{src_needle_3.c_str(), get_memory_resource()};

	const auto result_1 = haystack_2.last_index_not_of_any(0, needle_1, 1, 4);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.last_index_not_of_any(4, needle_2, 1, 6);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_not_of_any(4, needle_3, 1, 0);
	const auto is_valid_3 = result_3 == 4;

	const auto result_4 = haystack_1.last_index_not_of_any(4, needle_1, 1, 4);
	const auto is_valid_4 = result_4 == 16;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_w108m1syb9hxjchd_char()
{
	test_w108m1syb9hxjchd<char>();
}

void test_w108m1syb9hxjchd_wchar_t()
{
	test_w108m1syb9hxjchd<wchar_t>();
}

// ==========================================================================

// std::intptr_t last_index_not_of_any(std::intptr_t, BasicStringView<Char>) const noexcept
template<typename TChar>
void test_wejrnhb9vianv8ds()
{
	const auto src_haystack = make_string<TChar>("ABCDABCDABDE ABC ABCDAB");

	const auto haystack_1 = bstone::BasicString<TChar>{src_haystack.c_str(), get_memory_resource()};
	const auto haystack_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto src_needle_1 = make_string<TChar>("ABCD");
	const auto src_needle_2 = make_string<TChar>("ABCDE ");
	const auto src_needle_3 = make_string<TChar>("");

	const auto needle_1 = bstone::BasicStringView<TChar>{src_needle_1.c_str()};
	const auto needle_2 = bstone::BasicStringView<TChar>{src_needle_2.c_str()};
	const auto needle_3 = bstone::BasicStringView<TChar>{src_needle_3.c_str()};

	const auto result_1 = haystack_2.last_index_not_of_any(0, needle_1);
	const auto is_valid_1 = result_1 < 0;

	const auto result_2 = haystack_1.last_index_not_of_any(4, needle_2);
	const auto is_valid_2 = result_2 < 0;

	const auto result_3 = haystack_1.last_index_not_of_any(4, needle_3);
	const auto is_valid_3 = result_3 == 4;

	const auto result_4 = haystack_1.last_index_not_of_any(4, needle_1);
	const auto is_valid_4 = result_4 == 16;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_wejrnhb9vianv8ds_char()
{
	test_wejrnhb9vianv8ds<char>();
}

void test_wejrnhb9vianv8ds_wchar_t()
{
	test_wejrnhb9vianv8ds<wchar_t>();
}

// ==========================================================================

// bool contains(Char) const
template<typename TChar>
void test_2kjeinl8iy7f2cw0()
{
	const auto src_string = make_string<TChar>("string");

	const auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto result_1 = string_1.contains(TChar{'i'});
	const auto result_2 = !string_1.contains(TChar{'a'});
	const auto result_3 = !string_2.contains(TChar{'a'});

	const auto is_valid =
		result_1 &&
		result_2 &&
		result_3 &&
		true;

	tester.check(is_valid);
}

void test_2kjeinl8iy7f2cw0_char()
{
	test_2kjeinl8iy7f2cw0<char>();
}

void test_2kjeinl8iy7f2cw0_wchar_t()
{
	test_2kjeinl8iy7f2cw0<wchar_t>();
}

// ==========================================================================

// bool contains(const Char*) const
template<typename TChar>
void test_xfpj3dyh5ejh1gb9()
{
	const auto src_string = make_string<TChar>("ABC ABCDAB ABCDABCDABDE");
	const auto src_substring_1 = make_string<TChar>("ABCDABD");
	const auto src_substring_2 = make_string<TChar>("ABCDABd");
	const auto src_substring_3 = make_string<TChar>("");
	const auto src_substring_4 = make_string<TChar>("ABC ABCDAB ABCDABCDABDE!");

	const auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto result_1 = string_1.contains(src_substring_1.c_str());
	const auto result_2 = !string_1.contains(src_substring_2.c_str());
	const auto result_3 = string_1.contains(src_substring_3.c_str());
	const auto result_4 = !string_1.contains(src_substring_4.c_str());
	const auto result_5 = !string_2.contains(src_substring_1.c_str());

	const auto is_valid =
		result_1 &&
		result_2 &&
		result_3 &&
		result_4 &&
		result_5 &&
		true;

	tester.check(is_valid);
}

void test_xfpj3dyh5ejh1gb9_char()
{
	test_xfpj3dyh5ejh1gb9<char>();
}

void test_xfpj3dyh5ejh1gb9_wchar_t()
{
	test_xfpj3dyh5ejh1gb9<wchar_t>();
}

// ==========================================================================

// bool contains(const Char*, std::intptr_t) const
template<typename TChar>
void test_5fsnfktj1zsbbd7d()
{
	const auto src_string = make_string<TChar>("ABC ABCDAB ABCDABCDABDE");
	const auto src_substring_1 = make_string<TChar>("ABCDABD!");
	const auto src_substring_2 = make_string<TChar>("ABCDABd!");
	const auto src_substring_3 = make_string<TChar>("!");
	const auto src_substring_4 = make_string<TChar>("ABC ABCDAB ABCDABCDABDE!!");

	const auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto result_1 = string_1.contains(src_substring_1.c_str(), 7);
	const auto result_2 = !string_1.contains(src_substring_2.c_str(), 7);
	const auto result_3 = string_1.contains(src_substring_3.c_str(), 0);
	const auto result_4 = !string_1.contains(src_substring_4.c_str(), 24);
	const auto result_5 = !string_2.contains(src_substring_1.c_str(), 7);

	const auto is_valid =
		result_1 &&
		result_2 &&
		result_3 &&
		result_4 &&
		result_5 &&
		true;

	tester.check(is_valid);
}

void test_5fsnfktj1zsbbd7d_char()
{
	test_5fsnfktj1zsbbd7d<char>();
}

void test_5fsnfktj1zsbbd7d_wchar_t()
{
	test_5fsnfktj1zsbbd7d<wchar_t>();
}

// ==========================================================================

// bool contains(const BasicString&) const
template<typename TChar>
void test_zlwjynibncqbogvz()
{
	const auto src_string = make_string<TChar>("ABC ABCDAB ABCDABCDABDE");
	const auto src_substring_1 = make_string<TChar>("ABCDABD");
	const auto src_substring_2 = make_string<TChar>("ABCDABd");
	const auto src_substring_3 = make_string<TChar>("");
	const auto src_substring_4 = make_string<TChar>("ABC ABCDAB ABCDABCDABDE!");

	const auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto substring_1 = bstone::BasicString<TChar>{src_substring_1.c_str(), get_memory_resource()};
	const auto substring_2 = bstone::BasicString<TChar>{src_substring_2.c_str(), get_memory_resource()};
	const auto substring_3 = bstone::BasicString<TChar>{src_substring_3.c_str(), get_memory_resource()};
	const auto substring_4 = bstone::BasicString<TChar>{src_substring_4.c_str(), get_memory_resource()};

	const auto result_1 = string_1.contains(substring_1);
	const auto result_2 = !string_1.contains(substring_2);
	const auto result_3 = string_1.contains(substring_3);
	const auto result_4 = !string_1.contains(substring_4);
	const auto result_5 = !string_2.contains(substring_1);

	const auto is_valid =
		result_1 &&
		result_2 &&
		result_3 &&
		result_4 &&
		result_5 &&
		true;

	tester.check(is_valid);
}

void test_zlwjynibncqbogvz_char()
{
	test_zlwjynibncqbogvz<char>();
}

void test_zlwjynibncqbogvz_wchar_t()
{
	test_zlwjynibncqbogvz<wchar_t>();
}

// bool contains(BasicStringView<Char>) const noexcept
template<typename TChar>
void test_xchdeh0wl85jhwkj()
{
	const auto src_string = make_string<TChar>("ABC ABCDAB ABCDABCDABDE");
	const auto src_substring_1 = make_string<TChar>("ABCDABD");
	const auto src_substring_2 = make_string<TChar>("ABCDABd");
	const auto src_substring_3 = make_string<TChar>("");
	const auto src_substring_4 = make_string<TChar>("ABC ABCDAB ABCDABCDABDE!");

	const auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto subview_1 = bstone::BasicStringView<TChar>{
		src_substring_1.c_str(), static_cast<std::intptr_t>(src_substring_1.size())};
	const auto subview_2 = bstone::BasicStringView<TChar>{
		src_substring_2.c_str(), static_cast<std::intptr_t>(src_substring_2.size())};
	const auto subview_3 = bstone::BasicStringView<TChar>{
		src_substring_3.c_str(), static_cast<std::intptr_t>(src_substring_3.size())};
	const auto subview_4 = bstone::BasicStringView<TChar>{
		src_substring_4.c_str(), static_cast<std::intptr_t>(src_substring_4.size())};

	const auto result_1 = string_1.contains(subview_1);
	const auto result_2 = !string_1.contains(subview_2);
	const auto result_3 = string_1.contains(subview_3);
	const auto result_4 = !string_1.contains(subview_4);
	const auto result_5 = !string_2.contains(subview_1);

	const auto is_valid =
		result_1 &&
		result_2 &&
		result_3 &&
		result_4 &&
		result_5 &&
		true;

	tester.check(is_valid);
}

void test_xchdeh0wl85jhwkj_char()
{
	test_xchdeh0wl85jhwkj<char>();
}

void test_xchdeh0wl85jhwkj_wchar_t()
{
	test_xchdeh0wl85jhwkj<wchar_t>();
}

// ==========================================================================

// bool starts_with(Char) const
template<typename TChar>
void test_yh51kenp7rb6h1z6()
{
	const auto src_string = make_string<TChar>("string");

	const auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto result_1 = string_1.starts_with(TChar{'s'});
	const auto result_2 = !string_1.starts_with(TChar{'a'});
	const auto result_3 = !string_2.starts_with(TChar{'a'});

	const auto is_valid =
		result_1 &&
		result_2 &&
		result_3 &&
		true;

	tester.check(is_valid);
}

void test_yh51kenp7rb6h1z6_char()
{
	test_yh51kenp7rb6h1z6<char>();
}

void test_yh51kenp7rb6h1z6_wchar_t()
{
	test_yh51kenp7rb6h1z6<wchar_t>();
}

// ==========================================================================

// bool starts_with(const Char*) const
template<typename TChar>
void test_3b7l72jgaqoqdusm()
{
	const auto src_string = make_string<TChar>("string");
	const auto src_substring_1 = make_string<TChar>("str");
	const auto src_substring_2 = make_string<TChar>("stt");
	const auto src_substring_3 = make_string<TChar>("");
	const auto src_substring_4 = make_string<TChar>("string1");

	const auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto result_1 = string_1.starts_with(src_substring_1.c_str());
	const auto result_2 = !string_1.starts_with(src_substring_2.c_str());
	const auto result_3 = string_1.starts_with(src_substring_3.c_str());
	const auto result_4 = !string_1.starts_with(src_substring_4.c_str());
	const auto result_5 = !string_2.starts_with(src_substring_1.c_str());

	const auto is_valid =
		result_1 &&
		result_2 &&
		result_3 &&
		result_4 &&
		result_5 &&
		true;

	tester.check(is_valid);
}

void test_3b7l72jgaqoqdusm_char()
{
	test_3b7l72jgaqoqdusm<char>();
}

void test_3b7l72jgaqoqdusm_wchar_t()
{
	test_3b7l72jgaqoqdusm<wchar_t>();
}

// ==========================================================================

// bool starts_with(const Char*, std::intptr_t) const
template<typename TChar>
void test_8h8yv81b1zgo0dmo()
{
	const auto src_string = make_string<TChar>("string");
	const auto src_substring_1 = make_string<TChar>("str1");
	const auto src_substring_2 = make_string<TChar>("stt1");
	const auto src_substring_3 = make_string<TChar>("1");
	const auto src_substring_4 = make_string<TChar>("string11");

	const auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto result_1 = string_1.starts_with(src_substring_1.c_str(), 3);
	const auto result_2 = !string_1.starts_with(src_substring_2.c_str(), 3);
	const auto result_3 = string_1.starts_with(src_substring_3.c_str(), 0);
	const auto result_4 = !string_1.starts_with(src_substring_4.c_str(), 7);
	const auto result_5 = !string_2.starts_with(src_substring_1.c_str(), 3);

	const auto is_valid =
		result_1 &&
		result_2 &&
		result_3 &&
		result_4 &&
		result_5 &&
		true;

	tester.check(is_valid);
}

void test_8h8yv81b1zgo0dmo_char()
{
	test_8h8yv81b1zgo0dmo<char>();
}

void test_8h8yv81b1zgo0dmo_wchar_t()
{
	test_8h8yv81b1zgo0dmo<wchar_t>();
}

// ==========================================================================

// bool starts_with(const BasicString&) const
template<typename TChar>
void test_ir1zkpdn71mj1nqr()
{
	const auto src_string = make_string<TChar>("string");
	const auto src_substring_1 = make_string<TChar>("str");
	const auto src_substring_2 = make_string<TChar>("stt");
	const auto src_substring_3 = make_string<TChar>("");
	const auto src_substring_4 = make_string<TChar>("string1");

	const auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto substring_1 = bstone::BasicString<TChar>{src_substring_1.c_str(), get_memory_resource()};
	const auto substring_2 = bstone::BasicString<TChar>{src_substring_2.c_str(), get_memory_resource()};
	const auto substring_3 = bstone::BasicString<TChar>{src_substring_3.c_str(), get_memory_resource()};
	const auto substring_4 = bstone::BasicString<TChar>{src_substring_4.c_str(), get_memory_resource()};

	const auto result_1 = string_1.starts_with(substring_1);
	const auto result_2 = !string_1.starts_with(substring_2);
	const auto result_3 = string_1.starts_with(substring_3);
	const auto result_4 = !string_1.starts_with(substring_4);
	const auto result_5 = !string_2.starts_with(substring_1);

	const auto is_valid =
		result_1 &&
		result_2 &&
		result_3 &&
		result_4 &&
		result_5 &&
		true;

	tester.check(is_valid);
}

void test_ir1zkpdn71mj1nqr_char()
{
	test_ir1zkpdn71mj1nqr<char>();
}

void test_ir1zkpdn71mj1nqr_wchar_t()
{
	test_ir1zkpdn71mj1nqr<wchar_t>();
}

// ==========================================================================

// bool starts_with(BasicStringView<Char> subview) const noexcept
template<typename TChar>
void test_t0a55608och9z1bh()
{
	const auto src_string = make_string<TChar>("string");
	const auto src_substring_1 = make_string<TChar>("str");
	const auto src_substring_2 = make_string<TChar>("stt");
	const auto src_substring_3 = make_string<TChar>("");
	const auto src_substring_4 = make_string<TChar>("string1");

	const auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto subview_1 = bstone::BasicStringView<TChar>{
		src_substring_1.c_str(), static_cast<std::intptr_t>(src_substring_1.size())};
	const auto subview_2 = bstone::BasicStringView<TChar>{
		src_substring_2.c_str(), static_cast<std::intptr_t>(src_substring_2.size())};
	const auto subview_3 = bstone::BasicStringView<TChar>{
		src_substring_3.c_str(), static_cast<std::intptr_t>(src_substring_3.size())};
	const auto subview_4 = bstone::BasicStringView<TChar>{
		src_substring_4.c_str(), static_cast<std::intptr_t>(src_substring_4.size())};

	const auto result_1 = string_1.starts_with(subview_1);
	const auto result_2 = !string_1.starts_with(subview_2);
	const auto result_3 = string_1.starts_with(subview_3);
	const auto result_4 = !string_1.starts_with(subview_4);
	const auto result_5 = !string_2.starts_with(subview_1);

	const auto is_valid =
		result_1 &&
		result_2 &&
		result_3 &&
		result_4 &&
		result_5 &&
		true;

	tester.check(is_valid);
}

void test_t0a55608och9z1bh_char()
{
	test_t0a55608och9z1bh<char>();
}

void test_t0a55608och9z1bh_wchar_t()
{
	test_t0a55608och9z1bh<wchar_t>();
}

// ==========================================================================

// bool ends_with(Char) const
template<typename TChar>
void test_4ofvvffd01ief55j()
{
	const auto src_string = make_string<TChar>("string");

	const auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto result_1 = string_1.ends_with(TChar{'g'});
	const auto result_2 = !string_1.ends_with(TChar{'a'});
	const auto result_3 = !string_2.ends_with(TChar{'a'});

	const auto is_valid =
		result_1 &&
		result_2 &&
		result_3 &&
		true;

	tester.check(is_valid);
}

void test_4ofvvffd01ief55j_char()
{
	test_4ofvvffd01ief55j<char>();
}

void test_4ofvvffd01ief55j_wchar_t()
{
	test_4ofvvffd01ief55j<wchar_t>();
}

// ==========================================================================

// bool ends_with(const Char*) const
template<typename TChar>
void test_zcvexidebzt7ne76()
{
	const auto src_string = make_string<TChar>("string");
	const auto src_substring_1 = make_string<TChar>("ing");
	const auto src_substring_2 = make_string<TChar>("inh");
	const auto src_substring_3 = make_string<TChar>("");
	const auto src_substring_4 = make_string<TChar>("string1");

	const auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto result_1 = string_1.ends_with(src_substring_1.c_str());
	const auto result_2 = !string_1.ends_with(src_substring_2.c_str());
	const auto result_3 = string_1.ends_with(src_substring_3.c_str());
	const auto result_4 = !string_1.ends_with(src_substring_4.c_str());
	const auto result_5 = !string_2.ends_with(src_substring_1.c_str());

	const auto is_valid =
		result_1 &&
		result_2 &&
		result_3 &&
		result_4 &&
		result_5 &&
		true;

	tester.check(is_valid);
}

void test_zcvexidebzt7ne76_char()
{
	test_zcvexidebzt7ne76<char>();
}

void test_zcvexidebzt7ne76_wchar_t()
{
	test_zcvexidebzt7ne76<wchar_t>();
}

// ==========================================================================

// bool ends_with(const Char*, std::intptr_t) const
template<typename TChar>
void test_4edwoczh7b92li3i()
{
	const auto src_string = make_string<TChar>("string");
	const auto src_substring_1 = make_string<TChar>("ing1");
	const auto src_substring_2 = make_string<TChar>("inh1");
	const auto src_substring_3 = make_string<TChar>("1");
	const auto src_substring_4 = make_string<TChar>("string11");

	const auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto result_1 = string_1.ends_with(src_substring_1.c_str(), 3);
	const auto result_2 = !string_1.ends_with(src_substring_2.c_str(), 3);
	const auto result_3 = string_1.ends_with(src_substring_3.c_str(), 0);
	const auto result_4 = !string_1.ends_with(src_substring_4.c_str(), 7);
	const auto result_5 = !string_2.ends_with(src_substring_1.c_str(), 3);

	const auto is_valid =
		result_1 &&
		result_2 &&
		result_3 &&
		result_4 &&
		result_5 &&
		true;

	tester.check(is_valid);
}

void test_4edwoczh7b92li3i_char()
{
	test_4edwoczh7b92li3i<char>();
}

void test_4edwoczh7b92li3i_wchar_t()
{
	test_4edwoczh7b92li3i<wchar_t>();
}

// ==========================================================================

// bool ends_with(const BasicString&) const
template<typename TChar>
void test_sgsr33kcxyjc53di()
{
	const auto src_string = make_string<TChar>("string");
	const auto src_substring_1 = make_string<TChar>("ing");
	const auto src_substring_2 = make_string<TChar>("inh");
	const auto src_substring_3 = make_string<TChar>("");
	const auto src_substring_4 = make_string<TChar>("string1");

	const auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto substring_1 = bstone::BasicString<TChar>{src_substring_1.c_str(), get_memory_resource()};
	const auto substring_2 = bstone::BasicString<TChar>{src_substring_2.c_str(), get_memory_resource()};
	const auto substring_3 = bstone::BasicString<TChar>{src_substring_3.c_str(), get_memory_resource()};
	const auto substring_4 = bstone::BasicString<TChar>{src_substring_4.c_str(), get_memory_resource()};

	const auto result_1 = string_1.ends_with(substring_1);
	const auto result_2 = !string_1.ends_with(substring_2);
	const auto result_3 = string_1.ends_with(substring_3);
	const auto result_4 = !string_1.ends_with(substring_4);
	const auto result_5 = !string_2.ends_with(substring_1);

	const auto is_valid =
		result_1 &&
		result_2 &&
		result_3 &&
		result_4 &&
		result_5 &&
		true;

	tester.check(is_valid);
}

void test_sgsr33kcxyjc53di_char()
{
	test_sgsr33kcxyjc53di<char>();
}

void test_sgsr33kcxyjc53di_wchar_t()
{
	test_sgsr33kcxyjc53di<wchar_t>();
}

// ==========================================================================

// bool ends_with(BasicStringView<Char> subview) const noexcept
template<typename TChar>
void test_wjpxca3alaoiv5bn()
{
	const auto src_string = make_string<TChar>("string");
	const auto src_substring_1 = make_string<TChar>("ing");
	const auto src_substring_2 = make_string<TChar>("inh");
	const auto src_substring_3 = make_string<TChar>("");
	const auto src_substring_4 = make_string<TChar>("string1");

	const auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{get_memory_resource()};

	const auto subview_1 = bstone::BasicStringView<TChar>{
		src_substring_1.c_str(), static_cast<std::intptr_t>(src_substring_1.size())};
	const auto subview_2 = bstone::BasicStringView<TChar>{
		src_substring_2.c_str(), static_cast<std::intptr_t>(src_substring_2.size())};
	const auto subview_3 = bstone::BasicStringView<TChar>{
		src_substring_3.c_str(), static_cast<std::intptr_t>(src_substring_3.size())};
	const auto subview_4 = bstone::BasicStringView<TChar>{
		src_substring_4.c_str(), static_cast<std::intptr_t>(src_substring_4.size())};

	const auto result_1 = string_1.ends_with(subview_1);
	const auto result_2 = !string_1.ends_with(subview_2);
	const auto result_3 = string_1.ends_with(subview_3);
	const auto result_4 = !string_1.ends_with(subview_4);
	const auto result_5 = !string_2.ends_with(subview_1);

	const auto is_valid =
		result_1 &&
		result_2 &&
		result_3 &&
		result_4 &&
		result_5 &&
		true;

	tester.check(is_valid);
}

void test_wjpxca3alaoiv5bn_char()
{
	test_wjpxca3alaoiv5bn<char>();
}

void test_wjpxca3alaoiv5bn_wchar_t()
{
	test_wjpxca3alaoiv5bn<wchar_t>();
}

// ==========================================================================

// static BasicString make_with_capacity(std::intptr_t)
template<typename TChar>
void test_z22rdgewv090xrn0()
{
	constexpr auto ref_capacity = 3;
	const auto string = bstone::BasicString<TChar>::make_with_capacity(ref_capacity);
	const auto data = string.get_data();
	const auto capacity = string.get_capacity();
	const auto size = string.get_size();

	const auto is_valid_1 = size == 0;
	const auto is_valid_2 = capacity == ref_capacity;
	const auto is_valid_3 = data[0] == '\0';

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_z22rdgewv090xrn0_char()
{
	test_z22rdgewv090xrn0<char>();
}

void test_z22rdgewv090xrn0_wchar_t()
{
	test_z22rdgewv090xrn0<wchar_t>();
}

// ==========================================================================

// static BasicString make_with_fill(Char, std::intptr_t)
template<typename TChar>
void test_9cdzbziw1c3bpweg()
{
	constexpr auto ref_count = 3;
	const auto string = bstone::BasicString<TChar>::make_with_fill(TChar{'1'}, ref_count);
	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == ref_count &&
		data[0] == '1' &&
		data[1] == '1' &&
		data[2] == '1' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_9cdzbziw1c3bpweg_char()
{
	test_9cdzbziw1c3bpweg<char>();
}

void test_9cdzbziw1c3bpweg_wchar_t()
{
	test_9cdzbziw1c3bpweg<wchar_t>();
}

// ==========================================================================

// BasicString<TChar> operator+(const BasicString<TChar>&, TChar)
// BasicString<TChar> operator+(TChar, const BasicString<TChar>&)
template<typename TChar>
void test_iuxbt13i4286zs3q()
{
	const auto src_string = make_string<TChar>("st");

	auto string = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto string_1 = string + TChar{'r'};
	const auto data_1 = string_1.get_data();
	const auto size_1 = string_1.get_size();

	const auto is_valid_1 =
		size_1 == 3 &&
		data_1[0] == 's' &&
		data_1[1] == 't' &&
		data_1[2] == 'r' &&
		data_1[3] == '\0';

	const auto string_2 = TChar{'r'} + string;
	const auto data_2 = string_2.get_data();
	const auto size_2 = string_2.get_size();

	const auto is_valid_2 =
		size_2 == 3 &&
		data_2[0] == 'r' &&
		data_2[1] == 's' &&
		data_2[2] == 't' &&
		data_2[3] == '\0';

	const auto is_valid = is_valid_1 | is_valid_2;

	tester.check(is_valid);
}

void test_iuxbt13i4286zs3q_char()
{
	test_iuxbt13i4286zs3q<char>();
}

void test_iuxbt13i4286zs3q_wchar_t()
{
	test_iuxbt13i4286zs3q<wchar_t>();
}

// ==========================================================================

// BasicString<TChar> operator+(const BasicString<TChar>&, const TChar*)
// BasicString<TChar> operator+(const TChar*, const BasicString<TChar>&)
template<typename TChar>
void test_6e8tcyyagtlv1lwp()
{
	const auto src_string_1 = make_string<TChar>("11");
	const auto src_string_2 = make_string<TChar>("22");

	auto string = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto string_1 = string + src_string_2.c_str();
	const auto data_1 = string_1.get_data();
	const auto size_1 = string_1.get_size();

	const auto is_valid_1 =
		size_1 == 4 &&
		data_1[0] == '1' &&
		data_1[1] == '1' &&
		data_1[2] == '2' &&
		data_1[3] == '2' &&
		data_1[4] == '\0';

	const auto string_2 = src_string_2.c_str() + string;
	const auto data_2 = string_2.get_data();
	const auto size_2 = string_2.get_size();

	const auto is_valid_2 =
		size_2 == 4 &&
		data_2[0] == '2' &&
		data_2[1] == '2' &&
		data_2[2] == '1' &&
		data_2[3] == '1' &&
		data_2[4] == '\0';

	const auto is_valid = is_valid_1 | is_valid_2;

	tester.check(is_valid);
}

void test_6e8tcyyagtlv1lwp_char()
{
	test_6e8tcyyagtlv1lwp<char>();
}

void test_6e8tcyyagtlv1lwp_wchar_t()
{
	test_6e8tcyyagtlv1lwp<wchar_t>();
}

// ==========================================================================

// BasicString<TChar> operator+(const BasicString<TChar>&, const BasicString<TChar>&)
template<typename TChar>
void test_3jbkzlrs5mm9yp9r()
{
	const auto src_string_1 = make_string<TChar>("11");
	const auto src_string_2 = make_string<TChar>("22");

	const auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};

	const auto string = string_1 + string_2;
	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == 4 &&
		data[0] == '1' &&
		data[1] == '1' &&
		data[2] == '2' &&
		data[3] == '2' &&
		data[4] == '\0';

	tester.check(is_valid);
}

void test_3jbkzlrs5mm9yp9r_char()
{
	test_3jbkzlrs5mm9yp9r<char>();
}

void test_3jbkzlrs5mm9yp9r_wchar_t()
{
	test_3jbkzlrs5mm9yp9r<wchar_t>();
}

// ==========================================================================

// BasicString<TChar>& operator+=(BasicString<TChar>&, TChar)
template<typename TChar>
void test_7wud7k885palma6i()
{
	const auto src_string = make_string<TChar>("st");

	auto string_1 = bstone::BasicString<TChar>{src_string.c_str(), get_memory_resource()};

	const auto& string = string_1 += TChar{'r'};
	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == 3 &&
		data[0] == 's' &&
		data[1] == 't' &&
		data[2] == 'r' &&
		data[3] == '\0';

	tester.check(is_valid);
}

void test_7wud7k885palma6i_char()
{
	test_7wud7k885palma6i<char>();
}

void test_7wud7k885palma6i_wchar_t()
{
	test_7wud7k885palma6i<wchar_t>();
}

// ==========================================================================

// BasicString<TChar>& operator+=(BasicString<TChar>&, const TChar*)
template<typename TChar>
void test_jl07ic6eb87d2rfd()
{
	const auto src_string_1 = make_string<TChar>("11");
	const auto src_string_2 = make_string<TChar>("22");

	auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto& string = string_1 += src_string_2.c_str();
	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == 4 &&
		data[0] == '1' &&
		data[1] == '1' &&
		data[2] == '2' &&
		data[3] == '2' &&
		data[4] == '\0';

	tester.check(is_valid);
}

void test_jl07ic6eb87d2rfd_char()
{
	test_jl07ic6eb87d2rfd<char>();
}

void test_jl07ic6eb87d2rfd_wchar_t()
{
	test_jl07ic6eb87d2rfd<wchar_t>();
}

// ==========================================================================

// BasicString<TChar>& operator+=(BasicString<TChar>&, const BasicString<TChar>&)
template<typename TChar>
void test_6q6l42edz7ua050u()
{
	const auto src_string_1 = make_string<TChar>("11");
	const auto src_string_2 = make_string<TChar>("22");

	auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};

	const auto& string = string_1 += string_2;
	const auto data = string.get_data();
	const auto size = string.get_size();

	const auto is_valid =
		size == 4 &&
		data[0] == '1' &&
		data[1] == '1' &&
		data[2] == '2' &&
		data[3] == '2' &&
		data[4] == '\0';

	tester.check(is_valid);
}

void test_6q6l42edz7ua050u_char()
{
	test_6q6l42edz7ua050u<char>();
}

void test_6q6l42edz7ua050u_wchar_t()
{
	test_6q6l42edz7ua050u<wchar_t>();
}

// ==========================================================================

// bool operator==(const BasicString<TChar>&, const TChar*)
template<typename TChar>
void test_wa1ouvxiomiaya6k()
{
	const auto src_string_1 = make_string<TChar>("string");
	const auto src_string_2 = make_string<TChar>("!string");

	const auto string = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto is_valid_1 = string == src_string_1.c_str();
	const auto is_valid_2 = !(string == src_string_2.c_str());
	const auto is_valid = is_valid_1 && is_valid_2;

	tester.check(is_valid);
}

void test_wa1ouvxiomiaya6k_char()
{
	test_wa1ouvxiomiaya6k<char>();
}

void test_wa1ouvxiomiaya6k_wchar_t()
{
	test_wa1ouvxiomiaya6k<wchar_t>();
}

// ==========================================================================

// bool operator==(const TChar*, const BasicString<TChar>&)
template<typename TChar>
void test_38d4kl3up83frf73()
{
	const auto src_string_1 = make_string<TChar>("string");
	const auto src_string_2 = make_string<TChar>("!string");

	const auto string = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto is_valid_1 = src_string_1.c_str() == string;
	const auto is_valid_2 = !(src_string_2.c_str() == string);
	const auto is_valid = is_valid_1 && is_valid_2;

	tester.check(is_valid);
}

void test_38d4kl3up83frf73_char()
{
	test_38d4kl3up83frf73<char>();
}

void test_38d4kl3up83frf73_wchar_t()
{
	test_38d4kl3up83frf73<wchar_t>();
}

// ==========================================================================

// bool operator==(const BasicString<TChar>&, const BasicString<TChar>&)
template<typename TChar>
void test_8998irashz12kvu4()
{
	const auto src_string_1 = make_string<TChar>("string");
	const auto src_string_2 = make_string<TChar>("!string");

	const auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};

	const auto is_valid_1 = string_1 == string_1;
	const auto is_valid_2 = !(string_1 == string_2);
	const auto is_valid = is_valid_1 && is_valid_2;

	tester.check(is_valid);
}

void test_8998irashz12kvu4_char()
{
	test_8998irashz12kvu4<char>();
}

void test_8998irashz12kvu4_wchar_t()
{
	test_8998irashz12kvu4<wchar_t>();
}

// ==========================================================================

// bool operator!=(const BasicString<TChar>&, const TChar*)
template<typename TChar>
void test_abmys46p8fnggnf6()
{
	const auto src_string_1 = make_string<TChar>("string");
	const auto src_string_2 = make_string<TChar>("!string");

	const auto string = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto is_valid_1 = string != src_string_2.c_str();
	const auto is_valid_2 = !(string != src_string_1.c_str());
	const auto is_valid = is_valid_1 && is_valid_2;

	tester.check(is_valid);
}

void test_abmys46p8fnggnf6_char()
{
	test_abmys46p8fnggnf6<char>();
}

void test_abmys46p8fnggnf6_wchar_t()
{
	test_abmys46p8fnggnf6<wchar_t>();
}

// ==========================================================================

// bool operator!=(const TChar*, const BasicString<TChar>&)
template<typename TChar>
void test_89a4j82uthfo7qmt()
{
	const auto src_string_1 = make_string<TChar>("string");
	const auto src_string_2 = make_string<TChar>("!string");

	const auto string = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};

	const auto is_valid_1 = src_string_2.c_str() != string;
	const auto is_valid_2 = !(src_string_1.c_str() != string);
	const auto is_valid = is_valid_1 && is_valid_2;

	tester.check(is_valid);
}

void test_89a4j82uthfo7qmt_char()
{
	test_89a4j82uthfo7qmt<char>();
}

void test_89a4j82uthfo7qmt_wchar_t()
{
	test_89a4j82uthfo7qmt<wchar_t>();
}

// ==========================================================================

// bool operator!=(const BasicString<TChar>&, const BasicString<TChar>&)
template<typename TChar>
void test_ebpfgczocfwrt97s()
{
	const auto src_string_1 = make_string<TChar>("string");
	const auto src_string_2 = make_string<TChar>("!string");

	const auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};

	const auto is_valid_1 = string_1 != string_2;
	const auto is_valid_2 = !(string_1 != string_1);
	const auto is_valid = is_valid_1 && is_valid_2;

	tester.check(is_valid);
}

void test_ebpfgczocfwrt97s_char()
{
	test_ebpfgczocfwrt97s<char>();
}

void test_ebpfgczocfwrt97s_wchar_t()
{
	test_ebpfgczocfwrt97s<wchar_t>();
}

// ==========================================================================

// bool operator<(const BasicString<TChar>&, const TChar*)
template<typename TChar>
void test_6n5n6z3q7l4q5pmq()
{
	const auto src_string_1 = make_string<TChar>("string");
	const auto src_string_2 = make_string<TChar>("t");

	const auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};

	const auto is_valid_1 = string_1 < src_string_2.c_str();
	const auto is_valid_2 = !(string_2 < src_string_1.c_str());
	const auto is_valid = is_valid_1 && is_valid_2;

	tester.check(is_valid);
}

void test_6n5n6z3q7l4q5pmq_char()
{
	test_6n5n6z3q7l4q5pmq<char>();
}

void test_6n5n6z3q7l4q5pmq_wchar_t()
{
	test_6n5n6z3q7l4q5pmq<wchar_t>();
}

// ==========================================================================

// bool operator<(const TChar*, const BasicString<TChar>&)
template<typename TChar>
void test_f62pd62zh6fbmxkp()
{
	const auto src_string_1 = make_string<TChar>("string");
	const auto src_string_2 = make_string<TChar>("t");

	const auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};

	const auto is_valid_1 = src_string_1.c_str() < string_2;
	const auto is_valid_2 = !(src_string_2.c_str() < string_1);
	const auto is_valid = is_valid_1 && is_valid_2;

	tester.check(is_valid);
}

void test_f62pd62zh6fbmxkp_char()
{
	test_f62pd62zh6fbmxkp<char>();
}

void test_f62pd62zh6fbmxkp_wchar_t()
{
	test_f62pd62zh6fbmxkp<wchar_t>();
}

// ==========================================================================

// bool operator<(const BasicString<TChar>&, const BasicString<TChar>&)
template<typename TChar>
void test_8hp2ve1oemz7oqam()
{
	const auto src_string_1 = make_string<TChar>("string");
	const auto src_string_2 = make_string<TChar>("t");

	const auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};

	const auto is_valid_1 = string_1 < string_2;
	const auto is_valid_2 = !(string_2 < string_1);
	const auto is_valid = is_valid_1 && is_valid_2;

	tester.check(is_valid);
}

void test_8hp2ve1oemz7oqam_char()
{
	test_8hp2ve1oemz7oqam<char>();
}

void test_8hp2ve1oemz7oqam_wchar_t()
{
	test_8hp2ve1oemz7oqam<wchar_t>();
}

// ==========================================================================

// bool operator<=(const BasicString<TChar>&, const TChar*)
template<typename TChar>
void test_elv3ya3zycpz1zrm()
{
	const auto src_string_1 = make_string<TChar>("string");
	const auto src_string_2 = make_string<TChar>("t");

	const auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};

	const auto is_valid_1 = string_1 <= src_string_1.c_str();
	const auto is_valid_2 = string_1 <= src_string_2.c_str();
	const auto is_valid_3 = !(string_2 <= src_string_1.c_str());
	const auto is_valid = is_valid_1 && is_valid_2 && is_valid_3;

	tester.check(is_valid);
}

void test_elv3ya3zycpz1zrm_char()
{
	test_elv3ya3zycpz1zrm<char>();
}

void test_elv3ya3zycpz1zrm_wchar_t()
{
	test_elv3ya3zycpz1zrm<wchar_t>();
}

// ==========================================================================

// bool operator<=(const TChar*, const BasicString<TChar>&)
template<typename TChar>
void test_dqke1gl5cdg6f8uh()
{
	const auto src_string_1 = make_string<TChar>("string");
	const auto src_string_2 = make_string<TChar>("t");

	const auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};

	const auto is_valid_1 = src_string_1.c_str() <= string_1;
	const auto is_valid_2 = src_string_1.c_str() <= string_2;
	const auto is_valid_3 = !(src_string_2.c_str() <= string_1);
	const auto is_valid = is_valid_1 && is_valid_2 && is_valid_3;

	tester.check(is_valid);
}

void test_dqke1gl5cdg6f8uh_char()
{
	test_dqke1gl5cdg6f8uh<char>();
}

void test_dqke1gl5cdg6f8uh_wchar_t()
{
	test_dqke1gl5cdg6f8uh<wchar_t>();
}

// ==========================================================================

// bool operator<=(const BasicString<TChar>&, const BasicString<TChar>&)
template<typename TChar>
void test_o92tymhm176c7205()
{
	const auto src_string_1 = make_string<TChar>("string");
	const auto src_string_2 = make_string<TChar>("t");

	const auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};

	const auto is_valid_1 = string_1 <= string_1;
	const auto is_valid_2 = string_1 <= string_2;
	const auto is_valid_3 = !(string_2 <= string_1);
	const auto is_valid = is_valid_1 && is_valid_2 && is_valid_3;

	tester.check(is_valid);
}

void test_o92tymhm176c7205_char()
{
	test_o92tymhm176c7205<char>();
}

void test_o92tymhm176c7205_wchar_t()
{
	test_o92tymhm176c7205<wchar_t>();
}

// ==========================================================================

// bool operator>(const BasicString<TChar>&, const TChar*)
template<typename TChar>
void test_2v4zikw4talgf0iq()
{
	const auto src_string_1 = make_string<TChar>("t");
	const auto src_string_2 = make_string<TChar>("string");

	const auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};

	const auto is_valid_1 = string_1 > src_string_2.c_str();
	const auto is_valid_2 = !(string_2 > src_string_1.c_str());
	const auto is_valid = is_valid_1 && is_valid_2;

	tester.check(is_valid);
}

void test_2v4zikw4talgf0iq_char()
{
	test_2v4zikw4talgf0iq<char>();
}

void test_2v4zikw4talgf0iq_wchar_t()
{
	test_2v4zikw4talgf0iq<wchar_t>();
}

// ==========================================================================

// bool operator>(const TChar*, const BasicString<TChar>&)
template<typename TChar>
void test_bns5on3wcyqzchl7()
{
	const auto src_string_1 = make_string<TChar>("t");
	const auto src_string_2 = make_string<TChar>("string");

	const auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};

	const auto is_valid_1 = src_string_1.c_str() > string_2;
	const auto is_valid_2 = !(src_string_2.c_str() > string_1);
	const auto is_valid = is_valid_1 && is_valid_2;

	tester.check(is_valid);
}

void test_bns5on3wcyqzchl7_char()
{
	test_bns5on3wcyqzchl7<char>();
}

void test_bns5on3wcyqzchl7_wchar_t()
{
	test_bns5on3wcyqzchl7<wchar_t>();
}

// ==========================================================================

// bool operator>(const BasicString<TChar>&, const BasicString<TChar>&)
template<typename TChar>
void test_kf5uef6fmg5aer1t()
{
	const auto src_string_1 = make_string<TChar>("t");
	const auto src_string_2 = make_string<TChar>("string");

	const auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};

	const auto is_valid_1 = string_1 > string_2;
	const auto is_valid_2 = !(string_2 > string_1);
	const auto is_valid = is_valid_1 && is_valid_2;

	tester.check(is_valid);
}

void test_kf5uef6fmg5aer1t_char()
{
	test_kf5uef6fmg5aer1t<char>();
}

void test_kf5uef6fmg5aer1t_wchar_t()
{
	test_kf5uef6fmg5aer1t<wchar_t>();
}

// ==========================================================================

// bool operator>=(const BasicString<TChar>&, const TChar*)
template<typename TChar>
void test_qyzqy4z2lpyyplcy()
{
	const auto src_string_1 = make_string<TChar>("t");
	const auto src_string_2 = make_string<TChar>("string");

	const auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};

	const auto is_valid_1 = string_1 >= src_string_1.c_str();
	const auto is_valid_2 = string_1 >= src_string_2.c_str();
	const auto is_valid_3 = !(string_2 >= src_string_1.c_str());
	const auto is_valid = is_valid_1 && is_valid_2 && is_valid_3;

	tester.check(is_valid);
}

void test_qyzqy4z2lpyyplcy_char()
{
	test_qyzqy4z2lpyyplcy<char>();
}

void test_qyzqy4z2lpyyplcy_wchar_t()
{
	test_qyzqy4z2lpyyplcy<wchar_t>();
}

// ==========================================================================

// bool operator>=(const TChar*, const BasicString<TChar>&)
template<typename TChar>
void test_u2gzkd517wp1ek8d()
{
	const auto src_string_1 = make_string<TChar>("t");
	const auto src_string_2 = make_string<TChar>("string");

	const auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};

	const auto is_valid_1 = src_string_1.c_str() >= string_1;
	const auto is_valid_2 = src_string_1.c_str() >= string_2;
	const auto is_valid_3 = !(src_string_2.c_str() >= string_1);
	const auto is_valid = is_valid_1 && is_valid_2 && is_valid_3;

	tester.check(is_valid);
}

void test_u2gzkd517wp1ek8d_char()
{
	test_u2gzkd517wp1ek8d<char>();
}

void test_u2gzkd517wp1ek8d_wchar_t()
{
	test_u2gzkd517wp1ek8d<wchar_t>();
}

// ==========================================================================

// bool operator>=(const BasicString<TChar>&, const BasicString<TChar>&)
template<typename TChar>
void test_hnph2udi2dr63ntf()
{
	const auto src_string_1 = make_string<TChar>("t");
	const auto src_string_2 = make_string<TChar>("string");

	const auto string_1 = bstone::BasicString<TChar>{src_string_1.c_str(), get_memory_resource()};
	const auto string_2 = bstone::BasicString<TChar>{src_string_2.c_str(), get_memory_resource()};

	const auto is_valid_1 = string_1 >= string_1;
	const auto is_valid_2 = string_1 >= string_2;
	const auto is_valid_3 = !(string_2 >= string_1);
	const auto is_valid = is_valid_1 && is_valid_2 && is_valid_3;

	tester.check(is_valid);
}

void test_hnph2udi2dr63ntf_char()
{
	test_hnph2udi2dr63ntf<char>();
}

void test_hnph2udi2dr63ntf_wchar_t()
{
	test_hnph2udi2dr63ntf<wchar_t>();
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_basic_string();
		register_get_data();
		register_get_capacity();
		register_get_size();
		register_is_empty();
		register_begin();
		register_end();
		register_cbegin();
		register_cend();
		register_subscript_operator();
		register_get_front();
		register_get_back();
		register_reserve();
		register_resize();
		register_clear();
		register_swap();
		register_assign();
		register_append();
		register_insert();
		register_erase();
		register_push_back();
		register_pop_back();
		register_get_substring();
		register_get_string_view();
		register_operator_string_view();
		register_compare();
		register_index_of();
		register_last_index_of();
		register_index_of_any();
		register_index_not_of_any();
		register_last_index_of_any();
		register_last_index_not_of_any();
		register_contains();
		register_starts_with();
		register_ends_with();
		register_make_with_capacity();
		register_make_with_fill();
		register_operator_add();
		register_operator_add_assign();
		register_operator_equal_to();
		register_operator_not_equal_to();
		register_operator_less_than();
		register_operator_less_than_or_equal_to();
		register_operator_greater_than();
		register_operator_greater_than_or_equal_to();
	}

private:
	void register_basic_string()
	{
		tester.register_test("BasicString#ak0wbkeaar09ktgo", test_l46dsv6fbj3zravq_char);
		tester.register_test("BasicString#hqz2omeqyo2jt3h0", test_l46dsv6fbj3zravq_wchar_t);

		tester.register_test("BasicString#1mxtipnum77whwpi", test_8gl9zxe1fn4o78da_char);
		tester.register_test("BasicString#tyy7a02kmrzwq6ig", test_8gl9zxe1fn4o78da_wchar_t);

		tester.register_test("BasicString#y26y5gndi57plsuq", test_i3gssmuub094jhxi_char);
		tester.register_test("BasicString#2y6dettv9l73o7bb", test_i3gssmuub094jhxi_wchar_t);

		tester.register_test("BasicString#8a3wbs3a0vqrjv9b", test_f6nsg7ws4in9ypvw_char);
		tester.register_test("BasicString#f8v6itds4nl1vmt2", test_f6nsg7ws4in9ypvw_wchar_t);

		tester.register_test("BasicString#vs2lf0by9470jk3k", test_exn9k1icofwfnqm3_char);
		tester.register_test("BasicString#26ji9p1pztdmpftx", test_exn9k1icofwfnqm3_wchar_t);

		tester.register_test("BasicString#8rtrnzc4zqg21wbx", test_3wd7p3xrpf24pbpm_char);
		tester.register_test("BasicString#gatcxik07tvhfcke", test_3wd7p3xrpf24pbpm_wchar_t);

		tester.register_test("BasicString#4ff61cupleixehqb", test_gxlnmigx4uzpsxaw_char);
		tester.register_test("BasicString#dj0rnqhy8hl2633a", test_gxlnmigx4uzpsxaw_wchar_t);

		tester.register_test("BasicString#2ha9d8vusui0o8t5", test_hv6wr0cirp1u2r2d_char);
		tester.register_test("BasicString#k7hv45zvt2eu83ve", test_hv6wr0cirp1u2r2d_wchar_t);

		tester.register_test("BasicString#ibuadpn7y5uyoklc", test_sn0okd13g7zxzrhu_char);
		tester.register_test("BasicString#h2m9zqq43xkagwg0", test_sn0okd13g7zxzrhu_wchar_t);

		tester.register_test("BasicString#mwfxsn2dfs9es0x7", test_q19fgblkse5v8wzx_char);
		tester.register_test("BasicString#ofzs9okdtht9ue5p", test_q19fgblkse5v8wzx_wchar_t);

		tester.register_test("BasicString#1qru4lwr35pv98jq", test_8t2m6w3w79fic3th_char);
		tester.register_test("BasicString#1hvikj960ei0pvxp", test_8t2m6w3w79fic3th_wchar_t);

		tester.register_test("BasicString#s31bew1apqeq6r1a", test_asingpo2nvazj5yd_char);
		tester.register_test("BasicString#k0pt55bepc18lv7e", test_asingpo2nvazj5yd_wchar_t);

		tester.register_test("BasicString#8efaea5e5y7imxoh", test_q3byhe2g3l5j9x6c_char);
		tester.register_test("BasicString#sq8nabf78wv08tbp", test_q3byhe2g3l5j9x6c_wchar_t);

		tester.register_test("BasicString#i49mz2bjtkm9kikw", test_in9b2l1ltmse0oph_char);
		tester.register_test("BasicString#4hsq0eylsfz9nzdx", test_in9b2l1ltmse0oph_wchar_t);

		tester.register_test("BasicString#9sw2eyo1ciz2vwde", test_nobp510mnagq6ywj_char);
		tester.register_test("BasicString#um0g2jsm9lx67m5h", test_nobp510mnagq6ywj_wchar_t);

		tester.register_test("BasicString#1xc8tvzwt4dhmupe", test_621cu7goqwtwyv1o_char);
		tester.register_test("BasicString#rdoz8vjflj3p1uso", test_621cu7goqwtwyv1o_wchar_t);
	}

	void register_get_data()
	{
		tester.register_test("BasicString#8ry9s3avebp314x2", test_xfpsa3d6zmsisrc1_char);
		tester.register_test("BasicString#qct9ui1sapgeheeu", test_xfpsa3d6zmsisrc1_wchar_t);

		tester.register_test("BasicString#3orlfixkx7qhv285", test_xxz5fefaennocbmn_char);
		tester.register_test("BasicString#6n9wy86zrbqpennt", test_xxz5fefaennocbmn_wchar_t);
	}

	void register_get_capacity()
	{
		tester.register_test("BasicString#d20bmyf4xfwwq4uz", test_z1d46bdmylazmr5d_char);
		tester.register_test("BasicString#rszecop6sq1jsrnf", test_z1d46bdmylazmr5d_wchar_t);
	}

	void register_get_size()
	{
		tester.register_test("BasicString#oquy55v9hainmaex", test_9b5doqxmcsngrghn_char);
		tester.register_test("BasicString#hr3pb49kl0cmsktt", test_9b5doqxmcsngrghn_wchar_t);
	}

	void register_is_empty()
	{
		tester.register_test("BasicString#40d9ok2zzci0q0r4", test_9nzsk1kjog5jxvy9_char);
		tester.register_test("BasicString#1hfmzgcye4vuug5s", test_9nzsk1kjog5jxvy9_wchar_t);
	}

	void register_begin()
	{
		tester.register_test("BasicString#zzyf6xj8oqnne3kf", test_194mgikj8lq17j5y_char);
		tester.register_test("BasicString#t9aibm4pgo9qvkur", test_194mgikj8lq17j5y_wchar_t);

		tester.register_test("BasicString#0rk7cg0mlw5or09d", test_f4iwc1n12d4sj1qg_char);
		tester.register_test("BasicString#416x0oizuu0klrbl", test_f4iwc1n12d4sj1qg_wchar_t);
	}

	void register_end()
	{
		tester.register_test("BasicString#29t6mlq278y1isl7", test_fm33lb2a6ar3j1j1_char);
		tester.register_test("BasicString#agr958zticuuvaey", test_fm33lb2a6ar3j1j1_wchar_t);

		tester.register_test("BasicString#86t2iu2g1tmn3eft", test_dpxsbiqqf3y6crin_char);
		tester.register_test("BasicString#onpzliazbkxrniwe", test_dpxsbiqqf3y6crin_wchar_t);
	}

	void register_cbegin()
	{
		tester.register_test("BasicString#shakqindqwirxrbi", test_pohekjyqfujtdeg0_char);
		tester.register_test("BasicString#lb99sxkadm8nrf9d", test_pohekjyqfujtdeg0_wchar_t);
	}

	void register_cend()
	{
		tester.register_test("BasicString#ndx0hnoymbq5k8qu", test_pk0bpgspw292sgor_char);
		tester.register_test("BasicString#9ht5k184wi85gl8u", test_pk0bpgspw292sgor_wchar_t);
	}

	void register_subscript_operator()
	{
		tester.register_test("BasicString#vv2v5z77669odfp3", test_il9qs68frle53apy_char);
		tester.register_test("BasicString#zbqpcotg18dqad17", test_il9qs68frle53apy_wchar_t);

		tester.register_test("BasicString#3sdu6ny7f1xf4yze", test_p5ci6kxdkiyl9h62_char);
		tester.register_test("BasicString#u4q4qtytcuc2piwe", test_p5ci6kxdkiyl9h62_wchar_t);
	}

	void register_get_front()
	{
		tester.register_test("BasicString#41vy2whzo7a9kt3i", test_bobijkv4mva6a6iw_char);
		tester.register_test("BasicString#4gmn16e0g9oe6i52", test_bobijkv4mva6a6iw_wchar_t);

		tester.register_test("BasicString#cuc6l9suckmmpbuy", test_6ijbe3eea96r10tb_char);
		tester.register_test("BasicString#6j5ml9uoo9zx5lql", test_6ijbe3eea96r10tb_wchar_t);
	}

	void register_get_back()
	{
		tester.register_test("BasicString#mh8fsbdmsh5w2dx1", test_c7wlkcpbpyxnryt3_char);
		tester.register_test("BasicString#ew4jjdcd709rm5a8", test_c7wlkcpbpyxnryt3_wchar_t);

		tester.register_test("BasicString#esxl5xvjd48aabyg", test_e33fwe8myadjd3ye_char);
		tester.register_test("BasicString#m6uzunrfljam1cr8", test_e33fwe8myadjd3ye_wchar_t);
	}

	void register_reserve()
	{
		tester.register_test("BasicString#23l3w28wcd0nncuo", test_ob92iyvz3a4bxawv_char);
		tester.register_test("BasicString#e4362n9winuzb7f2", test_ob92iyvz3a4bxawv_wchar_t);
	}

	void register_resize()
	{
		tester.register_test("BasicString#y7fivqqmxqbaac8o", test_7dx09kya8th4p717_char);
		tester.register_test("BasicString#w01bcrfpbf9rlpkh", test_7dx09kya8th4p717_wchar_t);

		tester.register_test("BasicString#0tg1wx83tlo99ln6", test_wdj6n73o7vm8x9to_char);
		tester.register_test("BasicString#8f8r3kcclldu4vtj", test_wdj6n73o7vm8x9to_wchar_t);
	}

	void register_clear()
	{
		tester.register_test("BasicString#5w22a7go03u9027u", test_m15ygkaar58jzweu_char);
		tester.register_test("BasicString#7sp4207b3l2c232m", test_m15ygkaar58jzweu_wchar_t);
	}

	void register_swap()
	{
		tester.register_test("BasicString#k4n7rohbrxvftl0v", test_0p5uwwerzn5wsrh9_char);
		tester.register_test("BasicString#8bcyw95vc36wtcl4", test_0p5uwwerzn5wsrh9_wchar_t);
	}

	void register_assign()
	{
		tester.register_test("BasicString#ykt3wh8zqs1prlrf", test_g2763fzzb2efirox_char);
		tester.register_test("BasicString#2yt2uo1c1j37e3sl", test_g2763fzzb2efirox_wchar_t);

		tester.register_test("BasicString#ekw4yojlxzp5hy9k", test_9kcssb2px5eqysbo_char);
		tester.register_test("BasicString#4eim76ili69pawtc", test_9kcssb2px5eqysbo_wchar_t);

		tester.register_test("BasicString#r6fkv8ts54kl4c5d", test_538hehakfc0ftpzi_char);
		tester.register_test("BasicString#5kw6gjq5soohodqm", test_538hehakfc0ftpzi_wchar_t);

		tester.register_test("BasicString#prgfxt914qdq2te7", test_dle1ktr6lapaeu7c_char);
		tester.register_test("BasicString#l3wvarbo6d9m3f2s", test_dle1ktr6lapaeu7c_wchar_t);

		tester.register_test("BasicString#jz4n26cbxll64cmp", test_w9mioe7ry1g2i75g_char);
		tester.register_test("BasicString#f7gykfv1dhbwcxdu", test_w9mioe7ry1g2i75g_wchar_t);

		tester.register_test("BasicString#y4wyc46fyibh5cr3", test_itc6c77uw485667y_char);
		tester.register_test("BasicString#9tn4870gz63bk7ib", test_itc6c77uw485667y_wchar_t);

		tester.register_test("BasicString#rfnl2ug3z90ywhos", test_r4si1rv5vyk18sqw_char);
		tester.register_test("BasicString#byt6s16lsg2v9k45", test_r4si1rv5vyk18sqw_wchar_t);

		tester.register_test("BasicString#3icgjskxuvg8am0c", test_jdc52wzr9lwfglpn_char);
		tester.register_test("BasicString#vhsfs97af8qzq3f5", test_jdc52wzr9lwfglpn_wchar_t);

		tester.register_test("BasicString#kr6o1glnnx0fs3qk", test_b8b64tyi0ymosqh2_char);
		tester.register_test("BasicString#vkcglat10zx8ljab", test_b8b64tyi0ymosqh2_wchar_t);

		tester.register_test("BasicString#vowf789xh53fsvym", test_ms657yc4w46xd9hn_char);
		tester.register_test("BasicString#f3ib0udktthj13jg", test_ms657yc4w46xd9hn_wchar_t);

		tester.register_test("BasicString#03edy1qljo4j6lxt", test_2uc5kzilw6hxwpap_char);
		tester.register_test("BasicString#f7manaqhdjek1nud", test_2uc5kzilw6hxwpap_wchar_t);
	}

	void register_append()
	{
		tester.register_test("BasicString#dgmu1k81lz05dj8c", test_zl84ogc7998w799u_char);
		tester.register_test("BasicString#ez3hi6zcqxw4yaz5", test_zl84ogc7998w799u_wchar_t);

		tester.register_test("BasicString#myltue7ur2tftalb", test_cvp9cybqg2jg0ap2_char);
		tester.register_test("BasicString#jj7l5bga7w8o1tdb", test_cvp9cybqg2jg0ap2_wchar_t);

		tester.register_test("BasicString#tc0vaborkbdhun7r", test_koa3ubk2qqe5gi9l_char);
		tester.register_test("BasicString#1egbc07we6slg95q", test_koa3ubk2qqe5gi9l_wchar_t);

		tester.register_test("BasicString#lpu2cre5a6x2zc5g", test_h696ba837cicyicx_char);
		tester.register_test("BasicString#61k19n59107bp1hy", test_h696ba837cicyicx_wchar_t);

		tester.register_test("BasicString#juy7pbp2pyxyrb53", test_9d0v9x5hdpaz3u7c_char);
		tester.register_test("BasicString#zamf2kg2z5y1wnv4", test_9d0v9x5hdpaz3u7c_wchar_t);

		tester.register_test("BasicString#x3r1maw0kfk4m2w3", test_6z9ek2tengpxfrgl_char);
		tester.register_test("BasicString#w1w2lnbfzdfzbwh5", test_6z9ek2tengpxfrgl_wchar_t);

		tester.register_test("BasicString#th8w3l2g43l8fm21", test_0l93gfiuux94hbu6_char);
		tester.register_test("BasicString#p1t7stxazl7m670c", test_0l93gfiuux94hbu6_wchar_t);

		tester.register_test("BasicString#5o81ek6xrpoif4x4", test_39zmhpxm33s54k6l_char);
		tester.register_test("BasicString#o07f26jin0fgj3e1", test_39zmhpxm33s54k6l_wchar_t);

		tester.register_test("BasicString#621kd9yzmcvh052j", test_35lli5zs2siizeqt_char);
		tester.register_test("BasicString#piip5y1g5ng984ou", test_35lli5zs2siizeqt_wchar_t);

		tester.register_test("BasicString#qbup59nuez8uw4af", test_62hho616a8ahj1yg_char);
		tester.register_test("BasicString#qup6unqbz9efsug8", test_62hho616a8ahj1yg_wchar_t);

		tester.register_test("BasicString#geguwhp9huys0a3b", test_vloq4kjrkvfb8i87_char);
		tester.register_test("BasicString#xncs7b1m5rolv35d", test_vloq4kjrkvfb8i87_wchar_t);
	}

	void register_insert()
	{
		tester.register_test("BasicString#rt16ynfo4mny95iz", test_5j7de12j4ul2qy4j_char);
		tester.register_test("BasicString#0bddx69m4ecgrn67", test_5j7de12j4ul2qy4j_wchar_t);

		tester.register_test("BasicString#s4s9k0vklp4rto2p", test_x701f3h1mn7dyshg_char);
		tester.register_test("BasicString#tbpw3l5hbn37lsd5", test_x701f3h1mn7dyshg_wchar_t);

		tester.register_test("BasicString#nudxdy1bnvo4a0qu", test_pn23tndg5952a41h_char);
		tester.register_test("BasicString#1ph2cbqzr9rmgosn", test_pn23tndg5952a41h_wchar_t);

		tester.register_test("BasicString#hiu6ncppo6m58tip", test_95sf0vox5maogpi6_char);
		tester.register_test("BasicString#5dnkxhn6tvo195zq", test_95sf0vox5maogpi6_wchar_t);

		tester.register_test("BasicString#dfvz3ihc3hglwv70", test_k4en302xt2uxhq9v_char);
		tester.register_test("BasicString#3sf4j3zldqwsltov", test_k4en302xt2uxhq9v_wchar_t);

		tester.register_test("BasicString#2gi26d5cqygjo150", test_e3fkzl3fw2b4d26p_char);
		tester.register_test("BasicString#e4ow2filpsu6c6dd", test_e3fkzl3fw2b4d26p_wchar_t);

		tester.register_test("BasicString#f8nzcokr82hkv94f", test_hgcmuzkrgrih8bw6_char);
		tester.register_test("BasicString#2h3e18d229bxnu2c", test_hgcmuzkrgrih8bw6_wchar_t);

		tester.register_test("BasicString#qn8op00kanyhyc3t", test_1jhbg15jmysnvbnj_char);
		tester.register_test("BasicString#8cr0b1pgknid7npm", test_1jhbg15jmysnvbnj_wchar_t);

		tester.register_test("BasicString#cbompaq88mlo80p8", test_b9w1u8m6p7cg9c1k_char);
		tester.register_test("BasicString#xbf4tl1h37n2p3y3", test_b9w1u8m6p7cg9c1k_wchar_t);

		tester.register_test("BasicString#jd5lwt34szzbjfe4", test_alb7fc11vpuztxaj_char);
		tester.register_test("BasicString#0meiejlrk3xc6p5q", test_alb7fc11vpuztxaj_wchar_t);

		tester.register_test("BasicString#r1a8howmkcnc5tzw", test_hkk649lez4ahi368_char);
		tester.register_test("BasicString#1c04qghsto3f18aw", test_hkk649lez4ahi368_wchar_t);
	}

	void register_erase()
	{
		tester.register_test("BasicString#9omp14djto6qius8", test_13jut63zekqzr9xh_char);
		tester.register_test("BasicString#c68fxy49h2vbscws", test_13jut63zekqzr9xh_wchar_t);

		tester.register_test("BasicString#gibddh6ntddnquhr", test_o12sub2fiu7iosls_char);
		tester.register_test("BasicString#acwabygpssri0aj1", test_o12sub2fiu7iosls_wchar_t);

		tester.register_test("BasicString#lig40xwk5953vaff", test_svff6q2nrw8nwy35_char);
		tester.register_test("BasicString#tf882wcrnew61p2o", test_svff6q2nrw8nwy35_wchar_t);

		tester.register_test("BasicString#59xuyr95qnetclg7", test_shz5467p4n3tugca_char);
		tester.register_test("BasicString#ftjl35q7v81uz1im", test_shz5467p4n3tugca_wchar_t);
	}

	void register_push_back()
	{
		tester.register_test("BasicString#g2t72snu1o84x3os", test_ltq0f177mhzv4bb9_char);
		tester.register_test("BasicString#7z8bb8edbcuma55q", test_ltq0f177mhzv4bb9_wchar_t);
	}

	void register_pop_back()
	{
		tester.register_test("BasicString#2wrajek9thes90us", test_eqdb9m29r80rt00i_char);
		tester.register_test("BasicString#w2eryu82qeyjuk35", test_eqdb9m29r80rt00i_wchar_t);
	}

	void register_get_substring()
	{
		tester.register_test("BasicString#q3fdkbu1mevu61gc", test_r1lvuo2hj60v6aux_char);
		tester.register_test("BasicString#5nft6mcvvi5pwdp4", test_r1lvuo2hj60v6aux_wchar_t);

		tester.register_test("BasicString#ey7jb1nel5bqvt6s", test_gysak99o3p50flj2_char);
		tester.register_test("BasicString#d1w0ho44vq9vdm1q", test_gysak99o3p50flj2_wchar_t);
	}

	void register_get_string_view()
	{
		tester.register_test("BasicString#hsknskg8tqwkqtzh", test_et6hqny7js6hgzhm_char);
		tester.register_test("BasicString#8ufyk3d9p4pc85rl", test_et6hqny7js6hgzhm_wchar_t);

		tester.register_test("BasicString#ynu5fl6sd52tk07x", test_34e1px4up6l6nbht_char);
		tester.register_test("BasicString#d8g628syl7wox7co", test_34e1px4up6l6nbht_wchar_t);

		tester.register_test("BasicString#xba3ut106vhnfvkx", test_etkmnso9fu853ntt_char);
		tester.register_test("BasicString#drwktytrv9ux8blf", test_etkmnso9fu853ntt_wchar_t);
	}

	void register_operator_string_view()
	{
		tester.register_test("BasicString#ls8za5yrw9x29lfh", test_klf1ixu9tmzsl064_char);
		tester.register_test("BasicString#0smbugsfu1xdorvo", test_klf1ixu9tmzsl064_wchar_t);
	}

	void register_compare()
	{
		tester.register_test("BasicString#3wtqqtpto2k5ce8z", test_afjlv1efmzy4ti76_char);
		tester.register_test("BasicString#yh5uqsu1r0nmjsnt", test_afjlv1efmzy4ti76_wchar_t);

		tester.register_test("BasicString#o8h432pw8yp7blf6", test_rrp35kn244n393wo_char);
		tester.register_test("BasicString#p2fbqsh7p3xu3rzz", test_rrp35kn244n393wo_wchar_t);

		tester.register_test("BasicString#ys05hzmrt826a064", test_6c11npvqkd8o2v3e_char);
		tester.register_test("BasicString#rdkl9jy9yqcpzy0r", test_6c11npvqkd8o2v3e_wchar_t);

		tester.register_test("BasicString#g2gga1h0h9a7wg80", test_775yz1vluvocvro1_char);
		tester.register_test("BasicString#wuieshwjmsvwkkwf", test_775yz1vluvocvro1_wchar_t);
	}

	void register_index_of()
	{
		tester.register_test("BasicString#bw2o6xhkaii3mhsx", test_7uq7rypocz7n0w3a_char);
		tester.register_test("BasicString#rg3rvwavpxvs2pzh", test_7uq7rypocz7n0w3a_wchar_t);

		tester.register_test("BasicString#2mecmjl496uh55r7", test_z6br25ysjdr70112_char);
		tester.register_test("BasicString#zw32r44f4x5x7mho", test_z6br25ysjdr70112_wchar_t);

		tester.register_test("BasicString#36bpu7r4fjuubn3g", test_kf0ahsbhxwtfbrzm_char);
		tester.register_test("BasicString#lajot1cydge799ru", test_kf0ahsbhxwtfbrzm_wchar_t);

		tester.register_test("BasicString#hofp3irbbecxvmgt", test_qznvspfo1lllkomd_char);
		tester.register_test("BasicString#q92zai10r7js9hy7", test_qznvspfo1lllkomd_wchar_t);

		tester.register_test("BasicString#d307ofc7bbk49j5k", test_ebs57ypu63vcvpr2_char);
		tester.register_test("BasicString#17d7fdrbls64f7j7", test_ebs57ypu63vcvpr2_wchar_t);

		tester.register_test("BasicString#k40qyckr8hvllxs2", test_c8lkq6z8kkib4dri_char);
		tester.register_test("BasicString#km0ycsiomas9q40z", test_c8lkq6z8kkib4dri_wchar_t);

		tester.register_test("BasicString#rc4wciiyv3pxebfc", test_uz1gprkmx7nymomo_char);
		tester.register_test("BasicString#ea4oc6mpbm5sg7em", test_uz1gprkmx7nymomo_wchar_t);

		tester.register_test("BasicString#eh8o3lqc2y3ejiay", test_ju4s6jmtwbq5pbgo_char);
		tester.register_test("BasicString#h4emw5hetglzccz4", test_ju4s6jmtwbq5pbgo_wchar_t);

		tester.register_test("BasicString#sv3diy223nasuswm", test_uddtqthzq34c7mn6_char);
		tester.register_test("BasicString#8jodmeawpz0mavrf", test_uddtqthzq34c7mn6_wchar_t);

		tester.register_test("BasicString#hb7o3bo3op3zuerr", test_bjpo2szsl95k7lh7_char);
		tester.register_test("BasicString#2xpysiulslvzq8ny", test_bjpo2szsl95k7lh7_wchar_t);
	}

	void register_last_index_of()
	{
		tester.register_test("BasicString#z60u33gsgo8c3dnc", test_46l6sy7wjx5xaffc_char);
		tester.register_test("BasicString#40keds1be9fo8z8v", test_46l6sy7wjx5xaffc_wchar_t);

		tester.register_test("BasicString#q8q4ppuneo7ihjk6", test_wzq7eemoi8hpqj06_char);
		tester.register_test("BasicString#01tpvst0o90c2iha", test_wzq7eemoi8hpqj06_wchar_t);

		tester.register_test("BasicString#tfgemiow28rqtyl4", test_p0mdn7ac7o45r7gt_char);
		tester.register_test("BasicString#jfolk9nfc4jvq2qt", test_p0mdn7ac7o45r7gt_wchar_t);

		tester.register_test("BasicString#v176y9b1st6atezi", test_fkjdefgkjd9hbsmn_char);
		tester.register_test("BasicString#lhfsr1lqet31oyzv", test_fkjdefgkjd9hbsmn_wchar_t);

		tester.register_test("BasicString#wo48dmftnqmz32ph", test_3t442q45xlsdjpla_char);
		tester.register_test("BasicString#xf58pi8jts511ujt", test_3t442q45xlsdjpla_wchar_t);

		tester.register_test("BasicString#455un8f32yqthjck", test_d79qtwz97qgdwcx2_char);
		tester.register_test("BasicString#3bvtfl1mokdd7zze", test_d79qtwz97qgdwcx2_wchar_t);

		tester.register_test("BasicString#74gn8iur56k7ybes", test_lngmpj4clauqry8d_char);
		tester.register_test("BasicString#izngu92z2twvlskd", test_lngmpj4clauqry8d_wchar_t);

		tester.register_test("BasicString#nvg46dwtjwhlos3z", test_dg01g5oq4d1yrigu_char);
		tester.register_test("BasicString#8dawiwchmz93f7gi", test_dg01g5oq4d1yrigu_wchar_t);

		tester.register_test("BasicString#8mbz1zv3ilmq7i4y", test_wf6xgdnoipdh6mvu_char);
		tester.register_test("BasicString#qh9c4qx3b7nh7shv", test_wf6xgdnoipdh6mvu_wchar_t);

		tester.register_test("BasicString#344kwabd0rgvid7q", test_o49vbnlfhr0o8qgy_char);
		tester.register_test("BasicString#l7mtr78ztuej1qa9", test_o49vbnlfhr0o8qgy_wchar_t);
	}

	void register_index_of_any()
	{
		tester.register_test("BasicString#1t96n3ngmj4ntz66", test_v1k94bqmiskcer9g_char);
		tester.register_test("BasicString#y073thhqihx01wjw", test_v1k94bqmiskcer9g_wchar_t);

		tester.register_test("BasicString#cmumhj9tgu6cucm6", test_t0zm8e9yorlrmhyy_char);
		tester.register_test("BasicString#4schuklr1cixn52x", test_t0zm8e9yorlrmhyy_wchar_t);

		tester.register_test("BasicString#ko6jr7yvwhly7lcd", test_61c15k1dqjt9rfjs_char);
		tester.register_test("BasicString#n83ftgqhpmg06j01", test_61c15k1dqjt9rfjs_wchar_t);

		tester.register_test("BasicString#r8ek10k843m15joo", test_eu8d7kvdx79y5h18_char);
		tester.register_test("BasicString#ycttnhie8g5baa83", test_eu8d7kvdx79y5h18_wchar_t);

		tester.register_test("BasicString#i99aro1o8e5o5aag", test_ju2w7rm7607381x8_char);
		tester.register_test("BasicString#ccdjndrcn8d7y1ah", test_ju2w7rm7607381x8_wchar_t);

		tester.register_test("BasicString#0bt0s9f5ijvdzv6l", test_pxeykhvr6h1xskox_char);
		tester.register_test("BasicString#uvtr9qo5zywjk7ax", test_pxeykhvr6h1xskox_wchar_t);

		tester.register_test("BasicString#zujcsz4zd5msjrpk", test_i1ijmtkfo9vjq72e_char);
		tester.register_test("BasicString#cll1asblkk6x1pcp", test_i1ijmtkfo9vjq72e_wchar_t);

		tester.register_test("BasicString#bkhyhl56ezevdkyp", test_o66c69um4ba9jehw_char);
		tester.register_test("BasicString#vy4dbv1guiezgmuh", test_o66c69um4ba9jehw_wchar_t);

		tester.register_test("BasicString#ed0ti1244nsbj41m", test_7rwjpeui0h6fyj9w_char);
		tester.register_test("BasicString#q1ml4mtd363yf4vf", test_7rwjpeui0h6fyj9w_wchar_t);

		tester.register_test("BasicString#xrl6v8evp6thbqk6", test_q7g4y4c0fte1k8mo_char);
		tester.register_test("BasicString#d4b6f4fjxq6uspu2", test_q7g4y4c0fte1k8mo_wchar_t);

		tester.register_test("BasicString#pzml0oma6atlco9s", test_j0g63voyqye98v48_char);
		tester.register_test("BasicString#9t9v10mm3b3mnxmq", test_j0g63voyqye98v48_wchar_t);

		tester.register_test("BasicString#pez3fdaemnxjjo83", test_5ixrmtnfdpn2zf99_char);
		tester.register_test("BasicString#u1fj3xps6a4wfw66", test_5ixrmtnfdpn2zf99_wchar_t);

		tester.register_test("BasicString#0z35flfy14ane5ck", test_3pyycygs5y32ivp6_char);
		tester.register_test("BasicString#5fu2tjh2ogattxc9", test_3pyycygs5y32ivp6_wchar_t);

		tester.register_test("BasicString#cxszz00vdcfcjka1", test_8b1zzhrgsft08sll_char);
		tester.register_test("BasicString#kh7y6npouz0nx6xf", test_8b1zzhrgsft08sll_wchar_t);
	}

	void register_index_not_of_any()
	{
		tester.register_test("BasicString#gjqcy8oyt245j0mm", test_wb5bpimb7b8733m4_char);
		tester.register_test("BasicString#ch0cqwt3sno92cna", test_wb5bpimb7b8733m4_wchar_t);

		tester.register_test("BasicString#fdjri1mfesmj7bp3", test_3dexxbngdoszc9ao_char);
		tester.register_test("BasicString#fwjwghzw2ivk408l", test_3dexxbngdoszc9ao_wchar_t);

		tester.register_test("BasicString#hk36fk1jze552oa7", test_2270z9jmj36w9ro6_char);
		tester.register_test("BasicString#q0i39oawu4av5njq", test_2270z9jmj36w9ro6_wchar_t);

		tester.register_test("BasicString#9is17bk8i49ww3nj", test_9l5w87x8rukm8vrw_char);
		tester.register_test("BasicString#wzzfkytr9tpptdqt", test_9l5w87x8rukm8vrw_wchar_t);

		tester.register_test("BasicString#v66pr0fb4tugberz", test_8hqhk9twgig3w6iu_char);
		tester.register_test("BasicString#0vist7kn8ftmlpsf", test_8hqhk9twgig3w6iu_wchar_t);

		tester.register_test("BasicString#r1vq4tbzgtwzsx9v", test_sp0pxeqmryuj3l2q_char);
		tester.register_test("BasicString#eg4jw8yjejh3ciec", test_sp0pxeqmryuj3l2q_wchar_t);

		tester.register_test("BasicString#x63tvcfgaaofqhhs", test_yfvg1t8326lbwkuq_char);
		tester.register_test("BasicString#k2066hfih4jk7my5", test_yfvg1t8326lbwkuq_wchar_t);

		tester.register_test("BasicString#r7yuawi36fdiuzz4", test_xl1c3mek6i1vpuiw_char);
		tester.register_test("BasicString#2ugqj2gvkfltepnt", test_xl1c3mek6i1vpuiw_wchar_t);

		tester.register_test("BasicString#71jd69sbtgjuwxh1", test_h3chz1583d6rd9wo_char);
		tester.register_test("BasicString#c5exxqmrtjaqivkp", test_h3chz1583d6rd9wo_wchar_t);

		tester.register_test("BasicString#q7a5enow9dddgh3z", test_3fw7dhxj9pd8d4xt_char);
		tester.register_test("BasicString#c084c6o9vt7tmjyw", test_3fw7dhxj9pd8d4xt_wchar_t);

		tester.register_test("BasicString#nj484rj06idttlev", test_m5h2kycijbmewsgm_char);
		tester.register_test("BasicString#9807wjelysbsg3qm", test_m5h2kycijbmewsgm_wchar_t);

		tester.register_test("BasicString#s2r6cf1304376tl3", test_6w4r6acup3ggob06_char);
		tester.register_test("BasicString#ahzzb4g7bit9q7xb", test_6w4r6acup3ggob06_wchar_t);

		tester.register_test("BasicString#po5b17uoyfntdzw5", test_nxyobgy48oymiht0_char);
		tester.register_test("BasicString#v3mj5uqv88nvjt64", test_nxyobgy48oymiht0_wchar_t);

		tester.register_test("BasicString#nwpdp7k91pnzxkwb", test_sygb4b6uauyrn2mf_char);
		tester.register_test("BasicString#av0v3t2l9kvjdbi0", test_sygb4b6uauyrn2mf_wchar_t);
	}

	void register_last_index_of_any()
	{
		tester.register_test("BasicString#selec1itzqtc1qwp", test_7uevpg98ot04vqev_char);
		tester.register_test("BasicString#4f7vwqi7q84z3uij", test_7uevpg98ot04vqev_wchar_t);

		tester.register_test("BasicString#1uv0z9x9dfrbj9wd", test_830axldq4tob123f_char);
		tester.register_test("BasicString#kl8fp75z3dgdld92", test_830axldq4tob123f_wchar_t);

		tester.register_test("BasicString#9hqyfaf0d71pqev1", test_wakikixlvzuzklw1_char);
		tester.register_test("BasicString#2i7p1ji018l3h6to", test_wakikixlvzuzklw1_wchar_t);

		tester.register_test("BasicString#stqk0nzw78j1ryi7", test_9iw14ki41xa2l89s_char);
		tester.register_test("BasicString#4rdp1bs63on6utgk", test_9iw14ki41xa2l89s_wchar_t);

		tester.register_test("BasicString#25zim1mxofpsn437", test_02x7ueu9xlpj9cg7_char);
		tester.register_test("BasicString#rfjhik6vqucd06rl", test_02x7ueu9xlpj9cg7_wchar_t);

		tester.register_test("BasicString#h1gc7g1k31tc4aky", test_mai1fbqg6yv52fcz_char);
		tester.register_test("BasicString#7zr0tjly12h7h8h5", test_mai1fbqg6yv52fcz_wchar_t);

		tester.register_test("BasicString#vfudce023ojyz6g0", test_nconmvc8c0yvhnl1_char);
		tester.register_test("BasicString#uv26c9c6q8yklokf", test_nconmvc8c0yvhnl1_wchar_t);

		tester.register_test("BasicString#j8rk4dh83d8lx2mj", test_vzbjnbbsss9zukqy_char);
		tester.register_test("BasicString#m0e83bwyce0lyz3g", test_vzbjnbbsss9zukqy_wchar_t);

		tester.register_test("BasicString#2j1qvnqzsi832u8b", test_wr2qyobp1ksvuxc6_char);
		tester.register_test("BasicString#0v8hf5z9w259f3lg", test_wr2qyobp1ksvuxc6_wchar_t);

		tester.register_test("BasicString#s83ajwywzrg29gdj", test_gl0ptrl9jfl3ytwh_char);
		tester.register_test("BasicString#9a7w5hskhw0eyu1v", test_gl0ptrl9jfl3ytwh_wchar_t);

		tester.register_test("BasicString#yzbqqcfvetpi93vm", test_l3fzvn44www26uo5_char);
		tester.register_test("BasicString#qvy08x2oe0f3wqkv", test_l3fzvn44www26uo5_wchar_t);

		tester.register_test("BasicString#ooxpaokrlgp9gka9", test_cq8st269j2yqdn5x_char);
		tester.register_test("BasicString#7rvmkoumkqmolvhw", test_cq8st269j2yqdn5x_wchar_t);

		tester.register_test("BasicString#oyn7qj2uw83eicq6", test_0l0cii3esc23tzfj_char);
		tester.register_test("BasicString#0551uiapluq1i9lh", test_0l0cii3esc23tzfj_wchar_t);

		tester.register_test("BasicString#h35tq10qb9ixph0w", test_c7hhypzqsdj5u5mh_char);
		tester.register_test("BasicString#kbv1feo2gbg41sqd", test_c7hhypzqsdj5u5mh_wchar_t);
	}

	void register_last_index_not_of_any()
	{
		tester.register_test("BasicString#f6hcgpxa0lbih3rq", test_vpv19kni7j13vud2_char);
		tester.register_test("BasicString#lizo3q8jpwatfdif", test_vpv19kni7j13vud2_wchar_t);

		tester.register_test("BasicString#4nqk09ak38wsvelk", test_asx1po87jt9v3nw1_char);
		tester.register_test("BasicString#oj8uo495x3a8x12c", test_asx1po87jt9v3nw1_wchar_t);

		tester.register_test("BasicString#qk2f7rcwn052ipxr", test_j1kvhn51ol070iyl_char);
		tester.register_test("BasicString#es78eqalhfrkelfx", test_j1kvhn51ol070iyl_wchar_t);

		tester.register_test("BasicString#2lfkxwzqcdjh35kd", test_bsv4o4zni6nhoe5v_char);
		tester.register_test("BasicString#khz57lwtmq7986nl", test_bsv4o4zni6nhoe5v_wchar_t);

		tester.register_test("BasicString#a6o7j8e7z4dsmj3p", test_ti8qeb6scihydez4_char);
		tester.register_test("BasicString#c1zl5jj24jyehrrn", test_ti8qeb6scihydez4_wchar_t);

		tester.register_test("BasicString#mx02i5uuppw2dnf3", test_tk9itcnxuo8vk6ve_char);
		tester.register_test("BasicString#kbhhi7f908rlkrre", test_tk9itcnxuo8vk6ve_wchar_t);

		tester.register_test("BasicString#b5lbimtx5vffq7wl", test_glmots1o91mv3jon_char);
		tester.register_test("BasicString#uj5citzntbs5xqq2", test_glmots1o91mv3jon_wchar_t);

		tester.register_test("BasicString#d7xs67ak3f0ic7vo", test_g07lgdro09lxfqli_char);
		tester.register_test("BasicString#3onlp9tqw75zytkn", test_g07lgdro09lxfqli_wchar_t);

		tester.register_test("BasicString#qtbxotc5en6cue8d", test_ckckg9r6rykqal7f_char);
		tester.register_test("BasicString#ofs66q1l440llzb2", test_ckckg9r6rykqal7f_wchar_t);

		tester.register_test("BasicString#546iysrsxzczsoae", test_0jn3yi06557p0hfd_char);
		tester.register_test("BasicString#ccl7k0ma31gl1vll", test_0jn3yi06557p0hfd_wchar_t);

		tester.register_test("BasicString#6l76jlyy65wdfraq", test_jgpmnp7moki886n4_char);
		tester.register_test("BasicString#svkyf2skmlg0bslu", test_jgpmnp7moki886n4_wchar_t);

		tester.register_test("BasicString#swrlwi7oh8cle965", test_sv5ar3b3k2450vjn_char);
		tester.register_test("BasicString#y9x38crwsubinfat", test_sv5ar3b3k2450vjn_wchar_t);

		tester.register_test("BasicString#fkj8c81uowztldjg", test_w108m1syb9hxjchd_char);
		tester.register_test("BasicString#opft3jcpy70sn07g", test_w108m1syb9hxjchd_wchar_t);

		tester.register_test("BasicString#it777xab2grt0khi", test_wejrnhb9vianv8ds_char);
		tester.register_test("BasicString#8qnfwp1e1isx6uzq", test_wejrnhb9vianv8ds_wchar_t);
	}

	void register_contains()
	{
		tester.register_test("BasicString#loknu8bayp982fop", test_2kjeinl8iy7f2cw0_char);
		tester.register_test("BasicString#gf9qtgpwaeg6m0cl", test_2kjeinl8iy7f2cw0_wchar_t);

		tester.register_test("BasicString#lsx8viligfg43i39", test_xfpj3dyh5ejh1gb9_char);
		tester.register_test("BasicString#avb1mq0g5wvjd0yi", test_xfpj3dyh5ejh1gb9_wchar_t);

		tester.register_test("BasicString#a8vjb0d1ioh5zry3", test_5fsnfktj1zsbbd7d_char);
		tester.register_test("BasicString#rg815c7w713md91k", test_5fsnfktj1zsbbd7d_wchar_t);

		tester.register_test("BasicString#r9q9kumca48f0ucs", test_zlwjynibncqbogvz_char);
		tester.register_test("BasicString#7ku8cny7edl5w0gd", test_zlwjynibncqbogvz_wchar_t);

		tester.register_test("BasicString#tggfs2y4lrhpb353", test_xchdeh0wl85jhwkj_char);
		tester.register_test("BasicString#5m1ch2bqb0zra8d5", test_xchdeh0wl85jhwkj_wchar_t);
	}

	void register_starts_with()
	{
		tester.register_test("BasicString#zqy8jjxlqnrxis5a", test_yh51kenp7rb6h1z6_char);
		tester.register_test("BasicString#pgeuwb2uej2v5i1a", test_yh51kenp7rb6h1z6_wchar_t);

		tester.register_test("BasicString#v3ct38c9rda7p934", test_3b7l72jgaqoqdusm_char);
		tester.register_test("BasicString#m1z6rsuukewgxvup", test_3b7l72jgaqoqdusm_wchar_t);

		tester.register_test("BasicString#5mulavbm3t7f3cz8", test_8h8yv81b1zgo0dmo_char);
		tester.register_test("BasicString#4rbel8nvu7zfa2af", test_8h8yv81b1zgo0dmo_wchar_t);

		tester.register_test("BasicString#vwyvwhzhocu0mjnv", test_ir1zkpdn71mj1nqr_char);
		tester.register_test("BasicString#ncijt4wlq2qywvxx", test_ir1zkpdn71mj1nqr_wchar_t);

		tester.register_test("BasicString#3k2dzya5l94z539p", test_t0a55608och9z1bh_char);
		tester.register_test("BasicString#i45v4le6x4u8iy3u", test_t0a55608och9z1bh_wchar_t);
	}

	void register_ends_with()
	{
		tester.register_test("BasicString#y5et7oqe66c7ph9y", test_4ofvvffd01ief55j_char);
		tester.register_test("BasicString#rh340h06e1zsaxi3", test_4ofvvffd01ief55j_wchar_t);

		tester.register_test("BasicString#9l1j2sv736l09tep", test_zcvexidebzt7ne76_char);
		tester.register_test("BasicString#0sc44p38o4bth0zj", test_zcvexidebzt7ne76_wchar_t);

		tester.register_test("BasicString#5jyzue806a7flmzz", test_4edwoczh7b92li3i_char);
		tester.register_test("BasicString#si2yasft1cmua5v8", test_4edwoczh7b92li3i_wchar_t);

		tester.register_test("BasicString#mwubsusf82opgvhe", test_sgsr33kcxyjc53di_char);
		tester.register_test("BasicString#d5viwd2bmbi00e43", test_sgsr33kcxyjc53di_wchar_t);

		tester.register_test("BasicString#ear1w3ihqvncg698", test_wjpxca3alaoiv5bn_char);
		tester.register_test("BasicString#utjv7tnsbn9vs6m0", test_wjpxca3alaoiv5bn_wchar_t);
	}

	void register_make_with_capacity()
	{
		tester.register_test("BasicString#idag6wbvslmn2tt9", test_z22rdgewv090xrn0_char);
		tester.register_test("BasicString#llc9ujviufkq0si7", test_z22rdgewv090xrn0_wchar_t);
	}

	void register_make_with_fill()
	{
		tester.register_test("BasicString#29qaj7hk02kkmld2", test_9cdzbziw1c3bpweg_char);
		tester.register_test("BasicString#nxagrv3pobm3x640", test_9cdzbziw1c3bpweg_wchar_t);
	}

	void register_operator_add()
	{
		tester.register_test("BasicString#tljsge723fn97sqt", test_iuxbt13i4286zs3q_char);
		tester.register_test("BasicString#8u3pp8yzglmnk08e", test_iuxbt13i4286zs3q_wchar_t);

		tester.register_test("BasicString#mexoixzc5gjcjg68", test_6e8tcyyagtlv1lwp_char);
		tester.register_test("BasicString#2u3u8kmzk03pexdx", test_6e8tcyyagtlv1lwp_wchar_t);

		tester.register_test("BasicString#0169whnhiqagpmu4", test_3jbkzlrs5mm9yp9r_char);
		tester.register_test("BasicString#hmiq97wev0umgl83", test_3jbkzlrs5mm9yp9r_wchar_t);
	}

	void register_operator_add_assign()
	{
		tester.register_test("BasicString#ccgq65qegsdmeaw4", test_7wud7k885palma6i_char);
		tester.register_test("BasicString#zi4y4al99gdvu9n6", test_7wud7k885palma6i_wchar_t);

		tester.register_test("BasicString#f65cw8l9n4xhdnpr", test_jl07ic6eb87d2rfd_char);
		tester.register_test("BasicString#69nu4txjf357o59k", test_jl07ic6eb87d2rfd_wchar_t);

		tester.register_test("BasicString#bgl3h6n22otam0yt", test_6q6l42edz7ua050u_char);
		tester.register_test("BasicString#dyp6lnxmebc6edca", test_6q6l42edz7ua050u_wchar_t);
	}

	void register_operator_equal_to()
	{
		tester.register_test("BasicString#y2d7n9qtr0obucf1", test_wa1ouvxiomiaya6k_char);
		tester.register_test("BasicString#fsuempi6kgcaqxsw", test_wa1ouvxiomiaya6k_wchar_t);

		tester.register_test("BasicString#wpyud1hqzqig4oz3", test_38d4kl3up83frf73_char);
		tester.register_test("BasicString#a2bkcrws6haa150g", test_38d4kl3up83frf73_wchar_t);

		tester.register_test("BasicString#bp1nqe5or1dx1owu", test_8998irashz12kvu4_char);
		tester.register_test("BasicString#obobnd8peuv75yrx", test_8998irashz12kvu4_wchar_t);
	}

	void register_operator_not_equal_to()
	{
		tester.register_test("BasicString#nu29rydo2t6ol48u", test_abmys46p8fnggnf6_char);
		tester.register_test("BasicString#nxue9io1i3r4dbks", test_abmys46p8fnggnf6_wchar_t);

		tester.register_test("BasicString#uh42wbgjzko7lv2x", test_89a4j82uthfo7qmt_char);
		tester.register_test("BasicString#1rqv82a2dvutoq9m", test_89a4j82uthfo7qmt_wchar_t);

		tester.register_test("BasicString#i5hflvvcrklx68ha", test_ebpfgczocfwrt97s_char);
		tester.register_test("BasicString#p74hpwdyim7f9umd", test_ebpfgczocfwrt97s_wchar_t);
	}

	void register_operator_less_than()
	{
		tester.register_test("BasicString#ivb2cshrnkf2ajm2", test_6n5n6z3q7l4q5pmq_char);
		tester.register_test("BasicString#fdjj32kx8ng483u4", test_6n5n6z3q7l4q5pmq_wchar_t);

		tester.register_test("BasicString#y8wszk2li7ndtgnw", test_f62pd62zh6fbmxkp_char);
		tester.register_test("BasicString#exsmfdpogv5d34sm", test_f62pd62zh6fbmxkp_wchar_t);

		tester.register_test("BasicString#64ss4y5n1bf9h42o", test_8hp2ve1oemz7oqam_char);
		tester.register_test("BasicString#75n92ksop6uwhcdx", test_8hp2ve1oemz7oqam_wchar_t);
	}

	void register_operator_less_than_or_equal_to()
	{
		tester.register_test("BasicString#2qsoyjwxhfqbxqrf", test_elv3ya3zycpz1zrm_char);
		tester.register_test("BasicString#iv7345bnnhg0k091", test_elv3ya3zycpz1zrm_wchar_t);

		tester.register_test("BasicString#kuwgu84bdpketaqc", test_dqke1gl5cdg6f8uh_char);
		tester.register_test("BasicString#x6no6abd4a7ltsbx", test_dqke1gl5cdg6f8uh_wchar_t);

		tester.register_test("BasicString#1oxkxpepl1epzruj", test_o92tymhm176c7205_char);
		tester.register_test("BasicString#b1j49681icv8g0xy", test_o92tymhm176c7205_wchar_t);
	}

	void register_operator_greater_than()
	{
		tester.register_test("BasicString#qqmwxndaapa9w48l", test_2v4zikw4talgf0iq_char);
		tester.register_test("BasicString#uakuhoi5gphbezep", test_2v4zikw4talgf0iq_wchar_t);

		tester.register_test("BasicString#m7tb6fh50mha5k3y", test_bns5on3wcyqzchl7_char);
		tester.register_test("BasicString#ivp4zbwrqm117i65", test_bns5on3wcyqzchl7_wchar_t);

		tester.register_test("BasicString#3axwdz509rkgjket", test_kf5uef6fmg5aer1t_char);
		tester.register_test("BasicString#h54j4y25vxho9gji", test_kf5uef6fmg5aer1t_wchar_t);
	}

	void register_operator_greater_than_or_equal_to()
	{
		tester.register_test("BasicString#22cz0ny5e1x65ju2", test_qyzqy4z2lpyyplcy_char);
		tester.register_test("BasicString#6c8j2co1d0zf16ta", test_qyzqy4z2lpyyplcy_wchar_t);

		tester.register_test("BasicString#1hcrc4aqqeds763k", test_u2gzkd517wp1ek8d_char);
		tester.register_test("BasicString#0fnbyljigmsssk8l", test_u2gzkd517wp1ek8d_wchar_t);

		tester.register_test("BasicString#g4sewml5q4n25t2r", test_hnph2udi2dr63ntf_char);
		tester.register_test("BasicString#6k3vxrw7l7sk4kih", test_hnph2udi2dr63ntf_wchar_t);
	}
};

auto registrator = Registrator{};

} // namespace
