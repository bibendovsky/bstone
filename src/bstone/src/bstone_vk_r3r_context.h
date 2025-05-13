/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer: Context

#ifndef BSTONE_VK_R3R_CONTEXT_INCLUDED
#define BSTONE_VK_R3R_CONTEXT_INCLUDED

#include "vulkan/vulkan.h"
#include "bstone_r3r_cmd_buffer.h"
#include "bstone_r3r_shader_var.h"
#include "bstone_vk_r3r_observer.h"
#include "bstone_vk_r3r_raii.h"
#include <unordered_map>
#include <vector>

namespace bstone {

class VkR3rContext
{
public:
	static const char* const vk_layer_khronos_validation_extension_name;
	static const char* const vk_ext_debug_utils_extension_name;
	static constexpr std::uint32_t total_attachments = 2;
	static constexpr std::uint32_t color_attachment_index = 0;
	static constexpr std::uint32_t depth_attachment_index = 1;
	static constexpr std::uint32_t max_pipelines = 32;

	using Semaphores = std::vector<VkR3rSemaphoreResource>;
	using Images = std::vector<VkImage>;
	using ImageViews = std::vector<VkR3rImageViewResource>;
	using ImageLayouts = std::vector<VkImageLayout>;
	using Framebuffers = std::vector<VkR3rFramebufferResource>;
	using StringPointers = std::vector<const char*>;
	struct ShaderVarDrawState
	{
		R3rShaderVarTypeId type_id;
		R3rShaderVar* shader_var;

		union
		{
			int int32_value;
			float float32_value;
			R3rVec2 vec2_value;
			R3rVec4 vec4_value;
			R3rMat4 mat4_value;
		};
	};
	using ShaderVarsDrawState = std::vector<ShaderVarDrawState>;
	struct DrawState
	{
		bool is_culling_enabled;
		bool is_blending_enabled;
		bool is_depth_test_enabled;
		bool is_depth_write_enabled;
		VkViewport viewport;
		VkRect2D scissor;
		R3rR2Texture* r2_texture;
		R3rSampler* sampler;
		R3rVertexInput* vertex_input;
		R3rShaderStage* shader_stage;
	};

	PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr{};
	PFN_vkCreateInstance vkCreateInstance{};
	PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties{};
	PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties{};
	PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices{};
	PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures{};
	PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties{};
	PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties{};
	PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties{};
	PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties{};
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR{};
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR{};
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR{};
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR{};
	PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties{};
	PFN_vkCreateDevice vkCreateDevice{};
	PFN_vkDestroyDevice vkDestroyDevice{};
	PFN_vkGetDeviceQueue vkGetDeviceQueue{};
	PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR{};
	PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR{};
	PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR{};
	PFN_vkCreateImageView vkCreateImageView{};
	PFN_vkDestroyImageView vkDestroyImageView{};
	PFN_vkDeviceWaitIdle vkDeviceWaitIdle{};
	PFN_vkCreateRenderPass vkCreateRenderPass{};
	PFN_vkDestroyRenderPass vkDestroyRenderPass{};
	PFN_vkCreateFramebuffer vkCreateFramebuffer{};
	PFN_vkDestroyFramebuffer vkDestroyFramebuffer{};
	PFN_vkCreateCommandPool vkCreateCommandPool{};
	PFN_vkDestroyCommandPool vkDestroyCommandPool{};
	PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers{};
	PFN_vkFreeCommandBuffers vkFreeCommandBuffers{};
	PFN_vkBeginCommandBuffer vkBeginCommandBuffer{};
	PFN_vkEndCommandBuffer vkEndCommandBuffer{};
	PFN_vkResetCommandBuffer vkResetCommandBuffer{};
	PFN_vkCreateSemaphore vkCreateSemaphore{};
	PFN_vkDestroySemaphore vkDestroySemaphore{};
	PFN_vkCreateFence vkCreateFence{};
	PFN_vkDestroyFence vkDestroyFence{};
	PFN_vkResetFences vkResetFences{};
	PFN_vkWaitForFences vkWaitForFences{};
	PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR{};
	PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass{};
	PFN_vkCmdEndRenderPass vkCmdEndRenderPass{};
	PFN_vkQueueSubmit vkQueueSubmit{};
	PFN_vkQueuePresentKHR vkQueuePresentKHR{};
	PFN_vkQueueWaitIdle vkQueueWaitIdle{};
	PFN_vkCmdClearAttachments vkCmdClearAttachments{};
	PFN_vkCmdCopyBuffer vkCmdCopyBuffer{};
	PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage{};
	PFN_vkCmdCopyImageToBuffer vkCmdCopyImageToBuffer{};
	PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier{};
	PFN_vkCreateShaderModule vkCreateShaderModule{};
	PFN_vkDestroyShaderModule vkDestroyShaderModule{};
	PFN_vkAllocateMemory vkAllocateMemory{};
	PFN_vkFreeMemory vkFreeMemory{};
	PFN_vkMapMemory vkMapMemory{};
	PFN_vkUnmapMemory vkUnmapMemory{};
	PFN_vkCreateBuffer vkCreateBuffer{};
	PFN_vkDestroyBuffer vkDestroyBuffer{};
	PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements{};
	PFN_vkBindBufferMemory vkBindBufferMemory{};
	PFN_vkCreateImage vkCreateImage{};
	PFN_vkDestroyImage vkDestroyImage{};
	PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements{};
	PFN_vkBindImageMemory vkBindImageMemory{};
	PFN_vkCreateSampler vkCreateSampler{};
	PFN_vkDestroySampler vkDestroySampler{};
	PFN_vkCreateDescriptorPool vkCreateDescriptorPool{};
	PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool{};
	PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout{};
	PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout{};
	PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets{};
	PFN_vkFreeDescriptorSets vkFreeDescriptorSets{};
	PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets{};
	PFN_vkCreatePipelineLayout vkCreatePipelineLayout{};
	PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout{};
	PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines{};
	PFN_vkDestroyPipeline vkDestroyPipeline{};
	PFN_vkCmdSetViewport vkCmdSetViewport{};
	PFN_vkCmdSetScissor vkCmdSetScissor{};
	PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer{};
	PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers{};
	PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets{};
	PFN_vkCmdBindPipeline vkCmdBindPipeline{};
	PFN_vkCmdDrawIndexed vkCmdDrawIndexed{};
	PFN_vkCmdBlitImage vkCmdBlitImage{};
	PFN_vkCmdResolveImage vkCmdResolveImage{};

