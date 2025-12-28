/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: Extension Manager

#include <assert.h>
#include <stddef.h>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <unordered_map>

#include "bstone_exception.h"
#include "bstone_single_pool_resource.h"

#include "bstone_r3r_tests.h"

#include "bstone_gl_r3r_error.h"
#include "bstone_gl_r3r_extension_mgr.h"
#include "bstone_gl_r3r_utils.h"

#include "bstone_sys_gl_symbol_resolver.h"

// ==========================================================================

namespace bstone {

namespace {

class GlR3rExtensionMgrImpl final : public GlR3rExtensionMgr
{
public:
	GlR3rExtensionMgrImpl(const sys::GlSymbolResolver& symbol_resolver);
	~GlR3rExtensionMgrImpl() override {}

	void* operator new(size_t size);
	void operator delete(void* ptr);

	int get_count() const noexcept override;
	const std::string& get_name(int extension_index) const noexcept override;
	const GlR3rVersion& get_gl_version() const noexcept override;
	const GlR3rVersion& get_glsl_version() const noexcept override;

	void probe(GlR3rExtensionId extension_id) override;

	bool has(GlR3rExtensionId extension_id) const noexcept override;
	bool operator[](GlR3rExtensionId extension_id) const noexcept override;

private:
	using MemoryPool = SinglePoolResource<GlR3rExtensionMgrImpl>;

	using ExtensionNames = std::vector<std::string>;

	using GlSymbol = void (*)();
	using GlSymbolPtrs = std::vector<GlSymbol*>;
	using GlSymbolRegistry = std::unordered_map<GlSymbol*, const char*>;

	struct RegistryItem
	{
		bool is_virtual{};
		bool is_probed{};
		bool is_available{};

		std::string extension_name;
		GlSymbolPtrs* gl_symbol_ptrs{};
	};

	using Registry = std::vector<RegistryItem>;

private:
	static MemoryPool memory_pool_;

private:
	const sys::GlSymbolResolver& symbol_resolver_;
	ExtensionNames extension_names_{};
	Registry registry_{};
	GlR3rVersion gl_version_{};
	GlR3rVersion glsl_version_{};

private:
	static GlSymbolRegistry& get_gl_symbol_registry();
	static void clear_gl_symbols();
	void resolve_gl_symbols();
	static bool has_gl_symbol(const GlSymbolPtrs& gl_symbol_ptrs) noexcept;

	static GlSymbolPtrs& get_essentials_gl_symbol_ptrs();
	static GlSymbolPtrs& get_v2_0_gl_symbol_ptrs();
	static GlSymbolPtrs& get_v3_2_core_gl_symbol_ptrs();
	static GlSymbolPtrs& get_es_v2_0_gl_symbol_ptrs();
	static GlSymbolPtrs& get_arb_buffer_storage_gl_symbol_ptrs();
	static GlSymbolPtrs& get_arb_direct_state_access_gl_symbol_ptrs();
	static GlSymbolPtrs& get_arb_framebuffer_object_gl_symbol_ptrs();
	static GlSymbolPtrs& get_arb_sampler_objects_gl_symbol_ptrs();
	static GlSymbolPtrs& get_arb_separate_shader_objects_gl_symbol_ptrs();
	static GlSymbolPtrs& get_arb_vertex_array_object_gl_symbol_ptrs();
	static GlSymbolPtrs& get_ext_framebuffer_blit_gl_symbol_ptrs();
	static GlSymbolPtrs& get_ext_framebuffer_multisample_gl_symbol_ptrs();
	static GlSymbolPtrs& get_ext_framebuffer_object_gl_symbol_ptrs();

	static const std::string& get_empty_extension_name() noexcept;
	static int get_registered_extension_count() noexcept;
	static int get_extension_index(GlR3rExtensionId extension_id) noexcept;

	void initialize_registry();

	int parse_version_number(const std::string& string);
	void parse_version(const std::string& version_string, GlR3rVersion& gl_version);
	void get_version(GLenum version_enum, GlR3rVersion& gl_version);
	void get_version();

	void get_names_from_multiple_strings();
	void get_names_from_one_string();
	void get_names();

