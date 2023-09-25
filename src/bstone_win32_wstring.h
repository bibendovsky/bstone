/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// WIN32 wide string.

#if defined(_WIN32)

#include <memory>

#include "bstone_int.h"
#include "bstone_memory_resource.h"

namespace bstone {

class Win32WString
{
public:
	Win32WString(const char* u8_string);

	IntP get_size() const noexcept;

	const wchar_t* get_data() const noexcept;
	wchar_t* get_data() noexcept;

private:
	class StorageDeleter
	{
	public:
		StorageDeleter(MemoryResource& memory_resource);

		void operator()(wchar_t* ptr) const;

	private:
		MemoryResource* memory_resource_{};
	};

	using Storage = std::unique_ptr<wchar_t, StorageDeleter>;

private:
	IntP size_{};
	Storage storage_;

private:
	static Storage make_storage(const char* u8_string, IntP& u16_size);
};

} // namespace bstone

#endif // _WIN32
