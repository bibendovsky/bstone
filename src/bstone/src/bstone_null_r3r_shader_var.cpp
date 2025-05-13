/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Null 3D renderer: Shader variable

#include "bstone_null_r3r_shader_var.h"
#include <stddef.h>
#include <stdint.h>
#include "bstone_exception.h"
#include "bstone_fixed_pool_resource.h"
#include "bstone_utility.h"
#include "bstone_r3r_limits.h"

// ==========================================================================

namespace bstone {

namespace {

class NullR3rShaderVarImpl final : public R3rShaderVar
{
public:
	NullR3rShaderVarImpl(
		R3rShaderVarType type,
		R3rShaderVarTypeId type_id,
		int index,
		const char* name);
	~NullR3rShaderVarImpl() override {}

	void* operator new(size_t size);
	void operator delete(void* ptr);

private:
	R3rShaderVarType do_get_type() const override;
	R3rShaderVarTypeId do_get_type_id() const override;
	int do_get_index() const override;
	const std::string& do_get_name() const override;

	void do_set_int32(int32_t value) override;
	void do_set_float32(float value) override;
	void do_set_vec2(const float* value) override;
	void do_set_vec3(const float* value) override;
	void do_set_vec4(const float* value) override;
	void do_set_mat4(const float* value) override;
	void do_set_r2_sampler(int32_t value) override;

private:
	R3rShaderVarType type_{};
	R3rShaderVarTypeId type_id_{};
	int index_{};
	std::string name_{};

private:
	using MemoryPool = FixedPoolResource<NullR3rShaderVarImpl, R3rLimits::max_shader_vars()>;

private:
	static MemoryPool memory_pool_;
};

// --------------------------------------------------------------------------

NullR3rShaderVarImpl::MemoryPool NullR3rShaderVarImpl::memory_pool_;

// --------------------------------------------------------------------------

NullR3rShaderVarImpl::NullR3rShaderVarImpl(
	R3rShaderVarType type,
	R3rShaderVarTypeId type_id,
	int index,
	const char* name)
	:
	type_{type},
	type_id_{type_id},
	index_{index},
	name_{name}
{}

void* NullR3rShaderVarImpl::operator new(size_t size)
try {
	return memory_pool_.allocate(static_cast<intptr_t>(size));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void NullR3rShaderVarImpl::operator delete(void* ptr)
{
	memory_pool_.deallocate(ptr);
}

R3rShaderVarType NullR3rShaderVarImpl::do_get_type() const
{
	return type_;
}

R3rShaderVarTypeId NullR3rShaderVarImpl::do_get_type_id() const
{
	return type_id_;
}

int NullR3rShaderVarImpl::do_get_index() const
{
	return index_;
}

const std::string& NullR3rShaderVarImpl::do_get_name() const
{
	return name_;
}

void NullR3rShaderVarImpl::do_set_int32(int32_t value)
{
	maybe_unused(value);
}

void NullR3rShaderVarImpl::do_set_float32(float value)
{
	maybe_unused(value);
}

void NullR3rShaderVarImpl::do_set_vec2(const float* value)
{
	maybe_unused(value);
}

void NullR3rShaderVarImpl::do_set_vec3(const float* value)
{
	maybe_unused(value);
}

void NullR3rShaderVarImpl::do_set_vec4(const float* value)
{
	maybe_unused(value);
}

void NullR3rShaderVarImpl::do_set_mat4(const float* value)
{
	maybe_unused(value);
}

void NullR3rShaderVarImpl::do_set_r2_sampler(int32_t value)
{
	maybe_unused(value);
}

} // namespace

// ==========================================================================

NullR3rShaderVarUPtr make_null_r3r_shader_var(
	R3rShaderVarType type,
	R3rShaderVarTypeId type_id,
	int index,
	const char* name)
try {
	return NullR3rShaderVarUPtr{new NullR3rShaderVarImpl(type, type_id, index, name)};
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone
