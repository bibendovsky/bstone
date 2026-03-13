/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_VK_R3R_OBSERVER_INCLUDED
#define BSTONE_VK_R3R_OBSERVER_INCLUDED

#include <type_traits>
#include <unordered_set>

template<typename TContext>
class VkR3rObserver
{
public:
	virtual ~VkR3rObserver() = default;

	virtual void update(TContext& context) = 0;
};

// ======================================

template<>
class VkR3rObserver<void>
{
public:
	virtual ~VkR3rObserver() = default;

	virtual void update() = 0;
};

// ======================================

template<typename TContext>
class VkR3rSubject
{
public:
	using Observer = VkR3rObserver<TContext>;

public:
	virtual ~VkR3rSubject() = default;

	virtual void attach(Observer& observer) = 0;
	virtual void detach(Observer& observer) = 0;
	virtual void notify(TContext& context) = 0;
};

// ======================================

template<>
class VkR3rSubject<void>
{
public:
	using Observer = VkR3rObserver<void>;

public:
	virtual ~VkR3rSubject() = default;

	virtual void attach(Observer& observer) = 0;
	virtual void detach(Observer& observer) = 0;
	virtual void notify() = 0;
};

// ======================================

class VkR3rPostPresentSubject final : public VkR3rSubject<void>
{
public:
	VkR3rPostPresentSubject();
	~VkR3rPostPresentSubject() override = default;;

	void attach(Observer& observer) override;
	void detach(Observer& observer) override;
	void notify() override;

	using Observers = std::unordered_set<Observer*>;

	Observers observers_{};
	Observers observers_copy_{};
};

#endif // BSTONE_VK_R3R_OBSERVER_INCLUDED
