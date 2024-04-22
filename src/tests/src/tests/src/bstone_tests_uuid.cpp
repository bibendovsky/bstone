#include "bstone_uuid.h"

#include <algorithm>
#include <iterator>

#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// Uuid() noexcept
void test_qtudbxc80p7n0ro4()
{
	constexpr auto uuid = bstone::Uuid{};
	const auto& uuid_bytes = reinterpret_cast<const std::uint8_t (&)[16]>(uuid);

	const auto is_valid = std::all_of(
		std::cbegin(uuid_bytes),
		std::cend(uuid_bytes),
		[](std::uint8_t byte){ return byte == 0; });

	tester.check(is_valid);
}

// Uuid(const Uuid&) noexcept
void test_ddgndk5w6u9ifqef()
{
	constexpr static const std::uint8_t ref_uuid_bytes[16] =
	{
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
	};

	constexpr auto uuid_1 = bstone::Uuid{ref_uuid_bytes};
	constexpr auto uuid = uuid_1;

	const auto& uuid_bytes = reinterpret_cast<const std::uint8_t (&)[16]>(uuid);

	const auto is_valid = std::equal(
		std::cbegin(uuid_bytes),
		std::cend(uuid_bytes),
		ref_uuid_bytes);

	tester.check(is_valid);
}

// Uuid(const UuidValue&) noexcept
void test_zz4hrxf94ajv16fv()
{
	constexpr static const std::uint8_t ref_uuid_bytes[16] =
	{
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
	};

	constexpr static auto uuid_value = bstone::UuidValue
	{
		std::uint8_t{0x00}, std::uint8_t{0x11}, std::uint8_t{0x22}, std::uint8_t{0x33},
		std::uint8_t{0x44}, std::uint8_t{0x55}, std::uint8_t{0x66}, std::uint8_t{0x77},
		std::uint8_t{0x88}, std::uint8_t{0x99}, std::uint8_t{0xAA}, std::uint8_t{0xBB},
		std::uint8_t{0xCC}, std::uint8_t{0xDD}, std::uint8_t{0xEE}, std::uint8_t{0xFF},
	};

	constexpr auto uuid = bstone::Uuid{uuid_value};

	const auto& uuid_bytes = reinterpret_cast<const std::uint8_t (&)[16]>(uuid);

	const auto is_valid = std::equal(
		std::cbegin(uuid_bytes),
		std::cend(uuid_bytes),
		ref_uuid_bytes);

	tester.check(is_valid);
}

// Uuid(
//     std::uint32_t,
//     std::uint16_t,
//     std::uint16_t,
//     std::uint8_t,
//     std::uint8_t,
//     std::uint8_t,
//     std::uint8_t,
//     std::uint8_t,
//     std::uint8_t,
//     std::uint8_t,
//     std::uint8_t k) noexcept
void test_6mmfx7oqkvu46lmo()
{
	constexpr static const std::uint8_t ref_uuid_bytes[16] =
	{
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
	};

	constexpr auto uuid = bstone::Uuid
	{
		0x00112233U,
		0x4455,
		0x6677,
		0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
	};

	const auto& uuid_bytes = reinterpret_cast<const std::uint8_t (&)[16]>(uuid);

	const auto is_valid = std::equal(
		std::cbegin(uuid_bytes),
		std::cend(uuid_bytes),
		ref_uuid_bytes);

	tester.check(is_valid);
}

// Uuid(
//     std::uint32_t,
//     std::uint16_t,
//     std::uint16_t,
//     const std::uint8_t (&)[8]) noexcept
void test_wpfpbufa35y14jdg()
{
	constexpr static const std::uint8_t ref_uuid_bytes[16] =
	{
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
	};

	constexpr auto uuid = bstone::Uuid
	{
		0x00112233U,
		0x4455,
		0x6677,
		{0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}
	};

	const auto& uuid_bytes = reinterpret_cast<const std::uint8_t (&)[16]>(uuid);

	const auto is_valid = std::equal(
		std::cbegin(uuid_bytes),
		std::cend(uuid_bytes),
		ref_uuid_bytes);

	tester.check(is_valid);
}

// Uuid(const std::uint8_t (&)[16]) noexcept
void test_2itngjmrkmuzbk4n()
{
	constexpr static const std::uint8_t ref_uuid_bytes[16] =
	{
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
	};

	constexpr auto uuid = bstone::Uuid
	{{
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
	}};

	const auto& uuid_bytes = reinterpret_cast<const std::uint8_t (&)[16]>(uuid);

	const auto is_valid = std::equal(
		std::cbegin(uuid_bytes),
		std::cend(uuid_bytes),
		ref_uuid_bytes);

	tester.check(is_valid);
}

// ==========================================================================

// Uuid(BasicStringView<TChar>)
// Without hyphens.
void test_spu3juxzm3inqnqs()
{
	constexpr static const std::uint8_t ref_uuid_bytes[16] =
	{
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
	};

	constexpr auto uuid = bstone::Uuid{bstone::StringView{"00112233445566778899aaBBccDDeEFf"}};

	const auto& uuid_bytes = reinterpret_cast<const std::uint8_t (&)[16]>(uuid);

	const auto is_valid = std::equal(
		std::cbegin(uuid_bytes),
		std::cend(uuid_bytes),
		ref_uuid_bytes);

	tester.check(is_valid);
}

