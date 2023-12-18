#include <iterator>
#include <type_traits>

#include "bstone_string_view.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

template<typename T>
class HasNullptrCtor
{
private:
	template<typename U>
	static auto test(int) -> decltype(U{nullptr}, std::true_type{});
 
	template<typename>
	static std::false_type test(...);

public:
	static constexpr auto value = std::is_same<decltype(test<T>(0)), std::true_type>::value;
};

template<typename TChar>
struct Strings;

template<>
struct Strings<char>
{
	static constexpr auto empty = "";
	static constexpr auto test = "test";
	static constexpr auto test1 = "test1";
	static constexpr auto one_test = "1test";
	static constexpr auto aa_test_bb = "aa_test_bb";
	static constexpr auto aa_test_test_bb = "aa_test_test_bb";
	static constexpr auto t = "t";
	static constexpr auto yes = "yes";
	static constexpr auto u = "u";
	static constexpr auto a_tes = "a_tes";
	static constexpr auto t_b = "t_b";
	static constexpr auto a = "a";
	static constexpr auto b = "b";
};

template<>
struct Strings<wchar_t>
{
	static constexpr auto empty = L"";
	static constexpr auto test = L"test";
	static constexpr auto test1 = L"test1";
	static constexpr auto one_test = L"1test";
	static constexpr auto aa_test_bb = L"aa_test_bb";
	static constexpr auto aa_test_test_bb = L"aa_test_test_bb";
	static constexpr auto t = L"t";
	static constexpr auto yes = L"yes";
	static constexpr auto u = L"u";
	static constexpr auto a_tes = L"a_tes";
	static constexpr auto t_b = L"t_b";
	static constexpr auto a = L"a";
	static constexpr auto b = L"b";
};

// ==========================================================================

// constexpr BasicStringView()
template<typename TChar>
void test_av45voi54f4btjcw()
{
	constexpr auto sv = bstone::BasicStringView<TChar>{};

	constexpr auto data = sv.get_data();
	constexpr auto is_valid_1 = data == nullptr;

	constexpr auto size = sv.get_size();
	constexpr auto is_valid_2 = size == 0;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		true;

	tester.check(is_valid);
}

void test_av45voi54f4btjcw_char()
{
	test_av45voi54f4btjcw<char>();
}

void test_av45voi54f4btjcw_wchar_t()
{
	test_av45voi54f4btjcw<wchar_t>();
}

// ==========================================================================

// constexpr BasicStringView(const BasicStringView&)
template<typename TChar>
void test_9viu2sbdg5qfgezi()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{src_string};
	constexpr auto sv = sv_1;

	constexpr auto data = sv.get_data();
	constexpr auto is_valid_1 = data == src_string;

	constexpr auto size = sv.get_size();
	constexpr auto is_valid_2 = size == 4;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		true;

	tester.check(is_valid);
}

void test_9viu2sbdg5qfgezi_char()
{
	test_9viu2sbdg5qfgezi<char>();
}

void test_9viu2sbdg5qfgezi_wchar_t()
{
	test_9viu2sbdg5qfgezi<wchar_t>();
}

// ==========================================================================

// constexpr BasicStringView(const Char*, std::intptr_t)
template<typename TChar>
void test_leht3ef2qbhbvlk8()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{src_string, 3};
	constexpr auto sv = sv_1;

	constexpr auto data = sv.get_data();
	constexpr auto is_valid_1 = data == src_string;

	constexpr auto size = sv.get_size();
	constexpr auto is_valid_2 = size == 3;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		true;

	tester.check(is_valid);
}

void test_leht3ef2qbhbvlk8_char()
{
	test_leht3ef2qbhbvlk8<char>();
}

void test_leht3ef2qbhbvlk8_wchar_t()
{
	test_leht3ef2qbhbvlk8<wchar_t>();
}

// ==========================================================================

// constexpr BasicStringView(const Char*)
template<typename TChar>
void test_abnsmbym1acwzdqk()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{src_string};
	constexpr auto sv = sv_1;

	constexpr auto data = sv.get_data();
	constexpr auto is_valid_1 = data == src_string;

	constexpr auto size = sv.get_size();
	constexpr auto is_valid_2 = size == 4;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		true;

	tester.check(is_valid);
}

void test_abnsmbym1acwzdqk_char()
{
	test_abnsmbym1acwzdqk<char>();
}

void test_abnsmbym1acwzdqk_wchar_t()
{
	test_abnsmbym1acwzdqk<wchar_t>();
}

// ==========================================================================

// constexpr BasicStringView(UIterBegin first, UIterEnd last)
template<typename TChar>
void test_hqvek4n9ws8pky97()
{
	static constexpr TChar chars[] = {'t', 'e', 's', 't'};
	constexpr auto sv = bstone::BasicStringView<TChar>{std::cbegin(chars), std::cend(chars)};

	constexpr auto size = sv.get_size();
	constexpr auto is_valid_1 = size == 4;

	constexpr auto data = sv.get_data();

	constexpr auto is_valid_2 =
		data[0] == 't' &&
		data[1] == 'e' &&
		data[2] == 's' &&
		data[3] == 't' &&
		true;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		true;

	tester.check(is_valid);
}

void test_hqvek4n9ws8pky97_char()
{
	test_hqvek4n9ws8pky97<char>();
}

void test_hqvek4n9ws8pky97_wchar_t()
{
	test_hqvek4n9ws8pky97<wchar_t>();
}

// ==========================================================================

// BasicStringView(std::nullptr_t) = delete
template<typename TChar>
void test_a86kmgzvs440xahn()
{
	constexpr auto is_valid = !HasNullptrCtor<bstone::BasicStringView<TChar>>::value;

	tester.check(is_valid);
}

void test_a86kmgzvs440xahn_char()
{
	test_a86kmgzvs440xahn<char>();
}

void test_a86kmgzvs440xahn_wchar_t()
{
	test_a86kmgzvs440xahn<wchar_t>();
}

// ==========================================================================

template<typename TChar>
class Test_47gff4scnmvsp3vd
{
public:
	constexpr Test_47gff4scnmvsp3vd() noexcept
	{
		constexpr auto src_string = Strings<TChar>::test;

		const auto sv_1 = bstone::BasicStringView<TChar>{src_string};
		auto sv = bstone::BasicStringView<TChar>{};
		sv = sv_1;

		const auto data = sv.get_data();
		const auto is_valid_1 = data == src_string;

		const auto size = sv.get_size();
		const auto is_valid_2 = size == 4;

		is_valid_ =
			is_valid_1 &&
			is_valid_2 &&
			true;
	}

	constexpr bool is_valid() const noexcept
	{
		return is_valid_;
	}

private:
	bool is_valid_{};
};

// BasicStringView& operator=(const BasicStringView&)
template<typename TChar>
void test_47gff4scnmvsp3vd()
{
	constexpr auto is_valid = Test_47gff4scnmvsp3vd<TChar>{}.is_valid();

	tester.check(is_valid);
}

void test_47gff4scnmvsp3vd_char()
{
	test_47gff4scnmvsp3vd<char>();
}

void test_47gff4scnmvsp3vd_wchar_t()
{
	test_47gff4scnmvsp3vd<wchar_t>();
}

// ==========================================================================

// constexpr const Char* begin() const noexcept
template<typename TChar>
void test_5caelu43w4p0fwrx()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv = bstone::BasicStringView<TChar>{src_string};

	constexpr auto begin = sv.begin();
	static_assert(
		std::is_pointer<decltype(sv.begin())>::value &&
			std::is_const<std::remove_pointer_t<decltype(sv.begin())>>::value,
		"Invalid type.");
	constexpr auto is_valid_1 = begin == src_string;

	constexpr auto size = sv.get_size();
	constexpr auto is_valid_2 = size == 4;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		true;

	tester.check(is_valid);
}

void test_5caelu43w4p0fwrx_char()
{
	test_5caelu43w4p0fwrx<char>();
}

void test_5caelu43w4p0fwrx_wchar_t()
{
	test_5caelu43w4p0fwrx<wchar_t>();
}

// ==========================================================================

// constexpr const Char* cbegin() const noexcept
template<typename TChar>
void test_33m2dh4h4pslbxze()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv = bstone::BasicStringView<TChar>{src_string};

	constexpr auto cbegin = sv.cbegin();
	static_assert(
		std::is_pointer<decltype(sv.cbegin())>::value &&
			std::is_const<std::remove_pointer_t<decltype(sv.cbegin())>>::value,
		"Invalid type.");
	constexpr auto is_valid_1 = cbegin == src_string;

	constexpr auto size = sv.get_size();
	constexpr auto is_valid_2 = size == 4;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		true;

	tester.check(is_valid);
}

