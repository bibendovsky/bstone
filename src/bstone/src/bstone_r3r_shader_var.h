/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D Renderer: Shader Variable

#ifndef BSTONE_R3R_SHADER_VAR_INCLUDED
#define BSTONE_R3R_SHADER_VAR_INCLUDED

#include <stdint.h>
#include <string>

namespace bstone {

enum class R3rShaderVarType
{
	none = 0,
	attribute,
	sampler,
	uniform,
};

enum class R3rShaderVarTypeId
{
	none = 0,
	int32,
	float32,
	vec2,
	vec3,
	vec4,
	mat4,
	sampler2d,
};

enum class R3rShaderVarStage
{
	none = 0,
	fragment,
	vertex,
};

struct R3rShaderVarInfo
{
	const char* name;
	R3rShaderVarStage stage;
	R3rShaderVarType type;
	R3rShaderVarTypeId type_id;
	int binding;
};

// ==========================================================================

class R3rShaderVar
{
public:
	R3rShaderVar() {};
	virtual ~R3rShaderVar() {};

	R3rShaderVarType get_type() const;
	R3rShaderVarTypeId get_type_id() const;
	int get_index() const;
	const std::string& get_name() const;

	void set_int32(int32_t value);
	void set_float32(float value);
	void set_vec2(const float* value);
	void set_vec3(const float* value);
	void set_vec4(const float* value);
	void set_mat4(const float* value);
	void set_r2_sampler(int32_t value);

private:
	virtual R3rShaderVarType do_get_type() const = 0;
	virtual R3rShaderVarTypeId do_get_type_id() const = 0;
	virtual int do_get_index() const = 0;
	virtual const std::string& do_get_name() const = 0;

	virtual void do_set_int32(int32_t value) = 0;
	virtual void do_set_float32(float value) = 0;
	virtual void do_set_vec2(const float* value) = 0;
	virtual void do_set_vec3(const float* value) = 0;
	virtual void do_set_vec4(const float* value) = 0;
	virtual void do_set_mat4(const float* value) = 0;
	virtual void do_set_r2_sampler(int32_t value) = 0;
};

} // namespace bstone

#endif // BSTONE_R3R_SHADER_VAR_INCLUDED