// Uuid(BasicStringView<TChar>)
// Without hyphens.
// Unsupported string length.
void test_1fa65x799xzejdeo()
{
	auto is_failed = false;

	try
	{
		bstone::Uuid{bstone::StringView{"00112233445566778899aaBBccDDeEFf0"}};
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// Uuid(BasicStringView<TChar>)
// Without hyphens.
// Non-hex digit.
void test_9ekg4up92qi2f0q4()
{
	auto is_failed = false;

	try
	{
		bstone::Uuid{bstone::StringView{"00112233445566778899aaBBccDDeEGg"}};
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// Uuid(BasicStringView<TChar>)
// With hyphens.
void test_q3fshrenzkik54j8()
{
	constexpr static const std::uint8_t ref_uuid_bytes[16] =
	{
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
	};

	constexpr auto uuid = bstone::Uuid{bstone::StringView{"00112233-4455-6677-8899-aaBBccDDeEFf"}};

	const auto& uuid_bytes = reinterpret_cast<const std::uint8_t (&)[16]>(uuid);

	const auto is_valid = std::equal(
		std::cbegin(uuid_bytes),
		std::cend(uuid_bytes),
		ref_uuid_bytes);

	tester.check(is_valid);
}

// Uuid(BasicStringView<TChar>)
// With hyphens.
// Unsupported string length.
void test_wfgx9qbqand08fxw()
{
	auto is_failed = false;

	try
	{
		bstone::Uuid{bstone::StringView{"00112233-4455-6677-8899-aaBBccDDeEFf0"}};
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// Uuid(BasicStringView<TChar>)
// With hyphens.
// Non-hex digit.
void test_bccastxfwumxt148()
{
	auto is_failed = false;

	try
	{
		bstone::Uuid{bstone::StringView{"00112233-4455-6677-8899-aaBBccDDeEGg"}};
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// Uuid(BasicStringView<TChar>)
// With hyphens.
// Non-dash symbol.
void test_d68jxh59katyxkqr()
{
	auto is_failed = false;

	try
	{
		bstone::Uuid{bstone::StringView{"00112233-4455 6677-8899-aaBBccDDeEFf"}};
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// Uuid(BasicStringView<TChar>)
// With hyphens and braces.
void test_bdqyqxxrwvy8uka6()
{
	constexpr static const std::uint8_t ref_uuid_bytes[16] =
	{
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
	};

	constexpr auto uuid = bstone::Uuid{bstone::StringView{"{00112233-4455-6677-8899-aaBBccDDeEFf}"}};

	const auto& uuid_bytes = reinterpret_cast<const std::uint8_t (&)[16]>(uuid);

	const auto is_valid = std::equal(
		std::cbegin(uuid_bytes),
		std::cend(uuid_bytes),
		ref_uuid_bytes);

	tester.check(is_valid);
}

// Uuid(BasicStringView<TChar>)
// With hyphens and braces.
// Unsupported string length.
void test_hgt0xwk2pe9sx6jy()
{
	auto is_failed = false;

	try
	{
		bstone::Uuid{bstone::StringView{"{00112233-4455-6677-8899-aaBBccDDeEFf0}"}};
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// Uuid(BasicStringView<TChar>)
// With hyphens and braces.
// Non-hex digit.
void test_s2s9io6ypcvh3yjn()
{
	auto is_failed = false;

	try
	{
		bstone::Uuid{bstone::StringView{"{00112233-4455-6677-8899-aaBBccDDeEGg}"}};
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// Uuid(BasicStringView<TChar>)
// With hyphens and braces.
// Non-dash symbol.
void test_0j2vjyvziplh1y5n()
{
	auto is_failed = false;

	try
	{
		bstone::Uuid{bstone::StringView{"{00112233-4455 6677-8899-aaBBccDDeEFf}"}};
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// Uuid(const TChar*, std::intptr_t)
// Without hyphens.
void test_iaef6omlfc1irehg()
{
	constexpr static const std::uint8_t ref_uuid_bytes[16] =
	{
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
	};

	constexpr auto uuid = bstone::Uuid{"00112233445566778899aaBBccDDeEFf", 32};

	const auto& uuid_bytes = reinterpret_cast<const std::uint8_t (&)[16]>(uuid);

	const auto is_valid = std::equal(
		std::cbegin(uuid_bytes),
		std::cend(uuid_bytes),
		ref_uuid_bytes);

	tester.check(is_valid);
}

// Uuid(const TChar*, std::intptr_t)
// Without hyphens.
// Unsupported string length.
void test_9d9usg8eha3feusg()
{
	auto is_failed = false;

	try
	{
		bstone::Uuid{"00112233445566778899aaBBccDDeEFf0", 33};
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// Uuid(const TChar*, std::intptr_t)
// Without hyphens.
// Non-hex digit.
void test_7nwropq2r5ema6gi()
{
	auto is_failed = false;

	try
	{
		bstone::Uuid{"00112233445566778899aaBBccDDeEGg", 32};
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// Uuid(const TChar*, std::intptr_t)
// With hyphens.
void test_sx5i6kyqvvlyf949()
{
	constexpr static const std::uint8_t ref_uuid_bytes[16] =
	{
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
	};

	constexpr auto uuid = bstone::Uuid{"00112233-4455-6677-8899-aaBBccDDeEFf", 36};

	const auto& uuid_bytes = reinterpret_cast<const std::uint8_t (&)[16]>(uuid);

	const auto is_valid = std::equal(
		std::cbegin(uuid_bytes),
		std::cend(uuid_bytes),
		ref_uuid_bytes);

	tester.check(is_valid);
}

// Uuid(const TChar*, std::intptr_t)
// With hyphens.
// Unsupported string length.
void test_tdoghgiaudn0odkk()
{
	auto is_failed = false;

	try
	{
		bstone::Uuid{"00112233-4455-6677-8899-aaBBccDDeEFf0", 37};
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// Uuid(const TChar*, std::intptr_t)
// With hyphens.
// Non-hex digit.
void test_dilhb36cdq5c2fza()
{
	auto is_failed = false;

	try
	{
		bstone::Uuid{"00112233-4455-6677-8899-aaBBccDDeEGg", 36};
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// Uuid(const TChar*, std::intptr_t)
// With hyphens.
// Non-dash symbol.
void test_vbsnczccupzo4vau()
{
	auto is_failed = false;

	try
	{
		bstone::Uuid{"00112233-4455 6677-8899-aaBBccDDeEFf", 36};
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// Uuid(const TChar*, std::intptr_t)
// With hyphens and braces.
void test_3p23z0lro22owcjl()
{
	constexpr static const std::uint8_t ref_uuid_bytes[16] =
	{
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
	};

	constexpr auto uuid = bstone::Uuid{"{00112233-4455-6677-8899-aaBBccDDeEFf}", 38};

	const auto& uuid_bytes = reinterpret_cast<const std::uint8_t (&)[16]>(uuid);

	const auto is_valid = std::equal(
		std::cbegin(uuid_bytes),
		std::cend(uuid_bytes),
		ref_uuid_bytes);

	tester.check(is_valid);
}

// Uuid(const TChar*, std::intptr_t)
// With hyphens and braces.
// Unsupported string length.
void test_4wnie73jdzh9k1ok()
{
	auto is_failed = false;

	try
	{
		bstone::Uuid{"{00112233-4455-6677-8899-aaBBccDDeEFf0}", 39};
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// Uuid(const TChar*, std::intptr_t)
// With hyphens and braces.
// Non-hex digit.
void test_1bg3dnnz15xgpp0f()
{
	auto is_failed = false;

	try
	{
		bstone::Uuid{"{00112233-4455-6677-8899-aaBBccDDeEGg}", 38};
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// Uuid(const TChar*, std::intptr_t)
// With hyphens and braces.
// Non-dash symbol.
void test_63rx5actexr2zuyt()
{
	auto is_failed = false;

	try
	{
		bstone::Uuid{"{00112233-4455 6677-8899-aaBBccDDeEFf}", 38};
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// Uuid(const TChar*)
// Without hyphens.
void test_zxe1wp6zp263sqew()
{
	constexpr static const std::uint8_t ref_uuid_bytes[16] =
	{
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
	};

	constexpr auto uuid = bstone::Uuid{"00112233445566778899aaBBccDDeEFf"};

	const auto& uuid_bytes = reinterpret_cast<const std::uint8_t (&)[16]>(uuid);

	const auto is_valid = std::equal(
		std::cbegin(uuid_bytes),
		std::cend(uuid_bytes),
		ref_uuid_bytes);

	tester.check(is_valid);
}

// Uuid(const TChar*)
// Without hyphens.
// Unsupported string length.
void test_tknavxjeqq2k08ro()
{
	auto is_failed = false;

	try
	{
		bstone::Uuid{"00112233445566778899aaBBccDDeEFf0"};
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// Uuid(const TChar*)
// Without hyphens.
// Non-hex digit.
void test_kkdwdhbnn0fscujq()
{
	auto is_failed = false;

	try
	{
		bstone::Uuid{"00112233445566778899aaBBccDDeEGg"};
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// Uuid(const TChar*)
// With hyphens.
void test_pwn61148w0jfyw1m()
{
	constexpr static const std::uint8_t ref_uuid_bytes[16] =
	{
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
	};

	constexpr auto uuid = bstone::Uuid{"00112233-4455-6677-8899-aaBBccDDeEFf"};

	const auto& uuid_bytes = reinterpret_cast<const std::uint8_t (&)[16]>(uuid);

	const auto is_valid = std::equal(
		std::cbegin(uuid_bytes),
		std::cend(uuid_bytes),
		ref_uuid_bytes);

	tester.check(is_valid);
}

// Uuid(const TChar*)
// With hyphens.
// Unsupported string length.
void test_aigsp2bknghe5m8k()
{
	auto is_failed = false;

	try
	{
		bstone::Uuid{"00112233-4455-6677-8899-aaBBccDDeEFf0"};
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// Uuid(const TChar*)
// With hyphens.
// Non-hex digit.
void test_84ssv8kdvqw5xpcv()
{
	auto is_failed = false;

	try
	{
		bstone::Uuid{"00112233-4455-6677-8899-aaBBccDDeEGg"};
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// Uuid(const TChar*)
// With hyphens.
// Non-dash symbol.
void test_i9m7cf4t18r880s9()
{
	auto is_failed = false;

	try
	{
		bstone::Uuid{"00112233-4455 6677-8899-aaBBccDDeEFf"};
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// Uuid(const TChar*)
// With hyphens and braces.
void test_g2ro7esms6viss0t()
{
	constexpr static const std::uint8_t ref_uuid_bytes[16] =
	{
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
	};

	constexpr auto uuid = bstone::Uuid{"{00112233-4455-6677-8899-aaBBccDDeEFf}"};

	const auto& uuid_bytes = reinterpret_cast<const std::uint8_t (&)[16]>(uuid);

	const auto is_valid = std::equal(
		std::cbegin(uuid_bytes),
		std::cend(uuid_bytes),
		ref_uuid_bytes);

	tester.check(is_valid);
}

// Uuid(const TChar*)
// With hyphens and braces.
// Unsupported string length.
void test_vd7fhqi1hv5g98tz()
{
	auto is_failed = false;

	try
	{
		bstone::Uuid{"{00112233-4455-6677-8899-aaBBccDDeEFf0}"};
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// Uuid(const TChar*)
// With hyphens and braces.
// Non-hex digit.
void test_rakvay8580wryray()
{
	auto is_failed = false;

	try
	{
		bstone::Uuid{"{00112233-4455-6677-8899-aaBBccDDeEGg}"};
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// Uuid(const TChar*)
// With hyphens and braces.
// Non-dash symbol.
void test_0d6xx3ri31e8er8b()
{
	auto is_failed = false;

	try
	{
		bstone::Uuid{"{00112233-4455 6677-8899-aaBBccDDeEFf}"};
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// Uuid& operator=(const Uuid&) noexcept
void test_neqp351w0kgkyf99()
{
	constexpr static const std::uint8_t ref_uuid_bytes[16] =
	{
		0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
		0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
	};

	class Test
	{
	public:
		constexpr Test()
		{
			const auto uuid_1 = bstone::Uuid{ref_uuid_bytes};
			auto uuid = bstone::Uuid{};
			uuid = uuid_1;
			const auto& uuid_value = uuid.get_value();

			for (auto i = 0; i < 16; ++i)
			{
				if (uuid_value[i] != ref_uuid_bytes[i])
				{
					return;
				}
			}

			is_valid_ = true;
		}

		constexpr bool is_valid() const noexcept
		{
			return is_valid_;
		}

	private:
		bool is_valid_{};
	};

	constexpr auto test = Test{};
	tester.check(test.is_valid());
}

// ==========================================================================

// bool is_nil() const noexcept
void test_90c6d7njd6bseys6()
{
	constexpr auto uuid = bstone::Uuid{};
	constexpr auto is_valid = uuid.is_nil();
	tester.check(is_valid);
}

// bool is_nil() const noexcept
// Not nil.
void test_cymlorx8m3yxy44o()
{
	constexpr auto uuid = bstone::Uuid{"00112233445566778899aaBBccDDeEFf"};
	constexpr auto is_valid = !uuid.is_nil();
	tester.check(is_valid);
}

// ==========================================================================

// const UuidValue& get_value() const noexcept
void test_9u02y13vl2y7px9u()
{
	const auto uuid = bstone::Uuid{"00112233445566778899aaBBccDDeEFf"};
	const auto& value = uuid.get_value();

	const auto is_valid = 
		value[0] == 0x00 &&
		value[1] == 0x11 &&
		value[2] == 0x22 &&
		value[3] == 0x33 &&
		value[4] == 0x44 &&
		value[5] == 0x55 &&
		value[6] == 0x66 &&
		value[7] == 0x77 &&
		value[8] == 0x88 &&
		value[9] == 0x99 &&
		value[10] == 0xAA &&
		value[11] == 0xBB &&
		value[12] == 0xCC &&
		value[13] == 0xDD &&
		value[14] == 0xEE &&
		value[15] == 0xFF &&
		true;

	tester.check(is_valid);
}

// UuidValue& get_value() noexcept
void test_9p1qrjpf9viizoue()
{
	auto uuid = bstone::Uuid{"00112233445566778899aaBBccDDeEFf"};
	auto& value = uuid.get_value();

	const auto is_valid = 
		value[0] == 0x00 &&
		value[1] == 0x11 &&
		value[2] == 0x22 &&
		value[3] == 0x33 &&
		value[4] == 0x44 &&
		value[5] == 0x55 &&
		value[6] == 0x66 &&
		value[7] == 0x77 &&
		value[8] == 0x88 &&
		value[9] == 0x99 &&
		value[10] == 0xAA &&
		value[11] == 0xBB &&
		value[12] == 0xCC &&
		value[13] == 0xDD &&
		value[14] == 0xEE &&
		value[15] == 0xFF &&
		true;

	tester.check(is_valid);
}

// ==========================================================================

// TChar* to_chars(TChar*, TChar*, UuidStringFormat, UuidStringCase) const
// Without hyphens, lower-case.
void test_ntwult0yutibi05i()
{
	class Test
	{
	public:
		constexpr Test()
		{
			const auto uuid = bstone::Uuid{"00112233445566778899aaBBccDDeEFf"};
			char chars[32] = {};

			const auto chars_end = uuid.to_chars(
				chars,
				chars + 32,
				bstone::UuidStringFormat::without_hyphens,
				bstone::UuidStringCase::lower);

			is_valid_ =
				chars_end == chars + 32 &&
				chars[0] == '0' &&
				chars[1] == '0' &&
				chars[2] == '1' &&
				chars[3] == '1' &&
				chars[4] == '2' &&
				chars[5] == '2' &&
				chars[6] == '3' &&
				chars[7] == '3' &&
				chars[8] == '4' &&
				chars[9] == '4' &&
				chars[10] == '5' &&
				chars[11] == '5' &&
				chars[12] == '6' &&
				chars[13] == '6' &&
				chars[14] == '7' &&
				chars[15] == '7' &&
				chars[16] == '8' &&
				chars[17] == '8' &&
				chars[18] == '9' &&
				chars[19] == '9' &&
				chars[20] == 'a' &&
				chars[21] == 'a' &&
				chars[22] == 'b' &&
				chars[23] == 'b' &&
				chars[24] == 'c' &&
				chars[25] == 'c' &&
				chars[26] == 'd' &&
				chars[27] == 'd' &&
				chars[28] == 'e' &&
				chars[29] == 'e' &&
				chars[30] == 'f' &&
				chars[31] == 'f' &&
				true;
		}

		constexpr bool is_valid() const noexcept
		{
			return is_valid_;
		}

	private:
		bool is_valid_{};
	};

	constexpr auto test = Test{};
	tester.check(test.is_valid());
}

// TChar* to_chars(TChar*, TChar*, UuidStringFormat, UuidStringCase) const
// Without hyphens, lower-case.
// Buffer too small.
void test_nkqjgmxgfmf7f0b8()
{
	const auto uuid = bstone::Uuid{"00112233445566778899aaBBccDDeEFf"};
	auto is_failed = false;

	try
	{
		char chars[2] = {};

		uuid.to_chars(
			chars,
			chars + 2,
			bstone::UuidStringFormat::without_hyphens,
			bstone::UuidStringCase::lower);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// TChar* to_chars(TChar*, TChar*, UuidStringFormat, UuidStringCase) const
// With hyphens, lower-case.
void test_kxgvfs7hek3acfy8()
{
	class Test
	{
	public:
		constexpr Test()
		{
			const auto uuid = bstone::Uuid{"00112233445566778899aaBBccDDeEFf"};
			char chars[36] = {};

			const auto chars_end = uuid.to_chars(
				chars,
				chars + 36,
				bstone::UuidStringFormat::with_hyphens,
				bstone::UuidStringCase::lower);

			is_valid_ =
				chars_end == chars + 36 &&
				chars[0] == '0' &&
				chars[1] == '0' &&
				chars[2] == '1' &&
				chars[3] == '1' &&
				chars[4] == '2' &&
				chars[5] == '2' &&
				chars[6] == '3' &&
				chars[7] == '3' &&
				chars[8] == '-' &&
				chars[9] == '4' &&
				chars[10] == '4' &&
				chars[11] == '5' &&
				chars[12] == '5' &&
				chars[13] == '-' &&
				chars[14] == '6' &&
				chars[15] == '6' &&
				chars[16] == '7' &&
				chars[17] == '7' &&
				chars[18] == '-' &&
				chars[19] == '8' &&
				chars[20] == '8' &&
				chars[21] == '9' &&
				chars[22] == '9' &&
				chars[23] == '-' &&
				chars[24] == 'a' &&
				chars[25] == 'a' &&
				chars[26] == 'b' &&
				chars[27] == 'b' &&
				chars[28] == 'c' &&
				chars[29] == 'c' &&
				chars[30] == 'd' &&
				chars[31] == 'd' &&
				chars[32] == 'e' &&
				chars[33] == 'e' &&
				chars[34] == 'f' &&
				chars[35] == 'f' &&
				true;
		}

		constexpr bool is_valid() const noexcept
		{
			return is_valid_;
		}

	private:
		bool is_valid_{};
	};

	constexpr auto test = Test{};
	tester.check(test.is_valid());
}

// TChar* to_chars(TChar*, TChar*, UuidStringFormat, UuidStringCase) const
// With hyphens, lower-case.
// Buffer too small.
void test_3qflm4hp7jq7p0g0()
{
	const auto uuid = bstone::Uuid{"00112233445566778899aaBBccDDeEFf"};
	auto is_failed = false;

	try
	{
		char chars[2] = {};

		uuid.to_chars(
			chars,
			chars + 2,
			bstone::UuidStringFormat::with_hyphens,
			bstone::UuidStringCase::lower);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// TChar* to_chars(TChar*, TChar*, UuidStringFormat, UuidStringCase) const
// With hyphens and braces, lower-case.
void test_udoc6ccp23a73u8z()
{
	class Test
	{
	public:
		constexpr Test()
		{
			const auto uuid = bstone::Uuid{"00112233445566778899aaBBccDDeEFf"};
			char chars[38] = {};

			const auto chars_end = uuid.to_chars(
				chars,
				chars + 38,
				bstone::UuidStringFormat::with_hyphens_and_braces,
				bstone::UuidStringCase::lower);

			is_valid_ =
				chars_end == chars + 38 &&
				chars[0] == '{' &&
				chars[1] == '0' &&
				chars[2] == '0' &&
				chars[3] == '1' &&
				chars[4] == '1' &&
				chars[5] == '2' &&
				chars[6] == '2' &&
				chars[7] == '3' &&
				chars[8] == '3' &&
				chars[9] == '-' &&
				chars[10] == '4' &&
				chars[11] == '4' &&
				chars[12] == '5' &&
				chars[13] == '5' &&
				chars[14] == '-' &&
				chars[15] == '6' &&
				chars[16] == '6' &&
				chars[17] == '7' &&
				chars[18] == '7' &&
				chars[19] == '-' &&
				chars[20] == '8' &&
				chars[21] == '8' &&
				chars[22] == '9' &&
				chars[23] == '9' &&
				chars[24] == '-' &&
				chars[25] == 'a' &&
				chars[26] == 'a' &&
				chars[27] == 'b' &&
				chars[28] == 'b' &&
				chars[29] == 'c' &&
				chars[30] == 'c' &&
				chars[31] == 'd' &&
				chars[32] == 'd' &&
				chars[33] == 'e' &&
				chars[34] == 'e' &&
				chars[35] == 'f' &&
				chars[36] == 'f' &&
				chars[37] == '}' &&
				true;
		}

		constexpr bool is_valid() const noexcept
		{
			return is_valid_;
		}

	private:
		bool is_valid_{};
	};

	constexpr auto test = Test{};
	tester.check(test.is_valid());
}

// TChar* to_chars(TChar*, TChar*, UuidStringFormat, UuidStringCase) const
// With hyphens, lower-case.
// Buffer too small.
void test_b7bfnuswko2vbi9x()
{
	const auto uuid = bstone::Uuid{"00112233445566778899aaBBccDDeEFf"};
	auto is_failed = false;

	try
	{
		char chars[2] = {};

		uuid.to_chars(
			chars,
			chars + 2,
			bstone::UuidStringFormat::with_hyphens_and_braces,
			bstone::UuidStringCase::lower);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// TChar* to_chars(TChar*, TChar*, UuidStringFormat, UuidStringCase) const
// Without hyphens, upper-case.
void test_oiwckivn66vjgiqu()
{
	class Test
	{
	public:
		constexpr Test()
		{
			const auto uuid = bstone::Uuid{"00112233445566778899aaBBccDDeEFf"};
			char chars[32] = {};

			const auto chars_end = uuid.to_chars(
				chars,
				chars + 32,
				bstone::UuidStringFormat::without_hyphens,
				bstone::UuidStringCase::upper);

			is_valid_ =
				chars_end == chars + 32 &&
				chars[0] == '0' &&
				chars[1] == '0' &&
				chars[2] == '1' &&
				chars[3] == '1' &&
				chars[4] == '2' &&
				chars[5] == '2' &&
				chars[6] == '3' &&
				chars[7] == '3' &&
				chars[8] == '4' &&
				chars[9] == '4' &&
				chars[10] == '5' &&
				chars[11] == '5' &&
				chars[12] == '6' &&
				chars[13] == '6' &&
				chars[14] == '7' &&
				chars[15] == '7' &&
				chars[16] == '8' &&
				chars[17] == '8' &&
				chars[18] == '9' &&
				chars[19] == '9' &&
				chars[20] == 'A' &&
				chars[21] == 'A' &&
				chars[22] == 'B' &&
				chars[23] == 'B' &&
				chars[24] == 'C' &&
				chars[25] == 'C' &&
				chars[26] == 'D' &&
				chars[27] == 'D' &&
				chars[28] == 'E' &&
				chars[29] == 'E' &&
				chars[30] == 'F' &&
				chars[31] == 'F' &&
				true;
		}

		constexpr bool is_valid() const noexcept
		{
			return is_valid_;
		}

	private:
		bool is_valid_{};
	};

	constexpr auto test = Test{};
	tester.check(test.is_valid());
}

// TChar* to_chars(TChar*, TChar*, UuidStringFormat, UuidStringCase) const
// Without hyphens, upper-case.
// Buffer too small.
void test_u30b89jwpzkpow5h()
{
	const auto uuid = bstone::Uuid{"00112233445566778899aaBBccDDeEFf"};
	auto is_failed = false;

	try
	{
		char chars[2] = {};

		uuid.to_chars(
			chars,
			chars + 2,
			bstone::UuidStringFormat::without_hyphens,
			bstone::UuidStringCase::upper);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// TChar* to_chars(TChar*, TChar*, UuidStringFormat, UuidStringCase) const
// With hyphens, upper-case.
void test_v888gf79ab5v03a7()
{
	class Test
	{
	public:
		constexpr Test()
		{
			const auto uuid = bstone::Uuid{"00112233445566778899aaBBccDDeEFf"};
			char chars[36] = {};

			const auto chars_end = uuid.to_chars(
				chars,
				chars + 36,
				bstone::UuidStringFormat::with_hyphens,
				bstone::UuidStringCase::upper);

			is_valid_ =
				chars_end == chars + 36 &&
				chars[0] == '0' &&
				chars[1] == '0' &&
				chars[2] == '1' &&
				chars[3] == '1' &&
				chars[4] == '2' &&
				chars[5] == '2' &&
				chars[6] == '3' &&
				chars[7] == '3' &&
				chars[8] == '-' &&
				chars[9] == '4' &&
				chars[10] == '4' &&
				chars[11] == '5' &&
				chars[12] == '5' &&
				chars[13] == '-' &&
				chars[14] == '6' &&
				chars[15] == '6' &&
				chars[16] == '7' &&
				chars[17] == '7' &&
				chars[18] == '-' &&
				chars[19] == '8' &&
				chars[20] == '8' &&
				chars[21] == '9' &&
				chars[22] == '9' &&
				chars[23] == '-' &&
				chars[24] == 'A' &&
				chars[25] == 'A' &&
				chars[26] == 'B' &&
				chars[27] == 'B' &&
				chars[28] == 'C' &&
				chars[29] == 'C' &&
				chars[30] == 'D' &&
				chars[31] == 'D' &&
				chars[32] == 'E' &&
				chars[33] == 'E' &&
				chars[34] == 'F' &&
				chars[35] == 'F' &&
				true;
		}

		constexpr bool is_valid() const noexcept
		{
			return is_valid_;
		}

	private:
		bool is_valid_{};
	};

	constexpr auto test = Test{};
	tester.check(test.is_valid());
}

// TChar* to_chars(TChar*, TChar*, UuidStringFormat, UuidStringCase) const
// With hyphens, upper-case.
// Buffer too small.
void test_gmxafshxxafqu69f()
{
	const auto uuid = bstone::Uuid{"00112233445566778899aaBBccDDeEFf"};
	auto is_failed = false;

	try
	{
		char chars[2] = {};

		uuid.to_chars(
			chars,
			chars + 2,
			bstone::UuidStringFormat::with_hyphens,
			bstone::UuidStringCase::upper);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// TChar* to_chars(TChar*, TChar*, UuidStringFormat, UuidStringCase) const
// With hyphens and braces, upper-case.
void test_3y5olyj8p3c9kjia()
{
	class Test
	{
	public:
		constexpr Test()
		{
			const auto uuid = bstone::Uuid{"00112233445566778899aaBBccDDeEFf"};
			char chars[38] = {};

			const auto chars_end = uuid.to_chars(
				chars,
				chars + 38,
				bstone::UuidStringFormat::with_hyphens_and_braces,
				bstone::UuidStringCase::upper);

			is_valid_ =
				chars_end == chars + 38 &&
				chars[0] == '{' &&
				chars[1] == '0' &&
				chars[2] == '0' &&
				chars[3] == '1' &&
				chars[4] == '1' &&
				chars[5] == '2' &&
				chars[6] == '2' &&
				chars[7] == '3' &&
				chars[8] == '3' &&
				chars[9] == '-' &&
				chars[10] == '4' &&
				chars[11] == '4' &&
				chars[12] == '5' &&
				chars[13] == '5' &&
				chars[14] == '-' &&
				chars[15] == '6' &&
				chars[16] == '6' &&
				chars[17] == '7' &&
				chars[18] == '7' &&
				chars[19] == '-' &&
				chars[20] == '8' &&
				chars[21] == '8' &&
				chars[22] == '9' &&
				chars[23] == '9' &&
				chars[24] == '-' &&
				chars[25] == 'A' &&
				chars[26] == 'A' &&
				chars[27] == 'B' &&
				chars[28] == 'B' &&
				chars[29] == 'C' &&
				chars[30] == 'C' &&
				chars[31] == 'D' &&
				chars[32] == 'D' &&
				chars[33] == 'E' &&
				chars[34] == 'E' &&
				chars[35] == 'F' &&
				chars[36] == 'F' &&
				chars[37] == '}' &&
				true;
		}

		constexpr bool is_valid() const noexcept
		{
			return is_valid_;
		}

	private:
		bool is_valid_{};
	};

	constexpr auto test = Test{};
	tester.check(test.is_valid());
}

// TChar* to_chars(TChar*, TChar*, UuidStringFormat, UuidStringCase) const
// With hyphens, upper-case.
// Buffer too small.
void test_so1bej4etu2iuv4f()
{
	const auto uuid = bstone::Uuid{"00112233445566778899aaBBccDDeEFf"};
	auto is_failed = false;

	try
	{
		char chars[2] = {};

		uuid.to_chars(
			chars,
			chars + 2,
			bstone::UuidStringFormat::with_hyphens_and_braces,
			bstone::UuidStringCase::upper);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// TChar* to_chars(TChar*, TChar*, UuidStringFormat, UuidStringCase) const
// Unknown format, lower-case.
void test_acw5jbxnin8g4xne()
{
	const auto uuid = bstone::Uuid{"00112233445566778899aaBBccDDeEFf"};
	auto is_failed = false;

	try
	{
		char chars[38] = {};

		uuid.to_chars(
			chars,
			chars + 38,
			bstone::UuidStringFormat::none,
			bstone::UuidStringCase::lower);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// TChar* to_chars(TChar*, TChar*, UuidStringFormat, UuidStringCase) const
// Unknown format, upper-case.
void test_cok9jo8ewxafsqjz()
{
	const auto uuid = bstone::Uuid{"00112233445566778899aaBBccDDeEFf"};
	auto is_failed = false;

	try
	{
		char chars[38] = {};

		uuid.to_chars(
			chars,
			chars + 38,
			bstone::UuidStringFormat::none,
			bstone::UuidStringCase::upper);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// TChar* to_chars(TChar*, TChar*, UuidStringFormat, UuidStringCase) const
// Without hyphens, unknown case.
void test_95xurfwrdrfozozq()
{
	const auto uuid = bstone::Uuid{"00112233445566778899aaBBccDDeEFf"};
	auto is_failed = false;

	try
	{
		char chars[38] = {};

		uuid.to_chars(
			chars,
			chars + 38,
			bstone::UuidStringFormat::without_hyphens,
			bstone::UuidStringCase::none);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// TChar* to_chars(TChar*, TChar*, UuidStringFormat, UuidStringCase) const
// With hyphens, unknown case.
void test_5ts4cwo85kj5i10a()
{
	const auto uuid = bstone::Uuid{"00112233445566778899aaBBccDDeEFf"};
	auto is_failed = false;

	try
	{
		char chars[38] = {};

		uuid.to_chars(
			chars,
			chars + 38,
			bstone::UuidStringFormat::with_hyphens,
			bstone::UuidStringCase::none);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// TChar* to_chars(TChar*, TChar*, UuidStringFormat, UuidStringCase) const
// With hyphens and braces, unknown case.
void test_1l91bm5o4i43jfbs()
{
	const auto uuid = bstone::Uuid{"00112233445566778899aaBBccDDeEFf"};
	auto is_failed = false;

	try
	{
		char chars[38] = {};

		uuid.to_chars(
			chars,
			chars + 38,
			bstone::UuidStringFormat::with_hyphens_and_braces,
			bstone::UuidStringCase::none);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

void test_uurb3nm8qiuss2um()
{
	class Test
	{
	public:
		constexpr Test()
		{
			auto uuid = bstone::Uuid{"00112233445566778899aabbccddeeff"};
			uuid.swap_bytes(bstone::UuidEndianType::big);
			const auto& uuid_value = uuid.get_value();

			is_valid_ =
				uuid_value[0] == 0xFF &&
				uuid_value[1] == 0xEE &&
				uuid_value[2] == 0xDD &&
				uuid_value[3] == 0xCC &&
				uuid_value[4] == 0xBB &&
				uuid_value[5] == 0xAA &&
				uuid_value[6] == 0x99 &&
				uuid_value[7] == 0x88 &&
				uuid_value[8] == 0x77 &&
				uuid_value[9] == 0x66 &&
				uuid_value[10] == 0x55 &&
				uuid_value[11] == 0x44 &&
				uuid_value[12] == 0x33 &&
				uuid_value[13] == 0x22 &&
				uuid_value[14] == 0x11 &&
				uuid_value[15] == 0x00 &&
				true;
		}

		constexpr bool is_valid() const noexcept
		{
			return is_valid_;
		}

	private:
		bool is_valid_{};
	};

	constexpr auto test = Test{};
	tester.check(test.is_valid());
}

void test_6nk2s7hlfz54dq0z()
{
	class Test
	{
	public:
		constexpr Test()
		{
			auto uuid = bstone::Uuid{"00112233445566778899aabbccddeeff"};
			uuid.swap_bytes(bstone::UuidEndianType::little_mixed);
			const auto& uuid_value = uuid.get_value();

			is_valid_ =
				uuid_value[0] == 0x33 &&
				uuid_value[1] == 0x22 &&
				uuid_value[2] == 0x11 &&
				uuid_value[3] == 0x00 &&
				uuid_value[4] == 0x55 &&
				uuid_value[5] == 0x44 &&
				uuid_value[6] == 0x77 &&
				uuid_value[7] == 0x66 &&
				uuid_value[8] == 0x88 &&
				uuid_value[9] == 0x99 &&
				uuid_value[10] == 0xAA &&
				uuid_value[11] == 0xBB &&
				uuid_value[12] == 0xCC &&
				uuid_value[13] == 0xDD &&
				uuid_value[14] == 0xEE &&
				uuid_value[15] == 0xFF &&
				true;
		}

		constexpr bool is_valid() const noexcept
		{
			return is_valid_;
		}

	private:
		bool is_valid_{};
	};

	constexpr auto test = Test{};
	tester.check(test.is_valid());
}

// ==========================================================================

// Uuid generate()
void test_tkb0xqsowitk1opk()
{
	const auto uuid = bstone::Uuid::generate();
	const auto& uuid_value = uuid.get_value();
	const auto uuid_version = uuid_value[6] >> 4;
	const auto uuid_variant = uuid_value[8] >> 6;
	const auto is_version_4 = uuid_version == 0B0100;
	const auto is_variant_1 = uuid_variant == 0B10;
	const auto is_valid = is_version_4 && is_variant_1;
	tester.check(is_valid);
}

// ==========================================================================

// bool operator==(const Uuid&, const Uuid&) noexcept
void test_lf19l29qt21yykzm()
{
	constexpr auto uuid_1 = bstone::Uuid{"00112233445566778899aaBBccDDeEFf"};
	constexpr auto uuid_2 = bstone::Uuid{"00112233445566778899aaBBccDDeEFf"};
	constexpr auto is_valid = uuid_1 == uuid_2;
	tester.check(is_valid);
}

// bool operator==(const Uuid&, const Uuid&) noexcept
// Not equal.
void test_c3z1z1oxwnaxjoqz()
{
	constexpr auto uuid_1 = bstone::Uuid{"00112233445566778899aaBBccDDeEFf"};
	constexpr auto uuid_2 = bstone::Uuid{"00112233445566778899aaBBccDDeE00"};
	constexpr auto is_valid = !(uuid_1 == uuid_2);
	tester.check(is_valid);
}

// ==========================================================================

// bool operator!=(const Uuid&, const Uuid&) noexcept
void test_ka8mlkmskfvlqhri()
{
	constexpr auto uuid_1 = bstone::Uuid{"00112233445566778899aaBBccDDeEFf"};
	constexpr auto uuid_2 = bstone::Uuid{"00112233445566778899aaBBccDDeE00"};
	constexpr auto is_valid = uuid_1 != uuid_2;
	tester.check(is_valid);
}

// bool operator!=(const Uuid&, const Uuid&) noexcept
// Not equal.
void test_mwa3jwgt978uair0()
{
	constexpr auto uuid_1 = bstone::Uuid{"00112233445566778899aaBBccDDeEFf"};
	constexpr auto uuid_2 = bstone::Uuid{"00112233445566778899aaBBccDDeEFf"};
	constexpr auto is_valid = !(uuid_1 != uuid_2);
	tester.check(is_valid);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_ctor();
		register_assignment_operator();
		register_is_nil();
		register_get_value();
		register_to_chars();
		register_swap_bytes();
		register_generate();
		register_equality_operator();
		register_not_equality_operator();
	}

private:
	void register_ctor()
	{
		tester.register_test("Uuid#qtudbxc80p7n0ro4", test_qtudbxc80p7n0ro4);
		tester.register_test("Uuid#ddgndk5w6u9ifqef", test_ddgndk5w6u9ifqef);
		tester.register_test("Uuid#zz4hrxf94ajv16fv", test_zz4hrxf94ajv16fv);
		tester.register_test("Uuid#6mmfx7oqkvu46lmo", test_6mmfx7oqkvu46lmo);
		tester.register_test("Uuid#wpfpbufa35y14jdg", test_wpfpbufa35y14jdg);
		tester.register_test("Uuid#2itngjmrkmuzbk4n", test_2itngjmrkmuzbk4n);
		tester.register_test("Uuid#spu3juxzm3inqnqs", test_spu3juxzm3inqnqs);
		tester.register_test("Uuid#1fa65x799xzejdeo", test_1fa65x799xzejdeo);
		tester.register_test("Uuid#9ekg4up92qi2f0q4", test_9ekg4up92qi2f0q4);
		tester.register_test("Uuid#q3fshrenzkik54j8", test_q3fshrenzkik54j8);
		tester.register_test("Uuid#wfgx9qbqand08fxw", test_wfgx9qbqand08fxw);
		tester.register_test("Uuid#bccastxfwumxt148", test_bccastxfwumxt148);
		tester.register_test("Uuid#d68jxh59katyxkqr", test_d68jxh59katyxkqr);
		tester.register_test("Uuid#bdqyqxxrwvy8uka6", test_bdqyqxxrwvy8uka6);
		tester.register_test("Uuid#hgt0xwk2pe9sx6jy", test_hgt0xwk2pe9sx6jy);
		tester.register_test("Uuid#s2s9io6ypcvh3yjn", test_s2s9io6ypcvh3yjn);
		tester.register_test("Uuid#0j2vjyvziplh1y5n", test_0j2vjyvziplh1y5n);
		tester.register_test("Uuid#iaef6omlfc1irehg", test_iaef6omlfc1irehg);
		tester.register_test("Uuid#9d9usg8eha3feusg", test_9d9usg8eha3feusg);
		tester.register_test("Uuid#7nwropq2r5ema6gi", test_7nwropq2r5ema6gi);
		tester.register_test("Uuid#sx5i6kyqvvlyf949", test_sx5i6kyqvvlyf949);
		tester.register_test("Uuid#tdoghgiaudn0odkk", test_tdoghgiaudn0odkk);
		tester.register_test("Uuid#dilhb36cdq5c2fza", test_dilhb36cdq5c2fza);
		tester.register_test("Uuid#vbsnczccupzo4vau", test_vbsnczccupzo4vau);
		tester.register_test("Uuid#3p23z0lro22owcjl", test_3p23z0lro22owcjl);
		tester.register_test("Uuid#4wnie73jdzh9k1ok", test_4wnie73jdzh9k1ok);
		tester.register_test("Uuid#1bg3dnnz15xgpp0f", test_1bg3dnnz15xgpp0f);
		tester.register_test("Uuid#63rx5actexr2zuyt", test_63rx5actexr2zuyt);
		tester.register_test("Uuid#zxe1wp6zp263sqew", test_zxe1wp6zp263sqew);
		tester.register_test("Uuid#tknavxjeqq2k08ro", test_tknavxjeqq2k08ro);
		tester.register_test("Uuid#kkdwdhbnn0fscujq", test_kkdwdhbnn0fscujq);
		tester.register_test("Uuid#pwn61148w0jfyw1m", test_pwn61148w0jfyw1m);
		tester.register_test("Uuid#aigsp2bknghe5m8k", test_aigsp2bknghe5m8k);
		tester.register_test("Uuid#84ssv8kdvqw5xpcv", test_84ssv8kdvqw5xpcv);
		tester.register_test("Uuid#i9m7cf4t18r880s9", test_i9m7cf4t18r880s9);
		tester.register_test("Uuid#g2ro7esms6viss0t", test_g2ro7esms6viss0t);
		tester.register_test("Uuid#vd7fhqi1hv5g98tz", test_vd7fhqi1hv5g98tz);
		tester.register_test("Uuid#rakvay8580wryray", test_rakvay8580wryray);
		tester.register_test("Uuid#0d6xx3ri31e8er8b", test_0d6xx3ri31e8er8b);
	}

	void register_assignment_operator()
	{
		tester.register_test("Uuid#neqp351w0kgkyf99", test_neqp351w0kgkyf99);
	}

	void register_is_nil()
	{
		tester.register_test("Uuid#90c6d7njd6bseys6", test_90c6d7njd6bseys6);
		tester.register_test("Uuid#cymlorx8m3yxy44o", test_cymlorx8m3yxy44o);
	}

	void register_get_value()
	{
		tester.register_test("Uuid#9u02y13vl2y7px9u", test_9u02y13vl2y7px9u);
		tester.register_test("Uuid#9p1qrjpf9viizoue", test_9p1qrjpf9viizoue);
	}

	void register_to_chars()
	{
		tester.register_test("Uuid#ntwult0yutibi05i", test_ntwult0yutibi05i);
		tester.register_test("Uuid#nkqjgmxgfmf7f0b8", test_nkqjgmxgfmf7f0b8);
		tester.register_test("Uuid#kxgvfs7hek3acfy8", test_kxgvfs7hek3acfy8);
		tester.register_test("Uuid#3qflm4hp7jq7p0g0", test_3qflm4hp7jq7p0g0);
		tester.register_test("Uuid#udoc6ccp23a73u8z", test_udoc6ccp23a73u8z);
		tester.register_test("Uuid#b7bfnuswko2vbi9x", test_b7bfnuswko2vbi9x);

		tester.register_test("Uuid#oiwckivn66vjgiqu", test_oiwckivn66vjgiqu);
		tester.register_test("Uuid#u30b89jwpzkpow5h", test_u30b89jwpzkpow5h);
		tester.register_test("Uuid#v888gf79ab5v03a7", test_v888gf79ab5v03a7);
		tester.register_test("Uuid#gmxafshxxafqu69f", test_gmxafshxxafqu69f);
		tester.register_test("Uuid#3y5olyj8p3c9kjia", test_3y5olyj8p3c9kjia);
		tester.register_test("Uuid#so1bej4etu2iuv4f", test_so1bej4etu2iuv4f);

		tester.register_test("Uuid#acw5jbxnin8g4xne", test_acw5jbxnin8g4xne);
		tester.register_test("Uuid#cok9jo8ewxafsqjz", test_cok9jo8ewxafsqjz);
		tester.register_test("Uuid#95xurfwrdrfozozq", test_95xurfwrdrfozozq);
		tester.register_test("Uuid#5ts4cwo85kj5i10a", test_5ts4cwo85kj5i10a);
		tester.register_test("Uuid#1l91bm5o4i43jfbs", test_1l91bm5o4i43jfbs);
	}

	void register_swap_bytes()
	{
		tester.register_test("Uuid#uurb3nm8qiuss2um", test_uurb3nm8qiuss2um);
		tester.register_test("Uuid#6nk2s7hlfz54dq0z", test_6nk2s7hlfz54dq0z);
	}

	void register_generate()
	{
		tester.register_test("Uuid#tkb0xqsowitk1opk", test_tkb0xqsowitk1opk);
	}

	void register_equality_operator()
	{
		tester.register_test("Uuid#lf19l29qt21yykzm", test_lf19l29qt21yykzm);
		tester.register_test("Uuid#c3z1z1oxwnaxjoqz", test_c3z1z1oxwnaxjoqz);
	}

	void register_not_equality_operator()
	{
		tester.register_test("Uuid#ka8mlkmskfvlqhri", test_ka8mlkmskfvlqhri);
		tester.register_test("Uuid#mwa3jwgt978uair0", test_mwa3jwgt978uair0);
	}
};

auto registrator = Registrator{};

} // namespace
