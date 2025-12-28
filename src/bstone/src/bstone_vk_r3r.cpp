/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer

#include "bstone_vk_r3r.h"
#include "bstone_assert.h"
#include "bstone_ascii.h"
#include "bstone_char_conv.h"
#include "bstone_exception.h"
#include "bstone_scope_exit.h"
#include "bstone_single_pool_resource.h"
#include "bstone_uuid.h"
#include "bstone_r3r_cmd_buffer.h"
#include "bstone_r3r_limits.h"
#include "bstone_sys_logger.h"
#include "bstone_vk_r3r_array_extractor.h"
#include "bstone_vk_r3r_buffer.h"
#include "bstone_vk_r3r_context.h"
#include "bstone_vk_r3r_enum_strings.h"
#include "bstone_vk_r3r_info.h"
#include "bstone_vk_r3r_pipeline.h"
#include "bstone_vk_r3r_pipeline_mgr.h"
#include "bstone_vk_r3r_r2_texture.h"
#include "bstone_vk_r3r_sampler.h"
#include "bstone_vk_r3r_shader.h"
#include "bstone_vk_r3r_shader_stage.h"
#include "bstone_vk_r3r_vertex_input.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <array>
#include <bit>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

// ======================================

#define BSTONE_STRCTX(x) #x, context_.x

// ======================================

namespace bstone {

namespace {

class VkR3rImpl final : public R3r
{
public:
	VkR3rImpl(sys::VideoMgr& video_mgr, sys::WindowMgr& window_mgr, const R3rInitParam& param);
	~VkR3rImpl() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

	R3rType do_get_type() const noexcept override;
	std::string_view do_get_name() const noexcept override;
	std::string_view do_get_description() const noexcept override;

	const R3rDeviceFeatures& do_get_device_features() const noexcept override;
	const R3rDeviceInfo& do_get_device_info() const noexcept override;

	void do_enable_checking_api_calls_for_errors(bool is_enable) override;

	sys::Window& do_get_window() const noexcept override;
	void do_handle_resize(sys::WindowSize new_size) override;

	bool do_get_vsync() const noexcept override;
	void do_enable_vsync(bool is_enabled) override;

	void do_set_anti_aliasing(R3rAaType aa_type, int aa_value) override;

	void do_read_pixels(
		sys::PixelFormat pixel_format,
		void* buffer,
		bool& is_flipped_vertically) override;

	void do_present() override;

	R3rBufferUPtr do_create_buffer(const R3rBufferInitParam& param) override;
	R3rR2TextureUPtr do_create_r2_texture(const R3rR2TextureInitParam& param) override;
	R3rSamplerUPtr do_create_sampler(const R3rSamplerInitParam& param) override;
	R3rVertexInputUPtr do_create_vertex_input(const R3rCreateVertexInputParam& param) override;
	R3rShaderUPtr do_create_shader(const R3rShaderInitParam& param) override;
	R3rShaderStageUPtr do_create_shader_stage(const R3rShaderStageInitParam& param) override;
	void do_submit_commands(std::span<R3rCmdBuffer*> command_buffers) override;
	void do_wait_for_device() override;

	using MemoryPool = SinglePoolResource<VkR3rImpl>;
	using StringPointers = std::vector<const char*>;
	using QueueFamilies = std::vector<VkQueueFamilyProperties>;
	enum class FenceState
	{
		unsignaled = 0,
		signaled,
	};
	struct FrameState
	{
		bool is_awaited_for_previous_frame;
		bool is_acquired_image_from_swapchain;
		bool is_recorded_any_command;
	};

	static MemoryPool memory_pool_;

	sys::Logger& logger_;
	sys::VideoMgr& video_mgr_;
	sys::WindowMgr& window_mgr_;
	VkR3rInfo info_{logger_, context_};
	R3rType type_{};
	std::string_view name_{};
	std::string_view description_{};
	R3rDeviceInfo device_info_{};
	sys::WindowUPtr window_{};
	VkR3rContext context_{};
	VkR3rPipelineMgrUPtr pipeline_mgr_{};
	FrameState frame_state_{};

	static float color_byte_to_float(std::uint8_t value);
	static void ensure_not_null_shader_var(R3rShaderVar* shader_var);

	template<typename T>
	void resolve_symbol(VkInstance instance, const char* name, T& symbol)
	{
		BSTONE_ASSERT(context_.vkGetInstanceProcAddr != nullptr);
		PFN_vkVoidFunction const symbol_void = context_.vkGetInstanceProcAddr(
			/* instance */ instance,
			/* pName */    name
		);

		if (symbol_void == nullptr)
		{
			std::string message{};
			message.reserve(256);
			message += "Symbol ";
			message += name;
			message += " not found.";
			BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
		}

		symbol = std::bit_cast<T>(symbol_void);
	}

	template<typename T>
	void resolve_symbol(const char* name, T& symbol)
	{
		resolve_symbol(nullptr, name, symbol);
	}

	static VkBool32 VKAPI_PTR vk_debug_utils_messenger_callback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageTypes,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	static void ensure_vk_result(VkResult vk_result, const char* vk_name);
	static std::size_t align16(std::size_t value);
	int get_max_sample_count() const;
	int choose_sample_count(R3rAaType aa_type, int aa_degree) const;
	VkPresentModeKHR choose_present_mode(bool enable_vsync) const;
	void wait_for_device() const;

	VkR3rSemaphoreResource make_semaphore() const;

	VkR3rFenceResource make_fence(FenceState initial_state) const;
	VkR3rFenceResource make_unsignaled_fence() const;
	VkR3rFenceResource make_signaled_fence() const;
	void reset_fence(VkFence vk_fence) const;
	void wait_for_fence(VkFence vk_fence) const;

	void begin_submit_commands();
	void end_submit_commands();

	void initialize_defaults();
	void initialize_vulkan_manager();
	void initialize_window();
	void initialize_global_symbols();
	void initialize_validation_layers();
	void initialize_global_extensions();
	void initialize_enabled_global_layers();
	void initialize_enabled_global_extensions();
	void initialize_instance();
	void initialize_instance_symbols();
	void initialize_debug_utils_messenger();
	void terminate_surface();
	void initialize_surface();
	void update_surface_capabilities();
	void choose_physical_device();
	void initialize_physical_device_features();
	void initialize_physical_device_properties();
	void initialize_physical_device_memory_properties();
	void initialize_enabled_device_extensions();
	void initialize_logical_device();
	void terminate_swapchain();
	void initialize_swapchain();
	void terminate_swapchain_images();
	void initialize_swapchain_images();
	void terminate_swapchain_image_views();
	void initialize_swapchain_image_views();
	void terminate_render_pass();
	void initialize_render_pass();
	void terminate_offscreen_framebuffer();
	void initialize_offscreen_framebuffer();
	void terminate_screenshot_buffer();
	void initialize_screenshot_buffer();
	void terminate_screenshot_image();
	void initialize_screenshot_image();
	void initialize_transient_command_pool();
	void initialize_command_pool();
	void initialize_command_buffer();
	void terminate_swapchain_sync_objects();
	void initialize_swapchain_sync_objects();
	void initialize_sync_objects();
	void initialize_descriptor_pool();
	void initialize_pipeline_mgr();
	void initialize_sample_count(const R3rInitParam& r3r_init_param);
	void initialize_present_mode(const R3rInitParam& r3r_init_param);
	void initialize_r3r_device_features();

	void submit_none_commands();

	void wait_for_previous_frame();
	void swapchain_acquire_next_image();
	void recreate_swapchain();
	void blit_offscreen();

	void frame_state_record_command_buffer_internal();
	void frame_state_record_command_buffer();
	void frame_state_pre_present();
	void frame_state_reset();

	void submit_clear_command(const R3rClearCmd& r3r_cmd);
	void submit_set_viewport(const R3rSetViewportCmd& r3r_cmd);
	void submit_enable_culling(const R3rEnableCullingCmd& r3r_cmd);
	void submit_enable_depth_test(const R3rEnableDepthTestCmd& r3r_cmd);
	void submit_enable_depth_write(const R3rEnableDepthWriteCmd& r3r_cmd);
	void submit_enable_blending(const R3rEnableBlendingCmd& r3r_cmd);
	void submit_set_blending_func(const R3rSetBlendingFuncCmd& r3r_cmd);
	void submit_set_texture(const R3rSetTextureCmd& r3r_cmd);
	void submit_set_sampler(const R3rSetSamplerCmd& r3r_cmd);
	void submit_set_vertex_input(const R3rSetVertexInputCmd& r3r_cmd);
	void submit_set_shader_stage(const R3rSetShaderStageCmd& r3r_cmd);
	void submit_set_int32_uniform(const R3rSetInt32UniformCmd& r3r_cmd);
	void submit_set_float32_uniform(const R3rSetFloat32UniformCmd& r3r_cmd);
	void submit_set_vec2_uniform(const R3rSetVec2UniformCmd& r3r_cmd);
	void submit_set_vec4_uniform(const R3rSetVec4UniformCmd& r3r_cmd);
	void submit_set_mat4_uniform(const R3rSetMat4UniformCmd& r3r_cmd);
	void submit_set_r2_sampler_uniform(const R3rSetR2SamplerUniformCmd& r3r_cmd);
	void submit_draw_indexed(const R3rDrawIndexedCmd& r3r_cmd);

