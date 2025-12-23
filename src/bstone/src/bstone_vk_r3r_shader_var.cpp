/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer: Shader variable

#include "bstone_vk_r3r_shader_var.h"
#include "bstone_exception.h"
#include "bstone_fixed_pool_resource.h"
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

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	R3rShaderVarType do_get_type() const noexcept override;
	R3rShaderVarTypeId do_get_type_id() const noexcept override;
	int do_get_index() const noexcept override;
	const std::string& do_get_name() const noexcept override;

	void do_set_int32(std::int32_t value) override;
	void do_set_float32(float value) override;
	void do_set_vec2(const float* value) override;
	void do_set_vec3(const float* value) override;
	void do_set_vec4(const float* value) override;
	void do_set_mat4(const float* value) override;
	void do_set_r2_sampler(std::int32_t value) override;

	R3rShaderVarType type_{};
	R3rShaderVarTypeId type_id_{};
	int index_{};
	std::string name_{};
	void* mapped_memory_{};

	using MemoryPool = FixedPoolResource<VkR3rShaderVarImpl, R3rLimits::max_shader_vars()>;

	static MemoryPool memory_pool_;

	void ensure_is_not_vertex_attribute() const;
};

// --------------------------------------

VkR3rShaderVarImpl::MemoryPool VkR3rShaderVarImpl::memory_pool_;

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

void* VkR3rShaderVarImpl::operator new(std::size_t size)
try {
	return memory_pool_.allocate(static_cast<std::intptr_t>(size));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void VkR3rShaderVarImpl::operator delete(void* ptr)
{
	memory_pool_.deallocate(ptr);
}

R3rShaderVarType VkR3rShaderVarImpl::do_get_type() const noexcept
{
	return type_;
}

R3rShaderVarTypeId VkR3rShaderVarImpl::do_get_type_id() const noexcept
{
	return type_id_;
}

int VkR3rShaderVarImpl::do_get_index() const noexcept
{
	return index_;
}

const std::string& VkR3rShaderVarImpl::do_get_name() const noexcept
{
	return name_;
}

void VkR3rShaderVarImpl::do_set_int32(std::int32_t value)
{
	ensure_is_not_vertex_attribute();
	*static_cast<std::int32_t*>(mapped_memory_) = value;
}

void VkR3rShaderVarImpl::do_set_float32(float value)
{
	ensure_is_not_vertex_attribute();
	*static_cast<float*>(mapped_memory_) = value;
}

void VkR3rShaderVarImpl::do_set_vec2(const float* value)
{
	ensure_is_not_vertex_attribute();
	std::copy_n(value, 2, static_cast<float*>(mapped_memory_));
}

void VkR3rShaderVarImpl::do_set_vec3([[maybe_unused]] const float* value)
try {
	ensure_is_not_vertex_attribute();
	BSTONE_THROW_STATIC_SOURCE("Unsupported shader var type id.");
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void VkR3rShaderVarImpl::do_set_vec4(const float* value)
{
	ensure_is_not_vertex_attribute();
	std::copy_n(value, 4, static_cast<float*>(mapped_memory_));
}

void VkR3rShaderVarImpl::do_set_mat4(const float* value)
{
	ensure_is_not_vertex_attribute();
	std::copy_n(value, 16, static_cast<float*>(mapped_memory_));
}

void VkR3rShaderVarImpl::do_set_r2_sampler([[maybe_unused]] std::int32_t value)
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
