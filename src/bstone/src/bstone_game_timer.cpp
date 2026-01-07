/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_game_timer.h"
#include "bstone_exception.h"

namespace bstone {

void GameTimer::operator=(GameTimerTicks ticks)
{
	set_ticks(ticks);
}

GameTimer::~GameTimer()
{
	stop();
}

bool GameTimer::is_started() const
{
	return timer_.started();
}

void GameTimer::start(int frequency)
try
{
	stop();
	if (frequency < 10 || frequency > 100)
	{
		BSTONE_THROW_STATIC_SOURCE("Frequency out of range.");
	}
	set_ticks_internal(0);
	const long long period_ns = 1'000'000'000LL / frequency;
	timer_.start_ns(period_ns, &GameTimer::callback_proxy, this);
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GameTimer::stop()
{
	timer_.stop();
}

GameTimerTicks GameTimer::get_ticks() const
{
	ensure_is_started();
	return mt_ticks_.load(std::memory_order_acquire);
}

void GameTimer::set_ticks(GameTimerTicks ticks)
{
	ensure_is_started();
	set_ticks_internal(ticks);
}

void GameTimer::subtract_ticks(GameTimerTicks ticks)
{
	ensure_is_started();
	mt_ticks_.fetch_sub(ticks, std::memory_order_relaxed);
}

GameTimer::operator GameTimerTicks() const
{
	return get_ticks();
}

void GameTimer::ensure_is_started() const
{
	if (!is_started())
	{
		BSTONE_THROW_STATIC_SOURCE("Not started.");
	}
}

void GameTimer::set_ticks_internal(GameTimerTicks ticks)
{
	mt_ticks_.store(ticks, std::memory_order_relaxed);
}

void GameTimer::increase_ticks()
{
	mt_ticks_.fetch_add(1, std::memory_order_relaxed);
}

void GameTimer::callback_proxy(void* user_data)
{
	static_cast<GameTimer*>(user_data)->callback();
}

void GameTimer::callback()
{
	increase_ticks();
}

// ======================================

void operator-=(GameTimer& a, GameTimerTicks b)
{
	a.subtract_ticks(b);
}

} // namespace bstone
