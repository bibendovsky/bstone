/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: Buffer

#include "bstone_exception.h"
#include "bstone_fixed_pool_resource.h"
#include "bstone_unique_resource.h"

#include "bstone_r3r_limits.h"
#include "bstone_r3r_tests.h"

#include "bstone_gl_r3r_buffer.h"
#include "bstone_gl_r3r_context.h"
#include "bstone_gl_r3r_error.h"
#include "bstone_gl_r3r_utils.h"

namespace bstone {

GlR3rBuffer::GlR3rBuffer() = default;

GlR3rBuffer::~GlR3rBuffer() = default;

// =========================================================================

namespace {

class GlR3rBufferImpl final : public GlR3rBuffer
{
public:
	GlR3rBufferImpl(GlR3rContext& context, const R3rBufferInitParam& param);
	~GlR3rBufferImpl() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	R3rBufferType do_get_type() const noexcept override;
	R3rBufferUsageType do_get_usage_type() const noexcept override;
	int do_get_size() const noexcept override;

	void do_update(const R3rUpdateBufferParam& param) override;

private:
	void set(bool is_set) override;

private:
	struct BufferDeleter
	{
		void operator()(GLuint gl_name) const noexcept;
	};

	using BufferResource = UniqueResource<GLuint, BufferDeleter>;

private:
	GlR3rContext& context_;
	const GlR3rDeviceFeatures& gl_device_features_;

	R3rBufferType type_{};
	R3rBufferUsageType usage_type_{};
	int size_{};
	BufferResource buffer_resource_{};
	GLenum gl_target_{};

private:
	void validate(const R3rBufferInitParam& param);
	void validate(const R3rUpdateBufferParam& param);

	static GLenum gl_get_target(R3rBufferType type);
	static GLenum gl_get_usage(R3rBufferUsageType usage_type);
};

// =========================================================================

using GlR3rBufferImplPool = FixedPoolResource<GlR3rBufferImpl, R3rLimits::max_buffers()>;
GlR3rBufferImplPool gl_r3r_buffer_impl_pool{};

// =========================================================================

GlR3rBufferImpl::GlR3rBufferImpl(GlR3rContext& context, const R3rBufferInitParam& param)
try
	:
	context_{context},
	gl_device_features_{context_.get_gl_device_features()}
{
	validate(param);

	auto gl_name = GLuint{};

	if (gl_device_features_.is_dsa_available)
	{
		glCreateBuffers(1, &gl_name);
		GlR3rError::check_optionally();
	}
	else
	{
		glGenBuffers(1, &gl_name);
		GlR3rError::check_optionally();
	}

	buffer_resource_.reset(gl_name);

	if (buffer_resource_.get() == 0U)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to create a resource.");
	}

	type_ = param.type;
	usage_type_ = param.usage_type;
	size_ = param.size;
	gl_target_ = gl_get_target(param.type);
	const auto gl_usage = gl_get_usage(param.usage_type);

	if (gl_device_features_.is_dsa_available && gl_device_features_.is_buffer_storage_available)
	{
		glNamedBufferStorage(buffer_resource_.get(), param.size, nullptr, GL_DYNAMIC_STORAGE_BIT);
		GlR3rError::check_optionally();
	}
	else
	{
		set(true);

		glBufferData(gl_target_, param.size, nullptr, gl_usage);
		GlR3rError::check_optionally();
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlR3rBufferImpl::~GlR3rBufferImpl() = default;

void* GlR3rBufferImpl::operator new(std::size_t size)
try {
	return gl_r3r_buffer_impl_pool.allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rBufferImpl::operator delete(void* ptr)
{
	gl_r3r_buffer_impl_pool.deallocate(ptr);
}

R3rBufferType GlR3rBufferImpl::do_get_type() const noexcept
{
	return type_;
}

R3rBufferUsageType GlR3rBufferImpl::do_get_usage_type() const noexcept
{
	return usage_type_;
}

int GlR3rBufferImpl::do_get_size() const noexcept
{
	return size_;
}

void GlR3rBufferImpl::set(bool is_set)
try {
	const auto gl_name = (is_set ? buffer_resource_.get() : 0);
	glBindBuffer(gl_target_, gl_name);
	GlR3rError::check_optionally();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rBufferImpl::do_update(const R3rUpdateBufferParam& param)
try {
	validate(param);

	if (param.size == 0)
	{
		return;
	}

	if (gl_device_features_.is_dsa_available)
	{
		glNamedBufferSubData(buffer_resource_.get(), param.offset, param.size, param.data);
		GlR3rError::check_optionally();
	}
	else
	{
		set(true);

		glBufferSubData(gl_target_, param.offset, param.size, param.data);
		GlR3rError::check_optionally();
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rBufferImpl::BufferDeleter::operator()(GLuint gl_name) const noexcept
{
	glDeleteBuffers(1, &gl_name);
	GlR3rError::ensure_no_errors_assert();
}

void GlR3rBufferImpl::validate(const R3rBufferInitParam& param)
try {
	switch (param.type)
	{
		case R3rBufferType::index:
		case R3rBufferType::vertex:
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported type.");
	}

	switch (param.usage_type)
	{
		case R3rBufferUsageType::draw_streaming:
		case R3rBufferUsageType::draw_static:
		case R3rBufferUsageType::draw_dynamic:
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported usage type.");
	}

	if (param.size <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Size out of range.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rBufferImpl::validate(const R3rUpdateBufferParam& param)
try {
	if (param.offset < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Offset out of range.");
	}

	if (param.size < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Size out of range.");
	}

	if (param.offset > size_)
	{
		BSTONE_THROW_STATIC_SOURCE("Offset out of range.");
	}

	if (param.size > size_)
	{
		BSTONE_THROW_STATIC_SOURCE("Size out of range.");
	}

	if ((param.offset + param.size) > size_)
	{
		BSTONE_THROW_STATIC_SOURCE("End offset out of range.");
	}

	if (param.size > 0 && param.data == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null data.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GLenum GlR3rBufferImpl::gl_get_target(R3rBufferType type)
try {
	switch (type)
	{
		case R3rBufferType::index: return GL_ELEMENT_ARRAY_BUFFER;
		case R3rBufferType::vertex: return GL_ARRAY_BUFFER;
		default: BSTONE_THROW_STATIC_SOURCE("Unsupported type.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GLenum GlR3rBufferImpl::gl_get_usage(R3rBufferUsageType usage_type)
try {
	switch (usage_type)
	{
		case R3rBufferUsageType::draw_streaming: return GL_STREAM_DRAW;
		case R3rBufferUsageType::draw_static: return GL_STATIC_DRAW;
		case R3rBufferUsageType::draw_dynamic: return GL_DYNAMIC_DRAW;
		default: BSTONE_THROW_STATIC_SOURCE("Unsupported usage type.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace

// =========================================================================

GlR3rBufferUPtr make_gl_r3r_buffer(GlR3rContext& context, const R3rBufferInitParam& param)
try {
	return std::make_unique<GlR3rBufferImpl>(context, param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone
