/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer: Shader stage

#include "bstone_vk_r3r_shader_stage.h"
#include "bstone_exception.h"
#include "bstone_fixed_pool_resource.h"
#include "bstone_string_view_hasher.h"
#include "bstone_utility.h"
#include "bstone_r3r_limits.h"
#include "bstone_vk_r3r_context.h"
#include "bstone_vk_r3r_observer.h"
#include "bstone_vk_r3r_raii.h"
#include "bstone_vk_r3r_shader.h"
#include "bstone_vk_r3r_shader_var.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

// ======================================

namespace bstone {

namespace {

class VkR3rShaderStageImpl final : public VkR3rShaderStage
{
public:
	VkR3rShaderStageImpl(VkR3rContext& context, const R3rShaderStageInitParam& param);
	~VkR3rShaderStageImpl() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	R3rShaderVar* do_find_var(const char* name) override;
	R3rShaderVar* do_find_int32_var(const char* name) override;
	R3rShaderVar* do_find_float32_var(const char* name) override;
	R3rShaderVar* do_find_vec2_var(const char* name) override;
	R3rShaderVar* do_find_vec3_var(const char* name) override;
	R3rShaderVar* do_find_vec4_var(const char* name) override;
	R3rShaderVar* do_find_mat4_var(const char* name) override;
	R3rShaderVar* do_find_r2_sampler_var(const char* name) override;

	std::uint32_t do_get_vk_image_binding() const override;
	VkR3rShader* do_get_vertex_shader() const override;
	VkR3rShader* do_get_fragment_shader() const override;
	VkDescriptorSetLayout do_get_vk_descriptor_set_layout() const override;
	VkDescriptorSet do_acquire_vk_descriptor_set() override;

	static constexpr const char* const shader_module_default_entry_point = "main";

	class PostPresentObserver final : public VkR3rObserver<void>
	{
	public:
		explicit PostPresentObserver(VkR3rShaderStageImpl& shader_stage)
			:
			shader_stage_{shader_stage}
		{}

		~PostPresentObserver() override {}

	private:
		VkR3rShaderStageImpl& shader_stage_;

	private:
		void do_update() override
		{
			shader_stage_.on_post_present();
		}
	};

private:
	struct DescriptorContext
	{
		VkR3rDescriptorSetResource descriptor_set;
		VkR3rDeviceMemoryResource uniform_memory;
		VkR3rBufferResource uniform_buffer;
		void* uniform_mapped_memory;
	};
	using MemoryPool = FixedPoolResource<VkR3rShaderStageImpl, R3rLimits::max_shader_stages()>;
	using ShaderVars = std::vector<R3rShaderVarUPtr>;
	using DescriptorSetLayoutBindings = std::vector<VkDescriptorSetLayoutBinding>;
	using DescriptorContexts = std::vector<DescriptorContext>;
	using DescriptorBufferInfos = std::vector<VkDescriptorBufferInfo>;
	using WriteDescriptorSets = std::vector<VkWriteDescriptorSet>;
	using UniformHostMemory = std::vector<unsigned char>;

	static MemoryPool memory_pool_;
	VkR3rContext& context_;
	PostPresentObserver post_present_observer_;
	VkR3rShader* vertex_shader_{};
	VkR3rShader* fragment_shader_{};
	UniformHostMemory uniform_host_memory_{};
	std::uint32_t vk_image_binding_{};
	VkR3rDescriptorSetLayoutResource descriptor_set_layout_{};
	DescriptorBufferInfos descriptor_buffer_infos_{};
	WriteDescriptorSets write_descriptor_sets_{};
	std::size_t used_descriptor_set_count_{};
	DescriptorContexts descriptor_contexts_{};
	ShaderVars shader_vars_{};

