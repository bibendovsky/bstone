/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: Vertex Input

#include <algorithm>

#include "bstone_exception.h"
#include "bstone_fixed_pool_resource.h"
#include "bstone_unique_resource.h"

#include "bstone_r3r_limits.h"
#include "bstone_r3r_tests.h"

#include "bstone_gl_r3r_buffer.h"
#include "bstone_gl_r3r_context.h"
#include "bstone_gl_r3r_device_features.h"
#include "bstone_gl_r3r_error.h"
#include "bstone_gl_r3r_utils.h"
#include "bstone_gl_r3r_vertex_input.h"
#include "bstone_gl_r3r_vertex_input_mgr.h"
#include "bstone_r3r_utils.h"

namespace bstone {

GlR3rVertexInput::GlR3rVertexInput() = default;

GlR3rVertexInput::~GlR3rVertexInput() = default;

// =========================================================================

class GlR3rVertexInputImpl final : public GlR3rVertexInput
{
public:
	GlR3rVertexInputImpl(
		GlR3rVertexInputMgr& vertex_input_manager,
		const R3rCreateVertexInputParam& param);

	~GlR3rVertexInputImpl() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

	void bind_vao() override;
	R3rBuffer* get_index_buffer() const noexcept override;

	void bind() override;

private:
	GlR3rVertexInputMgr& manager_;
	const R3rDeviceFeatures& device_features_;
	const GlR3rDeviceFeatures& gl_device_features_;

	GlR3rBuffer* index_buffer_{};
	R3rVertexAttribDescrs attrib_descrs_{};

	struct VaoDeleter
	{
		void operator()(GLuint gl_name) noexcept;
	};

	using VaoResource = UniqueResource<GLuint, VaoDeleter>;
	VaoResource vao_resource_{};

private:
	void initialize_vao();

	void enable_attrib_array(int index, bool is_enable);

	void assign_default_attribute(const R3rVertexAttribDescr& attribute_description);
	void assign_regular_attribute(const R3rVertexAttribDescr& attribute_description);
	void assign_attribute(const R3rVertexAttribDescr& attribute_description);

