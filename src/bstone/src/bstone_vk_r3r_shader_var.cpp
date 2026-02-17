/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer: Shader variable

#include "bstone_vk_r3r_shader_var.h"
#include "bstone_exception.h"
#include "bstone_r3r_limits.h"
#include <cstddef>
#include <cstdint>
#include <algorithm>

namespace bstone {

namespace {

class VkR3rShaderVarImpl final : public R3rShaderVar
{
public:
	VkR3rShaderVarImpl(
		R3rShaderVarType type,
		R3rShaderVarTypeId type_id,
		int index,
		const char* name,
		void* mapped_memory);
	~VkR3rShaderVarImpl() override {}

	R3rShaderVarType get_type() const override;
	R3rShaderVarTypeId get_type_id() const override;
	int get_index() const override;
	const std::string& get_name() const override;

	void set_int32(std::int32_t value) override;
	void set_float32(float value) override;
	void set_vec2(const float* value) override;
	void set_vec3(const float* value) override;
	void set_vec4(const float* value) override;
	void set_mat4(const float* value) override;
	void set_r2_sampler(std::int32_t value) override;

private:
	R3rShaderVarType type_{};
	R3rShaderVarTypeId type_id_{};
	int index_{};
	std::string name_{};
	void* mapped_memory_{};

	void ensure_is_not_vertex_attribute() const;
};

// --------------------------------------

VkR3rShaderVarImpl::VkR3rShaderVarImpl(
	R3rShaderVarType type,
	R3rShaderVarTypeId type_id,
	int index,
	const char* name,
	void* mapped_memory)
	:
	type_{type},
	type_id_{type_id},
	index_{index},
	name_{name},
	mapped_memory_{mapped_memory}
{}

R3rShaderVarType VkR3rShaderVarImpl::get_type() const
{
	return type_;
}

R3rShaderVarTypeId VkR3rShaderVarImpl::get_type_id() const
{
	return type_id_;
}

int VkR3rShaderVarImpl::get_index() const
{
	return index_;
}

const std::string& VkR3rShaderVarImpl::get_name() const
{
	return name_;
}

void VkR3rShaderVarImpl::set_int32(std::int32_t value)
{
	ensure_is_not_vertex_attribute();
	*static_cast<std::int32_t*>(mapped_memory_) = value;
}

void VkR3rShaderVarImpl::set_float32(float value)
{
	ensure_is_not_vertex_attribute();
	*static_cast<float*>(mapped_memory_) = value;
}

void VkR3rShaderVarImpl::set_vec2(const float* value)
{
	ensure_is_not_vertex_attribute();
	std::copy_n(value, 2, static_cast<float*>(mapped_memory_));
}

void VkR3rShaderVarImpl::set_vec3([[maybe_unused]] const float* value)
try {
	ensure_is_not_vertex_attribute();
	BSTONE_THROW_STATIC_SOURCE("Unsupported shader var type id.");
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void VkR3rShaderVarImpl::set_vec4(const float* value)
{
	ensure_is_not_vertex_attribute();
	std::copy_n(value, 4, static_cast<float*>(mapped_memory_));
}

void VkR3rShaderVarImpl::set_mat4(const float* value)
{
	ensure_is_not_vertex_attribute();
	std::copy_n(value, 16, static_cast<float*>(mapped_memory_));
}

void VkR3rShaderVarImpl::set_r2_sampler([[maybe_unused]] std::int32_t value)
{
	ensure_is_not_vertex_attribute();
}

void VkR3rShaderVarImpl::ensure_is_not_vertex_attribute() const
try {
	if (type_ == R3rShaderVarType::attribute)
	{
		BSTONE_THROW_STATIC_SOURCE("Updating an attribute not supported.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace

// ======================================

R3rShaderVarUPtr make_vk_r3r_shader_var(
	R3rShaderVarType type,
	R3rShaderVarTypeId type_id,
	int index,
	const char* name,
	void* mapped_memory)
{
	return R3rShaderVarUPtr{new VkR3rShaderVarImpl(type, type_id, index, name, mapped_memory)};
}

} // namespace bstone