	[[noreturn]] void submit_unknown_command(R3rCmdId cmd_id);
};

// --------------------------------------

VkR3rImpl::MemoryPool VkR3rImpl::memory_pool_{};

// --------------------------------------

VkR3rImpl::~VkR3rImpl()
{
	wait_for_device();
}

VkR3rImpl::VkR3rImpl(sys::VideoMgr& video_mgr, sys::WindowMgr& window_mgr, const R3rInitParam& param)
try
	:
	logger_{video_mgr.get_logger()},
	video_mgr_{video_mgr},
	window_mgr_{window_mgr},
	type_{param.renderer_type},
	name_{"VK"},
	description_{"Vulkan"}
{
	BSTONE_ASSERT(param.renderer_type == R3rType::vulkan);
	initialize_defaults();
	initialize_vulkan_manager();
	initialize_window();
	initialize_global_symbols();
	initialize_validation_layers();
	initialize_global_extensions();
	initialize_instance();
	initialize_instance_symbols();
	initialize_debug_utils_messenger();
	initialize_surface();
	choose_physical_device();
	update_surface_capabilities();
	initialize_present_mode(param);
	initialize_physical_device_features();
	initialize_physical_device_properties();
	initialize_physical_device_memory_properties();
	initialize_logical_device();
	initialize_sample_count(param);
	initialize_render_pass();
	initialize_swapchain();
	initialize_swapchain_images();
	initialize_swapchain_image_views();
	initialize_offscreen_framebuffer();
	initialize_transient_command_pool();
	initialize_command_pool();
	initialize_command_buffer();
	initialize_swapchain_sync_objects();
	initialize_sync_objects();
	initialize_descriptor_pool();
	initialize_pipeline_mgr();
	initialize_r3r_device_features();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void* VkR3rImpl::operator new(std::size_t size)
try {
	return memory_pool_.allocate(static_cast<std::intptr_t>(size));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void VkR3rImpl::operator delete(void* ptr)
{
	memory_pool_.deallocate(ptr);
}

R3rType VkR3rImpl::do_get_type() const noexcept
{
	return type_;
}

std::string_view VkR3rImpl::do_get_name() const noexcept
{
	return name_;
}

std::string_view VkR3rImpl::do_get_description() const noexcept
{
	return description_;
}

const R3rDeviceFeatures& VkR3rImpl::do_get_device_features() const noexcept
{
	return context_.r3r_device_features;
}

const R3rDeviceInfo& VkR3rImpl::do_get_device_info() const noexcept
{
	return device_info_;
}

void VkR3rImpl::do_enable_checking_api_calls_for_errors([[maybe_unused]] bool is_enable)
{}

sys::Window& VkR3rImpl::do_get_window() const noexcept
{
	return *window_;
}

void VkR3rImpl::do_handle_resize(sys::WindowSize new_size)
try
{
	if (new_size.width < R3rLimits::min_viewport_width() ||
		new_size.width > context_.r3r_device_features.max_viewport_width ||
		new_size.height < R3rLimits::min_viewport_height() ||
		new_size.height > context_.r3r_device_features.max_viewport_height)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid size.");
	}
	const std::uint32_t old_width = context_.vk_offscreen_width;
	const std::uint32_t old_height = context_.vk_offscreen_height;
	context_.vk_offscreen_width = static_cast<std::uint32_t>(new_size.width);
	context_.vk_offscreen_height = static_cast<std::uint32_t>(new_size.height);
	if (context_.vk_offscreen_width != old_width || context_.vk_offscreen_height != old_height)
	{
		terminate_offscreen_framebuffer();
		initialize_offscreen_framebuffer();
		context_.draw_state_update_scissor();
		recreate_swapchain();
	}
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

bool VkR3rImpl::do_get_vsync() const noexcept
{
	return context_.vk_present_mode_khr == VK_PRESENT_MODE_FIFO_KHR;
}

void VkR3rImpl::do_enable_vsync(bool is_enabled)
{
	const VkPresentModeKHR old_vk_present_mode_khr = context_.vk_present_mode_khr;
	context_.vk_present_mode_khr = choose_present_mode(is_enabled);
	if (old_vk_present_mode_khr != context_.vk_present_mode_khr)
	{
		recreate_swapchain();
	}
}

void VkR3rImpl::do_set_anti_aliasing(R3rAaType aa_type, int aa_value)
{
	const int old_sample_count = context_.sample_count;
	context_.sample_count = choose_sample_count(aa_type, aa_value);
	if (old_sample_count == context_.sample_count)
	{
		return;
	}
	wait_for_device();
	terminate_offscreen_framebuffer();
	terminate_render_pass();
	pipeline_mgr_->clear();
	initialize_render_pass();
	initialize_offscreen_framebuffer();
}

void VkR3rImpl::do_read_pixels(
	sys::PixelFormat pixel_format,
	void* buffer,
	bool& is_flipped_vertically)
{
	if (pixel_format != sys::PixelFormat::r8g8b8)
	{
		BSTONE_THROW_STATIC_SOURCE("Unsupported pixel format.");
	}
	if (context_.surface_format != VK_FORMAT_R8G8B8A8_UNORM &&
		context_.surface_format != VK_FORMAT_B8G8R8A8_UNORM)
	{
		BSTONE_THROW_STATIC_SOURCE("Unsupported surface format for reading.");
	}
	is_flipped_vertically = false;
	// In the end release screenshot releated Vulkan resources.
	const auto screenshot_image_sentinel = make_scope_exit(
		[this]()
		{
			terminate_screenshot_image();
			terminate_screenshot_buffer();
		});
	VkR3rCommandBufferResource command_buffer_resource = context_.cmd_begin_single_time_commands();
	VkImage src_image;
	// We need the stage buffer anyway.
	initialize_screenshot_buffer();
	if (context_.sample_count > 1)
	{
		// Make image resource.
		initialize_screenshot_image();
		src_image = context_.screenshot_image.get();
		context_.cmd_image_memory_barrier(
			command_buffer_resource.get(),
			src_image,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			0,
			1,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		// Resolve offscreen framebuffer to the image resource.
		const VkImageResolve vk_image_resolve{
			/* srcSubresource */ VkImageSubresourceLayers{
				/* aspectMask     */ VK_IMAGE_ASPECT_COLOR_BIT,
				/* mipLevel       */ 0,
				/* baseArrayLayer */ 0,
				/* layerCount     */ 1,
			},
			/* srcOffset      */ VkOffset3D{
				/* x */ 0,
				/* y */ 0,
				/* z */ 0,
			},
			/* dstSubresource */ VkImageSubresourceLayers{
				/* aspectMask     */ VK_IMAGE_ASPECT_COLOR_BIT,
				/* mipLevel       */ 0,
				/* baseArrayLayer */ 0,
				/* layerCount     */ 1,
			},
			/* dstOffset      */ VkOffset3D{
				/* x */ 0,
				/* y */ 0,
				/* z */ 0,
			},
			/* extent         */ VkExtent3D{
				/* width  */ context_.vk_offscreen_width,
				/* height */ context_.vk_offscreen_height,
				/* depth  */ 1,
			},
		};
		context_.vkCmdResolveImage(
			/* commandBuffer  */ command_buffer_resource.get(),
			/* srcImage       */ context_.offscreen_color_image.get(),
			/* srcImageLayout */ VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			/* dstImage       */ src_image,
			/* dstImageLayout */ VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			/* regionCount    */ 1,
			/* pRegions       */ &vk_image_resolve);
		// Prepare the image resource for reading.
		context_.cmd_image_memory_barrier(
			command_buffer_resource.get(),
			src_image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			0,
			1,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	}
	else
	{
		// No need to resolve - just copy from the offscreen to the stage buffer.
		src_image = context_.offscreen_color_image.get();
	}
	// Copy image to the stage buffer.
	context_.cmd_copy_image_to_buffer(
		command_buffer_resource.get(),
		src_image,
		context_.vk_offscreen_width,
		context_.vk_offscreen_height,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		context_.screenshot_buffer.get());
	context_.cmd_end_single_time_commands(command_buffer_resource.get());
	// Map stage buffer.
	const void* const vk_pixels = context_.map_memory(context_.screenshot_buffer_memory.get());
	const std::size_t pixel_count = std::size_t{1} * context_.vk_offscreen_width * context_.vk_offscreen_height;
	const std::size_t data_size = pixel_count * 4;
	// Create the intermediate buffer.
	// Converting directly from the mapped memory to the user buffer is slower significantly (about 10 times).
	void* const intermediate_pixels = ::operator new(data_size);
	const auto intermediate_buffer_sentinel = make_scope_exit(
		[intermediate_pixels]()
		{
			::operator delete(intermediate_pixels);
		});
	// Copy all pixels.
	std::memcpy(intermediate_pixels, vk_pixels, data_size);
	// Convert to RGB.
	struct RgbPixel
	{
		std::uint8_t r;
		std::uint8_t g;
		std::uint8_t b;
	};
	if (context_.surface_format == VK_FORMAT_R8G8B8A8_UNORM)
	{
		// RGBA => RGB
		struct RgbaPixel
		{
			std::uint8_t r;
			std::uint8_t g;
			std::uint8_t b;
			std::uint8_t a;
		};
		const RgbaPixel* rgba_pixels = reinterpret_cast<const RgbaPixel*>(intermediate_pixels);
		RgbPixel* const rgb_pixels = static_cast<RgbPixel*>(buffer);
		for (std::size_t i_pixel = 0; i_pixel < pixel_count; ++i_pixel)
		{
			const RgbaPixel& rgba_pixel = rgba_pixels[i_pixel];
			RgbPixel& dst_pixel = rgb_pixels[i_pixel];
			dst_pixel.r = rgba_pixel.r;
			dst_pixel.g = rgba_pixel.g;
			dst_pixel.b = rgba_pixel.b;
		}
	}
	else
	{
		// BGRA => RGB
		struct BgraPixel
		{
			std::uint8_t b;
			std::uint8_t g;
			std::uint8_t r;
			std::uint8_t a;
		};
		const BgraPixel* bgra_pixels = reinterpret_cast<const BgraPixel*>(intermediate_pixels);
		RgbPixel* const rgb_pixels = static_cast<RgbPixel*>(buffer);
		for (std::size_t i_pixel = 0; i_pixel < pixel_count; ++i_pixel)
		{
			const BgraPixel& bgra_pixel = bgra_pixels[i_pixel];
			RgbPixel& dst_pixel = rgb_pixels[i_pixel];
			dst_pixel.r = bgra_pixel.r;
			dst_pixel.g = bgra_pixel.g;
			dst_pixel.b = bgra_pixel.b;
		}
	}
}

void VkR3rImpl::do_present()
{
	frame_state_pre_present();
	if (context_.has_swapchain())
	{
		VkResult vk_result;
		const VkSemaphore wait_semaphores[1] =
		{
			context_.render_finished_semaphores[context_.swapchain_image_index].get(),
		};
		const VkSwapchainKHR swapchains[1] = {context_.swapchain.get()};
		const VkPresentInfoKHR vk_present_info_khr
		{
			/* sType */              VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			/* pNext */              nullptr,
			/* waitSemaphoreCount */ 1,
			/* pWaitSemaphores */    wait_semaphores,
			/* swapchainCount */     1,
			/* pSwapchains */        swapchains,
			/* pImageIndices */      &context_.swapchain_image_index,
			/* pResults */           nullptr,
		};
		vk_result = context_.vkQueuePresentKHR(
			/* queue */        context_.queue,
			/* pPresentInfo */ &vk_present_info_khr
		);
		switch (vk_result)
		{
			case VK_ERROR_OUT_OF_DATE_KHR:
			case VK_SUBOPTIMAL_KHR:
				recreate_swapchain();
				break;
			default:
				ensure_vk_result(vk_result, "vkQueuePresentKHR");
				break;
		}
	}
	frame_state_reset();
	context_.post_present_subject.notify();
}

R3rBufferUPtr VkR3rImpl::do_create_buffer(const R3rBufferInitParam& param)
try {
	return make_vk_r3r_buffer(context_, param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rVertexInputUPtr VkR3rImpl::do_create_vertex_input(const R3rCreateVertexInputParam& param)
try {
	return make_vk_r3r_vertex_input(context_, param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rShaderUPtr VkR3rImpl::do_create_shader(const R3rShaderInitParam& param)
try {
	return make_vk_r3r_shader(context_, param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rShaderStageUPtr VkR3rImpl::do_create_shader_stage(const R3rShaderStageInitParam& param)
try {
	return make_vk_r3r_shader_stage(context_, param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rR2TextureUPtr VkR3rImpl::do_create_r2_texture(const R3rR2TextureInitParam& param)
try {
	return make_vk_r3r_r2_texture(context_, param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rSamplerUPtr VkR3rImpl::do_create_sampler(const R3rSamplerInitParam& param)
try {
	return make_vk_r3r_sampler(context_, param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void VkR3rImpl::do_submit_commands(std::span<R3rCmdBuffer*> command_buffers)
{
	for (R3rCmdBuffer* r3r_command_buffer : command_buffers)
	{
		if (!r3r_command_buffer->is_enabled())
		{
			continue;
		}
		r3r_command_buffer->begin_read();
		for (int i_command = 0, n_command = r3r_command_buffer->get_count(); i_command < n_command; ++i_command)
		{
			const R3rCmdId cmd_id = r3r_command_buffer->read_command_id();
			switch (cmd_id)
			{
				case R3rCmdId::clear:
					frame_state_record_command_buffer();
					submit_clear_command(r3r_command_buffer->read_clear());
					break;
				case R3rCmdId::set_viewport:
					submit_set_viewport(r3r_command_buffer->read_set_viewport());
					break;
				case R3rCmdId::enable_culling:
					submit_enable_culling(r3r_command_buffer->read_enable_culling());
					break;
				case R3rCmdId::enable_depth_test:
					submit_enable_depth_test(r3r_command_buffer->read_enable_depth_test());
					break;
				case R3rCmdId::enable_depth_write:
					submit_enable_depth_write(r3r_command_buffer->read_enable_depth_write());
					break;
				case R3rCmdId::enable_blending:
					submit_enable_blending(r3r_command_buffer->read_enable_blending());
					break;
				case R3rCmdId::set_blending_func:
					submit_set_blending_func(r3r_command_buffer->read_set_blending_func());
					break;
				case R3rCmdId::set_texture:
					submit_set_texture(r3r_command_buffer->read_set_texture());
					break;
				case R3rCmdId::set_sampler:
					submit_set_sampler(r3r_command_buffer->read_set_sampler());
					break;
				case R3rCmdId::set_vertex_input:
					submit_set_vertex_input(r3r_command_buffer->read_set_vertex_input());
					break;
				case R3rCmdId::set_shader_stage:
					submit_set_shader_stage(r3r_command_buffer->read_set_shader_stage());
					break;
				case R3rCmdId::set_int32_uniform:
					submit_set_int32_uniform(r3r_command_buffer->read_set_int32_uniform());
					break;
				case R3rCmdId::set_float32_uniform:
					submit_set_float32_uniform(r3r_command_buffer->read_set_float32_uniform());
					break;
				case R3rCmdId::set_vec2_uniform:
					submit_set_vec2_uniform(r3r_command_buffer->read_set_vec2_uniform());
					break;
				case R3rCmdId::set_vec4_uniform:
					submit_set_vec4_uniform(r3r_command_buffer->read_set_vec4_uniform());
					break;
				case R3rCmdId::set_mat4_uniform:
					submit_set_mat4_uniform(r3r_command_buffer->read_set_mat4_uniform());
					break;
				case R3rCmdId::set_r2_sampler_uniform:
					submit_set_r2_sampler_uniform(r3r_command_buffer->read_set_r2_sampler_uniform());
					break;
				case R3rCmdId::draw_indexed:
					frame_state_record_command_buffer();
					submit_draw_indexed(r3r_command_buffer->read_draw_indexed());
					break;
				default:
					submit_unknown_command(cmd_id);
					break;
			}
		}
		r3r_command_buffer->end_read();
	}
}

void VkR3rImpl::do_wait_for_device()
{
	wait_for_device();
}

VkBool32 VKAPI_PTR VkR3rImpl::vk_debug_utils_messenger_callback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	[[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	std::string message{};
	message.reserve(1024);
	for (
		const VkDebugUtilsMessengerCallbackDataEXT* data = pCallbackData;
		data != nullptr;
		data = static_cast<const VkDebugUtilsMessengerCallbackDataEXT*>(data->pNext))
	{
		if (!message.empty())
		{
			message += '\n';
		}
		message += "[VK] [";
		std::string severity_string{};
		if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) != 0)
		{
			severity_string += 'V';
		}
		if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) != 0)
		{
			severity_string += 'I';
		}
		if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0)
		{
			severity_string += 'W';
		}
		if ((messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0)
		{
			severity_string += 'E';
		}
		if (!severity_string.empty())
		{
			message += severity_string;
			message += ':';
		}
		message += std::to_string(pCallbackData->messageIdNumber);
		if (pCallbackData->pMessageIdName != nullptr)
		{
			message += ':';
			message += pCallbackData->pMessageIdName;
		}
		message += ']';
		if (pCallbackData->pMessage != nullptr)
		{
			message += ' ';
			message += pCallbackData->pMessage;
		}
	}
	sys::Logger& logger = *static_cast<sys::Logger*>(pUserData);
	logger.log_information(message.c_str());
	return VK_FALSE;
}

float VkR3rImpl::color_byte_to_float(std::uint8_t value)
{
	return value / 255.0F;
}

void VkR3rImpl::ensure_not_null_shader_var(R3rShaderVar* shader_var)
{
	if (shader_var == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null shader var.");
	}
}

void VkR3rImpl::ensure_vk_result(VkResult vk_result, const char* vk_name)
{
	if (vk_result == VK_SUCCESS)
	{
		return;
	}
	std::string message{};
	message.reserve(128);
	message += '[';
	message += vk_name;
	message += "] Result code: ";
	message += std::to_string(vk_result);
	const char* const vk_result_string = VkR3rEnumStrings::get_VkResult(vk_result);
	if (vk_result_string != nullptr)
	{
		message += '(';
		message += vk_result_string;
		message += ')';
	}
	BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
}

std::size_t VkR3rImpl::align16(std::size_t value)
{
	return (value + 15) & (~std::size_t{15});
}

int VkR3rImpl::get_max_sample_count() const
{
	for (int i_bit = 6; i_bit >= 0; --i_bit)
	{
		const int sample_count = 1 << i_bit;
		if ((context_.sample_count_bitmask & (1 << i_bit)) != 0)
		{
			return sample_count;
		}
	}
	return 1;
}

int VkR3rImpl::choose_sample_count(R3rAaType aa_type, int aa_degree) const
{
	switch (aa_type)
	{
		case R3rAaType::ms:
			break;
		case R3rAaType::none:
		default:
			return 1;
	}
	const int max_sample_count = std::min(aa_degree, get_max_sample_count());
	for (int i_bit = 6; i_bit >= 0; --i_bit)
	{
		const int sample_count = 1 << i_bit;
		if ((context_.sample_count_bitmask & sample_count) != 0 &&
			sample_count <= max_sample_count)
		{
			return sample_count;
		}
	}
	return 1;
}

VkPresentModeKHR VkR3rImpl::choose_present_mode(bool enable_vsync) const
{
	const bool can_control_vsync = context_.has_vk_present_mode_fifo_khr && context_.has_vk_present_mode_immediate_khr;
	if (can_control_vsync)
	{
		if (enable_vsync)
		{
			return VK_PRESENT_MODE_FIFO_KHR;
		}
		else
		{
			return VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}
	else
	{
		if (context_.has_vk_present_mode_fifo_khr)
		{
			return VK_PRESENT_MODE_FIFO_KHR;
		}
		else
		{
			return VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}
}

void VkR3rImpl::wait_for_device() const
{
	const VkResult vk_result = context_.vkDeviceWaitIdle(
		/* device */ context_.device.get()
	);
	ensure_vk_result(vk_result, "vkDeviceWaitIdle");
}

VkR3rSemaphoreResource VkR3rImpl::make_semaphore() const
{
	const VkSemaphoreCreateInfo vk_semaphore_create_info
	{
		/* sType */ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		/* pNext */ nullptr,
		/* flags */ VkSemaphoreCreateFlags{},
	};
	VkSemaphore vk_semaphore{};
	const VkResult vk_result = context_.vkCreateSemaphore(
		/* device */      context_.device.get(),
		/* pCreateInfo */ &vk_semaphore_create_info,
		/* pAllocator */  nullptr,
		/* pSemaphore */  &vk_semaphore
	);
	ensure_vk_result(vk_result, "vkCreateSemaphore");
	return VkR3rSemaphoreResource{vk_semaphore, VkR3rSemaphoreDeleter{context_}};
}

VkR3rFenceResource VkR3rImpl::make_fence(FenceState initial_state) const
{
	const VkFenceCreateFlags vk_fence_create_flags =
		initial_state == FenceState::signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
	const VkFenceCreateInfo vk_fence_create_info
	{
		/* sType */ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		/* pNext */ nullptr,
		/* flags */ vk_fence_create_flags,
	};
	VkFence vk_fence{};
	const VkResult vk_result = context_.vkCreateFence(
		/* device */      context_.device.get(),
		/* pCreateInfo */ &vk_fence_create_info,
		/* pAllocator */  nullptr,
		/* pFence */      &vk_fence
	);
	ensure_vk_result(vk_result, "vkCreateFence");
	return VkR3rFenceResource{vk_fence, VkR3rFenceDeleter{context_}};
}

VkR3rFenceResource VkR3rImpl::make_unsignaled_fence() const
{
	return make_fence(FenceState::unsignaled);
}

VkR3rFenceResource VkR3rImpl::make_signaled_fence() const
{
	return make_fence(FenceState::signaled);
}

void VkR3rImpl::reset_fence(VkFence vk_fence) const
{
	const VkResult vk_result = context_.vkResetFences(
		/* device */     context_.device.get(),
		/* fenceCount */ 1,
		/* pFences */    &vk_fence
	);
	ensure_vk_result(vk_result, "vkResetFences");
}

void VkR3rImpl::wait_for_fence(VkFence vk_fence) const
{
	const VkResult vk_result = context_.vkWaitForFences(
		/* device */     context_.device.get(),
		/* fenceCount */ 1,
		/* pFences */    &vk_fence,
		/* waitAll */    true,
		/* timeout */    UINT64_MAX
	);
	ensure_vk_result(vk_result, "vkWaitForFences");
}

void VkR3rImpl::begin_submit_commands()
{
	VkResult vk_result;
	vk_result = context_.vkResetCommandBuffer(
		/* commandBuffer */ context_.command_buffer,
		/* flags */         VkCommandBufferResetFlags{}
	);
	ensure_vk_result(vk_result, "vkResetCommandBuffer");
	const VkCommandBufferBeginInfo vk_command_buffer_begin_info
	{
		/* sType */            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		/* pNext */            nullptr,
		/* flags */            VkCommandBufferUsageFlags{},
		/* pInheritanceInfo */ nullptr,
	};
	vk_result = context_.vkBeginCommandBuffer(
		/* commandBuffer */ context_.command_buffer,
		/* pBeginInfo */    &vk_command_buffer_begin_info
	);
	ensure_vk_result(vk_result, "vkBeginCommandBuffer");

	const VkRenderPassBeginInfo vk_render_pass_begin_info
	{
		/* sType */           VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		/* pNext */           nullptr,
		/* renderPass */      context_.render_pass.get(),
		/* framebuffer */     context_.offscreen_framebuffer.get(),
		/* renderArea */      VkRect2D
		                      {
		                          /* offset */ VkOffset2D
		                                       {
		                                           /* x */      0,
		                                           /* y */      0,
		                                       },
		                          /* extent */ VkExtent2D
		                                       {
		                                           /* width */  context_.vk_offscreen_width,
		                                           /* height */ context_.vk_offscreen_height,
		                                       },
		                      },
		/* clearValueCount */ 0,
		/* pClearValues */    nullptr,
	};
	context_.vkCmdBeginRenderPass(context_.command_buffer, &vk_render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
}

void VkR3rImpl::end_submit_commands()
{
	VkResult vk_result;
	context_.vkCmdEndRenderPass(
		/* commandBuffer */ context_.command_buffer
	);
	vk_result = context_.vkEndCommandBuffer(
		/* commandBuffer */ context_.command_buffer
	);
	ensure_vk_result(vk_result, "vkEndCommandBuffer");

	std::uint32_t wait_semaphore_count = 0;
	VkSemaphore wait_semaphores[1]{};
	const VkSemaphore* wait_semaphores_ptr = nullptr;
	std::uint32_t signal_semaphore_count = 0;
	VkSemaphore signal_semaphores[1]{};
	const VkSemaphore* signal_semaphores_ptr = nullptr;
	if (context_.has_swapchain())
	{
		wait_semaphore_count = 1;
		wait_semaphores[0] = context_.image_available_semaphore.get();
		wait_semaphores_ptr = wait_semaphores;
		signal_semaphore_count = 1;
		signal_semaphores[0] = context_.render_finished_semaphores[context_.swapchain_image_index].get();
		signal_semaphores_ptr = signal_semaphores;
	}
	const VkPipelineStageFlags vk_pipeline_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	const VkSubmitInfo vk_submit_info
	{
		/* sType */                VK_STRUCTURE_TYPE_SUBMIT_INFO,
		/* pNext */                nullptr,
		/* waitSemaphoreCount */   wait_semaphore_count,
		/* pWaitSemaphores */      wait_semaphores_ptr,
		/* pWaitDstStageMask */    &vk_pipeline_stage_flags,
		/* commandBufferCount */   1,
		/* pCommandBuffers */      &context_.command_buffer,
		/* signalSemaphoreCount */ signal_semaphore_count,
		/* pSignalSemaphores */    signal_semaphores_ptr,
	};
	vk_result = context_.vkQueueSubmit(
		/* queue */       context_.queue,
		/* submitCount */ 1,
		/* pSubmits */    &vk_submit_info,
		/* fence */       context_.in_flight_fence.get()
	);
	ensure_vk_result(vk_result, "vkQueueSubmit");
}

void VkR3rImpl::initialize_defaults()
{
	context_.vk_surface_width = 0;
	context_.vk_surface_height = 0;
	context_.vk_offscreen_width = static_cast<std::uint32_t>(R3rLimits::min_viewport_width());
	context_.vk_offscreen_height = static_cast<std::uint32_t>(R3rLimits::min_viewport_height());
}

void VkR3rImpl::initialize_vulkan_manager()
{
	sys::VulkanMgr& vulkan_mgr = video_mgr_.get_vulkan_mgr();
	if (!vulkan_mgr.is_vulkan_available())
	{
		BSTONE_THROW_STATIC_SOURCE("Vulkan not available.");
	}
}

void VkR3rImpl::initialize_window()
{
	sys::WindowInitParam param{};
	param.x = sys::WindowOffset::make_centered();
	param.y = sys::WindowOffset::make_centered();
	param.width = static_cast<int>(context_.vk_offscreen_width);
	param.height = static_cast<int>(context_.vk_offscreen_height);
	param.rounded_corner_type = sys::WindowRoundedCornerType::none;
	param.renderer_type = sys::WindowRendererType::vulkan;
	window_ = window_mgr_.make_window(param);
}

void VkR3rImpl::initialize_global_symbols()
{
	context_.vkGetInstanceProcAddr = std::bit_cast<PFN_vkGetInstanceProcAddr>(video_mgr_.get_vulkan_mgr().get_instance_proc_addr());
	if (context_.vkGetInstanceProcAddr == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Symbol \"vkGetInstanceProcAddr\" not found.");
	}
	resolve_symbol(BSTONE_STRCTX(vkCreateInstance));
	resolve_symbol(BSTONE_STRCTX(vkEnumerateInstanceExtensionProperties));
	resolve_symbol(BSTONE_STRCTX(vkEnumerateInstanceLayerProperties));
}

void VkR3rImpl::initialize_validation_layers()
{
#ifndef NDEBUG
	info_.log_validation_layers();
	const auto layers = vk_r3r_extract_array(context_.vkEnumerateInstanceLayerProperties);
	for (const VkLayerProperties& layer : layers)
	{
		if (std::strcmp(layer.layerName, context_.vk_layer_khronos_validation_extension_name) == 0)
		{
			context_.has_vk_layer_khronos_validation = true;
		}
	}
#endif // NDEBUG
}

void VkR3rImpl::initialize_global_extensions()
{
	info_.log_extensions();
	const auto extensions = vk_r3r_extract_array(context_.vkEnumerateInstanceExtensionProperties, nullptr);
	for (const VkExtensionProperties& extension : extensions)
	{
		if (std::strcmp(extension.extensionName, context_.vk_ext_debug_utils_extension_name) == 0)
		{
			context_.has_ext_debug_utils = true;
		}
	}
}

void VkR3rImpl::initialize_enabled_global_layers()
{
	context_.enabled_layers.clear();
	context_.enabled_layers.reserve(16);
#ifndef NDEBUG
	if (context_.has_vk_layer_khronos_validation)
	{
		context_.enabled_layers.emplace_back(context_.vk_layer_khronos_validation_extension_name);
	}
#endif // NDEBUG
	info_.log_enabled_validation_layers();
}

void VkR3rImpl::initialize_enabled_global_extensions()
{
#ifndef NDEBUG
	if (context_.has_ext_debug_utils)
	{
		context_.enabled_extensions.emplace_back(context_.vk_ext_debug_utils_extension_name);
	}
#endif // NDEBUG
	std::span<const char* const> sys_required_extensions = video_mgr_.get_vulkan_mgr().get_required_extensions(*window_);
	context_.enabled_extensions.reserve(
		context_.enabled_extensions.size() + sys_required_extensions.size());
	context_.enabled_extensions.insert(
		context_.enabled_extensions.cend(),
		sys_required_extensions.begin(),
		sys_required_extensions.end());
	info_.log_enabled_extensions();
}

void VkR3rImpl::initialize_instance()
{
	initialize_enabled_global_layers();
	initialize_enabled_global_extensions();
	const VkApplicationInfo vk_application_info
	{
		/* sType */              VK_STRUCTURE_TYPE_APPLICATION_INFO,
		/* pNext */              nullptr,
		/* pApplicationName */   nullptr,
		/* applicationVersion */ 0,
		/* pEngineName */        nullptr,
		/* engineVersion */      0,
		/* apiVersion */         VK_API_VERSION_1_0,
	};
#ifndef NDEBUG
	const VkDebugUtilsMessengerCreateInfoEXT vk_debug_utils_messenger_create_info
	{
		/* sType */           VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		/* pNext */           nullptr,
		/* flags */           VkDebugUtilsMessengerCreateFlagsEXT{},
		/* messageSeverity */ VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		/* messageType */     VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		                          VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		                          VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		/* pfnUserCallback */ vk_debug_utils_messenger_callback,
		/* pUserData */       &logger_,
	};
#endif // NDEBUG
	VkInstanceCreateInfo vk_instance_create_info
	{
		/* sType */                   VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		/* pNext */                   nullptr,
		/* flags */                   VkInstanceCreateFlags{},
		/* pApplicationInfo */        nullptr,
		/* enabledLayerCount */       0,
		/* ppEnabledLayerNames */     nullptr,
		/* enabledExtensionCount */   0,
		/* ppEnabledExtensionNames */ nullptr,
	};
#ifndef NDEBUG
	if (context_.has_ext_debug_utils)
	{
		vk_instance_create_info.pNext = &vk_debug_utils_messenger_create_info;
	}
#endif // NDEBUG
	vk_instance_create_info.pApplicationInfo = &vk_application_info;
	vk_instance_create_info.enabledLayerCount = static_cast<std::uint32_t>(context_.enabled_layers.size());
	vk_instance_create_info.ppEnabledLayerNames = context_.enabled_layers.data();
	vk_instance_create_info.enabledExtensionCount = static_cast<std::uint32_t>(context_.enabled_extensions.size());
	vk_instance_create_info.ppEnabledExtensionNames = context_.enabled_extensions.data();
	VkInstance instance{};
	const VkResult vk_result = context_.vkCreateInstance(
		/* pCreateInfo */ &vk_instance_create_info,
		/* pAllocator */  nullptr,
		/* pInstance */   &instance
	);
	ensure_vk_result(vk_result, "vkCreateInstance");
	context_.instance = VkR3rInstanceResource{instance, VkR3rInstanceDeleter{context_}};

	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkDestroyInstance));
}

void VkR3rImpl::initialize_instance_symbols()
{
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkAcquireNextImageKHR));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkAllocateCommandBuffers));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkAllocateDescriptorSets));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkAllocateMemory));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkBeginCommandBuffer));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkBindBufferMemory));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkBindImageMemory));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCmdBeginRenderPass));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCmdBindDescriptorSets));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCmdBindIndexBuffer));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCmdBindPipeline));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCmdBindVertexBuffers));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCmdBlitImage));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCmdClearAttachments));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCmdCopyBuffer));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCmdCopyBufferToImage));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCmdCopyImageToBuffer));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCmdDrawIndexed));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCmdEndRenderPass));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCmdPipelineBarrier));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCmdResolveImage));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCmdSetScissor));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCmdSetViewport));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCreateBuffer));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCreateCommandPool));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCreateDescriptorPool));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCreateDescriptorSetLayout));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCreateDevice));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCreateFence));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCreateFramebuffer));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCreateGraphicsPipelines));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCreateImage));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCreateImageView));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCreatePipelineLayout));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCreateRenderPass));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCreateSampler));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCreateSemaphore));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCreateShaderModule));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCreateSwapchainKHR));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkDestroyBuffer));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkDestroyCommandPool));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkDestroyDescriptorPool));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkDestroyDescriptorSetLayout));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkDestroyDevice));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkDestroyFence));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkDestroyFramebuffer));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkDestroyImage));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkDestroyImageView));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkDestroyPipeline));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkDestroyPipelineLayout));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkDestroyRenderPass));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkDestroySampler));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkDestroySemaphore));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkDestroyShaderModule));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkDestroySurfaceKHR));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkDestroySwapchainKHR));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkDeviceWaitIdle));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkEndCommandBuffer));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkEnumerateDeviceExtensionProperties));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkEnumeratePhysicalDevices));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkFreeCommandBuffers));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkFreeDescriptorSets));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkFreeMemory));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkGetBufferMemoryRequirements));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkGetDeviceQueue));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkGetImageMemoryRequirements));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkGetPhysicalDeviceFeatures));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkGetPhysicalDeviceMemoryProperties));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkGetPhysicalDeviceFormatProperties));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkGetPhysicalDeviceProperties));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkGetPhysicalDeviceQueueFamilyProperties));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkGetPhysicalDeviceSurfaceCapabilitiesKHR));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkGetPhysicalDeviceSurfaceFormatsKHR));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkGetPhysicalDeviceSurfacePresentModesKHR));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkGetPhysicalDeviceSurfaceSupportKHR));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkGetSwapchainImagesKHR));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkMapMemory));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkQueuePresentKHR));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkQueueSubmit));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkQueueWaitIdle));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkResetCommandBuffer));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkResetFences));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkUnmapMemory));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkUpdateDescriptorSets));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkWaitForFences));
}

