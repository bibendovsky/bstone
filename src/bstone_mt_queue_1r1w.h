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
// A thread-safe, fixed-size queue for one reader and one writer.
//


#ifndef BSTONE_MT_QUEUE_1R1W_INCLUDED
#define BSTONE_MT_QUEUE_1R1W_INCLUDED


#include <atomic>
#include <functional>
#include <stdexcept>
#include <vector>


namespace bstone
{


template<typename T>
class MtQueue1R1W
{
public:
	MtQueue1R1W()
		:
		read_index_{},
		write_index_{},
		items_{}
	{
		read_index_ = 0;
		write_index_ = 0;
	}

	MtQueue1R1W(
		const MtQueue1R1W& that) = delete;

	MtQueue1R1W& operator=(
		const MtQueue1R1W& that) = delete;

	~MtQueue1R1W()
	{
	}

	bool is_initialized() const
	{
		return !items_.empty();
	}

	void initialize(
		const int size)
	{
		if (size <= 1)
		{
			throw std::runtime_error("Size should be greater then one.");
		}

		if (is_initialized())
		{
			throw std::runtime_error("Already initialized.");
		}

		items_.resize(size);
	}

	void uninitialize()
	{
		read_index_ = 0;
		write_index_ = 0;
		items_ = {};
	}

	// Tries to push a specified value into the queue.
	// Returns true if succeed or false otherwise.
	bool push(
		const T& value)
	{
		auto next_index = (write_index_ + 1) % items_.size();

		if (next_index == read_index_)
		{
			return false;
		}

		items_[write_index_] = value;

		write_index_ = next_index;

		return true;
	}

	// Tries to push a value initialized by function into the queue.
	// Returns true if succeed or false otherwise.
	bool push(
		std::function<void(T&)> function)
	{
		auto next_index = (write_index_ + 1) % items_.size();

		if (next_index == read_index_)
		{
			return false;
		}

		function(items_[write_index_]);

		write_index_ = next_index;

		return true;
	}

	// Tries to pop an item from the queue.
	// Returns true if succeed or false otherwise.
	bool pop(
		T& value)
	{
		if (read_index_ == write_index_)
		{
			return false;
		}

		auto next_index = (read_index_ + 1) % items_.size();

		value = items_[read_index_];

		read_index_ = next_index;

		return true;
	}


private:
	using Items = std::vector<T>;


	std::atomic_size_t read_index_;
	std::atomic_size_t write_index_;
	Items items_;
}; // MtQueue1R1W


} // bstone


#endif // !BSTONE_MT_QUEUE_1R1W_INCLUDED
