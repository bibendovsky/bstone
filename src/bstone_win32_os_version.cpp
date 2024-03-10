/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Windows OS version.

/*
Getting the OS version based on:
https://learn.microsoft.com/en-us/windows/win32/wmisdk/example--getting-wmi-data-from-the-local-computer
*/

#ifdef _WIN32

#include "bstone_win32_os_version.h"

#include <wbemidl.h>

#include "bstone_char_conv.h"
#include "bstone_exception.h"
#include "bstone_string_view.h"
#include "bstone_win32_com.h"

namespace bstone {
namespace win32 {

namespace {

class OsVersionImpl
{
public:
	OsVersionImpl();

	OsVersion get_os_version() const noexcept;

private:
	ComScope com_scope_{};
	OsVersion os_version_{};

private:
	static OsVersion parse_os_version(WStringView u16_version_sv);
};

// --------------------------------------------------------------------------

OsVersionImpl::OsVersionImpl()
try {
	using IWbemLocatorUPtr = ComUPtr<IWbemLocator>;
	using IWbemServicesUPtr = ComUPtr<IWbemServices>;
	using IEnumWbemClassObjectUPtr = ComUPtr<IEnumWbemClassObject>;
	using IWbemClassObjectUPtr = ComUPtr<IWbemClassObject>;

	auto hresult = HRESULT{};

	// Set general COM security levels.
	//
	hresult = CoInitializeSecurity(
		nullptr, // Access permissions.
		-1, // Authentication service count.
		nullptr, // Authentication services.
		nullptr, // Reserved.
		RPC_C_AUTHN_LEVEL_DEFAULT, // Default authentication.
		RPC_C_IMP_LEVEL_IMPERSONATE, // Default impersonation.
		nullptr, // Authentication info.
		EOAC_NONE, // Additional capabilities.
		nullptr // Reserved.
	);

	if (FAILED(hresult))
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to set general COM security levels.");
	}

	// Obtain the initial locator to WMI.
	//
	IWbemLocator* wbem_locator = nullptr;

	hresult = CoCreateInstance(
		CLSID_WbemLocator,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator,
		reinterpret_cast<LPVOID*>(&wbem_locator));

	if (FAILED(hresult))
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to create IWbemLocator object.");
	}

	const auto wbem_locator_uptr = IWbemLocatorUPtr{wbem_locator};

	// Connect to WMI through the IWbemLocator::ConnectServer method.
	//
	IWbemServices* wbem_services = nullptr;

	// Connect to the root\cimv2 namespace with the current user and
	// obtain pointer pSvc to make IWbemServices calls.
	hresult = wbem_locator_uptr->ConnectServer(
		BStr{L"ROOT\\CIMV2"}, // Object path of WMI namespace.
		nullptr, // User name.
		nullptr, // User password.
		nullptr, // Locale.
		0, // Security flags.
		nullptr, // Authority.
		nullptr, // Context object.
		&wbem_services // Pointer to IWbemServices proxy.
	);

	if (FAILED(hresult))
	{
		BSTONE_THROW_STATIC_SOURCE("Could not connect.");
	}

	const auto wbem_services_uptr = IWbemServicesUPtr{wbem_services};

	// Set security levels on the proxy.
	//
	hresult = CoSetProxyBlanket(
		wbem_services_uptr.get(), // Indicates the proxy to set.
		RPC_C_AUTHN_WINNT,
		RPC_C_AUTHZ_NONE,
		nullptr, // Server principal name.
		RPC_C_AUTHN_LEVEL_CALL,
		RPC_C_IMP_LEVEL_IMPERSONATE,
		nullptr, // Client identity.
		EOAC_NONE // Proxy capabilities.
	);

	if (FAILED(hresult))
	{
		BSTONE_THROW_STATIC_SOURCE("Could not set proxy blanket.");
	}

	// Use the IWbemServices pointer to make requests of WMI.
	//
	IEnumWbemClassObject* wbem_enumerator = nullptr;

	hresult = wbem_services_uptr->ExecQuery(
		BStr{L"WQL"},
		BStr{L"SELECT Version FROM Win32_OperatingSystem"},
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		nullptr,
		&wbem_enumerator);

	if (FAILED(hresult))
	{
		BSTONE_THROW_STATIC_SOURCE("WMI query failed.");
	}

	const auto wbem_enumerator_uptr = IEnumWbemClassObjectUPtr{wbem_enumerator};

	// Get the data from the query.
	//
	IWbemClassObject* wbem_version_object = nullptr;

	auto wbem_version_count = ULONG{};

	hresult = wbem_enumerator_uptr->Next(
		static_cast<long>(WBEM_INFINITE),
		1,
		&wbem_version_object,
		&wbem_version_count);

	if (wbem_version_count == 0)
	{
		BSTONE_THROW_STATIC_SOURCE("No version object.");
	}

	const auto wbem_version_object_uptr = IWbemClassObjectUPtr{wbem_version_object};

	auto wbem_version_property = Variant{};
	hresult = wbem_version_object_uptr->Get(L"Version", 0, &wbem_version_property.get(), 0, 0);

	if (FAILED(hresult))
	{
		BSTONE_THROW_STATIC_SOURCE("No version property.");
	}

	// Parse the version string.
	//
	const auto version_string = wbem_version_property.get().bstrVal;
	const auto version_string_size = static_cast<std::intptr_t>(SysStringLen(version_string));
	os_version_ = parse_os_version(WStringView{version_string, version_string_size});
}
catch (const std::exception& exception)
{
	MessageBoxA(nullptr, exception.what(), "BStone", MB_OK | MB_ICONERROR);
	throw;
}
catch (...)
{
	MessageBoxA(nullptr, "Failed to get OS version.", "BStone", MB_OK | MB_ICONERROR);
	throw;
}

// --------------------------------------------------------------------------

OsVersion OsVersionImpl::get_os_version() const noexcept
{
	return os_version_;
}

OsVersion OsVersionImpl::parse_os_version(WStringView version_sv)
{
	const auto dot_1_index = version_sv.index_of(L'.');
	const auto dot_2_index = version_sv.last_index_of(L'.');

	if (dot_1_index < 0 || dot_2_index < 0 || dot_1_index >= dot_2_index)
	{
		BSTONE_THROW_STATIC_SOURCE("Unsupported OS version string.");
	}

	const auto major_sv = version_sv.get_subview(0, dot_1_index);
	const auto minor_sv = version_sv.get_subview(dot_1_index + 1, dot_2_index - dot_1_index - 1);
	const auto build_sv = version_sv.get_subview(dot_2_index + 1);

	auto os_version = OsVersion{};
	from_chars(major_sv.cbegin(), major_sv.cend(), os_version.major);
	from_chars(minor_sv.cbegin(), minor_sv.cend(), os_version.minor);
	from_chars(build_sv.cbegin(), build_sv.cend(), os_version.build);
	return os_version;
}

// ==========================================================================

const OsVersionImpl os_version_impl{};

} // namespace

// ==========================================================================

OsVersion get_os_version() noexcept
{
	return os_version_impl.get_os_version();
}

} // namespace win32
} // namespace bstone

#endif // _WIN32