void test_33m2dh4h4pslbxze_char()
{
	test_33m2dh4h4pslbxze<char>();
}

void test_33m2dh4h4pslbxze_wchar_t()
{
	test_33m2dh4h4pslbxze<wchar_t>();
}

// ==========================================================================

// constexpr const Char* end() const noexcept
template<typename TChar>
void test_fpwgqzy1dlty1cx1()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv = bstone::BasicStringView<TChar>{src_string};

	constexpr auto end = sv.end();
	static_assert(
		std::is_pointer<decltype(sv.end())>::value &&
			std::is_const<std::remove_pointer_t<decltype(sv.end())>>::value,
		"Invalid type.");
	constexpr auto is_valid_1 = end == src_string + 4;

	constexpr auto size = sv.get_size();
	constexpr auto is_valid_2 = size == 4;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		true;

	tester.check(is_valid);
}

void test_fpwgqzy1dlty1cx1_char()
{
	test_fpwgqzy1dlty1cx1<char>();
}

void test_fpwgqzy1dlty1cx1_wchar_t()
{
	test_fpwgqzy1dlty1cx1<wchar_t>();
}

// ==========================================================================

// constexpr const Char* cend() const noexcept
template<typename TChar>
void test_cralngzuccb3zj33()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv = bstone::BasicStringView<TChar>{src_string};

	constexpr auto cend = sv.cend();
	static_assert(
		std::is_pointer<decltype(sv.cend())>::value &&
			std::is_const<std::remove_pointer_t<decltype(sv.cend())>>::value,
		"Invalid type.");
	constexpr auto is_valid_1 = cend == src_string + 4;

	constexpr auto size = sv.get_size();
	constexpr auto is_valid_2 = size == 4;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		true;

	tester.check(is_valid);
}

void test_cralngzuccb3zj33_char()
{
	test_cralngzuccb3zj33<char>();
}

void test_cralngzuccb3zj33_wchar_t()
{
	test_cralngzuccb3zj33<wchar_t>();
}

// ==========================================================================

// constexpr const Char& operator[](std::intptr_t) const
template<typename TChar>
void test_kp7o2h051zq4ibgj()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv = bstone::BasicStringView<TChar>{src_string};

	constexpr auto& char_1 = sv[0];
	static_assert(
		std::is_reference<decltype(sv[0])>::value &&
			std::is_const<std::remove_reference_t<decltype(sv[0])>>::value,
		"Invalid type.");
	constexpr auto is_valid_1 = char_1 == 't';

	constexpr auto& char_2 = sv[1];
	static_assert(
		std::is_reference<decltype(sv[1])>::value &&
			std::is_const<std::remove_reference_t<decltype(sv[1])>>::value,
		"Invalid type.");
	constexpr auto is_valid_2 = char_2 == 'e';

	constexpr auto& char_3 = sv[2];
	static_assert(
		std::is_reference<decltype(sv[2])>::value &&
			std::is_const<std::remove_reference_t<decltype(sv[2])>>::value,
		"Invalid type.");
	constexpr auto is_valid_3 = char_3 == 's';

	constexpr auto& char_4 = sv[3];
	static_assert(
		std::is_reference<decltype(sv[3])>::value &&
			std::is_const<std::remove_reference_t<decltype(sv[3])>>::value,
		"Invalid type.");
	constexpr auto is_valid_4 = char_4 == 't';

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_kp7o2h051zq4ibgj_char()
{
	test_kp7o2h051zq4ibgj<char>();
}

void test_kp7o2h051zq4ibgj_wchar_t()
{
	test_kp7o2h051zq4ibgj<wchar_t>();
}

// ==========================================================================

// constexpr const Char& get_front() const
template<typename TChar>
void test_d1i9i3cehotrl9wk()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv = bstone::BasicStringView<TChar>{src_string};

	constexpr auto& front = sv.get_front();
	static_assert(
		std::is_reference<decltype(sv.get_front())>::value &&
			std::is_const<std::remove_reference_t<decltype(sv.get_front())>>::value,
		"Invalid type.");
	constexpr auto is_valid_1 = front == 't';

	constexpr auto is_valid =
		is_valid_1 &&
		true;

	tester.check(is_valid);
}

void test_d1i9i3cehotrl9wk_char()
{
	test_d1i9i3cehotrl9wk<char>();
}

void test_d1i9i3cehotrl9wk_wchar_t()
{
	test_d1i9i3cehotrl9wk<wchar_t>();
}

// ==========================================================================

// constexpr const Char& get_back() const
template<typename TChar>
void test_6tlkmc9ldvawh8hl()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv = bstone::BasicStringView<TChar>{src_string, 3};

	constexpr auto& back = sv.get_back();
	static_assert(
		std::is_reference<decltype(sv.get_back())>::value &&
			std::is_const<std::remove_reference_t<decltype(sv.get_back())>>::value,
		"Invalid type.");
	constexpr auto is_valid_1 = back == 's';

	constexpr auto is_valid =
		is_valid_1 &&
		true;

	tester.check(is_valid);
}

void test_6tlkmc9ldvawh8hl_char()
{
	test_6tlkmc9ldvawh8hl<char>();
}

void test_6tlkmc9ldvawh8hl_wchar_t()
{
	test_6tlkmc9ldvawh8hl<wchar_t>();
}

// ==========================================================================

// constexpr const Char* get_data() const noexcept
template<typename TChar>
void test_gvb0khum3tk2o7cv()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv = bstone::BasicStringView<TChar>{src_string};

	constexpr auto data = sv.get_data();
	static_assert(
		std::is_pointer<decltype(sv.get_data())>::value &&
			std::is_const<std::remove_pointer_t<decltype(sv.get_data())>>::value,
		"Invalid type.");
	constexpr auto is_valid_1 = data == src_string;

	constexpr auto is_valid =
		is_valid_1 &&
		true;

	tester.check(is_valid);
}

void test_gvb0khum3tk2o7cv_char()
{
	test_gvb0khum3tk2o7cv<char>();
}

void test_gvb0khum3tk2o7cv_wchar_t()
{
	test_gvb0khum3tk2o7cv<wchar_t>();
}

// ==========================================================================

// constexpr bool has_data() const noexcept
template<typename TChar>
void test_wmeyb4p34n195qg6()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{};
	constexpr auto sv_2 = bstone::BasicStringView<TChar>{src_string};

	constexpr auto result_1 = sv_1.has_data();
	constexpr auto is_valid_1 = !result_1;

	constexpr auto result_2 = sv_2.has_data();
	constexpr auto is_valid_2 = result_2;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		true;

	tester.check(is_valid);
}

void test_wmeyb4p34n195qg6_char()
{
	test_wmeyb4p34n195qg6<char>();
}

void test_wmeyb4p34n195qg6_wchar_t()
{
	test_wmeyb4p34n195qg6<wchar_t>();
}

// ==========================================================================

// constexpr std::intptr_t get_size() const noexcept
template<typename TChar>
void test_4scxk1o1cx2s9xpp()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{};
	constexpr auto sv_2 = bstone::BasicStringView<TChar>{src_string};

	constexpr auto size_1 = sv_1.get_size();
	constexpr auto is_valid_1 = size_1 == 0;

	constexpr auto size_2 = sv_2.get_size();
	constexpr auto is_valid_2 = size_2 == 4;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		true;

	tester.check(is_valid);
}

void test_4scxk1o1cx2s9xpp_char()
{
	test_4scxk1o1cx2s9xpp<char>();
}

void test_4scxk1o1cx2s9xpp_wchar_t()
{
	test_4scxk1o1cx2s9xpp<wchar_t>();
}

// ==========================================================================

// constexpr bool is_empty() const noexcept
template<typename TChar>
void test_08etv5pos1r8gf99()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{};
	constexpr auto sv_2 = bstone::BasicStringView<TChar>{src_string};

	constexpr auto is_empty_1 = sv_1.is_empty();
	constexpr auto is_valid_1 = is_empty_1;

	constexpr auto is_empty_2 = sv_2.is_empty();
	constexpr auto is_valid_2 = !is_empty_2;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		true;

	tester.check(is_valid);
}

void test_08etv5pos1r8gf99_char()
{
	test_08etv5pos1r8gf99<char>();
}

void test_08etv5pos1r8gf99_wchar_t()
{
	test_08etv5pos1r8gf99<wchar_t>();
}

// ==========================================================================

template<typename TChar>
class Test_h4doovrnsodqnd0y
{
public:
	constexpr Test_h4doovrnsodqnd0y() noexcept
	{
		constexpr auto src_string = Strings<TChar>::test;

		auto sv = bstone::BasicStringView<TChar>{src_string};
		sv.remove_prefix(2);

		const auto data = sv.get_data();
		const auto is_valid_1 = data == src_string + 2;

		const auto size = sv.get_size();
		const auto is_valid_2 = size == 2;

		is_valid_ =
			is_valid_1 &&
			is_valid_2 &&
			true;
	}

