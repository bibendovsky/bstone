/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D Renderer: Buffer

#ifndef BSTONE_R3R_BUFFER_INCLUDED
#define BSTONE_R3R_BUFFER_INCLUDED

#include <memory>

namespace bstone {

enum class R3rBufferType
{
	none = 0,
	index,
	vertex,
};

enum class R3rBufferUsageType
{
	none = 0,

	// Target - drawing; updates - continuous.
	draw_streaming,

	// Target - drawing; updates - seldom.
	draw_static,

	// Target - drawing; updates - often.
	draw_dynamic,
};

// ==========================================================================

struct R3rBufferInitParam
{
	R3rBufferType type;
	R3rBufferUsageType usage_type;
	int size;
};

struct R3rUpdateBufferParam
{
	int offset;
	int size;
	const void* data;
};

// ==========================================================================

class R3rBuffer
{
public:
	R3rBuffer() noexcept;
	virtual ~R3rBuffer();

public:
	R3rBufferType get_type() const noexcept;
	R3rBufferUsageType get_usage_type() const noexcept;

	int get_size() const noexcept;
	void update(const R3rUpdateBufferParam& param);

private:
	virtual R3rBufferType do_get_type() const noexcept = 0;
	virtual R3rBufferUsageType do_get_usage_type() const noexcept = 0;

	virtual int do_get_size() const noexcept = 0;
	virtual void do_update(const R3rUpdateBufferParam& param) = 0;
};

// ==========================================================================

using R3rBufferUPtr = std::unique_ptr<R3rBuffer>;

} // namespace bstone

#endif // BSTONE_R3R_BUFFER_INCLUDED
