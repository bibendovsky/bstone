/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Null 3D renderer: Shader

#include "bstone_null_r3r_shader.h"
#include <stddef.h>
#include <stdint.h>
#include "bstone_exception.h"
#include "bstone_r3r_limits.h"

// ==========================================================================

namespace bstone {

namespace {

class NullR3rShaderImpl final : public R3rShader
{
public:
	NullR3rShaderImpl(const R3rShaderInitParam& param);
	~NullR3rShaderImpl() override {}

	R3rShaderType get_type() const override;

private:
	R3rShaderType type_{};
};

// --------------------------------------------------------------------------

NullR3rShaderImpl::NullR3rShaderImpl(const R3rShaderInitParam& param)
	:
	type_(param.type)
{}

R3rShaderType NullR3rShaderImpl::get_type() const
{
	return type_;
}

} // namespace

// ==========================================================================

NullR3rShaderUPtr make_null_r3r_shader(const R3rShaderInitParam& param)
{
	return std::make_unique<NullR3rShaderImpl>(param);
}

} // namespace bstone