	void probe_generic(GlR3rExtensionId extension_id);
};

// ==========================================================================

GlR3rExtensionMgrImpl::MemoryPool GlR3rExtensionMgrImpl::memory_pool_{};

// ==========================================================================

GlR3rExtensionMgrImpl::GlR3rExtensionMgrImpl(const sys::GlSymbolResolver& symbol_resolver)
try
	:
	symbol_resolver_{symbol_resolver}
{
	clear_gl_symbols();
	resolve_gl_symbols();

	get_version();
	get_names();

	initialize_registry();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void* GlR3rExtensionMgrImpl::operator new(size_t size)
try {
	return memory_pool_.allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rExtensionMgrImpl::operator delete(void* ptr)
{
	memory_pool_.deallocate(ptr);
}

int GlR3rExtensionMgrImpl::get_count() const noexcept
{
	return static_cast<int>(extension_names_.size());
}

const std::string& GlR3rExtensionMgrImpl::get_name(int extension_index) const noexcept
{
	if (extension_index < 0 || extension_index >= get_count())
	{
		return get_empty_extension_name();
	}

	return extension_names_[extension_index];
}

const GlR3rVersion& GlR3rExtensionMgrImpl::get_gl_version() const noexcept
{
	return gl_version_;
}

const GlR3rVersion& GlR3rExtensionMgrImpl::get_glsl_version() const noexcept
{
	return glsl_version_;
}

void GlR3rExtensionMgrImpl::probe(GlR3rExtensionId extension_id)
try {
	probe_generic(extension_id);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

bool GlR3rExtensionMgrImpl::has(GlR3rExtensionId extension_id) const noexcept
{
	const auto extension_index = get_extension_index(extension_id);

	if (extension_index < 0)
	{
		return false;
	}

	return registry_[extension_index].is_available;
}

bool GlR3rExtensionMgrImpl::operator[](GlR3rExtensionId extension_id) const noexcept
{
	return has(extension_id);
}

GlR3rExtensionMgrImpl::GlSymbolRegistry& GlR3rExtensionMgrImpl::get_gl_symbol_registry()
try {
	static auto gl_symbol_registry = GlSymbolRegistry
	{
		{reinterpret_cast<GlSymbol*>(&glAccum), "glAccum"},
		{reinterpret_cast<GlSymbol*>(&glActiveShaderProgram), "glActiveShaderProgram"},
		{reinterpret_cast<GlSymbol*>(&glActiveTexture), "glActiveTexture"},
		{reinterpret_cast<GlSymbol*>(&glAlphaFunc), "glAlphaFunc"},
		{reinterpret_cast<GlSymbol*>(&glAreTexturesResident), "glAreTexturesResident"},
		{reinterpret_cast<GlSymbol*>(&glArrayElement), "glArrayElement"},
		{reinterpret_cast<GlSymbol*>(&glAttachShader), "glAttachShader"},
		{reinterpret_cast<GlSymbol*>(&glBeginConditionalRender), "glBeginConditionalRender"},
		{reinterpret_cast<GlSymbol*>(&glBegin), "glBegin"},
		{reinterpret_cast<GlSymbol*>(&glBeginQuery), "glBeginQuery"},
		{reinterpret_cast<GlSymbol*>(&glBeginTransformFeedback), "glBeginTransformFeedback"},
		{reinterpret_cast<GlSymbol*>(&glBindAttribLocation), "glBindAttribLocation"},
		{reinterpret_cast<GlSymbol*>(&glBindBufferARB), "glBindBufferARB"},
		{reinterpret_cast<GlSymbol*>(&glBindBufferBase), "glBindBufferBase"},
		{reinterpret_cast<GlSymbol*>(&glBindBuffer), "glBindBuffer"},
		{reinterpret_cast<GlSymbol*>(&glBindBufferRange), "glBindBufferRange"},
		{reinterpret_cast<GlSymbol*>(&glBindFragDataLocation), "glBindFragDataLocation"},
		{reinterpret_cast<GlSymbol*>(&glBindFramebufferEXT), "glBindFramebufferEXT"},
		{reinterpret_cast<GlSymbol*>(&glBindFramebuffer), "glBindFramebuffer"},
		{reinterpret_cast<GlSymbol*>(&glBindProgramPipeline), "glBindProgramPipeline"},
		{reinterpret_cast<GlSymbol*>(&glBindRenderbufferEXT), "glBindRenderbufferEXT"},
		{reinterpret_cast<GlSymbol*>(&glBindRenderbuffer), "glBindRenderbuffer"},
		{reinterpret_cast<GlSymbol*>(&glBindSampler), "glBindSampler"},
		{reinterpret_cast<GlSymbol*>(&glBindTexture), "glBindTexture"},
		{reinterpret_cast<GlSymbol*>(&glBindTextureUnit), "glBindTextureUnit"},
		{reinterpret_cast<GlSymbol*>(&glBindVertexArray), "glBindVertexArray"},
		{reinterpret_cast<GlSymbol*>(&glBitmap), "glBitmap"},
		{reinterpret_cast<GlSymbol*>(&glBlendColor), "glBlendColor"},
		{reinterpret_cast<GlSymbol*>(&glBlendEquation), "glBlendEquation"},
		{reinterpret_cast<GlSymbol*>(&glBlendEquationSeparate), "glBlendEquationSeparate"},
		{reinterpret_cast<GlSymbol*>(&glBlendFunc), "glBlendFunc"},
		{reinterpret_cast<GlSymbol*>(&glBlendFuncSeparate), "glBlendFuncSeparate"},
		{reinterpret_cast<GlSymbol*>(&glBlitFramebufferEXT), "glBlitFramebufferEXT"},
		{reinterpret_cast<GlSymbol*>(&glBlitFramebuffer), "glBlitFramebuffer"},
		{reinterpret_cast<GlSymbol*>(&glBlitNamedFramebuffer), "glBlitNamedFramebuffer"},
		{reinterpret_cast<GlSymbol*>(&glBufferDataARB), "glBufferDataARB"},
		{reinterpret_cast<GlSymbol*>(&glBufferData), "glBufferData"},
		{reinterpret_cast<GlSymbol*>(&glBufferStorage), "glBufferStorage"},
		{reinterpret_cast<GlSymbol*>(&glBufferSubDataARB), "glBufferSubDataARB"},
		{reinterpret_cast<GlSymbol*>(&glBufferSubData), "glBufferSubData"},
		{reinterpret_cast<GlSymbol*>(&glCallList), "glCallList"},
		{reinterpret_cast<GlSymbol*>(&glCallLists), "glCallLists"},
		{reinterpret_cast<GlSymbol*>(&glCheckFramebufferStatusEXT), "glCheckFramebufferStatusEXT"},
		{reinterpret_cast<GlSymbol*>(&glCheckFramebufferStatus), "glCheckFramebufferStatus"},
		{reinterpret_cast<GlSymbol*>(&glCheckNamedFramebufferStatus), "glCheckNamedFramebufferStatus"},
		{reinterpret_cast<GlSymbol*>(&glClampColorARB), "glClampColorARB"},
		{reinterpret_cast<GlSymbol*>(&glClampColor), "glClampColor"},
		{reinterpret_cast<GlSymbol*>(&glClearAccum), "glClearAccum"},
		{reinterpret_cast<GlSymbol*>(&glClearBufferfi), "glClearBufferfi"},
		{reinterpret_cast<GlSymbol*>(&glClearBufferfv), "glClearBufferfv"},
		{reinterpret_cast<GlSymbol*>(&glClearBufferiv), "glClearBufferiv"},
		{reinterpret_cast<GlSymbol*>(&glClearBufferuiv), "glClearBufferuiv"},
		{reinterpret_cast<GlSymbol*>(&glClearColor), "glClearColor"},
		{reinterpret_cast<GlSymbol*>(&glClearDepthf), "glClearDepthf"},
		{reinterpret_cast<GlSymbol*>(&glClearDepth), "glClearDepth"},
		{reinterpret_cast<GlSymbol*>(&glClearIndex), "glClearIndex"},
		{reinterpret_cast<GlSymbol*>(&glClearNamedBufferData), "glClearNamedBufferData"},
		{reinterpret_cast<GlSymbol*>(&glClearNamedBufferSubData), "glClearNamedBufferSubData"},
		{reinterpret_cast<GlSymbol*>(&glClearNamedFramebufferfi), "glClearNamedFramebufferfi"},
		{reinterpret_cast<GlSymbol*>(&glClearNamedFramebufferfv), "glClearNamedFramebufferfv"},
		{reinterpret_cast<GlSymbol*>(&glClearNamedFramebufferiv), "glClearNamedFramebufferiv"},
		{reinterpret_cast<GlSymbol*>(&glClearNamedFramebufferuiv), "glClearNamedFramebufferuiv"},
		{reinterpret_cast<GlSymbol*>(&glClear), "glClear"},
		{reinterpret_cast<GlSymbol*>(&glClearStencil), "glClearStencil"},
		{reinterpret_cast<GlSymbol*>(&glClientActiveTexture), "glClientActiveTexture"},
		{reinterpret_cast<GlSymbol*>(&glClientWaitSync), "glClientWaitSync"},
		{reinterpret_cast<GlSymbol*>(&glClipPlane), "glClipPlane"},
		{reinterpret_cast<GlSymbol*>(&glColor3b), "glColor3b"},
		{reinterpret_cast<GlSymbol*>(&glColor3bv), "glColor3bv"},
		{reinterpret_cast<GlSymbol*>(&glColor3d), "glColor3d"},
		{reinterpret_cast<GlSymbol*>(&glColor3dv), "glColor3dv"},
		{reinterpret_cast<GlSymbol*>(&glColor3f), "glColor3f"},
		{reinterpret_cast<GlSymbol*>(&glColor3fv), "glColor3fv"},
		{reinterpret_cast<GlSymbol*>(&glColor3i), "glColor3i"},
		{reinterpret_cast<GlSymbol*>(&glColor3iv), "glColor3iv"},
		{reinterpret_cast<GlSymbol*>(&glColor3s), "glColor3s"},
		{reinterpret_cast<GlSymbol*>(&glColor3sv), "glColor3sv"},
		{reinterpret_cast<GlSymbol*>(&glColor3ub), "glColor3ub"},
		{reinterpret_cast<GlSymbol*>(&glColor3ubv), "glColor3ubv"},
		{reinterpret_cast<GlSymbol*>(&glColor3ui), "glColor3ui"},
		{reinterpret_cast<GlSymbol*>(&glColor3uiv), "glColor3uiv"},
		{reinterpret_cast<GlSymbol*>(&glColor3us), "glColor3us"},
		{reinterpret_cast<GlSymbol*>(&glColor3usv), "glColor3usv"},
		{reinterpret_cast<GlSymbol*>(&glColor4b), "glColor4b"},
		{reinterpret_cast<GlSymbol*>(&glColor4bv), "glColor4bv"},
		{reinterpret_cast<GlSymbol*>(&glColor4d), "glColor4d"},
		{reinterpret_cast<GlSymbol*>(&glColor4dv), "glColor4dv"},
		{reinterpret_cast<GlSymbol*>(&glColor4f), "glColor4f"},
		{reinterpret_cast<GlSymbol*>(&glColor4fv), "glColor4fv"},
		{reinterpret_cast<GlSymbol*>(&glColor4i), "glColor4i"},
		{reinterpret_cast<GlSymbol*>(&glColor4iv), "glColor4iv"},
		{reinterpret_cast<GlSymbol*>(&glColor4s), "glColor4s"},
		{reinterpret_cast<GlSymbol*>(&glColor4sv), "glColor4sv"},
		{reinterpret_cast<GlSymbol*>(&glColor4ub), "glColor4ub"},
		{reinterpret_cast<GlSymbol*>(&glColor4ubv), "glColor4ubv"},
		{reinterpret_cast<GlSymbol*>(&glColor4ui), "glColor4ui"},
		{reinterpret_cast<GlSymbol*>(&glColor4uiv), "glColor4uiv"},
		{reinterpret_cast<GlSymbol*>(&glColor4us), "glColor4us"},
		{reinterpret_cast<GlSymbol*>(&glColor4usv), "glColor4usv"},
		{reinterpret_cast<GlSymbol*>(&glColorMaski), "glColorMaski"},
		{reinterpret_cast<GlSymbol*>(&glColorMask), "glColorMask"},
		{reinterpret_cast<GlSymbol*>(&glColorMaterial), "glColorMaterial"},
		{reinterpret_cast<GlSymbol*>(&glColorPointer), "glColorPointer"},
		{reinterpret_cast<GlSymbol*>(&glCompileShader), "glCompileShader"},
		{reinterpret_cast<GlSymbol*>(&glCompressedTexImage1D), "glCompressedTexImage1D"},
		{reinterpret_cast<GlSymbol*>(&glCompressedTexImage2D), "glCompressedTexImage2D"},
		{reinterpret_cast<GlSymbol*>(&glCompressedTexImage3D), "glCompressedTexImage3D"},
		{reinterpret_cast<GlSymbol*>(&glCompressedTexSubImage1D), "glCompressedTexSubImage1D"},
		{reinterpret_cast<GlSymbol*>(&glCompressedTexSubImage2D), "glCompressedTexSubImage2D"},
		{reinterpret_cast<GlSymbol*>(&glCompressedTexSubImage3D), "glCompressedTexSubImage3D"},
		{reinterpret_cast<GlSymbol*>(&glCompressedTextureSubImage1D), "glCompressedTextureSubImage1D"},
		{reinterpret_cast<GlSymbol*>(&glCompressedTextureSubImage2D), "glCompressedTextureSubImage2D"},
		{reinterpret_cast<GlSymbol*>(&glCompressedTextureSubImage3D), "glCompressedTextureSubImage3D"},
		{reinterpret_cast<GlSymbol*>(&glCopyBufferSubData), "glCopyBufferSubData"},
		{reinterpret_cast<GlSymbol*>(&glCopyNamedBufferSubData), "glCopyNamedBufferSubData"},
		{reinterpret_cast<GlSymbol*>(&glCopyPixels), "glCopyPixels"},
		{reinterpret_cast<GlSymbol*>(&glCopyTexImage1D), "glCopyTexImage1D"},
		{reinterpret_cast<GlSymbol*>(&glCopyTexImage2D), "glCopyTexImage2D"},
		{reinterpret_cast<GlSymbol*>(&glCopyTexSubImage1D), "glCopyTexSubImage1D"},
		{reinterpret_cast<GlSymbol*>(&glCopyTexSubImage2D), "glCopyTexSubImage2D"},
		{reinterpret_cast<GlSymbol*>(&glCopyTexSubImage3D), "glCopyTexSubImage3D"},
		{reinterpret_cast<GlSymbol*>(&glCopyTextureSubImage1D), "glCopyTextureSubImage1D"},
		{reinterpret_cast<GlSymbol*>(&glCopyTextureSubImage2D), "glCopyTextureSubImage2D"},
		{reinterpret_cast<GlSymbol*>(&glCopyTextureSubImage3D), "glCopyTextureSubImage3D"},
		{reinterpret_cast<GlSymbol*>(&glCreateBuffers), "glCreateBuffers"},
		{reinterpret_cast<GlSymbol*>(&glCreateFramebuffers), "glCreateFramebuffers"},
		{reinterpret_cast<GlSymbol*>(&glCreateProgramPipelines), "glCreateProgramPipelines"},
		{reinterpret_cast<GlSymbol*>(&glCreateProgram), "glCreateProgram"},
		{reinterpret_cast<GlSymbol*>(&glCreateQueries), "glCreateQueries"},
		{reinterpret_cast<GlSymbol*>(&glCreateRenderbuffers), "glCreateRenderbuffers"},
		{reinterpret_cast<GlSymbol*>(&glCreateSamplers), "glCreateSamplers"},
		{reinterpret_cast<GlSymbol*>(&glCreateShader), "glCreateShader"},
		{reinterpret_cast<GlSymbol*>(&glCreateShaderProgramv), "glCreateShaderProgramv"},
		{reinterpret_cast<GlSymbol*>(&glCreateTextures), "glCreateTextures"},
		{reinterpret_cast<GlSymbol*>(&glCreateTransformFeedbacks), "glCreateTransformFeedbacks"},
		{reinterpret_cast<GlSymbol*>(&glCreateVertexArrays), "glCreateVertexArrays"},
		{reinterpret_cast<GlSymbol*>(&glCullFace), "glCullFace"},
		{reinterpret_cast<GlSymbol*>(&glDeleteBuffersARB), "glDeleteBuffersARB"},
		{reinterpret_cast<GlSymbol*>(&glDeleteBuffers), "glDeleteBuffers"},
		{reinterpret_cast<GlSymbol*>(&glDeleteFramebuffersEXT), "glDeleteFramebuffersEXT"},
		{reinterpret_cast<GlSymbol*>(&glDeleteFramebuffers), "glDeleteFramebuffers"},
		{reinterpret_cast<GlSymbol*>(&glDeleteLists), "glDeleteLists"},
		{reinterpret_cast<GlSymbol*>(&glDeleteProgramPipelines), "glDeleteProgramPipelines"},
		{reinterpret_cast<GlSymbol*>(&glDeleteProgram), "glDeleteProgram"},
		{reinterpret_cast<GlSymbol*>(&glDeleteQueries), "glDeleteQueries"},
		{reinterpret_cast<GlSymbol*>(&glDeleteRenderbuffersEXT), "glDeleteRenderbuffersEXT"},
		{reinterpret_cast<GlSymbol*>(&glDeleteRenderbuffers), "glDeleteRenderbuffers"},
		{reinterpret_cast<GlSymbol*>(&glDeleteSamplers), "glDeleteSamplers"},
		{reinterpret_cast<GlSymbol*>(&glDeleteShader), "glDeleteShader"},
		{reinterpret_cast<GlSymbol*>(&glDeleteSync), "glDeleteSync"},
		{reinterpret_cast<GlSymbol*>(&glDeleteTextures), "glDeleteTextures"},
		{reinterpret_cast<GlSymbol*>(&glDeleteVertexArrays), "glDeleteVertexArrays"},
		{reinterpret_cast<GlSymbol*>(&glDepthFunc), "glDepthFunc"},
		{reinterpret_cast<GlSymbol*>(&glDepthMask), "glDepthMask"},
		{reinterpret_cast<GlSymbol*>(&glDepthRangef), "glDepthRangef"},
		{reinterpret_cast<GlSymbol*>(&glDepthRange), "glDepthRange"},
		{reinterpret_cast<GlSymbol*>(&glDetachShader), "glDetachShader"},
		{reinterpret_cast<GlSymbol*>(&glDisableClientState), "glDisableClientState"},
		{reinterpret_cast<GlSymbol*>(&glDisablei), "glDisablei"},
		{reinterpret_cast<GlSymbol*>(&glDisable), "glDisable"},
		{reinterpret_cast<GlSymbol*>(&glDisableVertexArrayAttrib), "glDisableVertexArrayAttrib"},
		{reinterpret_cast<GlSymbol*>(&glDisableVertexAttribArray), "glDisableVertexAttribArray"},
		{reinterpret_cast<GlSymbol*>(&glDrawArraysInstanced), "glDrawArraysInstanced"},
		{reinterpret_cast<GlSymbol*>(&glDrawArrays), "glDrawArrays"},
		{reinterpret_cast<GlSymbol*>(&glDrawBuffer), "glDrawBuffer"},
		{reinterpret_cast<GlSymbol*>(&glDrawBuffers), "glDrawBuffers"},
		{reinterpret_cast<GlSymbol*>(&glDrawElementsBaseVertex), "glDrawElementsBaseVertex"},
		{reinterpret_cast<GlSymbol*>(&glDrawElementsInstancedBaseVertex), "glDrawElementsInstancedBaseVertex"},
		{reinterpret_cast<GlSymbol*>(&glDrawElementsInstanced), "glDrawElementsInstanced"},
		{reinterpret_cast<GlSymbol*>(&glDrawElements), "glDrawElements"},
		{reinterpret_cast<GlSymbol*>(&glDrawPixels), "glDrawPixels"},
		{reinterpret_cast<GlSymbol*>(&glDrawRangeElementsBaseVertex), "glDrawRangeElementsBaseVertex"},
		{reinterpret_cast<GlSymbol*>(&glDrawRangeElements), "glDrawRangeElements"},
		{reinterpret_cast<GlSymbol*>(&glEdgeFlagPointer), "glEdgeFlagPointer"},
		{reinterpret_cast<GlSymbol*>(&glEdgeFlag), "glEdgeFlag"},
		{reinterpret_cast<GlSymbol*>(&glEdgeFlagv), "glEdgeFlagv"},
		{reinterpret_cast<GlSymbol*>(&glEnableClientState), "glEnableClientState"},
		{reinterpret_cast<GlSymbol*>(&glEnablei), "glEnablei"},
		{reinterpret_cast<GlSymbol*>(&glEnable), "glEnable"},
		{reinterpret_cast<GlSymbol*>(&glEnableVertexArrayAttrib), "glEnableVertexArrayAttrib"},
		{reinterpret_cast<GlSymbol*>(&glEnableVertexAttribArray), "glEnableVertexAttribArray"},
		{reinterpret_cast<GlSymbol*>(&glEndConditionalRender), "glEndConditionalRender"},
		{reinterpret_cast<GlSymbol*>(&glEndList), "glEndList"},
		{reinterpret_cast<GlSymbol*>(&glEnd), "glEnd"},
		{reinterpret_cast<GlSymbol*>(&glEndQuery), "glEndQuery"},
		{reinterpret_cast<GlSymbol*>(&glEndTransformFeedback), "glEndTransformFeedback"},
		{reinterpret_cast<GlSymbol*>(&glEvalCoord1d), "glEvalCoord1d"},
		{reinterpret_cast<GlSymbol*>(&glEvalCoord1dv), "glEvalCoord1dv"},
		{reinterpret_cast<GlSymbol*>(&glEvalCoord1f), "glEvalCoord1f"},
		{reinterpret_cast<GlSymbol*>(&glEvalCoord1fv), "glEvalCoord1fv"},
		{reinterpret_cast<GlSymbol*>(&glEvalCoord2d), "glEvalCoord2d"},
		{reinterpret_cast<GlSymbol*>(&glEvalCoord2dv), "glEvalCoord2dv"},
		{reinterpret_cast<GlSymbol*>(&glEvalCoord2f), "glEvalCoord2f"},
		{reinterpret_cast<GlSymbol*>(&glEvalCoord2fv), "glEvalCoord2fv"},
		{reinterpret_cast<GlSymbol*>(&glEvalMesh1), "glEvalMesh1"},
		{reinterpret_cast<GlSymbol*>(&glEvalMesh2), "glEvalMesh2"},
		{reinterpret_cast<GlSymbol*>(&glEvalPoint1), "glEvalPoint1"},
		{reinterpret_cast<GlSymbol*>(&glEvalPoint2), "glEvalPoint2"},
		{reinterpret_cast<GlSymbol*>(&glFeedbackBuffer), "glFeedbackBuffer"},
		{reinterpret_cast<GlSymbol*>(&glFenceSync), "glFenceSync"},
		{reinterpret_cast<GlSymbol*>(&glFinish), "glFinish"},
		{reinterpret_cast<GlSymbol*>(&glFlushMappedBufferRange), "glFlushMappedBufferRange"},
		{reinterpret_cast<GlSymbol*>(&glFlushMappedNamedBufferRange), "glFlushMappedNamedBufferRange"},
		{reinterpret_cast<GlSymbol*>(&glFlush), "glFlush"},
		{reinterpret_cast<GlSymbol*>(&glFogCoordd), "glFogCoordd"},
		{reinterpret_cast<GlSymbol*>(&glFogCoorddv), "glFogCoorddv"},
		{reinterpret_cast<GlSymbol*>(&glFogCoordf), "glFogCoordf"},
		{reinterpret_cast<GlSymbol*>(&glFogCoordfv), "glFogCoordfv"},
		{reinterpret_cast<GlSymbol*>(&glFogCoordPointer), "glFogCoordPointer"},
		{reinterpret_cast<GlSymbol*>(&glFogf), "glFogf"},
		{reinterpret_cast<GlSymbol*>(&glFogfv), "glFogfv"},
		{reinterpret_cast<GlSymbol*>(&glFogi), "glFogi"},
		{reinterpret_cast<GlSymbol*>(&glFogiv), "glFogiv"},
		{reinterpret_cast<GlSymbol*>(&glFramebufferRenderbufferEXT), "glFramebufferRenderbufferEXT"},
		{reinterpret_cast<GlSymbol*>(&glFramebufferRenderbuffer), "glFramebufferRenderbuffer"},
		{reinterpret_cast<GlSymbol*>(&glFramebufferTexture1DEXT), "glFramebufferTexture1DEXT"},
		{reinterpret_cast<GlSymbol*>(&glFramebufferTexture1D), "glFramebufferTexture1D"},
		{reinterpret_cast<GlSymbol*>(&glFramebufferTexture2DEXT), "glFramebufferTexture2DEXT"},
		{reinterpret_cast<GlSymbol*>(&glFramebufferTexture2D), "glFramebufferTexture2D"},
		{reinterpret_cast<GlSymbol*>(&glFramebufferTexture3DEXT), "glFramebufferTexture3DEXT"},
		{reinterpret_cast<GlSymbol*>(&glFramebufferTexture3D), "glFramebufferTexture3D"},
		{reinterpret_cast<GlSymbol*>(&glFramebufferTextureLayer), "glFramebufferTextureLayer"},
		{reinterpret_cast<GlSymbol*>(&glFramebufferTexture), "glFramebufferTexture"},
		{reinterpret_cast<GlSymbol*>(&glFrontFace), "glFrontFace"},
		{reinterpret_cast<GlSymbol*>(&glFrustum), "glFrustum"},
		{reinterpret_cast<GlSymbol*>(&glGenBuffersARB), "glGenBuffersARB"},
		{reinterpret_cast<GlSymbol*>(&glGenBuffers), "glGenBuffers"},
		{reinterpret_cast<GlSymbol*>(&glGenerateMipmapEXT), "glGenerateMipmapEXT"},
		{reinterpret_cast<GlSymbol*>(&glGenerateMipmap), "glGenerateMipmap"},
		{reinterpret_cast<GlSymbol*>(&glGenerateTextureMipmap), "glGenerateTextureMipmap"},
		{reinterpret_cast<GlSymbol*>(&glGenFramebuffersEXT), "glGenFramebuffersEXT"},
		{reinterpret_cast<GlSymbol*>(&glGenFramebuffers), "glGenFramebuffers"},
		{reinterpret_cast<GlSymbol*>(&glGenLists), "glGenLists"},
		{reinterpret_cast<GlSymbol*>(&glGenProgramPipelines), "glGenProgramPipelines"},
		{reinterpret_cast<GlSymbol*>(&glGenQueries), "glGenQueries"},
		{reinterpret_cast<GlSymbol*>(&glGenRenderbuffersEXT), "glGenRenderbuffersEXT"},
		{reinterpret_cast<GlSymbol*>(&glGenRenderbuffers), "glGenRenderbuffers"},
		{reinterpret_cast<GlSymbol*>(&glGenSamplers), "glGenSamplers"},
		{reinterpret_cast<GlSymbol*>(&glGenTextures), "glGenTextures"},
		{reinterpret_cast<GlSymbol*>(&glGenVertexArrays), "glGenVertexArrays"},
		{reinterpret_cast<GlSymbol*>(&glGetActiveAttrib), "glGetActiveAttrib"},
		{reinterpret_cast<GlSymbol*>(&glGetActiveUniformBlockiv), "glGetActiveUniformBlockiv"},
		{reinterpret_cast<GlSymbol*>(&glGetActiveUniformBlockName), "glGetActiveUniformBlockName"},
		{reinterpret_cast<GlSymbol*>(&glGetActiveUniformName), "glGetActiveUniformName"},
		{reinterpret_cast<GlSymbol*>(&glGetActiveUniform), "glGetActiveUniform"},
		{reinterpret_cast<GlSymbol*>(&glGetActiveUniformsiv), "glGetActiveUniformsiv"},
		{reinterpret_cast<GlSymbol*>(&glGetAttachedShaders), "glGetAttachedShaders"},
		{reinterpret_cast<GlSymbol*>(&glGetAttribLocation), "glGetAttribLocation"},
		{reinterpret_cast<GlSymbol*>(&glGetBooleani_v), "glGetBooleani_v"},
		{reinterpret_cast<GlSymbol*>(&glGetBooleanv), "glGetBooleanv"},
		{reinterpret_cast<GlSymbol*>(&glGetBufferParameteri64v), "glGetBufferParameteri64v"},
		{reinterpret_cast<GlSymbol*>(&glGetBufferParameterivARB), "glGetBufferParameterivARB"},
		{reinterpret_cast<GlSymbol*>(&glGetBufferParameteriv), "glGetBufferParameteriv"},
		{reinterpret_cast<GlSymbol*>(&glGetBufferPointervARB), "glGetBufferPointervARB"},
		{reinterpret_cast<GlSymbol*>(&glGetBufferPointerv), "glGetBufferPointerv"},
		{reinterpret_cast<GlSymbol*>(&glGetBufferSubDataARB), "glGetBufferSubDataARB"},
		{reinterpret_cast<GlSymbol*>(&glGetBufferSubData), "glGetBufferSubData"},
		{reinterpret_cast<GlSymbol*>(&glGetClipPlane), "glGetClipPlane"},
		{reinterpret_cast<GlSymbol*>(&glGetCompressedTexImage), "glGetCompressedTexImage"},
		{reinterpret_cast<GlSymbol*>(&glGetCompressedTextureImage), "glGetCompressedTextureImage"},
		{reinterpret_cast<GlSymbol*>(&glGetDoublev), "glGetDoublev"},
		{reinterpret_cast<GlSymbol*>(&glGetError), "glGetError"},
		{reinterpret_cast<GlSymbol*>(&glGetFloatv), "glGetFloatv"},
		{reinterpret_cast<GlSymbol*>(&glGetFragDataLocation), "glGetFragDataLocation"},
		{reinterpret_cast<GlSymbol*>(&glGetFramebufferAttachmentParameterivEXT), "glGetFramebufferAttachmentParameterivEXT"},
		{reinterpret_cast<GlSymbol*>(&glGetFramebufferAttachmentParameteriv), "glGetFramebufferAttachmentParameteriv"},
		{reinterpret_cast<GlSymbol*>(&glGetInteger64i_v), "glGetInteger64i_v"},
		{reinterpret_cast<GlSymbol*>(&glGetInteger64v), "glGetInteger64v"},
		{reinterpret_cast<GlSymbol*>(&glGetIntegeri_v), "glGetIntegeri_v"},
		{reinterpret_cast<GlSymbol*>(&glGetIntegerv), "glGetIntegerv"},
		{reinterpret_cast<GlSymbol*>(&glGetLightfv), "glGetLightfv"},
		{reinterpret_cast<GlSymbol*>(&glGetLightiv), "glGetLightiv"},
		{reinterpret_cast<GlSymbol*>(&glGetMapdv), "glGetMapdv"},
		{reinterpret_cast<GlSymbol*>(&glGetMapfv), "glGetMapfv"},
		{reinterpret_cast<GlSymbol*>(&glGetMapiv), "glGetMapiv"},
		{reinterpret_cast<GlSymbol*>(&glGetMaterialfv), "glGetMaterialfv"},
		{reinterpret_cast<GlSymbol*>(&glGetMaterialiv), "glGetMaterialiv"},
		{reinterpret_cast<GlSymbol*>(&glGetMultisamplefv), "glGetMultisamplefv"},
		{reinterpret_cast<GlSymbol*>(&glGetNamedBufferParameteri64v), "glGetNamedBufferParameteri64v"},
		{reinterpret_cast<GlSymbol*>(&glGetNamedBufferParameteriv), "glGetNamedBufferParameteriv"},
		{reinterpret_cast<GlSymbol*>(&glGetNamedBufferPointerv), "glGetNamedBufferPointerv"},
		{reinterpret_cast<GlSymbol*>(&glGetNamedBufferSubData), "glGetNamedBufferSubData"},
		{reinterpret_cast<GlSymbol*>(&glGetNamedFramebufferAttachmentParameteriv), "glGetNamedFramebufferAttachmentParameteriv"},
		{reinterpret_cast<GlSymbol*>(&glGetNamedFramebufferParameteriv), "glGetNamedFramebufferParameteriv"},
		{reinterpret_cast<GlSymbol*>(&glGetNamedRenderbufferParameteriv), "glGetNamedRenderbufferParameteriv"},
		{reinterpret_cast<GlSymbol*>(&glGetPixelMapfv), "glGetPixelMapfv"},
		{reinterpret_cast<GlSymbol*>(&glGetPixelMapuiv), "glGetPixelMapuiv"},
		{reinterpret_cast<GlSymbol*>(&glGetPixelMapusv), "glGetPixelMapusv"},
		{reinterpret_cast<GlSymbol*>(&glGetPointerv), "glGetPointerv"},
		{reinterpret_cast<GlSymbol*>(&glGetPolygonStipple), "glGetPolygonStipple"},
		{reinterpret_cast<GlSymbol*>(&glGetProgramInfoLog), "glGetProgramInfoLog"},
		{reinterpret_cast<GlSymbol*>(&glGetProgramiv), "glGetProgramiv"},
		{reinterpret_cast<GlSymbol*>(&glGetProgramPipelineInfoLog), "glGetProgramPipelineInfoLog"},
		{reinterpret_cast<GlSymbol*>(&glGetProgramPipelineiv), "glGetProgramPipelineiv"},
		{reinterpret_cast<GlSymbol*>(&glGetQueryBufferObjecti64v), "glGetQueryBufferObjecti64v"},
		{reinterpret_cast<GlSymbol*>(&glGetQueryBufferObjectiv), "glGetQueryBufferObjectiv"},
		{reinterpret_cast<GlSymbol*>(&glGetQueryBufferObjectui64v), "glGetQueryBufferObjectui64v"},
		{reinterpret_cast<GlSymbol*>(&glGetQueryBufferObjectuiv), "glGetQueryBufferObjectuiv"},
		{reinterpret_cast<GlSymbol*>(&glGetQueryiv), "glGetQueryiv"},
		{reinterpret_cast<GlSymbol*>(&glGetQueryObjectiv), "glGetQueryObjectiv"},
		{reinterpret_cast<GlSymbol*>(&glGetQueryObjectuiv), "glGetQueryObjectuiv"},
		{reinterpret_cast<GlSymbol*>(&glGetRenderbufferParameterivEXT), "glGetRenderbufferParameterivEXT"},
		{reinterpret_cast<GlSymbol*>(&glGetRenderbufferParameteriv), "glGetRenderbufferParameteriv"},
		{reinterpret_cast<GlSymbol*>(&glGetSamplerParameterfv), "glGetSamplerParameterfv"},
		{reinterpret_cast<GlSymbol*>(&glGetSamplerParameterIiv), "glGetSamplerParameterIiv"},
		{reinterpret_cast<GlSymbol*>(&glGetSamplerParameterIuiv), "glGetSamplerParameterIuiv"},
		{reinterpret_cast<GlSymbol*>(&glGetSamplerParameteriv), "glGetSamplerParameteriv"},
		{reinterpret_cast<GlSymbol*>(&glGetShaderInfoLog), "glGetShaderInfoLog"},
		{reinterpret_cast<GlSymbol*>(&glGetShaderiv), "glGetShaderiv"},
		{reinterpret_cast<GlSymbol*>(&glGetShaderPrecisionFormat), "glGetShaderPrecisionFormat"},
		{reinterpret_cast<GlSymbol*>(&glGetShaderSource), "glGetShaderSource"},
		{reinterpret_cast<GlSymbol*>(&glGetStringi), "glGetStringi"},
		{reinterpret_cast<GlSymbol*>(&glGetString), "glGetString"},
		{reinterpret_cast<GlSymbol*>(&glGetSynciv), "glGetSynciv"},
		{reinterpret_cast<GlSymbol*>(&glGetTexEnvfv), "glGetTexEnvfv"},
		{reinterpret_cast<GlSymbol*>(&glGetTexEnviv), "glGetTexEnviv"},
		{reinterpret_cast<GlSymbol*>(&glGetTexGendv), "glGetTexGendv"},
		{reinterpret_cast<GlSymbol*>(&glGetTexGenfv), "glGetTexGenfv"},
		{reinterpret_cast<GlSymbol*>(&glGetTexGeniv), "glGetTexGeniv"},
		{reinterpret_cast<GlSymbol*>(&glGetTexImage), "glGetTexImage"},
		{reinterpret_cast<GlSymbol*>(&glGetTexLevelParameterfv), "glGetTexLevelParameterfv"},
		{reinterpret_cast<GlSymbol*>(&glGetTexLevelParameteriv), "glGetTexLevelParameteriv"},
		{reinterpret_cast<GlSymbol*>(&glGetTexParameterfv), "glGetTexParameterfv"},
		{reinterpret_cast<GlSymbol*>(&glGetTexParameterIiv), "glGetTexParameterIiv"},
		{reinterpret_cast<GlSymbol*>(&glGetTexParameterIuiv), "glGetTexParameterIuiv"},
		{reinterpret_cast<GlSymbol*>(&glGetTexParameteriv), "glGetTexParameteriv"},
		{reinterpret_cast<GlSymbol*>(&glGetTextureImage), "glGetTextureImage"},
		{reinterpret_cast<GlSymbol*>(&glGetTextureLevelParameterfv), "glGetTextureLevelParameterfv"},
		{reinterpret_cast<GlSymbol*>(&glGetTextureLevelParameteriv), "glGetTextureLevelParameteriv"},
		{reinterpret_cast<GlSymbol*>(&glGetTextureParameterfv), "glGetTextureParameterfv"},
		{reinterpret_cast<GlSymbol*>(&glGetTextureParameterIiv), "glGetTextureParameterIiv"},
		{reinterpret_cast<GlSymbol*>(&glGetTextureParameterIuiv), "glGetTextureParameterIuiv"},
		{reinterpret_cast<GlSymbol*>(&glGetTextureParameteriv), "glGetTextureParameteriv"},
		{reinterpret_cast<GlSymbol*>(&glGetTransformFeedbacki64_v), "glGetTransformFeedbacki64_v"},
		{reinterpret_cast<GlSymbol*>(&glGetTransformFeedbackiv), "glGetTransformFeedbackiv"},
		{reinterpret_cast<GlSymbol*>(&glGetTransformFeedbacki_v), "glGetTransformFeedbacki_v"},
		{reinterpret_cast<GlSymbol*>(&glGetTransformFeedbackVarying), "glGetTransformFeedbackVarying"},
		{reinterpret_cast<GlSymbol*>(&glGetUniformBlockIndex), "glGetUniformBlockIndex"},
		{reinterpret_cast<GlSymbol*>(&glGetUniformfv), "glGetUniformfv"},
		{reinterpret_cast<GlSymbol*>(&glGetUniformIndices), "glGetUniformIndices"},
		{reinterpret_cast<GlSymbol*>(&glGetUniformiv), "glGetUniformiv"},
		{reinterpret_cast<GlSymbol*>(&glGetUniformLocation), "glGetUniformLocation"},
		{reinterpret_cast<GlSymbol*>(&glGetUniformuiv), "glGetUniformuiv"},
		{reinterpret_cast<GlSymbol*>(&glGetVertexArrayIndexed64iv), "glGetVertexArrayIndexed64iv"},
		{reinterpret_cast<GlSymbol*>(&glGetVertexArrayIndexediv), "glGetVertexArrayIndexediv"},
		{reinterpret_cast<GlSymbol*>(&glGetVertexArrayiv), "glGetVertexArrayiv"},
		{reinterpret_cast<GlSymbol*>(&glGetVertexAttribdv), "glGetVertexAttribdv"},
		{reinterpret_cast<GlSymbol*>(&glGetVertexAttribfv), "glGetVertexAttribfv"},
		{reinterpret_cast<GlSymbol*>(&glGetVertexAttribIiv), "glGetVertexAttribIiv"},
		{reinterpret_cast<GlSymbol*>(&glGetVertexAttribIuiv), "glGetVertexAttribIuiv"},
		{reinterpret_cast<GlSymbol*>(&glGetVertexAttribiv), "glGetVertexAttribiv"},
		{reinterpret_cast<GlSymbol*>(&glGetVertexAttribPointerv), "glGetVertexAttribPointerv"},
		{reinterpret_cast<GlSymbol*>(&glHint), "glHint"},
		{reinterpret_cast<GlSymbol*>(&glIndexd), "glIndexd"},
		{reinterpret_cast<GlSymbol*>(&glIndexdv), "glIndexdv"},
		{reinterpret_cast<GlSymbol*>(&glIndexf), "glIndexf"},
		{reinterpret_cast<GlSymbol*>(&glIndexfv), "glIndexfv"},
		{reinterpret_cast<GlSymbol*>(&glIndexi), "glIndexi"},
		{reinterpret_cast<GlSymbol*>(&glIndexiv), "glIndexiv"},
		{reinterpret_cast<GlSymbol*>(&glIndexMask), "glIndexMask"},
		{reinterpret_cast<GlSymbol*>(&glIndexPointer), "glIndexPointer"},
		{reinterpret_cast<GlSymbol*>(&glIndexs), "glIndexs"},
		{reinterpret_cast<GlSymbol*>(&glIndexsv), "glIndexsv"},
		{reinterpret_cast<GlSymbol*>(&glIndexub), "glIndexub"},
		{reinterpret_cast<GlSymbol*>(&glIndexubv), "glIndexubv"},
		{reinterpret_cast<GlSymbol*>(&glInitNames), "glInitNames"},
		{reinterpret_cast<GlSymbol*>(&glInterleavedArrays), "glInterleavedArrays"},
		{reinterpret_cast<GlSymbol*>(&glInvalidateNamedFramebufferData), "glInvalidateNamedFramebufferData"},
		{reinterpret_cast<GlSymbol*>(&glInvalidateNamedFramebufferSubData), "glInvalidateNamedFramebufferSubData"},
		{reinterpret_cast<GlSymbol*>(&glIsBufferARB), "glIsBufferARB"},
		{reinterpret_cast<GlSymbol*>(&glIsBuffer), "glIsBuffer"},
		{reinterpret_cast<GlSymbol*>(&glIsEnabledi), "glIsEnabledi"},
		{reinterpret_cast<GlSymbol*>(&glIsEnabled), "glIsEnabled"},
		{reinterpret_cast<GlSymbol*>(&glIsFramebufferEXT), "glIsFramebufferEXT"},
		{reinterpret_cast<GlSymbol*>(&glIsFramebuffer), "glIsFramebuffer"},
		{reinterpret_cast<GlSymbol*>(&glIsList), "glIsList"},
		{reinterpret_cast<GlSymbol*>(&glIsProgramPipeline), "glIsProgramPipeline"},
		{reinterpret_cast<GlSymbol*>(&glIsProgram), "glIsProgram"},
		{reinterpret_cast<GlSymbol*>(&glIsQuery), "glIsQuery"},
		{reinterpret_cast<GlSymbol*>(&glIsRenderbufferEXT), "glIsRenderbufferEXT"},
		{reinterpret_cast<GlSymbol*>(&glIsRenderbuffer), "glIsRenderbuffer"},
		{reinterpret_cast<GlSymbol*>(&glIsSampler), "glIsSampler"},
		{reinterpret_cast<GlSymbol*>(&glIsShader), "glIsShader"},
		{reinterpret_cast<GlSymbol*>(&glIsSync), "glIsSync"},
		{reinterpret_cast<GlSymbol*>(&glIsTexture), "glIsTexture"},
		{reinterpret_cast<GlSymbol*>(&glIsVertexArray), "glIsVertexArray"},
		{reinterpret_cast<GlSymbol*>(&glLightf), "glLightf"},
		{reinterpret_cast<GlSymbol*>(&glLightfv), "glLightfv"},
		{reinterpret_cast<GlSymbol*>(&glLighti), "glLighti"},
		{reinterpret_cast<GlSymbol*>(&glLightiv), "glLightiv"},
		{reinterpret_cast<GlSymbol*>(&glLightModelf), "glLightModelf"},
		{reinterpret_cast<GlSymbol*>(&glLightModelfv), "glLightModelfv"},
		{reinterpret_cast<GlSymbol*>(&glLightModeli), "glLightModeli"},
		{reinterpret_cast<GlSymbol*>(&glLightModeliv), "glLightModeliv"},
		{reinterpret_cast<GlSymbol*>(&glLineStipple), "glLineStipple"},
		{reinterpret_cast<GlSymbol*>(&glLineWidth), "glLineWidth"},
		{reinterpret_cast<GlSymbol*>(&glLinkProgram), "glLinkProgram"},
		{reinterpret_cast<GlSymbol*>(&glListBase), "glListBase"},
		{reinterpret_cast<GlSymbol*>(&glLoadIdentity), "glLoadIdentity"},
		{reinterpret_cast<GlSymbol*>(&glLoadMatrixd), "glLoadMatrixd"},
		{reinterpret_cast<GlSymbol*>(&glLoadMatrixf), "glLoadMatrixf"},
		{reinterpret_cast<GlSymbol*>(&glLoadName), "glLoadName"},
		{reinterpret_cast<GlSymbol*>(&glLoadTransposeMatrixd), "glLoadTransposeMatrixd"},
		{reinterpret_cast<GlSymbol*>(&glLoadTransposeMatrixf), "glLoadTransposeMatrixf"},
		{reinterpret_cast<GlSymbol*>(&glLogicOp), "glLogicOp"},
		{reinterpret_cast<GlSymbol*>(&glMap1d), "glMap1d"},
		{reinterpret_cast<GlSymbol*>(&glMap1f), "glMap1f"},
		{reinterpret_cast<GlSymbol*>(&glMap2d), "glMap2d"},
		{reinterpret_cast<GlSymbol*>(&glMap2f), "glMap2f"},
		{reinterpret_cast<GlSymbol*>(&glMapBufferARB), "glMapBufferARB"},
		{reinterpret_cast<GlSymbol*>(&glMapBuffer), "glMapBuffer"},
		{reinterpret_cast<GlSymbol*>(&glMapBufferRange), "glMapBufferRange"},
		{reinterpret_cast<GlSymbol*>(&glMapGrid1d), "glMapGrid1d"},
		{reinterpret_cast<GlSymbol*>(&glMapGrid1f), "glMapGrid1f"},
		{reinterpret_cast<GlSymbol*>(&glMapGrid2d), "glMapGrid2d"},
		{reinterpret_cast<GlSymbol*>(&glMapGrid2f), "glMapGrid2f"},
		{reinterpret_cast<GlSymbol*>(&glMapNamedBuffer), "glMapNamedBuffer"},
		{reinterpret_cast<GlSymbol*>(&glMapNamedBufferRange), "glMapNamedBufferRange"},
		{reinterpret_cast<GlSymbol*>(&glMaterialf), "glMaterialf"},
		{reinterpret_cast<GlSymbol*>(&glMaterialfv), "glMaterialfv"},
		{reinterpret_cast<GlSymbol*>(&glMateriali), "glMateriali"},
		{reinterpret_cast<GlSymbol*>(&glMaterialiv), "glMaterialiv"},
		{reinterpret_cast<GlSymbol*>(&glMatrixMode), "glMatrixMode"},
		{reinterpret_cast<GlSymbol*>(&glMultiDrawArrays), "glMultiDrawArrays"},
		{reinterpret_cast<GlSymbol*>(&glMultiDrawElementsBaseVertex), "glMultiDrawElementsBaseVertex"},
		{reinterpret_cast<GlSymbol*>(&glMultiDrawElements), "glMultiDrawElements"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord1d), "glMultiTexCoord1d"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord1dv), "glMultiTexCoord1dv"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord1f), "glMultiTexCoord1f"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord1fv), "glMultiTexCoord1fv"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord1i), "glMultiTexCoord1i"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord1iv), "glMultiTexCoord1iv"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord1s), "glMultiTexCoord1s"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord1sv), "glMultiTexCoord1sv"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord2d), "glMultiTexCoord2d"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord2dv), "glMultiTexCoord2dv"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord2f), "glMultiTexCoord2f"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord2fv), "glMultiTexCoord2fv"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord2i), "glMultiTexCoord2i"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord2iv), "glMultiTexCoord2iv"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord2s), "glMultiTexCoord2s"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord2sv), "glMultiTexCoord2sv"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord3d), "glMultiTexCoord3d"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord3dv), "glMultiTexCoord3dv"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord3f), "glMultiTexCoord3f"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord3fv), "glMultiTexCoord3fv"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord3i), "glMultiTexCoord3i"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord3iv), "glMultiTexCoord3iv"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord3s), "glMultiTexCoord3s"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord3sv), "glMultiTexCoord3sv"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord4d), "glMultiTexCoord4d"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord4dv), "glMultiTexCoord4dv"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord4f), "glMultiTexCoord4f"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord4fv), "glMultiTexCoord4fv"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord4i), "glMultiTexCoord4i"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord4iv), "glMultiTexCoord4iv"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord4s), "glMultiTexCoord4s"},
		{reinterpret_cast<GlSymbol*>(&glMultiTexCoord4sv), "glMultiTexCoord4sv"},
		{reinterpret_cast<GlSymbol*>(&glMultMatrixd), "glMultMatrixd"},
		{reinterpret_cast<GlSymbol*>(&glMultMatrixf), "glMultMatrixf"},
		{reinterpret_cast<GlSymbol*>(&glMultTransposeMatrixd), "glMultTransposeMatrixd"},
		{reinterpret_cast<GlSymbol*>(&glMultTransposeMatrixf), "glMultTransposeMatrixf"},
		{reinterpret_cast<GlSymbol*>(&glNamedBufferData), "glNamedBufferData"},
		{reinterpret_cast<GlSymbol*>(&glNamedBufferStorage), "glNamedBufferStorage"},
		{reinterpret_cast<GlSymbol*>(&glNamedBufferSubData), "glNamedBufferSubData"},
		{reinterpret_cast<GlSymbol*>(&glNamedFramebufferDrawBuffer), "glNamedFramebufferDrawBuffer"},
		{reinterpret_cast<GlSymbol*>(&glNamedFramebufferDrawBuffers), "glNamedFramebufferDrawBuffers"},
		{reinterpret_cast<GlSymbol*>(&glNamedFramebufferParameteri), "glNamedFramebufferParameteri"},
		{reinterpret_cast<GlSymbol*>(&glNamedFramebufferReadBuffer), "glNamedFramebufferReadBuffer"},
		{reinterpret_cast<GlSymbol*>(&glNamedFramebufferRenderbuffer), "glNamedFramebufferRenderbuffer"},
		{reinterpret_cast<GlSymbol*>(&glNamedFramebufferTextureLayer), "glNamedFramebufferTextureLayer"},
		{reinterpret_cast<GlSymbol*>(&glNamedFramebufferTexture), "glNamedFramebufferTexture"},
		{reinterpret_cast<GlSymbol*>(&glNamedRenderbufferStorageMultisample), "glNamedRenderbufferStorageMultisample"},
		{reinterpret_cast<GlSymbol*>(&glNamedRenderbufferStorage), "glNamedRenderbufferStorage"},
		{reinterpret_cast<GlSymbol*>(&glNewList), "glNewList"},
		{reinterpret_cast<GlSymbol*>(&glNormal3b), "glNormal3b"},
		{reinterpret_cast<GlSymbol*>(&glNormal3bv), "glNormal3bv"},
		{reinterpret_cast<GlSymbol*>(&glNormal3d), "glNormal3d"},
		{reinterpret_cast<GlSymbol*>(&glNormal3dv), "glNormal3dv"},
		{reinterpret_cast<GlSymbol*>(&glNormal3f), "glNormal3f"},
		{reinterpret_cast<GlSymbol*>(&glNormal3fv), "glNormal3fv"},
		{reinterpret_cast<GlSymbol*>(&glNormal3i), "glNormal3i"},
		{reinterpret_cast<GlSymbol*>(&glNormal3iv), "glNormal3iv"},
		{reinterpret_cast<GlSymbol*>(&glNormal3s), "glNormal3s"},
		{reinterpret_cast<GlSymbol*>(&glNormal3sv), "glNormal3sv"},
		{reinterpret_cast<GlSymbol*>(&glNormalPointer), "glNormalPointer"},
		{reinterpret_cast<GlSymbol*>(&glOrtho), "glOrtho"},
		{reinterpret_cast<GlSymbol*>(&glPassThrough), "glPassThrough"},
		{reinterpret_cast<GlSymbol*>(&glPixelMapfv), "glPixelMapfv"},
		{reinterpret_cast<GlSymbol*>(&glPixelMapuiv), "glPixelMapuiv"},
		{reinterpret_cast<GlSymbol*>(&glPixelMapusv), "glPixelMapusv"},
		{reinterpret_cast<GlSymbol*>(&glPixelStoref), "glPixelStoref"},
		{reinterpret_cast<GlSymbol*>(&glPixelStorei), "glPixelStorei"},
		{reinterpret_cast<GlSymbol*>(&glPixelTransferf), "glPixelTransferf"},
		{reinterpret_cast<GlSymbol*>(&glPixelTransferi), "glPixelTransferi"},
		{reinterpret_cast<GlSymbol*>(&glPixelZoom), "glPixelZoom"},
		{reinterpret_cast<GlSymbol*>(&glPointParameterf), "glPointParameterf"},
		{reinterpret_cast<GlSymbol*>(&glPointParameterfv), "glPointParameterfv"},
		{reinterpret_cast<GlSymbol*>(&glPointParameteri), "glPointParameteri"},
		{reinterpret_cast<GlSymbol*>(&glPointParameteriv), "glPointParameteriv"},
		{reinterpret_cast<GlSymbol*>(&glPointSize), "glPointSize"},
		{reinterpret_cast<GlSymbol*>(&glPolygonMode), "glPolygonMode"},
		{reinterpret_cast<GlSymbol*>(&glPolygonOffset), "glPolygonOffset"},
		{reinterpret_cast<GlSymbol*>(&glPolygonStipple), "glPolygonStipple"},
		{reinterpret_cast<GlSymbol*>(&glPopAttrib), "glPopAttrib"},
		{reinterpret_cast<GlSymbol*>(&glPopClientAttrib), "glPopClientAttrib"},
		{reinterpret_cast<GlSymbol*>(&glPopMatrix), "glPopMatrix"},
		{reinterpret_cast<GlSymbol*>(&glPopName), "glPopName"},
		{reinterpret_cast<GlSymbol*>(&glPrimitiveRestartIndex), "glPrimitiveRestartIndex"},
		{reinterpret_cast<GlSymbol*>(&glPrioritizeTextures), "glPrioritizeTextures"},
		{reinterpret_cast<GlSymbol*>(&glProgramParameteri), "glProgramParameteri"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform1d), "glProgramUniform1d"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform1dv), "glProgramUniform1dv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform1f), "glProgramUniform1f"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform1fv), "glProgramUniform1fv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform1i), "glProgramUniform1i"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform1iv), "glProgramUniform1iv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform1ui), "glProgramUniform1ui"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform1uiv), "glProgramUniform1uiv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform2d), "glProgramUniform2d"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform2dv), "glProgramUniform2dv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform2f), "glProgramUniform2f"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform2fv), "glProgramUniform2fv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform2i), "glProgramUniform2i"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform2iv), "glProgramUniform2iv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform2ui), "glProgramUniform2ui"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform2uiv), "glProgramUniform2uiv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform3d), "glProgramUniform3d"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform3dv), "glProgramUniform3dv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform3f), "glProgramUniform3f"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform3fv), "glProgramUniform3fv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform3i), "glProgramUniform3i"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform3iv), "glProgramUniform3iv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform3ui), "glProgramUniform3ui"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform3uiv), "glProgramUniform3uiv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform4d), "glProgramUniform4d"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform4dv), "glProgramUniform4dv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform4f), "glProgramUniform4f"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform4fv), "glProgramUniform4fv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform4i), "glProgramUniform4i"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform4iv), "glProgramUniform4iv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform4ui), "glProgramUniform4ui"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniform4uiv), "glProgramUniform4uiv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix2dv), "glProgramUniformMatrix2dv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix2fv), "glProgramUniformMatrix2fv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix2x3dv), "glProgramUniformMatrix2x3dv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix2x3fv), "glProgramUniformMatrix2x3fv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix2x4dv), "glProgramUniformMatrix2x4dv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix2x4fv), "glProgramUniformMatrix2x4fv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix3dv), "glProgramUniformMatrix3dv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix3fv), "glProgramUniformMatrix3fv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix3x2dv), "glProgramUniformMatrix3x2dv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix3x2fv), "glProgramUniformMatrix3x2fv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix3x4dv), "glProgramUniformMatrix3x4dv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix3x4fv), "glProgramUniformMatrix3x4fv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix4dv), "glProgramUniformMatrix4dv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix4fv), "glProgramUniformMatrix4fv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix4x2dv), "glProgramUniformMatrix4x2dv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix4x2fv), "glProgramUniformMatrix4x2fv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix4x3dv), "glProgramUniformMatrix4x3dv"},
		{reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix4x3fv), "glProgramUniformMatrix4x3fv"},
		{reinterpret_cast<GlSymbol*>(&glProvokingVertex), "glProvokingVertex"},
		{reinterpret_cast<GlSymbol*>(&glPushAttrib), "glPushAttrib"},
		{reinterpret_cast<GlSymbol*>(&glPushClientAttrib), "glPushClientAttrib"},
		{reinterpret_cast<GlSymbol*>(&glPushMatrix), "glPushMatrix"},
		{reinterpret_cast<GlSymbol*>(&glPushName), "glPushName"},
		{reinterpret_cast<GlSymbol*>(&glRasterPos2d), "glRasterPos2d"},
		{reinterpret_cast<GlSymbol*>(&glRasterPos2dv), "glRasterPos2dv"},
		{reinterpret_cast<GlSymbol*>(&glRasterPos2f), "glRasterPos2f"},
		{reinterpret_cast<GlSymbol*>(&glRasterPos2fv), "glRasterPos2fv"},
		{reinterpret_cast<GlSymbol*>(&glRasterPos2i), "glRasterPos2i"},
		{reinterpret_cast<GlSymbol*>(&glRasterPos2iv), "glRasterPos2iv"},
		{reinterpret_cast<GlSymbol*>(&glRasterPos2s), "glRasterPos2s"},
		{reinterpret_cast<GlSymbol*>(&glRasterPos2sv), "glRasterPos2sv"},
		{reinterpret_cast<GlSymbol*>(&glRasterPos3d), "glRasterPos3d"},
		{reinterpret_cast<GlSymbol*>(&glRasterPos3dv), "glRasterPos3dv"},
		{reinterpret_cast<GlSymbol*>(&glRasterPos3f), "glRasterPos3f"},
		{reinterpret_cast<GlSymbol*>(&glRasterPos3fv), "glRasterPos3fv"},
		{reinterpret_cast<GlSymbol*>(&glRasterPos3i), "glRasterPos3i"},
		{reinterpret_cast<GlSymbol*>(&glRasterPos3iv), "glRasterPos3iv"},
		{reinterpret_cast<GlSymbol*>(&glRasterPos3s), "glRasterPos3s"},
		{reinterpret_cast<GlSymbol*>(&glRasterPos3sv), "glRasterPos3sv"},
		{reinterpret_cast<GlSymbol*>(&glRasterPos4d), "glRasterPos4d"},
		{reinterpret_cast<GlSymbol*>(&glRasterPos4dv), "glRasterPos4dv"},
		{reinterpret_cast<GlSymbol*>(&glRasterPos4f), "glRasterPos4f"},
		{reinterpret_cast<GlSymbol*>(&glRasterPos4fv), "glRasterPos4fv"},
		{reinterpret_cast<GlSymbol*>(&glRasterPos4i), "glRasterPos4i"},
		{reinterpret_cast<GlSymbol*>(&glRasterPos4iv), "glRasterPos4iv"},
		{reinterpret_cast<GlSymbol*>(&glRasterPos4s), "glRasterPos4s"},
		{reinterpret_cast<GlSymbol*>(&glRasterPos4sv), "glRasterPos4sv"},
		{reinterpret_cast<GlSymbol*>(&glReadBuffer), "glReadBuffer"},
		{reinterpret_cast<GlSymbol*>(&glReadPixels), "glReadPixels"},
		{reinterpret_cast<GlSymbol*>(&glRectd), "glRectd"},
		{reinterpret_cast<GlSymbol*>(&glRectdv), "glRectdv"},
		{reinterpret_cast<GlSymbol*>(&glRectf), "glRectf"},
		{reinterpret_cast<GlSymbol*>(&glRectfv), "glRectfv"},
		{reinterpret_cast<GlSymbol*>(&glRecti), "glRecti"},
		{reinterpret_cast<GlSymbol*>(&glRectiv), "glRectiv"},
		{reinterpret_cast<GlSymbol*>(&glRects), "glRects"},
		{reinterpret_cast<GlSymbol*>(&glRectsv), "glRectsv"},
		{reinterpret_cast<GlSymbol*>(&glReleaseShaderCompiler), "glReleaseShaderCompiler"},
		{reinterpret_cast<GlSymbol*>(&glRenderbufferStorageEXT), "glRenderbufferStorageEXT"},
		{reinterpret_cast<GlSymbol*>(&glRenderbufferStorageMultisampleEXT), "glRenderbufferStorageMultisampleEXT"},
		{reinterpret_cast<GlSymbol*>(&glRenderbufferStorageMultisample), "glRenderbufferStorageMultisample"},
		{reinterpret_cast<GlSymbol*>(&glRenderbufferStorage), "glRenderbufferStorage"},
		{reinterpret_cast<GlSymbol*>(&glRenderMode), "glRenderMode"},
		{reinterpret_cast<GlSymbol*>(&glRotated), "glRotated"},
		{reinterpret_cast<GlSymbol*>(&glRotatef), "glRotatef"},
		{reinterpret_cast<GlSymbol*>(&glSampleCoverage), "glSampleCoverage"},
		{reinterpret_cast<GlSymbol*>(&glSampleMaski), "glSampleMaski"},
		{reinterpret_cast<GlSymbol*>(&glSamplerParameterf), "glSamplerParameterf"},
		{reinterpret_cast<GlSymbol*>(&glSamplerParameterfv), "glSamplerParameterfv"},
		{reinterpret_cast<GlSymbol*>(&glSamplerParameterIiv), "glSamplerParameterIiv"},
		{reinterpret_cast<GlSymbol*>(&glSamplerParameteri), "glSamplerParameteri"},
		{reinterpret_cast<GlSymbol*>(&glSamplerParameterIuiv), "glSamplerParameterIuiv"},
		{reinterpret_cast<GlSymbol*>(&glSamplerParameteriv), "glSamplerParameteriv"},
		{reinterpret_cast<GlSymbol*>(&glScaled), "glScaled"},
		{reinterpret_cast<GlSymbol*>(&glScalef), "glScalef"},
		{reinterpret_cast<GlSymbol*>(&glScissor), "glScissor"},
		{reinterpret_cast<GlSymbol*>(&glSecondaryColor3b), "glSecondaryColor3b"},
		{reinterpret_cast<GlSymbol*>(&glSecondaryColor3bv), "glSecondaryColor3bv"},
		{reinterpret_cast<GlSymbol*>(&glSecondaryColor3d), "glSecondaryColor3d"},
		{reinterpret_cast<GlSymbol*>(&glSecondaryColor3dv), "glSecondaryColor3dv"},
		{reinterpret_cast<GlSymbol*>(&glSecondaryColor3f), "glSecondaryColor3f"},
		{reinterpret_cast<GlSymbol*>(&glSecondaryColor3fv), "glSecondaryColor3fv"},
		{reinterpret_cast<GlSymbol*>(&glSecondaryColor3i), "glSecondaryColor3i"},
		{reinterpret_cast<GlSymbol*>(&glSecondaryColor3iv), "glSecondaryColor3iv"},
		{reinterpret_cast<GlSymbol*>(&glSecondaryColor3s), "glSecondaryColor3s"},
		{reinterpret_cast<GlSymbol*>(&glSecondaryColor3sv), "glSecondaryColor3sv"},
		{reinterpret_cast<GlSymbol*>(&glSecondaryColor3ub), "glSecondaryColor3ub"},
		{reinterpret_cast<GlSymbol*>(&glSecondaryColor3ubv), "glSecondaryColor3ubv"},
		{reinterpret_cast<GlSymbol*>(&glSecondaryColor3ui), "glSecondaryColor3ui"},
		{reinterpret_cast<GlSymbol*>(&glSecondaryColor3uiv), "glSecondaryColor3uiv"},
		{reinterpret_cast<GlSymbol*>(&glSecondaryColor3us), "glSecondaryColor3us"},
		{reinterpret_cast<GlSymbol*>(&glSecondaryColor3usv), "glSecondaryColor3usv"},
		{reinterpret_cast<GlSymbol*>(&glSecondaryColorPointer), "glSecondaryColorPointer"},
		{reinterpret_cast<GlSymbol*>(&glSelectBuffer), "glSelectBuffer"},
		{reinterpret_cast<GlSymbol*>(&glShadeModel), "glShadeModel"},
		{reinterpret_cast<GlSymbol*>(&glShaderBinary), "glShaderBinary"},
		{reinterpret_cast<GlSymbol*>(&glShaderSource), "glShaderSource"},
		{reinterpret_cast<GlSymbol*>(&glStencilFunc), "glStencilFunc"},
		{reinterpret_cast<GlSymbol*>(&glStencilFuncSeparate), "glStencilFuncSeparate"},
		{reinterpret_cast<GlSymbol*>(&glStencilMask), "glStencilMask"},
		{reinterpret_cast<GlSymbol*>(&glStencilMaskSeparate), "glStencilMaskSeparate"},
		{reinterpret_cast<GlSymbol*>(&glStencilOp), "glStencilOp"},
		{reinterpret_cast<GlSymbol*>(&glStencilOpSeparate), "glStencilOpSeparate"},
		{reinterpret_cast<GlSymbol*>(&glTexBuffer), "glTexBuffer"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord1d), "glTexCoord1d"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord1dv), "glTexCoord1dv"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord1f), "glTexCoord1f"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord1fv), "glTexCoord1fv"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord1i), "glTexCoord1i"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord1iv), "glTexCoord1iv"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord1s), "glTexCoord1s"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord1sv), "glTexCoord1sv"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord2d), "glTexCoord2d"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord2dv), "glTexCoord2dv"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord2f), "glTexCoord2f"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord2fv), "glTexCoord2fv"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord2i), "glTexCoord2i"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord2iv), "glTexCoord2iv"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord2s), "glTexCoord2s"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord2sv), "glTexCoord2sv"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord3d), "glTexCoord3d"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord3dv), "glTexCoord3dv"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord3f), "glTexCoord3f"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord3fv), "glTexCoord3fv"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord3i), "glTexCoord3i"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord3iv), "glTexCoord3iv"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord3s), "glTexCoord3s"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord3sv), "glTexCoord3sv"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord4d), "glTexCoord4d"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord4dv), "glTexCoord4dv"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord4f), "glTexCoord4f"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord4fv), "glTexCoord4fv"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord4i), "glTexCoord4i"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord4iv), "glTexCoord4iv"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord4s), "glTexCoord4s"},
		{reinterpret_cast<GlSymbol*>(&glTexCoord4sv), "glTexCoord4sv"},
		{reinterpret_cast<GlSymbol*>(&glTexCoordPointer), "glTexCoordPointer"},
		{reinterpret_cast<GlSymbol*>(&glTexEnvf), "glTexEnvf"},
		{reinterpret_cast<GlSymbol*>(&glTexEnvfv), "glTexEnvfv"},
		{reinterpret_cast<GlSymbol*>(&glTexEnvi), "glTexEnvi"},
		{reinterpret_cast<GlSymbol*>(&glTexEnviv), "glTexEnviv"},
		{reinterpret_cast<GlSymbol*>(&glTexGend), "glTexGend"},
		{reinterpret_cast<GlSymbol*>(&glTexGendv), "glTexGendv"},
		{reinterpret_cast<GlSymbol*>(&glTexGenf), "glTexGenf"},
		{reinterpret_cast<GlSymbol*>(&glTexGenfv), "glTexGenfv"},
		{reinterpret_cast<GlSymbol*>(&glTexGeni), "glTexGeni"},
		{reinterpret_cast<GlSymbol*>(&glTexGeniv), "glTexGeniv"},
		{reinterpret_cast<GlSymbol*>(&glTexImage1D), "glTexImage1D"},
		{reinterpret_cast<GlSymbol*>(&glTexImage2DMultisample), "glTexImage2DMultisample"},
		{reinterpret_cast<GlSymbol*>(&glTexImage2D), "glTexImage2D"},
		{reinterpret_cast<GlSymbol*>(&glTexImage3DMultisample), "glTexImage3DMultisample"},
		{reinterpret_cast<GlSymbol*>(&glTexImage3D), "glTexImage3D"},
		{reinterpret_cast<GlSymbol*>(&glTexParameterf), "glTexParameterf"},
		{reinterpret_cast<GlSymbol*>(&glTexParameterfv), "glTexParameterfv"},
		{reinterpret_cast<GlSymbol*>(&glTexParameterIiv), "glTexParameterIiv"},
		{reinterpret_cast<GlSymbol*>(&glTexParameteri), "glTexParameteri"},
		{reinterpret_cast<GlSymbol*>(&glTexParameterIuiv), "glTexParameterIuiv"},
		{reinterpret_cast<GlSymbol*>(&glTexParameteriv), "glTexParameteriv"},
		{reinterpret_cast<GlSymbol*>(&glTexSubImage1D), "glTexSubImage1D"},
		{reinterpret_cast<GlSymbol*>(&glTexSubImage2D), "glTexSubImage2D"},
		{reinterpret_cast<GlSymbol*>(&glTexSubImage3D), "glTexSubImage3D"},
		{reinterpret_cast<GlSymbol*>(&glTextureBuffer), "glTextureBuffer"},
		{reinterpret_cast<GlSymbol*>(&glTextureBufferRange), "glTextureBufferRange"},
		{reinterpret_cast<GlSymbol*>(&glTextureParameterf), "glTextureParameterf"},
		{reinterpret_cast<GlSymbol*>(&glTextureParameterfv), "glTextureParameterfv"},
		{reinterpret_cast<GlSymbol*>(&glTextureParameterIiv), "glTextureParameterIiv"},
		{reinterpret_cast<GlSymbol*>(&glTextureParameteri), "glTextureParameteri"},
		{reinterpret_cast<GlSymbol*>(&glTextureParameterIuiv), "glTextureParameterIuiv"},
		{reinterpret_cast<GlSymbol*>(&glTextureParameteriv), "glTextureParameteriv"},
		{reinterpret_cast<GlSymbol*>(&glTextureStorage1D), "glTextureStorage1D"},
		{reinterpret_cast<GlSymbol*>(&glTextureStorage2DMultisample), "glTextureStorage2DMultisample"},
		{reinterpret_cast<GlSymbol*>(&glTextureStorage2D), "glTextureStorage2D"},
		{reinterpret_cast<GlSymbol*>(&glTextureStorage3DMultisample), "glTextureStorage3DMultisample"},
		{reinterpret_cast<GlSymbol*>(&glTextureStorage3D), "glTextureStorage3D"},
		{reinterpret_cast<GlSymbol*>(&glTextureSubImage1D), "glTextureSubImage1D"},
		{reinterpret_cast<GlSymbol*>(&glTextureSubImage2D), "glTextureSubImage2D"},
		{reinterpret_cast<GlSymbol*>(&glTextureSubImage3D), "glTextureSubImage3D"},
		{reinterpret_cast<GlSymbol*>(&glTransformFeedbackBufferBase), "glTransformFeedbackBufferBase"},
		{reinterpret_cast<GlSymbol*>(&glTransformFeedbackBufferRange), "glTransformFeedbackBufferRange"},
		{reinterpret_cast<GlSymbol*>(&glTransformFeedbackVaryings), "glTransformFeedbackVaryings"},
		{reinterpret_cast<GlSymbol*>(&glTranslated), "glTranslated"},
		{reinterpret_cast<GlSymbol*>(&glTranslatef), "glTranslatef"},
		{reinterpret_cast<GlSymbol*>(&glUniform1f), "glUniform1f"},
		{reinterpret_cast<GlSymbol*>(&glUniform1fv), "glUniform1fv"},
		{reinterpret_cast<GlSymbol*>(&glUniform1i), "glUniform1i"},
		{reinterpret_cast<GlSymbol*>(&glUniform1iv), "glUniform1iv"},
		{reinterpret_cast<GlSymbol*>(&glUniform1ui), "glUniform1ui"},
		{reinterpret_cast<GlSymbol*>(&glUniform1uiv), "glUniform1uiv"},
		{reinterpret_cast<GlSymbol*>(&glUniform2f), "glUniform2f"},
		{reinterpret_cast<GlSymbol*>(&glUniform2fv), "glUniform2fv"},
		{reinterpret_cast<GlSymbol*>(&glUniform2i), "glUniform2i"},
		{reinterpret_cast<GlSymbol*>(&glUniform2iv), "glUniform2iv"},
		{reinterpret_cast<GlSymbol*>(&glUniform2ui), "glUniform2ui"},
		{reinterpret_cast<GlSymbol*>(&glUniform2uiv), "glUniform2uiv"},
		{reinterpret_cast<GlSymbol*>(&glUniform3f), "glUniform3f"},
		{reinterpret_cast<GlSymbol*>(&glUniform3fv), "glUniform3fv"},
		{reinterpret_cast<GlSymbol*>(&glUniform3i), "glUniform3i"},
		{reinterpret_cast<GlSymbol*>(&glUniform3iv), "glUniform3iv"},
		{reinterpret_cast<GlSymbol*>(&glUniform3ui), "glUniform3ui"},
		{reinterpret_cast<GlSymbol*>(&glUniform3uiv), "glUniform3uiv"},
		{reinterpret_cast<GlSymbol*>(&glUniform4f), "glUniform4f"},
		{reinterpret_cast<GlSymbol*>(&glUniform4fv), "glUniform4fv"},
		{reinterpret_cast<GlSymbol*>(&glUniform4i), "glUniform4i"},
		{reinterpret_cast<GlSymbol*>(&glUniform4iv), "glUniform4iv"},
		{reinterpret_cast<GlSymbol*>(&glUniform4ui), "glUniform4ui"},
		{reinterpret_cast<GlSymbol*>(&glUniform4uiv), "glUniform4uiv"},
		{reinterpret_cast<GlSymbol*>(&glUniformBlockBinding), "glUniformBlockBinding"},
		{reinterpret_cast<GlSymbol*>(&glUniformMatrix2fv), "glUniformMatrix2fv"},
		{reinterpret_cast<GlSymbol*>(&glUniformMatrix2x3fv), "glUniformMatrix2x3fv"},
		{reinterpret_cast<GlSymbol*>(&glUniformMatrix2x4fv), "glUniformMatrix2x4fv"},
		{reinterpret_cast<GlSymbol*>(&glUniformMatrix3fv), "glUniformMatrix3fv"},
		{reinterpret_cast<GlSymbol*>(&glUniformMatrix3x2fv), "glUniformMatrix3x2fv"},
		{reinterpret_cast<GlSymbol*>(&glUniformMatrix3x4fv), "glUniformMatrix3x4fv"},
		{reinterpret_cast<GlSymbol*>(&glUniformMatrix4fv), "glUniformMatrix4fv"},
		{reinterpret_cast<GlSymbol*>(&glUniformMatrix4x2fv), "glUniformMatrix4x2fv"},
		{reinterpret_cast<GlSymbol*>(&glUniformMatrix4x3fv), "glUniformMatrix4x3fv"},
		{reinterpret_cast<GlSymbol*>(&glUnmapBufferARB), "glUnmapBufferARB"},
		{reinterpret_cast<GlSymbol*>(&glUnmapBuffer), "glUnmapBuffer"},
		{reinterpret_cast<GlSymbol*>(&glUnmapNamedBuffer), "glUnmapNamedBuffer"},
		{reinterpret_cast<GlSymbol*>(&glUseProgram), "glUseProgram"},
		{reinterpret_cast<GlSymbol*>(&glUseProgramStages), "glUseProgramStages"},
		{reinterpret_cast<GlSymbol*>(&glValidateProgramPipeline), "glValidateProgramPipeline"},
		{reinterpret_cast<GlSymbol*>(&glValidateProgram), "glValidateProgram"},
		{reinterpret_cast<GlSymbol*>(&glVertex2d), "glVertex2d"},
		{reinterpret_cast<GlSymbol*>(&glVertex2dv), "glVertex2dv"},
		{reinterpret_cast<GlSymbol*>(&glVertex2f), "glVertex2f"},
		{reinterpret_cast<GlSymbol*>(&glVertex2fv), "glVertex2fv"},
		{reinterpret_cast<GlSymbol*>(&glVertex2i), "glVertex2i"},
		{reinterpret_cast<GlSymbol*>(&glVertex2iv), "glVertex2iv"},
		{reinterpret_cast<GlSymbol*>(&glVertex2s), "glVertex2s"},
		{reinterpret_cast<GlSymbol*>(&glVertex2sv), "glVertex2sv"},
		{reinterpret_cast<GlSymbol*>(&glVertex3d), "glVertex3d"},
		{reinterpret_cast<GlSymbol*>(&glVertex3dv), "glVertex3dv"},
		{reinterpret_cast<GlSymbol*>(&glVertex3f), "glVertex3f"},
		{reinterpret_cast<GlSymbol*>(&glVertex3fv), "glVertex3fv"},
		{reinterpret_cast<GlSymbol*>(&glVertex3i), "glVertex3i"},
		{reinterpret_cast<GlSymbol*>(&glVertex3iv), "glVertex3iv"},
		{reinterpret_cast<GlSymbol*>(&glVertex3s), "glVertex3s"},
		{reinterpret_cast<GlSymbol*>(&glVertex3sv), "glVertex3sv"},
		{reinterpret_cast<GlSymbol*>(&glVertex4d), "glVertex4d"},
		{reinterpret_cast<GlSymbol*>(&glVertex4dv), "glVertex4dv"},
		{reinterpret_cast<GlSymbol*>(&glVertex4f), "glVertex4f"},
		{reinterpret_cast<GlSymbol*>(&glVertex4fv), "glVertex4fv"},
		{reinterpret_cast<GlSymbol*>(&glVertex4i), "glVertex4i"},
		{reinterpret_cast<GlSymbol*>(&glVertex4iv), "glVertex4iv"},
		{reinterpret_cast<GlSymbol*>(&glVertex4s), "glVertex4s"},
		{reinterpret_cast<GlSymbol*>(&glVertex4sv), "glVertex4sv"},
		{reinterpret_cast<GlSymbol*>(&glVertexArrayAttribBinding), "glVertexArrayAttribBinding"},
		{reinterpret_cast<GlSymbol*>(&glVertexArrayAttribFormat), "glVertexArrayAttribFormat"},
		{reinterpret_cast<GlSymbol*>(&glVertexArrayAttribIFormat), "glVertexArrayAttribIFormat"},
		{reinterpret_cast<GlSymbol*>(&glVertexArrayAttribLFormat), "glVertexArrayAttribLFormat"},
		{reinterpret_cast<GlSymbol*>(&glVertexArrayBindingDivisor), "glVertexArrayBindingDivisor"},
		{reinterpret_cast<GlSymbol*>(&glVertexArrayElementBuffer), "glVertexArrayElementBuffer"},
		{reinterpret_cast<GlSymbol*>(&glVertexArrayVertexBuffer), "glVertexArrayVertexBuffer"},
		{reinterpret_cast<GlSymbol*>(&glVertexArrayVertexBuffers), "glVertexArrayVertexBuffers"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib1d), "glVertexAttrib1d"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib1dv), "glVertexAttrib1dv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib1f), "glVertexAttrib1f"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib1fv), "glVertexAttrib1fv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib1s), "glVertexAttrib1s"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib1sv), "glVertexAttrib1sv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib2d), "glVertexAttrib2d"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib2dv), "glVertexAttrib2dv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib2f), "glVertexAttrib2f"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib2fv), "glVertexAttrib2fv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib2s), "glVertexAttrib2s"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib2sv), "glVertexAttrib2sv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib3d), "glVertexAttrib3d"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib3dv), "glVertexAttrib3dv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib3f), "glVertexAttrib3f"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib3fv), "glVertexAttrib3fv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib3s), "glVertexAttrib3s"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib3sv), "glVertexAttrib3sv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib4bv), "glVertexAttrib4bv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib4d), "glVertexAttrib4d"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib4dv), "glVertexAttrib4dv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib4f), "glVertexAttrib4f"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib4fv), "glVertexAttrib4fv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib4iv), "glVertexAttrib4iv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib4Nbv), "glVertexAttrib4Nbv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib4Niv), "glVertexAttrib4Niv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib4Nsv), "glVertexAttrib4Nsv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib4Nub), "glVertexAttrib4Nub"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib4Nubv), "glVertexAttrib4Nubv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib4Nuiv), "glVertexAttrib4Nuiv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib4Nusv), "glVertexAttrib4Nusv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib4s), "glVertexAttrib4s"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib4sv), "glVertexAttrib4sv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib4ubv), "glVertexAttrib4ubv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib4uiv), "glVertexAttrib4uiv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttrib4usv), "glVertexAttrib4usv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttribI1i), "glVertexAttribI1i"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttribI1iv), "glVertexAttribI1iv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttribI1ui), "glVertexAttribI1ui"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttribI1uiv), "glVertexAttribI1uiv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttribI2i), "glVertexAttribI2i"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttribI2iv), "glVertexAttribI2iv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttribI2ui), "glVertexAttribI2ui"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttribI2uiv), "glVertexAttribI2uiv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttribI3i), "glVertexAttribI3i"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttribI3iv), "glVertexAttribI3iv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttribI3ui), "glVertexAttribI3ui"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttribI3uiv), "glVertexAttribI3uiv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttribI4bv), "glVertexAttribI4bv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttribI4i), "glVertexAttribI4i"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttribI4iv), "glVertexAttribI4iv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttribI4sv), "glVertexAttribI4sv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttribI4ubv), "glVertexAttribI4ubv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttribI4ui), "glVertexAttribI4ui"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttribI4uiv), "glVertexAttribI4uiv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttribI4usv), "glVertexAttribI4usv"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttribIPointer), "glVertexAttribIPointer"},
		{reinterpret_cast<GlSymbol*>(&glVertexAttribPointer), "glVertexAttribPointer"},
		{reinterpret_cast<GlSymbol*>(&glVertexPointer), "glVertexPointer"},
		{reinterpret_cast<GlSymbol*>(&glViewport), "glViewport"},
		{reinterpret_cast<GlSymbol*>(&glWaitSync), "glWaitSync"},
		{reinterpret_cast<GlSymbol*>(&glWindowPos2d), "glWindowPos2d"},
		{reinterpret_cast<GlSymbol*>(&glWindowPos2dv), "glWindowPos2dv"},
		{reinterpret_cast<GlSymbol*>(&glWindowPos2f), "glWindowPos2f"},
		{reinterpret_cast<GlSymbol*>(&glWindowPos2fv), "glWindowPos2fv"},
		{reinterpret_cast<GlSymbol*>(&glWindowPos2i), "glWindowPos2i"},
		{reinterpret_cast<GlSymbol*>(&glWindowPos2iv), "glWindowPos2iv"},
		{reinterpret_cast<GlSymbol*>(&glWindowPos2s), "glWindowPos2s"},
		{reinterpret_cast<GlSymbol*>(&glWindowPos2sv), "glWindowPos2sv"},
		{reinterpret_cast<GlSymbol*>(&glWindowPos3d), "glWindowPos3d"},
		{reinterpret_cast<GlSymbol*>(&glWindowPos3dv), "glWindowPos3dv"},
		{reinterpret_cast<GlSymbol*>(&glWindowPos3f), "glWindowPos3f"},
		{reinterpret_cast<GlSymbol*>(&glWindowPos3fv), "glWindowPos3fv"},
		{reinterpret_cast<GlSymbol*>(&glWindowPos3i), "glWindowPos3i"},
		{reinterpret_cast<GlSymbol*>(&glWindowPos3iv), "glWindowPos3iv"},
		{reinterpret_cast<GlSymbol*>(&glWindowPos3s), "glWindowPos3s"},
		{reinterpret_cast<GlSymbol*>(&glWindowPos3sv), "glWindowPos3sv"},
	};

	return gl_symbol_registry;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rExtensionMgrImpl::clear_gl_symbols()