void VkR3rImpl::initialize_debug_utils_messenger()
{
#ifndef NDEBUG
	if (!context_.has_ext_debug_utils)
	{
		return;
	}
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkCreateDebugUtilsMessengerEXT));
	resolve_symbol(context_.instance.get(), BSTONE_STRCTX(vkDestroyDebugUtilsMessengerEXT));
	const VkDebugUtilsMessengerCreateInfoEXT vk_debug_utils_messenger_create_info_ext
	{
		/* sType */           VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		/* pNext */           nullptr,
		/* flags */           VkDebugUtilsMessengerCreateFlagsEXT{},
		/* messageSeverity */ VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		                          VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
		/* messageType */     VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		                          VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		                          VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
		/* pfnUserCallback */ vk_debug_utils_messenger_callback,
		/* pUserData */       &logger_,
	};
	VkDebugUtilsMessengerEXT vk_debug_utils_messenger_ext{};
	const VkResult vk_result = context_.vkCreateDebugUtilsMessengerEXT(
		/* instance */    context_.instance.get(),
		/* pCreateInfo */ &vk_debug_utils_messenger_create_info_ext,
		/* pAllocator */  nullptr,
		/* pMessenger */  &vk_debug_utils_messenger_ext
	);
	ensure_vk_result(vk_result, "vkCreateDebugUtilsMessengerEXT");
	context_.debug_utils_messenger = VkR3rDebugUtilsMessengerResource{
		vk_debug_utils_messenger_ext, VkR3rDebugUtilsMessengerDeleter{context_}};
