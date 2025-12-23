/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer: Vertex input

#include "bstone_vk_r3r_vertex_input.h"
#include "bstone_exception.h"
#include "bstone_fixed_pool_resource.h"
#include "bstone_r3r_limits.h"
#include "bstone_vk_r3r_buffer.h"
#include "bstone_vk_r3r_context.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

namespace bstone {

namespace {

class VkR3rVertexInputImpl final : public VkR3rVertexInput
{
public:
	VkR3rVertexInputImpl(VkR3rContext& context, const R3rCreateVertexInputParam& param);
	~VkR3rVertexInputImpl() override {}

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	VkR3rBuffer* do_get_index_buffer() const override;
	VkR3rBuffer* do_get_vertex_buffer() const override;
	VkBuffer do_get_vk_generic_buffer() const override;
	const VkPipelineVertexInputStateCreateInfo& do_get_vk_create_info() const override;

	constexpr static std::uint32_t default_value_size = sizeof(R3rVec4);

	using MemoryPool = FixedPoolResource<VkR3rVertexInputImpl, R3rLimits::max_vertex_inputs()>;
	using VkAttributeDescriptions = std::vector<VkVertexInputAttributeDescription>;
	using VkVertexInputBindingDescriptions = std::vector<VkVertexInputBindingDescription>;
	using DefaultValues = std::vector<R3rVec4>;

	static MemoryPool memory_pool_;
	VkR3rContext& context_;
	VkR3rBuffer* index_buffer_{};
	VkR3rBuffer* vertex_buffer_{};
	VkVertexInputBindingDescriptions vk_vertex_input_binding_descriptions_;
	VkAttributeDescriptions vk_attribute_descriptions_{};
	VkPipelineVertexInputStateCreateInfo vk_pipeline_vertex_input_state_create_info_;
	VkR3rDeviceMemoryResource generic_buffer_memory_resource_{};
	VkR3rBufferResource generic_buffer_resource_{};

