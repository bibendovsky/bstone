/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Null 3D renderer: Shader stage

#include "bstone_null_r3r_shader_stage.h"
#include <stddef.h>
#include <stdint.h>
#include <utility>
#include <vector>
#include "bstone_exception.h"
#include "bstone_fixed_pool_resource.h"
#include "bstone_string_view.h"
#include "bstone_utility.h"
#include "bstone_r3r_limits.h"
#include "bstone_null_r3r_shader_var.h"

// ==========================================================================

namespace bstone {

namespace {

class NullR3rShaderStageImpl final : public R3rShaderStage
{
public:
	NullR3rShaderStageImpl(const R3rShaderStageInitParam& param);
	~NullR3rShaderStageImpl() override {}

	void* operator new(size_t size);
	void operator delete(void* ptr);

private:
	R3rShaderVar* do_find_var(const char* name) override;
	R3rShaderVar* do_find_int32_var(const char* name) override;
	R3rShaderVar* do_find_float32_var(const char* name) override;
	R3rShaderVar* do_find_vec2_var(const char* name) override;
	R3rShaderVar* do_find_vec3_var(const char* name) override;
	R3rShaderVar* do_find_vec4_var(const char* name) override;
	R3rShaderVar* do_find_mat4_var(const char* name) override;
	R3rShaderVar* do_find_r2_sampler_var(const char* name) override;

private:
	using MemoryPool = FixedPoolResource<NullR3rShaderStageImpl, R3rLimits::max_shader_stages()>;
	using ShaderVars = std::vector<NullR3rShaderVarUPtr>;

private:
	static MemoryPool memory_pool_;

private:
	ShaderVars shader_vars_{make_shader_vars()};

private:
	static ShaderVars make_shader_vars();
	R3rShaderVar* impl_find_var(const char* name, R3rShaderVarTypeId shader_var_type_id);
	static R3rShaderVarType get_type_from_name(StringView name);
};

// --------------------------------------------------------------------------

NullR3rShaderStageImpl::MemoryPool NullR3rShaderStageImpl::memory_pool_{};

// --------------------------------------------------------------------------

NullR3rShaderStageImpl::NullR3rShaderStageImpl(const R3rShaderStageInitParam& param)
{
	maybe_unused(param);
}

void* NullR3rShaderStageImpl::operator new(size_t size)
try {
	return memory_pool_.allocate(static_cast<intptr_t>(size));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void NullR3rShaderStageImpl::operator delete(void* ptr)
{
	memory_pool_.deallocate(ptr);
}

R3rShaderVar* NullR3rShaderStageImpl::do_find_var(const char* name)
{
	return impl_find_var(name, R3rShaderVarTypeId::none);
}

R3rShaderVar* NullR3rShaderStageImpl::do_find_int32_var(const char* name)
{
	return impl_find_var(name, R3rShaderVarTypeId::int32);
}

R3rShaderVar* NullR3rShaderStageImpl::do_find_float32_var(const char* name)
{
	return impl_find_var(name, R3rShaderVarTypeId::float32);
}

R3rShaderVar* NullR3rShaderStageImpl::do_find_vec2_var(const char* name)
{
	return impl_find_var(name, R3rShaderVarTypeId::vec2);
}

R3rShaderVar* NullR3rShaderStageImpl::do_find_vec3_var(const char* name)
{
	return impl_find_var(name, R3rShaderVarTypeId::vec3);
}

R3rShaderVar* NullR3rShaderStageImpl::do_find_vec4_var(const char* name)
{
	return impl_find_var(name, R3rShaderVarTypeId::vec4);
}

R3rShaderVar* NullR3rShaderStageImpl::do_find_mat4_var(const char* name)
{
	return impl_find_var(name, R3rShaderVarTypeId::mat4);
}

R3rShaderVar* NullR3rShaderStageImpl::do_find_r2_sampler_var(const char* name)
{
	return impl_find_var(name, R3rShaderVarTypeId::sampler2d);
}

auto NullR3rShaderStageImpl::make_shader_vars() -> ShaderVars
{
	ShaderVars shader_vars{};
	shader_vars.reserve(R3rLimits::max_shader_vars());
	return shader_vars;
}

R3rShaderVar* NullR3rShaderStageImpl::impl_find_var(const char* name, R3rShaderVarTypeId shader_var_type_id)
{
	bool is_name_matched = false;
	bool is_any_type_id = shader_var_type_id == R3rShaderVarTypeId::none;

	for (NullR3rShaderVarUPtr& shader_var_uptr : shader_vars_)
	{
		if (shader_var_uptr->get_name() != name)
		{
			continue;
		}

		is_name_matched = true;

		if (!is_any_type_id && shader_var_uptr->get_type_id() != shader_var_type_id)
		{
			continue;
		}

		return shader_var_uptr.get();
	}

	if (is_name_matched || is_any_type_id)
	{
		return nullptr;
	}

	if (shader_vars_.size() == shader_vars_.capacity())
	{
		return nullptr;
	}

	const R3rShaderVarType type = get_type_from_name(name);
	const int index = static_cast<int>(shader_vars_.size());
	NullR3rShaderVarUPtr shader_var_uptr = make_null_r3r_shader_var(type, shader_var_type_id, index, name);
	shader_vars_.emplace_back(std::move(shader_var_uptr));
	return shader_vars_.back().get();
}

R3rShaderVarType NullR3rShaderStageImpl::get_type_from_name(StringView name)
{
	constexpr StringView attribute_prefix_string = "a_";
	constexpr StringView uniform_prefix_string = "u_";
	constexpr StringView sampler_string_string = "sampler";

	if (name.contains(sampler_string_string))
	{
		return R3rShaderVarType::sampler;
	}

	if (name.starts_with(attribute_prefix_string))
	{
		return R3rShaderVarType::attribute;
	}

	if (name.starts_with(uniform_prefix_string))
	{
		return R3rShaderVarType::uniform;
	}

	return R3rShaderVarType::none;
}

} // namespace

// ==========================================================================

NullR3rShaderStageUPtr make_null_r3r_shader_stage(const R3rShaderStageInitParam& param)
{
	return std::make_unique<NullR3rShaderStageImpl>(param);
}

} // namespace bstone