#endif
}

void VkR3rImpl::terminate_surface()
{
	context_.surface = vk_r3r_resource_null;
}

void VkR3rImpl::initialize_surface()
{
	sys::VulkanMgr& vulkan_mgr = video_mgr_.get_vulkan_mgr();
	context_.surface = VkR3rSurfaceKhrResource{
		vulkan_mgr.create_surface(*window_, context_.instance.get()),
		VkR3rSurfaceKhrDeleter{context_}};
}

void VkR3rImpl::update_surface_capabilities()
{
	const VkResult vk_result = context_.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		/* physicalDevice */       context_.physical_device,
		/* surface */              context_.surface.get(),
		/* pSurfaceCapabilities */ &context_.surface_capabilities
	);
	ensure_vk_result(vk_result, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
	if (context_.surface_capabilities.currentExtent.width == 0 &&
		context_.surface_capabilities.currentExtent.height == 0)
	{
		context_.vk_surface_width = 0;
		context_.vk_surface_height = 0;
	}
	else if (context_.surface_capabilities.currentExtent.width == UINT32_MAX &&
		context_.surface_capabilities.currentExtent.height == UINT32_MAX)
	{
		context_.vk_surface_width = context_.vk_offscreen_width;
		context_.vk_surface_height = context_.vk_offscreen_height;
	}
	else
	{
		if (context_.surface_capabilities.currentExtent.width == 0 ||
			context_.surface_capabilities.currentExtent.width == UINT32_MAX ||
			context_.surface_capabilities.currentExtent.width < context_.surface_capabilities.minImageExtent.width ||
			context_.surface_capabilities.currentExtent.width > context_.surface_capabilities.maxImageExtent.width ||
			context_.surface_capabilities.currentExtent.height == 0 ||
			context_.surface_capabilities.currentExtent.height == UINT32_MAX ||
			context_.surface_capabilities.currentExtent.height < context_.surface_capabilities.minImageExtent.height ||
			context_.surface_capabilities.currentExtent.height > context_.surface_capabilities.maxImageExtent.height)
		{
			BSTONE_THROW_STATIC_SOURCE("Invalid surface extent.");
		}
		context_.vk_surface_width = context_.surface_capabilities.currentExtent.width;
		context_.vk_surface_height = context_.surface_capabilities.currentExtent.height;
	}
}

