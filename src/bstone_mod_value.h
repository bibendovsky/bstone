/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


//
// A value wrapper to track changes.
//


#ifndef BSTONE_MOD_VALUE_INCLUDE
#define BSTONE_MOD_VALUE_INCLUDE


namespace bstone
{


template<typename T>
class ModValue final
{
public:
	ModValue()
		:
		is_modified_{true},
		value_{}
	{
	}

	explicit ModValue(
		const T& value)
		:
		is_modified_{true},
		value_{value}
	{
	}

	ModValue(
		ModValue&& rhs) noexcept
		:
		is_modified_{std::move(rhs.is_modified_)},
		value_{std::move(rhs.value_)}
	{
	}

	ModValue& operator=(
		const T& rhs)
	{
		is_modified_ |= (value_ != rhs);
		value_ = rhs;

		return *this;
	}

	ModValue& operator=(
		const ModValue& rhs)
	{
		if (std::addressof(rhs) != this)
		{
			is_modified_ = rhs.is_modified_;
			value_ = rhs.value_;
		}

		return *this;
	}

	ModValue& operator=(
		ModValue&& rhs) noexcept
	{
		is_modified_ = std::move(rhs.is_modified_);
		value_ = std::move(rhs.value_);

		return *this;
	}

	~ModValue() = default;


	operator const T&() const noexcept
	{
		return value_;
	}


	T& operator*() noexcept
	{
		return value_;
	}

	const T& operator*() const noexcept
	{
		return value_;
	}


	T* operator->() noexcept
	{
		return &value_;
	}

	const T* operator->() const noexcept
	{
		return &value_;
	}


	bool is_modified() const noexcept
	{
		return is_modified_;
	}

	void set_is_modified(
		const bool value) noexcept
	{
		is_modified_ = value;
	}


private:
	bool is_modified_;
	T value_;
}; // ModValue


} // bstone


#endif // !BSTONE_MOD_VALUE_INCLUDE
