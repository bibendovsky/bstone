/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer: RAII

#ifndef BSTONE_VK_R3R_RAII_INCLUDED
#define BSTONE_VK_R3R_RAII_INCLUDED

#include "vulkan/vulkan.h"
#include "bstone_utility.h"

namespace bstone {

class VkR3rContext;

// ======================================

struct VkR3rResourceNull {};

extern const VkR3rResourceNull vk_r3r_resource_null;

// ======================================

template<typename TResource, typename TDeleter, TResource TEmptyValue = TResource{}>
class VkR3rResource
{
public:
	using Resource = TResource;
	using Deleter = TDeleter;

	static constexpr Resource empty_value = TEmptyValue;

	VkR3rResource() = default;
	VkR3rResource(Resource resource, Deleter deleter);
	VkR3rResource(const VkR3rResource&) = delete;
	VkR3rResource& operator=(VkR3rResourceNull);
	VkR3rResource& operator=(const VkR3rResource&) = delete;
	VkR3rResource(VkR3rResource&& that) noexcept;
	VkR3rResource& operator=(VkR3rResource&& that) noexcept;
	~VkR3rResource();

	Resource get() const;
	Deleter get_deleter() const;
	bool is_empty() const;

	void reset();
	void reset(Resource resource, Deleter deleter);
	Resource release();
	void swap(VkR3rResource& that);

private:
	Resource resource_{empty_value};
	Deleter deleter_{};
};

// --------------------------------------

template<typename TResource, typename TDeleter, TResource TEmptyValue>
VkR3rResource<TResource, TDeleter, TEmptyValue>::VkR3rResource(Resource resource, Deleter deleter)
	:
	resource_(resource),
	deleter_(deleter)
{}

template<typename TResource, typename TDeleter, TResource TEmptyValue>
auto VkR3rResource<TResource, TDeleter, TEmptyValue>::operator=(VkR3rResourceNull) -> VkR3rResource&
{
	reset();
	return *this;
}

template<typename TResource, typename TDeleter, TResource TEmptyValue>
VkR3rResource<TResource, TDeleter, TEmptyValue>::VkR3rResource(VkR3rResource&& that) noexcept
	:
	resource_{that.resource_},
	deleter_{that.deleter_}
{
	that.resource_ = empty_value;
	that.deleter_ = Deleter{};
}

template<typename TResource, typename TDeleter, TResource TEmptyValue>
auto VkR3rResource<TResource, TDeleter, TEmptyValue>::operator=(VkR3rResource&& that) noexcept -> VkR3rResource&
{
	reset(that.release(), that.get_deleter());
	return *this;
}

template<typename TResource, typename TDeleter, TResource TEmptyValue>
VkR3rResource<TResource, TDeleter, TEmptyValue>::~VkR3rResource()
{
	if (!is_empty())
	{
		deleter_(resource_);
	}
}

template<typename TResource, typename TDeleter, TResource TEmptyValue>
auto VkR3rResource<TResource, TDeleter, TEmptyValue>::get() const -> Resource
{
	return resource_;
}

template<typename TResource, typename TDeleter, TResource TEmptyValue>
auto VkR3rResource<TResource, TDeleter, TEmptyValue>::get_deleter() const -> Deleter
{
	return deleter_;
}

template<typename TResource, typename TDeleter, TResource TEmptyValue>
bool VkR3rResource<TResource, TDeleter, TEmptyValue>::is_empty() const
{
	return get() == empty_value;
}

template<typename TResource, typename TDeleter, TResource TEmptyValue>
void VkR3rResource<TResource, TDeleter, TEmptyValue>::reset()
{
	if (!is_empty())
	{
		deleter_(resource_);
		resource_ = empty_value;
	}
}

template<typename TResource, typename TDeleter, TResource TEmptyValue>
void VkR3rResource<TResource, TDeleter, TEmptyValue>::reset(Resource resource, Deleter deleter)
{
	if (!is_empty())
	{
		deleter_(resource_);
	}
	resource_ = resource;
	deleter_ = deleter;
}

template<typename TResource, typename TDeleter, TResource TEmptyValue>
auto VkR3rResource<TResource, TDeleter, TEmptyValue>::release() -> Resource
{
	Resource result = get();
	resource_ = empty_value;
	return result;
}

template<typename TResource, typename TDeleter, TResource TEmptyValue>
void VkR3rResource<TResource, TDeleter, TEmptyValue>::swap(VkR3rResource& that)
{
	swop(resource_, that.resource_);
	swop(deleter_, that.deleter_);
}

// ======================================

class VkR3rInstanceDeleter
{
public:
	VkR3rInstanceDeleter() = default;
	explicit VkR3rInstanceDeleter(const VkR3rContext& vk_context);

