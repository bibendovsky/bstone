/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Timer

#ifndef BSTONE_TIMER_INCLUDED
#define BSTONE_TIMER_INCLUDED

namespace bstone {

using TimerCallback = void (*)(void* user_data);

class Timer
{
public:
	Timer() = default;
	Timer(const Timer& rhs) = delete;
	Timer& operator=(const Timer& rhs) = delete;
	~Timer();

	bool started() const;
	void start_ns(long long period_ns, TimerCallback callback, void* user_data);
	void stop();

private:
	class Impl;

	void* user_data_{};
	TimerCallback callback_{};
	unsigned int native_handle_{};
};

} // namespace bstone

#endif // BSTONE_TIMER_INCLUDED
