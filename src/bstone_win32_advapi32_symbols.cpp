/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// advapi32.dll dynamic symbols.

#ifdef _WIN32

#include "bstone_win32_advapi32_symbols.h"

#include "bstone_shared_library.h"

namespace bstone {
namespace win32 {

namespace {

class AdvApi32SymbolsImpl
{
public:
	AdvApi32SymbolsImpl();
	RegDeleteKeyExWFunc get_reg_delete_key_ex_w() const noexcept;

	static AdvApi32SymbolsImpl& get_object();

private:
	SharedLibrary shared_library_{};
	RegDeleteKeyExWFunc reg_delete_key_ex_w_{};
};

// --------------------------------------------------------------------------

AdvApi32SymbolsImpl::AdvApi32SymbolsImpl()
	:
	shared_library_{"advapi32.dll"},
	reg_delete_key_ex_w_{shared_library_.find_symbol<RegDeleteKeyExWFunc>("RegDeleteKeyExW")}
{}

RegDeleteKeyExWFunc AdvApi32SymbolsImpl::get_reg_delete_key_ex_w() const noexcept
{
	return reg_delete_key_ex_w_;
}

AdvApi32SymbolsImpl& AdvApi32SymbolsImpl::get_object()
{
	static AdvApi32SymbolsImpl advapi32_symbols_impl{};
	return advapi32_symbols_impl;
}

} // namespace

// ==========================================================================

RegDeleteKeyExWFunc AdvApi32Symbols::get_reg_delete_key_ex_w() noexcept
{
	return AdvApi32SymbolsImpl::get_object().get_reg_delete_key_ex_w();
}

} // namespace win32
} // namespace bstone

#endif // _WIN32