try {
	auto& gl_symbol_registry = get_gl_symbol_registry();

	for (auto& gl_symbol_item : gl_symbol_registry)
	{
		*gl_symbol_item.first = nullptr;
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rExtensionMgrImpl::resolve_gl_symbols()
try {
	auto& gl_symbol_registry = get_gl_symbol_registry();

	for (auto& gl_symbol_item : gl_symbol_registry)
	{
		*gl_symbol_item.first = symbol_resolver_.find_symbol(gl_symbol_item.second);
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

bool GlR3rExtensionMgrImpl::has_gl_symbol(const GlSymbolPtrs& gl_symbol_ptrs) noexcept
{
	return std::all_of(
		gl_symbol_ptrs.cbegin(),
		gl_symbol_ptrs.cend(),
		[](GlSymbol* item)
		{
			return *item != nullptr;
		});
}

GlR3rExtensionMgrImpl::GlSymbolPtrs& GlR3rExtensionMgrImpl::get_essentials_gl_symbol_ptrs()
try {
	static auto gl_symbols = GlSymbolPtrs
	{
		reinterpret_cast<GlSymbol*>(&glGetError),
		reinterpret_cast<GlSymbol*>(&glGetIntegerv),
	};

	return gl_symbols;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlR3rExtensionMgrImpl::GlSymbolPtrs& GlR3rExtensionMgrImpl::get_v2_0_gl_symbol_ptrs()
try {
	static auto gl_symbols = GlSymbolPtrs
	{
		reinterpret_cast<GlSymbol*>(&glAccum),
		reinterpret_cast<GlSymbol*>(&glActiveTexture),
		reinterpret_cast<GlSymbol*>(&glAlphaFunc),
		reinterpret_cast<GlSymbol*>(&glAreTexturesResident),
		reinterpret_cast<GlSymbol*>(&glArrayElement),
		reinterpret_cast<GlSymbol*>(&glAttachShader),
		reinterpret_cast<GlSymbol*>(&glBegin),
		reinterpret_cast<GlSymbol*>(&glBeginQuery),
		reinterpret_cast<GlSymbol*>(&glBindAttribLocation),
		reinterpret_cast<GlSymbol*>(&glBindBuffer),
		reinterpret_cast<GlSymbol*>(&glBindTexture),
		reinterpret_cast<GlSymbol*>(&glBitmap),
		reinterpret_cast<GlSymbol*>(&glBlendColor),
		reinterpret_cast<GlSymbol*>(&glBlendEquation),
		reinterpret_cast<GlSymbol*>(&glBlendEquationSeparate),
		reinterpret_cast<GlSymbol*>(&glBlendFunc),
		reinterpret_cast<GlSymbol*>(&glBlendFuncSeparate),
		reinterpret_cast<GlSymbol*>(&glBufferData),
		reinterpret_cast<GlSymbol*>(&glBufferSubData),
		reinterpret_cast<GlSymbol*>(&glCallList),
		reinterpret_cast<GlSymbol*>(&glCallLists),
		reinterpret_cast<GlSymbol*>(&glClear),
		reinterpret_cast<GlSymbol*>(&glClearAccum),
		reinterpret_cast<GlSymbol*>(&glClearColor),
		reinterpret_cast<GlSymbol*>(&glClearDepth),
		reinterpret_cast<GlSymbol*>(&glClearIndex),
		reinterpret_cast<GlSymbol*>(&glClearStencil),
		reinterpret_cast<GlSymbol*>(&glClientActiveTexture),
		reinterpret_cast<GlSymbol*>(&glClipPlane),
		reinterpret_cast<GlSymbol*>(&glColor3b),
		reinterpret_cast<GlSymbol*>(&glColor3bv),
		reinterpret_cast<GlSymbol*>(&glColor3d),
		reinterpret_cast<GlSymbol*>(&glColor3dv),
		reinterpret_cast<GlSymbol*>(&glColor3f),
		reinterpret_cast<GlSymbol*>(&glColor3fv),
		reinterpret_cast<GlSymbol*>(&glColor3i),
		reinterpret_cast<GlSymbol*>(&glColor3iv),
		reinterpret_cast<GlSymbol*>(&glColor3s),
		reinterpret_cast<GlSymbol*>(&glColor3sv),
		reinterpret_cast<GlSymbol*>(&glColor3ub),
		reinterpret_cast<GlSymbol*>(&glColor3ubv),
		reinterpret_cast<GlSymbol*>(&glColor3ui),
		reinterpret_cast<GlSymbol*>(&glColor3uiv),
		reinterpret_cast<GlSymbol*>(&glColor3us),
		reinterpret_cast<GlSymbol*>(&glColor3usv),
		reinterpret_cast<GlSymbol*>(&glColor4b),
		reinterpret_cast<GlSymbol*>(&glColor4bv),
		reinterpret_cast<GlSymbol*>(&glColor4d),
		reinterpret_cast<GlSymbol*>(&glColor4dv),
		reinterpret_cast<GlSymbol*>(&glColor4f),
		reinterpret_cast<GlSymbol*>(&glColor4fv),
		reinterpret_cast<GlSymbol*>(&glColor4i),
		reinterpret_cast<GlSymbol*>(&glColor4iv),
		reinterpret_cast<GlSymbol*>(&glColor4s),
		reinterpret_cast<GlSymbol*>(&glColor4sv),
		reinterpret_cast<GlSymbol*>(&glColor4ub),
		reinterpret_cast<GlSymbol*>(&glColor4ubv),
		reinterpret_cast<GlSymbol*>(&glColor4ui),
		reinterpret_cast<GlSymbol*>(&glColor4uiv),
		reinterpret_cast<GlSymbol*>(&glColor4us),
		reinterpret_cast<GlSymbol*>(&glColor4usv),
		reinterpret_cast<GlSymbol*>(&glColorMask),
		reinterpret_cast<GlSymbol*>(&glColorMaterial),
		reinterpret_cast<GlSymbol*>(&glColorPointer),
		reinterpret_cast<GlSymbol*>(&glCompileShader),
		reinterpret_cast<GlSymbol*>(&glCompressedTexImage1D),
		reinterpret_cast<GlSymbol*>(&glCompressedTexImage2D),
		reinterpret_cast<GlSymbol*>(&glCompressedTexImage3D),
		reinterpret_cast<GlSymbol*>(&glCompressedTexSubImage1D),
		reinterpret_cast<GlSymbol*>(&glCompressedTexSubImage2D),
		reinterpret_cast<GlSymbol*>(&glCompressedTexSubImage3D),
		reinterpret_cast<GlSymbol*>(&glCopyPixels),
		reinterpret_cast<GlSymbol*>(&glCopyTexImage1D),
		reinterpret_cast<GlSymbol*>(&glCopyTexImage2D),
		reinterpret_cast<GlSymbol*>(&glCopyTexSubImage1D),
		reinterpret_cast<GlSymbol*>(&glCopyTexSubImage2D),
		reinterpret_cast<GlSymbol*>(&glCopyTexSubImage3D),
		reinterpret_cast<GlSymbol*>(&glCreateProgram),
		reinterpret_cast<GlSymbol*>(&glCreateShader),
		reinterpret_cast<GlSymbol*>(&glCullFace),
		reinterpret_cast<GlSymbol*>(&glDeleteBuffers),
		reinterpret_cast<GlSymbol*>(&glDeleteLists),
		reinterpret_cast<GlSymbol*>(&glDeleteProgram),
		reinterpret_cast<GlSymbol*>(&glDeleteQueries),
		reinterpret_cast<GlSymbol*>(&glDeleteShader),
		reinterpret_cast<GlSymbol*>(&glDeleteTextures),
		reinterpret_cast<GlSymbol*>(&glDepthFunc),
		reinterpret_cast<GlSymbol*>(&glDepthMask),
		reinterpret_cast<GlSymbol*>(&glDepthRange),
		reinterpret_cast<GlSymbol*>(&glDetachShader),
		reinterpret_cast<GlSymbol*>(&glDisable),
		reinterpret_cast<GlSymbol*>(&glDisableClientState),
		reinterpret_cast<GlSymbol*>(&glDisableVertexAttribArray),
		reinterpret_cast<GlSymbol*>(&glDrawArrays),
		reinterpret_cast<GlSymbol*>(&glDrawBuffer),
		reinterpret_cast<GlSymbol*>(&glDrawBuffers),
		reinterpret_cast<GlSymbol*>(&glDrawElements),
		reinterpret_cast<GlSymbol*>(&glDrawPixels),
		reinterpret_cast<GlSymbol*>(&glDrawRangeElements),
		reinterpret_cast<GlSymbol*>(&glEdgeFlag),
		reinterpret_cast<GlSymbol*>(&glEdgeFlagPointer),
		reinterpret_cast<GlSymbol*>(&glEdgeFlagv),
		reinterpret_cast<GlSymbol*>(&glEnable),
		reinterpret_cast<GlSymbol*>(&glEnableClientState),
		reinterpret_cast<GlSymbol*>(&glEnableVertexAttribArray),
		reinterpret_cast<GlSymbol*>(&glEnd),
		reinterpret_cast<GlSymbol*>(&glEndList),
		reinterpret_cast<GlSymbol*>(&glEndQuery),
		reinterpret_cast<GlSymbol*>(&glEvalCoord1d),
		reinterpret_cast<GlSymbol*>(&glEvalCoord1dv),
		reinterpret_cast<GlSymbol*>(&glEvalCoord1f),
		reinterpret_cast<GlSymbol*>(&glEvalCoord1fv),
		reinterpret_cast<GlSymbol*>(&glEvalCoord2d),
		reinterpret_cast<GlSymbol*>(&glEvalCoord2dv),
		reinterpret_cast<GlSymbol*>(&glEvalCoord2f),
		reinterpret_cast<GlSymbol*>(&glEvalCoord2fv),
		reinterpret_cast<GlSymbol*>(&glEvalMesh1),
		reinterpret_cast<GlSymbol*>(&glEvalMesh2),
		reinterpret_cast<GlSymbol*>(&glEvalPoint1),
		reinterpret_cast<GlSymbol*>(&glEvalPoint2),
		reinterpret_cast<GlSymbol*>(&glFeedbackBuffer),
		reinterpret_cast<GlSymbol*>(&glFinish),
		reinterpret_cast<GlSymbol*>(&glFlush),
		reinterpret_cast<GlSymbol*>(&glFogCoordPointer),
		reinterpret_cast<GlSymbol*>(&glFogCoordd),
		reinterpret_cast<GlSymbol*>(&glFogCoorddv),
		reinterpret_cast<GlSymbol*>(&glFogCoordf),
		reinterpret_cast<GlSymbol*>(&glFogCoordfv),
		reinterpret_cast<GlSymbol*>(&glFogf),
		reinterpret_cast<GlSymbol*>(&glFogfv),
		reinterpret_cast<GlSymbol*>(&glFogi),
		reinterpret_cast<GlSymbol*>(&glFogiv),
		reinterpret_cast<GlSymbol*>(&glFrontFace),
		reinterpret_cast<GlSymbol*>(&glFrustum),
		reinterpret_cast<GlSymbol*>(&glGenBuffers),
		reinterpret_cast<GlSymbol*>(&glGenLists),
		reinterpret_cast<GlSymbol*>(&glGenQueries),
		reinterpret_cast<GlSymbol*>(&glGenTextures),
		reinterpret_cast<GlSymbol*>(&glGetActiveAttrib),
		reinterpret_cast<GlSymbol*>(&glGetActiveUniform),
		reinterpret_cast<GlSymbol*>(&glGetAttachedShaders),
		reinterpret_cast<GlSymbol*>(&glGetAttribLocation),
		reinterpret_cast<GlSymbol*>(&glGetBooleanv),
		reinterpret_cast<GlSymbol*>(&glGetBufferParameteriv),
		reinterpret_cast<GlSymbol*>(&glGetBufferPointerv),
		reinterpret_cast<GlSymbol*>(&glGetBufferSubData),
		reinterpret_cast<GlSymbol*>(&glGetClipPlane),
		reinterpret_cast<GlSymbol*>(&glGetCompressedTexImage),
		reinterpret_cast<GlSymbol*>(&glGetDoublev),
		reinterpret_cast<GlSymbol*>(&glGetError),
		reinterpret_cast<GlSymbol*>(&glGetFloatv),
		reinterpret_cast<GlSymbol*>(&glGetIntegerv),
		reinterpret_cast<GlSymbol*>(&glGetLightfv),
		reinterpret_cast<GlSymbol*>(&glGetLightiv),
		reinterpret_cast<GlSymbol*>(&glGetMapdv),
		reinterpret_cast<GlSymbol*>(&glGetMapfv),
		reinterpret_cast<GlSymbol*>(&glGetMapiv),
		reinterpret_cast<GlSymbol*>(&glGetMaterialfv),
		reinterpret_cast<GlSymbol*>(&glGetMaterialiv),
		reinterpret_cast<GlSymbol*>(&glGetPixelMapfv),
		reinterpret_cast<GlSymbol*>(&glGetPixelMapuiv),
		reinterpret_cast<GlSymbol*>(&glGetPixelMapusv),
		reinterpret_cast<GlSymbol*>(&glGetPointerv),
		reinterpret_cast<GlSymbol*>(&glGetPolygonStipple),
		reinterpret_cast<GlSymbol*>(&glGetProgramInfoLog),
		reinterpret_cast<GlSymbol*>(&glGetProgramiv),
		reinterpret_cast<GlSymbol*>(&glGetQueryObjectiv),
		reinterpret_cast<GlSymbol*>(&glGetQueryObjectuiv),
		reinterpret_cast<GlSymbol*>(&glGetQueryiv),
		reinterpret_cast<GlSymbol*>(&glGetShaderInfoLog),
		reinterpret_cast<GlSymbol*>(&glGetShaderSource),
		reinterpret_cast<GlSymbol*>(&glGetShaderiv),
		reinterpret_cast<GlSymbol*>(&glGetString),
		reinterpret_cast<GlSymbol*>(&glGetTexEnvfv),
		reinterpret_cast<GlSymbol*>(&glGetTexEnviv),
		reinterpret_cast<GlSymbol*>(&glGetTexGendv),
		reinterpret_cast<GlSymbol*>(&glGetTexGenfv),
		reinterpret_cast<GlSymbol*>(&glGetTexGeniv),
		reinterpret_cast<GlSymbol*>(&glGetTexImage),
		reinterpret_cast<GlSymbol*>(&glGetTexLevelParameterfv),
		reinterpret_cast<GlSymbol*>(&glGetTexLevelParameteriv),
		reinterpret_cast<GlSymbol*>(&glGetTexParameterfv),
		reinterpret_cast<GlSymbol*>(&glGetTexParameteriv),
		reinterpret_cast<GlSymbol*>(&glGetUniformLocation),
		reinterpret_cast<GlSymbol*>(&glGetUniformfv),
		reinterpret_cast<GlSymbol*>(&glGetUniformiv),
		reinterpret_cast<GlSymbol*>(&glGetVertexAttribPointerv),
		reinterpret_cast<GlSymbol*>(&glGetVertexAttribdv),
		reinterpret_cast<GlSymbol*>(&glGetVertexAttribfv),
		reinterpret_cast<GlSymbol*>(&glGetVertexAttribiv),
		reinterpret_cast<GlSymbol*>(&glHint),
		reinterpret_cast<GlSymbol*>(&glIndexMask),
		reinterpret_cast<GlSymbol*>(&glIndexPointer),
		reinterpret_cast<GlSymbol*>(&glIndexd),
		reinterpret_cast<GlSymbol*>(&glIndexdv),
		reinterpret_cast<GlSymbol*>(&glIndexf),
		reinterpret_cast<GlSymbol*>(&glIndexfv),
		reinterpret_cast<GlSymbol*>(&glIndexi),
		reinterpret_cast<GlSymbol*>(&glIndexiv),
		reinterpret_cast<GlSymbol*>(&glIndexs),
		reinterpret_cast<GlSymbol*>(&glIndexsv),
		reinterpret_cast<GlSymbol*>(&glIndexub),
		reinterpret_cast<GlSymbol*>(&glIndexubv),
		reinterpret_cast<GlSymbol*>(&glInitNames),
		reinterpret_cast<GlSymbol*>(&glInterleavedArrays),
		reinterpret_cast<GlSymbol*>(&glIsBuffer),
		reinterpret_cast<GlSymbol*>(&glIsEnabled),
		reinterpret_cast<GlSymbol*>(&glIsList),
		reinterpret_cast<GlSymbol*>(&glIsProgram),
		reinterpret_cast<GlSymbol*>(&glIsQuery),
		reinterpret_cast<GlSymbol*>(&glIsShader),
		reinterpret_cast<GlSymbol*>(&glIsTexture),
		reinterpret_cast<GlSymbol*>(&glLightModelf),
		reinterpret_cast<GlSymbol*>(&glLightModelfv),
		reinterpret_cast<GlSymbol*>(&glLightModeli),
		reinterpret_cast<GlSymbol*>(&glLightModeliv),
		reinterpret_cast<GlSymbol*>(&glLightf),
		reinterpret_cast<GlSymbol*>(&glLightfv),
		reinterpret_cast<GlSymbol*>(&glLighti),
		reinterpret_cast<GlSymbol*>(&glLightiv),
		reinterpret_cast<GlSymbol*>(&glLineStipple),
		reinterpret_cast<GlSymbol*>(&glLineWidth),
		reinterpret_cast<GlSymbol*>(&glLinkProgram),
		reinterpret_cast<GlSymbol*>(&glListBase),
		reinterpret_cast<GlSymbol*>(&glLoadIdentity),
		reinterpret_cast<GlSymbol*>(&glLoadMatrixd),
		reinterpret_cast<GlSymbol*>(&glLoadMatrixf),
		reinterpret_cast<GlSymbol*>(&glLoadName),
		reinterpret_cast<GlSymbol*>(&glLoadTransposeMatrixd),
		reinterpret_cast<GlSymbol*>(&glLoadTransposeMatrixf),
		reinterpret_cast<GlSymbol*>(&glLogicOp),
		reinterpret_cast<GlSymbol*>(&glMap1d),
		reinterpret_cast<GlSymbol*>(&glMap1f),
		reinterpret_cast<GlSymbol*>(&glMap2d),
		reinterpret_cast<GlSymbol*>(&glMap2f),
		reinterpret_cast<GlSymbol*>(&glMapBuffer),
		reinterpret_cast<GlSymbol*>(&glMapGrid1d),
		reinterpret_cast<GlSymbol*>(&glMapGrid1f),
		reinterpret_cast<GlSymbol*>(&glMapGrid2d),
		reinterpret_cast<GlSymbol*>(&glMapGrid2f),
		reinterpret_cast<GlSymbol*>(&glMaterialf),
		reinterpret_cast<GlSymbol*>(&glMaterialfv),
		reinterpret_cast<GlSymbol*>(&glMateriali),
		reinterpret_cast<GlSymbol*>(&glMaterialiv),
		reinterpret_cast<GlSymbol*>(&glMatrixMode),
		reinterpret_cast<GlSymbol*>(&glMultMatrixd),
		reinterpret_cast<GlSymbol*>(&glMultMatrixf),
		reinterpret_cast<GlSymbol*>(&glMultTransposeMatrixd),
		reinterpret_cast<GlSymbol*>(&glMultTransposeMatrixf),
		reinterpret_cast<GlSymbol*>(&glMultiDrawArrays),
		reinterpret_cast<GlSymbol*>(&glMultiDrawElements),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord1d),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord1dv),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord1f),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord1fv),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord1i),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord1iv),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord1s),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord1sv),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord2d),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord2dv),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord2f),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord2fv),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord2i),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord2iv),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord2s),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord2sv),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord3d),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord3dv),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord3f),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord3fv),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord3i),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord3iv),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord3s),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord3sv),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord4d),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord4dv),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord4f),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord4fv),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord4i),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord4iv),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord4s),
		reinterpret_cast<GlSymbol*>(&glMultiTexCoord4sv),
		reinterpret_cast<GlSymbol*>(&glNewList),
		reinterpret_cast<GlSymbol*>(&glNormal3b),
		reinterpret_cast<GlSymbol*>(&glNormal3bv),
		reinterpret_cast<GlSymbol*>(&glNormal3d),
		reinterpret_cast<GlSymbol*>(&glNormal3dv),
		reinterpret_cast<GlSymbol*>(&glNormal3f),
		reinterpret_cast<GlSymbol*>(&glNormal3fv),
		reinterpret_cast<GlSymbol*>(&glNormal3i),
		reinterpret_cast<GlSymbol*>(&glNormal3iv),
		reinterpret_cast<GlSymbol*>(&glNormal3s),
		reinterpret_cast<GlSymbol*>(&glNormal3sv),
		reinterpret_cast<GlSymbol*>(&glNormalPointer),
		reinterpret_cast<GlSymbol*>(&glOrtho),
		reinterpret_cast<GlSymbol*>(&glPassThrough),
		reinterpret_cast<GlSymbol*>(&glPixelMapfv),
		reinterpret_cast<GlSymbol*>(&glPixelMapuiv),
		reinterpret_cast<GlSymbol*>(&glPixelMapusv),
		reinterpret_cast<GlSymbol*>(&glPixelStoref),
		reinterpret_cast<GlSymbol*>(&glPixelStorei),
		reinterpret_cast<GlSymbol*>(&glPixelTransferf),
		reinterpret_cast<GlSymbol*>(&glPixelTransferi),
		reinterpret_cast<GlSymbol*>(&glPixelZoom),
		reinterpret_cast<GlSymbol*>(&glPointParameterf),
		reinterpret_cast<GlSymbol*>(&glPointParameterfv),
		reinterpret_cast<GlSymbol*>(&glPointParameteri),
		reinterpret_cast<GlSymbol*>(&glPointParameteriv),
		reinterpret_cast<GlSymbol*>(&glPointSize),
		reinterpret_cast<GlSymbol*>(&glPolygonMode),
		reinterpret_cast<GlSymbol*>(&glPolygonOffset),
		reinterpret_cast<GlSymbol*>(&glPolygonStipple),
		reinterpret_cast<GlSymbol*>(&glPopAttrib),
		reinterpret_cast<GlSymbol*>(&glPopClientAttrib),
		reinterpret_cast<GlSymbol*>(&glPopMatrix),
		reinterpret_cast<GlSymbol*>(&glPopName),
		reinterpret_cast<GlSymbol*>(&glPrioritizeTextures),
		reinterpret_cast<GlSymbol*>(&glPushAttrib),
		reinterpret_cast<GlSymbol*>(&glPushClientAttrib),
		reinterpret_cast<GlSymbol*>(&glPushMatrix),
		reinterpret_cast<GlSymbol*>(&glPushName),
		reinterpret_cast<GlSymbol*>(&glRasterPos2d),
		reinterpret_cast<GlSymbol*>(&glRasterPos2dv),
		reinterpret_cast<GlSymbol*>(&glRasterPos2f),
		reinterpret_cast<GlSymbol*>(&glRasterPos2fv),
		reinterpret_cast<GlSymbol*>(&glRasterPos2i),
		reinterpret_cast<GlSymbol*>(&glRasterPos2iv),
		reinterpret_cast<GlSymbol*>(&glRasterPos2s),
		reinterpret_cast<GlSymbol*>(&glRasterPos2sv),
		reinterpret_cast<GlSymbol*>(&glRasterPos3d),
		reinterpret_cast<GlSymbol*>(&glRasterPos3dv),
		reinterpret_cast<GlSymbol*>(&glRasterPos3f),
		reinterpret_cast<GlSymbol*>(&glRasterPos3fv),
		reinterpret_cast<GlSymbol*>(&glRasterPos3i),
		reinterpret_cast<GlSymbol*>(&glRasterPos3iv),
		reinterpret_cast<GlSymbol*>(&glRasterPos3s),
		reinterpret_cast<GlSymbol*>(&glRasterPos3sv),
		reinterpret_cast<GlSymbol*>(&glRasterPos4d),
		reinterpret_cast<GlSymbol*>(&glRasterPos4dv),
		reinterpret_cast<GlSymbol*>(&glRasterPos4f),
		reinterpret_cast<GlSymbol*>(&glRasterPos4fv),
		reinterpret_cast<GlSymbol*>(&glRasterPos4i),
		reinterpret_cast<GlSymbol*>(&glRasterPos4iv),
		reinterpret_cast<GlSymbol*>(&glRasterPos4s),
		reinterpret_cast<GlSymbol*>(&glRasterPos4sv),
		reinterpret_cast<GlSymbol*>(&glReadBuffer),
		reinterpret_cast<GlSymbol*>(&glReadPixels),
		reinterpret_cast<GlSymbol*>(&glRectd),
		reinterpret_cast<GlSymbol*>(&glRectdv),
		reinterpret_cast<GlSymbol*>(&glRectf),
		reinterpret_cast<GlSymbol*>(&glRectfv),
		reinterpret_cast<GlSymbol*>(&glRecti),
		reinterpret_cast<GlSymbol*>(&glRectiv),
		reinterpret_cast<GlSymbol*>(&glRects),
		reinterpret_cast<GlSymbol*>(&glRectsv),
		reinterpret_cast<GlSymbol*>(&glRenderMode),
		reinterpret_cast<GlSymbol*>(&glRotated),
		reinterpret_cast<GlSymbol*>(&glRotatef),
		reinterpret_cast<GlSymbol*>(&glSampleCoverage),
		reinterpret_cast<GlSymbol*>(&glScaled),
		reinterpret_cast<GlSymbol*>(&glScalef),
		reinterpret_cast<GlSymbol*>(&glScissor),
		reinterpret_cast<GlSymbol*>(&glSecondaryColor3b),
		reinterpret_cast<GlSymbol*>(&glSecondaryColor3bv),
		reinterpret_cast<GlSymbol*>(&glSecondaryColor3d),
		reinterpret_cast<GlSymbol*>(&glSecondaryColor3dv),
		reinterpret_cast<GlSymbol*>(&glSecondaryColor3f),
		reinterpret_cast<GlSymbol*>(&glSecondaryColor3fv),
		reinterpret_cast<GlSymbol*>(&glSecondaryColor3i),
		reinterpret_cast<GlSymbol*>(&glSecondaryColor3iv),
		reinterpret_cast<GlSymbol*>(&glSecondaryColor3s),
		reinterpret_cast<GlSymbol*>(&glSecondaryColor3sv),
		reinterpret_cast<GlSymbol*>(&glSecondaryColor3ub),
		reinterpret_cast<GlSymbol*>(&glSecondaryColor3ubv),
		reinterpret_cast<GlSymbol*>(&glSecondaryColor3ui),
		reinterpret_cast<GlSymbol*>(&glSecondaryColor3uiv),
		reinterpret_cast<GlSymbol*>(&glSecondaryColor3us),
		reinterpret_cast<GlSymbol*>(&glSecondaryColor3usv),
		reinterpret_cast<GlSymbol*>(&glSecondaryColorPointer),
		reinterpret_cast<GlSymbol*>(&glSelectBuffer),
		reinterpret_cast<GlSymbol*>(&glShadeModel),
		reinterpret_cast<GlSymbol*>(&glShaderSource),
		reinterpret_cast<GlSymbol*>(&glStencilFunc),
		reinterpret_cast<GlSymbol*>(&glStencilFuncSeparate),
		reinterpret_cast<GlSymbol*>(&glStencilMask),
		reinterpret_cast<GlSymbol*>(&glStencilMaskSeparate),
		reinterpret_cast<GlSymbol*>(&glStencilOp),
		reinterpret_cast<GlSymbol*>(&glStencilOpSeparate),
		reinterpret_cast<GlSymbol*>(&glTexCoord1d),
		reinterpret_cast<GlSymbol*>(&glTexCoord1dv),
		reinterpret_cast<GlSymbol*>(&glTexCoord1f),
		reinterpret_cast<GlSymbol*>(&glTexCoord1fv),
		reinterpret_cast<GlSymbol*>(&glTexCoord1i),
		reinterpret_cast<GlSymbol*>(&glTexCoord1iv),
		reinterpret_cast<GlSymbol*>(&glTexCoord1s),
		reinterpret_cast<GlSymbol*>(&glTexCoord1sv),
		reinterpret_cast<GlSymbol*>(&glTexCoord2d),
		reinterpret_cast<GlSymbol*>(&glTexCoord2dv),
		reinterpret_cast<GlSymbol*>(&glTexCoord2f),
		reinterpret_cast<GlSymbol*>(&glTexCoord2fv),
		reinterpret_cast<GlSymbol*>(&glTexCoord2i),
		reinterpret_cast<GlSymbol*>(&glTexCoord2iv),
		reinterpret_cast<GlSymbol*>(&glTexCoord2s),
		reinterpret_cast<GlSymbol*>(&glTexCoord2sv),
		reinterpret_cast<GlSymbol*>(&glTexCoord3d),
		reinterpret_cast<GlSymbol*>(&glTexCoord3dv),
		reinterpret_cast<GlSymbol*>(&glTexCoord3f),
		reinterpret_cast<GlSymbol*>(&glTexCoord3fv),
		reinterpret_cast<GlSymbol*>(&glTexCoord3i),
		reinterpret_cast<GlSymbol*>(&glTexCoord3iv),
		reinterpret_cast<GlSymbol*>(&glTexCoord3s),
		reinterpret_cast<GlSymbol*>(&glTexCoord3sv),
		reinterpret_cast<GlSymbol*>(&glTexCoord4d),
		reinterpret_cast<GlSymbol*>(&glTexCoord4dv),
		reinterpret_cast<GlSymbol*>(&glTexCoord4f),
		reinterpret_cast<GlSymbol*>(&glTexCoord4fv),
		reinterpret_cast<GlSymbol*>(&glTexCoord4i),
		reinterpret_cast<GlSymbol*>(&glTexCoord4iv),
		reinterpret_cast<GlSymbol*>(&glTexCoord4s),
		reinterpret_cast<GlSymbol*>(&glTexCoord4sv),
		reinterpret_cast<GlSymbol*>(&glTexCoordPointer),
		reinterpret_cast<GlSymbol*>(&glTexEnvf),
		reinterpret_cast<GlSymbol*>(&glTexEnvfv),
		reinterpret_cast<GlSymbol*>(&glTexEnvi),
		reinterpret_cast<GlSymbol*>(&glTexEnviv),
		reinterpret_cast<GlSymbol*>(&glTexGend),
		reinterpret_cast<GlSymbol*>(&glTexGendv),
		reinterpret_cast<GlSymbol*>(&glTexGenf),
		reinterpret_cast<GlSymbol*>(&glTexGenfv),
		reinterpret_cast<GlSymbol*>(&glTexGeni),
		reinterpret_cast<GlSymbol*>(&glTexGeniv),
		reinterpret_cast<GlSymbol*>(&glTexImage1D),
		reinterpret_cast<GlSymbol*>(&glTexImage2D),
		reinterpret_cast<GlSymbol*>(&glTexImage3D),
		reinterpret_cast<GlSymbol*>(&glTexParameterf),
		reinterpret_cast<GlSymbol*>(&glTexParameterfv),
		reinterpret_cast<GlSymbol*>(&glTexParameteri),
		reinterpret_cast<GlSymbol*>(&glTexParameteriv),
		reinterpret_cast<GlSymbol*>(&glTexSubImage1D),
		reinterpret_cast<GlSymbol*>(&glTexSubImage2D),
		reinterpret_cast<GlSymbol*>(&glTexSubImage3D),
		reinterpret_cast<GlSymbol*>(&glTranslated),
		reinterpret_cast<GlSymbol*>(&glTranslatef),
		reinterpret_cast<GlSymbol*>(&glUniform1f),
		reinterpret_cast<GlSymbol*>(&glUniform1fv),
		reinterpret_cast<GlSymbol*>(&glUniform1i),
		reinterpret_cast<GlSymbol*>(&glUniform1iv),
		reinterpret_cast<GlSymbol*>(&glUniform2f),
		reinterpret_cast<GlSymbol*>(&glUniform2fv),
		reinterpret_cast<GlSymbol*>(&glUniform2i),
		reinterpret_cast<GlSymbol*>(&glUniform2iv),
		reinterpret_cast<GlSymbol*>(&glUniform3f),
		reinterpret_cast<GlSymbol*>(&glUniform3fv),
		reinterpret_cast<GlSymbol*>(&glUniform3i),
		reinterpret_cast<GlSymbol*>(&glUniform3iv),
		reinterpret_cast<GlSymbol*>(&glUniform4f),
		reinterpret_cast<GlSymbol*>(&glUniform4fv),
		reinterpret_cast<GlSymbol*>(&glUniform4i),
		reinterpret_cast<GlSymbol*>(&glUniform4iv),
		reinterpret_cast<GlSymbol*>(&glUniformMatrix2fv),
		reinterpret_cast<GlSymbol*>(&glUniformMatrix3fv),
		reinterpret_cast<GlSymbol*>(&glUniformMatrix4fv),
		reinterpret_cast<GlSymbol*>(&glUnmapBuffer),
		reinterpret_cast<GlSymbol*>(&glUseProgram),
		reinterpret_cast<GlSymbol*>(&glValidateProgram),
		reinterpret_cast<GlSymbol*>(&glVertex2d),
		reinterpret_cast<GlSymbol*>(&glVertex2dv),
		reinterpret_cast<GlSymbol*>(&glVertex2f),
		reinterpret_cast<GlSymbol*>(&glVertex2fv),
		reinterpret_cast<GlSymbol*>(&glVertex2i),
		reinterpret_cast<GlSymbol*>(&glVertex2iv),
		reinterpret_cast<GlSymbol*>(&glVertex2s),
		reinterpret_cast<GlSymbol*>(&glVertex2sv),
		reinterpret_cast<GlSymbol*>(&glVertex3d),
		reinterpret_cast<GlSymbol*>(&glVertex3dv),
		reinterpret_cast<GlSymbol*>(&glVertex3f),
		reinterpret_cast<GlSymbol*>(&glVertex3fv),
		reinterpret_cast<GlSymbol*>(&glVertex3i),
		reinterpret_cast<GlSymbol*>(&glVertex3iv),
		reinterpret_cast<GlSymbol*>(&glVertex3s),
		reinterpret_cast<GlSymbol*>(&glVertex3sv),
		reinterpret_cast<GlSymbol*>(&glVertex4d),
		reinterpret_cast<GlSymbol*>(&glVertex4dv),
		reinterpret_cast<GlSymbol*>(&glVertex4f),
		reinterpret_cast<GlSymbol*>(&glVertex4fv),
		reinterpret_cast<GlSymbol*>(&glVertex4i),
		reinterpret_cast<GlSymbol*>(&glVertex4iv),
		reinterpret_cast<GlSymbol*>(&glVertex4s),
		reinterpret_cast<GlSymbol*>(&glVertex4sv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib1d),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib1dv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib1f),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib1fv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib1s),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib1sv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib2d),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib2dv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib2f),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib2fv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib2s),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib2sv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib3d),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib3dv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib3f),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib3fv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib3s),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib3sv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4Nbv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4Niv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4Nsv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4Nub),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4Nubv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4Nuiv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4Nusv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4bv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4d),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4dv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4f),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4fv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4iv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4s),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4sv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4ubv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4uiv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4usv),
		reinterpret_cast<GlSymbol*>(&glVertexAttribPointer),
		reinterpret_cast<GlSymbol*>(&glVertexPointer),
		reinterpret_cast<GlSymbol*>(&glViewport),
		reinterpret_cast<GlSymbol*>(&glWindowPos2d),
		reinterpret_cast<GlSymbol*>(&glWindowPos2dv),
		reinterpret_cast<GlSymbol*>(&glWindowPos2f),
		reinterpret_cast<GlSymbol*>(&glWindowPos2fv),
		reinterpret_cast<GlSymbol*>(&glWindowPos2i),
		reinterpret_cast<GlSymbol*>(&glWindowPos2iv),
		reinterpret_cast<GlSymbol*>(&glWindowPos2s),
		reinterpret_cast<GlSymbol*>(&glWindowPos2sv),
		reinterpret_cast<GlSymbol*>(&glWindowPos3d),
		reinterpret_cast<GlSymbol*>(&glWindowPos3dv),
		reinterpret_cast<GlSymbol*>(&glWindowPos3f),
		reinterpret_cast<GlSymbol*>(&glWindowPos3fv),
		reinterpret_cast<GlSymbol*>(&glWindowPos3i),
		reinterpret_cast<GlSymbol*>(&glWindowPos3iv),
		reinterpret_cast<GlSymbol*>(&glWindowPos3s),
		reinterpret_cast<GlSymbol*>(&glWindowPos3sv),
	};

	return gl_symbols;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlR3rExtensionMgrImpl::GlSymbolPtrs& GlR3rExtensionMgrImpl::get_v3_2_core_gl_symbol_ptrs()