	PFN_vkDestroyInstance vkDestroyInstance{};
	PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT{};
	PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT{};
	PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR{};

	bool has_vk_layer_khronos_validation{};
	bool has_ext_debug_utils{};
	bool has_vk_present_mode_immediate_khr{};
	bool has_vk_present_mode_fifo_khr{};

	R3rDeviceFeatures r3r_device_features{};
	VkFormat surface_format{};
	int sample_count_bitmask{};
	int sample_count{};
	VkPresentModeKHR vk_present_mode_khr{};

	std::uint32_t vk_surface_width{};
	std::uint32_t vk_surface_height{};

	DrawState draw_state{};
	ShaderVarsDrawState shader_vars_draw_state{};

	StringPointers enabled_layers{};
	StringPointers enabled_extensions{};
	StringPointers enabled_device_extensions{};
	VkR3rInstanceResource instance{};
	VkR3rDebugUtilsMessengerResource debug_utils_messenger{};
	VkR3rSurfaceKhrResource surface{};
	VkSurfaceCapabilitiesKHR surface_capabilities{};
	VkPhysicalDevice physical_device{};
	std::uint32_t queue_family_index{};
	VkPhysicalDeviceFeatures physical_device_features{};
	VkPhysicalDeviceMemoryProperties memory_properties{};
	VkPhysicalDeviceProperties physical_device_properties{};
	VkR3rDeviceResource device{};
	VkQueue queue{};
	std::uint32_t vk_offscreen_width{};
	std::uint32_t vk_offscreen_height{};
	VkR3rDeviceMemoryResource offscreen_color_image_memory{};
	VkR3rImageResource offscreen_color_image{};
	VkR3rImageViewResource offscreen_color_image_view{};
	VkR3rDeviceMemoryResource offscreen_depth_image_memory{};
	VkR3rImageResource offscreen_depth_image{};
	VkR3rImageViewResource offscreen_depth_image_view{};
	VkR3rFramebufferResource offscreen_framebuffer{};
	VkR3rDeviceMemoryResource screenshot_buffer_memory{};
	VkR3rBufferResource screenshot_buffer{};
	VkR3rDeviceMemoryResource screenshot_image_memory{};
	VkR3rImageResource screenshot_image{};
	VkR3rSwapchainKhrResource old_swapchain{};
	VkR3rSwapchainKhrResource swapchain{};
	std::uint32_t swapchain_image_index{UINT32_MAX};
	Images swapchain_images{};
	ImageViews swapchain_image_views{};
	ImageLayouts swapchain_image_layouts{};
	VkR3rRenderPassResource render_pass{};
	VkR3rCommandPoolResource transient_command_pool{};
	VkR3rCommandPoolResource command_pool{};
	VkCommandBuffer command_buffer{};
	VkR3rSemaphoreResource image_available_semaphore{};
	Semaphores render_finished_semaphores{};
	VkR3rFenceResource in_flight_fence{};
	VkR3rDescriptorPoolResource descriptor_pool{};
	VkR3rPostPresentSubject post_present_subject{};

