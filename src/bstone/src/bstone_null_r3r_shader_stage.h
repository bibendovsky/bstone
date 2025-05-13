/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Null 3D renderer: Shader stage

#ifndef BSTONE_NULL_R3R_SHADER_STAGE_INCLUDED
#define BSTONE_NULL_R3R_SHADER_STAGE_INCLUDED

#include "bstone_r3r_shader_stage.h"

// ==========================================================================

namespace bstone {

using NullR3rShaderStageUPtr = std::unique_ptr<R3rShaderStage>;

NullR3rShaderStageUPtr make_null_r3r_shader_stage(const R3rShaderStageInitParam& param);

} // namespace bstone

#endif // BSTONE_NULL_R3R_SHADER_STAGE_INCLUDED
