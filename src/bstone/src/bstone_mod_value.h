/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
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
