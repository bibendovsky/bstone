/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: Vertex Input Manager

#include "bstone_exception.h"
#include "bstone_single_pool_resource.h"

#include "bstone_r3r_tests.h"

#include "bstone_gl_r3r_context.h"
#include "bstone_gl_r3r_vertex_input.h"
#include "bstone_gl_r3r_vertex_input_mgr.h"

namespace bstone {

GlR3rVertexInputMgr::GlR3rVertexInputMgr() noexcept = default;

GlR3rVertexInputMgr::~GlR3rVertexInputMgr() = default;

// ==========================================================================

class GlR3rVertexInputMgrImpl final : public GlR3rVertexInputMgr
{
public:
	GlR3rVertexInputMgrImpl(GlR3rContext& context);
	~GlR3rVertexInputMgrImpl() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

	GlR3rContext& get_context() const noexcept override;
	R3rVertexInputUPtr create(const R3rCreateVertexInputParam& param) override;
	void set(R3rVertexInput& vertex_input) override;
	void bind_default_vao() override;

private:
	GlR3rContext& context_;
	GlR3rVertexInputUPtr default_vertex_input_{};

private:
	void initialize_default_vertex_input();
};

// ==========================================================================

using GlR3rVertexInputMgrImplPool = SinglePoolResource<GlR3rVertexInputMgrImpl>;
GlR3rVertexInputMgrImplPool gl_r3r_vertex_input_mgr_impl_pool{};

// ==========================================================================

GlR3rVertexInputMgrImpl::GlR3rVertexInputMgrImpl(GlR3rContext& context)
try
	:
	context_{context},
	default_vertex_input_{}
{
	initialize_default_vertex_input();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlR3rVertexInputMgrImpl::~GlR3rVertexInputMgrImpl() = default;

void* GlR3rVertexInputMgrImpl::operator new(std::size_t size)
try {
	return gl_r3r_vertex_input_mgr_impl_pool.allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rVertexInputMgrImpl::operator delete(void* ptr)
{
	gl_r3r_vertex_input_mgr_impl_pool.deallocate(ptr);
}

GlR3rContext& GlR3rVertexInputMgrImpl::get_context() const noexcept
{
	return context_;
}

R3rVertexInputUPtr GlR3rVertexInputMgrImpl::create(const R3rCreateVertexInputParam& param)
try {
	return make_gl_r3r_vertex_input(*this, param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rVertexInputMgrImpl::set(R3rVertexInput& vertex_input)
try {
	static_cast<GlR3rVertexInput&>(vertex_input).bind();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rVertexInputMgrImpl::bind_default_vao()
try {
	if (default_vertex_input_ != nullptr)
	{
		default_vertex_input_->bind_vao();
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rVertexInputMgrImpl::initialize_default_vertex_input()
try {
	const auto param = R3rCreateVertexInputParam{};
	default_vertex_input_ = make_gl_r3r_vertex_input(*this, param);;
	bind_default_vao();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

GlR3rVertexInputMgrUPtr make_gl_r3r_vertex_input_mgr(GlR3rContext& context)
{
	return std::make_unique<GlR3rVertexInputMgrImpl>(context);
}

} // bstone
