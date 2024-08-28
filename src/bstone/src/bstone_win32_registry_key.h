/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Windows registry key primitive.

#ifndef BSTONE_WIN32_REGISTRY_KEY_INCLUDED
#define BSTONE_WIN32_REGISTRY_KEY_INCLUDED

#if defined(_WIN32)

#include <cstdint>

#include <memory>

#include "bstone_enum_flags.h"

namespace bstone {
namespace win32 {

enum class RegistryOpenFlags : unsigned int
{
	none = 0,

	create = 1U << 0, // Implies `write`.
	read = 1U << 1,
	write = 1U << 2,

	read_write = read | write,

	wow64_32 = 1U << 3,
	wow64_64 = 1U << 4,
};

BSTONE_ENABLE_ENUM_CLASS_BITWISE_OPS_FOR(RegistryOpenFlags)

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
	void operator()(RegistryKeyHandle* handle) const noexcept;
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
	RegistryKey() noexcept;

	RegistryKey(
		const char* subkey_name,
		RegistryRootKeyType root_key_type,
		RegistryOpenFlags open_flags);

	bool try_open(
		const char* subkey_name,
		RegistryRootKeyType root_key_type,
		RegistryOpenFlags open_flags);

	void open(
		const char* subkey_name,
		RegistryRootKeyType root_key_type,
		RegistryOpenFlags open_flags);

	void close() noexcept;
	bool is_open() const noexcept;

	// Retreives a null-terminated string value into the provided buffer.
	//
	// Returns:
	//  - Length of the string without null symbol.
	//
	bool try_get_string(
		const char* name,
		char* buffer,
		std::intptr_t buffer_size,
		std::intptr_t& written_size) const;

	// Retreives a null-terminated string value into the provided buffer.
	//
	// Returns:
	//  - Length of the string without null symbol.
	//
	std::intptr_t get_string(const char* name, char* buffer, std::intptr_t buffer_size) const;

	bool try_set_string(const char* name, const char* value) const;
	void set_string(const char* name, const char* value) const;

	bool try_delete_value(const char* name) const;
	void delete_value(const char* name) const;

	// Parameters:
	//  - open_flags
	//    Valid values: `none`, `wow64_32`, `wow64_64`.
	//    All other values will be ignored.
	static bool try_delete_key(
		const char* subkey_name,
		RegistryRootKeyType root_key_type,
		RegistryOpenFlags open_flags = RegistryOpenFlags::none);

	// Parameters:
	//  - open_flags
	//    Valid values: `none`, `wow64_32`, `wow64_64`.
	//    All other values will be ignored.
	static void delete_key(
		const char* subkey_name,
		RegistryRootKeyType root_key_type,
		RegistryOpenFlags open_flags = RegistryOpenFlags::none);

private:
	RegistryKeyHandleUPtr handle_{};

private:
	bool try_or_open(
		const char* subkey_name,
		RegistryRootKeyType root_key_type,
		RegistryOpenFlags open_flags,
		bool is_ignore_errors);

	bool try_or_get_string(
		const char* name,
		char* buffer,
		std::intptr_t buffer_size,
		bool is_ignore_errors,
		std::intptr_t& written_size) const;

	bool try_or_set_string(const char* name, const char* value, bool is_ignore_errors) const;

	bool try_or_delete_value(const char* name, bool is_ignore_errors) const;

	static bool try_or_delete_key(
		const char* subkey_name,
		RegistryRootKeyType root_key_type,
		RegistryOpenFlags open_flags,
		bool is_ignore_errors);
};

} // namespace win32
} // namespace bstone

#endif // _WIN32

#endif // BSTONE_WIN32_REGISTRY_KEY_INCLUDED