	static void ensure_success_vk_result(VkResult vk_result, const char* message);
	bool has_swapchain() const;
	std::uint32_t find_memory_type_index(
		std::uint32_t memory_type_bits,
		VkMemoryPropertyFlags memory_property_flags) const;
	VkR3rCommandPoolResource create_command_pool(VkCommandPoolCreateFlags vk_flags) const;
	VkR3rCommandBufferResource create_command_buffer_resource(VkCommandPool vk_command_pool) const;
	VkR3rCommandBufferResource cmd_begin_single_time_commands() const;
	void cmd_end_single_time_commands(VkCommandBuffer vk_command_buffer) const;
	VkR3rDeviceMemoryResource allocate_device_memory_resource(
		VkDeviceSize vk_allocation_size,
		std::uint32_t memory_type_index) const;
	void* map_memory(VkDeviceMemory vk_device_memory) const;
	void unmap_memory(VkDeviceMemory vk_device_memory) const;
	void create_buffer_resource(
		VkDeviceSize vk_size,
		VkBufferUsageFlags vk_buffer_usage_flags,
		VkMemoryPropertyFlags vk_memory_property_flags,
		VkR3rBufferResource& buffer_resource,
		VkR3rDeviceMemoryResource& device_memory_resource) const;
	void cmd_copy_buffer(
		VkCommandBuffer vk_command_buffer,
		VkBuffer vk_src_buffer,
		VkBuffer vk_dst_buffer,
		VkDeviceSize vk_size) const;
	void create_image_resource(
		VkFormat vk_format,
		std::uint32_t vk_width,
		std::uint32_t vk_height,
		std::uint32_t vk_mip_levels,
		VkSampleCountFlagBits vk_sample_count,
		VkImageTiling vk_image_tiling,
		VkImageUsageFlags vk_image_usage_flags,
		VkMemoryPropertyFlags vk_memory_property_flags,
		VkR3rImageResource& image_resource,
		VkR3rDeviceMemoryResource& device_memory_resource) const;
	VkR3rImageViewResource create_image_view_resource(
		VkImage vk_image,
		VkFormat vk_format,
		VkImageAspectFlags vk_image_aspect_flags) const;
	void cmd_copy_buffer_to_image(
		VkCommandBuffer vk_command_buffer,
		VkBuffer vk_buffer,
		VkImage vk_image,
		std::uint32_t vk_width,
		std::uint32_t vk_height,
		std::uint32_t vk_mip_level) const;
	void cmd_copy_image_to_buffer(
		VkCommandBuffer vk_command_buffer,
		VkImage vk_image,
		std::uint32_t vk_image_width,
		std::uint32_t vk_image_height,
		VkImageLayout vk_image_layout,
		VkBuffer vk_buffer) const;
	void cmd_blit_image(
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
		VkFilter vk_filter) const;
	void cmd_image_memory_barrier(
		VkCommandBuffer vk_command_buffer,
		VkImage vk_image,
		VkImageLayout vk_old_layout,
		VkImageLayout vk_new_layout,
		std::uint32_t vk_base_mip_level,
		std::uint32_t vk_mip_level_count,
		VkPipelineStageFlags vk_src_stage_flags,
		VkPipelineStageFlags vk_dst_stage_flags) const;
	void draw_state_update_scissor();
	void update_draw_state();
};

} // namespace bstone

// ======================================

bool operator==(const VkOffset2D& a, const VkOffset2D& b);
bool operator==(const VkExtent2D& a, const VkExtent2D& b);
bool operator==(const VkRect2D& a, const VkRect2D& b);
bool operator==(const VkViewport& a, const VkViewport& b);

#endif // BSTONE_VK_R3R_CONTEXT_INCLUDED