try {
	static auto gl_symbols = GlSymbolPtrs
	{
		reinterpret_cast<GlSymbol*>(&glActiveTexture),
		reinterpret_cast<GlSymbol*>(&glAttachShader),
		reinterpret_cast<GlSymbol*>(&glBeginConditionalRender),
		reinterpret_cast<GlSymbol*>(&glBeginQuery),
		reinterpret_cast<GlSymbol*>(&glBeginTransformFeedback),
		reinterpret_cast<GlSymbol*>(&glBindAttribLocation),
		reinterpret_cast<GlSymbol*>(&glBindBuffer),
		reinterpret_cast<GlSymbol*>(&glBindBufferBase),
		reinterpret_cast<GlSymbol*>(&glBindBufferRange),
		reinterpret_cast<GlSymbol*>(&glBindFragDataLocation),
		reinterpret_cast<GlSymbol*>(&glBindFramebuffer),
		reinterpret_cast<GlSymbol*>(&glBindRenderbuffer),
		reinterpret_cast<GlSymbol*>(&glBindTexture),
		reinterpret_cast<GlSymbol*>(&glBindVertexArray),
		reinterpret_cast<GlSymbol*>(&glBlendColor),
		reinterpret_cast<GlSymbol*>(&glBlendEquation),
		reinterpret_cast<GlSymbol*>(&glBlendEquationSeparate),
		reinterpret_cast<GlSymbol*>(&glBlendFunc),
		reinterpret_cast<GlSymbol*>(&glBlendFuncSeparate),
		reinterpret_cast<GlSymbol*>(&glBlitFramebuffer),
		reinterpret_cast<GlSymbol*>(&glBufferData),
		reinterpret_cast<GlSymbol*>(&glBufferSubData),
		reinterpret_cast<GlSymbol*>(&glCheckFramebufferStatus),
		reinterpret_cast<GlSymbol*>(&glClampColor),
		reinterpret_cast<GlSymbol*>(&glClear),
		reinterpret_cast<GlSymbol*>(&glClearBufferfi),
		reinterpret_cast<GlSymbol*>(&glClearBufferfv),
		reinterpret_cast<GlSymbol*>(&glClearBufferiv),
		reinterpret_cast<GlSymbol*>(&glClearBufferuiv),
		reinterpret_cast<GlSymbol*>(&glClearColor),
		reinterpret_cast<GlSymbol*>(&glClearDepth),
		reinterpret_cast<GlSymbol*>(&glClearStencil),
		reinterpret_cast<GlSymbol*>(&glClientWaitSync),
		reinterpret_cast<GlSymbol*>(&glColorMask),
		reinterpret_cast<GlSymbol*>(&glColorMaski),
		reinterpret_cast<GlSymbol*>(&glCompileShader),
		reinterpret_cast<GlSymbol*>(&glCompressedTexImage1D),
		reinterpret_cast<GlSymbol*>(&glCompressedTexImage2D),
		reinterpret_cast<GlSymbol*>(&glCompressedTexImage3D),
		reinterpret_cast<GlSymbol*>(&glCompressedTexSubImage1D),
		reinterpret_cast<GlSymbol*>(&glCompressedTexSubImage2D),
		reinterpret_cast<GlSymbol*>(&glCompressedTexSubImage3D),
		reinterpret_cast<GlSymbol*>(&glCopyBufferSubData),
		reinterpret_cast<GlSymbol*>(&glCopyTexImage1D),
		reinterpret_cast<GlSymbol*>(&glCopyTexImage2D),
		reinterpret_cast<GlSymbol*>(&glCopyTexSubImage1D),
		reinterpret_cast<GlSymbol*>(&glCopyTexSubImage2D),
		reinterpret_cast<GlSymbol*>(&glCopyTexSubImage3D),
		reinterpret_cast<GlSymbol*>(&glCreateProgram),
		reinterpret_cast<GlSymbol*>(&glCreateShader),
		reinterpret_cast<GlSymbol*>(&glCullFace),
		reinterpret_cast<GlSymbol*>(&glDeleteBuffers),
		reinterpret_cast<GlSymbol*>(&glDeleteFramebuffers),
		reinterpret_cast<GlSymbol*>(&glDeleteProgram),
		reinterpret_cast<GlSymbol*>(&glDeleteQueries),
		reinterpret_cast<GlSymbol*>(&glDeleteRenderbuffers),
		reinterpret_cast<GlSymbol*>(&glDeleteShader),
		reinterpret_cast<GlSymbol*>(&glDeleteSync),
		reinterpret_cast<GlSymbol*>(&glDeleteTextures),
		reinterpret_cast<GlSymbol*>(&glDeleteVertexArrays),
		reinterpret_cast<GlSymbol*>(&glDepthFunc),
		reinterpret_cast<GlSymbol*>(&glDepthMask),
		reinterpret_cast<GlSymbol*>(&glDepthRange),
		reinterpret_cast<GlSymbol*>(&glDetachShader),
		reinterpret_cast<GlSymbol*>(&glDisable),
		reinterpret_cast<GlSymbol*>(&glDisableVertexAttribArray),
		reinterpret_cast<GlSymbol*>(&glDisablei),
		reinterpret_cast<GlSymbol*>(&glDrawArrays),
		reinterpret_cast<GlSymbol*>(&glDrawArraysInstanced),
		reinterpret_cast<GlSymbol*>(&glDrawBuffer),
		reinterpret_cast<GlSymbol*>(&glDrawBuffers),
		reinterpret_cast<GlSymbol*>(&glDrawElements),
		reinterpret_cast<GlSymbol*>(&glDrawElementsBaseVertex),
		reinterpret_cast<GlSymbol*>(&glDrawElementsInstanced),
		reinterpret_cast<GlSymbol*>(&glDrawElementsInstancedBaseVertex),
		reinterpret_cast<GlSymbol*>(&glDrawRangeElements),
		reinterpret_cast<GlSymbol*>(&glDrawRangeElementsBaseVertex),
		reinterpret_cast<GlSymbol*>(&glEnable),
		reinterpret_cast<GlSymbol*>(&glEnableVertexAttribArray),
		reinterpret_cast<GlSymbol*>(&glEnablei),
		reinterpret_cast<GlSymbol*>(&glEndConditionalRender),
		reinterpret_cast<GlSymbol*>(&glEndQuery),
		reinterpret_cast<GlSymbol*>(&glEndTransformFeedback),
		reinterpret_cast<GlSymbol*>(&glFenceSync),
		reinterpret_cast<GlSymbol*>(&glFinish),
		reinterpret_cast<GlSymbol*>(&glFlush),
		reinterpret_cast<GlSymbol*>(&glFlushMappedBufferRange),
		reinterpret_cast<GlSymbol*>(&glFramebufferRenderbuffer),
		reinterpret_cast<GlSymbol*>(&glFramebufferTexture),
		reinterpret_cast<GlSymbol*>(&glFramebufferTexture1D),
		reinterpret_cast<GlSymbol*>(&glFramebufferTexture2D),
		reinterpret_cast<GlSymbol*>(&glFramebufferTexture3D),
		reinterpret_cast<GlSymbol*>(&glFramebufferTextureLayer),
		reinterpret_cast<GlSymbol*>(&glFrontFace),
		reinterpret_cast<GlSymbol*>(&glGenBuffers),
		reinterpret_cast<GlSymbol*>(&glGenFramebuffers),
		reinterpret_cast<GlSymbol*>(&glGenQueries),
		reinterpret_cast<GlSymbol*>(&glGenRenderbuffers),
		reinterpret_cast<GlSymbol*>(&glGenTextures),
		reinterpret_cast<GlSymbol*>(&glGenVertexArrays),
		reinterpret_cast<GlSymbol*>(&glGenerateMipmap),
		reinterpret_cast<GlSymbol*>(&glGetActiveAttrib),
		reinterpret_cast<GlSymbol*>(&glGetActiveUniform),
		reinterpret_cast<GlSymbol*>(&glGetActiveUniformBlockName),
		reinterpret_cast<GlSymbol*>(&glGetActiveUniformBlockiv),
		reinterpret_cast<GlSymbol*>(&glGetActiveUniformName),
		reinterpret_cast<GlSymbol*>(&glGetActiveUniformsiv),
		reinterpret_cast<GlSymbol*>(&glGetAttachedShaders),
		reinterpret_cast<GlSymbol*>(&glGetAttribLocation),
		reinterpret_cast<GlSymbol*>(&glGetBooleani_v),
		reinterpret_cast<GlSymbol*>(&glGetBooleanv),
		reinterpret_cast<GlSymbol*>(&glGetBufferParameteri64v),
		reinterpret_cast<GlSymbol*>(&glGetBufferParameteriv),
		reinterpret_cast<GlSymbol*>(&glGetBufferPointerv),
		reinterpret_cast<GlSymbol*>(&glGetBufferSubData),
		reinterpret_cast<GlSymbol*>(&glGetCompressedTexImage),
		reinterpret_cast<GlSymbol*>(&glGetDoublev),
		reinterpret_cast<GlSymbol*>(&glGetError),
		reinterpret_cast<GlSymbol*>(&glGetFloatv),
		reinterpret_cast<GlSymbol*>(&glGetFragDataLocation),
		reinterpret_cast<GlSymbol*>(&glGetFramebufferAttachmentParameteriv),
		reinterpret_cast<GlSymbol*>(&glGetInteger64i_v),
		reinterpret_cast<GlSymbol*>(&glGetInteger64v),
		reinterpret_cast<GlSymbol*>(&glGetIntegeri_v),
		reinterpret_cast<GlSymbol*>(&glGetIntegerv),
		reinterpret_cast<GlSymbol*>(&glGetMultisamplefv),
		reinterpret_cast<GlSymbol*>(&glGetProgramInfoLog),
		reinterpret_cast<GlSymbol*>(&glGetProgramiv),
		reinterpret_cast<GlSymbol*>(&glGetQueryObjectiv),
		reinterpret_cast<GlSymbol*>(&glGetQueryObjectuiv),
		reinterpret_cast<GlSymbol*>(&glGetQueryiv),
		reinterpret_cast<GlSymbol*>(&glGetRenderbufferParameteriv),
		reinterpret_cast<GlSymbol*>(&glGetShaderInfoLog),
		reinterpret_cast<GlSymbol*>(&glGetShaderSource),
		reinterpret_cast<GlSymbol*>(&glGetShaderiv),
		reinterpret_cast<GlSymbol*>(&glGetString),
		reinterpret_cast<GlSymbol*>(&glGetStringi),
		reinterpret_cast<GlSymbol*>(&glGetSynciv),
		reinterpret_cast<GlSymbol*>(&glGetTexImage),
		reinterpret_cast<GlSymbol*>(&glGetTexLevelParameterfv),
		reinterpret_cast<GlSymbol*>(&glGetTexLevelParameteriv),
		reinterpret_cast<GlSymbol*>(&glGetTexParameterIiv),
		reinterpret_cast<GlSymbol*>(&glGetTexParameterIuiv),
		reinterpret_cast<GlSymbol*>(&glGetTexParameterfv),
		reinterpret_cast<GlSymbol*>(&glGetTexParameteriv),
		reinterpret_cast<GlSymbol*>(&glGetTransformFeedbackVarying),
		reinterpret_cast<GlSymbol*>(&glGetUniformBlockIndex),
		reinterpret_cast<GlSymbol*>(&glGetUniformIndices),
		reinterpret_cast<GlSymbol*>(&glGetUniformLocation),
		reinterpret_cast<GlSymbol*>(&glGetUniformfv),
		reinterpret_cast<GlSymbol*>(&glGetUniformiv),
		reinterpret_cast<GlSymbol*>(&glGetUniformuiv),
		reinterpret_cast<GlSymbol*>(&glGetVertexAttribIiv),
		reinterpret_cast<GlSymbol*>(&glGetVertexAttribIuiv),
		reinterpret_cast<GlSymbol*>(&glGetVertexAttribPointerv),
		reinterpret_cast<GlSymbol*>(&glGetVertexAttribdv),
		reinterpret_cast<GlSymbol*>(&glGetVertexAttribfv),
		reinterpret_cast<GlSymbol*>(&glGetVertexAttribiv),
		reinterpret_cast<GlSymbol*>(&glHint),
		reinterpret_cast<GlSymbol*>(&glIsBuffer),
		reinterpret_cast<GlSymbol*>(&glIsEnabled),
		reinterpret_cast<GlSymbol*>(&glIsEnabledi),
		reinterpret_cast<GlSymbol*>(&glIsFramebuffer),
		reinterpret_cast<GlSymbol*>(&glIsProgram),
		reinterpret_cast<GlSymbol*>(&glIsQuery),
		reinterpret_cast<GlSymbol*>(&glIsRenderbuffer),
		reinterpret_cast<GlSymbol*>(&glIsShader),
		reinterpret_cast<GlSymbol*>(&glIsSync),
		reinterpret_cast<GlSymbol*>(&glIsTexture),
		reinterpret_cast<GlSymbol*>(&glIsVertexArray),
		reinterpret_cast<GlSymbol*>(&glLineWidth),
		reinterpret_cast<GlSymbol*>(&glLinkProgram),
		reinterpret_cast<GlSymbol*>(&glLogicOp),
		reinterpret_cast<GlSymbol*>(&glMapBuffer),
		reinterpret_cast<GlSymbol*>(&glMapBufferRange),
		reinterpret_cast<GlSymbol*>(&glMultiDrawArrays),
		reinterpret_cast<GlSymbol*>(&glMultiDrawElements),
		reinterpret_cast<GlSymbol*>(&glMultiDrawElementsBaseVertex),
		reinterpret_cast<GlSymbol*>(&glPixelStoref),
		reinterpret_cast<GlSymbol*>(&glPixelStorei),
		reinterpret_cast<GlSymbol*>(&glPointParameterf),
		reinterpret_cast<GlSymbol*>(&glPointParameterfv),
		reinterpret_cast<GlSymbol*>(&glPointParameteri),
		reinterpret_cast<GlSymbol*>(&glPointParameteriv),
		reinterpret_cast<GlSymbol*>(&glPointSize),
		reinterpret_cast<GlSymbol*>(&glPolygonMode),
		reinterpret_cast<GlSymbol*>(&glPolygonOffset),
		reinterpret_cast<GlSymbol*>(&glPrimitiveRestartIndex),
		reinterpret_cast<GlSymbol*>(&glProvokingVertex),
		reinterpret_cast<GlSymbol*>(&glReadBuffer),
		reinterpret_cast<GlSymbol*>(&glReadPixels),
		reinterpret_cast<GlSymbol*>(&glRenderbufferStorage),
		reinterpret_cast<GlSymbol*>(&glRenderbufferStorageMultisample),
		reinterpret_cast<GlSymbol*>(&glSampleCoverage),
		reinterpret_cast<GlSymbol*>(&glSampleMaski),
		reinterpret_cast<GlSymbol*>(&glScissor),
		reinterpret_cast<GlSymbol*>(&glShaderSource),
		reinterpret_cast<GlSymbol*>(&glStencilFunc),
		reinterpret_cast<GlSymbol*>(&glStencilFuncSeparate),
		reinterpret_cast<GlSymbol*>(&glStencilMask),
		reinterpret_cast<GlSymbol*>(&glStencilMaskSeparate),
		reinterpret_cast<GlSymbol*>(&glStencilOp),
		reinterpret_cast<GlSymbol*>(&glStencilOpSeparate),
		reinterpret_cast<GlSymbol*>(&glTexBuffer),
		reinterpret_cast<GlSymbol*>(&glTexImage1D),
		reinterpret_cast<GlSymbol*>(&glTexImage2D),
		reinterpret_cast<GlSymbol*>(&glTexImage2DMultisample),
		reinterpret_cast<GlSymbol*>(&glTexImage3D),
		reinterpret_cast<GlSymbol*>(&glTexImage3DMultisample),
		reinterpret_cast<GlSymbol*>(&glTexParameterIiv),
		reinterpret_cast<GlSymbol*>(&glTexParameterIuiv),
		reinterpret_cast<GlSymbol*>(&glTexParameterf),
		reinterpret_cast<GlSymbol*>(&glTexParameterfv),
		reinterpret_cast<GlSymbol*>(&glTexParameteri),
		reinterpret_cast<GlSymbol*>(&glTexParameteriv),
		reinterpret_cast<GlSymbol*>(&glTexSubImage1D),
		reinterpret_cast<GlSymbol*>(&glTexSubImage2D),
		reinterpret_cast<GlSymbol*>(&glTexSubImage3D),
		reinterpret_cast<GlSymbol*>(&glTransformFeedbackVaryings),
		reinterpret_cast<GlSymbol*>(&glUniform1f),
		reinterpret_cast<GlSymbol*>(&glUniform1fv),
		reinterpret_cast<GlSymbol*>(&glUniform1i),
		reinterpret_cast<GlSymbol*>(&glUniform1iv),
		reinterpret_cast<GlSymbol*>(&glUniform1ui),
		reinterpret_cast<GlSymbol*>(&glUniform1uiv),
		reinterpret_cast<GlSymbol*>(&glUniform2f),
		reinterpret_cast<GlSymbol*>(&glUniform2fv),
		reinterpret_cast<GlSymbol*>(&glUniform2i),
		reinterpret_cast<GlSymbol*>(&glUniform2iv),
		reinterpret_cast<GlSymbol*>(&glUniform2ui),
		reinterpret_cast<GlSymbol*>(&glUniform2uiv),
		reinterpret_cast<GlSymbol*>(&glUniform3f),
		reinterpret_cast<GlSymbol*>(&glUniform3fv),
		reinterpret_cast<GlSymbol*>(&glUniform3i),
		reinterpret_cast<GlSymbol*>(&glUniform3iv),
		reinterpret_cast<GlSymbol*>(&glUniform3ui),
		reinterpret_cast<GlSymbol*>(&glUniform3uiv),
		reinterpret_cast<GlSymbol*>(&glUniform4f),
		reinterpret_cast<GlSymbol*>(&glUniform4fv),
		reinterpret_cast<GlSymbol*>(&glUniform4i),
		reinterpret_cast<GlSymbol*>(&glUniform4iv),
		reinterpret_cast<GlSymbol*>(&glUniform4ui),
		reinterpret_cast<GlSymbol*>(&glUniform4uiv),
		reinterpret_cast<GlSymbol*>(&glUniformBlockBinding),
		reinterpret_cast<GlSymbol*>(&glUniformMatrix2fv),
		reinterpret_cast<GlSymbol*>(&glUniformMatrix2x3fv),
		reinterpret_cast<GlSymbol*>(&glUniformMatrix2x4fv),
		reinterpret_cast<GlSymbol*>(&glUniformMatrix3fv),
		reinterpret_cast<GlSymbol*>(&glUniformMatrix3x2fv),
		reinterpret_cast<GlSymbol*>(&glUniformMatrix3x4fv),
		reinterpret_cast<GlSymbol*>(&glUniformMatrix4fv),
		reinterpret_cast<GlSymbol*>(&glUniformMatrix4x2fv),
		reinterpret_cast<GlSymbol*>(&glUniformMatrix4x3fv),
		reinterpret_cast<GlSymbol*>(&glUnmapBuffer),
		reinterpret_cast<GlSymbol*>(&glUseProgram),
		reinterpret_cast<GlSymbol*>(&glValidateProgram),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib1d),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib1dv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib1f),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib1fv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib1s),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib1sv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib2d),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib2dv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib2f),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib2fv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib2s),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib2sv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib3d),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib3dv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib3f),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib3fv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib3s),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib3sv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4Nbv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4Niv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4Nsv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4Nub),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4Nubv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4Nuiv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4Nusv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4bv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4d),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4dv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4f),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4fv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4iv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4s),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4sv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4ubv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4uiv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4usv),
		reinterpret_cast<GlSymbol*>(&glVertexAttribI1i),
		reinterpret_cast<GlSymbol*>(&glVertexAttribI1iv),
		reinterpret_cast<GlSymbol*>(&glVertexAttribI1ui),
		reinterpret_cast<GlSymbol*>(&glVertexAttribI1uiv),
		reinterpret_cast<GlSymbol*>(&glVertexAttribI2i),
		reinterpret_cast<GlSymbol*>(&glVertexAttribI2iv),
		reinterpret_cast<GlSymbol*>(&glVertexAttribI2ui),
		reinterpret_cast<GlSymbol*>(&glVertexAttribI2uiv),
		reinterpret_cast<GlSymbol*>(&glVertexAttribI3i),
		reinterpret_cast<GlSymbol*>(&glVertexAttribI3iv),
		reinterpret_cast<GlSymbol*>(&glVertexAttribI3ui),
		reinterpret_cast<GlSymbol*>(&glVertexAttribI3uiv),
		reinterpret_cast<GlSymbol*>(&glVertexAttribI4bv),
		reinterpret_cast<GlSymbol*>(&glVertexAttribI4i),
		reinterpret_cast<GlSymbol*>(&glVertexAttribI4iv),
		reinterpret_cast<GlSymbol*>(&glVertexAttribI4sv),
		reinterpret_cast<GlSymbol*>(&glVertexAttribI4ubv),
		reinterpret_cast<GlSymbol*>(&glVertexAttribI4ui),
		reinterpret_cast<GlSymbol*>(&glVertexAttribI4uiv),
		reinterpret_cast<GlSymbol*>(&glVertexAttribI4usv),
		reinterpret_cast<GlSymbol*>(&glVertexAttribIPointer),
		reinterpret_cast<GlSymbol*>(&glVertexAttribPointer),
		reinterpret_cast<GlSymbol*>(&glViewport),
		reinterpret_cast<GlSymbol*>(&glWaitSync),
	};

	return gl_symbols;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlR3rExtensionMgrImpl::GlSymbolPtrs& GlR3rExtensionMgrImpl::get_es_v2_0_gl_symbol_ptrs()