	constexpr bool is_valid() const noexcept
	{
		return is_valid_;
	}

private:
	bool is_valid_{};
};

// constexpr void remove_prefix(std::intptr_t)
template<typename TChar>
void test_h4doovrnsodqnd0y()
{
	constexpr auto is_valid = Test_h4doovrnsodqnd0y<TChar>{}.is_valid();

	tester.check(is_valid);
}

void test_h4doovrnsodqnd0y_char()
{
	test_h4doovrnsodqnd0y<char>();
}

void test_h4doovrnsodqnd0y_wchar_t()
{
	test_h4doovrnsodqnd0y<wchar_t>();
}

// ==========================================================================

template<typename TChar>
class Test_p9yqq8ppiaulg9p1
{
public:
	constexpr Test_p9yqq8ppiaulg9p1() noexcept
	{
		constexpr auto src_string = Strings<TChar>::test;

		auto sv = bstone::BasicStringView<TChar>{src_string};
		sv.remove_suffix(2);

		const auto data = sv.get_data();
		const auto is_valid_1 = data == src_string;

		const auto size = sv.get_size();
		const auto is_valid_2 = size == 2;

		is_valid_ =
			is_valid_1 &&
			is_valid_2 &&
			true;
	}

	constexpr bool is_valid() const noexcept
	{
		return is_valid_;
	}

private:
	bool is_valid_{};
};

// constexpr void remove_suffix(std::intptr_t)
template<typename TChar>
void test_p9yqq8ppiaulg9p1()
{
	constexpr auto is_valid = Test_p9yqq8ppiaulg9p1<TChar>{}.is_valid();

	tester.check(is_valid);
}

void test_p9yqq8ppiaulg9p1_char()
{
	test_p9yqq8ppiaulg9p1<char>();
}

void test_p9yqq8ppiaulg9p1_wchar_t()
{
	test_p9yqq8ppiaulg9p1<wchar_t>();
}

// ==========================================================================

template<typename TChar>
class Test_qqgxzi7djyrwfcfr
{
public:
	constexpr Test_qqgxzi7djyrwfcfr() noexcept
	{
		constexpr auto src_string = Strings<TChar>::test;

		auto sv_1 = bstone::BasicStringView<TChar>{src_string};
		auto sv_2 = bstone::BasicStringView<TChar>{};

		const auto data_1 = sv_1.get_data();
		const auto is_valid_1 = data_1 == src_string;

		const auto size_1 = sv_1.get_size();
		const auto is_valid_2 = size_1 == 4;

		const auto data_2 = sv_2.get_data();
		const auto is_valid_3 = data_2 == nullptr;

		const auto size_2 = sv_2.get_size();
		const auto is_valid_4 = size_2 == 0;

		sv_1.swap(sv_2);

		const auto data_3 = sv_1.get_data();
		const auto is_valid_5 = data_3 == nullptr;

		const auto size_3 = sv_1.get_size();
		const auto is_valid_6 = size_3 == 0;

		const auto data_4 = sv_2.get_data();
		const auto is_valid_7 = data_4 == src_string;

		const auto size_4 = sv_2.get_size();
		const auto is_valid_8 = size_4 == 4;

		is_valid_ =
			is_valid_1 &&
			is_valid_2 &&
			is_valid_3 &&
			is_valid_4 &&
			is_valid_5 &&
			is_valid_6 &&
			is_valid_7 &&
			is_valid_8 &&
			true;
	}

	constexpr bool is_valid() const noexcept
	{
		return is_valid_;
	}

private:
	bool is_valid_{};
};

// constexpr void swap(BasicStringView&) noexcept
template<typename TChar>
void test_qqgxzi7djyrwfcfr()
{
	constexpr auto is_valid = Test_qqgxzi7djyrwfcfr<TChar>{}.is_valid();

	tester.check(is_valid);
}

void test_qqgxzi7djyrwfcfr_char()
{
	test_qqgxzi7djyrwfcfr<char>();
}

void test_qqgxzi7djyrwfcfr_wchar_t()
{
	test_qqgxzi7djyrwfcfr<wchar_t>();
}

// ==========================================================================

// constexpr BasicStringView get_subview(std::intptr_t) const
template<typename TChar>
void test_pbsd6o4z7p9bdnm9()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{src_string};
	constexpr auto sv = sv_1.get_subview(1);

	constexpr auto data = sv.get_data();
	constexpr auto is_valid_1 = data == src_string + 1;

	constexpr auto size = sv.get_size();
	constexpr auto is_valid_2 = size == 3;

	constexpr auto is_valid_3 =
		data[0] == 'e' &&
		data[1] == 's' &&
		data[2] == 't' &&
		true;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_pbsd6o4z7p9bdnm9_char()
{
	test_pbsd6o4z7p9bdnm9<char>();
}

void test_pbsd6o4z7p9bdnm9_wchar_t()
{
	test_pbsd6o4z7p9bdnm9<wchar_t>();
}

// ==========================================================================

// constexpr BasicStringView get_subview(std::intptr_t, std::intptr_t) const
template<typename TChar>
void test_bopexh7dko28ckjx()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{src_string};
	constexpr auto sv = sv_1.get_subview(1, 2);

	constexpr auto data = sv.get_data();
	constexpr auto is_valid_1 = data == src_string + 1;

	constexpr auto size = sv.get_size();
	constexpr auto is_valid_2 = size == 2;

	constexpr auto is_valid_3 =
		data[0] == 'e' &&
		data[1] == 's' &&
		true;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_bopexh7dko28ckjx_char()
{
	test_bopexh7dko28ckjx<char>();
}

void test_bopexh7dko28ckjx_wchar_t()
{
	test_bopexh7dko28ckjx<wchar_t>();
}

// ==========================================================================

