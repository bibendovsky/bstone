/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Windows registry key primitive.

#if !defined(BSTONE_WIN32_REGISTRY_KEY_INCLUDED)
#define BSTONE_WIN32_REGISTRY_KEY_INCLUDED

#if defined(_WIN32)

#include <memory>

#include "bstone_enum_flags.h"
#include "bstone_int.h"

namespace bstone {
namespace win32 {

enum class RegistryAccessType : unsigned int
{
	none = 0,

	create = 1U << 0, // Implies `write`.
	read = 1U << 1,
	write = 1U << 2,

	read_write = read | write,

	wow64_32 = 1U << 3,
	wow64_64 = 1U << 4,
};

BSTONE_ENABLE_ENUM_CLASS_BITWISE_OPS_FOR(RegistryAccessType)

// ==========================================================================

enum class RegistryRootKeyType
{
	none = 0,
	local_machine,
	current_user,
};

// ==========================================================================

struct RegistryKeyHandle {};

struct RegistryKeyHandleDeleter
{
	void operator()(RegistryKeyHandle* handle) const;
};

using RegistryKeyHandleUPtr = std::unique_ptr<RegistryKeyHandle, RegistryKeyHandleDeleter>;

// ==========================================================================

// Notes:
//  - Pass null or empty string for a value name to operate on unnamed value.
//
class RegistryKey
{
public:
	static constexpr auto max_string_length = 16'383;

public:
	RegistryKey() = default;

	RegistryKey(
		const char* subkey_name,
		RegistryRootKeyType root_key_type,
		RegistryAccessType access_type);

	void open(
		const char* subkey_name,
		RegistryRootKeyType root_key_type,
		RegistryAccessType access_type);

	void close();
	bool is_open() const noexcept;

	bool has_string(const char* name) const;

	// Retreives a null-terminated string value into the provided buffer.
	//
	// Returns:
	//  - Length of the string without null symbol.
	//
	IntP get_string(const char* name, char* buffer, IntP buffer_size) const;

	void set_string(const char* name, const char* value) const;

	void delete_value(const char* name) const;

	// Parameters:
	//  - access_type
	//    Valid values: `none`, `wow64_32`, `wow64_64`.
	//    All other values will be ignored.
	static bool has_key(
		const char* subkey_name,
		RegistryRootKeyType root_key_type,
		RegistryAccessType access_type = RegistryAccessType::none);

	// Parameters:
	//  - access_type
	//    Valid values: `none`, `wow64_32`, `wow64_64`.
	//    All other values will be ignored.
	static void delete_key(
		const char* subkey_name,
		RegistryRootKeyType root_key_type,
		RegistryAccessType access_type = RegistryAccessType::none);

private:
	RegistryKeyHandleUPtr handle_{};

private:
	void ensure_is_open() const;
};

} // namespace win32
} // namespace bstone

#endif // _WIN32

#endif // BSTONE_WIN32_REGISTRY_KEY_INCLUDED
