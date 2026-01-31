/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Wide string stuff for Windows

#include "bstone_win32_utility.h"

namespace bstone::win32 {

template<int TStackCapacity>
class StackOrHeapWString
{
public:
	StackOrHeapWString(const StackOrHeapWString&) = delete;
	StackOrHeapWString& operator=(const StackOrHeapWString&) = delete;

	explicit StackOrHeapWString(int capacity)
	{
		if (capacity < TStackCapacity)
		{
			storage_ = stack_storage_;
		}
		else
		{
			const int storage_size = (capacity + 1) * 2;
			storage_ = static_cast<wchar_t*>(::operator new(storage_size));
		}
		size_ = capacity;
	}

	explicit StackOrHeapWString(const char* u8_string)
	{
		const int u8_size_with_null = get_string_size(u8_string) + 1;
		const int u16_size_with_null = utf8_to_utf16(u8_string, u8_size_with_null);
		if (u16_size_with_null < 0)
		{
			return;
		}
		if (u16_size_with_null <= TStackCapacity)
		{
			storage_ = stack_storage_;
		}
		else
		{
			const int storage_size = u16_size_with_null * 2;
			storage_ = static_cast<wchar_t*>(::operator new(storage_size));
		}
		const int written_u16_size_with_null = utf8_to_utf16(u8_string, u8_size_with_null, storage_, u16_size_with_null);
		if (written_u16_size_with_null != u16_size_with_null)
		{
			deallocate();
			storage_ = nullptr;
			return;
		}
		size_ = u16_size_with_null - 1;
	}

	~StackOrHeapWString()
	{
		deallocate();
	}

	const wchar_t* get() const
	{
		return storage_;
	}

	wchar_t* get()
	{
		return storage_;
	}

	int get_size() const
	{
		return size_;
	}

	operator const wchar_t*() const
	{
		return get();
	}

	operator wchar_t*()
	{
		return get();
	}

private:
	wchar_t stack_storage_[TStackCapacity];
	wchar_t* storage_;
	int size_{};

	void deallocate()
	{
		if (storage_ != stack_storage_)
		{
			::operator delete(storage_);
		}
	}
};

} // namespace bstone::win32
