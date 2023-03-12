/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_GAME_TICKER_INCLUDED)
#define BSTONE_GAME_TICKER_INCLUDED

#include <thread>
#include "bstone_atomic_flag.h"

namespace bstone {

class GameTicker
{
public:
	using TickValue = unsigned int;

public:
	GameTicker() = default;
	GameTicker(const GameTicker& rhs) = delete;
	GameTicker(GameTicker&& rhs) noexcept = delete;
	void operator=(TickValue value) noexcept;
	GameTicker& operator=(const GameTicker& rhs) = delete;
	GameTicker& operator=(GameTicker&& rhs) noexcept = delete;
	~GameTicker();

	void open(int frequency);
	void close() noexcept;

	operator TickValue() const noexcept;
	void operator-=(TickValue value) noexcept;

private:
	using Tick = std::atomic<TickValue>;
	using Thread = std::thread;

	bool is_open_{};
	std::chrono::milliseconds interval_{};
	AtomicFlag quit_flag_{};
	AtomicFlag quit_flag_ack_{};
	Tick tick_{};
	Thread thread_{};

	void callback();
};

} // namespace bstone

#endif // BSTONE_GAME_TICKER_INCLUDED
