/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer: Buffer

#include "bstone_vk_r3r_buffer.h"
#include "bstone_assert.h"
#include "bstone_fixed_pool_resource.h"
#include "bstone_utility.h"
#include "bstone_r3r_limits.h"
#include "bstone_vk_r3r_context.h"
#include "bstone_vk_r3r_raii.h"
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace bstone {

namespace {

class VkR3rBufferImpl final : public VkR3rBuffer
{
public:
	VkR3rBufferImpl(VkR3rContext& context, const R3rBufferInitParam& param);
	~VkR3rBufferImpl() override {}

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	R3rBufferType do_get_type() const noexcept override;
	R3rBufferUsageType do_get_usage_type() const noexcept override;
	int do_get_size() const noexcept override;
	void do_update(const R3rUpdateBufferParam& param) override;

	VkBuffer do_get_vk_buffer() const override;

	using MemoryPool = FixedPoolResource<VkR3rBufferImpl, R3rLimits::max_buffers()>;

	static MemoryPool memory_pool_;

	VkR3rContext& context_;
	R3rBufferType type_{};
	R3rBufferUsageType usage_type_{};
	int size_{};
	VkR3rDeviceMemoryResource device_memory_resource_{};
	VkR3rBufferResource buffer_resource_{};
	void* mapped_memory_{};
};

// --------------------------------------

VkR3rBufferImpl::MemoryPool VkR3rBufferImpl::memory_pool_{};

// --------------------------------------

VkR3rBufferImpl::VkR3rBufferImpl(VkR3rContext& context, const R3rBufferInitParam& param)
try :
	context_{context},
	type_{param.type},
	usage_type_{param.usage_type},
	size_{param.size}
{
	VkBufferUsageFlagBits vk_buffer_usage_flag_bits;
	switch (type_)
	{
		case R3rBufferType::index:
			vk_buffer_usage_flag_bits = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			break;
		case R3rBufferType::vertex:
			vk_buffer_usage_flag_bits = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			break;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown buffer type.");
	}
	switch (usage_type_)
	{
		case R3rBufferUsageType::draw_streaming:
		case R3rBufferUsageType::draw_static:
		case R3rBufferUsageType::draw_dynamic:
			break;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown buffer usage type.");
	}
	context_.create_buffer_resource(
		static_cast<VkDeviceSize>(size_),
		vk_buffer_usage_flag_bits,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		buffer_resource_,
		device_memory_resource_);
	mapped_memory_ = context_.map_memory(device_memory_resource_.get());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void* VkR3rBufferImpl::operator new(std::size_t size)
try {
	return memory_pool_.allocate(static_cast<std::intptr_t>(size));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void VkR3rBufferImpl::operator delete(void* ptr)
{
	memory_pool_.deallocate(ptr);
}

R3rBufferType VkR3rBufferImpl::do_get_type() const noexcept
{
	return type_;
}

R3rBufferUsageType VkR3rBufferImpl::do_get_usage_type() const noexcept
{
	return usage_type_;
}

int VkR3rBufferImpl::do_get_size() const noexcept
{
	return size_;
}

void VkR3rBufferImpl::do_update(const R3rUpdateBufferParam& param)
try {
	if (param.offset < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Negative offset.");
	}
	if (param.size <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Negative size.");
	}
	if (param.offset + param.size > size_)
	{
		BSTONE_THROW_STATIC_SOURCE("Range out of bounds.");
	}
	std::memcpy(static_cast<unsigned char*>(mapped_memory_) + param.offset, param.data, param.size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

VkBuffer VkR3rBufferImpl::do_get_vk_buffer() const
{
	return buffer_resource_.get();
}

} // namespace

// ======================================

VkBuffer VkR3rBuffer::get_vk_buffer() const
{
	return do_get_vk_buffer();
}

// ======================================

VkR3rBufferUPtr make_vk_r3r_buffer(VkR3rContext& context, const R3rBufferInitParam& param)
try {
	return std::make_unique<VkR3rBufferImpl>(context, param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone
