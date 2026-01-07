/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_GAME_TICKER_INCLUDED
#define BSTONE_GAME_TICKER_INCLUDED

#include <cstdint>
#include <atomic>
#include "bstone_timer.h"

namespace bstone {

using GameTimerTicks = std::int32_t;

class GameTimer
{
public:
	GameTimer() = default;
	GameTimer(const GameTimer& rhs) = delete;
	GameTimer(GameTimer&& rhs) noexcept = delete;
	void operator=(GameTimerTicks ticks);
	GameTimer& operator=(const GameTimer& rhs) = delete;
	GameTimer& operator=(GameTimer&& rhs) noexcept = delete;
	~GameTimer();

	bool is_started() const;
	void start(int frequency);
	void stop();
	GameTimerTicks get_ticks() const;
	void set_ticks(GameTimerTicks ticks);
	void subtract_ticks(GameTimerTicks ticks);
	operator GameTimerTicks() const;

private:
	using MtTicks = std::atomic<GameTimerTicks>;

	MtTicks mt_ticks_{};
	Timer timer_{};

	void ensure_is_started() const;
	void set_ticks_internal(GameTimerTicks ticks);
	void increase_ticks();

	static void callback_proxy(void* user_data);
	void callback();
};

// ======================================

void operator-=(GameTimer& a, GameTimerTicks b);

} // namespace bstone

#endif // BSTONE_GAME_TICKER_INCLUDED
