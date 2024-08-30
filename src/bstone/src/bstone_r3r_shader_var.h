/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D Renderer: Shader Variable

#ifndef BSTONE_R3R_SHADER_VAR_INCLUDED
#define BSTONE_R3R_SHADER_VAR_INCLUDED

#include <cstdint>
#include <memory>
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

// ==========================================================================

class R3rShaderVar
{
public:
	R3rShaderVar() noexcept;
	virtual ~R3rShaderVar();

	R3rShaderVarType get_type() const noexcept;
	R3rShaderVarTypeId get_type_id() const noexcept;
	int get_index() const noexcept;
	const std::string& get_name() const noexcept;
	int get_input_index() const noexcept;

private:
	virtual R3rShaderVarType do_get_type() const noexcept = 0;
	virtual R3rShaderVarTypeId do_get_type_id() const noexcept = 0;
	virtual int do_get_index() const noexcept = 0;
	virtual const std::string& do_get_name() const noexcept = 0;
	virtual int do_get_input_index() const noexcept = 0;
};

// ==========================================================================

class R3rShaderInt32Var : public virtual R3rShaderVar
{
public:
	R3rShaderInt32Var() noexcept;
	virtual ~R3rShaderInt32Var();

	void set_int32(std::int32_t value);

private:
	virtual void do_set_int32(std::int32_t value) = 0;
};

// ==========================================================================

class R3rShaderFloat32Var : public virtual R3rShaderVar
{
public:
	R3rShaderFloat32Var() noexcept;
	virtual ~R3rShaderFloat32Var();

	void set_float32(float value);

private:
	virtual void do_set_float32(float value) = 0;
};

// ==========================================================================

class R3rShaderVec2Var : public virtual R3rShaderVar
{
public:
	R3rShaderVec2Var() noexcept;
	virtual ~R3rShaderVec2Var();

	void set_vec2(const float* value);

private:
	virtual void do_set_vec2(const float* value) = 0;
};

// ==========================================================================

class R3rShaderVec4Var : public virtual R3rShaderVar
{
public:
	R3rShaderVec4Var() noexcept;
	virtual ~R3rShaderVec4Var();

	void set_vec4(const float* value);

private:
	virtual void do_set_vec4(const float* value) = 0;
};

// ==========================================================================

class R3rShaderMat4Var : public virtual R3rShaderVar
{
public:
	R3rShaderMat4Var() noexcept;
	virtual ~R3rShaderMat4Var();

	void set_mat4(const float* value);

private:
	virtual void do_set_mat4(const float* value) = 0;
};

// ==========================================================================

class R3rShaderR2SamplerVar : public virtual R3rShaderVar
{
public:
	R3rShaderR2SamplerVar() noexcept;
	virtual ~R3rShaderR2SamplerVar();

	void set_r2_sampler(std::int32_t value);

private:
	virtual void do_set_r2_sampler(std::int32_t value) = 0;
};

} // namespace bstone

#endif // BSTONE_R3R_SHADER_VAR_INCLUDED
