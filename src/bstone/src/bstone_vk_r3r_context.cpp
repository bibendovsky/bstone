/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer: API

#include "bstone_vk_r3r_context.h"
#include "bstone_assert.h"
#include "bstone_exception.h"
#include "bstone_utility.h"
#include "bstone_vk_r3r_enum_strings.h"
#include <string>

namespace bstone {

const char* const VkR3rContext::vk_layer_khronos_validation_extension_name = "VK_LAYER_KHRONOS_validation";
const char* const VkR3rContext::vk_ext_debug_utils_extension_name = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

// --------------------------------------

void VkR3rContext::ensure_success_vk_result(VkResult vk_result, const char* message)
{
	if (vk_result == VK_SUCCESS)
	{
		return;
	}
	const std::size_t message_length = std::string::traits_type::length(message);
	const char* const vk_result_string = VkR3rEnumStrings::get_VkResult(vk_result);
	std::string error_message{};
	error_message.reserve(message_length + 128);
	error_message += message;
	error_message += " (code: ";
	if (vk_result_string != nullptr)
	{
		error_message += vk_result_string;
		error_message += " (";
	}
	error_message += std::to_string(vk_result);
	if (vk_result_string != nullptr)
	{
		error_message += ')';
	}
	error_message += ')';
	BSTONE_THROW_DYNAMIC_SOURCE(error_message.c_str());
}

bool VkR3rContext::has_swapchain() const
{
	return !swapchain.is_empty();
}

std::uint32_t VkR3rContext::find_memory_type_index(
	std::uint32_t memory_type_bits,
	VkMemoryPropertyFlags memory_property_flags) const
{
	for (std::uint32_t i_memory_type = 0; i_memory_type < memory_properties.memoryTypeCount; ++i_memory_type)
	{
		const VkMemoryType& memory_type = memory_properties.memoryTypes[i_memory_type];
		if ((memory_type_bits & (1U << i_memory_type)) != 0 &&
			(memory_type.propertyFlags & memory_property_flags) == memory_property_flags)
		{
			return i_memory_type;
		}
	}
	BSTONE_THROW_STATIC_SOURCE("Required memory type not found.");
}

VkR3rCommandPoolResource VkR3rContext::create_command_pool(VkCommandPoolCreateFlags vk_flags) const
{
	const VkCommandPoolCreateInfo vk_command_pool_create_info
	{
		/* sType */            VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		/* pNext */            nullptr,
		/* flags */            vk_flags,
		/* queueFamilyIndex */ queue_family_index,
	};
	VkCommandPool vk_command_pool{};
	const VkResult vk_result = vkCreateCommandPool(
		/* device */       device.get(),
		/* pCreateInfo */  &vk_command_pool_create_info,
		/* pAllocator */   nullptr,
		/* pCommandPool */ &vk_command_pool
	);
	ensure_success_vk_result(vk_result, "vkCreateCommandPool");
	return VkR3rCommandPoolResource{vk_command_pool, VkR3rCommandPoolDeleter{*this}};
}

VkR3rCommandBufferResource VkR3rContext::create_command_buffer_resource(VkCommandPool vk_command_pool) const
{
	const VkCommandBufferAllocateInfo vk_command_buffer_allocate_info
	{
		/* sType */              VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		/* pNext */              nullptr,
		/* commandPool */        vk_command_pool,
		/* level */              VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		/* commandBufferCount */ 1,
	};
	VkCommandBuffer vk_command_buffer{};
	const VkResult vk_result = vkAllocateCommandBuffers(
		/* device */ device.get(),
		/* pAllocateInfo */ &vk_command_buffer_allocate_info,
		/* pCommandBuffers */ &vk_command_buffer
	);
	ensure_success_vk_result(vk_result, "vkAllocateCommandBuffers");
	return VkR3rCommandBufferResource{vk_command_buffer, VkR3rCommandBufferDeleter{*this, vk_command_pool}};
}

VkR3rCommandBufferResource VkR3rContext::cmd_begin_single_time_commands() const
{
	VkR3rCommandBufferResource command_buffer_resource = create_command_buffer_resource(transient_command_pool.get());
	const VkCommandBufferBeginInfo vk_command_buffer_begin_info
	{
		/* sType */            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		/* pNext */            nullptr,
		/* flags */            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		/* pInheritanceInfo */ nullptr,
	};
	const VkResult vk_result = vkBeginCommandBuffer(
		/* commandBuffer */ command_buffer_resource.get(),
		/* pBeginInfo */    &vk_command_buffer_begin_info
	);
	ensure_success_vk_result(vk_result, "vkBeginCommandBuffer");
	return command_buffer_resource;
}

void VkR3rContext::cmd_end_single_time_commands(VkCommandBuffer vk_command_buffer) const
{
	VkResult vk_result;
	// Stop recording the command buffer.
	vk_result = vkEndCommandBuffer(
		/* commandBuffer */ vk_command_buffer
	);
	ensure_success_vk_result(vk_result, "vkEndCommandBuffer");
	// Submit the command buffer.
	const VkSubmitInfo vk_submit_info
	{
		/* sType */                VK_STRUCTURE_TYPE_SUBMIT_INFO,
		/* pNext */                nullptr,
		/* waitSemaphoreCount */   0,
		/* pWaitSemaphores */      nullptr,
		/* pWaitDstStageMask */    nullptr,
		/* commandBufferCount */   1,
		/* pCommandBuffers */      &vk_command_buffer,
		/* signalSemaphoreCount */ 0,
		/* pSignalSemaphores */    nullptr
	};
	vk_result = vkQueueSubmit(
		/* queue */       queue,
		/* submitCount */ 1,
		/* pSubmits */    &vk_submit_info,
		/* fence */       VkFence{}
	);
	ensure_success_vk_result(vk_result, "vkQueueSubmit");
	// Wait for the queue.
	vk_result = vkQueueWaitIdle(
		/* queue */ queue
	);
	ensure_success_vk_result(vk_result, "vkQueueWaitIdle");
}

VkR3rDeviceMemoryResource VkR3rContext::allocate_device_memory_resource(
	VkDeviceSize vk_allocation_size,
	std::uint32_t memory_type_index) const
{
	const VkMemoryAllocateInfo vk_memory_allocate_info
	{
		/* sType */           VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		/* pNext */           nullptr,
		/* allocationSize */  vk_allocation_size,
		/* memoryTypeIndex */ memory_type_index,
	};
	VkDeviceMemory vk_device_memory{};
	const VkResult vk_result = vkAllocateMemory(
		/* device        */ device.get(),
		/* pAllocateInfo */ &vk_memory_allocate_info,
		/* pAllocator    */ nullptr,
		/* pMemory       */ &vk_device_memory
	);
	ensure_success_vk_result(vk_result, "vkAllocateMemory");
	return VkR3rDeviceMemoryResource{vk_device_memory, VkR3rDeviceMemoryDeleter{*this}};;
}

void* VkR3rContext::map_memory(VkDeviceMemory vk_device_memory) const
{
	void* mapped_memory;
	const VkResult vk_result = vkMapMemory(
		/* device */ device.get(),
		/* memory */ vk_device_memory,
		/* offset */ 0,
		/* size   */ VK_WHOLE_SIZE,
		/* flags  */ 0,
		/* ppData */ &mapped_memory
	);
	ensure_success_vk_result(vk_result, "vkMapMemory");
	return mapped_memory;
}

void VkR3rContext::unmap_memory(VkDeviceMemory vk_device_memory) const
{
	vkUnmapMemory(device.get(), vk_device_memory);
}

void VkR3rContext::create_buffer_resource(
	VkDeviceSize vk_size,
	VkBufferUsageFlags vk_buffer_usage_flags,
	VkMemoryPropertyFlags vk_memory_property_flags,
	VkR3rBufferResource& buffer_resource,
	VkR3rDeviceMemoryResource& device_memory_resource) const
try {
	VkResult vk_result;
	// Create the buffer.
	const VkBufferCreateInfo vk_buffer_create_info
	{
		/* sType */                 VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		/* pNext */                 nullptr,
		/* flags */                 0,
		/* size */                  vk_size,
		/* usage */                 vk_buffer_usage_flags,
		/* sharingMode */           VK_SHARING_MODE_EXCLUSIVE,
		/* queueFamilyIndexCount */ 0,
		/* pQueueFamilyIndices */   nullptr
	};
	VkBuffer vk_buffer{};
	vk_result = vkCreateBuffer(
		/* device      */ device.get(),
		/* pCreateInfo */ &vk_buffer_create_info,
		/* pAllocator  */ nullptr,
		/* pBuffer     */ &vk_buffer
	);
	VkR3rContext::ensure_success_vk_result(vk_result, "vkCreateBuffer");
	VkR3rBufferResource local_buffer_resource{vk_buffer, VkR3rBufferDeleter{*this}};
	// Get memory requirements.
	VkMemoryRequirements vk_memory_requirements{};
	vkGetBufferMemoryRequirements(
		/* device              */ device.get(),
		/* buffer              */ vk_buffer,
		/* pMemoryRequirements */ &vk_memory_requirements
	);
	// Allocate memory.
	const std::uint32_t memory_type_index = find_memory_type_index(
		vk_memory_requirements.memoryTypeBits, vk_memory_property_flags);
	VkR3rDeviceMemoryResource local_device_memory_resource = allocate_device_memory_resource(
		vk_memory_requirements.size, memory_type_index);
	// Bind the buffer to the allocated memory.
	vk_result = vkBindBufferMemory(
		/* device       */ device.get(),
		/* buffer       */ vk_buffer,
		/* memory       */ local_device_memory_resource.get(),
		/* memoryOffset */ 0
	);
	VkR3rContext::ensure_success_vk_result(vk_result, "vkBindBufferMemory");
	// Return resources.
	buffer_resource.swap(local_buffer_resource);
	device_memory_resource.swap(local_device_memory_resource);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void VkR3rContext::cmd_copy_buffer(
	VkCommandBuffer vk_command_buffer,
	VkBuffer vk_src_buffer,
	VkBuffer vk_dst_buffer,
	VkDeviceSize vk_size) const
{
	const VkBufferCopy vk_buffer_copy
	{
		/* srcOffset */ 0,
		/* dstOffset */ 0,
		/* size */      vk_size,
	};
	vkCmdCopyBuffer(
		/* commandBuffer */ vk_command_buffer,
		/* srcBuffer     */ vk_src_buffer,
		/* dstBuffer     */ vk_dst_buffer,
		/* regionCount   */ 1,
		/* pRegions      */ &vk_buffer_copy
	);
}

void VkR3rContext::create_image_resource(
	VkFormat vk_format,
	std::uint32_t vk_width,
	std::uint32_t vk_height,
	std::uint32_t vk_mip_levels,
	VkSampleCountFlagBits vk_sample_count,
	VkImageTiling vk_image_tiling,
	VkImageUsageFlags vk_image_usage_flags,
	VkMemoryPropertyFlags vk_memory_property_flags,
	VkR3rImageResource& image_resource,
	VkR3rDeviceMemoryResource& device_memory_resource) const
try {
	VkResult vk_result;
	// Create an image.
	const VkImageCreateInfo vk_image_create_info
	{
		/* sType */                 VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		/* pNext */                 nullptr,
		/* flags */                 0,
		/* imageType */             VK_IMAGE_TYPE_2D,
		/* format */                vk_format,
		/* extent */                VkExtent3D
		                            {
		                                /* width */  vk_width,
		                                /* height */ vk_height,
		                                /* depth */  1,
		                            },
		/* mipLevels */             vk_mip_levels,
		/* arrayLayers */           1,
		/* samples */               vk_sample_count,
		/* tiling */                vk_image_tiling,
		/* usage */                 vk_image_usage_flags,
		/* sharingMode */           VK_SHARING_MODE_EXCLUSIVE,
		/* queueFamilyIndexCount */ 0,
		/* pQueueFamilyIndices */   nullptr,
		/* initialLayout */         VK_IMAGE_LAYOUT_UNDEFINED,
	};
	VkImage vk_image{};
	vk_result = vkCreateImage(
		/* device */ device.get(),
		/* pCreateInfo */ &vk_image_create_info,
		/* pAllocator */ nullptr,
		/* pImage */ &vk_image
	);
	ensure_success_vk_result(vk_result, "vkCreateImage");
	VkR3rImageResource local_image_resource{vk_image, VkR3rImageDeleter{*this}};
	// Get it's memory requirements.
	VkMemoryRequirements vk_memory_requirements{};
	vkGetImageMemoryRequirements(
		/* device              */ device.get(),
		/* image               */ vk_image,
		/* pMemoryRequirements */ &vk_memory_requirements
	);
	const std::uint32_t memory_type_index = find_memory_type_index(
		vk_memory_requirements.memoryTypeBits, vk_memory_property_flags);
	// Allocate memory.
	VkR3rDeviceMemoryResource local_device_memory_resource = allocate_device_memory_resource(
		vk_memory_requirements.size, memory_type_index);
	// Bind the memory to the image.
	vk_result = vkBindImageMemory(
		/* device       */ device.get(),
		/* image        */ vk_image,
		/* memory       */ local_device_memory_resource.get(),
		/* memoryOffset */ 0
	);
	ensure_success_vk_result(vk_result, "vkBindImageMemory");
	//
	image_resource.swap(local_image_resource);
	device_memory_resource.swap(local_device_memory_resource);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

VkR3rImageViewResource VkR3rContext::create_image_view_resource(
	VkImage vk_image,
	VkFormat vk_format,
	VkImageAspectFlags vk_image_aspect_flags) const
{
	const VkImageViewCreateInfo vk_image_view_create_info
	{
		/* sType */            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		/* pNext */            nullptr,
		/* flags */            0,
		/* image */            vk_image,
		/* viewType */         VK_IMAGE_VIEW_TYPE_2D,
		/* format */           vk_format,
		/* components */       VkComponentMapping
		                       {
		                           /* r */ VK_COMPONENT_SWIZZLE_IDENTITY,
		                           /* g */ VK_COMPONENT_SWIZZLE_IDENTITY,
		                           /* b */ VK_COMPONENT_SWIZZLE_IDENTITY,
		                           /* a */ VK_COMPONENT_SWIZZLE_IDENTITY,
		},
		/* subresourceRange */ VkImageSubresourceRange
		                       {
		                           /* aspectMask     */ vk_image_aspect_flags,
		                           /* baseMipLevel   */ 0,
		                           /* levelCount     */ 1,
		                           /* baseArrayLayer */ 0,
		                           /* layerCount     */ 1,
		                       },
	};
	VkImageView vk_image_view{};
	const VkResult vk_result = vkCreateImageView(
		/* device */      device.get(),
		/* pCreateInfo */ &vk_image_view_create_info,
		/* pAllocator */  nullptr,
		/* pView */       &vk_image_view
	);
	ensure_success_vk_result(vk_result, "vkCreateImageView");
	return VkR3rImageViewResource{vk_image_view, VkR3rImageViewDeleter{*this}};
}

void VkR3rContext::cmd_copy_buffer_to_image(
	VkCommandBuffer vk_command_buffer,
	VkBuffer vk_buffer,
	VkImage vk_image,
	std::uint32_t vk_width,
	std::uint32_t vk_height,
	std::uint32_t vk_mip_level) const
{
	const VkBufferImageCopy vk_buffer_image_copy
	{
		/* bufferOffset */      0,
		/* bufferRowLength */   0,
		/* bufferImageHeight */ 0,
		/* imageSubresource */  VkImageSubresourceLayers
		                        {
		                            /* aspectMask */ VK_IMAGE_ASPECT_COLOR_BIT,
		                            /* mipLevel */ vk_mip_level,
		                            /* baseArrayLayer */ 0,
		                            /* layerCount */ 1,
		                        },
		/* imageOffset */       VkOffset3D
		                        {
		                            /* x */ 0,
		                            /* y */ 0,
		                            /* z */ 0,
		                        },
		/* imageExtent */       VkExtent3D
		                        {
		                            /* width */ vk_width,
		                            /* height */ vk_height,
		                            /* depth */ 1,
		                        },
	};
	vkCmdCopyBufferToImage(
		/* commandBuffer */  vk_command_buffer,
		/* srcBuffer */      vk_buffer,
		/* dstImage */       vk_image,
		/* dstImageLayout */ VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		/* regionCount */    1,
		/* pRegions */       &vk_buffer_image_copy
	);
}

void VkR3rContext::cmd_copy_image_to_buffer(
	VkCommandBuffer vk_command_buffer,
	VkImage vk_image,
	std::uint32_t vk_image_width,
	std::uint32_t vk_image_height,
	VkImageLayout vk_image_layout,
	VkBuffer vk_buffer) const
{
	const VkBufferImageCopy vk_buffer_image_copy{
		/* bufferOffset      */ 0,
		/* bufferRowLength   */ 0,
		/* bufferImageHeight */ 0,
		/* imageSubresource  */ VkImageSubresourceLayers{
									/* aspectMask     */ VK_IMAGE_ASPECT_COLOR_BIT,
									/* mipLevel       */ 0,
									/* baseArrayLayer */ 0,
									/* layerCount     */ 1,
								},
		/* imageOffset       */ VkOffset3D{
									/* x */ 0,
									/* y */ 0,
									/* z */ 0,
								},
		/* imageExtent       */ VkExtent3D{
									/* width  */ vk_image_width,
									/* height */ vk_image_height,
									/* depth  */ 1,
								}
	};
	vkCmdCopyImageToBuffer(
		/* commandBuffer  */ vk_command_buffer,
		/* srcImage       */ vk_image,
		/* srcImageLayout */ vk_image_layout,
		/* dstBuffer      */ vk_buffer,
		/* regionCount    */ 1,
		/* pRegions       */ &vk_buffer_image_copy);
}

void VkR3rContext::cmd_blit_image(
	VkCommandBuffer vk_command_buffer,
	VkImage src_vk_image,
	std::uint32_t src_mip_level,
	std::int32_t src_vk_width,
	std::int32_t src_vk_height,
	VkImageLayout src_vk_image_layout,
	VkImage dst_vk_image,
	std::uint32_t dst_mip_level,
	std::int32_t dst_vk_width,
	std::int32_t dst_vk_height,
	VkImageLayout dst_vk_image_layout,
	VkFilter vk_filter) const
{
	const VkImageBlit vk_image_blit{
		/* srcSubresource */ VkImageSubresourceLayers{
								/* aspectMask */     VK_IMAGE_ASPECT_COLOR_BIT,
								/* mipLevel */       src_mip_level,
								/* baseArrayLayer */ 0,
								/* layerCount */     1
							},
		/* srcOffsets[2] */  {
								VkOffset3D{
									/* x */ 0,
									/* y */ 0,
									/* z */ 0,
								},
								VkOffset3D{
									/* x */ src_vk_width,
									/* y */ src_vk_height,
									/* z */ 1,
								},
							},
		/* dstSubresource */ VkImageSubresourceLayers{
								/* aspectMask */     VK_IMAGE_ASPECT_COLOR_BIT,
								/* mipLevel */       dst_mip_level,
								/* baseArrayLayer */ 0,
								/* layerCount */     1
							},
		/* dstOffsets[2] */  {
								VkOffset3D{
									/* x */ 0,
									/* y */ 0,
									/* z */ 0,
								},
								VkOffset3D{
									/* x */ dst_vk_width,
									/* y */ dst_vk_height,
									/* z */ 1,
								},
							}
	};
	vkCmdBlitImage(
		/* commandBuffer */  vk_command_buffer,
		/* srcImage */       src_vk_image,
		/* srcImageLayout */ src_vk_image_layout,
		/* dstImage */       dst_vk_image,
		/* dstImageLayout */ dst_vk_image_layout,
		/* regionCount */    1,
		/* pRegions */       &vk_image_blit,
		/* filter */         vk_filter);
}

void VkR3rContext::cmd_image_memory_barrier(
	VkCommandBuffer vk_command_buffer,
	VkImage vk_image,
	VkImageLayout vk_old_layout,
	VkImageLayout vk_new_layout,
	std::uint32_t vk_base_mip_level,
	std::uint32_t vk_mip_level_count,
	VkPipelineStageFlags vk_src_stage_flags,
	VkPipelineStageFlags vk_dst_stage_flags) const
{
	BSTONE_ASSERT(vk_old_layout != vk_new_layout);
	BSTONE_ASSERT(vk_mip_level_count > 0);
	const VkImageMemoryBarrier vk_image_memory_barrier
	{
		/* sType */               VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		/* pNext */               nullptr,
		/* srcAccessMask */       VK_ACCESS_NONE,
		/* dstAccessMask */       VK_ACCESS_NONE,
		/* oldLayout */           vk_old_layout,
		/* newLayout */           vk_new_layout,
		/* srcQueueFamilyIndex */ 0,
		/* dstQueueFamilyIndex */ 0,
		/* image */               vk_image,
		/* subresourceRange */    VkImageSubresourceRange
		                          {
		                              /* aspectMask */     VK_IMAGE_ASPECT_COLOR_BIT,
		                              /* baseMipLevel */   vk_base_mip_level,
		                              /* levelCount */     vk_mip_level_count,
		                              /* baseArrayLayer */ 0,
		                              /* layerCount */     1,
		                          },
	};
	vkCmdPipelineBarrier(
		/* commandBuffer */            vk_command_buffer,
		/* srcStageMask */             vk_src_stage_flags,
		/* dstStageMask */             vk_dst_stage_flags,
		/* dependencyFlags */          0,
		/* memoryBarrierCount */       0,
		/* pMemoryBarriers */          nullptr,
		/* bufferMemoryBarrierCount */ 0,
		/* pBufferMemoryBarriers */    nullptr,
		/* imageMemoryBarrierCount */  1,
		/* pImageMemoryBarriers */     &vk_image_memory_barrier
	);
}

void VkR3rContext::draw_state_update_scissor()
{
	draw_state.scissor = VkRect2D
	{
		/* offset */ VkOffset2D
		             {
		                 /* x */ 0,
		                 /* y */ 0,
		             },
		/* extent */ VkExtent2D
		             {
		                 /* width */  vk_offscreen_width,
		                 /* height */ vk_offscreen_height,
		             }
	};
}

void VkR3rContext::update_draw_state()
{
	shader_vars_draw_state.clear();
}

} // namespace bstone

// ======================================

bool operator==(const VkOffset2D& a, const VkOffset2D& b)
{
	return a.x == b.x && a.y == b.y;
}

bool operator==(const VkExtent2D& a, const VkExtent2D& b)
{
	return a.width == b.width && a.height == b.height;
}

bool operator==(const VkRect2D& a, const VkRect2D& b)
{
	return a.offset == b.offset && a.extent == b.extent;
}

bool operator==(const VkViewport& a, const VkViewport& b)
{
	return
		a.x == b.x &&
		a.y == b.y &&
		a.width == b.width &&
		a.height == b.height &&
		a.minDepth == b.minDepth &&
		a.maxDepth == b.maxDepth;
}