try {
	static auto gl_symbols = GlSymbolPtrs
	{
		reinterpret_cast<GlSymbol*>(&glActiveTexture),
		reinterpret_cast<GlSymbol*>(&glAttachShader),
		reinterpret_cast<GlSymbol*>(&glBindAttribLocation),
		reinterpret_cast<GlSymbol*>(&glBindBuffer),
		reinterpret_cast<GlSymbol*>(&glBindFramebuffer),
		reinterpret_cast<GlSymbol*>(&glBindRenderbuffer),
		reinterpret_cast<GlSymbol*>(&glBindTexture),
		reinterpret_cast<GlSymbol*>(&glBlendColor),
		reinterpret_cast<GlSymbol*>(&glBlendEquation),
		reinterpret_cast<GlSymbol*>(&glBlendEquationSeparate),
		reinterpret_cast<GlSymbol*>(&glBlendFunc),
		reinterpret_cast<GlSymbol*>(&glBlendFuncSeparate),
		reinterpret_cast<GlSymbol*>(&glBufferData),
		reinterpret_cast<GlSymbol*>(&glBufferSubData),
		reinterpret_cast<GlSymbol*>(&glCheckFramebufferStatus),
		reinterpret_cast<GlSymbol*>(&glClear),
		reinterpret_cast<GlSymbol*>(&glClearColor),
		reinterpret_cast<GlSymbol*>(&glClearDepthf),
		reinterpret_cast<GlSymbol*>(&glClearStencil),
		reinterpret_cast<GlSymbol*>(&glColorMask),
		reinterpret_cast<GlSymbol*>(&glCompileShader),
		reinterpret_cast<GlSymbol*>(&glCompressedTexImage2D),
		reinterpret_cast<GlSymbol*>(&glCompressedTexSubImage2D),
		reinterpret_cast<GlSymbol*>(&glCopyTexImage2D),
		reinterpret_cast<GlSymbol*>(&glCopyTexSubImage2D),
		reinterpret_cast<GlSymbol*>(&glCreateProgram),
		reinterpret_cast<GlSymbol*>(&glCreateShader),
		reinterpret_cast<GlSymbol*>(&glCullFace),
		reinterpret_cast<GlSymbol*>(&glDeleteBuffers),
		reinterpret_cast<GlSymbol*>(&glDeleteFramebuffers),
		reinterpret_cast<GlSymbol*>(&glDeleteProgram),
		reinterpret_cast<GlSymbol*>(&glDeleteRenderbuffers),
		reinterpret_cast<GlSymbol*>(&glDeleteShader),
		reinterpret_cast<GlSymbol*>(&glDeleteTextures),
		reinterpret_cast<GlSymbol*>(&glDepthFunc),
		reinterpret_cast<GlSymbol*>(&glDepthMask),
		reinterpret_cast<GlSymbol*>(&glDepthRangef),
		reinterpret_cast<GlSymbol*>(&glDetachShader),
		reinterpret_cast<GlSymbol*>(&glDisable),
		reinterpret_cast<GlSymbol*>(&glDisableVertexAttribArray),
		reinterpret_cast<GlSymbol*>(&glDrawArrays),
		reinterpret_cast<GlSymbol*>(&glDrawElements),
		reinterpret_cast<GlSymbol*>(&glEnable),
		reinterpret_cast<GlSymbol*>(&glEnableVertexAttribArray),
		reinterpret_cast<GlSymbol*>(&glFinish),
		reinterpret_cast<GlSymbol*>(&glFlush),
		reinterpret_cast<GlSymbol*>(&glFramebufferRenderbuffer),
		reinterpret_cast<GlSymbol*>(&glFramebufferTexture2D),
		reinterpret_cast<GlSymbol*>(&glFrontFace),
		reinterpret_cast<GlSymbol*>(&glGenBuffers),
		reinterpret_cast<GlSymbol*>(&glGenFramebuffers),
		reinterpret_cast<GlSymbol*>(&glGenRenderbuffers),
		reinterpret_cast<GlSymbol*>(&glGenTextures),
		reinterpret_cast<GlSymbol*>(&glGenerateMipmap),
		reinterpret_cast<GlSymbol*>(&glGetActiveAttrib),
		reinterpret_cast<GlSymbol*>(&glGetActiveUniform),
		reinterpret_cast<GlSymbol*>(&glGetAttachedShaders),
		reinterpret_cast<GlSymbol*>(&glGetAttribLocation),
		reinterpret_cast<GlSymbol*>(&glGetBooleanv),
		reinterpret_cast<GlSymbol*>(&glGetBufferParameteriv),
		reinterpret_cast<GlSymbol*>(&glGetError),
		reinterpret_cast<GlSymbol*>(&glGetFloatv),
		reinterpret_cast<GlSymbol*>(&glGetFramebufferAttachmentParameteriv),
		reinterpret_cast<GlSymbol*>(&glGetIntegerv),
		reinterpret_cast<GlSymbol*>(&glGetProgramInfoLog),
		reinterpret_cast<GlSymbol*>(&glGetProgramiv),
		reinterpret_cast<GlSymbol*>(&glGetRenderbufferParameteriv),
		reinterpret_cast<GlSymbol*>(&glGetShaderInfoLog),
		reinterpret_cast<GlSymbol*>(&glGetShaderPrecisionFormat),
		reinterpret_cast<GlSymbol*>(&glGetShaderSource),
		reinterpret_cast<GlSymbol*>(&glGetShaderiv),
		reinterpret_cast<GlSymbol*>(&glGetString),
		reinterpret_cast<GlSymbol*>(&glGetTexParameterfv),
		reinterpret_cast<GlSymbol*>(&glGetTexParameteriv),
		reinterpret_cast<GlSymbol*>(&glGetUniformLocation),
		reinterpret_cast<GlSymbol*>(&glGetUniformfv),
		reinterpret_cast<GlSymbol*>(&glGetUniformiv),
		reinterpret_cast<GlSymbol*>(&glGetVertexAttribPointerv),
		reinterpret_cast<GlSymbol*>(&glGetVertexAttribfv),
		reinterpret_cast<GlSymbol*>(&glGetVertexAttribiv),
		reinterpret_cast<GlSymbol*>(&glHint),
		reinterpret_cast<GlSymbol*>(&glIsBuffer),
		reinterpret_cast<GlSymbol*>(&glIsEnabled),
		reinterpret_cast<GlSymbol*>(&glIsFramebuffer),
		reinterpret_cast<GlSymbol*>(&glIsProgram),
		reinterpret_cast<GlSymbol*>(&glIsRenderbuffer),
		reinterpret_cast<GlSymbol*>(&glIsShader),
		reinterpret_cast<GlSymbol*>(&glIsTexture),
		reinterpret_cast<GlSymbol*>(&glLineWidth),
		reinterpret_cast<GlSymbol*>(&glLinkProgram),
		reinterpret_cast<GlSymbol*>(&glPixelStorei),
		reinterpret_cast<GlSymbol*>(&glPolygonOffset),
		reinterpret_cast<GlSymbol*>(&glReadPixels),
		reinterpret_cast<GlSymbol*>(&glReleaseShaderCompiler),
		reinterpret_cast<GlSymbol*>(&glRenderbufferStorage),
		reinterpret_cast<GlSymbol*>(&glSampleCoverage),
		reinterpret_cast<GlSymbol*>(&glScissor),
		reinterpret_cast<GlSymbol*>(&glShaderBinary),
		reinterpret_cast<GlSymbol*>(&glShaderSource),
		reinterpret_cast<GlSymbol*>(&glStencilFunc),
		reinterpret_cast<GlSymbol*>(&glStencilFuncSeparate),
		reinterpret_cast<GlSymbol*>(&glStencilMask),
		reinterpret_cast<GlSymbol*>(&glStencilMaskSeparate),
		reinterpret_cast<GlSymbol*>(&glStencilOp),
		reinterpret_cast<GlSymbol*>(&glStencilOpSeparate),
		reinterpret_cast<GlSymbol*>(&glTexImage2D),
		reinterpret_cast<GlSymbol*>(&glTexParameterf),
		reinterpret_cast<GlSymbol*>(&glTexParameterfv),
		reinterpret_cast<GlSymbol*>(&glTexParameteri),
		reinterpret_cast<GlSymbol*>(&glTexParameteriv),
		reinterpret_cast<GlSymbol*>(&glTexSubImage2D),
		reinterpret_cast<GlSymbol*>(&glUniform1f),
		reinterpret_cast<GlSymbol*>(&glUniform1fv),
		reinterpret_cast<GlSymbol*>(&glUniform1i),
		reinterpret_cast<GlSymbol*>(&glUniform1iv),
		reinterpret_cast<GlSymbol*>(&glUniform2f),
		reinterpret_cast<GlSymbol*>(&glUniform2fv),
		reinterpret_cast<GlSymbol*>(&glUniform2i),
		reinterpret_cast<GlSymbol*>(&glUniform2iv),
		reinterpret_cast<GlSymbol*>(&glUniform3f),
		reinterpret_cast<GlSymbol*>(&glUniform3fv),
		reinterpret_cast<GlSymbol*>(&glUniform3i),
		reinterpret_cast<GlSymbol*>(&glUniform3iv),
		reinterpret_cast<GlSymbol*>(&glUniform4f),
		reinterpret_cast<GlSymbol*>(&glUniform4fv),
		reinterpret_cast<GlSymbol*>(&glUniform4i),
		reinterpret_cast<GlSymbol*>(&glUniform4iv),
		reinterpret_cast<GlSymbol*>(&glUniformMatrix2fv),
		reinterpret_cast<GlSymbol*>(&glUniformMatrix3fv),
		reinterpret_cast<GlSymbol*>(&glUniformMatrix4fv),
		reinterpret_cast<GlSymbol*>(&glUseProgram),
		reinterpret_cast<GlSymbol*>(&glValidateProgram),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib1f),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib1fv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib2f),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib2fv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib3f),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib3fv),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4f),
		reinterpret_cast<GlSymbol*>(&glVertexAttrib4fv),
		reinterpret_cast<GlSymbol*>(&glVertexAttribPointer),
		reinterpret_cast<GlSymbol*>(&glViewport),
	};

	return gl_symbols;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlR3rExtensionMgrImpl::GlSymbolPtrs& GlR3rExtensionMgrImpl::get_arb_buffer_storage_gl_symbol_ptrs()
