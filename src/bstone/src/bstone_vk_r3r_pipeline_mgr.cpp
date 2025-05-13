/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer: Pipeline manager

#include "bstone_vk_r3r_pipeline_mgr.h"
#include "bstone_exception.h"
#include "bstone_single_pool_resource.h"
#include "bstone_r3r_limits.h"
#include "bstone_vk_r3r_context.h"
#include "bstone_vk_r3r_pipeline.h"
#include <cstddef>
#include <cstdint>
#include <unordered_map>

namespace bstone {

namespace {

class VkR3rPipelineMgrImpl final : public VkR3rPipelineMgr
{
public:
	VkR3rPipelineMgrImpl(VkR3rContext& context);
	~VkR3rPipelineMgrImpl() override {}

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	void do_clear() override;
	VkR3rPipeline* do_acquire_pipeline() override;

	using MemoryPool = SinglePoolResource<VkR3rPipelineMgrImpl>;
	using PipelineMapKey = VkR3rContext::DrawState;
	class ByteHasher
	{
	public:
		std::size_t get() const
		{
			return hash_;
		}

		void update(const void* data, int data_size)
		{
			const unsigned char* const bytes = static_cast<const unsigned char*>(data);

			for (int i = 0; i < data_size; ++i)
			{
				hash_ = hash_ * 31 + bytes[i] + 1;
			}
		}

		template<typename T>
		void update(const T& value)
		{
			update(&value, sizeof(T));
		}

	private:
		std::size_t hash_{};
	};
	struct PipelineMapKeyHasher
	{
		std::size_t operator()(const PipelineMapKey& key) const
		{
			ByteHasher hasher{};
			hasher.update(key.is_culling_enabled);
			hasher.update(key.is_blending_enabled);
			hasher.update(key.is_depth_test_enabled);
			hasher.update(key.is_depth_write_enabled);
			hasher.update(key.vertex_input);
			hasher.update(key.shader_stage);
			return hasher.get();
		}
	};
	struct PipelineMapKeyComparer
	{
		bool operator()(const PipelineMapKey& a, const PipelineMapKey& b) const
		{
			return
				a.is_culling_enabled == b.is_culling_enabled &&
				a.is_blending_enabled == b.is_blending_enabled &&
				a.is_depth_test_enabled == b.is_depth_test_enabled &&
				a.is_depth_write_enabled == b.is_depth_write_enabled &&
				a.vertex_input == b.vertex_input &&
				a.shader_stage == b.shader_stage;
		}
	};
	using PipelineMap = std::unordered_map<
		PipelineMapKey,
		VkR3rPipelineUPtr,
		PipelineMapKeyHasher,
		PipelineMapKeyComparer>;

	static MemoryPool memory_pool_;

	VkR3rContext& context_;
	PipelineMap pipeline_map_{};
};

// --------------------------------------

VkR3rPipelineMgrImpl::MemoryPool VkR3rPipelineMgrImpl::memory_pool_{};

// --------------------------------------

VkR3rPipelineMgrImpl::VkR3rPipelineMgrImpl(VkR3rContext& context)
	:
	context_{context}
{
	pipeline_map_.reserve(32);
}

void* VkR3rPipelineMgrImpl::operator new(std::size_t size)
try {
	return memory_pool_.allocate(static_cast<std::intptr_t>(size));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void VkR3rPipelineMgrImpl::operator delete(void* ptr)
{
	memory_pool_.deallocate(ptr);
}

void VkR3rPipelineMgrImpl::do_clear()
{
	pipeline_map_.clear();
}

VkR3rPipeline* VkR3rPipelineMgrImpl::do_acquire_pipeline()
{
	const auto pipeline_map_iter = pipeline_map_.find(context_.draw_state);
	if (pipeline_map_iter != pipeline_map_.cend())
	{
		return pipeline_map_iter->second.get();
	}
	VkR3rPipelineUPtr pipeline = make_vk_r3r_pipeline(context_);
	pipeline_map_[context_.draw_state].swap(pipeline);
	return pipeline_map_[context_.draw_state].get();
}

} // namespace

// ======================================

void VkR3rPipelineMgr::clear()
{
	return do_clear();
}

VkR3rPipeline* VkR3rPipelineMgr::acquire_pipeline()
{
	return do_acquire_pipeline();
}

// ======================================

VkR3rPipelineMgrUPtr make_vk_r3r_pipeline_mgr(VkR3rContext& context)
{
	return std::make_unique<VkR3rPipelineMgrImpl>(context);
}

} // namespace bstone
