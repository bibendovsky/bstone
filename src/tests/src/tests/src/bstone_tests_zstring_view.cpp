#include <type_traits>

#include "bstone_zstring_view.h"
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
};

// ==========================================================================

// constexpr BasicZStringView()
template<typename TChar>
void test_aw7cpphusqlv650c()
{
	constexpr auto sv = bstone::BasicZStringView<TChar>{};

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

void test_aw7cpphusqlv650c_char()
{
	test_aw7cpphusqlv650c<char>();
}

void test_aw7cpphusqlv650c_wchar_t()
{
	test_aw7cpphusqlv650c<wchar_t>();
}

// ==========================================================================

// constexpr BasicZStringView(const BasicZStringView&)
template<typename TChar>
void test_hv98okrz5vvpvvw7()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv_1 = bstone::BasicZStringView<TChar>{src_string};
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

void test_hv98okrz5vvpvvw7_char()
{
	test_hv98okrz5vvpvvw7<char>();
}

void test_hv98okrz5vvpvvw7_wchar_t()
{
	test_hv98okrz5vvpvvw7<wchar_t>();
}

// ==========================================================================

// constexpr BasicZStringView(const Char*)
template<typename TChar>
void test_5lj766w71688do43()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv_1 = bstone::BasicZStringView<TChar>{src_string};
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

void test_5lj766w71688do43_char()
{
	test_5lj766w71688do43<char>();
}

void test_5lj766w71688do43_wchar_t()
{
	test_5lj766w71688do43<wchar_t>();
}

// ==========================================================================

// BasicZStringView(std::nullptr_t) = delete
template<typename TChar>
void test_0i4ouufm6p869wgh()
{
	constexpr auto is_valid = !HasNullptrCtor<bstone::BasicZStringView<TChar>>::value;

	tester.check(is_valid);
}

void test_0i4ouufm6p869wgh_char()
{
	test_0i4ouufm6p869wgh<char>();
}

void test_0i4ouufm6p869wgh_wchar_t()
{
	test_0i4ouufm6p869wgh<wchar_t>();
}

// ==========================================================================

template<typename TChar>
class Test_9ai1dbgbb81vvjgo
{
public:
	constexpr Test_9ai1dbgbb81vvjgo() noexcept
	{
		constexpr auto src_string = Strings<TChar>::test;

		const auto sv_1 = bstone::BasicZStringView<TChar>{src_string};
		auto sv = bstone::BasicZStringView<TChar>{};
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

// BasicZStringView& operator=(const BasicZStringView&)
template<typename TChar>
void test_9ai1dbgbb81vvjgo()
{
	constexpr auto is_valid = Test_9ai1dbgbb81vvjgo<TChar>{}.is_valid();

	tester.check(is_valid);
}

void test_9ai1dbgbb81vvjgo_char()
{
	test_9ai1dbgbb81vvjgo<char>();
}

void test_9ai1dbgbb81vvjgo_wchar_t()
{
	test_9ai1dbgbb81vvjgo<wchar_t>();
}

// ==========================================================================

// constexpr const Char* begin() const noexcept
template<typename TChar>
void test_lm5vhcm5l3rwxvc5()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv = bstone::BasicZStringView<TChar>{src_string};

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

void test_lm5vhcm5l3rwxvc5_char()
{
	test_lm5vhcm5l3rwxvc5<char>();
}

void test_lm5vhcm5l3rwxvc5_wchar_t()
{
	test_lm5vhcm5l3rwxvc5<wchar_t>();
}

// ==========================================================================

// constexpr const Char* cbegin() const noexcept
template<typename TChar>
void test_glzhw581vwr2qiqy()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv = bstone::BasicZStringView<TChar>{src_string};

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

void test_glzhw581vwr2qiqy_char()
{
	test_glzhw581vwr2qiqy<char>();
}

void test_glzhw581vwr2qiqy_wchar_t()
{
	test_glzhw581vwr2qiqy<wchar_t>();
}

// ==========================================================================

// constexpr const Char* end() const noexcept
template<typename TChar>
void test_pcx3zznvvy6bghsg()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv = bstone::BasicZStringView<TChar>{src_string};

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

void test_pcx3zznvvy6bghsg_char()
{
	test_pcx3zznvvy6bghsg<char>();
}

void test_pcx3zznvvy6bghsg_wchar_t()
{
	test_pcx3zznvvy6bghsg<wchar_t>();
}

// ==========================================================================

// constexpr const Char* cend() const noexcept
template<typename TChar>
void test_xyflxt2c0vgyagdm()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv = bstone::BasicZStringView<TChar>{src_string};

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

void test_xyflxt2c0vgyagdm_char()
{
	test_xyflxt2c0vgyagdm<char>();
}

void test_xyflxt2c0vgyagdm_wchar_t()
{
	test_xyflxt2c0vgyagdm<wchar_t>();
}

// ==========================================================================

// constexpr operator BasicStringView<Char>() const
template<typename TChar>
void test_qshhzzwkuhxddlh0()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv_1 = bstone::BasicZStringView<TChar>{src_string};
	constexpr auto data_1 = sv_1.get_data();
	constexpr auto size_1 = sv_1.get_size();

	constexpr auto sv_2 = static_cast<bstone::BasicStringView<TChar>>(sv_1);
	constexpr auto data_2 = sv_2.get_data();
	constexpr auto size_2 = sv_2.get_size();

	constexpr auto is_valid_1 =
		size_1 == size_2 &&
		size_1 == 4 &&
		data_1[0] == data_2[0] &&
		data_1[1] == data_2[1] &&
		data_1[2] == data_2[2] &&
		data_1[3] == data_2[3] &&
		data_1[0] == 't' &&
		data_1[1] == 'e' &&
		data_1[2] == 's' &&
		data_1[3] == 't' &&
		true;

	constexpr auto is_valid =
		is_valid_1 &&
		true;

	tester.check(is_valid);
}

void test_qshhzzwkuhxddlh0_char()
{
	test_qshhzzwkuhxddlh0<char>();
}

void test_qshhzzwkuhxddlh0_wchar_t()
{
	test_qshhzzwkuhxddlh0<wchar_t>();
}

// ==========================================================================

// constexpr const Char& operator[](std::intptr_t) const
template<typename TChar>
void test_m9w6fhjxli1gnckm()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv = bstone::BasicZStringView<TChar>{src_string};

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

void test_m9w6fhjxli1gnckm_char()
{
	test_m9w6fhjxli1gnckm<char>();
}

void test_m9w6fhjxli1gnckm_wchar_t()
{
	test_m9w6fhjxli1gnckm<wchar_t>();
}

// ==========================================================================

// constexpr const Char& get_front() const
template<typename TChar>
void test_h2slm58x68z8wsyw()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv = bstone::BasicZStringView<TChar>{src_string};

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

void test_h2slm58x68z8wsyw_char()
{
	test_h2slm58x68z8wsyw<char>();
}

void test_h2slm58x68z8wsyw_wchar_t()
{
	test_h2slm58x68z8wsyw<wchar_t>();
}

// ==========================================================================

// constexpr const Char& get_back() const
template<typename TChar>
void test_98ss8s7e5bp9hgei()
{
	constexpr auto src_string = Strings<TChar>::yes;

	constexpr auto sv = bstone::BasicZStringView<TChar>{src_string};

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

void test_98ss8s7e5bp9hgei_char()
{
	test_98ss8s7e5bp9hgei<char>();
}

void test_98ss8s7e5bp9hgei_wchar_t()
{
	test_98ss8s7e5bp9hgei<wchar_t>();
}

// ==========================================================================

// constexpr const Char* get_data() const noexcept
template<typename TChar>
void test_9ze7j48hmqwsyciy()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv = bstone::BasicZStringView<TChar>{src_string};

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

void test_9ze7j48hmqwsyciy_char()
{
	test_9ze7j48hmqwsyciy<char>();
}

void test_9ze7j48hmqwsyciy_wchar_t()
{
	test_9ze7j48hmqwsyciy<wchar_t>();
}

// ==========================================================================

// constexpr bool has_data() const noexcept
template<typename TChar>
void test_vhas56q51cm9v963()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv_1 = bstone::BasicZStringView<TChar>{};
	constexpr auto sv_2 = bstone::BasicZStringView<TChar>{src_string};

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

void test_vhas56q51cm9v963_char()
{
	test_vhas56q51cm9v963<char>();
}

void test_vhas56q51cm9v963_wchar_t()
{
	test_vhas56q51cm9v963<wchar_t>();
}

// ==========================================================================

// constexpr std::intptr_t get_size() const noexcept
template<typename TChar>
void test_k59wb2kzkcpsdwlw()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv_1 = bstone::BasicZStringView<TChar>{};
	constexpr auto sv_2 = bstone::BasicZStringView<TChar>{src_string};

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

void test_k59wb2kzkcpsdwlw_char()
{
	test_k59wb2kzkcpsdwlw<char>();
}

void test_k59wb2kzkcpsdwlw_wchar_t()
{
	test_k59wb2kzkcpsdwlw<wchar_t>();
}

// ==========================================================================

// constexpr bool is_empty() const noexcept
template<typename TChar>
void test_4lag028hueofnxoz()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv_1 = bstone::BasicZStringView<TChar>{};
	constexpr auto sv_2 = bstone::BasicZStringView<TChar>{src_string};

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

void test_4lag028hueofnxoz_char()
{
	test_4lag028hueofnxoz<char>();
}

void test_4lag028hueofnxoz_wchar_t()
{
	test_4lag028hueofnxoz<wchar_t>();
}

// ==========================================================================

template<typename TChar>
class Test_ae4q6ahbkuzv0a3e
{
public:
	constexpr Test_ae4q6ahbkuzv0a3e() noexcept
	{
		constexpr auto src_string = Strings<TChar>::test;

		auto sv = bstone::BasicZStringView<TChar>{src_string};
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
void test_ae4q6ahbkuzv0a3e()
{
	constexpr auto is_valid = Test_ae4q6ahbkuzv0a3e<TChar>{}.is_valid();

	tester.check(is_valid);
}

void test_ae4q6ahbkuzv0a3e_char()
{
	test_ae4q6ahbkuzv0a3e<char>();
}

void test_ae4q6ahbkuzv0a3e_wchar_t()
{
	test_ae4q6ahbkuzv0a3e<wchar_t>();
}

// ==========================================================================

template<typename TChar>
class Test_vddjcmr53e22opf2
{
public:
	constexpr Test_vddjcmr53e22opf2() noexcept
	{
		constexpr auto src_string = Strings<TChar>::test;

		auto sv_1 = bstone::BasicZStringView<TChar>{src_string};
		auto sv_2 = bstone::BasicZStringView<TChar>{};

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

// constexpr void swap(BasicZStringView&) noexcept
template<typename TChar>
void test_vddjcmr53e22opf2()
{
	constexpr auto is_valid = Test_vddjcmr53e22opf2<TChar>{}.is_valid();

	tester.check(is_valid);
}

void test_vddjcmr53e22opf2_char()
{
	test_vddjcmr53e22opf2<char>();
}

void test_vddjcmr53e22opf2_wchar_t()
{
	test_vddjcmr53e22opf2<wchar_t>();
}

// ==========================================================================

// constexpr BasicZStringView get_subview(std::intptr_t) const
template<typename TChar>
void test_gb86ia8b5ryz5aig()
{
	constexpr auto src_string = Strings<TChar>::test;

	constexpr auto sv_1 = bstone::BasicZStringView<TChar>{src_string};
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

void test_gb86ia8b5ryz5aig_char()
{
	test_gb86ia8b5ryz5aig<char>();
}

void test_gb86ia8b5ryz5aig_wchar_t()
{
	test_gb86ia8b5ryz5aig<wchar_t>();
}

// ==========================================================================

// constexpr int compare(BasicZStringView) const noexcept
template<typename TChar>
void test_obxlwkr9o9w5mj9j()
{
	constexpr auto src_string_1 = Strings<TChar>::test;
	constexpr auto src_string_2 = Strings<TChar>::test1;
	constexpr auto src_string_3 = Strings<TChar>::u;

	constexpr auto sv_1 = bstone::BasicZStringView<TChar>{};
	constexpr auto sv_2 = bstone::BasicZStringView<TChar>{src_string_1};
	constexpr auto sv_3 = bstone::BasicZStringView<TChar>{src_string_2};
	constexpr auto sv_4 = bstone::BasicZStringView<TChar>{src_string_3};

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

void test_obxlwkr9o9w5mj9j_char()
{
	test_obxlwkr9o9w5mj9j<char>();
}

void test_obxlwkr9o9w5mj9j_wchar_t()
{
	test_obxlwkr9o9w5mj9j<wchar_t>();
}

// ==========================================================================

// constexpr bool starts_with(BasicZStringView) const noexcept
template<typename TChar>
void test_ygakn1ivtn0wqto0()
{
	constexpr auto src_string_1 = Strings<TChar>::test1;
	constexpr auto src_string_2 = Strings<TChar>::test;
	constexpr auto src_string_3 = Strings<TChar>::u;

	constexpr auto sv_1 = bstone::BasicZStringView<TChar>{};
	constexpr auto sv_2 = bstone::BasicZStringView<TChar>{src_string_1};
	constexpr auto sv_3 = bstone::BasicZStringView<TChar>{src_string_2};
	constexpr auto sv_4 = bstone::BasicZStringView<TChar>{src_string_3};

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

void test_ygakn1ivtn0wqto0_char()
{
	test_ygakn1ivtn0wqto0<char>();
}

void test_ygakn1ivtn0wqto0_wchar_t()
{
	test_ygakn1ivtn0wqto0<wchar_t>();
}

// ==========================================================================

// constexpr bool starts_with(Char) const noexcept
template<typename TChar>
void test_h4s5ytj68szcubu2()
{
	constexpr auto src_string = Strings<TChar>::test;
	constexpr auto sv = bstone::BasicZStringView<TChar>{src_string};

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

void test_h4s5ytj68szcubu2_char()
{
	test_h4s5ytj68szcubu2<char>();
}

void test_h4s5ytj68szcubu2_wchar_t()
{
	test_h4s5ytj68szcubu2<wchar_t>();
}

// ==========================================================================

// constexpr bool starts_with(const Char*) const noexcept
template<typename TChar>
void test_29j8z5jyphz5zs4m()
{
	constexpr auto src_string_1 = Strings<TChar>::empty;
	constexpr auto src_string_2 = Strings<TChar>::test1;
	constexpr auto src_string_3 = Strings<TChar>::test;
	constexpr auto src_string_4 = Strings<TChar>::u;

	constexpr auto sv_1 = bstone::BasicZStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicZStringView<TChar>{src_string_2};

	constexpr auto result_1 = sv_1.starts_with(src_string_1);
	constexpr auto is_valid_1 = result_1;

	constexpr auto result_2 = sv_1.starts_with(src_string_2);
	constexpr auto is_valid_2 = !result_2;

	constexpr auto result_3 = sv_2.starts_with(src_string_1);
	constexpr auto is_valid_3 = result_3;

	constexpr auto result_4 = sv_2.starts_with(src_string_3);
	constexpr auto is_valid_4 = result_4;

	constexpr auto result_5 = sv_2.starts_with(src_string_4);
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

void test_29j8z5jyphz5zs4m_char()
{
	test_29j8z5jyphz5zs4m<char>();
}

void test_29j8z5jyphz5zs4m_wchar_t()
{
	test_29j8z5jyphz5zs4m<wchar_t>();
}

// ==========================================================================

// constexpr bool ends_with(BasicZStringView) const noexcept
template<typename TChar>
void test_kwdv4fkv77cuespi()
{
	constexpr auto src_string_1 = Strings<TChar>::one_test;
	constexpr auto src_string_2 = Strings<TChar>::test;
	constexpr auto src_string_3 = Strings<TChar>::u;

	constexpr auto sv_1 = bstone::BasicZStringView<TChar>{};
	constexpr auto sv_2 = bstone::BasicZStringView<TChar>{src_string_1};
	constexpr auto sv_3 = bstone::BasicZStringView<TChar>{src_string_2};
	constexpr auto sv_4 = bstone::BasicZStringView<TChar>{src_string_3};

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

void test_kwdv4fkv77cuespi_char()
{
	test_kwdv4fkv77cuespi<char>();
}

void test_kwdv4fkv77cuespi_wchar_t()
{
	test_kwdv4fkv77cuespi<wchar_t>();
}

// ==========================================================================

// constexpr bool ends_with(Char) const noexcept
template<typename TChar>
void test_cmzg12qjl5frj8e4()
{
	constexpr auto src_string = Strings<TChar>::test1;
	constexpr auto sv = bstone::BasicZStringView<TChar>{src_string};

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

void test_cmzg12qjl5frj8e4_char()
{
	test_cmzg12qjl5frj8e4<char>();
}

void test_cmzg12qjl5frj8e4_wchar_t()
{
	test_cmzg12qjl5frj8e4<wchar_t>();
}

// ==========================================================================

// constexpr bool ends_with(const Char*) const noexcept
template<typename TChar>
void test_ymrzkvqi461cbf55()
{
	constexpr auto src_string_1 = Strings<TChar>::empty;
	constexpr auto src_string_2 = Strings<TChar>::one_test;
	constexpr auto src_string_3 = Strings<TChar>::test;
	constexpr auto src_string_4 = Strings<TChar>::u;

	constexpr auto sv_1 = bstone::BasicZStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicZStringView<TChar>{src_string_2};

	constexpr auto result_1 = sv_1.ends_with(src_string_1);
	constexpr auto is_valid_1 = result_1;

	constexpr auto result_2 = sv_1.ends_with(src_string_2);
	constexpr auto is_valid_2 = !result_2;

	constexpr auto result_3 = sv_2.ends_with(src_string_1);
	constexpr auto is_valid_3 = result_3;

	constexpr auto result_4 = sv_2.ends_with(src_string_3);
	constexpr auto is_valid_4 = result_4;

	constexpr auto result_5 = sv_2.ends_with(src_string_4);
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

void test_ymrzkvqi461cbf55_char()
{
	test_ymrzkvqi461cbf55<char>();
}

void test_ymrzkvqi461cbf55_wchar_t()
{
	test_ymrzkvqi461cbf55<wchar_t>();
}

// ==========================================================================

// constexpr bool contains(BasicZStringView) const noexcept
template<typename TChar>
void test_7mzyiu7wx0aso1id()
{
	constexpr auto src_string_1 = Strings<TChar>::empty;
	constexpr auto src_string_2 = Strings<TChar>::aa_test_bb;
	constexpr auto src_string_3 = Strings<TChar>::test;
	constexpr auto src_string_4 = Strings<TChar>::test1;

	constexpr auto sv_1 = bstone::BasicZStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicZStringView<TChar>{src_string_2};
	constexpr auto sv_3 = bstone::BasicZStringView<TChar>{src_string_3};
	constexpr auto sv_4 = bstone::BasicZStringView<TChar>{src_string_4};

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

void test_7mzyiu7wx0aso1id_char()
{
	test_7mzyiu7wx0aso1id<char>();
}

void test_7mzyiu7wx0aso1id_wchar_t()
{
	test_7mzyiu7wx0aso1id<wchar_t>();
}

// ==========================================================================

// constexpr bool contains(Char) const noexcept
template<typename TChar>
void test_sdppdi35caq9k37l()
{
	constexpr auto src_string_1 = Strings<TChar>::empty;
	constexpr auto src_string_2 = Strings<TChar>::aa_test_bb;

	constexpr auto sv_1 = bstone::BasicZStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicZStringView<TChar>{src_string_2};

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

void test_sdppdi35caq9k37l_char()
{
	test_sdppdi35caq9k37l<char>();
}

void test_sdppdi35caq9k37l_wchar_t()
{
	test_sdppdi35caq9k37l<wchar_t>();
}

// ==========================================================================

// constexpr bool contains(const Char*) const noexcept
template<typename TChar>
void test_ukmoc06ui63bk34u()
{
	constexpr auto src_string_1 = Strings<TChar>::empty;
	constexpr auto src_string_2 = Strings<TChar>::aa_test_bb;
	constexpr auto src_string_3 = Strings<TChar>::test;
	constexpr auto src_string_4 = Strings<TChar>::test1;

	constexpr auto sv_1 = bstone::BasicZStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicZStringView<TChar>{src_string_2};
	constexpr auto sv_3 = bstone::BasicZStringView<TChar>{src_string_3};

	constexpr auto result_1 = sv_1.contains(src_string_1);
	constexpr auto is_valid_1 = result_1;

	constexpr auto result_2 = sv_1.contains(src_string_2);
	constexpr auto is_valid_2 = !result_2;

	constexpr auto result_3 = sv_2.contains(src_string_1);
	constexpr auto is_valid_3 = result_3;

	constexpr auto result_4 = sv_2.contains(src_string_3);
	constexpr auto is_valid_4 = result_4;

	constexpr auto result_5 = sv_2.contains(src_string_4);
	constexpr auto is_valid_5 = !result_5;

	constexpr auto result_6 = sv_3.contains(src_string_4);
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

void test_ukmoc06ui63bk34u_char()
{
	test_ukmoc06ui63bk34u<char>();
}

void test_ukmoc06ui63bk34u_wchar_t()
{
	test_ukmoc06ui63bk34u<wchar_t>();
}

// ==========================================================================

// constexpr std::intptr_t index_of(BasicZStringView) const noexcept
template<typename TChar>
void test_apdxbjnq0ojjcz2v()
{
	constexpr auto src_string_1 = Strings<TChar>::empty;
	constexpr auto src_string_2 = Strings<TChar>::aa_test_bb;
	constexpr auto src_string_3 = Strings<TChar>::test;
	constexpr auto src_string_4 = Strings<TChar>::test1;

	constexpr auto sv_1 = bstone::BasicZStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicZStringView<TChar>{src_string_2};
	constexpr auto sv_3 = bstone::BasicZStringView<TChar>{src_string_3};
	constexpr auto sv_4 = bstone::BasicZStringView<TChar>{src_string_4};

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

void test_apdxbjnq0ojjcz2v_char()
{
	test_apdxbjnq0ojjcz2v<char>();
}

void test_apdxbjnq0ojjcz2v_wchar_t()
{
	test_apdxbjnq0ojjcz2v<wchar_t>();
}

// ==========================================================================

// constexpr std::intptr_t index_of(Char) const noexcept
template<typename TChar>
void test_2x4un39xr0d7h702()
{
	constexpr auto src_string_1 = Strings<TChar>::empty;
	constexpr auto src_string_2 = Strings<TChar>::aa_test_bb;

	constexpr auto sv_1 = bstone::BasicZStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicZStringView<TChar>{src_string_2};

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

void test_2x4un39xr0d7h702_char()
{
	test_2x4un39xr0d7h702<char>();
}

void test_2x4un39xr0d7h702_wchar_t()
{
	test_2x4un39xr0d7h702<wchar_t>();
}

// ==========================================================================

// constexpr std::intptr_t last_index_of(BasicZStringView) const noexcept
template<typename TChar>
void test_dov3mthnw6aw6opn()
{
	constexpr auto src_string_1 = Strings<TChar>::empty;
	constexpr auto src_string_2 = Strings<TChar>::aa_test_test_bb;
	constexpr auto src_string_3 = Strings<TChar>::test;
	constexpr auto src_string_4 = Strings<TChar>::test1;

	constexpr auto sv_1 = bstone::BasicZStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicZStringView<TChar>{src_string_2};
	constexpr auto sv_3 = bstone::BasicZStringView<TChar>{src_string_3};
	constexpr auto sv_4 = bstone::BasicZStringView<TChar>{src_string_4};

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

void test_dov3mthnw6aw6opn_char()
{
	test_dov3mthnw6aw6opn<char>();
}

void test_dov3mthnw6aw6opn_wchar_t()
{
	test_dov3mthnw6aw6opn<wchar_t>();
}

// ==========================================================================

// constexpr std::intptr_t last_index_of(Char) const noexcept
template<typename TChar>
void test_3mbog95iu7u7k1y3()
{
	constexpr auto src_string_1 = Strings<TChar>::empty;
	constexpr auto src_string_2 = Strings<TChar>::aa_test_bb;

	constexpr auto sv_1 = bstone::BasicZStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicZStringView<TChar>{src_string_2};

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

void test_3mbog95iu7u7k1y3_char()
{
	test_3mbog95iu7u7k1y3<char>();
}

void test_3mbog95iu7u7k1y3_wchar_t()
{
	test_3mbog95iu7u7k1y3<wchar_t>();
}

// ==========================================================================

// constexpr std::intptr_t index_of_any(BasicZStringView) const noexcept
template<typename TChar>
void test_yzqq6p4izbe9xl8k()
{
	constexpr auto src_string_1 = Strings<TChar>::aa_test_bb;
	constexpr auto src_string_2 = Strings<TChar>::t;
	constexpr auto src_string_3 = Strings<TChar>::yes;

	constexpr auto sv_1 = bstone::BasicZStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicZStringView<TChar>{src_string_2};
	constexpr auto sv_3 = bstone::BasicZStringView<TChar>{src_string_3};

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

void test_yzqq6p4izbe9xl8k_char()
{
	test_yzqq6p4izbe9xl8k<char>();
}

void test_yzqq6p4izbe9xl8k_wchar_t()
{
	test_yzqq6p4izbe9xl8k<wchar_t>();
}

// ==========================================================================

// constexpr std::intptr_t last_index_of_any(BasicZStringView) const noexcept
template<typename TChar>
void test_0ae0tsgq4l58s0vi()
{
	constexpr auto src_string_1 = Strings<TChar>::aa_test_test_bb;
	constexpr auto src_string_2 = Strings<TChar>::t;
	constexpr auto src_string_3 = Strings<TChar>::yes;

	constexpr auto sv_1 = bstone::BasicZStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicZStringView<TChar>{src_string_2};
	constexpr auto sv_3 = bstone::BasicZStringView<TChar>{src_string_3};

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

void test_0ae0tsgq4l58s0vi_char()
{
	test_0ae0tsgq4l58s0vi<char>();
}

void test_0ae0tsgq4l58s0vi_wchar_t()
{
	test_0ae0tsgq4l58s0vi<wchar_t>();
}

// ==========================================================================

// constexpr bool operator==(BasicZStringView<TChar>, BasicZStringView<TChar>) noexcept
template<typename TChar>
void test_x82tireoxbqn2frf()
{
	constexpr auto src_string_1 = Strings<TChar>::test;
	constexpr auto src_string_2 = Strings<TChar>::test1;
	constexpr auto src_string_3 = Strings<TChar>::u;

	constexpr auto sv_1 = bstone::BasicZStringView<TChar>{};
	constexpr auto sv_2 = bstone::BasicZStringView<TChar>{src_string_1};
	constexpr auto sv_3 = bstone::BasicZStringView<TChar>{src_string_2};
	constexpr auto sv_4 = bstone::BasicZStringView<TChar>{src_string_3};

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

void test_x82tireoxbqn2frf_char()
{
	test_x82tireoxbqn2frf<char>();
}

void test_x82tireoxbqn2frf_wchar_t()
{
	test_x82tireoxbqn2frf<wchar_t>();
}

// ==========================================================================

// constexpr bool operator!=(BasicZStringView<TChar>, BasicZStringView<TChar>) noexcept
template<typename TChar>
void test_s74w9fg2aeyurcnc()
{
	constexpr auto src_string_1 = Strings<TChar>::test;
	constexpr auto src_string_2 = Strings<TChar>::test1;
	constexpr auto src_string_3 = Strings<TChar>::u;

	constexpr auto sv_1 = bstone::BasicZStringView<TChar>{};
	constexpr auto sv_2 = bstone::BasicZStringView<TChar>{src_string_1};
	constexpr auto sv_3 = bstone::BasicZStringView<TChar>{src_string_2};
	constexpr auto sv_4 = bstone::BasicZStringView<TChar>{src_string_3};

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

void test_s74w9fg2aeyurcnc_char()
{
	test_s74w9fg2aeyurcnc<char>();
}

void test_s74w9fg2aeyurcnc_wchar_t()
{
	test_s74w9fg2aeyurcnc<wchar_t>();
}

// ==========================================================================

// constexpr bool operator<(BasicZStringView<TChar>, BasicZStringView<TChar>) noexcept
template<typename TChar>
void test_fla4vuh3r5ch8qj4()
{
	constexpr auto src_string_1 = Strings<TChar>::empty;
	constexpr auto src_string_2 = Strings<TChar>::test;
	constexpr auto src_string_3 = Strings<TChar>::test1;
	constexpr auto src_string_4 = Strings<TChar>::u;

	constexpr auto sv_1 = bstone::BasicZStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicZStringView<TChar>{src_string_2};
	constexpr auto sv_3 = bstone::BasicZStringView<TChar>{src_string_3};
	constexpr auto sv_4 = bstone::BasicZStringView<TChar>{src_string_4};

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

void test_fla4vuh3r5ch8qj4_char()
{
	test_fla4vuh3r5ch8qj4<char>();
}

void test_fla4vuh3r5ch8qj4_wchar_t()
{
	test_fla4vuh3r5ch8qj4<wchar_t>();
}

// ==========================================================================

// constexpr bool operator<=(BasicZStringView<TChar>, BasicZStringView<TChar>) noexcept
template<typename TChar>
void test_vihlb1ailywvjhe6()
{
	constexpr auto src_string_1 = Strings<TChar>::empty;
	constexpr auto src_string_2 = Strings<TChar>::test;
	constexpr auto src_string_3 = Strings<TChar>::test1;
	constexpr auto src_string_4 = Strings<TChar>::u;

	constexpr auto sv_1 = bstone::BasicZStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicZStringView<TChar>{src_string_2};
	constexpr auto sv_3 = bstone::BasicZStringView<TChar>{src_string_3};
	constexpr auto sv_4 = bstone::BasicZStringView<TChar>{src_string_4};

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

void test_vihlb1ailywvjhe6_char()
{
	test_vihlb1ailywvjhe6<char>();
}

void test_vihlb1ailywvjhe6_wchar_t()
{
	test_vihlb1ailywvjhe6<wchar_t>();
}

// ==========================================================================

// constexpr bool operator>(BasicZStringView<TChar>, BasicZStringView<TChar>) noexcept
template<typename TChar>
void test_ucg0gn05w32osmh3()
{
	constexpr auto src_string_1 = Strings<TChar>::empty;
	constexpr auto src_string_2 = Strings<TChar>::test;
	constexpr auto src_string_3 = Strings<TChar>::test1;
	constexpr auto src_string_4 = Strings<TChar>::u;

	constexpr auto sv_1 = bstone::BasicZStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicZStringView<TChar>{src_string_2};
	constexpr auto sv_3 = bstone::BasicZStringView<TChar>{src_string_3};
	constexpr auto sv_4 = bstone::BasicZStringView<TChar>{src_string_4};

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

void test_ucg0gn05w32osmh3_char()
{
	test_ucg0gn05w32osmh3<char>();
}

void test_ucg0gn05w32osmh3_wchar_t()
{
	test_ucg0gn05w32osmh3<wchar_t>();
}

// ==========================================================================

// constexpr bool operator>=(BasicZStringView<TChar>, BasicZStringView<TChar>) noexcept
template<typename TChar>
void test_dvsp6mcob65miku3()
{
	constexpr auto src_string_1 = Strings<TChar>::empty;
	constexpr auto src_string_2 = Strings<TChar>::test;
	constexpr auto src_string_3 = Strings<TChar>::test1;
	constexpr auto src_string_4 = Strings<TChar>::u;

	constexpr auto sv_1 = bstone::BasicZStringView<TChar>{src_string_1};
	constexpr auto sv_2 = bstone::BasicZStringView<TChar>{src_string_2};
	constexpr auto sv_3 = bstone::BasicZStringView<TChar>{src_string_3};
	constexpr auto sv_4 = bstone::BasicZStringView<TChar>{src_string_4};

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

void test_dvsp6mcob65miku3_char()
{
	test_dvsp6mcob65miku3<char>();
}

void test_dvsp6mcob65miku3_wchar_t()
{
	test_dvsp6mcob65miku3<wchar_t>();
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
		register_string_view_cast_operator();
		register_subscript_operator();
		register_get_front();
		register_get_back();
		register_get_data();
		register_has_data();
		register_get_size();
		register_is_empty();
		register_remove_prefix();
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
		tester.register_test("BasicZStringView#2i9bnq9ufcgnbrhv", test_aw7cpphusqlv650c_char);
		tester.register_test("BasicZStringView#ywz189dg7dq99fge", test_aw7cpphusqlv650c_wchar_t);

		tester.register_test("BasicZStringView#2sghz0trszvwz1l5", test_hv98okrz5vvpvvw7_char);
		tester.register_test("BasicZStringView#rq27u0ez7mkir81o", test_hv98okrz5vvpvvw7_wchar_t);

		tester.register_test("BasicZStringView#r0ykb5kw9crzwhx2", test_5lj766w71688do43_char);
		tester.register_test("BasicZStringView#m7xpc804kq98z6gj", test_5lj766w71688do43_wchar_t);

		tester.register_test("BasicZStringView#n5yrmsvlanicbycu", test_0i4ouufm6p869wgh_char);
		tester.register_test("BasicZStringView#ul7un43yoaa9p6pz", test_0i4ouufm6p869wgh_wchar_t);
	}

	void register_assignment_operator()
	{
		tester.register_test("BasicZStringView#mve89m2k8ey3diny", test_9ai1dbgbb81vvjgo_char);
		tester.register_test("BasicZStringView#uaeg0utswaxgd9n9", test_9ai1dbgbb81vvjgo_wchar_t);
	}

	void register_begin()
	{
		tester.register_test("BasicZStringView#1fk1lwwstz0ae9sx", test_lm5vhcm5l3rwxvc5_char);
		tester.register_test("BasicZStringView#jti42p5rd4qexbg2", test_lm5vhcm5l3rwxvc5_wchar_t);
	}

	void register_cbegin()
	{
		tester.register_test("BasicZStringView#jk7d3r01musdev11", test_glzhw581vwr2qiqy_char);
		tester.register_test("BasicZStringView#zu1jskyrikyyay8t", test_glzhw581vwr2qiqy_wchar_t);
	}

	void register_end()
	{
		tester.register_test("BasicZStringView#2d5ssyviszimqoiu", test_pcx3zznvvy6bghsg_char);
		tester.register_test("BasicZStringView#iuyxrhb5zymbsjm8", test_pcx3zznvvy6bghsg_wchar_t);
	}

	void register_cend()
	{
		tester.register_test("BasicZStringView#sbr9ccooaahc5du6", test_xyflxt2c0vgyagdm_char);
		tester.register_test("BasicZStringView#jucrx20bbcz0e0cr", test_xyflxt2c0vgyagdm_wchar_t);
	}

	void register_string_view_cast_operator()
	{
		tester.register_test("BasicZStringView#btpph1aijy08ymdy", test_qshhzzwkuhxddlh0_char);
		tester.register_test("BasicZStringView#vvpxpdhqtim9o4xl", test_qshhzzwkuhxddlh0_wchar_t);
	}

	void register_subscript_operator()
	{
		tester.register_test("BasicZStringView#2fo3pr3t2ewfb4fj", test_m9w6fhjxli1gnckm_char);
		tester.register_test("BasicZStringView#8hq6laiax9sm6bbc", test_m9w6fhjxli1gnckm_wchar_t);
	}

	void register_get_front()
	{
		tester.register_test("BasicZStringView#opiqtl957fobopv1", test_h2slm58x68z8wsyw_char);
		tester.register_test("BasicZStringView#bb77miitm5x45pxv", test_h2slm58x68z8wsyw_wchar_t);
	}

	void register_get_back()
	{
		tester.register_test("BasicZStringView#miz8lcx0u09aovn8", test_98ss8s7e5bp9hgei_char);
		tester.register_test("BasicZStringView#hsrivrcm8bthjd7d", test_98ss8s7e5bp9hgei_wchar_t);
	}

	void register_get_data()
	{
		tester.register_test("BasicZStringView#vo1ymdif1q6mv1ao", test_9ze7j48hmqwsyciy_char);
		tester.register_test("BasicZStringView#eh3nwfy6ig14gkfx", test_9ze7j48hmqwsyciy_wchar_t);
	}

	void register_has_data()
	{
		tester.register_test("BasicZStringView#fgjlzamezg8l9k2x", test_vhas56q51cm9v963_char);
		tester.register_test("BasicZStringView#57p3o0670sni0xzb", test_vhas56q51cm9v963_wchar_t);
	}

	void register_get_size()
	{
		tester.register_test("BasicZStringView#blqmwkdvnghme53r", test_k59wb2kzkcpsdwlw_char);
		tester.register_test("BasicZStringView#wots70n7uekqtcp0", test_k59wb2kzkcpsdwlw_wchar_t);
	}

	void register_is_empty()
	{
		tester.register_test("BasicZStringView#dvm0bmcdpf5tsqlv", test_4lag028hueofnxoz_char);
		tester.register_test("BasicZStringView#shu5825nwvsvxp2s", test_4lag028hueofnxoz_wchar_t);
	}

	void register_remove_prefix()
	{
		tester.register_test("BasicZStringView#xniuojc6kcb4220y", test_ae4q6ahbkuzv0a3e_char);
		tester.register_test("BasicZStringView#8rv41c6uik0k1rc2", test_ae4q6ahbkuzv0a3e_wchar_t);
	}

	void register_swap()
	{
		tester.register_test("BasicZStringView#ls4y72zi8wbkkrnz", test_vddjcmr53e22opf2_char);
		tester.register_test("BasicZStringView#53pfg0v4gc1ac1ag", test_vddjcmr53e22opf2_wchar_t);
	}

	void register_get_subview()
	{
		tester.register_test("BasicZStringView#lf9hl5niebfxyyty", test_gb86ia8b5ryz5aig_char);
		tester.register_test("BasicZStringView#f5b85k4dtzh58pgd", test_gb86ia8b5ryz5aig_wchar_t);
	}

	void register_compare()
	{
		tester.register_test("BasicZStringView#ac5vvb99yfsu1nbe", test_obxlwkr9o9w5mj9j_char);
		tester.register_test("BasicZStringView#xwdp8ep6i08muqla", test_obxlwkr9o9w5mj9j_wchar_t);
	}

	void register_starts_with()
	{
		tester.register_test("BasicZStringView#gspw0pxnyg0xs591", test_ygakn1ivtn0wqto0_char);
		tester.register_test("BasicZStringView#acdyqjvmrr6obexz", test_ygakn1ivtn0wqto0_wchar_t);

		tester.register_test("BasicZStringView#yizpeqi89pggw0jl", test_h4s5ytj68szcubu2_char);
		tester.register_test("BasicZStringView#3nhh78gqp702mqdj", test_h4s5ytj68szcubu2_wchar_t);

		tester.register_test("BasicZStringView#yac0j8whcsit5lvz", test_29j8z5jyphz5zs4m_char);
		tester.register_test("BasicZStringView#hqmfpky2kxbhxhll", test_29j8z5jyphz5zs4m_wchar_t);
	}

	void register_ends_with()
	{
		tester.register_test("BasicZStringView#1e5fvplz84ehf73d", test_kwdv4fkv77cuespi_char);
		tester.register_test("BasicZStringView#vhazemagq8293h6t", test_kwdv4fkv77cuespi_wchar_t);

		tester.register_test("BasicZStringView#ooghc87rvu2xpadw", test_cmzg12qjl5frj8e4_char);
		tester.register_test("BasicZStringView#25gtol70c63eb47g", test_cmzg12qjl5frj8e4_wchar_t);

		tester.register_test("BasicZStringView#i1x45tfkb7pqov4o", test_ymrzkvqi461cbf55_char);
		tester.register_test("BasicZStringView#bctmdpkeipmsdma8", test_ymrzkvqi461cbf55_wchar_t);
	}

	void register_contains()
	{
		tester.register_test("BasicZStringView#xcw7sg988evjuhos", test_7mzyiu7wx0aso1id_char);
		tester.register_test("BasicZStringView#urbgh1ikku5u0qcd", test_7mzyiu7wx0aso1id_wchar_t);

		tester.register_test("BasicZStringView#8tv3biop8llojuu2", test_sdppdi35caq9k37l_char);
		tester.register_test("BasicZStringView#643fks2h3hc6qrl2", test_sdppdi35caq9k37l_wchar_t);

		tester.register_test("BasicZStringView#asmw7a4xs011d60e", test_ukmoc06ui63bk34u_char);
		tester.register_test("BasicZStringView#8qtwu5omenc69qmi", test_ukmoc06ui63bk34u_wchar_t);
	}

	void register_index_of()
	{
		tester.register_test("BasicZStringView#5wei9ktp3geuzb0g", test_apdxbjnq0ojjcz2v_char);
		tester.register_test("BasicZStringView#hllcc5zo0lfae306", test_apdxbjnq0ojjcz2v_wchar_t);

		tester.register_test("BasicZStringView#ao8gxhnofyc27r1q", test_2x4un39xr0d7h702_char);
		tester.register_test("BasicZStringView#wrgfx9yowckptzyp", test_2x4un39xr0d7h702_wchar_t);
	}

	void register_last_index_of()
	{
		tester.register_test("BasicZStringView#qnubk2vva47dgmni", test_dov3mthnw6aw6opn_char);
		tester.register_test("BasicZStringView#cr3ljex3o5nfp679", test_dov3mthnw6aw6opn_wchar_t);

		tester.register_test("BasicZStringView#ci9xtvv5q018jd2x", test_3mbog95iu7u7k1y3_char);
		tester.register_test("BasicZStringView#cel0002leuvvxn2x", test_3mbog95iu7u7k1y3_wchar_t);
	}

	void register_index_of_any()
	{
		tester.register_test("BasicZStringView#ptfmhzueloulea4r", test_yzqq6p4izbe9xl8k_char);
		tester.register_test("BasicZStringView#qv2wkp2ndlv58ode", test_yzqq6p4izbe9xl8k_wchar_t);
	}

	void register_last_index_of_any()
	{
		tester.register_test("BasicZStringView#2qhoehgcp2ilajru", test_0ae0tsgq4l58s0vi_char);
		tester.register_test("BasicZStringView#ro0zosw12h2kfirx", test_0ae0tsgq4l58s0vi_wchar_t);
	}

	void register_operator_equal_to()
	{
		tester.register_test("BasicZStringView#pnybbalkhm06bkhn", test_x82tireoxbqn2frf_char);
		tester.register_test("BasicZStringView#upzrllkjx4og37z2", test_x82tireoxbqn2frf_wchar_t);
	}

	void register_operator_not_equal_to()
	{
		tester.register_test("BasicZStringView#iy980p8till6ghws", test_s74w9fg2aeyurcnc_char);
		tester.register_test("BasicZStringView#e6430yrc9km1llk6", test_s74w9fg2aeyurcnc_wchar_t);
	}

	void register_operator_less_than()
	{
		tester.register_test("BasicZStringView#oy6hlzohhoro7iny", test_fla4vuh3r5ch8qj4_char);
		tester.register_test("BasicZStringView#zgdv08wuln0rbhqv", test_fla4vuh3r5ch8qj4_wchar_t);
	}

	void register_operator_less_than_or_equal_to()
	{
		tester.register_test("BasicZStringView#l9d0zksc97a4mh36", test_vihlb1ailywvjhe6_char);
		tester.register_test("BasicZStringView#jw2mjtw1kc1vnd2b", test_vihlb1ailywvjhe6_wchar_t);
	}

	void register_operator_greater_than()
	{
		tester.register_test("BasicZStringView#nlo4d2gnmlece7hz", test_ucg0gn05w32osmh3_char);
		tester.register_test("BasicZStringView#6dqwump2zm9qxzne", test_ucg0gn05w32osmh3_wchar_t);
	}

	void register_operator_greater_than_or_equal_to()
	{
		tester.register_test("BasicZStringView#pr65wsi8xhq2yzlv", test_dvsp6mcob65miku3_char);
		tester.register_test("BasicZStringView#exx1i5uxx9v70yno", test_dvsp6mcob65miku3_wchar_t);
	}
};

auto registrator = Registrator{};

} // namespace
