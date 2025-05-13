/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer: 2D texture

#include "bstone_vk_r3r_r2_texture.h"
#include "bstone_assert.h"
#include "bstone_exception.h"
#include "bstone_fixed_pool_resource.h"
#include "bstone_r3r_limits.h"
#include "bstone_vk_r3r_context.h"
#include "bstone_vk_r3r_raii.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <array>

namespace bstone {

namespace {

class VkR3rR2TextureImpl final : public VkR3rR2Texture
{
public:
	VkR3rR2TextureImpl(VkR3rContext& context, const R3rR2TextureInitParam& param);
	~VkR3rR2TextureImpl() override {}

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	void do_update(const R3rR2TextureUpdateParam& param) override;
	void do_generate_mipmaps() override;

	VkImageView do_get_vk_image_view() const override;

	constexpr static const VkFormat vk_default_format = VK_FORMAT_R8G8B8A8_UNORM;
	using MemoryPool = FixedPoolResource<VkR3rR2TextureImpl, R3rLimits::max_textures()>;
	using ImageLayouts = std::array<VkImageLayout, R3rLimits::max_mipmap_count()>;

	static MemoryPool memory_pool_;
	VkR3rContext& context_;
	VkR3rDeviceMemoryResource image_device_memory_resource_{};
	VkR3rImageViewResource image_view_resource_{};
	VkR3rImageResource image_resource_{};
	VkR3rDeviceMemoryResource staging_buffer_device_memory_resource_{};
	VkR3rBufferResource staging_buffer_resource_{};
	void* staging_buffer_mapped_memory_{};
	int width_{};
	int height_{};
	int mip_count_{};
	ImageLayouts image_layouts_{};

