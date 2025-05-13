/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer: Pipeline

#include "bstone_vk_r3r_pipeline.h"
#include "bstone_exception.h"
#include "bstone_fixed_pool_resource.h"
#include "bstone_r3r_limits.h"
#include "bstone_vk_r3r_buffer.h"
#include "bstone_vk_r3r_context.h"
#include "bstone_vk_r3r_r2_texture.h"
#include "bstone_vk_r3r_raii.h"
#include "bstone_vk_r3r_sampler.h"
#include "bstone_vk_r3r_shader.h"
#include "bstone_vk_r3r_shader_stage.h"
#include "bstone_vk_r3r_vertex_input.h"
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace bstone {

namespace {

class VkR3rPipelineImpl final : public VkR3rPipeline
{
public:
	VkR3rPipelineImpl(VkR3rContext& context);
	~VkR3rPipelineImpl() override {}

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

	using MemoryPool = FixedPoolResource<VkR3rPipelineImpl, VkR3rContext::max_pipelines>;

	static MemoryPool memory_pool_;

	VkR3rContext& context_;
	VkR3rPipelineLayoutResource pipeline_layout_{};
	VkR3rPipelineResource pipeline_{};

	void do_draw_indexed(const VkR3rPipelineDrawIndexedParam& param) override;

	void enqueue_bind_pipeline();
	void enqueue_viewport();
	void enqueue_scissor();
	void enqueue_bind_index_buffer(const VkR3rPipelineDrawIndexedParam& param);
	void enqueue_bind_vertex_buffer();
	void enqueue_bind_descriptor_set();
	void enqueue_draw_indexed(const VkR3rPipelineDrawIndexedParam& param);
};

// --------------------------------------

VkR3rPipelineImpl::MemoryPool VkR3rPipelineImpl::memory_pool_{};

// --------------------------------------

VkR3rPipelineImpl::VkR3rPipelineImpl(VkR3rContext& context)
	:
	context_{context}
{
	VkResult vk_result;
	// Stages.
	constexpr const char* const shader_entry_name = "main";
	VkR3rShaderStage* shader_stage = static_cast<VkR3rShaderStage*>(context.draw_state.shader_stage);
	VkShaderModule vk_vertex_shader_module = shader_stage->get_vertex_shader()->get_vk_shader_module();
	VkShaderModule vk_fragment_shader_module = shader_stage->get_fragment_shader()->get_vk_shader_module();
	const VkPipelineShaderStageCreateInfo vk_pipeline_shader_stage_create_infos[] =
	{
		VkPipelineShaderStageCreateInfo
		{
			/* sType */               VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			/* pNext */               nullptr,
			/* flags */               VkPipelineShaderStageCreateFlags{},
			/* stage */               VK_SHADER_STAGE_VERTEX_BIT,
			/* module */              vk_vertex_shader_module,
			/* pName */               shader_entry_name,
			/* pSpecializationInfo */ nullptr,
		},
		VkPipelineShaderStageCreateInfo
		{
			/* sType */               VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			/* pNext */               nullptr,
			/* flags */               VkPipelineShaderStageCreateFlags{},
			/* stage */               VK_SHADER_STAGE_FRAGMENT_BIT,
			/* module */              vk_fragment_shader_module,
			/* pName */               shader_entry_name,
			/* pSpecializationInfo */ nullptr,
		}
	};
	constexpr std::uint32_t stage_count = std::extent<decltype(vk_pipeline_shader_stage_create_infos)>::value;
	// Vertex input state.
	VkR3rVertexInput* vertex_input = static_cast<VkR3rVertexInput*>(context_.draw_state.vertex_input);
	// Input assembly.
	const VkPipelineInputAssemblyStateCreateInfo vk_pipeline_input_assembly_state_create_info
	{
		/* sType */                  VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		/* pNext */                  nullptr,
		/* flags */                  VkPipelineInputAssemblyStateCreateFlags{},
		/* topology */               VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		/* primitiveRestartEnable */ VK_FALSE,
	};
	// Viewport state.
	const VkViewport vk_viewport
	{
		/* x */        0.0F,
		/* y */        0.0F,
		/* width */    0.0F,
		/* height */   0.0F,
		/* minDepth */ 0.0F,
		/* maxDepth */ 0.0F,
	};
	const VkRect2D vk_scissor
	{
		/* offset */ VkOffset2D
		             {
		                 /* x */ 0,
		                 /* y */ 0,
		             },
		/* extent */ VkExtent2D
		             {
		                 /* width */  0,
		                 /* height */ 0,
		             }
	};
	const VkPipelineViewportStateCreateInfo vk_pipeline_viewport_state_create_info
	{
		/* sType */         VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		/* pNext */         nullptr,
		/* flags */         VkPipelineViewportStateCreateFlags{},
		/* viewportCount */ 1,
		/* pViewports */    &vk_viewport,
		/* scissorCount */  1,
		/* pScissors */     &vk_scissor,
	};
	// Rasterization state.
	const bool is_cull_mode_enabled = context_.draw_state.is_culling_enabled;
	const VkCullModeFlags vk_cull_mode_flags = is_cull_mode_enabled ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
	const VkPipelineRasterizationStateCreateInfo vk_pipeline_rasterization_state_create_info
	{
		/* sType */                   VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		/* pNext */                   nullptr,
		/* flags */                   VkPipelineRasterizationStateCreateFlags{},
		/* depthClampEnable */        VK_FALSE,
		/* rasterizerDiscardEnable */ VK_FALSE,
		/* polygonMode */             VK_POLYGON_MODE_FILL,
		/* cullMode */                vk_cull_mode_flags,
		/* frontFace */               VK_FRONT_FACE_COUNTER_CLOCKWISE,
		/* depthBiasEnable */         VK_FALSE,
		/* depthBiasConstantFactor */ 0.0F,
		/* depthBiasClamp */          0.0F,
		/* depthBiasSlopeFactor */    0.0F,
		/* lineWidth */               1.0F,
	};
	// Multisample state.
	const VkPipelineMultisampleStateCreateInfo vk_pipeline_multisample_state_create_info
	{
		/* sType */                 VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		/* pNext */                 nullptr,
		/* flags */                 VkPipelineMultisampleStateCreateFlags{},
		/* rasterizationSamples */  static_cast<VkSampleCountFlagBits>(context_.sample_count),
		/* sampleShadingEnable */   VK_FALSE,
		/* minSampleShading */      1.0F,
		/* pSampleMask */           nullptr,
		/* alphaToCoverageEnable */ VK_FALSE,
		/* alphaToOneEnable */      VK_FALSE,
	};
	// Depth-stencil state.
	const VkPipelineDepthStencilStateCreateInfo vk_pipeline_depth_stencil_state_create_info
	{
		/* sType */                 VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		/* pNext */                 nullptr,
		/* flags */                 VkPipelineDepthStencilStateCreateFlags{},
		/* depthTestEnable */       context_.draw_state.is_depth_test_enabled,
		/* depthWriteEnable */      context_.draw_state.is_depth_write_enabled,
		/* depthCompareOp */        VK_COMPARE_OP_LESS,
		/* depthBoundsTestEnable */ VK_FALSE,
		/* stencilTestEnable */     VK_FALSE,
		/* front */                 VkStencilOpState
									{
										/* failOp */      VkStencilOp{},
										/* passOp */      VkStencilOp{},
										/* depthFailOp */ VkStencilOp{},
										/* compareOp */   VkCompareOp{},
										/* compareMask */ 0,
										/* writeMask */   0,
										/* reference */   0,
									},
		/* back */                  VkStencilOpState
									{
										/* failOp */      VkStencilOp{},
										/* passOp */      VkStencilOp{},
										/* depthFailOp */ VkStencilOp{},
										/* compareOp */   VkCompareOp{},
										/* compareMask */ 0,
										/* writeMask */   0,
										/* reference */   0,
									},
		/* minDepthBounds */        0.0F,
		/* maxDepthBounds */        1.0F,
	};
	// Dynamic state.
	const VkDynamicState vk_dynamic_states[] =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};
	constexpr std::uint32_t dynamic_state_count = std::extent<decltype(vk_dynamic_states)>::value;
	const VkPipelineDynamicStateCreateInfo vk_pipeline_dynamic_state_create_info
	{
		/* sType */             VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		/* pNext */             nullptr,
		/* flags */             VkPipelineDynamicStateCreateFlags{},
		/* dynamicStateCount */ dynamic_state_count,
		/* pDynamicStates */    vk_dynamic_states,
	};
	// Color blend state.
	const VkPipelineColorBlendAttachmentState vk_pipeline_color_blend_attachment_state
	{
		/* blendEnable */         context_.draw_state.is_blending_enabled,
		/* srcColorBlendFactor */ VK_BLEND_FACTOR_SRC_ALPHA,
		/* dstColorBlendFactor */ VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		/* colorBlendOp */        VK_BLEND_OP_ADD,
		/* srcAlphaBlendFactor */ VK_BLEND_FACTOR_SRC_ALPHA,
		/* dstAlphaBlendFactor */ VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
		/* alphaBlendOp */        VK_BLEND_OP_ADD,
		/* colorWriteMask */      VK_COLOR_COMPONENT_R_BIT |
		                              VK_COLOR_COMPONENT_G_BIT |
		                              VK_COLOR_COMPONENT_B_BIT |
		                              VK_COLOR_COMPONENT_A_BIT,
	};
	const VkPipelineColorBlendStateCreateInfo vk_pipeline_color_blend_state_create_info
	{
		/* sType */           VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		/* pNext */           nullptr,
		/* flags */           VkPipelineColorBlendStateCreateFlags{},
		/* logicOpEnable */   VK_FALSE,
		/* logicOp */         VK_LOGIC_OP_CLEAR,
		/* attachmentCount */ 1,
		/* pAttachments */    &vk_pipeline_color_blend_attachment_state,
		/* blendConstants */  {0.0F, 0.0F, 0.0F, 0.0F}
	};
	// Pipeline layout.
	const VkDescriptorSetLayout& vk_descriptor_set_layout =
		static_cast<VkR3rShaderStage*>(context_.draw_state.shader_stage)->get_vk_descriptor_set_layout();
	const VkPipelineLayoutCreateInfo vk_pipeline_layout_create_info
	{
		/* sType */                  VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		/* pNext */                  nullptr,
		/* flags */                  VkPipelineLayoutCreateFlags{},
		/* setLayoutCount */         1,
		/* pSetLayouts */            &vk_descriptor_set_layout,
		/* pushConstantRangeCount */ 0,
		/* pPushConstantRanges */    nullptr,
	};
	VkPipelineLayout vk_pipeline_layout{};
	vk_result = context_.vkCreatePipelineLayout(
		/* device */          context_.device.get(),
		/* pCreateInfo */     &vk_pipeline_layout_create_info,
		/* pAllocator */      nullptr,
		/* pPipelineLayout */ &vk_pipeline_layout
	);
	context_.ensure_success_vk_result(vk_result, "vkCreatePipelineLayout");
	VkR3rPipelineLayoutResource pipeline_layout{vk_pipeline_layout, VkR3rPipelineLayoutDeleter{context_}};
	// Graphics pipeline.
	const VkGraphicsPipelineCreateInfo vk_graphics_pipeline_create_info
	{
		/* sType */               VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		/* pNext */               nullptr,
		/* flags */               VkPipelineCreateFlags{},
		/* stageCount */          stage_count,
		/* pStages */             vk_pipeline_shader_stage_create_infos,
		/* pVertexInputState */   &vertex_input->get_vk_create_info(),
		/* pInputAssemblyState */ &vk_pipeline_input_assembly_state_create_info,
		/* pTessellationState */  nullptr,
		/* pViewportState */      &vk_pipeline_viewport_state_create_info,
		/* pRasterizationState */ &vk_pipeline_rasterization_state_create_info,
		/* pMultisampleState */   &vk_pipeline_multisample_state_create_info,
		/* pDepthStencilState */  &vk_pipeline_depth_stencil_state_create_info,
		/* pColorBlendState */    &vk_pipeline_color_blend_state_create_info,
		/* pDynamicState */       &vk_pipeline_dynamic_state_create_info,
		/* layout */              vk_pipeline_layout,
		/* renderPass */          context_.render_pass.get(),
		/* subpass */             0,
		/* basePipelineHandle */  VkPipeline{},
		/* basePipelineIndex */   0,
	};
	VkPipeline vk_pipeline{};
	vk_result = context_.vkCreateGraphicsPipelines(
		/* device */          context.device.get(),
		/* pipelineCache */   VkPipelineCache{},
		/* createInfoCount */ 1,
		/* pCreateInfos */    &vk_graphics_pipeline_create_info,
		/* pAllocator */      nullptr,
		/* pPipelines */      &vk_pipeline
	);
	context_.ensure_success_vk_result(vk_result, "vkCreateGraphicsPipelines");
	VkR3rPipelineResource pipeline{vk_pipeline, VkR3rPipelineDeleter{context_}};
	//
	pipeline_layout_.swap(pipeline_layout);
	pipeline_.swap(pipeline);
}

void* VkR3rPipelineImpl::operator new(std::size_t size)
try {
	return memory_pool_.allocate(static_cast<std::intptr_t>(size));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void VkR3rPipelineImpl::operator delete(void* ptr)
{
	memory_pool_.deallocate(ptr);
}

void VkR3rPipelineImpl::do_draw_indexed(const VkR3rPipelineDrawIndexedParam& param)
{
	enqueue_bind_pipeline();
	enqueue_viewport();
	enqueue_scissor();
	enqueue_bind_index_buffer(param);
	enqueue_bind_vertex_buffer();
	enqueue_bind_descriptor_set();
	enqueue_draw_indexed(param);
}

void VkR3rPipelineImpl::enqueue_bind_pipeline()
{
	context_.vkCmdBindPipeline(
		/* commandBuffer */     context_.command_buffer,
		/* pipelineBindPoint */ VK_PIPELINE_BIND_POINT_GRAPHICS,
		/* pipeline */          pipeline_.get()
	);
}

void VkR3rPipelineImpl::enqueue_viewport()
{
	context_.vkCmdSetViewport(
		/* commandBuffer */ context_.command_buffer,
		/* firstViewport */ 0,
		/* viewportCount */ 1,
		/* pViewports */    &context_.draw_state.viewport
	);
}

void VkR3rPipelineImpl::enqueue_scissor()
{
	context_.vkCmdSetScissor(
		/* commandBuffer */ context_.command_buffer,
		/* firstScissor */  0,
		/* scissorCount */  1,
		/* pScissors */     &context_.draw_state.scissor
	);
}

void VkR3rPipelineImpl::enqueue_bind_index_buffer(const VkR3rPipelineDrawIndexedParam& param)
{
	VkBuffer const vk_index_buffer =
		static_cast<VkR3rVertexInput*>(context_.draw_state.vertex_input)->get_index_buffer()->get_vk_buffer();
	const VkIndexType vk_index_type = param.index_byte_depth <= 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
	context_.vkCmdBindIndexBuffer(
		/* commandBuffer */ context_.command_buffer,
		/* buffer */        vk_index_buffer,
		/* offset */        static_cast<std::uint32_t>(param.index_buffer_offset),
		/* indexType */     vk_index_type
	);
}

void VkR3rPipelineImpl::enqueue_bind_vertex_buffer()
{
	VkBuffer vk_vertex_buffers[2] =
	{
		static_cast<VkR3rVertexInput*>(context_.draw_state.vertex_input)->get_vertex_buffer()->get_vk_buffer(),
		static_cast<VkR3rVertexInput*>(context_.draw_state.vertex_input)->get_vk_generic_buffer()
	};
	const bool has_generic_buffer = vk_vertex_buffers[1] != VkBuffer{};
	const VkDeviceSize vk_offsets[2] = {0, 0};
	context_.vkCmdBindVertexBuffers(
		/* commandBuffer */ context_.command_buffer,
		/* firstBinding */  0,
		/* bindingCount */  1 + has_generic_buffer,
		/* pBuffers */      vk_vertex_buffers,
		/* pOffsets */      vk_offsets
	);
}

void VkR3rPipelineImpl::enqueue_bind_descriptor_set()
{
	VkR3rShaderStage* const shader_stage = static_cast<VkR3rShaderStage*>(context_.draw_state.shader_stage);
	VkDescriptorSet const vk_descriptor_set = shader_stage->acquire_vk_descriptor_set();
	std::uint32_t vk_image_binding = shader_stage->get_vk_image_binding();
	VkSampler vk_sampler = static_cast<VkR3rSampler*>(context_.draw_state.sampler)->get_vk_sampler();
	VkImageView const vk_image_view = static_cast<VkR3rR2Texture*>(context_.draw_state.r2_texture)->get_vk_image_view();
	const VkDescriptorImageInfo vk_descriptor_image_info
	{
		/* sampler */     vk_sampler,
		/* imageView */   vk_image_view,
		/* imageLayout */ VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};
	const VkWriteDescriptorSet vk_write_descriptor_set
	{
		/* sType */            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		/* pNext */            nullptr,
		/* dstSet */           vk_descriptor_set,
		/* dstBinding */       vk_image_binding,
		/* dstArrayElement */  0,
		/* descriptorCount */  1,
		/* descriptorType */   VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		/* pImageInfo */       &vk_descriptor_image_info,
		/* pBufferInfo */      nullptr,
		/* pTexelBufferView */ nullptr,
	};
	context_.vkUpdateDescriptorSets(
		/* device */               context_.device.get(),
		/* descriptorWriteCount */ 1,
		/* pDescriptorWrites */    &vk_write_descriptor_set,
		/* descriptorCopyCount */  0,
		/* pDescriptorCopies */    nullptr
	);
	context_.vkCmdBindDescriptorSets(
		/* commandBuffer */      context_.command_buffer,
		/* pipelineBindPoint */  VK_PIPELINE_BIND_POINT_GRAPHICS,
		/* layout */             pipeline_layout_.get(),
		/* firstSet */           0,
		/* descriptorSetCount */ 1,
		/* pDescriptorSets */    &vk_descriptor_set,
		/* dynamicOffsetCount */ 0,
		/* pDynamicOffsets */    nullptr
	);
}

void VkR3rPipelineImpl::enqueue_draw_indexed(const VkR3rPipelineDrawIndexedParam& param)
{
	context_.vkCmdDrawIndexed(
		/* commandBuffer */ context_.command_buffer,
		/* indexCount */    static_cast<std::uint32_t>(param.vertex_count),
		/* instanceCount */ 1,
		/* firstIndex */    param.index_offset,
		/* vertexOffset */  0,
		/* firstInstance */ 0
	);
}

} // namespace

// ======================================

void VkR3rPipeline::draw_indexed(const VkR3rPipelineDrawIndexedParam& param)
{
	do_draw_indexed(param);
}

// ======================================

VkR3rPipelineUPtr make_vk_r3r_pipeline(VkR3rContext& context)
{
	return std::make_unique<VkR3rPipelineImpl>(context);
}

} // namespace bstone