	template<int N>
	static int pot_align(int value);
	static int align(int value, int alignment);
	static int calculate_member_alignment(R3rShaderVarTypeId shader_var_type_id);
	static int get_unit_size(R3rShaderVarTypeId type_id);
	void initialize(const R3rShaderStageInitParam& param);
	R3rShaderVar* impl_find_var(const char* name, R3rShaderVarTypeId shader_var_type_id);
	void on_post_present();
	void commit_uniforms(DescriptorContext& descriptor_context);
};

// --------------------------------------

VkR3rShaderStageImpl::MemoryPool VkR3rShaderStageImpl::memory_pool_{};

// --------------------------------------

VkR3rShaderStageImpl::VkR3rShaderStageImpl(VkR3rContext& context, const R3rShaderStageInitParam& param)
	:
	context_{context},
	post_present_observer_{*this}
{
	if (param.fragment_shader == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("No fragment shader.");
	}
	if (param.vertex_shader == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("No vertex shader.");
	}
	if (param.input_bindings.get_size() > R3rLimits::max_vertex_inputs())
	{
		BSTONE_THROW_STATIC_SOURCE("Too many input bindings.");
	}
	if (param.shader_var_infos.get_size() > R3rLimits::max_shader_vars())
	{
		BSTONE_THROW_STATIC_SOURCE("Too many shader varaible layouts.");
	}
	initialize(param);
}

VkR3rShaderStageImpl::~VkR3rShaderStageImpl()
{
	context_.post_present_subject.detach(post_present_observer_);
}

void* VkR3rShaderStageImpl::operator new(std::size_t size)
try {
	return memory_pool_.allocate(static_cast<std::intptr_t>(size));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void VkR3rShaderStageImpl::operator delete(void* ptr)
{
	memory_pool_.deallocate(ptr);
}

R3rShaderVar* VkR3rShaderStageImpl::do_find_var(const char* name)
{
	return impl_find_var(name, R3rShaderVarTypeId::none);
}

R3rShaderVar* VkR3rShaderStageImpl::do_find_int32_var(const char* name)
{
	return impl_find_var(name, R3rShaderVarTypeId::int32);
}

R3rShaderVar* VkR3rShaderStageImpl::do_find_float32_var(const char* name)
{
	return impl_find_var(name, R3rShaderVarTypeId::float32);
}

R3rShaderVar* VkR3rShaderStageImpl::do_find_vec2_var(const char* name)
{
	return impl_find_var(name, R3rShaderVarTypeId::vec2);
}

R3rShaderVar* VkR3rShaderStageImpl::do_find_vec3_var(const char* name)
{
	return impl_find_var(name, R3rShaderVarTypeId::vec4);
}

R3rShaderVar* VkR3rShaderStageImpl::do_find_vec4_var(const char* name)
{
	return impl_find_var(name, R3rShaderVarTypeId::vec4);
}

R3rShaderVar* VkR3rShaderStageImpl::do_find_mat4_var(const char* name)
{
	return impl_find_var(name, R3rShaderVarTypeId::mat4);
}

R3rShaderVar* VkR3rShaderStageImpl::do_find_r2_sampler_var(const char* name)
{
	return impl_find_var(name, R3rShaderVarTypeId::sampler2d);
}

std::uint32_t VkR3rShaderStageImpl::do_get_vk_image_binding() const
{
	return vk_image_binding_;
}

VkR3rShader* VkR3rShaderStageImpl::do_get_vertex_shader() const
{
	return vertex_shader_;
}

VkR3rShader* VkR3rShaderStageImpl::do_get_fragment_shader() const
{
	return fragment_shader_;
}

VkDescriptorSetLayout VkR3rShaderStageImpl::do_get_vk_descriptor_set_layout() const
{
	return descriptor_set_layout_.get();
}

VkDescriptorSet VkR3rShaderStageImpl::do_acquire_vk_descriptor_set()
{
	if (used_descriptor_set_count_ < descriptor_contexts_.size())
	{
		DescriptorContext& descriptor_context = descriptor_contexts_[used_descriptor_set_count_++];
		commit_uniforms(descriptor_context);
		return descriptor_context.descriptor_set.get();
	}
	VkDescriptorSetLayout const vk_descriptor_set_layout = descriptor_set_layout_.get();
	const VkDescriptorSetAllocateInfo vk_descriptor_set_allocate_info
	{
		/* sType */              VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		/* pNext */              nullptr,
		/* descriptorPool */     context_.descriptor_pool.get(),
		/* descriptorSetCount */ 1,
		/* pSetLayouts */        &vk_descriptor_set_layout,
	};
	VkDescriptorSet vk_descriptor_set{};
	const VkResult vk_result = context_.vkAllocateDescriptorSets(
		/* device */          context_.device.get(),
		/* pAllocateInfo */   &vk_descriptor_set_allocate_info,
		/* pDescriptorSets */ &vk_descriptor_set
	);
	context_.ensure_success_vk_result(vk_result, "vkAllocateDescriptorSets");
	DescriptorContext descriptor_context{};
	descriptor_context.descriptor_set.reset(vk_descriptor_set, VkR3rDescriptorSetDeleter{context_});
	context_.create_buffer_resource(
		uniform_host_memory_.size(),
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		descriptor_context.uniform_buffer,
		descriptor_context.uniform_memory);
	descriptor_context.uniform_mapped_memory = context_.map_memory(descriptor_context.uniform_memory.get());
	commit_uniforms(descriptor_context);
	for (VkWriteDescriptorSet& write_descriptor_set : write_descriptor_sets_)
	{
		write_descriptor_set.dstSet = vk_descriptor_set;
	}
	for (auto& descriptor_buffer_info : descriptor_buffer_infos_)
	{
		descriptor_buffer_info.buffer = descriptor_context.uniform_buffer.get();
	}
	context_.vkUpdateDescriptorSets(
		/* device */               context_.device.get(),
		/* descriptorWriteCount */ static_cast<std::uint32_t>(write_descriptor_sets_.size()),
		/* pDescriptorWrites */    write_descriptor_sets_.data(),
		/* descriptorCopyCount */  0,
		/* pDescriptorCopies */    nullptr
	);
	descriptor_contexts_.emplace_back(std::move(descriptor_context));
	++used_descriptor_set_count_;
	return vk_descriptor_set;
}

template<int N>
int VkR3rShaderStageImpl::pot_align(int value)
{
	constexpr int alignment_minus_one = (1 << N) - 1;
	return (value + alignment_minus_one) & (~alignment_minus_one);
}

int VkR3rShaderStageImpl::align(int value, int alignment)
{
	switch (alignment)
	{
		case 4: return pot_align<2>(value);
		case 8: return pot_align<3>(value);
		case 16: return pot_align<4>(value);
		default: return ((value + alignment - 1) / alignment) * alignment;
	}
}

int VkR3rShaderStageImpl::calculate_member_alignment(R3rShaderVarTypeId shader_var_type_id)
{
	switch (shader_var_type_id)
	{
		case R3rShaderVarTypeId::int32:
		case R3rShaderVarTypeId::float32:
			return 4;
		case R3rShaderVarTypeId::vec2:
			return 8;
		case R3rShaderVarTypeId::vec4:
		case R3rShaderVarTypeId::mat4:
			return 16;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown shader var type id.");
	}
}

int VkR3rShaderStageImpl::get_unit_size(R3rShaderVarTypeId type_id)
try {
	switch (type_id)
	{
		case R3rShaderVarTypeId::int32:
		case R3rShaderVarTypeId::float32:
		case R3rShaderVarTypeId::sampler2d:
			return 4;
		case R3rShaderVarTypeId::vec2: return 2 * 4;
		case R3rShaderVarTypeId::vec3: return 3 * 4;
		case R3rShaderVarTypeId::vec4: return 4 * 4;
		case R3rShaderVarTypeId::mat4: return 4 * 4 * 4;
		default: BSTONE_THROW_STATIC_SOURCE("Unsupported type.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void VkR3rShaderStageImpl::initialize(const R3rShaderStageInitParam& param)
{
	using NameViewSet = std::unordered_set<std::string_view, StringViewHasher>;
	struct Info
	{
		const R3rShaderVarInfo* var;
		int offset;
	};
	using Infos = std::vector<Info>;
	using BindingToInfoMap = std::unordered_map<int, Infos>;
	using TypeToBindingMap = std::unordered_map<R3rShaderVarType, BindingToInfoMap>;
	using StageToTypesMap = std::unordered_map<R3rShaderVarStage, TypeToBindingMap>;
	StageToTypesMap stage_to_type_map{};
	NameViewSet unique_names{};
	unique_names.reserve(static_cast<std::size_t>(param.shader_var_infos.get_size()));
	for (const R3rShaderVarInfo& info : param.shader_var_infos)
	{
		if (info.name == nullptr)
		{
			BSTONE_THROW_STATIC_SOURCE("Null shader variable name.");
		}
		const std::string_view info_name_view{info.name};
		if (info_name_view.empty())
		{
			BSTONE_THROW_STATIC_SOURCE("Empty shader variable name.");
		}
		if (!unique_names.emplace(info_name_view).second)
		{
			BSTONE_THROW_STATIC_SOURCE("Duplicate shader variable name.");
		}
		switch (info.stage)
		{
			case R3rShaderVarStage::fragment:
			case R3rShaderVarStage::vertex:
				break;
			default:
				BSTONE_THROW_STATIC_SOURCE("Unknown shader variable stage.");
		}
		switch (info.type)
		{
			case R3rShaderVarType::attribute:
			case R3rShaderVarType::sampler:
			case R3rShaderVarType::uniform:
				break;
			default:
				BSTONE_THROW_STATIC_SOURCE("Unknown shader variable type.");
		}
		switch (info.type_id)
		{
			case R3rShaderVarTypeId::int32:
			case R3rShaderVarTypeId::float32:
			case R3rShaderVarTypeId::vec2:
			case R3rShaderVarTypeId::vec3:
			case R3rShaderVarTypeId::vec4:
			case R3rShaderVarTypeId::mat4:
			case R3rShaderVarTypeId::sampler2d:
				break;
			default:
				BSTONE_THROW_STATIC_SOURCE("Unknown shader variable type id.");
		}

		if (info.binding < 0)
		{
			BSTONE_THROW_STATIC_SOURCE("Negative shader variable binding.");
		}
		//
		auto stage_to_type_iter = stage_to_type_map.find(info.stage);
		if (stage_to_type_iter == stage_to_type_map.cend())
		{
			stage_to_type_iter = stage_to_type_map.emplace(info.stage, TypeToBindingMap{}).first;
		}
		//
		TypeToBindingMap& type_to_binding_map = stage_to_type_iter->second;
		auto type_to_binding_iter = type_to_binding_map.find(info.type);
		if (type_to_binding_iter == type_to_binding_map.cend())
		{
			type_to_binding_iter = type_to_binding_map.emplace(info.type, BindingToInfoMap{}).first;
		}
		//
		BindingToInfoMap& binding_to_info_map = type_to_binding_iter->second;
		auto binding_to_info_iter = binding_to_info_map.find(info.binding);
		if (binding_to_info_iter == binding_to_info_map.cend())
		{
			binding_to_info_iter = binding_to_info_map.emplace(info.binding, Infos{}).first;
		}
		binding_to_info_iter->second.emplace_back(Info{&info, 0});
	}
	const int uniform_buffer_offset_alignment = static_cast<int>(
		context_.physical_device_properties.limits.minUniformBufferOffsetAlignment);
	int uniform_buffer_offset = 0;
	using DescriptorSetLayoutBindings = std::vector<VkDescriptorSetLayoutBinding>;
	DescriptorSetLayoutBindings descriptor_set_layout_bindings{};
	descriptor_set_layout_bindings.reserve(static_cast<std::size_t>(param.shader_var_infos.get_size()));
	descriptor_buffer_infos_.reserve(static_cast<std::size_t>(param.shader_var_infos.get_size()));
	write_descriptor_sets_.reserve(static_cast<std::size_t>(param.shader_var_infos.get_size()));
	for (auto& stage_to_type : stage_to_type_map)
	{
		for (auto& type_to_binding : stage_to_type.second)
		{
			if (type_to_binding.first != R3rShaderVarType::uniform)
			{
				continue;
			}
			for (auto& binding_to_info : type_to_binding.second)
			{
				Infos& infos = binding_to_info.second;
				if (infos.empty())
				{
					BSTONE_THROW_STATIC_SOURCE("Empty info list.");
				}
				bool is_first = true;
				uniform_buffer_offset = align(uniform_buffer_offset, uniform_buffer_offset_alignment);
				int old_uniform_buffer_offset = uniform_buffer_offset;
				for (Info& info : infos)
				{
					const int alignment = calculate_member_alignment(info.var->type_id);
					uniform_buffer_offset = align(uniform_buffer_offset, alignment);
					if (is_first)
					{
						is_first = false;
						old_uniform_buffer_offset = uniform_buffer_offset;
					}
					info.offset = uniform_buffer_offset;
					uniform_buffer_offset += get_unit_size(info.var->type_id);
				}
				const std::uint32_t vk_binding = static_cast<std::uint32_t>(binding_to_info.first);
				VkShaderStageFlags vk_stage{};
				switch (stage_to_type.first)
				{
					case R3rShaderVarStage::fragment:
						vk_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
						break;
					case R3rShaderVarStage::vertex:
						vk_stage = VK_SHADER_STAGE_VERTEX_BIT;
						break;
				}
				descriptor_set_layout_bindings.emplace_back(
					VkDescriptorSetLayoutBinding
					{
						/* binding */            vk_binding,
						/* descriptorType */     VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
						/* descriptorCount */    1,
						/* stageFlags */         vk_stage,
						/* pImmutableSamplers */ nullptr,
					}
				);
				descriptor_buffer_infos_.emplace_back(
					VkDescriptorBufferInfo
					{
						/* buffer */ VkBuffer{},
						/* offset */ static_cast<VkDeviceSize>(old_uniform_buffer_offset),
						/* range */  static_cast<VkDeviceSize>(uniform_buffer_offset - old_uniform_buffer_offset),
					}
				);
				write_descriptor_sets_.emplace_back(
					VkWriteDescriptorSet
					{
						/* sType */            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
						/* pNext */            nullptr,
						/* dstSet */           VkDescriptorSet{},
						/* dstBinding */       vk_binding,
						/* dstArrayElement */  0,
						/* descriptorCount */  1,
						/* descriptorType */   VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
						/* pImageInfo */       nullptr,
						/* pBufferInfo */      &descriptor_buffer_infos_.back(),
						/* pTexelBufferView */ nullptr,
					}
				);
			}
		}
	}
	const int uniform_buffer_size = align(uniform_buffer_offset, uniform_buffer_offset_alignment);
	uniform_host_memory_.resize(uniform_buffer_size);
	vk_image_binding_ = UINT32_MAX;
	for (const auto& stage_to_type : stage_to_type_map)
	{
		for (const auto& type_to_binding : stage_to_type.second)
		{
			if (type_to_binding.first != R3rShaderVarType::sampler)
			{
				continue;
			}
			for (const auto& binding_to_info : type_to_binding.second)
			{
				if (vk_image_binding_ != UINT32_MAX)
				{
					BSTONE_THROW_STATIC_SOURCE("Too many samplers.");
				}
				if (stage_to_type.first != R3rShaderVarStage::fragment)
				{
					BSTONE_THROW_STATIC_SOURCE("Expected fragment stage.");
				}
				vk_image_binding_ = static_cast<std::uint32_t>(binding_to_info.first);
				descriptor_set_layout_bindings.emplace_back(
					VkDescriptorSetLayoutBinding
					{
						/* binding */            vk_image_binding_,
						/* descriptorType */     VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
						/* descriptorCount */    1,
						/* stageFlags */         VK_SHADER_STAGE_FRAGMENT_BIT,
						/* pImmutableSamplers */ nullptr,
					}
				);
			}
		}
	}
	if (vk_image_binding_ == UINT32_MAX)
	{
		BSTONE_THROW_STATIC_SOURCE("Expected a sampler.");
	}
	VkResult vk_result;
	const VkDescriptorSetLayoutCreateInfo vk_descriptor_set_layout_create_info
	{
		/* sType */        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		/* pNext */        nullptr,
		/* flags */        VkDescriptorSetLayoutCreateFlags{},
		/* bindingCount */ static_cast<std::uint32_t>(descriptor_set_layout_bindings.size()),
		/* pBindings */    descriptor_set_layout_bindings.data(),
	};
	VkDescriptorSetLayout vk_descriptor_set_layout{};
	vk_result = context_.vkCreateDescriptorSetLayout(
		/* device */      context_.device.get(),
		/* pCreateInfo */ &vk_descriptor_set_layout_create_info,
		/* pAllocator */  nullptr,
		/* pSetLayout */  &vk_descriptor_set_layout
	);
	context_.ensure_success_vk_result(vk_result, "vkCreateDescriptorSetLayout");
	descriptor_set_layout_.reset(vk_descriptor_set_layout, VkR3rDescriptorSetLayoutDeleter{context_});
	for (auto& stage_to_type : stage_to_type_map)
	{
		for (auto& type_to_binding : stage_to_type.second)
		{
			for (auto& binding_to_info : type_to_binding.second)
			{
				int index = 0;
				for (Info& info : binding_to_info.second)
				{
					shader_vars_.emplace_back(make_vk_r3r_shader_var(
						info.var->type,
						info.var->type_id,
						index++,
						info.var->name,
						uniform_host_memory_.data() + info.offset));
				}
			}
		}
	}
	vertex_shader_ = static_cast<VkR3rShader*>(param.vertex_shader);
	fragment_shader_ = static_cast<VkR3rShader*>(param.fragment_shader);
	descriptor_contexts_.reserve(64);
	context_.post_present_subject.attach(post_present_observer_);
}

R3rShaderVar* VkR3rShaderStageImpl::impl_find_var(const char* name, R3rShaderVarTypeId shader_var_type_id)
{
	for (R3rShaderVarUPtr& shader_var : shader_vars_)
	{
		if (shader_var->get_name() == name && shader_var->get_type_id() == shader_var_type_id)
		{
			return shader_var.get();
		}
	}
	return nullptr;
}

void VkR3rShaderStageImpl::on_post_present()
{
	used_descriptor_set_count_ = 0;
}

void VkR3rShaderStageImpl::commit_uniforms(DescriptorContext& descriptor_context)
{
	void* const uniform_mapped_memory = descriptor_context.uniform_mapped_memory;
	std::memcpy(uniform_mapped_memory, uniform_host_memory_.data(), uniform_host_memory_.size());
}

} // namespace

// ======================================

std::uint32_t VkR3rShaderStage::get_vk_image_binding() const
{
	return do_get_vk_image_binding();
}

VkR3rShader* VkR3rShaderStage::get_vertex_shader() const
{
	return do_get_vertex_shader();
}

VkR3rShader* VkR3rShaderStage::get_fragment_shader() const
{
	return do_get_fragment_shader();
}

VkDescriptorSetLayout VkR3rShaderStage::get_vk_descriptor_set_layout() const
{
	return do_get_vk_descriptor_set_layout();
}

VkDescriptorSet VkR3rShaderStage::acquire_vk_descriptor_set()
{
	return do_acquire_vk_descriptor_set();
}

// ======================================

VkR3rShaderStageUPtr make_vk_r3r_shader_stage(VkR3rContext& context, const R3rShaderStageInitParam& param)
{
	return std::make_unique<VkR3rShaderStageImpl>(context, param);
}

} // namespace bstone
