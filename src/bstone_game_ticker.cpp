/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_exception.h"
#include "bstone_game_ticker.h"

namespace bstone {

void GameTicker::operator=(TickValue value) noexcept
{
	tick_.store(value, std::memory_order_release);
}

GameTicker::~GameTicker()
{
	close();
}

void GameTicker::open(int frequency)
BSTONE_BEGIN_FUNC_TRY
	close();

	if (frequency < 10 || frequency > 100)
	{
		BSTONE_THROW_STATIC_SOURCE("Frequency out of range.");
	}

	interval_ = std::chrono::milliseconds{1000 / frequency};
	quit_flag_ = false;
	quit_flag_ack_ = false;
	tick_.store(TickValue{}, std::memory_order_release);
	thread_ = Thread{&GameTicker::callback, this};
	is_open_ = true;
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GameTicker::close() noexcept
{
	if (is_open_)
	{
		constexpr auto sleep_duration = std::chrono::milliseconds{1};

		quit_flag_ = true;

		while (!quit_flag_ack_ || !thread_.joinable())
		{
			std::this_thread::sleep_for(sleep_duration);
		}

		thread_.join();
	}

	is_open_ = false;
}

GameTicker::operator TickValue() const noexcept
{
	return tick_.load(std::memory_order_acquire);
}

void GameTicker::operator-=(TickValue value) noexcept
{
	tick_ -= value;
}

void GameTicker::callback()
{
	while (!quit_flag_)
	{
		std::this_thread::sleep_for(interval_);
		++tick_;
	}

	quit_flag_ack_ = true;
}

} // namespace bstone