	void bind_internal();
};

// =========================================================================

using GlR3rVertexInputImplPool = FixedPoolResource<GlR3rVertexInputImpl, R3rLimits::max_vertex_inputs()>;
GlR3rVertexInputImplPool gl_r3r_vertex_input_impl_pool{};

// =========================================================================

GlR3rVertexInputImpl::GlR3rVertexInputImpl(
	GlR3rVertexInputMgr& vertex_input_manager,
	const R3rCreateVertexInputParam& param)
try
	:
	manager_{vertex_input_manager},
	device_features_{vertex_input_manager.get_context().get_device_features()},
	gl_device_features_{vertex_input_manager.get_context().get_gl_device_features()}
{
	const auto max_locations = device_features_.max_vertex_input_locations;

	R3rUtils::vertex_input_validate_param(max_locations, param);

	const auto is_location_out_of_range = std::any_of(
		param.attrib_descrs.cbegin(),
		param.attrib_descrs.cend(),
		[max_locations](const R3rVertexAttribDescr& item)
		{
			return item.location < 0 || item.location >= max_locations;
		}
	);

	if (is_location_out_of_range)
	{
		BSTONE_THROW_STATIC_SOURCE("Location out of range.");
	}

	index_buffer_ = static_cast<GlR3rBuffer*>(param.index_buffer);
	attrib_descrs_ = param.attrib_descrs;

	initialize_vao();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlR3rVertexInputImpl::~GlR3rVertexInputImpl()
{
	manager_.bind_default_vao();
}

void* GlR3rVertexInputImpl::operator new(std::size_t size)
try {
	return gl_r3r_vertex_input_impl_pool.allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rVertexInputImpl::operator delete(void* ptr)
{
	gl_r3r_vertex_input_impl_pool.deallocate(ptr);
}

void GlR3rVertexInputImpl::bind_vao()
try {
	if (vao_resource_.get() != 0U)
	{
		glBindVertexArray(vao_resource_.get());
		GlR3rError::check_optionally();
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

R3rBuffer* GlR3rVertexInputImpl::get_index_buffer() const noexcept
{
	return index_buffer_;
}

void GlR3rVertexInputImpl::bind()
try {
	if (vao_resource_.get() != 0U)
	{
		bind_vao();

		for (const auto& attribute_description : attrib_descrs_)
		{
			if (attribute_description.is_default)
			{
				assign_default_attribute(attribute_description);
			}
		}
	}
	else
	{
		bind_internal();
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rVertexInputImpl::VaoDeleter::operator()(GLuint gl_name) noexcept
{
	glDeleteVertexArrays(1, &gl_name);
	GlR3rError::ensure_no_errors_assert();
}

void GlR3rVertexInputImpl::initialize_vao()
try {
	if (gl_device_features_.is_vao_available)
	{
		auto gl_name = GLuint{};
		glGenVertexArrays(1, &gl_name);
		GlR3rError::check_optionally();

		vao_resource_.reset(gl_name);

		if (vao_resource_.get() == 0U)
		{
			BSTONE_THROW_STATIC_SOURCE("Failed to create VAO.");
		}

		bind_vao();

		if (index_buffer_ != nullptr)
		{
			index_buffer_->set(true);
		}

		for (const auto& attribute_description : attrib_descrs_)
		{
			if (!attribute_description.is_default)
			{
				assign_regular_attribute(attribute_description);
			}
		}
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rVertexInputImpl::enable_attrib_array(int index, bool is_enable)
try {
	const auto gl_func = (is_enable ? glEnableVertexAttribArray : glDisableVertexAttribArray);
	gl_func(index);
	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rVertexInputImpl::assign_default_attribute(const R3rVertexAttribDescr& attribute_description)
try {
	glVertexAttrib4fv(attribute_description.location, attribute_description.default_value.data());
	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rVertexInputImpl::assign_regular_attribute(const R3rVertexAttribDescr& attribute_description)
try {
	auto gl_component_count = GLint{};
	auto gl_component_format = GLenum{};
	auto gl_is_normalized = GLboolean{};

	switch (attribute_description.format)
	{
		case R3rVertexAttribFormat::rgba_8_unorm:
			gl_is_normalized = true;
			gl_component_count = 4;
			gl_component_format = GL_UNSIGNED_BYTE;
			break;

		case R3rVertexAttribFormat::rg_32_sfloat:
			gl_component_count = 2;
			gl_component_format = GL_FLOAT;
			break;

		case R3rVertexAttribFormat::rgb_32_sfloat:
			gl_component_count = 3;
			gl_component_format = GL_FLOAT;
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Invalid format.");
	}

	enable_attrib_array(attribute_description.location, true);

	auto vertex_buffer = static_cast<GlR3rBuffer*>(attribute_description.vertex_buffer);
	vertex_buffer->set(true);

	const auto vertex_buffer_data = reinterpret_cast<const void*>(
		static_cast<std::intptr_t>(attribute_description.offset));

	glVertexAttribPointer(
		attribute_description.location,
		gl_component_count,
		gl_component_format,
		gl_is_normalized,
		attribute_description.stride,
		vertex_buffer_data);

	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rVertexInputImpl::assign_attribute(const R3rVertexAttribDescr& attribute_description)
try {
	if (attribute_description.is_default)
	{
		assign_default_attribute(attribute_description);
	}
	else
	{
		assign_regular_attribute(attribute_description);
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rVertexInputImpl::bind_internal()
try {
	if (index_buffer_ != nullptr)
	{
		index_buffer_->set(true);
	}

	for (const auto& attribute_description : attrib_descrs_)
	{
		assign_attribute(attribute_description);
	}

	for (auto i = 0; i < device_features_.max_vertex_input_locations; ++i)
	{
		const auto it_end = attrib_descrs_.cend();

		const auto it = std::find_if(
			attrib_descrs_.cbegin(),
			it_end,
			[i](const R3rVertexAttribDescr& item)
			{
				return !item.is_default && item.location == i;
			}
		);

		if (it == it_end)
		{
			enable_attrib_array(i, false);
		}
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// =========================================================================

GlR3rVertexInputUPtr make_gl_r3r_vertex_input(
	GlR3rVertexInputMgr& vertex_input_manager,
	const R3rCreateVertexInputParam& param)
{
	return std::make_unique<GlR3rVertexInputImpl>(vertex_input_manager, param);
}

} // bstone