// constexpr int compare(BasicStringView) const noexcept
template<typename TChar>
void test_uwqzhs8xl6kzdpoe()
{
	constexpr auto src_string_1 = Strings<TChar>::test;
	constexpr auto src_string_2 = Strings<TChar>::test1;
	constexpr auto src_string_3 = Strings<TChar>::u;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{};
	constexpr auto sv_2 = bstone::BasicStringView<TChar>{src_string_1};
	constexpr auto sv_3 = bstone::BasicStringView<TChar>{src_string_2};
	constexpr auto sv_4 = bstone::BasicStringView<TChar>{src_string_3};

	constexpr auto result_1 = sv_1.compare(sv_1);
	constexpr auto is_valid_1 = result_1 == 0;

	constexpr auto result_2 = sv_1.compare(sv_2);
	constexpr auto is_valid_2 = result_2 < 0;

	constexpr auto result_3 = sv_2.compare(sv_1);
	constexpr auto is_valid_3 = result_3 > 0;

	constexpr auto result_4 = sv_2.compare(sv_2);
	constexpr auto is_valid_4 = result_4 == 0;

	constexpr auto result_5 = sv_2.compare(sv_3);
	constexpr auto is_valid_5 = result_5 < 0;

	constexpr auto result_6 = sv_2.compare(sv_4);
	constexpr auto is_valid_6 = result_6 < 0;

	constexpr auto result_7 = sv_3.compare(sv_2);
	constexpr auto is_valid_7 = result_7 > 0;

	constexpr auto result_8 = sv_4.compare(sv_2);
	constexpr auto is_valid_8 = result_8 > 0;

	constexpr auto is_valid =
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

void test_uwqzhs8xl6kzdpoe_char()
{
	test_uwqzhs8xl6kzdpoe<char>();
}

void test_uwqzhs8xl6kzdpoe_wchar_t()
{
	test_uwqzhs8xl6kzdpoe<wchar_t>();
}

// ==========================================================================

// constexpr bool starts_with(Char) const noexcept
template<typename TChar>
void test_g66w8x31rg3j7r0g()
{
	constexpr auto src_string = Strings<TChar>::test;
	constexpr auto sv = bstone::BasicStringView<TChar>{src_string};

	constexpr auto result_1 = sv.starts_with(TChar{'t'});
	constexpr auto is_valid_1 = result_1;

	constexpr auto result_2 = sv.starts_with(TChar{'u'});
	constexpr auto is_valid_2 = !result_2;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		true;

	tester.check(is_valid);
}

void test_g66w8x31rg3j7r0g_char()
{
	test_g66w8x31rg3j7r0g<char>();
}

void test_g66w8x31rg3j7r0g_wchar_t()
{
	test_g66w8x31rg3j7r0g<wchar_t>();
}

// ==========================================================================

// constexpr bool starts_with(BasicStringView) const noexcept
template<typename TChar>
void test_k8mysmccu55m21xe()
{
	constexpr auto src_string_1 = Strings<TChar>::test1;
	constexpr auto src_string_2 = Strings<TChar>::test;
	constexpr auto src_string_3 = Strings<TChar>::u;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{};
	constexpr auto sv_2 = bstone::BasicStringView<TChar>{src_string_1};
	constexpr auto sv_3 = bstone::BasicStringView<TChar>{src_string_2};
	constexpr auto sv_4 = bstone::BasicStringView<TChar>{src_string_3};

	constexpr auto result_1 = sv_1.starts_with(sv_1);
	constexpr auto is_valid_1 = result_1;

	constexpr auto result_2 = sv_1.starts_with(sv_2);
	constexpr auto is_valid_2 = !result_2;

	constexpr auto result_3 = sv_2.starts_with(sv_1);
	constexpr auto is_valid_3 = result_3;

	constexpr auto result_4 = sv_2.starts_with(sv_3);
	constexpr auto is_valid_4 = result_4;

	constexpr auto result_5 = sv_2.starts_with(sv_4);
	constexpr auto is_valid_5 = !result_5;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		is_valid_5 &&
		true;

	tester.check(is_valid);
}

void test_k8mysmccu55m21xe_char()
{
	test_k8mysmccu55m21xe<char>();
}

void test_k8mysmccu55m21xe_wchar_t()
{
	test_k8mysmccu55m21xe<wchar_t>();
}

// ==========================================================================

// constexpr bool ends_with(Char) const noexcept
template<typename TChar>
void test_osgdlefyitvgi5bc()
{
	constexpr auto src_string = Strings<TChar>::test1;
	constexpr auto sv = bstone::BasicStringView<TChar>{src_string};

	constexpr auto result_1 = sv.ends_with(TChar{'1'});
	constexpr auto is_valid_1 = result_1;

	constexpr auto result_2 = sv.ends_with(TChar{'u'});
	constexpr auto is_valid_2 = !result_2;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		true;

	tester.check(is_valid);
}

void test_osgdlefyitvgi5bc_char()
{
	test_osgdlefyitvgi5bc<char>();
}

void test_osgdlefyitvgi5bc_wchar_t()
{
	test_osgdlefyitvgi5bc<wchar_t>();
}

// ==========================================================================

// constexpr bool ends_with(BasicStringView) const noexcept
template<typename TChar>
void test_fyhl3tb82owzusoh()
{
	constexpr auto src_string_1 = Strings<TChar>::one_test;
	constexpr auto src_string_2 = Strings<TChar>::test;
	constexpr auto src_string_3 = Strings<TChar>::u;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{};
	constexpr auto sv_2 = bstone::BasicStringView<TChar>{src_string_1};
	constexpr auto sv_3 = bstone::BasicStringView<TChar>{src_string_2};
	constexpr auto sv_4 = bstone::BasicStringView<TChar>{src_string_3};

	constexpr auto result_1 = sv_1.ends_with(sv_1);
	constexpr auto is_valid_1 = result_1;

	constexpr auto result_2 = sv_1.ends_with(sv_2);
	constexpr auto is_valid_2 = !result_2;

	constexpr auto result_3 = sv_2.ends_with(sv_1);
	constexpr auto is_valid_3 = result_3;

	constexpr auto result_4 = sv_2.ends_with(sv_3);
	constexpr auto is_valid_4 = result_4;

	constexpr auto result_5 = sv_2.ends_with(sv_4);
	constexpr auto is_valid_5 = !result_5;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		is_valid_5 &&
		true;

	tester.check(is_valid);
}

void test_fyhl3tb82owzusoh_char()
{
	test_fyhl3tb82owzusoh<char>();
}

void test_fyhl3tb82owzusoh_wchar_t()
{
	test_fyhl3tb82owzusoh<wchar_t>();
}

// ==========================================================================

// constexpr bool contains(Char) const noexcept
template<typename TChar>
void test_e8n3odb6r207265g()
{
	constexpr auto src_string_1 = Strings<TChar>::empty;
	constexpr auto src_string_2 = Strings<TChar>::aa_test_bb;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicStringView<TChar>{src_string_2};

	constexpr auto result_1 = sv_1.contains(TChar{'e'});
	constexpr auto is_valid_1 = !result_1;

	constexpr auto result_2 = sv_2.contains(TChar{'e'});
	constexpr auto is_valid_2 = result_2;

	constexpr auto result_3 = sv_2.contains(TChar{'?'});
	constexpr auto is_valid_3 = !result_3;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_e8n3odb6r207265g_char()
{
	test_e8n3odb6r207265g<char>();
}

void test_e8n3odb6r207265g_wchar_t()
{
	test_e8n3odb6r207265g<wchar_t>();
}

// ==========================================================================

// constexpr bool contains(BasicStringView) const noexcept
template<typename TChar>
void test_pfd7d7ff2ceqnir7()
{
	constexpr auto src_string_1 = Strings<TChar>::empty;
	constexpr auto src_string_2 = Strings<TChar>::aa_test_bb;
	constexpr auto src_string_3 = Strings<TChar>::test;
	constexpr auto src_string_4 = Strings<TChar>::test1;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicStringView<TChar>{src_string_2};
	constexpr auto sv_3 = bstone::BasicStringView<TChar>{src_string_3};
	constexpr auto sv_4 = bstone::BasicStringView<TChar>{src_string_4};

	constexpr auto result_1 = sv_1.contains(sv_1);
	constexpr auto is_valid_1 = result_1;

	constexpr auto result_2 = sv_1.contains(sv_2);
	constexpr auto is_valid_2 = !result_2;

	constexpr auto result_3 = sv_2.contains(sv_1);
	constexpr auto is_valid_3 = result_3;

	constexpr auto result_4 = sv_2.contains(sv_3);
	constexpr auto is_valid_4 = result_4;

	constexpr auto result_5 = sv_2.contains(sv_4);
	constexpr auto is_valid_5 = !result_5;

	constexpr auto result_6 = sv_3.contains(sv_4);
	constexpr auto is_valid_6 = !result_6;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		is_valid_5 &&
		is_valid_6 &&
		true;

	tester.check(is_valid);
}

void test_pfd7d7ff2ceqnir7_char()
{
	test_pfd7d7ff2ceqnir7<char>();
}

void test_pfd7d7ff2ceqnir7_wchar_t()
{
	test_pfd7d7ff2ceqnir7<wchar_t>();
}

// ==========================================================================

// constexpr std::intptr_t index_of(Char) const noexcept
template<typename TChar>
void test_k8397fucccii3cl8()
{
	constexpr auto src_string_1 = Strings<TChar>::empty;
	constexpr auto src_string_2 = Strings<TChar>::aa_test_bb;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicStringView<TChar>{src_string_2};

	constexpr auto result_1 = sv_1.index_of(TChar{'e'});
	constexpr auto is_valid_1 = result_1 < 0;

	constexpr auto result_2 = sv_2.index_of(TChar{'e'});
	constexpr auto is_valid_2 = result_2 == 4;

	constexpr auto result_3 = sv_2.index_of(TChar{'?'});
	constexpr auto is_valid_3 = result_3 < 0;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_k8397fucccii3cl8_char()
{
	test_k8397fucccii3cl8<char>();
}

void test_k8397fucccii3cl8_wchar_t()
{
	test_k8397fucccii3cl8<wchar_t>();
}

// ==========================================================================

// constexpr std::intptr_t index_of(BasicStringView) const noexcept
template<typename TChar>
void test_cb4svaoq61nfqidf()
{
	constexpr auto src_string_1 = Strings<TChar>::empty;
	constexpr auto src_string_2 = Strings<TChar>::aa_test_bb;
	constexpr auto src_string_3 = Strings<TChar>::test;
	constexpr auto src_string_4 = Strings<TChar>::test1;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicStringView<TChar>{src_string_2};
	constexpr auto sv_3 = bstone::BasicStringView<TChar>{src_string_3};
	constexpr auto sv_4 = bstone::BasicStringView<TChar>{src_string_4};

	constexpr auto result_1 = sv_1.index_of(sv_1);
	constexpr auto is_valid_1 = result_1 == 0;

	constexpr auto result_2 = sv_1.index_of(sv_2);
	constexpr auto is_valid_2 = result_2 < 0;

	constexpr auto result_3 = sv_2.index_of(sv_1);
	constexpr auto is_valid_3 = result_3 == 0;

	constexpr auto result_4 = sv_2.index_of(sv_3);
	constexpr auto is_valid_4 = result_4 == 3;

	constexpr auto result_5 = sv_2.index_of(sv_4);
	constexpr auto is_valid_5 = result_5 < 0;

	constexpr auto result_6 = sv_3.index_of(sv_4);
	constexpr auto is_valid_6 = result_6 < 0;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		is_valid_5 &&
		is_valid_6 &&
		true;

	tester.check(is_valid);
}

void test_cb4svaoq61nfqidf_char()
{
	test_cb4svaoq61nfqidf<char>();
}

void test_cb4svaoq61nfqidf_wchar_t()
{
	test_cb4svaoq61nfqidf<wchar_t>();
}

// ==========================================================================

// constexpr std::intptr_t last_index_of(Char) const noexcept
template<typename TChar>
void test_0ro7y1lzrldveta9()
{
	constexpr auto src_string_1 = Strings<TChar>::empty;
	constexpr auto src_string_2 = Strings<TChar>::aa_test_bb;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicStringView<TChar>{src_string_2};

	constexpr auto result_1 = sv_1.last_index_of(TChar{'t'});
	constexpr auto is_valid_1 = result_1 < 0;

	constexpr auto result_2 = sv_2.last_index_of(TChar{'t'});
	constexpr auto is_valid_2 = result_2 == 6;

	constexpr auto result_3 = sv_2.last_index_of(TChar{'?'});
	constexpr auto is_valid_3 = result_3 < 0;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_0ro7y1lzrldveta9_char()
{
	test_0ro7y1lzrldveta9<char>();
}

void test_0ro7y1lzrldveta9_wchar_t()
{
	test_0ro7y1lzrldveta9<wchar_t>();
}

// ==========================================================================

// constexpr std::intptr_t last_index_of(BasicStringView) const noexcept
template<typename TChar>
void test_ey9ajjtxk3rs9oq4()
{
	constexpr auto src_string_1 = Strings<TChar>::empty;
	constexpr auto src_string_2 = Strings<TChar>::aa_test_test_bb;
	constexpr auto src_string_3 = Strings<TChar>::test;
	constexpr auto src_string_4 = Strings<TChar>::test1;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicStringView<TChar>{src_string_2};
	constexpr auto sv_3 = bstone::BasicStringView<TChar>{src_string_3};
	constexpr auto sv_4 = bstone::BasicStringView<TChar>{src_string_4};

	constexpr auto result_1 = sv_1.last_index_of(sv_1);
	constexpr auto is_valid_1 = result_1 == 0;

	constexpr auto result_2 = sv_1.last_index_of(sv_2);
	constexpr auto is_valid_2 = result_2 < 0;

	constexpr auto result_3 = sv_2.last_index_of(sv_1);
	constexpr auto is_valid_3 = result_3 == 0;

	constexpr auto result_4 = sv_2.last_index_of(sv_3);
	constexpr auto is_valid_4 = result_4 == 8;

	constexpr auto result_5 = sv_2.last_index_of(sv_4);
	constexpr auto is_valid_5 = result_5 < 0;

	constexpr auto result_6 = sv_3.last_index_of(sv_4);
	constexpr auto is_valid_6 = result_6 < 0;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		is_valid_5 &&
		is_valid_6 &&
		true;

	tester.check(is_valid);
}

void test_ey9ajjtxk3rs9oq4_char()
{
	test_ey9ajjtxk3rs9oq4<char>();
}

void test_ey9ajjtxk3rs9oq4_wchar_t()
{
	test_ey9ajjtxk3rs9oq4<wchar_t>();
}

// ==========================================================================

// constexpr std::intptr_t index_of_any(Char) const noexcept
template<typename TChar>
void test_kkx2vucgrbnkxt3n()
{
	constexpr auto src_string_1 = Strings<TChar>::empty;
	constexpr auto src_string_2 = Strings<TChar>::aa_test_bb;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicStringView<TChar>{src_string_2};

	constexpr auto result_1 = sv_1.index_of_any(TChar{'e'});
	constexpr auto is_valid_1 = result_1 < 0;

	constexpr auto result_2 = sv_2.index_of_any(TChar{'e'});
	constexpr auto is_valid_2 = result_2 == 4;

	constexpr auto result_3 = sv_2.index_of_any(TChar{'?'});
	constexpr auto is_valid_3 = result_3 < 0;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_kkx2vucgrbnkxt3n_char()
{
	test_kkx2vucgrbnkxt3n<char>();
}

void test_kkx2vucgrbnkxt3n_wchar_t()
{
	test_kkx2vucgrbnkxt3n<wchar_t>();
}

// ==========================================================================

// constexpr std::intptr_t index_of_any(BasicStringView) const noexcept
template<typename TChar>
void test_hpackjsosij4uv1w()
{
	constexpr auto src_string_1 = Strings<TChar>::aa_test_bb;
	constexpr auto src_string_2 = Strings<TChar>::t;
	constexpr auto src_string_3 = Strings<TChar>::yes;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicStringView<TChar>{src_string_2};
	constexpr auto sv_3 = bstone::BasicStringView<TChar>{src_string_3};

	constexpr auto result_1 = sv_1.index_of_any(sv_2);
	constexpr auto is_valid_1 = result_1 == 3;

	constexpr auto result_2 = sv_1.index_of_any(sv_3);
	constexpr auto is_valid_2 = result_2 == 4;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		true;

	tester.check(is_valid);
}

void test_hpackjsosij4uv1w_char()
{
	test_hpackjsosij4uv1w<char>();
}

void test_hpackjsosij4uv1w_wchar_t()
{
	test_hpackjsosij4uv1w<wchar_t>();
}

// ==========================================================================

// constexpr std::intptr_t last_index_of_any(Char) const noexcept
template<typename TChar>
void test_n2l8g1j54ti0qnon()
{
	constexpr auto src_string_1 = Strings<TChar>::empty;
	constexpr auto src_string_2 = Strings<TChar>::aa_test_bb;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicStringView<TChar>{src_string_2};

	constexpr auto result_1 = sv_1.last_index_of_any(TChar{'t'});
	constexpr auto is_valid_1 = result_1 < 0;

	constexpr auto result_2 = sv_2.last_index_of_any(TChar{'t'});
	constexpr auto is_valid_2 = result_2 == 6;

	constexpr auto result_3 = sv_2.last_index_of_any(TChar{'?'});
	constexpr auto is_valid_3 = result_3 < 0;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_n2l8g1j54ti0qnon_char()
{
	test_n2l8g1j54ti0qnon<char>();
}

void test_n2l8g1j54ti0qnon_wchar_t()
{
	test_n2l8g1j54ti0qnon<wchar_t>();
}

// ==========================================================================

// constexpr std::intptr_t last_index_of_any(BasicStringView) const noexcept
template<typename TChar>
void test_mbvh29wymbz4px8x()
{
	constexpr auto src_string_1 = Strings<TChar>::aa_test_test_bb;
	constexpr auto src_string_2 = Strings<TChar>::t;
	constexpr auto src_string_3 = Strings<TChar>::yes;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicStringView<TChar>{src_string_2};
	constexpr auto sv_3 = bstone::BasicStringView<TChar>{src_string_3};

	constexpr auto result_1 = sv_1.last_index_of_any(sv_2);
	constexpr auto is_valid_1 = result_1 == 11;

	constexpr auto result_2 = sv_1.last_index_of_any(sv_3);
	constexpr auto is_valid_2 = result_2 == 10;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		true;

	tester.check(is_valid);
}

void test_mbvh29wymbz4px8x_char()
{
	test_mbvh29wymbz4px8x<char>();
}

void test_mbvh29wymbz4px8x_wchar_t()
{
	test_mbvh29wymbz4px8x<wchar_t>();
}

// ==========================================================================

// constexpr std::intptr_t index_not_of_any(Char) const noexcept
template<typename TChar>
void test_fxhsac6grntd0qon()
{
	constexpr auto src_string = Strings<TChar>::aa_test_bb;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{src_string};
	constexpr auto sv_2 = bstone::BasicStringView<TChar>{};

	constexpr auto result_1 = sv_1.index_not_of_any('a');
	constexpr auto is_valid_1 = result_1 == 2;

	constexpr auto result_2 = sv_2.index_not_of_any('a');
	constexpr auto is_valid_2 = result_2 < 0;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		true;

	tester.check(is_valid);
}

void test_fxhsac6grntd0qon_char()
{
	test_fxhsac6grntd0qon<char>();
}

void test_fxhsac6grntd0qon_wchar_t()
{
	test_fxhsac6grntd0qon<wchar_t>();
}

// ==========================================================================

// constexpr std::intptr_t index_not_of_any(BasicStringView) const noexcept
template<typename TChar>
void test_m8d3d4expk72bk2i()
{
	constexpr auto src_string_1 = Strings<TChar>::aa_test_bb;
	constexpr auto src_string_2 = Strings<TChar>::a;
	constexpr auto src_string_3 = Strings<TChar>::a_tes;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicStringView<TChar>{src_string_2};
	constexpr auto sv_3 = bstone::BasicStringView<TChar>{src_string_3};

	constexpr auto result_1 = sv_1.index_not_of_any(sv_2);
	constexpr auto is_valid_1 = result_1 == 2;

	constexpr auto result_2 = sv_1.index_not_of_any(sv_3);
	constexpr auto is_valid_2 = result_2 == 8;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		true;

	tester.check(is_valid);
}

void test_m8d3d4expk72bk2i_char()
{
	test_m8d3d4expk72bk2i<char>();
}

void test_m8d3d4expk72bk2i_wchar_t()
{
	test_m8d3d4expk72bk2i<wchar_t>();
}

// ==========================================================================

// constexpr std::intptr_t last_index_not_of_any(Char) const noexcept
template<typename TChar>
void test_vroaq5eyhsv288zm()
{
	constexpr auto src_string = Strings<TChar>::aa_test_test_bb;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{src_string};
	constexpr auto sv_2 = bstone::BasicStringView<TChar>{};

	constexpr auto result_1 = sv_1.last_index_not_of_any('b');
	constexpr auto is_valid_1 = result_1 == 12;

	constexpr auto result_2 = sv_2.last_index_not_of_any('b');
	constexpr auto is_valid_2 = result_2 < 0;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		true;

	tester.check(is_valid);
}

void test_vroaq5eyhsv288zm_char()
{
	test_vroaq5eyhsv288zm<char>();
}

void test_vroaq5eyhsv288zm_wchar_t()
{
	test_vroaq5eyhsv288zm<wchar_t>();
}

// ==========================================================================

// constexpr std::intptr_t last_index_not_of_any(BasicStringView) const noexcept
template<typename TChar>
void test_rsnbejkyw0pkmkkk()
{
	constexpr auto src_string_1 = Strings<TChar>::aa_test_test_bb;
	constexpr auto src_string_2 = Strings<TChar>::b;
	constexpr auto src_string_3 = Strings<TChar>::t_b;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicStringView<TChar>{src_string_2};
	constexpr auto sv_3 = bstone::BasicStringView<TChar>{src_string_3};

	constexpr auto result_1 = sv_1.last_index_not_of_any(sv_2);
	constexpr auto is_valid_1 = result_1 == 12;

	constexpr auto result_2 = sv_1.last_index_not_of_any(sv_3);
	constexpr auto is_valid_2 = result_2 == 10;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		true;

	tester.check(is_valid);
}

void test_rsnbejkyw0pkmkkk_char()
{
	test_rsnbejkyw0pkmkkk<char>();
}

void test_rsnbejkyw0pkmkkk_wchar_t()
{
	test_rsnbejkyw0pkmkkk<wchar_t>();
}

// ==========================================================================

// constexpr bool operator==(BasicStringView<TChar>, BasicStringView<TChar>) noexcept
template<typename TChar>
void test_9zvexhuopsdl8ctj()
{
	constexpr auto src_string_1 = Strings<TChar>::test;
	constexpr auto src_string_2 = Strings<TChar>::test1;
	constexpr auto src_string_3 = Strings<TChar>::u;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{};
	constexpr auto sv_2 = bstone::BasicStringView<TChar>{src_string_1};
	constexpr auto sv_3 = bstone::BasicStringView<TChar>{src_string_2};
	constexpr auto sv_4 = bstone::BasicStringView<TChar>{src_string_3};

	constexpr auto result_1 = sv_1 == sv_1;
	constexpr auto is_valid_1 = result_1;

	constexpr auto result_2 = sv_1 == sv_2;
	constexpr auto is_valid_2 = !result_2;

	constexpr auto result_3 = sv_2 == sv_1;
	constexpr auto is_valid_3 = !result_3;

	constexpr auto result_4 = sv_2 == sv_2;
	constexpr auto is_valid_4 = result_4;

	constexpr auto result_5 = sv_2 == sv_3;
	constexpr auto is_valid_5 = !result_5;

	constexpr auto result_6 = sv_2 == sv_4;
	constexpr auto is_valid_6 = !result_6;

	constexpr auto result_7 = sv_3 == sv_2;
	constexpr auto is_valid_7 = !result_7;

	constexpr auto result_8 = sv_4 == sv_2;
	constexpr auto is_valid_8 = !result_8;

	constexpr auto is_valid =
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

void test_9zvexhuopsdl8ctj_char()
{
	test_9zvexhuopsdl8ctj<char>();
}

void test_9zvexhuopsdl8ctj_wchar_t()
{
	test_9zvexhuopsdl8ctj<wchar_t>();
}

// ==========================================================================

// constexpr bool operator!=(BasicStringView<TChar>, BasicStringView<TChar>) noexcept
template<typename TChar>
void test_8kipycjlzukscyed()
{
	constexpr auto src_string_1 = Strings<TChar>::test;
	constexpr auto src_string_2 = Strings<TChar>::test1;
	constexpr auto src_string_3 = Strings<TChar>::u;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{};
	constexpr auto sv_2 = bstone::BasicStringView<TChar>{src_string_1};
	constexpr auto sv_3 = bstone::BasicStringView<TChar>{src_string_2};
	constexpr auto sv_4 = bstone::BasicStringView<TChar>{src_string_3};

	constexpr auto result_1 = sv_1 != sv_1;
	constexpr auto is_valid_1 = !result_1;

	constexpr auto result_2 = sv_1 != sv_2;
	constexpr auto is_valid_2 = result_2;

	constexpr auto result_3 = sv_2 != sv_1;
	constexpr auto is_valid_3 = result_3;

	constexpr auto result_4 = sv_2 != sv_2;
	constexpr auto is_valid_4 = !result_4;

	constexpr auto result_5 = sv_2 != sv_3;
	constexpr auto is_valid_5 = result_5;

	constexpr auto result_6 = sv_2 != sv_4;
	constexpr auto is_valid_6 = result_6;

	constexpr auto result_7 = sv_3 != sv_2;
	constexpr auto is_valid_7 = result_7;

	constexpr auto result_8 = sv_4 != sv_2;
	constexpr auto is_valid_8 = result_8;

	constexpr auto is_valid =
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

void test_8kipycjlzukscyed_char()
{
	test_8kipycjlzukscyed<char>();
}

void test_8kipycjlzukscyed_wchar_t()
{
	test_8kipycjlzukscyed<wchar_t>();
}

// ==========================================================================

// constexpr bool operator<(BasicStringView<TChar>, BasicStringView<TChar>) noexcept
template<typename TChar>
void test_brwf6pepo8aqijly()
{
	constexpr auto src_string_1 = Strings<TChar>::empty;
	constexpr auto src_string_2 = Strings<TChar>::test;
	constexpr auto src_string_3 = Strings<TChar>::test1;
	constexpr auto src_string_4 = Strings<TChar>::u;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicStringView<TChar>{src_string_2};
	constexpr auto sv_3 = bstone::BasicStringView<TChar>{src_string_3};
	constexpr auto sv_4 = bstone::BasicStringView<TChar>{src_string_4};

	constexpr auto result_1 = sv_2 < sv_2;
	constexpr auto is_valid_1 = !result_1;

	constexpr auto result_2 = sv_1 < sv_2;
	constexpr auto is_valid_2 = result_2;

	constexpr auto result_3 = sv_2 < sv_3;
	constexpr auto is_valid_3 = result_3;

	constexpr auto result_4 = sv_3 < sv_4;
	constexpr auto is_valid_4 = result_4;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_brwf6pepo8aqijly_char()
{
	test_brwf6pepo8aqijly<char>();
}

void test_brwf6pepo8aqijly_wchar_t()
{
	test_brwf6pepo8aqijly<wchar_t>();
}

// ==========================================================================

// constexpr bool operator<=(BasicStringView<TChar>, BasicStringView<TChar>) noexcept
template<typename TChar>
void test_qke0k9d2l1akew9k()
{
	constexpr auto src_string_1 = Strings<TChar>::empty;
	constexpr auto src_string_2 = Strings<TChar>::test;
	constexpr auto src_string_3 = Strings<TChar>::test1;
	constexpr auto src_string_4 = Strings<TChar>::u;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicStringView<TChar>{src_string_2};
	constexpr auto sv_3 = bstone::BasicStringView<TChar>{src_string_3};
	constexpr auto sv_4 = bstone::BasicStringView<TChar>{src_string_4};

	constexpr auto result_1 = sv_2 <= sv_2;
	constexpr auto is_valid_1 = result_1;

	constexpr auto result_2 = sv_1 <= sv_2;
	constexpr auto is_valid_2 = result_2;

	constexpr auto result_3 = sv_2 <= sv_3;
	constexpr auto is_valid_3 = result_3;

	constexpr auto result_4 = sv_3 <= sv_4;
	constexpr auto is_valid_4 = result_4;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_qke0k9d2l1akew9k_char()
{
	test_qke0k9d2l1akew9k<char>();
}

void test_qke0k9d2l1akew9k_wchar_t()
{
	test_qke0k9d2l1akew9k<wchar_t>();
}

// ==========================================================================

// constexpr bool operator>(BasicStringView<TChar>, BasicStringView<TChar>) noexcept
template<typename TChar>
void test_7oh2o25j61f5z6vt()
{
	constexpr auto src_string_1 = Strings<TChar>::empty;
	constexpr auto src_string_2 = Strings<TChar>::test;
	constexpr auto src_string_3 = Strings<TChar>::test1;
	constexpr auto src_string_4 = Strings<TChar>::u;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicStringView<TChar>{src_string_2};
	constexpr auto sv_3 = bstone::BasicStringView<TChar>{src_string_3};
	constexpr auto sv_4 = bstone::BasicStringView<TChar>{src_string_4};

	constexpr auto result_1 = sv_2 > sv_2;
	constexpr auto is_valid_1 = !result_1;

	constexpr auto result_2 = sv_2 > sv_1;
	constexpr auto is_valid_2 = result_2;

	constexpr auto result_3 = sv_3 > sv_2;
	constexpr auto is_valid_3 = result_3;

	constexpr auto result_4 = sv_4 > sv_3;
	constexpr auto is_valid_4 = result_4;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_7oh2o25j61f5z6vt_char()
{
	test_7oh2o25j61f5z6vt<char>();
}

void test_7oh2o25j61f5z6vt_wchar_t()
{
	test_7oh2o25j61f5z6vt<wchar_t>();
}

// ==========================================================================

// constexpr bool operator>=(BasicStringView<TChar>, BasicStringView<TChar>) noexcept
template<typename TChar>
void test_qj1mh1p3l2za95no()
{
	constexpr auto src_string_1 = Strings<TChar>::empty;
	constexpr auto src_string_2 = Strings<TChar>::test;
	constexpr auto src_string_3 = Strings<TChar>::test1;
	constexpr auto src_string_4 = Strings<TChar>::u;

	constexpr auto sv_1 = bstone::BasicStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicStringView<TChar>{src_string_2};
	constexpr auto sv_3 = bstone::BasicStringView<TChar>{src_string_3};
	constexpr auto sv_4 = bstone::BasicStringView<TChar>{src_string_4};

	constexpr auto result_1 = sv_2 >= sv_2;
	constexpr auto is_valid_1 = result_1;

	constexpr auto result_2 = sv_2 >= sv_1;
	constexpr auto is_valid_2 = result_2;

	constexpr auto result_3 = sv_3 >= sv_2;
	constexpr auto is_valid_3 = result_3;

	constexpr auto result_4 = sv_4 >= sv_3;
	constexpr auto is_valid_4 = result_4;

	constexpr auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

void test_qj1mh1p3l2za95no_char()
{
	test_qj1mh1p3l2za95no<char>();
}

void test_qj1mh1p3l2za95no_wchar_t()
{
	test_qj1mh1p3l2za95no<wchar_t>();
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_ctor();
		register_assignment_operator();
		register_begin();
		register_cbegin();
		register_end();
		register_cend();
		register_subscript_operator();
		register_get_front();
		register_get_back();
		register_get_data();
		register_has_data();
		register_get_size();
		register_is_empty();
		register_remove_prefix();
		register_remove_suffix();
		register_swap();
		register_get_subview();
		register_compare();
		register_starts_with();
		register_ends_with();
		register_contains();
		register_index_of();
		register_last_index_of();
		register_index_of_any();
		register_last_index_of_any();
		register_index_not_of_any();
		register_last_index_not_of_any();
		register_operator_equal_to();
		register_operator_not_equal_to();
		register_operator_less_than();
		register_operator_less_than_or_equal_to();
		register_operator_greater_than();
		register_operator_greater_than_or_equal_to();
	}

private:
	void register_ctor()
	{
		tester.register_test("BasicStringView#7j68r9171sosy4l3", test_av45voi54f4btjcw_char);
		tester.register_test("BasicStringView#b6ofvzsm30och5q4", test_av45voi54f4btjcw_wchar_t);

		tester.register_test("BasicStringView#8spawp38pwvakfcp", test_9viu2sbdg5qfgezi_char);
		tester.register_test("BasicStringView#jgtu8zn7ckycf5d4", test_9viu2sbdg5qfgezi_wchar_t);

		tester.register_test("BasicStringView#x8a2p4u4kvomwdj1", test_leht3ef2qbhbvlk8_char);
		tester.register_test("BasicStringView#vtooniyp6sv6fxnn", test_leht3ef2qbhbvlk8_wchar_t);

		tester.register_test("BasicStringView#pd6kawcdwobxsu8c", test_abnsmbym1acwzdqk_char);
		tester.register_test("BasicStringView#sprft349f2uwf1nw", test_abnsmbym1acwzdqk_wchar_t);

		tester.register_test("BasicStringView#qzb7bcjfbbt8290z", test_hqvek4n9ws8pky97_char);
		tester.register_test("BasicStringView#k9dwa3bsn5e10jic", test_hqvek4n9ws8pky97_wchar_t);

		tester.register_test("BasicStringView#5oryhcsyqeu4xd9u", test_a86kmgzvs440xahn_char);
		tester.register_test("BasicStringView#7r9etme722tnxohm", test_a86kmgzvs440xahn_wchar_t);
	}

	void register_assignment_operator()
	{
		tester.register_test("BasicStringView#x2tla55ll78ma24g", test_47gff4scnmvsp3vd_char);
		tester.register_test("BasicStringView#p02gthh9v6hv7zi2", test_47gff4scnmvsp3vd_wchar_t);
	}

	void register_begin()
	{
		tester.register_test("BasicStringView#voollas8bk96gh9w", test_5caelu43w4p0fwrx_char);
		tester.register_test("BasicStringView#zzdlees0wgssg7gq", test_5caelu43w4p0fwrx_wchar_t);
	}

	void register_cbegin()
	{
		tester.register_test("BasicStringView#tab67kp32z8k0c9v", test_33m2dh4h4pslbxze_char);
		tester.register_test("BasicStringView#yaog5vm6k1xlyhu6", test_33m2dh4h4pslbxze_wchar_t);
	}

	void register_end()
	{
		tester.register_test("BasicStringView#1mryo4vh0fojmhe9", test_fpwgqzy1dlty1cx1_char);
		tester.register_test("BasicStringView#v6otaud33zfj1o8p", test_fpwgqzy1dlty1cx1_wchar_t);
	}

	void register_cend()
	{
		tester.register_test("BasicStringView#yjhw1ypqpacoe7r7", test_cralngzuccb3zj33_char);
		tester.register_test("BasicStringView#bnukrcptr9rjipcg", test_cralngzuccb3zj33_wchar_t);
	}

	void register_subscript_operator()
	{
		tester.register_test("BasicStringView#7mxdnqcm8c3l1wip", test_kp7o2h051zq4ibgj_char);
		tester.register_test("BasicStringView#xdxdpke3rcmmljcv", test_kp7o2h051zq4ibgj_wchar_t);
	}

	void register_get_front()
	{
		tester.register_test("BasicStringView#hrjimn2c0zfae298", test_d1i9i3cehotrl9wk_char);
		tester.register_test("BasicStringView#h846r7gbm0vrlskm", test_d1i9i3cehotrl9wk_wchar_t);
	}

	void register_get_back()
	{
		tester.register_test("BasicStringView#v6mf6v4fbxambzvc", test_6tlkmc9ldvawh8hl_char);
		tester.register_test("BasicStringView#mtdtkss8prn897pu", test_6tlkmc9ldvawh8hl_wchar_t);
	}

	void register_get_data()
	{
		tester.register_test("BasicStringView#ymcxyvja3c9mvexh", test_gvb0khum3tk2o7cv_char);
		tester.register_test("BasicStringView#ncrpxzi0d2o2pvlm", test_gvb0khum3tk2o7cv_wchar_t);
	}

	void register_has_data()
	{
		tester.register_test("BasicStringView#kiggnk63q8pplslj", test_wmeyb4p34n195qg6_char);
		tester.register_test("BasicStringView#rn4cqqjrwldgmbdc", test_wmeyb4p34n195qg6_wchar_t);
	}

	void register_get_size()
	{
		tester.register_test("BasicStringView#trpkct3mojxc8c6m", test_4scxk1o1cx2s9xpp_char);
		tester.register_test("BasicStringView#u4b5totpy535idb6", test_4scxk1o1cx2s9xpp_wchar_t);
	}

	void register_is_empty()
	{
		tester.register_test("BasicStringView#jq4tda8qpmnrhx68", test_08etv5pos1r8gf99_char);
		tester.register_test("BasicStringView#qef7ihitzxypgaa2", test_08etv5pos1r8gf99_wchar_t);
	}

	void register_remove_prefix()
	{
		tester.register_test("BasicStringView#y14iepb1e6v6i4or", test_h4doovrnsodqnd0y_char);
		tester.register_test("BasicStringView#1aot75gyaymwwt0x", test_h4doovrnsodqnd0y_wchar_t);
	}

	void register_remove_suffix()
	{
		tester.register_test("BasicStringView#cpegsso78c6mlvvf", test_p9yqq8ppiaulg9p1_char);
		tester.register_test("BasicStringView#ba41ua9vyniotnxa", test_p9yqq8ppiaulg9p1_wchar_t);
	}

	void register_swap()
	{
		tester.register_test("BasicStringView#5y8c781k99i1xqot", test_qqgxzi7djyrwfcfr_char);
		tester.register_test("BasicStringView#4n4kxp598zsy148y", test_qqgxzi7djyrwfcfr_wchar_t);
	}

	void register_get_subview()
	{
		tester.register_test("BasicStringView#7e5ibjbzcjci3sbh", test_pbsd6o4z7p9bdnm9_char);
		tester.register_test("BasicStringView#9btn9ozfudoajcy2", test_pbsd6o4z7p9bdnm9_wchar_t);

		tester.register_test("BasicStringView#cl3rsxclrt96j2bx", test_bopexh7dko28ckjx_char);
		tester.register_test("BasicStringView#u22hgumpeyoaucc6", test_bopexh7dko28ckjx_wchar_t);
	}

	void register_compare()
	{
		tester.register_test("BasicStringView#xvgb8yrpob85kzf1", test_uwqzhs8xl6kzdpoe_char);
		tester.register_test("BasicStringView#3tara8h6dh5y6l5r", test_uwqzhs8xl6kzdpoe_wchar_t);
	}

	void register_starts_with()
	{
		tester.register_test("BasicStringView#nkyyvbmjweaplyc0", test_g66w8x31rg3j7r0g_char);
		tester.register_test("BasicStringView#hd5d7mcc0lajyya3", test_g66w8x31rg3j7r0g_wchar_t);

		tester.register_test("BasicStringView#86yuibork6bwx8mz", test_k8mysmccu55m21xe_char);
		tester.register_test("BasicStringView#yf8ae4hkfjku58jz", test_k8mysmccu55m21xe_wchar_t);
	}

	void register_ends_with()
	{
		tester.register_test("BasicStringView#1dmi6qlppqnawdcj", test_osgdlefyitvgi5bc_char);
		tester.register_test("BasicStringView#dvfayyepgwuupitl", test_osgdlefyitvgi5bc_wchar_t);

		tester.register_test("BasicStringView#fawh25w1a5n067rn", test_fyhl3tb82owzusoh_char);
		tester.register_test("BasicStringView#n0sfqlw4nevlxsm6", test_fyhl3tb82owzusoh_wchar_t);
	}

	void register_contains()
	{
		tester.register_test("BasicStringView#f33axltsp8tai4uw", test_e8n3odb6r207265g_char);
		tester.register_test("BasicStringView#kji6rbp0p0tnvk0p", test_e8n3odb6r207265g_wchar_t);

		tester.register_test("BasicStringView#mbeptnwfurbep278", test_pfd7d7ff2ceqnir7_char);
		tester.register_test("BasicStringView#utrmgdd4h24j0cu0", test_pfd7d7ff2ceqnir7_wchar_t);
	}

	void register_index_of()
	{
		tester.register_test("BasicStringView#6b6ofc3490yer2x0", test_k8397fucccii3cl8_char);
		tester.register_test("BasicStringView#dvriokdejerxpnau", test_k8397fucccii3cl8_wchar_t);

		tester.register_test("BasicStringView#28tchxpfjyyk6h8z", test_cb4svaoq61nfqidf_char);
		tester.register_test("BasicStringView#r9rsm5yrchxgpwje", test_cb4svaoq61nfqidf_wchar_t);
	}

	void register_last_index_of()
	{
		tester.register_test("BasicStringView#zuxnct5as2bhwgbr", test_0ro7y1lzrldveta9_char);
		tester.register_test("BasicStringView#lq6h1vrwzw7xq4ot", test_0ro7y1lzrldveta9_wchar_t);

		tester.register_test("BasicStringView#rec8xiczsljwfkse", test_ey9ajjtxk3rs9oq4_char);
		tester.register_test("BasicStringView#6re9bredvxm91gbc", test_ey9ajjtxk3rs9oq4_wchar_t);
	}

	void register_index_of_any()
	{
		tester.register_test("BasicStringView#uwpl8gw8iwij2g34", test_kkx2vucgrbnkxt3n_char);
		tester.register_test("BasicStringView#mma4ger75868j1ur", test_kkx2vucgrbnkxt3n_wchar_t);

		tester.register_test("BasicStringView#qwoevkr77n7e979z", test_hpackjsosij4uv1w_char);
		tester.register_test("BasicStringView#rfca403gjt3gqi5f", test_hpackjsosij4uv1w_wchar_t);
	}

	void register_last_index_of_any()
	{
		tester.register_test("BasicStringView#kpeeu1vn1p1m34s4", test_n2l8g1j54ti0qnon_char);
		tester.register_test("BasicStringView#cwf8u9j5z2zplm68", test_n2l8g1j54ti0qnon_wchar_t);

		tester.register_test("BasicStringView#1yq2chaev9wzva6j", test_mbvh29wymbz4px8x_char);
		tester.register_test("BasicStringView#1dzuwtxq24407cy1", test_mbvh29wymbz4px8x_wchar_t);
	}

	void register_index_not_of_any()
	{
		tester.register_test("BasicStringView#zcop3hotfbk905zp", test_fxhsac6grntd0qon_char);
		tester.register_test("BasicStringView#3armqocnj2iwnaob", test_fxhsac6grntd0qon_wchar_t);

		tester.register_test("BasicStringView#9mxvwwpt0svud7w9", test_m8d3d4expk72bk2i_char);
		tester.register_test("BasicStringView#qxuce5n9n5m3scbh", test_m8d3d4expk72bk2i_wchar_t);
	}

	void register_last_index_not_of_any()
	{
		tester.register_test("BasicStringView#f3fg16n6j37fxgqm", test_vroaq5eyhsv288zm_char);
		tester.register_test("BasicStringView#czdx2kkdyfk9glic", test_vroaq5eyhsv288zm_wchar_t);

		tester.register_test("BasicStringView#d54v44kmp1mxrz1s", test_rsnbejkyw0pkmkkk_char);
		tester.register_test("BasicStringView#vdicdgdjh98kfjv4", test_rsnbejkyw0pkmkkk_wchar_t);
	}

	void register_operator_equal_to()
	{
		tester.register_test("BasicStringView#fxn90fbcfl58jsia", test_9zvexhuopsdl8ctj_char);
		tester.register_test("BasicStringView#4x9ci42wn9i49d63", test_9zvexhuopsdl8ctj_wchar_t);
	}

	void register_operator_not_equal_to()
	{
		tester.register_test("BasicStringView#fvbrxzqnwvbuoi3l", test_8kipycjlzukscyed_char);
		tester.register_test("BasicStringView#kmlihflzmrub3hc6", test_8kipycjlzukscyed_wchar_t);
	}

	void register_operator_less_than()
	{
		tester.register_test("BasicStringView#tmbrmnu1zje1kekj", test_brwf6pepo8aqijly_char);
		tester.register_test("BasicStringView#mjgk6trmu9uggu3r", test_brwf6pepo8aqijly_wchar_t);
	}

	void register_operator_less_than_or_equal_to()
	{
		tester.register_test("BasicStringView#nm29pogmh0ay3ywb", test_qke0k9d2l1akew9k_char);
		tester.register_test("BasicStringView#k9yjnzg83cpa2p1j", test_qke0k9d2l1akew9k_wchar_t);
	}

	void register_operator_greater_than()
	{
		tester.register_test("BasicStringView#xqa798ol75kp9im4", test_7oh2o25j61f5z6vt_char);
		tester.register_test("BasicStringView#3kzh50055ajlfc6l", test_7oh2o25j61f5z6vt_wchar_t);
	}

	void register_operator_greater_than_or_equal_to()
	{
		tester.register_test("BasicStringView#qa8ngx0mw10pa9a9", test_qj1mh1p3l2za95no_char);
		tester.register_test("BasicStringView#5014o68o6ri3k18c", test_qj1mh1p3l2za95no_wchar_t);
	}
};

auto registrator = Registrator{};

} // namespace