try {
	static auto gl_symbols = GlSymbolPtrs{reinterpret_cast<GlSymbol*>(&glBufferStorage)};
	return gl_symbols;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlR3rExtensionMgrImpl::GlSymbolPtrs& GlR3rExtensionMgrImpl::get_arb_direct_state_access_gl_symbol_ptrs()
try {
	static auto gl_symbols = GlSymbolPtrs
	{
		reinterpret_cast<GlSymbol*>(&glBindTextureUnit),
		reinterpret_cast<GlSymbol*>(&glBlitNamedFramebuffer),
		reinterpret_cast<GlSymbol*>(&glCheckNamedFramebufferStatus),
		reinterpret_cast<GlSymbol*>(&glClearNamedBufferData),
		reinterpret_cast<GlSymbol*>(&glClearNamedBufferSubData),
		reinterpret_cast<GlSymbol*>(&glClearNamedFramebufferfi),
		reinterpret_cast<GlSymbol*>(&glClearNamedFramebufferfv),
		reinterpret_cast<GlSymbol*>(&glClearNamedFramebufferiv),
		reinterpret_cast<GlSymbol*>(&glClearNamedFramebufferuiv),
		reinterpret_cast<GlSymbol*>(&glCompressedTextureSubImage1D),
		reinterpret_cast<GlSymbol*>(&glCompressedTextureSubImage2D),
		reinterpret_cast<GlSymbol*>(&glCompressedTextureSubImage3D),
		reinterpret_cast<GlSymbol*>(&glCopyNamedBufferSubData),
		reinterpret_cast<GlSymbol*>(&glCopyTextureSubImage1D),
		reinterpret_cast<GlSymbol*>(&glCopyTextureSubImage2D),
		reinterpret_cast<GlSymbol*>(&glCopyTextureSubImage3D),
		reinterpret_cast<GlSymbol*>(&glCreateBuffers),
		reinterpret_cast<GlSymbol*>(&glCreateFramebuffers),
		reinterpret_cast<GlSymbol*>(&glCreateProgramPipelines),
		reinterpret_cast<GlSymbol*>(&glCreateQueries),
		reinterpret_cast<GlSymbol*>(&glCreateRenderbuffers),
		reinterpret_cast<GlSymbol*>(&glCreateSamplers),
		reinterpret_cast<GlSymbol*>(&glCreateTextures),
		reinterpret_cast<GlSymbol*>(&glCreateTransformFeedbacks),
		reinterpret_cast<GlSymbol*>(&glCreateVertexArrays),
		reinterpret_cast<GlSymbol*>(&glDisableVertexArrayAttrib),
		reinterpret_cast<GlSymbol*>(&glEnableVertexArrayAttrib),
		reinterpret_cast<GlSymbol*>(&glFlushMappedNamedBufferRange),
		reinterpret_cast<GlSymbol*>(&glGenerateTextureMipmap),
		reinterpret_cast<GlSymbol*>(&glGetCompressedTextureImage),
		reinterpret_cast<GlSymbol*>(&glGetNamedBufferParameteri64v),
		reinterpret_cast<GlSymbol*>(&glGetNamedBufferParameteriv),
		reinterpret_cast<GlSymbol*>(&glGetNamedBufferPointerv),
		reinterpret_cast<GlSymbol*>(&glGetNamedBufferSubData),
		reinterpret_cast<GlSymbol*>(&glGetNamedFramebufferAttachmentParameteriv),
		reinterpret_cast<GlSymbol*>(&glGetNamedFramebufferParameteriv),
		reinterpret_cast<GlSymbol*>(&glGetNamedRenderbufferParameteriv),
		reinterpret_cast<GlSymbol*>(&glGetQueryBufferObjecti64v),
		reinterpret_cast<GlSymbol*>(&glGetQueryBufferObjectiv),
		reinterpret_cast<GlSymbol*>(&glGetQueryBufferObjectui64v),
		reinterpret_cast<GlSymbol*>(&glGetQueryBufferObjectuiv),
		reinterpret_cast<GlSymbol*>(&glGetTextureImage),
		reinterpret_cast<GlSymbol*>(&glGetTextureLevelParameterfv),
		reinterpret_cast<GlSymbol*>(&glGetTextureLevelParameteriv),
		reinterpret_cast<GlSymbol*>(&glGetTextureParameterIiv),
		reinterpret_cast<GlSymbol*>(&glGetTextureParameterIuiv),
		reinterpret_cast<GlSymbol*>(&glGetTextureParameterfv),
		reinterpret_cast<GlSymbol*>(&glGetTextureParameteriv),
		reinterpret_cast<GlSymbol*>(&glGetTransformFeedbacki64_v),
		reinterpret_cast<GlSymbol*>(&glGetTransformFeedbacki_v),
		reinterpret_cast<GlSymbol*>(&glGetTransformFeedbackiv),
		reinterpret_cast<GlSymbol*>(&glGetVertexArrayIndexed64iv),
		reinterpret_cast<GlSymbol*>(&glGetVertexArrayIndexediv),
		reinterpret_cast<GlSymbol*>(&glGetVertexArrayiv),
		reinterpret_cast<GlSymbol*>(&glInvalidateNamedFramebufferData),
		reinterpret_cast<GlSymbol*>(&glInvalidateNamedFramebufferSubData),
		reinterpret_cast<GlSymbol*>(&glMapNamedBuffer),
		reinterpret_cast<GlSymbol*>(&glMapNamedBufferRange),
		reinterpret_cast<GlSymbol*>(&glNamedBufferData),
		reinterpret_cast<GlSymbol*>(&glNamedBufferStorage),
		reinterpret_cast<GlSymbol*>(&glNamedBufferSubData),
		reinterpret_cast<GlSymbol*>(&glNamedFramebufferDrawBuffer),
		reinterpret_cast<GlSymbol*>(&glNamedFramebufferDrawBuffers),
		reinterpret_cast<GlSymbol*>(&glNamedFramebufferParameteri),
		reinterpret_cast<GlSymbol*>(&glNamedFramebufferReadBuffer),
		reinterpret_cast<GlSymbol*>(&glNamedFramebufferRenderbuffer),
		reinterpret_cast<GlSymbol*>(&glNamedFramebufferTexture),
		reinterpret_cast<GlSymbol*>(&glNamedFramebufferTextureLayer),
		reinterpret_cast<GlSymbol*>(&glNamedRenderbufferStorage),
		reinterpret_cast<GlSymbol*>(&glNamedRenderbufferStorageMultisample),
		reinterpret_cast<GlSymbol*>(&glTextureBuffer),
		reinterpret_cast<GlSymbol*>(&glTextureBufferRange),
		reinterpret_cast<GlSymbol*>(&glTextureParameterIiv),
		reinterpret_cast<GlSymbol*>(&glTextureParameterIuiv),
		reinterpret_cast<GlSymbol*>(&glTextureParameterf),
		reinterpret_cast<GlSymbol*>(&glTextureParameterfv),
		reinterpret_cast<GlSymbol*>(&glTextureParameteri),
		reinterpret_cast<GlSymbol*>(&glTextureParameteriv),
		reinterpret_cast<GlSymbol*>(&glTextureStorage1D),
		reinterpret_cast<GlSymbol*>(&glTextureStorage2D),
		reinterpret_cast<GlSymbol*>(&glTextureStorage2DMultisample),
		reinterpret_cast<GlSymbol*>(&glTextureStorage3D),
		reinterpret_cast<GlSymbol*>(&glTextureStorage3DMultisample),
		reinterpret_cast<GlSymbol*>(&glTextureSubImage1D),
		reinterpret_cast<GlSymbol*>(&glTextureSubImage2D),
		reinterpret_cast<GlSymbol*>(&glTextureSubImage3D),
		reinterpret_cast<GlSymbol*>(&glTransformFeedbackBufferBase),
		reinterpret_cast<GlSymbol*>(&glTransformFeedbackBufferRange),
		reinterpret_cast<GlSymbol*>(&glUnmapNamedBuffer),
		reinterpret_cast<GlSymbol*>(&glVertexArrayAttribBinding),
		reinterpret_cast<GlSymbol*>(&glVertexArrayAttribFormat),
		reinterpret_cast<GlSymbol*>(&glVertexArrayAttribIFormat),
		reinterpret_cast<GlSymbol*>(&glVertexArrayAttribLFormat),
		reinterpret_cast<GlSymbol*>(&glVertexArrayBindingDivisor),
		reinterpret_cast<GlSymbol*>(&glVertexArrayElementBuffer),
		reinterpret_cast<GlSymbol*>(&glVertexArrayVertexBuffer),
		reinterpret_cast<GlSymbol*>(&glVertexArrayVertexBuffers),
	};

	return gl_symbols;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlR3rExtensionMgrImpl::GlSymbolPtrs& GlR3rExtensionMgrImpl::get_arb_framebuffer_object_gl_symbol_ptrs()
try {
	static auto gl_symbols = GlSymbolPtrs
	{
		reinterpret_cast<GlSymbol*>(&glBindFramebuffer),
		reinterpret_cast<GlSymbol*>(&glBindRenderbuffer),
		reinterpret_cast<GlSymbol*>(&glBlitFramebuffer),
		reinterpret_cast<GlSymbol*>(&glCheckFramebufferStatus),
		reinterpret_cast<GlSymbol*>(&glDeleteFramebuffers),
		reinterpret_cast<GlSymbol*>(&glDeleteRenderbuffers),
		reinterpret_cast<GlSymbol*>(&glFramebufferRenderbuffer),
		reinterpret_cast<GlSymbol*>(&glFramebufferTexture1D),
		reinterpret_cast<GlSymbol*>(&glFramebufferTexture2D),
		reinterpret_cast<GlSymbol*>(&glFramebufferTexture3D),
		reinterpret_cast<GlSymbol*>(&glFramebufferTextureLayer),
		reinterpret_cast<GlSymbol*>(&glGenFramebuffers),
		reinterpret_cast<GlSymbol*>(&glGenRenderbuffers),
		reinterpret_cast<GlSymbol*>(&glGenerateMipmap),
		reinterpret_cast<GlSymbol*>(&glGetFramebufferAttachmentParameteriv),
		reinterpret_cast<GlSymbol*>(&glGetRenderbufferParameteriv),
		reinterpret_cast<GlSymbol*>(&glIsFramebuffer),
		reinterpret_cast<GlSymbol*>(&glIsRenderbuffer),
		reinterpret_cast<GlSymbol*>(&glRenderbufferStorage),
		reinterpret_cast<GlSymbol*>(&glRenderbufferStorageMultisample),
	};

	return gl_symbols;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlR3rExtensionMgrImpl::GlSymbolPtrs& GlR3rExtensionMgrImpl::get_arb_sampler_objects_gl_symbol_ptrs()
try {
	static auto gl_symbols = GlSymbolPtrs
	{
		reinterpret_cast<GlSymbol*>(&glBindSampler),
		reinterpret_cast<GlSymbol*>(&glDeleteSamplers),
		reinterpret_cast<GlSymbol*>(&glGenSamplers),
		reinterpret_cast<GlSymbol*>(&glGetSamplerParameterIiv),
		reinterpret_cast<GlSymbol*>(&glGetSamplerParameterIuiv),
		reinterpret_cast<GlSymbol*>(&glGetSamplerParameterfv),
		reinterpret_cast<GlSymbol*>(&glGetSamplerParameteriv),
		reinterpret_cast<GlSymbol*>(&glIsSampler),
		reinterpret_cast<GlSymbol*>(&glSamplerParameterIiv),
		reinterpret_cast<GlSymbol*>(&glSamplerParameterIuiv),
		reinterpret_cast<GlSymbol*>(&glSamplerParameterf),
		reinterpret_cast<GlSymbol*>(&glSamplerParameterfv),
		reinterpret_cast<GlSymbol*>(&glSamplerParameteri),
		reinterpret_cast<GlSymbol*>(&glSamplerParameteriv),
	};

	return gl_symbols;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlR3rExtensionMgrImpl::GlSymbolPtrs& GlR3rExtensionMgrImpl::get_arb_separate_shader_objects_gl_symbol_ptrs()
try {
	static auto gl_symbols = GlSymbolPtrs
	{
		reinterpret_cast<GlSymbol*>(&glActiveShaderProgram),
		reinterpret_cast<GlSymbol*>(&glBindProgramPipeline),
		reinterpret_cast<GlSymbol*>(&glCreateShaderProgramv),
		reinterpret_cast<GlSymbol*>(&glDeleteProgramPipelines),
		reinterpret_cast<GlSymbol*>(&glGenProgramPipelines),
		reinterpret_cast<GlSymbol*>(&glGetProgramPipelineInfoLog),
		reinterpret_cast<GlSymbol*>(&glGetProgramPipelineiv),
		reinterpret_cast<GlSymbol*>(&glIsProgramPipeline),
		reinterpret_cast<GlSymbol*>(&glProgramParameteri),
		reinterpret_cast<GlSymbol*>(&glProgramUniform1d),
		reinterpret_cast<GlSymbol*>(&glProgramUniform1dv),
		reinterpret_cast<GlSymbol*>(&glProgramUniform1f),
		reinterpret_cast<GlSymbol*>(&glProgramUniform1fv),
		reinterpret_cast<GlSymbol*>(&glProgramUniform1i),
		reinterpret_cast<GlSymbol*>(&glProgramUniform1iv),
		reinterpret_cast<GlSymbol*>(&glProgramUniform1ui),
		reinterpret_cast<GlSymbol*>(&glProgramUniform1uiv),
		reinterpret_cast<GlSymbol*>(&glProgramUniform2d),
		reinterpret_cast<GlSymbol*>(&glProgramUniform2dv),
		reinterpret_cast<GlSymbol*>(&glProgramUniform2f),
		reinterpret_cast<GlSymbol*>(&glProgramUniform2fv),
		reinterpret_cast<GlSymbol*>(&glProgramUniform2i),
		reinterpret_cast<GlSymbol*>(&glProgramUniform2iv),
		reinterpret_cast<GlSymbol*>(&glProgramUniform2ui),
		reinterpret_cast<GlSymbol*>(&glProgramUniform2uiv),
		reinterpret_cast<GlSymbol*>(&glProgramUniform3d),
		reinterpret_cast<GlSymbol*>(&glProgramUniform3dv),
		reinterpret_cast<GlSymbol*>(&glProgramUniform3f),
		reinterpret_cast<GlSymbol*>(&glProgramUniform3fv),
		reinterpret_cast<GlSymbol*>(&glProgramUniform3i),
		reinterpret_cast<GlSymbol*>(&glProgramUniform3iv),
		reinterpret_cast<GlSymbol*>(&glProgramUniform3ui),
		reinterpret_cast<GlSymbol*>(&glProgramUniform3uiv),
		reinterpret_cast<GlSymbol*>(&glProgramUniform4d),
		reinterpret_cast<GlSymbol*>(&glProgramUniform4dv),
		reinterpret_cast<GlSymbol*>(&glProgramUniform4f),
		reinterpret_cast<GlSymbol*>(&glProgramUniform4fv),
		reinterpret_cast<GlSymbol*>(&glProgramUniform4i),
		reinterpret_cast<GlSymbol*>(&glProgramUniform4iv),
		reinterpret_cast<GlSymbol*>(&glProgramUniform4ui),
		reinterpret_cast<GlSymbol*>(&glProgramUniform4uiv),
		reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix2dv),
		reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix2fv),
		reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix2x3dv),
		reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix2x3fv),
		reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix2x4dv),
		reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix2x4fv),
		reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix3dv),
		reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix3fv),
		reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix3x2dv),
		reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix3x2fv),
		reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix3x4dv),
		reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix3x4fv),
		reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix4dv),
		reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix4fv),
		reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix4x2dv),
		reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix4x2fv),
		reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix4x3dv),
		reinterpret_cast<GlSymbol*>(&glProgramUniformMatrix4x3fv),
		reinterpret_cast<GlSymbol*>(&glUseProgramStages),
		reinterpret_cast<GlSymbol*>(&glValidateProgramPipeline),
	};

	return gl_symbols;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlR3rExtensionMgrImpl::GlSymbolPtrs& GlR3rExtensionMgrImpl::get_arb_vertex_array_object_gl_symbol_ptrs()
