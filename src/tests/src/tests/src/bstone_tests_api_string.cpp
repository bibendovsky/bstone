#include <algorithm>
#include <memory>
#include <string>

#include "bstone_tester.h"
#include "bstone_api_string.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

constexpr auto memory_fill_value = 'H';

class MemoryResource final : public bstone::MemoryResource
{
public:
	MemoryResource() = default;
	~MemoryResource() override = default;

private:
	void* do_allocate(std::intptr_t size) override
	{
		const auto ptr = bstone::get_default_memory_resource().allocate(size);
		std::fill_n(static_cast<char*>(ptr), size, memory_fill_value);
		return ptr;
	}

	void do_deallocate(void* ptr) noexcept override
	{
		bstone::get_default_memory_resource().deallocate(ptr);
	}
};

MemoryResource& get_memory_resource()
{
	static auto memory_resource = MemoryResource{};
	return memory_resource;
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

// ==========================================================================

// ApiString(MemoryResource&)
template<typename TChar>
void test_h4wpvm5gq2u3g8j8()
{
	using ApiString = bstone::ApiString<TChar>;
	const auto string = ApiString{get_memory_resource()};

	const auto is_valid_1 = string.get_size() == 0;
	const auto is_valid_2 = string.get_capacity() == 0;
	const auto is_valid_3 = string.get_data() == nullptr;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_h4wpvm5gq2u3g8j8_char()
{
	test_h4wpvm5gq2u3g8j8<char>();
}

void test_h4wpvm5gq2u3g8j8_wchar_t()
{
	test_h4wpvm5gq2u3g8j8<wchar_t>();
}

// ==========================================================================

// ApiString(std::intptr_t, MemoryResource&)
template<typename TChar>
void test_57l7o3z8qsclpief()
{
	using ApiString = bstone::ApiString<TChar>;

	constexpr auto ref_capacity = 2;
	const auto string = ApiString{ref_capacity, get_memory_resource()};

	const auto is_valid_1 = string.get_size() == 0;
	const auto is_valid_2 = string.get_capacity() == ref_capacity;

	const auto chars = string.get_data();
	const auto is_valid_3 = chars[0] == '\0';

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_57l7o3z8qsclpief_char()
{
	test_57l7o3z8qsclpief<char>();
}

void test_57l7o3z8qsclpief_wchar_t()
{
	test_57l7o3z8qsclpief<wchar_t>();
}

// ==========================================================================

// ApiString(const Char*, MemoryResource&)
template<typename TChar>
void test_ofrffqdz3lb24kcn()
{
	using ApiString = bstone::ApiString<TChar>;

	const auto src_string = make_string<TChar>("str");
	const auto string = ApiString{src_string.c_str(), get_memory_resource()};
	const auto chars = string.get_data();

	const auto is_valid_1 = string.get_size() == static_cast<std::intptr_t>(src_string.size());
	const auto is_valid_2 = string.get_capacity() >= static_cast<std::intptr_t>(src_string.size());

	const auto is_valid_3 =
		chars[0] == 's' &&
		chars[1] == 't' &&
		chars[2] == 'r' &&
		chars[3] == '\0' &&
		true;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

void test_ofrffqdz3lb24kcn_char()
{
	test_ofrffqdz3lb24kcn<char>();
}

void test_ofrffqdz3lb24kcn_wchar_t()
{
	test_ofrffqdz3lb24kcn<wchar_t>();
}

// ==========================================================================

// MemoryResource& get_memory_resource() const noexcept
template<typename TChar>
void test_8nywu68tfpfx8vre()
{
	using ApiString = bstone::ApiString<TChar>;

	const auto string = ApiString{get_memory_resource()};
	const auto& memory_resource = string.get_memory_resource();

	const auto is_valid_1 = &memory_resource == &get_memory_resource();

	const auto is_valid =
		is_valid_1 &&
		true;

	tester.check(is_valid);
}

void test_8nywu68tfpfx8vre_char()
{
	test_8nywu68tfpfx8vre<char>();
}

void test_8nywu68tfpfx8vre_wchar_t()
{
	test_8nywu68tfpfx8vre<wchar_t>();
}

// ==========================================================================

// std::intptr_t get_size() const noexcept
template<typename TChar>
void test_syc2o9c84ybycw6y()
{
	using ApiString = bstone::ApiString<TChar>;

	const auto src_string = make_string<TChar>("str");
	const auto string = ApiString{src_string.c_str(), get_memory_resource()};

	const auto is_valid_1 = string.get_size() == static_cast<std::intptr_t>(src_string.size());

	const auto is_valid =
		is_valid_1 &&
		true;

	tester.check(is_valid);
}

void test_syc2o9c84ybycw6y_char()
{
	test_syc2o9c84ybycw6y<char>();
}

void test_syc2o9c84ybycw6y_wchar_t()
{
	test_syc2o9c84ybycw6y<wchar_t>();
}

// ==========================================================================

// set_size(std::intptr_t)
template<typename TChar>
void test_ns6qwbtdos580b76()
{
	using ApiString = bstone::ApiString<TChar>;

	const auto src_string = make_string<TChar>("str");
	auto string = ApiString{src_string.c_str(), get_memory_resource()};
	const auto chars = string.get_data();

	const auto is_valid_1 =
		chars[0] == 's' &&
		chars[1] == 't' &&
		chars[2] == 'r' &&
		chars[3] == '\0' &&
		true;

	string.set_size(1);

	const auto is_valid_2 =
		chars[0] == 's' &&
		chars[1] == '\0' &&
		chars[2] == 'r' &&
		chars[3] == '\0' &&
		true;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		true;

	tester.check(is_valid);
}

void test_ns6qwbtdos580b76_char()
{
	test_ns6qwbtdos580b76<char>();
}

void test_ns6qwbtdos580b76_wchar_t()
{
	test_ns6qwbtdos580b76<wchar_t>();
}

// ==========================================================================

// std::intptr_t get_capacity() const noexcept
template<typename TChar>
void test_pfvn6xovlxamcubr()
{
	using ApiString = bstone::ApiString<TChar>;

	const auto src_string = make_string<TChar>("str");
	const auto string = ApiString{src_string.c_str(), get_memory_resource()};

	const auto is_valid_1 = string.get_capacity() == static_cast<std::intptr_t>(src_string.size());

	const auto is_valid =
		is_valid_1 &&
		true;

	tester.check(is_valid);
}

void test_pfvn6xovlxamcubr_char()
{
	test_pfvn6xovlxamcubr<char>();
}

void test_pfvn6xovlxamcubr_wchar_t()
{
	test_pfvn6xovlxamcubr<wchar_t>();
}

// ==========================================================================

// std::intptr_t get_capacity() const noexcept
template<typename TChar>
void test_j8k9bqhh1fb5j9wi()
{
	using ApiString = bstone::ApiString<TChar>;

	const auto src_string = make_string<TChar>("str");
	const auto string = ApiString{src_string.c_str(), get_memory_resource()};

	const auto is_valid_1 = string.get_capacity() == static_cast<std::intptr_t>(src_string.size());

	const auto is_valid =
		is_valid_1 &&
		true;

	tester.check(is_valid);
}

void test_j8k9bqhh1fb5j9wi_char()
{
	test_j8k9bqhh1fb5j9wi<char>();
}

void test_j8k9bqhh1fb5j9wi_wchar_t()
{
	test_j8k9bqhh1fb5j9wi<wchar_t>();
}

// ==========================================================================

// const Char* get_data() const noexcept
template<typename TChar>
void test_snmwamd3ytziyg82()
{
	using ApiString = bstone::ApiString<TChar>;

	const auto src_string = make_string<TChar>("str");
	const auto string = ApiString{src_string.c_str(), get_memory_resource()};
	const auto chars = string.get_data();
	const auto size = string.get_size();

	const auto is_valid_1 =
		size == 3 &&
		chars[0] == 's' &&
		chars[1] == 't' &&
		chars[2] == 'r' &&
		chars[3] == '\0' &&
		true;

	const auto is_valid =
		is_valid_1 &&
		true;

	tester.check(is_valid);
}

void test_snmwamd3ytziyg82_char()
{
	test_snmwamd3ytziyg82<char>();
}

void test_snmwamd3ytziyg82_wchar_t()
{
	test_snmwamd3ytziyg82<wchar_t>();
}

// ==========================================================================

// Char* get_data() noexcept
template<typename TChar>
void test_bwwm6qjhok2h05oh()
{
	using ApiString = bstone::ApiString<TChar>;

	const auto src_string = make_string<TChar>("str");
	auto string = ApiString{src_string.c_str(), get_memory_resource()};
	const auto chars = string.get_data();
	const auto size = string.get_size();

	const auto is_valid_1 =
		size == 3 &&
		chars[0] == 's' &&
		chars[1] == 't' &&
		chars[2] == 'r' &&
		chars[3] == '\0' &&
		true;

	const auto is_valid =
		is_valid_1 &&
		true;

	tester.check(is_valid);
}

void test_bwwm6qjhok2h05oh_char()
{
	test_bwwm6qjhok2h05oh<char>();
}

void test_bwwm6qjhok2h05oh_wchar_t()
{
	test_bwwm6qjhok2h05oh<wchar_t>();
}

// void swap(ApiString&) noexcept
template<typename TChar>
void test_iidkx6djksk66vw5()
{
	using ApiString = bstone::ApiString<TChar>;

	const auto src_string = make_string<TChar>("str");
	auto string_1 = ApiString{src_string.c_str(), get_memory_resource()};
	const auto chars_1 = string_1.get_data();
	const auto size_1 = string_1.get_size();

	const auto is_valid_1 =
		size_1 == 3 &&
		chars_1[0] == 's' &&
		chars_1[1] == 't' &&
		chars_1[2] == 'r' &&
		chars_1[3] == '\0' &&
		true;

	auto string_2 = ApiString{get_memory_resource()};
	string_2.swap(string_1);
	const auto chars_2 = string_2.get_data();
	const auto size_2 = string_2.get_size();

	const auto is_valid_2 =
		size_2 == 3 &&
		chars_2[0] == 's' &&
		chars_2[1] == 't' &&
		chars_2[2] == 'r' &&
		chars_2[3] == '\0' &&
		true;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		true;

	tester.check(is_valid);
}

void test_iidkx6djksk66vw5_char()
{
	test_iidkx6djksk66vw5<char>();
}

void test_iidkx6djksk66vw5_wchar_t()
{
	test_iidkx6djksk66vw5<wchar_t>();
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_ctor();
		register_get_memory_resource();
		register_get_size();
		register_set_size();
		register_get_capacity();
		register_get_data();
		register_swap();
	}

private:
	void register_ctor()
	{
		tester.register_test("ApiString#cr8zcr40cwntl25e", test_h4wpvm5gq2u3g8j8_char);
		tester.register_test("ApiString#8w1zxd472gd0xaj2", test_h4wpvm5gq2u3g8j8_wchar_t);

		tester.register_test("ApiString#i323ukq6swnqr0d8", test_57l7o3z8qsclpief_char);
		tester.register_test("ApiString#4kzz68ku8nk8gtmh", test_57l7o3z8qsclpief_wchar_t);

		tester.register_test("ApiString#po56tbbt90v60ib4", test_ofrffqdz3lb24kcn_char);
		tester.register_test("ApiString#snzcjse94ylslmrv", test_ofrffqdz3lb24kcn_wchar_t);
	}

	void register_get_memory_resource()
	{
		tester.register_test("ApiString#okt6s4be0vdjd8bs", test_8nywu68tfpfx8vre_char);
		tester.register_test("ApiString#uprta4ggy5apkv9q", test_8nywu68tfpfx8vre_wchar_t);
	}

	void register_get_size()
	{
		tester.register_test("ApiString#3s7hted3koud7bpr", test_syc2o9c84ybycw6y_char);
		tester.register_test("ApiString#7s55g7vzngq4c1bx", test_syc2o9c84ybycw6y_wchar_t);
	}

	void register_set_size()
	{
		tester.register_test("ApiString#lmy9yeat3tggwz7s", test_ns6qwbtdos580b76_char);
		tester.register_test("ApiString#5i4mjxm0yizjblqj", test_ns6qwbtdos580b76_wchar_t);
	}

	void register_get_capacity()
	{
		tester.register_test("ApiString#65nyw04bxnwwmmhq", test_pfvn6xovlxamcubr_char);
		tester.register_test("ApiString#r3xtmsu93iqj8jbr", test_pfvn6xovlxamcubr_wchar_t);

		tester.register_test("ApiString#kedg30m6x1i2ztdb", test_j8k9bqhh1fb5j9wi_char);
		tester.register_test("ApiString#amue7po78eba8woy", test_j8k9bqhh1fb5j9wi_wchar_t);
	}

	void register_get_data()
	{
		tester.register_test("ApiString#fzt44vowcvo0l0yp", test_snmwamd3ytziyg82_char);
		tester.register_test("ApiString#2933f0saqjlt8apv", test_snmwamd3ytziyg82_wchar_t);

		tester.register_test("ApiString#9i1l1wsvd0swa3m7", test_bwwm6qjhok2h05oh_char);
		tester.register_test("ApiString#o1aqlugdqjbzx0s5", test_bwwm6qjhok2h05oh_wchar_t);
	}

	void register_swap()
	{
		tester.register_test("ApiString#5lbo05lppvfovdzs", test_iidkx6djksk66vw5_char);
		tester.register_test("ApiString#zkcuby4c2m6j4uz8", test_iidkx6djksk66vw5_wchar_t);
	}
};

auto registrator = Registrator{};

} // namespace