	void initialize_generic_buffer(const DefaultValues& default_values);
};

// --------------------------------------

VkR3rVertexInputImpl::MemoryPool VkR3rVertexInputImpl::memory_pool_{};

// --------------------------------------

VkR3rVertexInputImpl::VkR3rVertexInputImpl(VkR3rContext& context, const R3rCreateVertexInputParam& param)
try
	:
	context_{context}
{
	if (param.index_buffer == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("No index buffer.");
	}
	if (param.attrib_descrs.empty())
	{
		BSTONE_THROW_STATIC_SOURCE("No attribute descriptions.");
	}
	if (param.attrib_descrs.size() > R3rLimits::max_vertex_attributes() ||
		param.attrib_descrs.size() > context_.physical_device_properties.limits.maxVertexInputAttributes)
	{
		BSTONE_THROW_STATIC_SOURCE("Too many descriptions.");
	}
	std::uint32_t vk_generic_offset = 0;
	std::uint32_t last_stride = UINT32_MAX;
	R3rBuffer* last_vertex_buffer = nullptr;
	DefaultValues default_values{};
	default_values.reserve(param.attrib_descrs.size());
	vk_vertex_input_binding_descriptions_.reserve(2);
	vk_attribute_descriptions_.reserve(param.attrib_descrs.size());
	for (const R3rVertexAttribDescr& descr : param.attrib_descrs)
	{
		if (descr.location < 0)
		{
			BSTONE_THROW_STATIC_SOURCE("Location out of range.");
		}
		if (descr.is_default)
		{
			vk_attribute_descriptions_.emplace_back(
				VkVertexInputAttributeDescription
				{
					/* location */ static_cast<std::uint32_t>(descr.location),
					/* binding */  1,
					/* format */   VK_FORMAT_R32G32B32A32_SFLOAT,
					/* offset */   vk_generic_offset,
				}
			);
			default_values.emplace_back(descr.default_value);
			vk_generic_offset += default_value_size;
			continue;
		}
		VkFormat vk_format;
		switch (descr.format)
		{
			case R3rVertexAttribFormat::rgba_8_unorm:
				vk_format = VK_FORMAT_R8G8B8A8_UNORM;
				break;
			case R3rVertexAttribFormat::rg_32_sfloat:
				vk_format = VK_FORMAT_R32G32_SFLOAT;
				break;
			case R3rVertexAttribFormat::rgb_32_sfloat:
				vk_format = VK_FORMAT_R32G32B32_SFLOAT;
				break;
			default:
				BSTONE_THROW_STATIC_SOURCE("Unknown attribute format.");
		}
		if (descr.vertex_buffer == nullptr)
		{
			BSTONE_THROW_STATIC_SOURCE("No vertex buffer.");
		}
		if (last_stride != UINT32_MAX && static_cast<std::uint32_t>(descr.stride) != last_stride)
		{
			BSTONE_THROW_STATIC_SOURCE("Expected same stride.");
		}
		last_stride = descr.stride;
		if (last_vertex_buffer != nullptr && descr.vertex_buffer != last_vertex_buffer)
		{
			BSTONE_THROW_STATIC_SOURCE("Expected same vertex buffer.");
		}
		last_vertex_buffer = descr.vertex_buffer;
		if (descr.offset < 0)
		{
			BSTONE_THROW_STATIC_SOURCE("Offset out of range.");
		}
		if (descr.stride <= 0)
		{
			BSTONE_THROW_STATIC_SOURCE("Stride out of range.");
		}
		vk_attribute_descriptions_.emplace_back(
			VkVertexInputAttributeDescription
			{
				/* location */ static_cast<std::uint32_t>(descr.location),
				/* binding */  0,
				/* format */   vk_format,
				/* offset */   static_cast<std::uint32_t>(descr.offset),
			});
	}
	index_buffer_ = static_cast<VkR3rBuffer*>(param.index_buffer);
	vertex_buffer_ = static_cast<VkR3rBuffer*>(last_vertex_buffer);
	vk_vertex_input_binding_descriptions_.emplace_back(
		VkVertexInputBindingDescription
		{
			/* binding */   0,
			/* stride */    last_stride,
			/* inputRate */ VK_VERTEX_INPUT_RATE_VERTEX,
		}
	);
	if (vk_generic_offset > 0)
	{
		vk_vertex_input_binding_descriptions_.emplace_back(
			VkVertexInputBindingDescription
			{
				/* binding */   1,
				/* stride */    0,
				/* inputRate */ VK_VERTEX_INPUT_RATE_VERTEX,
			}
		);
		initialize_generic_buffer(default_values);
	}
	vk_pipeline_vertex_input_state_create_info_ = VkPipelineVertexInputStateCreateInfo
	{
		/* sType */                           VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		/* pNext */                           nullptr,
		/* flags */                           VkPipelineVertexInputStateCreateFlags{},
		/* vertexBindingDescriptionCount */   static_cast<std::uint32_t>(vk_vertex_input_binding_descriptions_.size()),
		/* pVertexBindingDescriptions */      vk_vertex_input_binding_descriptions_.data(),
		/* vertexAttributeDescriptionCount */ static_cast<std::uint32_t>(vk_attribute_descriptions_.size()),
		/* pVertexAttributeDescriptions */    vk_attribute_descriptions_.data(),
	};
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void* VkR3rVertexInputImpl::operator new(std::size_t size)
try {
	return memory_pool_.allocate(static_cast<std::intptr_t>(size));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void VkR3rVertexInputImpl::operator delete(void* ptr)
{
	memory_pool_.deallocate(ptr);
}

VkR3rBuffer* VkR3rVertexInputImpl::do_get_index_buffer() const
{
	return index_buffer_;
}

VkR3rBuffer* VkR3rVertexInputImpl::do_get_vertex_buffer() const
{
	return vertex_buffer_;
}

VkBuffer VkR3rVertexInputImpl::do_get_vk_generic_buffer() const
{
	return generic_buffer_resource_.get();
}

const VkPipelineVertexInputStateCreateInfo& VkR3rVertexInputImpl::do_get_vk_create_info() const
{
	return vk_pipeline_vertex_input_state_create_info_;
}

void VkR3rVertexInputImpl::initialize_generic_buffer(const DefaultValues& default_values)
{
	const std::size_t resource_size = default_values.size() * default_value_size;
	context_.create_buffer_resource(
		resource_size,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		generic_buffer_resource_,
		generic_buffer_memory_resource_);
	void* const mapped_memory = context_.map_memory(generic_buffer_memory_resource_.get());
	memcpy(mapped_memory, default_values.data(), resource_size);
	context_.unmap_memory(generic_buffer_memory_resource_.get());
}

} // namespace

// ======================================

VkR3rBuffer* VkR3rVertexInput::get_index_buffer() const
{
	return do_get_index_buffer();
}

VkR3rBuffer* VkR3rVertexInput::get_vertex_buffer() const
{
	return do_get_vertex_buffer();
}

VkBuffer VkR3rVertexInput::get_vk_generic_buffer() const
{
	return do_get_vk_generic_buffer();
}

const VkPipelineVertexInputStateCreateInfo& VkR3rVertexInput::get_vk_create_info() const
{
	return do_get_vk_create_info();
}

// ======================================

VkR3rVertexInputUPtr make_vk_r3r_vertex_input(VkR3rContext& context, const R3rCreateVertexInputParam& param)
{
	return std::make_unique<VkR3rVertexInputImpl>(context, param);
}

} // namespace bstone
