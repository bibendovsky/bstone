/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer: RAII

#include "bstone_vk_r3r_raii.h"
#include "bstone_vk_r3r_context.h"

namespace bstone {

const VkR3rResourceNull vk_r3r_resource_null{};

// ======================================

VkR3rInstanceDeleter::VkR3rInstanceDeleter(const VkR3rContext& context)
	:
	context_{&context}
{}

void VkR3rInstanceDeleter::operator()(VkInstance vk_instance) const
{
	context_->vkDestroyInstance(
		/* instance */   vk_instance,
		/* pAllocator */ nullptr
	);
}

// ======================================

VkR3rDebugUtilsMessengerDeleter::VkR3rDebugUtilsMessengerDeleter(const VkR3rContext& context)
	:
	context_{&context}
{}

void VkR3rDebugUtilsMessengerDeleter::operator()(VkDebugUtilsMessengerEXT vk_debug_utils_messenger) const
{
	context_->vkDestroyDebugUtilsMessengerEXT(
		/* instance */   context_->instance.get(),
		/* messenger */  vk_debug_utils_messenger,
		/* pAllocator */ nullptr
	);
}

// ======================================

VkR3rSurfaceKhrDeleter::VkR3rSurfaceKhrDeleter(const VkR3rContext& context)
	:
	context_{&context}
{}

void VkR3rSurfaceKhrDeleter::operator()(VkSurfaceKHR vk_surface_khr) const
{
	context_->vkDestroySurfaceKHR(
		/* instance */   context_->instance.get(),
		/* surface */    vk_surface_khr,
		/* pAllocator */ nullptr
	);
}

// ======================================

VkR3rDeviceDeleter::VkR3rDeviceDeleter(const VkR3rContext& context)
	:
	context_{&context}
{}

void VkR3rDeviceDeleter::operator()(VkDevice vk_device) const
{
	context_->vkDestroyDevice(
		/* device */     vk_device,
		/* pAllocator */ nullptr
	);
}

// ======================================

VkR3rSwapchainKhrDeleter::VkR3rSwapchainKhrDeleter(const VkR3rContext& context)
	:
	context_{&context}
{}

void VkR3rSwapchainKhrDeleter::operator()(VkSwapchainKHR vk_swapchain_khr) const
{
	context_->vkDestroySwapchainKHR(
		/* device */     context_->device.get(),
		/* swapchain */  vk_swapchain_khr,
		/* pAllocator */ nullptr
	);
}

// ======================================

VkR3rImageViewDeleter::VkR3rImageViewDeleter(const VkR3rContext& context)
	:
	context_{&context}
{}

void VkR3rImageViewDeleter::operator()(VkImageView vk_image_view) const
{
	context_->vkDestroyImageView(
		/* device */     context_->device.get(),
		/* imageView */  vk_image_view,
		/* pAllocator */ nullptr
	);
}

// ======================================

VkR3rRenderPassDeleter::VkR3rRenderPassDeleter(const VkR3rContext& context)
	:
	context_{&context}
{}

void VkR3rRenderPassDeleter::operator()(VkRenderPass vk_render_pass) const
{
	context_->vkDestroyRenderPass(
		/* device */     context_->device.get(),
		/* renderPass */ vk_render_pass,
		/* pAllocator */ nullptr
	);
}

// ======================================

VkR3rFramebufferDeleter::VkR3rFramebufferDeleter(const VkR3rContext& context)
	:
	context_{&context}
{}

void VkR3rFramebufferDeleter::operator()(VkFramebuffer vk_framebuffer) const
{
	context_->vkDestroyFramebuffer(
		/* device */      context_->device.get(),
		/* framebuffer */ vk_framebuffer,
		/* pAllocator */  nullptr
	);
}

// ======================================

VkR3rCommandPoolDeleter::VkR3rCommandPoolDeleter(const VkR3rContext& context)
	:
	context_{&context}
{}


void VkR3rCommandPoolDeleter::operator()(VkCommandPool vk_command_pool) const
{
	context_->vkDestroyCommandPool(
		/* device */      context_->device.get(),
		/* commandPool */ vk_command_pool,
		/* pAllocator */  nullptr
	);
}

// ======================================

VkR3rSemaphoreDeleter::VkR3rSemaphoreDeleter(const VkR3rContext& context)
	:
	context_{&context}
{}

void VkR3rSemaphoreDeleter::operator()(VkSemaphore vk_semaphore) const
{
	context_->vkDestroySemaphore(
		/* device */     context_->device.get(),
		/* semaphore */  vk_semaphore,
		/* pAllocator */ nullptr
	);
}

// ======================================

VkR3rFenceDeleter::VkR3rFenceDeleter(const VkR3rContext& context)
	:
	context_{&context}
{}

void VkR3rFenceDeleter::operator()(VkFence vk_fence) const
{
	context_->vkDestroyFence(
		/* device */     context_->device.get(),
		/* fence */      vk_fence,
		/* pAllocator */ nullptr
	);
}

// ======================================

VkR3rShaderModuleDeleter::VkR3rShaderModuleDeleter(const VkR3rContext& context)
	:
	context_{&context}
{}

void VkR3rShaderModuleDeleter::operator()(VkShaderModule vk_shader_module) const
{
	context_->vkDestroyShaderModule(
		/* device */       context_->device.get(),
		/* shaderModule */ vk_shader_module,
		/* pAllocator */   nullptr
	);
}

// ======================================

VkR3rDeviceMemoryDeleter::VkR3rDeviceMemoryDeleter(const VkR3rContext& context)
	:
	context_{&context}
{}

void VkR3rDeviceMemoryDeleter::operator()(VkDeviceMemory vk_device_memory) const
{
	context_->vkFreeMemory(
		/* device */     context_->device.get(),
		/* memory */     vk_device_memory,
		/* pAllocator */ nullptr
	);
}

// ======================================

VkR3rBufferDeleter::VkR3rBufferDeleter(const VkR3rContext& context)
	:
	context_{&context}
{}

void VkR3rBufferDeleter::operator()(VkBuffer vk_buffer) const
{
	context_->vkDestroyBuffer(
		/* device */     context_->device.get(),
		/* buffer */     vk_buffer,
		/* pAllocator */ nullptr
	);
}

// ======================================

VkR3rImageDeleter::VkR3rImageDeleter(const VkR3rContext& context)
	:
	context_{&context}
{}

void VkR3rImageDeleter::operator()(VkImage vk_image) const
{
	context_->vkDestroyImage(
		/* device */     context_->device.get(),
		/* image */      vk_image,
		/* pAllocator */ nullptr
	);
}

// ======================================

VkR3rSamplerDeleter::VkR3rSamplerDeleter(const VkR3rContext& context)
	:
	context_{&context}
{}

void VkR3rSamplerDeleter::operator()(VkSampler vk_sampler) const
{
	context_->vkDestroySampler(
		/* device */     context_->device.get(),
		/* sampler */    vk_sampler,
		/* pAllocator */ nullptr
	);
}

// ======================================

VkR3rCommandBufferDeleter::VkR3rCommandBufferDeleter(const VkR3rContext& context, VkCommandPool vk_command_pool)
	:
	context_{&context},
	vk_command_pool_{vk_command_pool}
{}

void VkR3rCommandBufferDeleter::operator()(VkCommandBuffer vk_command_buffer) const
{
	context_->vkFreeCommandBuffers(
		/* device */             context_->device.get(),
		/* commandPool */        vk_command_pool_,
		/* commandBufferCount */ 1,
		/* pCommandBuffers */    &vk_command_buffer
	);
}

// ======================================

VkR3rDescriptorPoolDeleter::VkR3rDescriptorPoolDeleter(const VkR3rContext& context)
	:
	context_{&context}
{}

void VkR3rDescriptorPoolDeleter::operator()(VkDescriptorPool vk_descriptor_pool) const
{
	context_->vkDestroyDescriptorPool(
		/* device */         context_->device.get(),
		/* descriptorPool */ vk_descriptor_pool,
		/* pAllocator */     nullptr
	);
}

// ======================================

VkR3rDescriptorSetLayoutDeleter::VkR3rDescriptorSetLayoutDeleter(const VkR3rContext& context)
	:
	context_{&context}
{}

void VkR3rDescriptorSetLayoutDeleter::operator()(VkDescriptorSetLayout vk_descriptor_set_layout) const
{
	context_->vkDestroyDescriptorSetLayout(
		/* device */              context_->device.get(),
		/* descriptorSetLayout */ vk_descriptor_set_layout,
		/* pAllocator */          nullptr
	);
}

// ======================================

VkR3rDescriptorSetDeleter::VkR3rDescriptorSetDeleter(const VkR3rContext& context)
	:
	context_{&context}
{}

void VkR3rDescriptorSetDeleter::operator()(VkDescriptorSet vk_descriptor_set) const
{
	context_->vkFreeDescriptorSets(
		/* device */             context_->device.get(),
		/* descriptorPool */     context_->descriptor_pool.get(),
		/* descriptorSetCount */ 1,
		/* pDescriptorSets */    &vk_descriptor_set
	);
}

// ======================================

VkR3rPipelineLayoutDeleter::VkR3rPipelineLayoutDeleter(const VkR3rContext& context)
	:
	context_{&context}
{}

void VkR3rPipelineLayoutDeleter::operator()(VkPipelineLayout vk_pipeline_layout) const
{
	context_->vkDestroyPipelineLayout(
		/* device */         context_->device.get(),
		/* pipelineLayout */ vk_pipeline_layout,
		/* pAllocator */     nullptr
	);
}

// ======================================

VkR3rPipelineDeleter::VkR3rPipelineDeleter(const VkR3rContext& context)
	:
	context_{&context}
{}

void VkR3rPipelineDeleter::operator()(VkPipeline vk_pipeline) const
{
	context_->vkDestroyPipeline(
		/* device */     context_->device.get(),
		/* pipeline */   vk_pipeline,
		/* pAllocator */ nullptr
	);
}

} // namespace bstone