	void operator()(VkInstance instance) const;

private:
	const VkR3rContext* context_{};
};

using VkR3rInstanceResource = VkR3rResource<VkInstance, VkR3rInstanceDeleter>;

// ======================================

class VkR3rDebugUtilsMessengerDeleter
{
public:
	VkR3rDebugUtilsMessengerDeleter() = default;
	explicit VkR3rDebugUtilsMessengerDeleter(const VkR3rContext& context);

	void operator()(VkDebugUtilsMessengerEXT vk_debug_utils_messenger) const;

private:
	const VkR3rContext* context_{};
};

using VkR3rDebugUtilsMessengerResource = VkR3rResource<VkDebugUtilsMessengerEXT, VkR3rDebugUtilsMessengerDeleter>;

// ======================================

class VkR3rSurfaceKhrDeleter
{
public:
	VkR3rSurfaceKhrDeleter() = default;
	explicit VkR3rSurfaceKhrDeleter(const VkR3rContext& context);

	void operator()(VkSurfaceKHR vk_surface_khr) const;

private:
	const VkR3rContext* context_{};
};

using VkR3rSurfaceKhrResource = VkR3rResource<VkSurfaceKHR, VkR3rSurfaceKhrDeleter>;

// ======================================

class VkR3rDeviceDeleter
{
public:
	VkR3rDeviceDeleter() = default;
	explicit VkR3rDeviceDeleter(const VkR3rContext& context);

	void operator()(VkDevice vk_device) const;

private:
	const VkR3rContext* context_{};
};

using VkR3rDeviceResource = VkR3rResource<VkDevice, VkR3rDeviceDeleter>;

// ======================================

class VkR3rSwapchainKhrDeleter
{
public:
	VkR3rSwapchainKhrDeleter() = default;
	explicit VkR3rSwapchainKhrDeleter(const VkR3rContext& context);

	void operator()(VkSwapchainKHR vk_swapchain_khr) const;

private:
	const VkR3rContext* context_{};
};

using VkR3rSwapchainKhrResource = VkR3rResource<VkSwapchainKHR, VkR3rSwapchainKhrDeleter>;

// ======================================

class VkR3rImageViewDeleter
{
public:
	VkR3rImageViewDeleter() = default;
	explicit VkR3rImageViewDeleter(const VkR3rContext& context);

	void operator()(VkImageView vk_image_view) const;

private:
	const VkR3rContext* context_{};
};

using VkR3rImageViewResource = VkR3rResource<VkImageView, VkR3rImageViewDeleter>;

// ======================================

class VkR3rRenderPassDeleter
{
public:
	VkR3rRenderPassDeleter() = default;
	explicit VkR3rRenderPassDeleter(const VkR3rContext& context);

	void operator()(VkRenderPass vk_render_pass) const;

private:
	const VkR3rContext* context_{};
};

using VkR3rRenderPassResource = VkR3rResource<VkRenderPass, VkR3rRenderPassDeleter>;

// ======================================

class VkR3rFramebufferDeleter
{
public:
	VkR3rFramebufferDeleter() = default;
	explicit VkR3rFramebufferDeleter(const VkR3rContext& context);

	void operator()(VkFramebuffer vk_framebuffer) const;

private:
	const VkR3rContext* context_{};
};

using VkR3rFramebufferResource = VkR3rResource<VkFramebuffer, VkR3rFramebufferDeleter>;

// ======================================

class VkR3rCommandPoolDeleter
{
public:
	VkR3rCommandPoolDeleter() = default;
	explicit VkR3rCommandPoolDeleter(const VkR3rContext& context);

	void operator()(VkCommandPool vk_command_pool) const;

private:
	const VkR3rContext* context_{};
};

using VkR3rCommandPoolResource = VkR3rResource<VkCommandPool, VkR3rCommandPoolDeleter>;

// ======================================

class VkR3rSemaphoreDeleter
{
public:
	VkR3rSemaphoreDeleter() = default;
	explicit VkR3rSemaphoreDeleter(const VkR3rContext& context);

	void operator()(VkSemaphore vk_semaphore) const;

private:
	const VkR3rContext* context_{};
};

using VkR3rSemaphoreResource = VkR3rResource<VkSemaphore, VkR3rSemaphoreDeleter>;

// ======================================

class VkR3rFenceDeleter
{
public:
	VkR3rFenceDeleter() = default;
	explicit VkR3rFenceDeleter(const VkR3rContext& context);

	void operator()(VkFence vk_fence) const;

private:
	const VkR3rContext* context_{};
};

using VkR3rFenceResource = VkR3rResource<VkFence, VkR3rFenceDeleter>;

// ======================================

class VkR3rShaderModuleDeleter
{
public:
	VkR3rShaderModuleDeleter() = default;
	explicit VkR3rShaderModuleDeleter(const VkR3rContext& context);

