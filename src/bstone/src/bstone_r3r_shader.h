/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D Renderer: Shader

#ifndef BSTONE_R3R_SHADER_INCLUDED
#define BSTONE_R3R_SHADER_INCLUDED

#include <memory>

namespace bstone {

enum class R3rShaderType
{
	none = 0,
	fragment,
	vertex,
};

struct R3rShaderSource
{
	const void* data;
	int size;
};

// ==========================================================================

struct R3rShaderInitParam
{
	R3rShaderType type;
	R3rShaderSource source;
};

// ==========================================================================

class R3rShader
{
public:
	R3rShader() noexcept;
	virtual ~R3rShader();

	R3rShaderType get_type() const noexcept;

private:
	virtual R3rShaderType do_get_type() const noexcept = 0;
};

// ==========================================================================

using R3rShaderUPtr = std::unique_ptr<R3rShader>;

} // namespace bstone

#endif // BSTONE_R3R_SHADER_INCLUDED
