#if defined(_WIN32)

#include <string>

#include "bstone_tester.h"
#include "bstone_win32_registry_key.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

#define SUBKEY_NAME "software\\bibendovsky\\bstone\\tests"

constexpr auto subkey_name = SUBKEY_NAME;
constexpr auto subkey_name_2 = SUBKEY_NAME "\\sub1";

const char* get_invalid_key_name()
{
	// Max length of a value name is 16383.
	// https://learn.microsoft.com/en-us/windows/win32/sysinfo/registry-element-size-limits
	static const auto result = std::string(std::string::size_type{16'384}, '1');

	return result.c_str();
}

// ==========================================================================

// RegistryKey()
void test_z0ihlwqbgea0hviu()
{
	const auto registry_key = bstone::win32::RegistryKey{};
	const auto is_open = registry_key.is_open();
	tester.check(!is_open);
}

// ==========================================================================

// RegistryKey(const char*, RegistryRootKeyType, RegistryOpenFlags)
void test_l2ldncmu6curwuf1()
{
	const auto registry_key = bstone::win32::RegistryKey
	{
		nullptr,
		bstone::win32::RegistryRootKeyType::current_user,
		bstone::win32::RegistryOpenFlags::read};

	const auto is_open = registry_key.is_open();

	tester.check(is_open);
}

// ==========================================================================

// RegistryKey(RegistryKey&&)
void test_mzipl9rotruiszn0()
{
	auto registry_key_1 = bstone::win32::RegistryKey
	{
		nullptr,
		bstone::win32::RegistryRootKeyType::current_user,
		bstone::win32::RegistryOpenFlags::read
	};

	const auto is_open_1 = registry_key_1.is_open();

	auto registry_key_2 = std::move(registry_key_1);
	const auto is_open_2 = registry_key_2.is_open();

	tester.check(is_open_1 && is_open_2);
}

// ==========================================================================

// RegistryKey& operator=(RegistryKey&&)
void test_a3yj341s6kff63wf()
{
	auto registry_key_1 = bstone::win32::RegistryKey
	{
		nullptr,
		bstone::win32::RegistryRootKeyType::current_user,
		bstone::win32::RegistryOpenFlags::read
	};

	const auto is_open_1 = registry_key_1.is_open();

	auto registry_key_2 = bstone::win32::RegistryKey{};
	registry_key_2 = std::move(registry_key_1);
	const auto is_open_2 = registry_key_2.is_open();

	tester.check(is_open_1 && is_open_2);
}

// ==========================================================================

// bool try_open(const char*, RegistryRootKeyType, RegistryOpenFlags)
void test_0r76500o7gnx7asv()
{
	auto registry_key = bstone::win32::RegistryKey{};

	const auto is_open = registry_key.try_open(
		nullptr,
		bstone::win32::RegistryRootKeyType::current_user,
		bstone::win32::RegistryOpenFlags::read);

	tester.check(is_open);
}

// ==========================================================================

// void open(const char*, RegistryRootKeyType, RegistryOpenFlags)
void test_l6osw97li1kjthro()
{
	auto registry_key = bstone::win32::RegistryKey{};

	registry_key.open(
		nullptr,
		bstone::win32::RegistryRootKeyType::current_user,
		bstone::win32::RegistryOpenFlags::read);

	const auto is_open = registry_key.is_open();

	tester.check(is_open);
}

// ==========================================================================

// void close()
void test_qb43oqjdfwrjgjwq()
{
	auto registry_key = bstone::win32::RegistryKey{};

	registry_key.open(
		nullptr,
		bstone::win32::RegistryRootKeyType::current_user,
		bstone::win32::RegistryOpenFlags::read);

	const auto is_open_1 = registry_key.is_open();
	registry_key.close();
	const auto is_open_2 = registry_key.is_open();

	tester.check(is_open_1 && !is_open_2);
}

// ==========================================================================

// bool is_open() const noexcept
void test_tkki906da9bi5vqm()
{
	auto registry_key = bstone::win32::RegistryKey
	{
		nullptr,
		bstone::win32::RegistryRootKeyType::current_user,
		bstone::win32::RegistryOpenFlags::read
	};

	const auto is_open = registry_key.is_open();
	tester.check(is_open);
}

// ==========================================================================

// bool try_get_string(const char*, char*, std::intptr_t, std::intptr_t&) const
void test_dl0mp0me8gnwty1x()
{
	constexpr auto ref_name = "str1";
	constexpr auto ref_value = "\x23\xC2\xA9\xE2\x84\x96\xF0\x9D\x9B\x81"; // "#©№𝛁"

	{
		const auto registry_key = bstone::win32::RegistryKey
		{
			subkey_name,
			bstone::win32::RegistryRootKeyType::current_user,
			bstone::win32::RegistryOpenFlags::create
		};

		registry_key.set_string(ref_name, ref_value);
	}

	const auto registry_key = bstone::win32::RegistryKey
	{
		subkey_name,
		bstone::win32::RegistryRootKeyType::current_user,
		bstone::win32::RegistryOpenFlags::read,
	};

	char buffer[100];
	auto written_count = std::intptr_t{};
	const auto is_written = registry_key.try_get_string(ref_name, buffer, 100, written_count);
	const auto value = std::string{buffer, static_cast<std::size_t>(written_count)};

	tester.check(is_written && written_count == 10 && value == ref_value);
}

// ==========================================================================

// bool try_get_string(const char*, char*, std::intptr_t, std::intptr_t&) const
// Fail.
void test_zx5lavr0xn68xc27()
{
	const auto registry_key = bstone::win32::RegistryKey
	{
		nullptr,
		bstone::win32::RegistryRootKeyType::current_user,
		bstone::win32::RegistryOpenFlags::read
	};

	char buffer[100];
	auto written_count = std::intptr_t{};
	const auto is_written = registry_key.try_get_string(get_invalid_key_name(), buffer, 100, written_count);

	tester.check(!is_written);
}

// ==========================================================================

// std::intptr_t get_string(const char*, char*, std::intptr_t) const
void test_9rylj1a9cwtzjg7t()
{
	constexpr auto ref_name = "str1";
	constexpr auto ref_value = "\x23\xC2\xA9\xE2\x84\x96\xF0\x9D\x9B\x81"; // "#©№𝛁"

	{
		const auto registry_key = bstone::win32::RegistryKey
		{
			subkey_name,
			bstone::win32::RegistryRootKeyType::current_user,
			bstone::win32::RegistryOpenFlags::create
		};

		registry_key.set_string(ref_name, ref_value);
	}

	const auto registry_key = bstone::win32::RegistryKey
	{
		subkey_name,
		bstone::win32::RegistryRootKeyType::current_user,
		bstone::win32::RegistryOpenFlags::read,
	};

	char buffer[100];
	const auto written_count = registry_key.get_string(ref_name, buffer, 100);
	const auto value = std::string{buffer, static_cast<std::size_t>(written_count)};

	tester.check(written_count == 10 && value == ref_value);
}

// ==========================================================================

// std::intptr_t get_string(const char*, char*, std::intptr_t) const
// Fail.
void test_4ekia0b4x3yp6964()
{
	const auto registry_key = bstone::win32::RegistryKey
	{
		nullptr,
		bstone::win32::RegistryRootKeyType::current_user,
		bstone::win32::RegistryOpenFlags::read
	};

	auto is_failed = false;

	try
	{
		char buffer[100];
		registry_key.get_string(get_invalid_key_name(), buffer, 100);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// bool try_set_string(const char*, const char*) const
void test_ap6rzmym5iskb9wn()
{
	constexpr auto ref_name = "str1";
	constexpr auto ref_value = "\x23\xC2\xA9\xE2\x84\x96\xF0\x9D\x9B\x81"; // "#©№𝛁"

	const auto registry_key = bstone::win32::RegistryKey
	{
		subkey_name,
		bstone::win32::RegistryRootKeyType::current_user,
		bstone::win32::RegistryOpenFlags::create | bstone::win32::RegistryOpenFlags::read_write,
	};

	const auto is_written = registry_key.try_set_string(ref_name, ref_value);

	char buffer[100];
	const auto written_count = registry_key.get_string(ref_name, buffer, 100);
	const auto value = std::string{buffer, static_cast<std::size_t>(written_count)};

	tester.check(is_written && written_count == 10 && value == ref_value);
}

// ==========================================================================

// bool try_set_string(const char*, const char*) const
// Fail.
void test_upcbtyz46waps9v8()
{
	constexpr auto ref_value = "\x23\xC2\xA9\xE2\x84\x96\xF0\x9D\x9B\x81"; // "#©№𝛁"

	const auto registry_key = bstone::win32::RegistryKey
	{
		subkey_name,
		bstone::win32::RegistryRootKeyType::current_user,
		bstone::win32::RegistryOpenFlags::create | bstone::win32::RegistryOpenFlags::read,
	};

	const auto is_written = registry_key.try_set_string(get_invalid_key_name(), ref_value);

	tester.check(!is_written);
}

// ==========================================================================

// void set_string(const char*, const char*) const
void test_1rlh04njnc0bol5i()
{
	constexpr auto ref_name = "str1";
	constexpr auto ref_value = "\x23\xC2\xA9\xE2\x84\x96\xF0\x9D\x9B\x81"; // "#©№𝛁"

	const auto registry_key = bstone::win32::RegistryKey
	{
		subkey_name,
		bstone::win32::RegistryRootKeyType::current_user,
		bstone::win32::RegistryOpenFlags::create | bstone::win32::RegistryOpenFlags::read_write,
	};

	registry_key.set_string(ref_name, ref_value);

	char buffer[100];
	const auto written_count = registry_key.get_string(ref_name, buffer, 100);
	const auto value = std::string{buffer, static_cast<std::size_t>(written_count)};

	tester.check(written_count == 10 && value == ref_value);
}

// ==========================================================================

// void set_string(const char*, const char*) const
// Fail.
void test_zneaxh6wo1jfvll0()
{
	constexpr auto ref_value = "\x23\xC2\xA9\xE2\x84\x96\xF0\x9D\x9B\x81"; // "#©№𝛁"

	const auto registry_key = bstone::win32::RegistryKey
	{
		subkey_name,
		bstone::win32::RegistryRootKeyType::current_user,
		bstone::win32::RegistryOpenFlags::create | bstone::win32::RegistryOpenFlags::read,
	};

	auto is_failed = false;

	try
	{
		registry_key.set_string(get_invalid_key_name(), ref_value);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// void delete_value(const char*) const
void test_9ob1yjtphri2petm()
{
	constexpr auto ref_name = "str1";
	constexpr auto ref_value = "\x23\xC2\xA9\xE2\x84\x96\xF0\x9D\x9B\x81"; // "#©№𝛁"

	{
		const auto registry_key = bstone::win32::RegistryKey
		{
			subkey_name,
			bstone::win32::RegistryRootKeyType::current_user,
			bstone::win32::RegistryOpenFlags::create
		};

		registry_key.set_string(ref_name, ref_value);
	}

	const auto registry_key = bstone::win32::RegistryKey
	{
		subkey_name,
		bstone::win32::RegistryRootKeyType::current_user,
		bstone::win32::RegistryOpenFlags::write
	};

	registry_key.delete_value(ref_name);
}

// ==========================================================================

// void delete_value(const char*) const
// Fail.
void test_1hiy32f55v4zy4pg()
{
	constexpr auto ref_name = "str1";
	constexpr auto ref_value = "\x23\xC2\xA9\xE2\x84\x96\xF0\x9D\x9B\x81"; // "#©№𝛁"

	{
		const auto registry_key = bstone::win32::RegistryKey
		{
			subkey_name,
			bstone::win32::RegistryRootKeyType::current_user,
			bstone::win32::RegistryOpenFlags::create
		};

		registry_key.set_string(ref_name, ref_value);
	}

	const auto registry_key = bstone::win32::RegistryKey
	{
		subkey_name,
		bstone::win32::RegistryRootKeyType::current_user,
		bstone::win32::RegistryOpenFlags::write,
	};

	auto is_failed = false;

	try
	{
		registry_key.delete_value(get_invalid_key_name());
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// bool try_delete_key(const char*, RegistryRootKeyType, RegistryOpenFlags)
void test_7xnbw9oaaaw568yu()
{
	constexpr auto ref_name = "str2";
	constexpr auto ref_value = "\x23\xC2\xA9\xE2\x84\x96\xF0\x9D\x9B\x81"; // "#©№𝛁"

	{
		const auto registry_key = bstone::win32::RegistryKey
		{
			subkey_name_2,
			bstone::win32::RegistryRootKeyType::current_user,
			bstone::win32::RegistryOpenFlags::create
		};

		registry_key.set_string(ref_name, ref_value);
	}

	const auto is_deleted = bstone::win32::RegistryKey::try_delete_key(
		subkey_name_2,
		bstone::win32::RegistryRootKeyType::current_user,
		bstone::win32::RegistryOpenFlags::none);

	tester.check(is_deleted);
}

// ==========================================================================

// bool try_delete_key(const char*, RegistryRootKeyType, RegistryOpenFlags)
// Fail.
void test_io5ba1cdznps0kp1()
{
	const auto is_deleted = bstone::win32::RegistryKey::try_delete_key(
		get_invalid_key_name(),
		bstone::win32::RegistryRootKeyType::current_user,
		bstone::win32::RegistryOpenFlags::none);

	tester.check(!is_deleted);
}

// ==========================================================================

// void delete_key(const char*, RegistryRootKeyType, RegistryOpenFlags)
void test_ivskndeco8qvjdlp()
{
	constexpr auto ref_name = "str2";
	constexpr auto ref_value = "\x23\xC2\xA9\xE2\x84\x96\xF0\x9D\x9B\x81"; // "#©№𝛁"

	{
		const auto registry_key = bstone::win32::RegistryKey
		{
			subkey_name_2,
			bstone::win32::RegistryRootKeyType::current_user,
			bstone::win32::RegistryOpenFlags::create
		};

		registry_key.set_string(ref_name, ref_value);
	}

	auto is_failed = false;

	try
	{
		bstone::win32::RegistryKey::delete_key(
			subkey_name_2,
			bstone::win32::RegistryRootKeyType::current_user,
			bstone::win32::RegistryOpenFlags::none);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_failed);
}

// ==========================================================================

// void delete_key(const char*, RegistryRootKeyType, RegistryOpenFlags)
// Fail.
void test_pnp8gmhyk1u155h6()
{
	auto is_failed = false;

	try
	{
		bstone::win32::RegistryKey::delete_key(
			get_invalid_key_name(),
			bstone::win32::RegistryRootKeyType::current_user,
			bstone::win32::RegistryOpenFlags::none);
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
		register_registry_key();
		register_try_open();
		register_open();
		register_close();
		register_is_open();
		register_try_get_string();
		register_get_string();
		register_try_set_string();
		register_set_string();
		register_delete_value();
		register_delete_key();
		register_try_delete_key();
	}

private:
	void register_registry_key()
	{
		tester.register_test("win32::RegistryKey#z0ihlwqbgea0hviu", test_z0ihlwqbgea0hviu);
		tester.register_test("win32::RegistryKey#l2ldncmu6curwuf1", test_l2ldncmu6curwuf1);
		tester.register_test("win32::RegistryKey#mzipl9rotruiszn0", test_mzipl9rotruiszn0);
		tester.register_test("win32::RegistryKey#a3yj341s6kff63wf", test_a3yj341s6kff63wf);
	}

	void register_try_open()
	{
		tester.register_test("win32::RegistryKey#0r76500o7gnx7asv", test_0r76500o7gnx7asv);
	}

	void register_open()
	{
		tester.register_test("win32::RegistryKey#l6osw97li1kjthro", test_l6osw97li1kjthro);
	}

	void register_close()
	{
		tester.register_test("win32::RegistryKey#qb43oqjdfwrjgjwq", test_qb43oqjdfwrjgjwq);
	}

	void register_is_open()
	{
		tester.register_test("win32::RegistryKey#tkki906da9bi5vqm", test_tkki906da9bi5vqm);
	}

	void register_try_get_string()
	{
		tester.register_test("win32::RegistryKey#dl0mp0me8gnwty1x", test_dl0mp0me8gnwty1x);
		tester.register_test("win32::RegistryKey#zx5lavr0xn68xc27", test_zx5lavr0xn68xc27);
	}

	void register_get_string()
	{
		tester.register_test("win32::RegistryKey#9rylj1a9cwtzjg7t", test_9rylj1a9cwtzjg7t);
		tester.register_test("win32::RegistryKey#4ekia0b4x3yp6964", test_4ekia0b4x3yp6964);
	}

	void register_try_set_string()
	{
		tester.register_test("win32::RegistryKey#ap6rzmym5iskb9wn", test_ap6rzmym5iskb9wn);
		tester.register_test("win32::RegistryKey#upcbtyz46waps9v8", test_upcbtyz46waps9v8);
	}

	void register_set_string()
	{
		tester.register_test("win32::RegistryKey#1rlh04njnc0bol5i", test_1rlh04njnc0bol5i);
		tester.register_test("win32::RegistryKey#zneaxh6wo1jfvll0", test_zneaxh6wo1jfvll0);
	}

	void register_delete_value()
	{
		tester.register_test("win32::RegistryKey#9ob1yjtphri2petm", test_9ob1yjtphri2petm);
		tester.register_test("win32::RegistryKey#1hiy32f55v4zy4pg", test_1hiy32f55v4zy4pg);
	}

	void register_try_delete_key()
	{
		tester.register_test("win32::RegistryKey#7xnbw9oaaaw568yu", test_7xnbw9oaaaw568yu);
		tester.register_test("win32::RegistryKey#io5ba1cdznps0kp1", test_io5ba1cdznps0kp1);
	}

	void register_delete_key()
	{
		tester.register_test("win32::RegistryKey#ivskndeco8qvjdlp", test_ivskndeco8qvjdlp);
		tester.register_test("win32::RegistryKey#pnp8gmhyk1u155h6", test_pnp8gmhyk1u155h6);
	}
};

auto registrator = Registrator{};

} // namespace

#endif // _WIN32