try {
	static auto gl_symbols = GlSymbolPtrs
	{
		reinterpret_cast<GlSymbol*>(&glBindVertexArray),
		reinterpret_cast<GlSymbol*>(&glDeleteVertexArrays),
		reinterpret_cast<GlSymbol*>(&glGenVertexArrays),
		reinterpret_cast<GlSymbol*>(&glIsVertexArray),
	};

	return gl_symbols;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlR3rExtensionMgrImpl::GlSymbolPtrs& GlR3rExtensionMgrImpl::get_ext_framebuffer_blit_gl_symbol_ptrs()
try {
	static auto gl_symbols = GlSymbolPtrs{reinterpret_cast<GlSymbol*>(&glBlitFramebufferEXT)};
	return gl_symbols;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlR3rExtensionMgrImpl::GlSymbolPtrs& GlR3rExtensionMgrImpl::get_ext_framebuffer_multisample_gl_symbol_ptrs()
try {
	static auto gl_symbols = GlSymbolPtrs{reinterpret_cast<GlSymbol*>(&glRenderbufferStorageMultisampleEXT)};
	return gl_symbols;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlR3rExtensionMgrImpl::GlSymbolPtrs& GlR3rExtensionMgrImpl::get_ext_framebuffer_object_gl_symbol_ptrs()
try {
	static auto gl_symbols = GlSymbolPtrs
	{
		reinterpret_cast<GlSymbol*>(&glBindFramebufferEXT),
		reinterpret_cast<GlSymbol*>(&glBindRenderbufferEXT),
		reinterpret_cast<GlSymbol*>(&glCheckFramebufferStatusEXT),
		reinterpret_cast<GlSymbol*>(&glDeleteFramebuffersEXT),
		reinterpret_cast<GlSymbol*>(&glDeleteRenderbuffersEXT),
		reinterpret_cast<GlSymbol*>(&glFramebufferRenderbufferEXT),
		reinterpret_cast<GlSymbol*>(&glFramebufferTexture1DEXT),
		reinterpret_cast<GlSymbol*>(&glFramebufferTexture2DEXT),
		reinterpret_cast<GlSymbol*>(&glFramebufferTexture3DEXT),
		reinterpret_cast<GlSymbol*>(&glGenFramebuffersEXT),
		reinterpret_cast<GlSymbol*>(&glGenRenderbuffersEXT),
		reinterpret_cast<GlSymbol*>(&glGenerateMipmapEXT),
		reinterpret_cast<GlSymbol*>(&glGetFramebufferAttachmentParameterivEXT),
		reinterpret_cast<GlSymbol*>(&glGetRenderbufferParameterivEXT),
		reinterpret_cast<GlSymbol*>(&glIsFramebufferEXT),
		reinterpret_cast<GlSymbol*>(&glIsRenderbufferEXT),
		reinterpret_cast<GlSymbol*>(&glRenderbufferStorageEXT),
	};

	return gl_symbols;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const std::string& GlR3rExtensionMgrImpl::get_empty_extension_name() noexcept
{
	static const auto result = std::string{};
	return result;
}

int GlR3rExtensionMgrImpl::get_registered_extension_count() noexcept
{
	return static_cast<int>(GlR3rExtensionId::count_);
}

int GlR3rExtensionMgrImpl::get_extension_index(GlR3rExtensionId extension_id) noexcept
{
	const auto extension_index = static_cast<int>(extension_id);

	if (extension_index < 0 || extension_index >= get_registered_extension_count())
	{
		return -1;
	}

	return extension_index;
}

void GlR3rExtensionMgrImpl::initialize_registry()
try {
	registry_.clear();
	registry_.resize(static_cast<int>(GlR3rExtensionId::count_));

	{
		auto& registry_item = registry_[static_cast<int>(GlR3rExtensionId::essentials)];
		registry_item.is_virtual = true;
		registry_item.is_probed = false;
		registry_item.is_available = false;
		registry_item.extension_name = "essentials";
		registry_item.gl_symbol_ptrs = &get_essentials_gl_symbol_ptrs();
	}

	{
		auto& registry_item = registry_[static_cast<int>(GlR3rExtensionId::v2_0)];
		registry_item.is_virtual = true;
		registry_item.is_probed = false;
		registry_item.is_available = false;
		registry_item.extension_name = "v2.0";
		registry_item.gl_symbol_ptrs = &get_v2_0_gl_symbol_ptrs();
	}

	{
		auto& registry_item = registry_[static_cast<int>(GlR3rExtensionId::v3_2_core)];
		registry_item.is_virtual = true;
		registry_item.is_probed = false;
		registry_item.is_available = false;
		registry_item.extension_name = "v3.2 core";
		registry_item.gl_symbol_ptrs = &get_v3_2_core_gl_symbol_ptrs();
	}

	{
		auto& registry_item = registry_[static_cast<int>(GlR3rExtensionId::es_v2_0)];
		registry_item.is_virtual = true;
		registry_item.is_probed = false;
		registry_item.is_available = false;
		registry_item.extension_name = "ES v2.0";
		registry_item.gl_symbol_ptrs = &get_es_v2_0_gl_symbol_ptrs();
	}

	{
		auto& registry_item = registry_[static_cast<int>(GlR3rExtensionId::arb_buffer_storage)];
		registry_item.is_virtual = false;
		registry_item.is_probed = false;
		registry_item.is_available = false;
		registry_item.extension_name = "GL_ARB_buffer_storage";
		registry_item.gl_symbol_ptrs = &get_arb_buffer_storage_gl_symbol_ptrs();
	}

	{
		auto& registry_item = registry_[static_cast<int>(GlR3rExtensionId::arb_direct_state_access)];
		registry_item.is_virtual = false;
		registry_item.is_probed = false;
		registry_item.is_available = false;
		registry_item.extension_name = "GL_ARB_direct_state_access";
		registry_item.gl_symbol_ptrs = &get_arb_direct_state_access_gl_symbol_ptrs();
	}

	{
		auto& registry_item = registry_[static_cast<int>(GlR3rExtensionId::arb_framebuffer_object)];
		registry_item.is_virtual = false;
		registry_item.is_probed = false;
		registry_item.is_available = false;
		registry_item.extension_name = "GL_ARB_framebuffer_object";
		registry_item.gl_symbol_ptrs = &get_arb_framebuffer_object_gl_symbol_ptrs();
	}

	{
		auto& registry_item = registry_[static_cast<int>(GlR3rExtensionId::arb_texture_filter_anisotropic)];
		registry_item.is_virtual = false;
		registry_item.is_probed = false;
		registry_item.is_available = false;
		registry_item.extension_name = "GL_ARB_texture_filter_anisotropic";
		registry_item.gl_symbol_ptrs = nullptr;
	}

	{
		auto& registry_item = registry_[static_cast<int>(GlR3rExtensionId::arb_texture_non_power_of_two)];
		registry_item.is_virtual = false;
		registry_item.is_probed = false;
		registry_item.is_available = false;
		registry_item.extension_name = "GL_ARB_texture_non_power_of_two";
		registry_item.gl_symbol_ptrs = nullptr;
	}

	{
		auto& registry_item = registry_[static_cast<int>(GlR3rExtensionId::arb_sampler_objects)];
		registry_item.is_virtual = false;
		registry_item.is_probed = false;
		registry_item.is_available = false;
		registry_item.extension_name = "GL_ARB_sampler_objects";
		registry_item.gl_symbol_ptrs = &get_arb_sampler_objects_gl_symbol_ptrs();
	}

	{
		auto& registry_item = registry_[static_cast<int>(GlR3rExtensionId::arb_separate_shader_objects)];
		registry_item.is_virtual = false;
		registry_item.is_probed = false;
		registry_item.is_available = false;
		registry_item.extension_name = "GL_ARB_separate_shader_objects";
		registry_item.gl_symbol_ptrs = &get_arb_separate_shader_objects_gl_symbol_ptrs();
	}

	{
		auto& registry_item = registry_[static_cast<int>(GlR3rExtensionId::arb_vertex_array_object)];
		registry_item.is_virtual = false;
		registry_item.is_probed = false;
		registry_item.is_available = false;
		registry_item.extension_name = "GL_ARB_vertex_array_object";
		registry_item.gl_symbol_ptrs = &get_arb_vertex_array_object_gl_symbol_ptrs();
	}

	{
		auto& registry_item = registry_[static_cast<int>(GlR3rExtensionId::ext_framebuffer_object)];
		registry_item.is_virtual = false;
		registry_item.is_probed = false;
		registry_item.is_available = false;
		registry_item.extension_name = "GL_EXT_framebuffer_object";
		registry_item.gl_symbol_ptrs = &get_ext_framebuffer_object_gl_symbol_ptrs();
	}

	{
		auto& registry_item = registry_[static_cast<int>(GlR3rExtensionId::ext_framebuffer_blit)];
		registry_item.is_virtual = false;
		registry_item.is_probed = false;
		registry_item.is_available = false;
		registry_item.extension_name = "GL_EXT_framebuffer_blit";
		registry_item.gl_symbol_ptrs = &get_ext_framebuffer_blit_gl_symbol_ptrs();
	}

	{
		auto& registry_item = registry_[static_cast<int>(GlR3rExtensionId::ext_framebuffer_multisample)];
		registry_item.is_virtual = false;
		registry_item.is_probed = false;
		registry_item.is_available = false;
		registry_item.extension_name = "GL_EXT_framebuffer_multisample";
		registry_item.gl_symbol_ptrs = &get_ext_framebuffer_multisample_gl_symbol_ptrs();
	}

	{
		auto& registry_item = registry_[static_cast<int>(GlR3rExtensionId::ext_packed_depth_stencil)];
		registry_item.is_virtual = false;
		registry_item.is_probed = false;
		registry_item.is_available = false;
		registry_item.extension_name = "GL_EXT_packed_depth_stencil";
		registry_item.gl_symbol_ptrs = nullptr;
	}

	{
		auto& registry_item = registry_[static_cast<int>(GlR3rExtensionId::ext_texture_filter_anisotropic)];
		registry_item.is_virtual = false;
		registry_item.is_probed = false;
		registry_item.is_available = false;
		registry_item.extension_name = "GL_EXT_texture_filter_anisotropic";
		registry_item.gl_symbol_ptrs = nullptr;
	}

	{
		auto& registry_item = registry_[static_cast<int>(GlR3rExtensionId::oes_texture_npot)];
		registry_item.is_virtual = false;
		registry_item.is_probed = false;
		registry_item.is_available = false;
		registry_item.extension_name = "GL_OES_texture_npot";
		registry_item.gl_symbol_ptrs = nullptr;
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

int GlR3rExtensionMgrImpl::parse_version_number(const std::string& string)
try {
	if (string.empty())
	{
		BSTONE_THROW_STATIC_SOURCE("Expected a digit.");
	}

	constexpr auto max_digit_count = 10;

	auto number = 0;
	auto digit_count = 0;

	for (const auto ch : string)
	{
		++digit_count;

		if (digit_count > max_digit_count)
		{
			BSTONE_THROW_STATIC_SOURCE("Too many digits.");
		}

		switch (ch)
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				number = (number * 10) + (ch - '0');
				break;

			default:
				BSTONE_THROW_STATIC_SOURCE("Non-digit character.");
		}
	}

	return number;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rExtensionMgrImpl::parse_version(const std::string& version_string, GlR3rVersion& gl_version)
try {
	static const auto digits = std::string{"0123456789"};
	static const auto gles_prefix = std::string{"OpenGL ES "};

	const auto digits_pos = version_string.find_first_of(digits);

	if (digits_pos == std::string::npos)
	{
		BSTONE_THROW_STATIC_SOURCE("Unsupported format.");
	}

	if (digits_pos > 0)
	{
		if (version_string.find(gles_prefix) == 0)
		{
			gl_version.is_es = true;
		}
		else
		{
			BSTONE_THROW_STATIC_SOURCE("Unsupported prefix.");
		}
	}

	const auto vendor_info_pos = version_string.find(' ', digits_pos);
	const auto has_vendor = (vendor_info_pos != std::string::npos);

	const auto first_dot_pos = version_string.find('.', digits_pos);

	if (first_dot_pos == std::string::npos)
	{
		BSTONE_THROW_STATIC_SOURCE("Expected the dot separator.");
	}

	const auto second_dot_pos = version_string.rfind('.', vendor_info_pos);
	const auto has_release = (first_dot_pos != second_dot_pos);

	// Major.
	//
	const auto major_version_string_view = version_string.substr(digits_pos, first_dot_pos - digits_pos);
	gl_version.major = parse_version_number(major_version_string_view);

	// Minor.
	//
	const auto minor_begin_pos = first_dot_pos + 1;

	const auto minor_end_pos =
		has_release ?
		second_dot_pos :
		(has_vendor ? vendor_info_pos : static_cast<int>(version_string.size()));

	const auto minor_version_string_view = version_string.substr(
		minor_begin_pos,
		minor_end_pos - minor_begin_pos);

	gl_version.minor = parse_version_number(minor_version_string_view);

	// Release.
	//
	if (has_release)
	{
		const auto release_begin_pos = second_dot_pos + 1;

		const auto release_version_str_view = version_string.substr(
			release_begin_pos,
			vendor_info_pos - release_begin_pos);

		gl_version.release = parse_version_number(release_version_str_view);
	}

	// Vendor information.
	//
	if (has_vendor)
	{
		gl_version.vendor = version_string.substr(vendor_info_pos + 1);
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rExtensionMgrImpl::get_version(GLenum version_enum, GlR3rVersion& gl_version)
try {
	if (glGetString == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null \"glGetString\".");
	}

	const auto gl_version_string = reinterpret_cast<const char*>(glGetString(version_enum));

	if (gl_version_string == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null version string.");
	}

	const auto version_string = std::string{gl_version_string};
	parse_version(version_string, gl_version);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rExtensionMgrImpl::get_version()
try {
	get_version(GL_VERSION, gl_version_);
	get_version(GL_SHADING_LANGUAGE_VERSION, glsl_version_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rExtensionMgrImpl::get_names_from_multiple_strings()
try {
	if (glGetIntegerv == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null \"glGetIntegerv\".");
	}

	if (glGetStringi == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null \"glGetStringi\".");
	}

	auto gl_extension_count = GLint{};

	glGetIntegerv(GL_NUM_EXTENSIONS, &gl_extension_count);
	GlR3rError::check_optionally();

	if (gl_extension_count == 0)
	{
		return;
	}

	extension_names_.resize(gl_extension_count);

	for (auto i = GLint{}; i < gl_extension_count; ++i)
	{
		const auto extension_name = glGetStringi(GL_EXTENSIONS, static_cast<GLuint>(i));

		if (extension_name == nullptr)
		{
			BSTONE_THROW_STATIC_SOURCE("Null extension name.");
		}

		extension_names_[i] = reinterpret_cast<const char*>(extension_name);
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rExtensionMgrImpl::get_names_from_one_string()
try {
	if (glGetString == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null \"glGetString\".");
	}

	const auto gl_extensions_c_string = glGetString(GL_EXTENSIONS);
	GlR3rError::check_optionally();

	if (gl_extensions_c_string == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null extensions string.");
	}

	const auto gl_extensions_std_string = std::string
	{
		reinterpret_cast<const char*>(gl_extensions_c_string)
	};

	const auto extension_count = 1 + std::count(
		gl_extensions_std_string.cbegin(),
		gl_extensions_std_string.cend(),
		' ');

	auto iss = std::istringstream{gl_extensions_std_string};
	extension_names_.reserve(extension_count);

	extension_names_.assign(
		std::istream_iterator<std::string>{iss},
		std::istream_iterator<std::string>{});
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rExtensionMgrImpl::get_names()
try {
	if (gl_version_.major >= 3)
	{
		get_names_from_multiple_strings();
	}
	else
	{
		get_names_from_one_string();
	}

	std::sort(extension_names_.begin(), extension_names_.end());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void GlR3rExtensionMgrImpl::probe_generic(GlR3rExtensionId extension_id)
try {
	const auto extension_index = get_extension_index(extension_id);

	if (extension_index < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid extension id.");
	}

	auto& registry_item = registry_[extension_index];

	if (registry_item.is_probed)
	{
		return;
	}

	registry_item.is_probed = true;

	if (registry_item.is_virtual && registry_item.gl_symbol_ptrs == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Expected symbol pointer for virtual extension.");
	}

	if (!registry_item.is_virtual)
	{
		const auto has_extension_name = std::any_of(
			extension_names_.cbegin(),
			extension_names_.cend(),
			[&registry_item](const std::string& item)
			{
				return item == registry_item.extension_name;
			}
		);

		if (!has_extension_name)
		{
			return;
		}
	}

	if (registry_item.gl_symbol_ptrs != nullptr)
	{
		const auto has_symbols = has_gl_symbol(*registry_item.gl_symbol_ptrs);

		if (!has_symbols)
		{
			return;
		}
	}

	registry_item.is_available = true;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace

// ==========================================================================

GlR3rExtensionMgrUPtr make_gl_r3r_extension_mgr(const sys::GlSymbolResolver& symbol_resolver)
{
	return std::make_unique<GlR3rExtensionMgrImpl>(symbol_resolver);
}

} // namespace bstone