	void transition_image_layout(VkCommandBuffer vk_command_buffer, int mip_level, VkImageLayout new_vk_image_layout);
	void transition_image_layouts(VkCommandBuffer vk_command_buffer, VkImageLayout new_vk_image_layout);
};

// --------------------------------------

VkR3rR2TextureImpl::MemoryPool VkR3rR2TextureImpl::memory_pool_{};

// --------------------------------------

VkR3rR2TextureImpl::VkR3rR2TextureImpl(VkR3rContext& context, const R3rR2TextureInitParam& param)
	:
	context_{context}
{
	if (param.width <= 0 || param.width > context_.r3r_device_features.max_texture_dimension)
	{
		BSTONE_THROW_STATIC_SOURCE("Width out of range.");
	}
	if (param.height <= 0 || param.height > context_.r3r_device_features.max_texture_dimension)
	{
		BSTONE_THROW_STATIC_SOURCE("Height out of range.");
	}
	if (param.mipmap_count < 1 || param.mipmap_count > R3rLimits::max_mipmap_count())
	{
		BSTONE_THROW_STATIC_SOURCE("Mipmap count out of range.");
	}
	switch (param.pixel_format)
	{
		case R3rPixelFormat::rgba_8_unorm: break;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown pixel format.");
	}
	const std::uint32_t vk_width = static_cast<std::uint32_t>(param.width);
	const std::uint32_t vk_height = static_cast<std::uint32_t>(param.height);
	const std::uint32_t vk_size = vk_width * vk_height * 4;
	context_.create_image_resource(
		vk_default_format,
		vk_width,
		vk_height,
		static_cast<std::uint32_t>(param.mipmap_count),
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		image_resource_,
		image_device_memory_resource_);
	context_.create_buffer_resource(
		vk_size,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		staging_buffer_resource_,
		staging_buffer_device_memory_resource_);
	staging_buffer_mapped_memory_ = context_.map_memory(staging_buffer_device_memory_resource_.get());
	image_view_resource_ = context_.create_image_view_resource(
		image_resource_.get(), vk_default_format, VK_IMAGE_ASPECT_COLOR_BIT);
	width_ = param.width;
	height_ = param.height;
	mip_count_ = param.mipmap_count;
	image_layouts_.fill(VK_IMAGE_LAYOUT_UNDEFINED);
}

void* VkR3rR2TextureImpl::operator new(std::size_t size)
try {
	return memory_pool_.allocate(static_cast<std::intptr_t>(size));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void VkR3rR2TextureImpl::operator delete(void* ptr)
{
	memory_pool_.deallocate(ptr);
}

void VkR3rR2TextureImpl::do_update(const R3rR2TextureUpdateParam& param)
{
	if (param.image == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null image data.");
	}
	if (param.mipmap_level < 0 || param.mipmap_level >= mip_count_)
	{
		BSTONE_THROW_STATIC_SOURCE("Mip level out of range.");
	}
	VkR3rCommandBufferResource command_buffer_resource = context_.cmd_begin_single_time_commands();
	transition_image_layout(command_buffer_resource.get(), param.mipmap_level, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	const std::uint32_t mip_width = static_cast<std::uint32_t>(std::max(width_ >> param.mipmap_level, 1));
	const std::uint32_t mip_height = static_cast<std::uint32_t>(std::max(height_ >> param.mipmap_level, 1));
	const std::size_t mip_data_size = std::size_t{4} * mip_width * mip_height;
	std::memcpy(staging_buffer_mapped_memory_, param.image, mip_data_size);
	context_.cmd_copy_buffer_to_image(
		command_buffer_resource.get(),
		staging_buffer_resource_.get(),
		image_resource_.get(),
		mip_width,
		mip_height,
		param.mipmap_level);
	transition_image_layout(command_buffer_resource.get(), param.mipmap_level, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	context_.cmd_end_single_time_commands(command_buffer_resource.get());
}

void VkR3rR2TextureImpl::do_generate_mipmaps()
{
	if (!context_.r3r_device_features.is_mipmap_available)
	{
		BSTONE_THROW_STATIC_SOURCE("Mipmap generation not supported.");
	}
	if (mip_count_ == 1)
	{
		return;
	}
	VkR3rCommandBufferResource command_buffer_resource = context_.cmd_begin_single_time_commands();
	int prev_mip_width = width_;
	int prev_mip_height = height_;
	for (int mip_level = 1; mip_level < mip_count_; ++mip_level)
	{
		const int mip_width = std::max(prev_mip_width / 2, 1);
		const int mip_height = std::max(prev_mip_height / 2, 1);
		transition_image_layout(command_buffer_resource.get(), mip_level - 1, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
		transition_image_layout(command_buffer_resource.get(), mip_level, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		context_.cmd_blit_image(
			command_buffer_resource.get(),
			image_resource_.get(),
			mip_level - 1,
			prev_mip_width,
			prev_mip_height,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image_resource_.get(),
			mip_level,
			mip_width,
			mip_height,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_FILTER_NEAREST);
		prev_mip_width = mip_width;
		prev_mip_height = mip_height;
	}
	transition_image_layouts(command_buffer_resource.get(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	context_.cmd_end_single_time_commands(command_buffer_resource.get());
}

VkImageView VkR3rR2TextureImpl::do_get_vk_image_view() const
{
	return image_view_resource_.get();
}

void VkR3rR2TextureImpl::transition_image_layout(
	VkCommandBuffer vk_command_buffer,
	int mip_level,
	VkImageLayout new_vk_image_layout)
{
	VkImageLayout& vk_image_layout = image_layouts_[mip_level];
	if (vk_image_layout == new_vk_image_layout)
	{
		return;
	}
	context_.cmd_image_memory_barrier(
		vk_command_buffer,
		image_resource_.get(),
		vk_image_layout,
		new_vk_image_layout,
		mip_level,
		1,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT);
	vk_image_layout = new_vk_image_layout;
}

void VkR3rR2TextureImpl::transition_image_layouts(VkCommandBuffer vk_command_buffer, VkImageLayout new_vk_image_layout)
{
	for (int mip_level = 0; mip_level < mip_count_; )
	{
		const VkImageLayout& run_image_layout = image_layouts_[mip_level];
		int run_length = 0;
		for (int run_mip_level = mip_level; run_mip_level < mip_count_; ++run_mip_level)
		{
			const VkImageLayout& current_image_layout = image_layouts_[run_mip_level];
			if (current_image_layout != run_image_layout)
			{
				break;
			}
			++run_length;
		}
		if (run_image_layout != new_vk_image_layout)
		{
			context_.cmd_image_memory_barrier(
				vk_command_buffer,
				image_resource_.get(),
				run_image_layout,
				new_vk_image_layout,
				mip_level,
				run_length,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT);
			std::fill_n(image_layouts_.begin() + mip_level, run_length, new_vk_image_layout);
		}
		mip_level += run_length;
	}
}

} // namespace

// ======================================

VkImageView VkR3rR2Texture::get_vk_image_view() const
{
	return do_get_vk_image_view();
}

// ======================================

VkR3rR2TextureUPtr make_vk_r3r_r2_texture(VkR3rContext& context, const R3rR2TextureInitParam& param)
{
	return std::make_unique<VkR3rR2TextureImpl>(context, param);
}

} // namespace bstone