void VkR3rImpl::choose_physical_device()
{
	info_.log_physical_devices();
	const auto physical_devices = vk_r3r_extract_array(context_.vkEnumeratePhysicalDevices, context_.instance.get());
	if (physical_devices.empty())
	{
		BSTONE_THROW_STATIC_SOURCE("No Vulkan physical devices.");
	}
	for (const VkPhysicalDevice& physical_device : physical_devices)
	{
		bool has_suitable_queue_family_index = false;
		const auto physical_device_queue_families = vk_r3r_extract_array(
			context_.vkGetPhysicalDeviceQueueFamilyProperties,
			physical_device);
		VkResult vk_result;
		for (const VkQueueFamilyProperties& queue_family : physical_device_queue_families)
		{
			const std::uint32_t i_family = static_cast<std::uint32_t>(&queue_family - &(*physical_device_queue_families.begin()));
			VkBool32 vk_has_presentation_queue_family = false;
			vk_result = context_.vkGetPhysicalDeviceSurfaceSupportKHR(
				/* physicalDevice */   physical_device,
				/* queueFamilyIndex */ i_family,
				/* surface */          context_.surface.get(),
				/* pSupported */       &vk_has_presentation_queue_family
			);
			ensure_vk_result(vk_result, "vkGetPhysicalDeviceSurfaceSupportKHR");

			const bool vk_has_gfx_queue_family = (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;
			if (vk_has_presentation_queue_family && vk_has_gfx_queue_family)
			{
				has_suitable_queue_family_index = true;
				context_.queue_family_index = i_family;
				break;
			}
		}
		const auto physical_device_extensions = vk_r3r_extract_array(
			context_.vkEnumerateDeviceExtensionProperties, physical_device, nullptr);
		bool has_vk_khr_swapchain = false;
		for (const VkExtensionProperties& extension : physical_device_extensions)
		{
			if (std::strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
			{
				has_vk_khr_swapchain = true;
				break;
			}
		}

		vk_result = context_.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			/* physicalDevice */       physical_device,
			/* surface */              context_.surface.get(),
			/* pSurfaceCapabilities */ &context_.surface_capabilities
		);
		ensure_vk_result(vk_result, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");
		constexpr VkImageUsageFlags required_surface_usage_flags = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		const bool has_suitable_surface_usage_flags =
			(context_.surface_capabilities.supportedUsageFlags & required_surface_usage_flags) ==
				required_surface_usage_flags;
		bool has_suitable_surface_format = false;
		const auto surface_formats = vk_r3r_extract_array(
			context_.vkGetPhysicalDeviceSurfaceFormatsKHR, physical_device, context_.surface.get());
		VkFormat surface_format = VK_FORMAT_UNDEFINED;
		for (const VkSurfaceFormatKHR& format : surface_formats)
		{
			if ((format.format == VK_FORMAT_R8G8B8A8_UNORM || format.format == VK_FORMAT_B8G8R8A8_UNORM) &&
				format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				surface_format = format.format;
				has_suitable_surface_format = true;
				break;
			}
		}

		bool has_vk_present_mode_immediate_khr = false;
		bool has_vk_present_mode_fifo_khr = false;
		const auto present_modes = vk_r3r_extract_array(
			context_.vkGetPhysicalDeviceSurfacePresentModesKHR, physical_device, context_.surface.get());
		for (const VkPresentModeKHR& present_mode : present_modes)
		{
			switch (present_mode)
			{
				case VK_PRESENT_MODE_IMMEDIATE_KHR:
					has_vk_present_mode_immediate_khr = true;
					break;
				case VK_PRESENT_MODE_FIFO_KHR:
					has_vk_present_mode_fifo_khr = true;
					break;
			}
		}
		const bool has_suitable_present_mode = has_vk_present_mode_immediate_khr || has_vk_present_mode_fifo_khr;

		if (has_suitable_queue_family_index &&
			has_vk_khr_swapchain &&
			has_suitable_surface_usage_flags &&
			has_suitable_surface_format &&
			has_suitable_present_mode)
		{
			context_.has_vk_present_mode_immediate_khr = has_vk_present_mode_immediate_khr;
			context_.has_vk_present_mode_fifo_khr = has_vk_present_mode_fifo_khr;
			context_.surface_format = surface_format;
			context_.physical_device = physical_device;
			break;
		}
	}
	if (context_.physical_device == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Could not found a suitable Vulkan physical device.");
	}
}

void VkR3rImpl::initialize_physical_device_features()
{
	context_.vkGetPhysicalDeviceFeatures(
		/* physicalDevice */ context_.physical_device,
		/* pFeatures */      &context_.physical_device_features
	);
}

void VkR3rImpl::initialize_physical_device_properties()
{
	context_.vkGetPhysicalDeviceProperties(
		/* physicalDevice */ context_.physical_device,
		/* pProperties */    &context_.physical_device_properties
	);
}

void VkR3rImpl::initialize_physical_device_memory_properties()
{
	context_.vkGetPhysicalDeviceMemoryProperties(
		/* physicalDevice */    context_.physical_device,
		/* pMemoryProperties */ &context_.memory_properties
	);
}

void VkR3rImpl::initialize_enabled_device_extensions()
{
	context_.enabled_device_extensions.clear();
	context_.enabled_device_extensions.reserve(4);
	context_.enabled_device_extensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

void VkR3rImpl::initialize_logical_device()
{
	initialize_enabled_device_extensions();

	const float queue_priorities[1] = {1.0F};
	const VkDeviceQueueCreateInfo vk_device_queue_create_info
	{
		/* sType */            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		/* pNext */            nullptr,
		/* flags */            VkDeviceQueueCreateFlags{},
		/* queueFamilyIndex */ context_.queue_family_index,
		/* queueCount */       1,
		/* pQueuePriorities */ queue_priorities,
	};
	VkPhysicalDeviceFeatures vk_physical_device_features{};
	vk_physical_device_features.samplerAnisotropy = context_.physical_device_features.samplerAnisotropy;
	const VkDeviceCreateInfo vk_device_create_info
	{
		/* sType */                   VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		/* pNext */                   nullptr,
		/* flags */                   VkDeviceCreateFlags{},
		/* queueCreateInfoCount */    1,
		/* pQueueCreateInfos */       &vk_device_queue_create_info,
		/* enabledLayerCount */       static_cast<std::uint32_t>(context_.enabled_layers.size()),
		/* ppEnabledLayerNames */     context_.enabled_layers.data(),
		/* enabledExtensionCount */   static_cast<std::uint32_t>(context_.enabled_device_extensions.size()),
		/* ppEnabledExtensionNames */ context_.enabled_device_extensions.data(),
		/* pEnabledFeatures */        &vk_physical_device_features,
	};
	VkDevice vk_device{};
	const VkResult vk_result = context_.vkCreateDevice(
		/* physicalDevice */ context_.physical_device,
		/* pCreateInfo */    &vk_device_create_info,
		/* pAllocator */     nullptr,
		/* pDevice */        &vk_device
	);
	ensure_vk_result(vk_result, "vkCreateDevice");

	context_.device = VkR3rDeviceResource{vk_device, VkR3rDeviceDeleter{context_}};
	context_.vkGetDeviceQueue(
		/* device */           vk_device,
		/* queueFamilyIndex */ context_.queue_family_index,
		/* queueIndex */       0,
		/* pQueue */           &context_.queue
	);
}

void VkR3rImpl::terminate_swapchain()
{
	context_.old_swapchain = std::move(context_.swapchain);
}

void VkR3rImpl::initialize_swapchain()
{
	if (context_.vk_surface_width == 0 || context_.vk_surface_height == 0)
	{
		context_.swapchain_image_index = UINT32_MAX;
		return;
	}
	const std::uint32_t vk_queue_family_indices[1] = {context_.queue_family_index};
	const VkSwapchainCreateInfoKHR vk_swapchain_create_info_khr
	{
		/* sType */                 VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		/* pNext */                 nullptr,
		/* flags */                 VkSwapchainCreateFlagsKHR{},
		/* surface */               context_.surface.get(),
		/* minImageCount */         context_.surface_capabilities.minImageCount,
		/* imageFormat */           context_.surface_format,
		/* imageColorSpace */       VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
		/* imageExtent */           VkExtent2D
		                            {
		                                /* width */  context_.vk_surface_width,
		                                /* height */ context_.vk_surface_height,
		                            },
		/* imageArrayLayers */      1,
		/* imageUsage */            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		/* imageSharingMode */      VK_SHARING_MODE_EXCLUSIVE,
		/* queueFamilyIndexCount */ 1,
		/* pQueueFamilyIndices */   vk_queue_family_indices,
		/* preTransform */          context_.surface_capabilities.currentTransform,
		/* compositeAlpha */        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		/* presentMode */           context_.vk_present_mode_khr,
		/* clipped */               true,
		/* oldSwapchain */          context_.old_swapchain.get(),
	};
	VkSwapchainKHR vk_swapchain_khr{};
	const VkResult vk_result = context_.vkCreateSwapchainKHR(
		/* device */      context_.device.get(),
		/* pCreateInfo */ &vk_swapchain_create_info_khr,
		/* pAllocator */  nullptr,
		/* pSwapchain */  &vk_swapchain_khr
	);
	ensure_vk_result(vk_result, "vkCreateSwapchainKHR");
	context_.swapchain = VkR3rSwapchainKhrResource{vk_swapchain_khr, VkR3rSwapchainKhrDeleter{context_}};
	context_.old_swapchain = vk_r3r_resource_null;
}

void VkR3rImpl::terminate_swapchain_images()
{
	context_.swapchain_images.clear();
	context_.swapchain_image_layouts.clear();
}

void VkR3rImpl::initialize_swapchain_images()
{
	vk_r3r_extract_array(
		context_.vkGetSwapchainImagesKHR,
		context_.swapchain_images,
		context_.device.get(),
		context_.swapchain.get());
	context_.swapchain_image_layouts.resize(context_.swapchain_images.size(), VK_IMAGE_LAYOUT_UNDEFINED);
}

void VkR3rImpl::terminate_swapchain_image_views()
{
	context_.swapchain_image_views.clear();
}

void VkR3rImpl::initialize_swapchain_image_views()
{
	context_.swapchain_image_views.clear();
	context_.swapchain_image_views.reserve(context_.swapchain_images.size());
	for (const VkImage& vk_image : context_.swapchain_images)
	{
		context_.swapchain_image_views.emplace_back(
			std::move(
				context_.create_image_view_resource(
					vk_image,
					context_.surface_format,
					VK_IMAGE_ASPECT_COLOR_BIT)));
	}
}

void VkR3rImpl::terminate_render_pass()
{
	context_.render_pass = vk_r3r_resource_null;
}

void VkR3rImpl::initialize_render_pass()
{
	using AttachmentDescriptions = std::array<VkAttachmentDescription, VkR3rContext::total_attachments>;
	AttachmentDescriptions vk_attachment_descriptions{};
	// Color.
	vk_attachment_descriptions[0] = VkAttachmentDescription
	{
		/* flags */          VkAttachmentDescriptionFlags{},
		/* format */         context_.surface_format,
		/* samples */        static_cast<VkSampleCountFlagBits>(context_.sample_count),
		/* loadOp */         VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		/* storeOp */        VK_ATTACHMENT_STORE_OP_STORE,
		/* stencilLoadOp */  VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		/* stencilStoreOp */ VK_ATTACHMENT_STORE_OP_DONT_CARE,
		/* initialLayout */  VK_IMAGE_LAYOUT_UNDEFINED,
		/* finalLayout */    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	};
	// Depth-stencil.
	vk_attachment_descriptions[1] = VkAttachmentDescription
	{
		/* flags */          VkAttachmentDescriptionFlags{},
		/* format */         VK_FORMAT_D16_UNORM,
		/* samples */        static_cast<VkSampleCountFlagBits>(context_.sample_count),
		/* loadOp */         VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		/* storeOp */        VK_ATTACHMENT_STORE_OP_STORE,
		/* stencilLoadOp */  VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		/* stencilStoreOp */ VK_ATTACHMENT_STORE_OP_DONT_CARE,
		/* initialLayout */  VK_IMAGE_LAYOUT_UNDEFINED,
		/* finalLayout */    VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};
	//
	const VkAttachmentReference vk_color_attachment_reference
	{
		/* attachment */ VkR3rContext::color_attachment_index,
		/* layout */     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};
	const VkAttachmentReference vk_depth_attachment_reference
	{
		/* attachment */ VkR3rContext::depth_attachment_index,
		/* layout */     VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};
	const VkSubpassDescription vk_subpass_description
	{
		/* flags */                   VkSubpassDescriptionFlags{},
		/* pipelineBindPoint */       VK_PIPELINE_BIND_POINT_GRAPHICS,
		/* inputAttachmentCount */    0,
		/* pInputAttachments */       nullptr,
		/* colorAttachmentCount */    1,
		/* pColorAttachments */       &vk_color_attachment_reference,
		/* pResolveAttachments */     nullptr,
		/* pDepthStencilAttachment */ &vk_depth_attachment_reference,
		/* preserveAttachmentCount */ 0,
		/* pPreserveAttachments */    nullptr,
	};
	const VkSubpassDependency vk_subpass_dependency
	{
		/* srcSubpass */      VK_SUBPASS_EXTERNAL,
		/* dstSubpass */      0,
		/* srcStageMask */    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		/* dstStageMask */    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		/* srcAccessMask */   VkAccessFlags{},
		/* dstAccessMask */   VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		/* dependencyFlags */ VkDependencyFlags{},
	};
	const VkRenderPassCreateInfo vk_render_pass_create_info
	{
		/* sType */           VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		/* pNext */           nullptr,
		/* flags */           VkRenderPassCreateFlags{},
		/* attachmentCount */ static_cast<std::uint32_t>(vk_attachment_descriptions.size()),
		/* pAttachments */    vk_attachment_descriptions.data(),
		/* subpassCount */    1,
		/* pSubpasses */      &vk_subpass_description,
		/* dependencyCount */ 1,
		/* pDependencies */   &vk_subpass_dependency,
	};
	VkRenderPass vk_render_pass{};
	const VkResult vk_result = context_.vkCreateRenderPass(
		/* device */      context_.device.get(),
		/* pCreateInfo */ &vk_render_pass_create_info,
		/* pAllocator */  nullptr,
		/* pRenderPass */ &vk_render_pass
	);
	ensure_vk_result(vk_result, "vkCreateRenderPass");
	context_.render_pass = VkR3rRenderPassResource{vk_render_pass, VkR3rRenderPassDeleter{context_}};
}

void VkR3rImpl::terminate_offscreen_framebuffer()
{
	context_.offscreen_framebuffer = vk_r3r_resource_null;
	context_.offscreen_color_image_view = vk_r3r_resource_null;
	context_.offscreen_color_image = vk_r3r_resource_null;
	context_.offscreen_color_image_memory = vk_r3r_resource_null;
	context_.offscreen_depth_image_view = vk_r3r_resource_null;
	context_.offscreen_depth_image = vk_r3r_resource_null;
	context_.offscreen_depth_image_memory = vk_r3r_resource_null;
}

void VkR3rImpl::initialize_offscreen_framebuffer()
{
	context_.create_image_resource(
		context_.surface_format,
		context_.vk_offscreen_width,
		context_.vk_offscreen_height,
		1,
		static_cast<VkSampleCountFlagBits>(context_.sample_count),
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		context_.offscreen_color_image,
		context_.offscreen_color_image_memory
	);
	context_.offscreen_color_image_view = context_.create_image_view_resource(
		context_.offscreen_color_image.get(),
		context_.surface_format,
		VK_IMAGE_ASPECT_COLOR_BIT
	);
	context_.create_image_resource(
		VK_FORMAT_D16_UNORM,
		context_.vk_offscreen_width,
		context_.vk_offscreen_height,
		1,
		static_cast<VkSampleCountFlagBits>(context_.sample_count),
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		context_.offscreen_depth_image,
		context_.offscreen_depth_image_memory
	);
	context_.offscreen_depth_image_view = context_.create_image_view_resource(
		context_.offscreen_depth_image.get(),
		VK_FORMAT_D16_UNORM,
		VK_IMAGE_ASPECT_DEPTH_BIT
	);
	VkImageView const attachments[2] =
	{
		context_.offscreen_color_image_view.get(),
		context_.offscreen_depth_image_view.get(),
	};
	const VkFramebufferCreateInfo vk_framebuffer_create_info
	{
		/* sType */           VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		/* pNext */           nullptr,
		/* flags */           VkFramebufferCreateFlags{},
		/* renderPass */      context_.render_pass.get(),
		/* attachmentCount */ static_cast<std::uint32_t>(std::extent<decltype(attachments)>::value),
		/* pAttachments */    attachments,
		/* width */           context_.vk_offscreen_width,
		/* height */          context_.vk_offscreen_height,
		/* layers */          1,
	};
	VkFramebuffer vk_framebuffer{};
	const VkResult vk_result = context_.vkCreateFramebuffer(
		/* device */ context_.device.get(),
		/* pCreateInfo */ &vk_framebuffer_create_info,
		/* pAllocator */ nullptr,
		/* pFramebuffer */ &vk_framebuffer
	);
	context_.ensure_success_vk_result(vk_result, "vkCreateFramebuffer");
	context_.offscreen_framebuffer.reset(vk_framebuffer, VkR3rFramebufferDeleter{context_});
}

void VkR3rImpl::terminate_screenshot_buffer()
{
	context_.screenshot_buffer = vk_r3r_resource_null;
}

void VkR3rImpl::initialize_screenshot_buffer()
{
	const VkDeviceSize vk_buffer_size = VkDeviceSize{4} * context_.vk_offscreen_width * context_.vk_offscreen_height;
	context_.create_buffer_resource(
		vk_buffer_size,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		context_.screenshot_buffer,
		context_.screenshot_buffer_memory);
}

void VkR3rImpl::terminate_screenshot_image()
{
	context_.screenshot_image_memory = vk_r3r_resource_null;
	context_.screenshot_image = vk_r3r_resource_null;
}

void VkR3rImpl::initialize_screenshot_image()
{
	context_.create_image_resource(
		context_.surface_format,
		context_.vk_offscreen_width,
		context_.vk_offscreen_height,
		1,
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		context_.screenshot_image,
		context_.screenshot_image_memory);
}

void VkR3rImpl::initialize_transient_command_pool()
{
	context_.transient_command_pool = context_.create_command_pool(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
}

void VkR3rImpl::initialize_command_pool()
{
	context_.command_pool = context_.create_command_pool(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
}

void VkR3rImpl::initialize_command_buffer()
{
	const VkCommandBufferAllocateInfo vk_command_buffer_allocate_info
	{
		/* sType */              VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		/* pNext */              nullptr,
		/* commandPool */        context_.command_pool.get(),
		/* level */              VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		/* commandBufferCount */ 1,
	};
	const VkResult vk_result = context_.vkAllocateCommandBuffers(
		/* device */          context_.device.get(),
		/* pAllocateInfo */   &vk_command_buffer_allocate_info,
		/* pCommandBuffers */ &context_.command_buffer
	);
	ensure_vk_result(vk_result, "vkAllocateCommandBuffers");
}

void VkR3rImpl::terminate_swapchain_sync_objects()
{
	context_.render_finished_semaphores.clear();
}

void VkR3rImpl::initialize_swapchain_sync_objects()
{
	const std::size_t render_finished_count = context_.swapchain_image_views.size();
	context_.render_finished_semaphores.reserve(render_finished_count);
	for (std::uint32_t i = 0; i < render_finished_count; ++i)
	{
		context_.render_finished_semaphores.emplace_back(make_semaphore());
	}
}

void VkR3rImpl::initialize_sync_objects()
{
	initialize_swapchain_sync_objects();
	context_.image_available_semaphore = make_semaphore();
	context_.in_flight_fence = make_fence(FenceState::signaled);
}

void VkR3rImpl::initialize_descriptor_pool()
{
	const VkDescriptorPoolSize vk_sampler_descriptor_pool_size
	{
		/* type */            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		/* descriptorCount */ R3rLimits::max_textures(),
	};
	const VkDescriptorPoolSize vk_ubo_descriptor_pool_size
	{
		/* type */            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		/* descriptorCount */ R3rLimits::max_textures(),
	};
	const VkDescriptorPoolSize descriptor_pool_sizes[] =
	{
		vk_sampler_descriptor_pool_size,
		vk_ubo_descriptor_pool_size,
	};
	const VkDescriptorPoolCreateInfo vk_descriptor_pool_create_info
	{
		/* sType */         VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		/* pNext */         nullptr,
		/* flags */         VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
		/* maxSets */       R3rLimits::max_textures(),
		/* poolSizeCount */ static_cast<std::uint32_t>(std::extent<decltype(descriptor_pool_sizes)>::value),
		/* pPoolSizes */    descriptor_pool_sizes,
	};
	VkDescriptorPool vk_descriptor_pool{};
	const VkResult vk_result = context_.vkCreateDescriptorPool(
		/* device */          context_.device.get(),
		/* pCreateInfo */     &vk_descriptor_pool_create_info,
		/* pAllocator */      nullptr,
		/* pDescriptorPool */ &vk_descriptor_pool
	);
	context_.ensure_success_vk_result(vk_result, "vkCreateDescriptorPool");
	context_.descriptor_pool.reset(vk_descriptor_pool, VkR3rDescriptorPoolDeleter{context_});
}

void VkR3rImpl::initialize_pipeline_mgr()
{
	pipeline_mgr_ = make_vk_r3r_pipeline_mgr(context_);
}

void VkR3rImpl::initialize_sample_count(const R3rInitParam& r3r_init_param)
{
	const VkPhysicalDeviceProperties& vk_props = context_.physical_device_properties;
	const VkPhysicalDeviceLimits& vk_limits = vk_props.limits;
	// Intersect all required aspects.
	context_.sample_count_bitmask = vk_limits.framebufferColorSampleCounts & vk_limits.framebufferDepthSampleCounts;
	// Ensure at least one sample count.
	context_.sample_count_bitmask |= VK_SAMPLE_COUNT_1_BIT;
	// Apply the limit.
	context_.sample_count_bitmask &= R3rLimits::max_aa() - 1;
	BSTONE_ASSERT(context_.sample_count_bitmask != 0);
	context_.sample_count = choose_sample_count(r3r_init_param.aa_type, r3r_init_param.aa_value);
}

void VkR3rImpl::initialize_present_mode(const R3rInitParam& r3r_init_param)
{
	context_.vk_present_mode_khr = choose_present_mode(r3r_init_param.is_vsync);
}

void VkR3rImpl::initialize_r3r_device_features()
{
	R3rDeviceFeatures& r3r_features = context_.r3r_device_features;
	const VkPhysicalDeviceFeatures& vk_features = context_.physical_device_features;
	const VkPhysicalDeviceProperties& vk_props = context_.physical_device_properties;
	const VkPhysicalDeviceLimits& vk_limits = vk_props.limits;
	// VSync
	r3r_features.is_vsync_available = context_.has_vk_present_mode_fifo_khr && context_.has_vk_present_mode_immediate_khr;
	r3r_features.is_vsync_requires_restart = false;
	//
	BSTONE_ASSERT(vk_limits.maxImageDimension2D > 0);
	r3r_features.max_texture_dimension = static_cast<int>(std::min(
		vk_limits.maxImageDimension2D, std::uint32_t{INT32_MAX}));
	//
	BSTONE_ASSERT(vk_limits.maxViewportDimensions[0] > 0);
	r3r_features.max_viewport_width = static_cast<int>(std::min(
		vk_limits.maxViewportDimensions[0], std::uint32_t{INT32_MAX}));
	//
	BSTONE_ASSERT(vk_limits.maxViewportDimensions[1] > 0);
	r3r_features.max_viewport_height = static_cast<int>(std::min(
		vk_limits.maxViewportDimensions[1], std::uint32_t{INT32_MAX}));
	//
	if (vk_features.samplerAnisotropy == VK_TRUE)
	{
		BSTONE_ASSERT(vk_limits.maxSamplerAnisotropy > 1.0F);
		r3r_features.is_anisotropy_available = true;
		r3r_features.max_anisotropy_degree = static_cast<int>(vk_limits.maxSamplerAnisotropy);
	}
	else
	{
		r3r_features.is_anisotropy_available = false;
		r3r_features.max_anisotropy_degree = R3rLimits::min_anisotropy_off();
	}
	//
	r3r_features.is_npot_available = true;
	//
	r3r_features.is_mipmap_available = true;
	//
	r3r_features.is_sampler_available = true;
	//
	const int vk_sample_count = std::min(
		vk_props.limits.framebufferColorSampleCounts,
		vk_props.limits.framebufferDepthSampleCounts);
	const int sample_count = std::min(std::max(vk_sample_count, 1), R3rLimits::max_aa());
	r3r_features.is_msaa_available = (sample_count > 1);
	r3r_features.is_msaa_render_to_window = false;
	r3r_features.is_msaa_requires_restart = false;
	r3r_features.max_msaa_degree = get_max_sample_count();
	//
	r3r_features.max_vertex_input_locations = static_cast<int>(vk_limits.maxVertexInputAttributes);
}

void VkR3rImpl::submit_none_commands()
{
	begin_submit_commands();
	end_submit_commands();
}

void VkR3rImpl::wait_for_previous_frame()
{
	wait_for_fence(context_.in_flight_fence.get());
	reset_fence(context_.in_flight_fence.get());
}

void VkR3rImpl::swapchain_acquire_next_image()
{
	if (!context_.has_swapchain())
	{
		recreate_swapchain();
		if (!context_.has_swapchain())
		{
			return;
		}
	}
	for (;;)
	{
		const VkResult vk_result = context_.vkAcquireNextImageKHR(
			/* device */      context_.device.get(),
			/* swapchain */   context_.swapchain.get(),
			/* timeout */     UINT64_MAX,
			/* semaphore */   context_.image_available_semaphore.get(),
			/* fence */       VkFence{},
			/* pImageIndex */ &context_.swapchain_image_index
		);
		switch (vk_result)
		{
			case VK_ERROR_OUT_OF_DATE_KHR:
				recreate_swapchain();
				break;
			case VK_SUBOPTIMAL_KHR:
				return;
			default:
				ensure_vk_result(vk_result, "vkAcquireNextImageKHR");
				return;
		}
	}
}

void VkR3rImpl::recreate_swapchain()
{
	wait_for_device();
	terminate_swapchain_image_views();
	terminate_swapchain_images();
	terminate_swapchain();
	terminate_swapchain_sync_objects();
	context_.swapchain_image_index = UINT32_MAX;
	update_surface_capabilities();
	initialize_swapchain();
	if (context_.has_swapchain())
	{
		initialize_swapchain_images();
		initialize_swapchain_image_views();
		initialize_swapchain_sync_objects();
	}
}

void VkR3rImpl::blit_offscreen()
{
	if (!context_.has_swapchain())
	{
		return;
	}
	VkR3rCommandBufferResource command_buffer_resource = context_.cmd_begin_single_time_commands();
	VkImage const vk_swapchain_image = context_.swapchain_images[context_.swapchain_image_index];
	VkImageLayout& vk_swapchain_image_layout = context_.swapchain_image_layouts[context_.swapchain_image_index];
	if (vk_swapchain_image_layout == VK_IMAGE_LAYOUT_UNDEFINED ||
		vk_swapchain_image_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
	{
		context_.cmd_image_memory_barrier(
			command_buffer_resource.get(),
			vk_swapchain_image,
			vk_swapchain_image_layout,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			0,
			1,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
		);

		vk_swapchain_image_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	}
	else
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid swapchain image layout.");
	}
	if (context_.sample_count > 1)
	{
		const VkImageResolve vk_image_resolve{
			/* srcSubresource */ VkImageSubresourceLayers{
				/* aspectMask     */ VK_IMAGE_ASPECT_COLOR_BIT,
				/* mipLevel       */ 0,
				/* baseArrayLayer */ 0,
				/* layerCount     */ 1,
			},
			/* srcOffset      */ VkOffset3D{
				/* x */ 0,
				/* y */ 0,
				/* z */ 0,
			},
			/* dstSubresource */ VkImageSubresourceLayers{
				/* aspectMask     */ VK_IMAGE_ASPECT_COLOR_BIT,
				/* mipLevel       */ 0,
				/* baseArrayLayer */ 0,
				/* layerCount     */ 1,
			},
			/* dstOffset      */ VkOffset3D{
				/* x */ 0,
				/* y */ 0,
				/* z */ 0,
			},
			/* extent         */ VkExtent3D{
				/* width  */ context_.vk_surface_width,
				/* height */ context_.vk_surface_height,
				/* depth  */ 1,
			},
		};
		context_.vkCmdResolveImage(
			/* commandBuffer  */ command_buffer_resource.get(),
			/* srcImage       */ context_.offscreen_color_image.get(),
			/* srcImageLayout */ VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			/* dstImage       */ vk_swapchain_image,
			/* dstImageLayout */ VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			/* regionCount    */ 1,
			/* pRegions       */ &vk_image_resolve
		);
	}
	else
	{
		VkImageBlit vk_image_blit
		{
			/* srcSubresource */ VkImageSubresourceLayers{
				/* aspectMask */     VK_IMAGE_ASPECT_COLOR_BIT,
				/* mipLevel */       0,
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
					/* x */ static_cast<std::int32_t>(context_.vk_offscreen_width),
					/* y */ static_cast<std::int32_t>(context_.vk_offscreen_height),
					/* z */ 1,
				},
			},
			/* dstSubresource */ VkImageSubresourceLayers{
				/* aspectMask */     VK_IMAGE_ASPECT_COLOR_BIT,
				/* mipLevel */       0,
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
					/* x */ static_cast<std::int32_t>(context_.vk_surface_width),
					/* y */ static_cast<std::int32_t>(context_.vk_surface_height),
					/* z */ 1,
				},
			}
		};
		context_.vkCmdBlitImage(
			/* commandBuffer */  command_buffer_resource.get(),
			/* srcImage */       context_.offscreen_color_image.get(),
			/* srcImageLayout */ VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			/* dstImage */       vk_swapchain_image,
			/* dstImageLayout */ VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			/* regionCount */    1,
			/* pRegions */       &vk_image_blit,
			/* filter */         VK_FILTER_NEAREST
		);
	}
	context_.cmd_image_memory_barrier(
		command_buffer_resource.get(),
		vk_swapchain_image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		0,
		1,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT
	);
	vk_swapchain_image_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	context_.cmd_end_single_time_commands(command_buffer_resource.get());
}

void VkR3rImpl::frame_state_record_command_buffer_internal()
{
	if (!frame_state_.is_awaited_for_previous_frame)
	{
		wait_for_previous_frame();
		frame_state_.is_awaited_for_previous_frame = true;
	}
	if (!frame_state_.is_acquired_image_from_swapchain)
	{
		swapchain_acquire_next_image();
		frame_state_.is_acquired_image_from_swapchain = true;
	}
}

void VkR3rImpl::frame_state_record_command_buffer()
{
	frame_state_record_command_buffer_internal();
	if (!frame_state_.is_recorded_any_command)
	{
		begin_submit_commands();
		frame_state_.is_recorded_any_command = true;
	}
}

void VkR3rImpl::frame_state_pre_present()
{
	frame_state_record_command_buffer_internal();
	if (frame_state_.is_recorded_any_command)
	{
		end_submit_commands();
	}
	else
	{
		begin_submit_commands();
		end_submit_commands();
		frame_state_.is_recorded_any_command = true;
	}
	blit_offscreen();
}

void VkR3rImpl::frame_state_reset()
{
	frame_state_ = FrameState{};
}

void VkR3rImpl::submit_clear_command(const R3rClearCmd& r3r_cmd)
{
	const sys::Color& src_color = r3r_cmd.clear.color;
	constexpr std::uint32_t total_attachments = 2;
	using ClearAttachments = std::array<VkClearAttachment, total_attachments>;
	using ClearRects = std::array<VkClearRect, total_attachments>;
	ClearAttachments clear_attachments{};

	VkClearAttachment& vk_color_clear_attachment = clear_attachments[VkR3rContext::color_attachment_index];
	vk_color_clear_attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	vk_color_clear_attachment.colorAttachment = VkR3rContext::color_attachment_index;
	vk_color_clear_attachment.clearValue.color.float32[0] = color_byte_to_float(src_color.r);
	vk_color_clear_attachment.clearValue.color.float32[1] = color_byte_to_float(src_color.g);
	vk_color_clear_attachment.clearValue.color.float32[2] = color_byte_to_float(src_color.b);
	vk_color_clear_attachment.clearValue.color.float32[3] = color_byte_to_float(src_color.a);

	VkClearAttachment& vk_depth_clear_attachment = clear_attachments[VkR3rContext::depth_attachment_index];
	vk_depth_clear_attachment.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	vk_color_clear_attachment.colorAttachment = 0;
	vk_depth_clear_attachment.clearValue.depthStencil.depth = 1.0F;

	const VkClearRect vk_clear_rect
	{
		/* rect */           VkRect2D{
			/* offset */ VkOffset2D{
				/* x */      0,
				/* y */      0,
			},
			/* extent */ VkExtent2D{
				/* width */  context_.vk_offscreen_width,
				/* height */ context_.vk_offscreen_height,
			},
		},
		/* baseArrayLayer */ 0,
		/* layerCount */     1,

	};
	ClearRects vk_clear_rects{};
	vk_clear_rects.fill(vk_clear_rect);

	context_.vkCmdClearAttachments(
		/* commandBuffer */   context_.command_buffer,
		/* attachmentCount */ static_cast<std::uint32_t>(clear_attachments.size()),
		/* pAttachments */    clear_attachments.data(),
		/* rectCount */       static_cast<std::uint32_t>(vk_clear_rects.size()),
		/* pRects */          vk_clear_rects.data()
	);
}

void VkR3rImpl::submit_set_viewport(const R3rSetViewportCmd& r3r_cmd)
{
	const R3rViewport& viewport = r3r_cmd.viewport;
	const VkPhysicalDeviceLimits& limits = context_.physical_device_properties.limits;
	if (viewport.width < 0 ||
		static_cast<std::uint32_t>(viewport.width) > limits.maxViewportDimensions[0])
	{
		BSTONE_THROW_STATIC_SOURCE("Viewport width out of range.");
	}
	if (viewport.height < 0 ||
		static_cast<std::uint32_t>(viewport.height) > limits.maxViewportDimensions[1])
	{
		BSTONE_THROW_STATIC_SOURCE("Viewport height out of range.");
	}
	if (static_cast<float>(viewport.x) < limits.viewportBoundsRange[0] ||
		static_cast<float>(viewport.x + viewport.width) > limits.viewportBoundsRange[1])
	{
		BSTONE_THROW_STATIC_SOURCE("Viewport X out of range.");
	}
	if (static_cast<float>(viewport.y) < limits.viewportBoundsRange[0] ||
		static_cast<float>(viewport.y) > limits.viewportBoundsRange[1] ||
		static_cast<float>(viewport.y + viewport.height) < limits.viewportBoundsRange[0] ||
		static_cast<float>(viewport.y + viewport.height) > limits.viewportBoundsRange[1])
	{
		BSTONE_THROW_STATIC_SOURCE("Viewport Y out of range.");
	}
	if (viewport.min_depth < 0.0F || viewport.min_depth > 1.0F)
	{
		BSTONE_THROW_STATIC_SOURCE("Viewport min depth out of range.");
	}
	if (viewport.max_depth < 0.0F || viewport.max_depth > 1.0F)
	{
		BSTONE_THROW_STATIC_SOURCE("Viewport max depth out of range.");
	}
	context_.draw_state.viewport = VkViewport
	{
		/* x */        static_cast<float>(viewport.x),
		/* y */        static_cast<float>(static_cast<int>(context_.vk_surface_height) - viewport.y - viewport.height),
		/* width */    static_cast<float>(viewport.width),
		/* height */   static_cast<float>(viewport.height),
		/* minDepth */ static_cast<float>(viewport.min_depth),
		/* maxDepth */ static_cast<float>(viewport.max_depth),
	};
}

void VkR3rImpl::submit_enable_culling(const R3rEnableCullingCmd& r3r_cmd)
{
	context_.draw_state.is_culling_enabled = r3r_cmd.is_enable;
}

void VkR3rImpl::submit_enable_depth_test(const R3rEnableDepthTestCmd& r3r_cmd)
{
	context_.draw_state.is_depth_test_enabled = r3r_cmd.is_enable;
}

void VkR3rImpl::submit_enable_depth_write(const R3rEnableDepthWriteCmd& r3r_cmd)
{
	context_.draw_state.is_depth_write_enabled = r3r_cmd.is_enable;
}

void VkR3rImpl::submit_enable_blending(const R3rEnableBlendingCmd& r3r_cmd)
{
	context_.draw_state.is_blending_enabled = r3r_cmd.is_enable;
}

void VkR3rImpl::submit_set_blending_func(const R3rSetBlendingFuncCmd& r3r_cmd)
{
	if (r3r_cmd.blending_func.src_factor != R3rBlendingFactor::src_alpha &&
		r3r_cmd.blending_func.dst_factor != R3rBlendingFactor::one_minus_src_alpha)
	{
		BSTONE_THROW_STATIC_SOURCE("Unsupported blending function.");
	}
}

void VkR3rImpl::submit_set_texture(const R3rSetTextureCmd& r3r_cmd)
{
	if (r3r_cmd.r2_texture == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null 2D texture.");
	}
	context_.draw_state.r2_texture = r3r_cmd.r2_texture;
}

void VkR3rImpl::submit_set_sampler(const R3rSetSamplerCmd& r3r_cmd)
{
	if (r3r_cmd.sampler == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null sampler.");
	}
	context_.draw_state.sampler = r3r_cmd.sampler;
}

void VkR3rImpl::submit_set_vertex_input(const R3rSetVertexInputCmd& r3r_cmd)
{
	if (r3r_cmd.vertex_input == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null vertex input.");
	}
	context_.draw_state.vertex_input = r3r_cmd.vertex_input;
}

void VkR3rImpl::submit_set_shader_stage(const R3rSetShaderStageCmd& r3r_cmd)
{
	if (r3r_cmd.shader_stage == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null shader stage.");
	}
	context_.draw_state.shader_stage = r3r_cmd.shader_stage;
}

void VkR3rImpl::submit_set_int32_uniform(const R3rSetInt32UniformCmd& r3r_cmd)
{
	ensure_not_null_shader_var(r3r_cmd.var);
	context_.shader_vars_draw_state.emplace_back();
	VkR3rContext::ShaderVarDrawState& state = context_.shader_vars_draw_state.back();
	state.type_id = R3rShaderVarTypeId::int32;
	state.shader_var = r3r_cmd.var;
	state.int32_value = r3r_cmd.value;
}

void VkR3rImpl::submit_set_float32_uniform(const R3rSetFloat32UniformCmd& r3r_cmd)
{
	ensure_not_null_shader_var(r3r_cmd.var);
	context_.shader_vars_draw_state.emplace_back();
	VkR3rContext::ShaderVarDrawState& state = context_.shader_vars_draw_state.back();
	state.type_id = R3rShaderVarTypeId::float32;
	state.shader_var = r3r_cmd.var;
	state.float32_value = r3r_cmd.value;
}

void VkR3rImpl::submit_set_vec2_uniform(const R3rSetVec2UniformCmd& r3r_cmd)
{
	ensure_not_null_shader_var(r3r_cmd.var);
	context_.shader_vars_draw_state.emplace_back();
	VkR3rContext::ShaderVarDrawState& state = context_.shader_vars_draw_state.back();
	state.type_id = R3rShaderVarTypeId::vec2;
	state.shader_var = r3r_cmd.var;
	state.vec2_value = r3r_cmd.value;
}

void VkR3rImpl::submit_set_vec4_uniform(const R3rSetVec4UniformCmd& r3r_cmd)
{
	ensure_not_null_shader_var(r3r_cmd.var);
	context_.shader_vars_draw_state.emplace_back();
	VkR3rContext::ShaderVarDrawState& state = context_.shader_vars_draw_state.back();
	state.type_id = R3rShaderVarTypeId::vec4;
	state.shader_var = r3r_cmd.var;
	state.vec4_value = r3r_cmd.value;
}

void VkR3rImpl::submit_set_mat4_uniform(const R3rSetMat4UniformCmd& r3r_cmd)
{
	ensure_not_null_shader_var(r3r_cmd.var);
	context_.shader_vars_draw_state.emplace_back();
	VkR3rContext::ShaderVarDrawState& state = context_.shader_vars_draw_state.back();
	state.type_id = R3rShaderVarTypeId::mat4;
	state.shader_var = r3r_cmd.var;
	state.mat4_value = r3r_cmd.value;
}

void VkR3rImpl::submit_set_r2_sampler_uniform(const R3rSetR2SamplerUniformCmd& r3r_cmd)
{
	ensure_not_null_shader_var(r3r_cmd.var);
	context_.shader_vars_draw_state.emplace_back();
	VkR3rContext::ShaderVarDrawState& state = context_.shader_vars_draw_state.back();
	state.type_id = R3rShaderVarTypeId::sampler2d;
	state.shader_var = r3r_cmd.var;
	state.int32_value = r3r_cmd.value;
}

void VkR3rImpl::submit_draw_indexed(const R3rDrawIndexedCmd& r3r_cmd)
{
	const R3rDrawIndexedParam& draw_indexed = r3r_cmd.draw_indexed;
	switch (draw_indexed.primitive_type)
	{
		case R3rPrimitiveType::triangle_list:
			break;
		default:
			BSTONE_THROW_STATIC_SOURCE("Unknown primitive type.");
	}
	if (draw_indexed.vertex_count < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Vertex count out of range.");
	}
	switch (draw_indexed.index_byte_depth)
	{
		case 2:
		case 4:
			break;
		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported index byte depth.");
	}
	if (draw_indexed.index_buffer_offset < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Offset into index buffer out of range.");
	}
	if (draw_indexed.index_offset < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Index offset out of range.");
	}
	if (context_.draw_state.r2_texture == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("No 2D texture.");
	}
	if (context_.draw_state.sampler == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("No sampler.");
	}
	if (context_.draw_state.vertex_input == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Vertex input.");
	}
	if (context_.draw_state.shader_stage == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Shader stage.");
	}
	for (const VkR3rContext::ShaderVarDrawState& shader_var_draw_state : context_.shader_vars_draw_state)
	{
		if (shader_var_draw_state.shader_var == nullptr)
		{
			BSTONE_THROW_STATIC_SOURCE("No shader variable.");
		}
		switch (shader_var_draw_state.type_id)
		{
			case R3rShaderVarTypeId::int32:
				shader_var_draw_state.shader_var->set_int32(shader_var_draw_state.int32_value);
				break;
			case R3rShaderVarTypeId::float32:
				shader_var_draw_state.shader_var->set_float32(shader_var_draw_state.float32_value);
				break;
			case R3rShaderVarTypeId::vec2:
				shader_var_draw_state.shader_var->set_vec2(shader_var_draw_state.vec2_value.data());
				break;
			case R3rShaderVarTypeId::vec4:
				shader_var_draw_state.shader_var->set_vec4(shader_var_draw_state.vec4_value.data());
				break;
			case R3rShaderVarTypeId::mat4:
				shader_var_draw_state.shader_var->set_mat4(shader_var_draw_state.mat4_value.data());
				break;
			case R3rShaderVarTypeId::sampler2d:
				shader_var_draw_state.shader_var->set_r2_sampler(shader_var_draw_state.int32_value);
				break;
			default:
				BSTONE_THROW_STATIC_SOURCE("Unknown shader type id.");
		}
	}
	if (draw_indexed.vertex_count > 0)
	{
		VkR3rPipeline* pipeline = pipeline_mgr_->acquire_pipeline();
		const VkR3rPipelineDrawIndexedParam param
		{
			/* vertex_count */        draw_indexed.vertex_count,
			/* index_byte_depth */    draw_indexed.index_byte_depth,
			/* index_buffer_offset */ draw_indexed.index_buffer_offset,
			/* index_offset */        draw_indexed.index_offset,
		};
		pipeline->draw_indexed(param);
	}
	context_.update_draw_state();
}

[[noreturn]] void VkR3rImpl::submit_unknown_command(R3rCmdId cmd_id)
{
	std::string message{};
	message.reserve(64);
	message += "Unknown command id (id=";
	message += std::to_string(static_cast<std::underlying_type_t<R3rCmdId>>(cmd_id));
	message += ").";
	BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
}

} // namespace

// ======================================

R3rUPtr make_vk_r3r(sys::VideoMgr& video_mgr, sys::WindowMgr& window_mgr, const R3rInitParam& param)
try {
	return std::make_unique<VkR3rImpl>(video_mgr, window_mgr, param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone

// ======================================

#undef BSTONE_STRCTX