	void operator()(VkShaderModule vk_shader_module) const;

private:
	const VkR3rContext* context_{};
};

using VkR3rShaderModuleResource = VkR3rResource<VkShaderModule, VkR3rShaderModuleDeleter>;

// ======================================

class VkR3rDeviceMemoryDeleter
{
public:
	VkR3rDeviceMemoryDeleter() = default;
	explicit VkR3rDeviceMemoryDeleter(const VkR3rContext& context);

	void operator()(VkDeviceMemory vk_device_memory) const;

private:
	const VkR3rContext* context_{};
};

using VkR3rDeviceMemoryResource = VkR3rResource<VkDeviceMemory, VkR3rDeviceMemoryDeleter>;

// ======================================

class VkR3rBufferDeleter
{
public:
	VkR3rBufferDeleter() = default;
	explicit VkR3rBufferDeleter(const VkR3rContext& context);

	void operator()(VkBuffer vk_buffer) const;

private:
	const VkR3rContext* context_{};
};

using VkR3rBufferResource = VkR3rResource<VkBuffer, VkR3rBufferDeleter>;

// ======================================

class VkR3rImageDeleter
{
public:
	VkR3rImageDeleter() = default;
	explicit VkR3rImageDeleter(const VkR3rContext& context);

	void operator()(VkImage vk_image) const;

private:
	const VkR3rContext* context_{};
};

using VkR3rImageResource = VkR3rResource<VkImage, VkR3rImageDeleter>;

// ======================================

class VkR3rSamplerDeleter
{
public:
	VkR3rSamplerDeleter() = default;
	explicit VkR3rSamplerDeleter(const VkR3rContext& context);

	void operator()(VkSampler vk_sampler) const;

private:
	const VkR3rContext* context_{};
};

using VkR3rSamplerResource = VkR3rResource<VkSampler, VkR3rSamplerDeleter>;

// ======================================

class VkR3rCommandBufferDeleter
{
public:
	VkR3rCommandBufferDeleter() = default;
	VkR3rCommandBufferDeleter(const VkR3rContext& context, VkCommandPool vk_command_pool);

	void operator()(VkCommandBuffer vk_command_buffer) const;

private:
	const VkR3rContext* context_{};
	VkCommandPool vk_command_pool_{};
};

using VkR3rCommandBufferResource = VkR3rResource<VkCommandBuffer, VkR3rCommandBufferDeleter>;

// ======================================

class VkR3rDescriptorPoolDeleter
{
public:
	VkR3rDescriptorPoolDeleter() = default;
	explicit VkR3rDescriptorPoolDeleter(const VkR3rContext& context);

	void operator()(VkDescriptorPool vk_descriptor_pool) const;

private:
	const VkR3rContext* context_{};
};

using VkR3rDescriptorPoolResource = VkR3rResource<VkDescriptorPool, VkR3rDescriptorPoolDeleter>;

// ======================================

class VkR3rDescriptorSetLayoutDeleter
{
public:
	VkR3rDescriptorSetLayoutDeleter() = default;
	explicit VkR3rDescriptorSetLayoutDeleter(const VkR3rContext& context);

	void operator()(VkDescriptorSetLayout vk_descriptor_set_layout) const;

private:
	const VkR3rContext* context_{};
};

using VkR3rDescriptorSetLayoutResource = VkR3rResource<VkDescriptorSetLayout, VkR3rDescriptorSetLayoutDeleter>;

// ======================================

class VkR3rDescriptorSetDeleter
{
public:
	VkR3rDescriptorSetDeleter() = default;
	explicit VkR3rDescriptorSetDeleter(const VkR3rContext& context);

	void operator()(VkDescriptorSet vk_descriptor_set) const;

private:
	const VkR3rContext* context_{};
};

using VkR3rDescriptorSetResource = VkR3rResource<VkDescriptorSet, VkR3rDescriptorSetDeleter>;

// ======================================

class VkR3rPipelineLayoutDeleter
{
public:
	VkR3rPipelineLayoutDeleter() = default;
	explicit VkR3rPipelineLayoutDeleter(const VkR3rContext& context);

	void operator()(VkPipelineLayout vk_pipeline_layout) const;

private:
	const VkR3rContext* context_{};
};

using VkR3rPipelineLayoutResource = VkR3rResource<VkPipelineLayout, VkR3rPipelineLayoutDeleter>;

// ======================================

class VkR3rPipelineDeleter
{
public:
	VkR3rPipelineDeleter() = default;
	explicit VkR3rPipelineDeleter(const VkR3rContext& context);

	void operator()(VkPipeline vk_pipeline) const;

private:
	const VkR3rContext* context_{};
};

using VkR3rPipelineResource = VkR3rResource<VkPipeline, VkR3rPipelineDeleter>;

} // namespace bstone

#endif // BSTONE_VK_R3R_RAII_INCLUDED
