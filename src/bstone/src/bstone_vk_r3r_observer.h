/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
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
	virtual ~VkR3rObserver() {}

	void update(TContext& context)
	{
		return do_update(context);
	}

private:
	virtual void do_update(TContext& context) = 0;
};

// ======================================

template<>
class VkR3rObserver<void>
{
public:
	virtual ~VkR3rObserver() {}

	void update()
	{
		return do_update();
	}

private:
	virtual void do_update() = 0;
};

// ======================================

template<typename TContext>
class VkR3rSubject
{
public:
	using Observer = VkR3rObserver<TContext>;

public:
	virtual ~VkR3rSubject() {}

	void attach(Observer& observer)
	{
		do_attach(observer);
	}

	void detach(Observer& observer)
	{
		do_detach(observer);
	}

	void notify(TContext& context)
	{
		do_notify(context);
	}

private:
	virtual void do_attach(Observer& observer) = 0;
	virtual void do_detach(Observer& observer) = 0;
	virtual void do_notify(TContext& context) = 0;
};

// ======================================

template<>
class VkR3rSubject<void>
{
public:
	using Observer = VkR3rObserver<void>;

public:
	virtual ~VkR3rSubject() {}

	void attach(Observer& observer)
	{
		do_attach(observer);
	}

	void detach(Observer& observer)
	{
		do_detach(observer);
	}

	void notify()
	{
		do_notify();
	}

private:
	virtual void do_attach(Observer& observer) = 0;
	virtual void do_detach(Observer& observer) = 0;
	virtual void do_notify() = 0;
};

// ======================================

class VkR3rPostPresentSubject final : public VkR3rSubject<void>
{
public:
	VkR3rPostPresentSubject();
	~VkR3rPostPresentSubject() override {};

	using Observers = std::unordered_set<Observer*>;

	Observers observers_{};
	Observers observers_copy_{};

	void do_attach(Observer& observer) override;
	void do_detach(Observer& observer) override;
	void do_notify() override;
};

#endif // BSTONE_VK_R3R_OBSERVER_INCLUDED
