#include <algorithm>
#include <cstdint>
#include <vector>

#include "bstone_tester.h"
#include "jm_lzh.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

using Bytes = std::vector<std::uint8_t>;

Bytes compress(const Bytes& decompressed)
{
	auto compressed = Bytes{};
	compressed.resize((4 * decompressed.size()) + 1024);
	const auto compressed_size = LZH_Compress(
		decompressed.data(),
		compressed.data(),
		static_cast<int>(decompressed.size()));
	compressed.resize(static_cast<std::size_t>(compressed_size));
	return compressed;
}

// ==========================================================================

// int LZH_Decompress(const std::uint8_t*, std::uint8_t*, int, int)
// Round trip.
void test_kkgkq1zjcmmzcgc5()
{
	auto decompressed = Bytes{};
	decompressed.resize(1000);

	for (auto i = std::size_t{}; i < decompressed.size(); ++i)
	{
		decompressed[i] = static_cast<std::uint8_t>((i / 10) + (i % 7));
	}

	const auto compressed = compress(decompressed);

	auto result = Bytes{};
	result.resize(decompressed.size());

	const auto result_size = LZH_Decompress(
		compressed.data(),
		result.data(),
		static_cast<int>(result.size()),
		static_cast<int>(compressed.size()));

	tester.check(
		result_size == static_cast<int>(decompressed.size()) &&
		result == decompressed);
}

// int LZH_Decompress(const std::uint8_t*, std::uint8_t*, int, int)
// Writes no more than the requested number of bytes.
void test_p4rzvt4x38fmn9pe()
{
	constexpr auto guard_size = std::size_t{64};
	constexpr auto guard_value = std::uint8_t{0xCD};
	constexpr auto data_value = std::uint8_t{'A'};

	// A long run of the same byte compresses into maximum length matches, so
	// most of the requested sizes below end up in the middle of a match.
	auto decompressed = Bytes{};
	decompressed.resize(256, data_value);

	const auto compressed = compress(decompressed);

	auto is_succeed = true;
	auto result = Bytes{};

	for (auto size = std::size_t{1}; size <= decompressed.size(); ++size)
	{
		result.clear();
		result.resize(size + guard_size, guard_value);

		const auto result_size = LZH_Decompress(
			compressed.data(),
			result.data(),
			static_cast<int>(size),
			static_cast<int>(compressed.size()));

		is_succeed &= result_size == static_cast<int>(size);
		is_succeed &= std::all_of(
			result.cbegin(),
			result.cbegin() + static_cast<std::ptrdiff_t>(size),
			[](std::uint8_t value) { return value == data_value; });
		is_succeed &= std::all_of(
			result.cbegin() + static_cast<std::ptrdiff_t>(size),
			result.cend(),
			[](std::uint8_t value) { return value == guard_value; });
	}

	tester.check(is_succeed);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_lzh_decompress();
	}

private:
	void register_lzh_decompress()
	{
		tester.register_test("LZH_Decompress#kkgkq1zjcmmzcgc5", test_kkgkq1zjcmmzcgc5);
		tester.register_test("LZH_Decompress#p4rzvt4x38fmn9pe", test_p4rzvt4x38fmn9pe);
	}
};

auto registrator = Registrator{};

} // namespace
