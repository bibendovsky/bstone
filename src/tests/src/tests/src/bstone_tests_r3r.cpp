#include <cstdint>

#include "bstone_r3r.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

std::uint8_t buffer[3 * 4 * 2];

// ==========================================================================

// void r3r_validate_read_pixels_param(const R3rReadPixelsParam&, int, int)
void test_togn86jyn8b42a2p()
{
	const auto param = bstone::R3rReadPixelsParam{
		.pixel_format = bstone::sys::PixelFormat::r8g8b8,
		.width = 4,
		.height = 2,
		.buffer = buffer};

	auto is_failed = false;

	try
	{
		bstone::r3r_validate_read_pixels_param(param, 4, 2);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_failed);
}

// void r3r_validate_read_pixels_param(const R3rReadPixelsParam&, int, int)
// Unsupported pixel format.
void test_2xzgqvr59z3omnta()
{
	const auto param = bstone::R3rReadPixelsParam{
		.pixel_format = bstone::sys::PixelFormat::b8g8r8a8,
		.width = 4,
		.height = 2,
		.buffer = buffer};

	auto is_failed = false;

	try
	{
		bstone::r3r_validate_read_pixels_param(param, 4, 2);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// void r3r_validate_read_pixels_param(const R3rReadPixelsParam&, int, int)
// Null buffer.
void test_63kn34qkp1o6p160()
{
	const auto param = bstone::R3rReadPixelsParam{
		.pixel_format = bstone::sys::PixelFormat::r8g8b8,
		.width = 4,
		.height = 2,
		.buffer = nullptr};

	auto is_failed = false;

	try
	{
		bstone::r3r_validate_read_pixels_param(param, 4, 2);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// void r3r_validate_read_pixels_param(const R3rReadPixelsParam&, int, int)
// Destination narrower than the image.
void test_0w65hpbd2w0u877t()
{
	const auto param = bstone::R3rReadPixelsParam{
		.pixel_format = bstone::sys::PixelFormat::r8g8b8,
		.width = 4,
		.height = 2,
		.buffer = buffer};

	auto is_failed = false;

	try
	{
		bstone::r3r_validate_read_pixels_param(param, 8, 2);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// void r3r_validate_read_pixels_param(const R3rReadPixelsParam&, int, int)
// Destination wider than the image.
void test_6036ue7dgnn36t8z()
{
	const auto param = bstone::R3rReadPixelsParam{
		.pixel_format = bstone::sys::PixelFormat::r8g8b8,
		.width = 4,
		.height = 2,
		.buffer = buffer};

	auto is_failed = false;

	try
	{
		bstone::r3r_validate_read_pixels_param(param, 2, 2);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// void r3r_validate_read_pixels_param(const R3rReadPixelsParam&, int, int)
// Destination shorter than the image.
void test_xjkwh00fug8d2tih()
{
	const auto param = bstone::R3rReadPixelsParam{
		.pixel_format = bstone::sys::PixelFormat::r8g8b8,
		.width = 4,
		.height = 2,
		.buffer = buffer};

	auto is_failed = false;

	try
	{
		bstone::r3r_validate_read_pixels_param(param, 4, 4);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_validate_read_pixels_param();
	}

private:
	void register_validate_read_pixels_param()
	{
		tester.register_test("R3r#togn86jyn8b42a2p", test_togn86jyn8b42a2p);
		tester.register_test("R3r#2xzgqvr59z3omnta", test_2xzgqvr59z3omnta);
		tester.register_test("R3r#63kn34qkp1o6p160", test_63kn34qkp1o6p160);
		tester.register_test("R3r#0w65hpbd2w0u877t", test_0w65hpbd2w0u877t);
		tester.register_test("R3r#6036ue7dgnn36t8z", test_6036ue7dgnn36t8z);
		tester.register_test("R3r#xjkwh00fug8d2tih", test_xjkwh00fug8d2tih);
	}
};

auto registrator = Registrator{};

} // namespace
