/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


//
// OpenGL extension manager (implementation).
//
// !!! Internal usage only !!!
//


#include "bstone_detail_ren_3d_gl_extension_mgr.h"

#include <cassert>

#include <algorithm>
#include <iterator>
#include <sstream>
#include <unordered_map>

#include "bstone_exception.h"
#include "bstone_ren_3d_tests.h"
#include "bstone_sdl2_types.h"

#include "bstone_detail_ren_3d_gl_error.h"
#include "bstone_detail_ren_3d_gl_utils.h"


namespace bstone
{
namespace detail
{


class Ren3dGlExtensionMgrImplGlVersionException :
	public Exception
{
public:
	explicit Ren3dGlExtensionMgrImplGlVersionException(
		const char* const message)
		:
		Exception{std::string{"[GL_VER] "} + message}
	{
	}
}; // Ren3dGlExtensionMgrImplGlVersionException

class Ren3dGlExtensionMgrImplException :
	public Exception
{
public:
	explicit Ren3dGlExtensionMgrImplException(
		const char* const message)
		:
		Exception{std::string{"[GL_EXT_MGR] "} + message}
	{
	}
}; // Ren3dGlExtensionMgrImplException

class Ren3dGlExtensionMgrImplMissingSymbolException :
	public Exception
{
public:
	explicit Ren3dGlExtensionMgrImplMissingSymbolException(
		const char* const symbol_name)
		:
		Exception{std::string{"[GL_EXT_MGR] ["} + symbol_name + "] Symbol not found."}
	{
	}
}; // Ren3dGlExtensionMgrImplMissingSymbolException


// ==========================================================================
// Ren3dGlExtensionMgrImpl
//

class Ren3dGlExtensionMgrImpl final :
	public Ren3dGlExtensionMgr
{
public:
	Ren3dGlExtensionMgrImpl();

	~Ren3dGlExtensionMgrImpl() override;


	int get_count() const noexcept override;

	const std::string& get_name(
		const int extension_index) const noexcept override;

	const Ren3dGlVersion& get_gl_version() const noexcept override;

	const Ren3dGlVersion& get_glsl_version() const noexcept override;


	void probe(
		const Ren3dGlExtensionId extension_id) override;


	bool has(
		const Ren3dGlExtensionId extension_id) const noexcept override;

	bool operator[](
		const Ren3dGlExtensionId extension_id) const noexcept override;


private:
	using ExtensionNames = std::vector<std::string>;

	using GlSymbolPtrs = std::vector<void**>;
	using GlSymbolRegistry = std::unordered_map<void**, const char*>;

	struct RegistryItem
	{
		bool is_virtual_;
		bool is_probed_;
		bool is_available_;

		std::string extension_name_;

		GlSymbolPtrs* gl_symbol_ptrs_;
	}; // RegistryItem

	using Registry = std::vector<RegistryItem>;


	Ren3dGlContextKind context_kind_;
	ExtensionNames extension_names_;
	Registry registry_;
	Ren3dGlVersion gl_version_;
	Ren3dGlVersion glsl_version_;


	static GlSymbolRegistry& get_gl_symbol_registry();

	static void clear_gl_symbols();

	static void resolve_gl_symbols();


	static bool has_gl_symbol(
		const GlSymbolPtrs& gl_symbol_ptrs);


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

	static int get_extension_index(
		const Ren3dGlExtensionId extension_id) noexcept;


	void initialize_registry();


	int parse_version_number(
		const std::string& string);

	void parse_version(
		const std::string& version_string,
		Ren3dGlVersion& gl_version);

	void get_version(
		const GLenum version_enum,
		Ren3dGlVersion& gl_version);

	void get_version();


	void get_context_attributes();


	void get_names_from_multiple_strings();

	void get_names_from_one_string();

	void get_names();


	void probe_generic(
		const Ren3dGlExtensionId extension_id);
}; // Ren3dGlExtensionMgrImpl


Ren3dGlExtensionMgrImpl::Ren3dGlExtensionMgrImpl()
	:
	context_kind_{},
	extension_names_{},
	registry_{},
	gl_version_{},
	glsl_version_{}
{
	clear_gl_symbols();
	resolve_gl_symbols();

	get_version();
	get_context_attributes();
	get_names();

	initialize_registry();
}

Ren3dGlExtensionMgrImpl::~Ren3dGlExtensionMgrImpl() = default;

int Ren3dGlExtensionMgrImpl::get_count() const noexcept
{
	return static_cast<int>(extension_names_.size());
}

const std::string& Ren3dGlExtensionMgrImpl::get_name(
	const int extension_index) const noexcept
{
	if (extension_index < 0 || extension_index >= get_count())
	{
		return get_empty_extension_name();
	}

	return extension_names_[extension_index];
}

const Ren3dGlVersion& Ren3dGlExtensionMgrImpl::get_gl_version() const noexcept
{
	return gl_version_;
}

const Ren3dGlVersion& Ren3dGlExtensionMgrImpl::get_glsl_version() const noexcept
{
	return glsl_version_;
}

void Ren3dGlExtensionMgrImpl::probe(
	const Ren3dGlExtensionId extension_id)
{
	probe_generic(extension_id);
}

bool Ren3dGlExtensionMgrImpl::has(
	const Ren3dGlExtensionId extension_id) const noexcept
{
	const auto extension_index = get_extension_index(extension_id);

	if (extension_index < 0)
	{
		return false;
	}

	return registry_[extension_index].is_available_;
}

bool Ren3dGlExtensionMgrImpl::operator[](
	const Ren3dGlExtensionId extension_id) const noexcept
{
	return has(extension_id);
}

Ren3dGlExtensionMgrImpl::GlSymbolRegistry& Ren3dGlExtensionMgrImpl::get_gl_symbol_registry()
{
	static auto gl_symbol_registry = GlSymbolRegistry
	{
		{reinterpret_cast<void**>(&glAccum), "glAccum"},
		{reinterpret_cast<void**>(&glActiveShaderProgram), "glActiveShaderProgram"},
		{reinterpret_cast<void**>(&glActiveTexture), "glActiveTexture"},
		{reinterpret_cast<void**>(&glAlphaFunc), "glAlphaFunc"},
		{reinterpret_cast<void**>(&glAreTexturesResident), "glAreTexturesResident"},
		{reinterpret_cast<void**>(&glArrayElement), "glArrayElement"},
		{reinterpret_cast<void**>(&glAttachShader), "glAttachShader"},
		{reinterpret_cast<void**>(&glBeginConditionalRender), "glBeginConditionalRender"},
		{reinterpret_cast<void**>(&glBegin), "glBegin"},
		{reinterpret_cast<void**>(&glBeginQuery), "glBeginQuery"},
		{reinterpret_cast<void**>(&glBeginTransformFeedback), "glBeginTransformFeedback"},
		{reinterpret_cast<void**>(&glBindAttribLocation), "glBindAttribLocation"},
		{reinterpret_cast<void**>(&glBindBufferARB), "glBindBufferARB"},
		{reinterpret_cast<void**>(&glBindBufferBase), "glBindBufferBase"},
		{reinterpret_cast<void**>(&glBindBuffer), "glBindBuffer"},
		{reinterpret_cast<void**>(&glBindBufferRange), "glBindBufferRange"},
		{reinterpret_cast<void**>(&glBindFragDataLocation), "glBindFragDataLocation"},
		{reinterpret_cast<void**>(&glBindFramebufferEXT), "glBindFramebufferEXT"},
		{reinterpret_cast<void**>(&glBindFramebuffer), "glBindFramebuffer"},
		{reinterpret_cast<void**>(&glBindProgramPipeline), "glBindProgramPipeline"},
		{reinterpret_cast<void**>(&glBindRenderbufferEXT), "glBindRenderbufferEXT"},
		{reinterpret_cast<void**>(&glBindRenderbuffer), "glBindRenderbuffer"},
		{reinterpret_cast<void**>(&glBindSampler), "glBindSampler"},
		{reinterpret_cast<void**>(&glBindTexture), "glBindTexture"},
		{reinterpret_cast<void**>(&glBindTextureUnit), "glBindTextureUnit"},
		{reinterpret_cast<void**>(&glBindVertexArray), "glBindVertexArray"},
		{reinterpret_cast<void**>(&glBitmap), "glBitmap"},
		{reinterpret_cast<void**>(&glBlendColor), "glBlendColor"},
		{reinterpret_cast<void**>(&glBlendEquation), "glBlendEquation"},
		{reinterpret_cast<void**>(&glBlendEquationSeparate), "glBlendEquationSeparate"},
		{reinterpret_cast<void**>(&glBlendFunc), "glBlendFunc"},
		{reinterpret_cast<void**>(&glBlendFuncSeparate), "glBlendFuncSeparate"},
		{reinterpret_cast<void**>(&glBlitFramebufferEXT), "glBlitFramebufferEXT"},
		{reinterpret_cast<void**>(&glBlitFramebuffer), "glBlitFramebuffer"},
		{reinterpret_cast<void**>(&glBlitNamedFramebuffer), "glBlitNamedFramebuffer"},
		{reinterpret_cast<void**>(&glBufferDataARB), "glBufferDataARB"},
		{reinterpret_cast<void**>(&glBufferData), "glBufferData"},
		{reinterpret_cast<void**>(&glBufferStorage), "glBufferStorage"},
		{reinterpret_cast<void**>(&glBufferSubDataARB), "glBufferSubDataARB"},
		{reinterpret_cast<void**>(&glBufferSubData), "glBufferSubData"},
		{reinterpret_cast<void**>(&glCallList), "glCallList"},
		{reinterpret_cast<void**>(&glCallLists), "glCallLists"},
		{reinterpret_cast<void**>(&glCheckFramebufferStatusEXT), "glCheckFramebufferStatusEXT"},
		{reinterpret_cast<void**>(&glCheckFramebufferStatus), "glCheckFramebufferStatus"},
		{reinterpret_cast<void**>(&glCheckNamedFramebufferStatus), "glCheckNamedFramebufferStatus"},
		{reinterpret_cast<void**>(&glClampColorARB), "glClampColorARB"},
		{reinterpret_cast<void**>(&glClampColor), "glClampColor"},
		{reinterpret_cast<void**>(&glClearAccum), "glClearAccum"},
		{reinterpret_cast<void**>(&glClearBufferfi), "glClearBufferfi"},
		{reinterpret_cast<void**>(&glClearBufferfv), "glClearBufferfv"},
		{reinterpret_cast<void**>(&glClearBufferiv), "glClearBufferiv"},
		{reinterpret_cast<void**>(&glClearBufferuiv), "glClearBufferuiv"},
		{reinterpret_cast<void**>(&glClearColor), "glClearColor"},
		{reinterpret_cast<void**>(&glClearDepthf), "glClearDepthf"},
		{reinterpret_cast<void**>(&glClearDepth), "glClearDepth"},
		{reinterpret_cast<void**>(&glClearIndex), "glClearIndex"},
		{reinterpret_cast<void**>(&glClearNamedBufferData), "glClearNamedBufferData"},
		{reinterpret_cast<void**>(&glClearNamedBufferSubData), "glClearNamedBufferSubData"},
		{reinterpret_cast<void**>(&glClearNamedFramebufferfi), "glClearNamedFramebufferfi"},
		{reinterpret_cast<void**>(&glClearNamedFramebufferfv), "glClearNamedFramebufferfv"},
		{reinterpret_cast<void**>(&glClearNamedFramebufferiv), "glClearNamedFramebufferiv"},
		{reinterpret_cast<void**>(&glClearNamedFramebufferuiv), "glClearNamedFramebufferuiv"},
		{reinterpret_cast<void**>(&glClear), "glClear"},
		{reinterpret_cast<void**>(&glClearStencil), "glClearStencil"},
		{reinterpret_cast<void**>(&glClientActiveTexture), "glClientActiveTexture"},
		{reinterpret_cast<void**>(&glClientWaitSync), "glClientWaitSync"},
		{reinterpret_cast<void**>(&glClipPlane), "glClipPlane"},
		{reinterpret_cast<void**>(&glColor3b), "glColor3b"},
		{reinterpret_cast<void**>(&glColor3bv), "glColor3bv"},
		{reinterpret_cast<void**>(&glColor3d), "glColor3d"},
		{reinterpret_cast<void**>(&glColor3dv), "glColor3dv"},
		{reinterpret_cast<void**>(&glColor3f), "glColor3f"},
		{reinterpret_cast<void**>(&glColor3fv), "glColor3fv"},
		{reinterpret_cast<void**>(&glColor3i), "glColor3i"},
		{reinterpret_cast<void**>(&glColor3iv), "glColor3iv"},
		{reinterpret_cast<void**>(&glColor3s), "glColor3s"},
		{reinterpret_cast<void**>(&glColor3sv), "glColor3sv"},
		{reinterpret_cast<void**>(&glColor3ub), "glColor3ub"},
		{reinterpret_cast<void**>(&glColor3ubv), "glColor3ubv"},
		{reinterpret_cast<void**>(&glColor3ui), "glColor3ui"},
		{reinterpret_cast<void**>(&glColor3uiv), "glColor3uiv"},
		{reinterpret_cast<void**>(&glColor3us), "glColor3us"},
		{reinterpret_cast<void**>(&glColor3usv), "glColor3usv"},
		{reinterpret_cast<void**>(&glColor4b), "glColor4b"},
		{reinterpret_cast<void**>(&glColor4bv), "glColor4bv"},
		{reinterpret_cast<void**>(&glColor4d), "glColor4d"},
		{reinterpret_cast<void**>(&glColor4dv), "glColor4dv"},
		{reinterpret_cast<void**>(&glColor4f), "glColor4f"},
		{reinterpret_cast<void**>(&glColor4fv), "glColor4fv"},
		{reinterpret_cast<void**>(&glColor4i), "glColor4i"},
		{reinterpret_cast<void**>(&glColor4iv), "glColor4iv"},
		{reinterpret_cast<void**>(&glColor4s), "glColor4s"},
		{reinterpret_cast<void**>(&glColor4sv), "glColor4sv"},
		{reinterpret_cast<void**>(&glColor4ub), "glColor4ub"},
		{reinterpret_cast<void**>(&glColor4ubv), "glColor4ubv"},
		{reinterpret_cast<void**>(&glColor4ui), "glColor4ui"},
		{reinterpret_cast<void**>(&glColor4uiv), "glColor4uiv"},
		{reinterpret_cast<void**>(&glColor4us), "glColor4us"},
		{reinterpret_cast<void**>(&glColor4usv), "glColor4usv"},
		{reinterpret_cast<void**>(&glColorMaski), "glColorMaski"},
		{reinterpret_cast<void**>(&glColorMask), "glColorMask"},
		{reinterpret_cast<void**>(&glColorMaterial), "glColorMaterial"},
		{reinterpret_cast<void**>(&glColorPointer), "glColorPointer"},
		{reinterpret_cast<void**>(&glCompileShader), "glCompileShader"},
		{reinterpret_cast<void**>(&glCompressedTexImage1D), "glCompressedTexImage1D"},
		{reinterpret_cast<void**>(&glCompressedTexImage2D), "glCompressedTexImage2D"},
		{reinterpret_cast<void**>(&glCompressedTexImage3D), "glCompressedTexImage3D"},
		{reinterpret_cast<void**>(&glCompressedTexSubImage1D), "glCompressedTexSubImage1D"},
		{reinterpret_cast<void**>(&glCompressedTexSubImage2D), "glCompressedTexSubImage2D"},
		{reinterpret_cast<void**>(&glCompressedTexSubImage3D), "glCompressedTexSubImage3D"},
		{reinterpret_cast<void**>(&glCompressedTextureSubImage1D), "glCompressedTextureSubImage1D"},
		{reinterpret_cast<void**>(&glCompressedTextureSubImage2D), "glCompressedTextureSubImage2D"},
		{reinterpret_cast<void**>(&glCompressedTextureSubImage3D), "glCompressedTextureSubImage3D"},
		{reinterpret_cast<void**>(&glCopyBufferSubData), "glCopyBufferSubData"},
		{reinterpret_cast<void**>(&glCopyNamedBufferSubData), "glCopyNamedBufferSubData"},
		{reinterpret_cast<void**>(&glCopyPixels), "glCopyPixels"},
		{reinterpret_cast<void**>(&glCopyTexImage1D), "glCopyTexImage1D"},
		{reinterpret_cast<void**>(&glCopyTexImage2D), "glCopyTexImage2D"},
		{reinterpret_cast<void**>(&glCopyTexSubImage1D), "glCopyTexSubImage1D"},
		{reinterpret_cast<void**>(&glCopyTexSubImage2D), "glCopyTexSubImage2D"},
		{reinterpret_cast<void**>(&glCopyTexSubImage3D), "glCopyTexSubImage3D"},
		{reinterpret_cast<void**>(&glCopyTextureSubImage1D), "glCopyTextureSubImage1D"},
		{reinterpret_cast<void**>(&glCopyTextureSubImage2D), "glCopyTextureSubImage2D"},
		{reinterpret_cast<void**>(&glCopyTextureSubImage3D), "glCopyTextureSubImage3D"},
		{reinterpret_cast<void**>(&glCreateBuffers), "glCreateBuffers"},
		{reinterpret_cast<void**>(&glCreateFramebuffers), "glCreateFramebuffers"},
		{reinterpret_cast<void**>(&glCreateProgramPipelines), "glCreateProgramPipelines"},
		{reinterpret_cast<void**>(&glCreateProgram), "glCreateProgram"},
		{reinterpret_cast<void**>(&glCreateQueries), "glCreateQueries"},
		{reinterpret_cast<void**>(&glCreateRenderbuffers), "glCreateRenderbuffers"},
		{reinterpret_cast<void**>(&glCreateSamplers), "glCreateSamplers"},
		{reinterpret_cast<void**>(&glCreateShader), "glCreateShader"},
		{reinterpret_cast<void**>(&glCreateShaderProgramv), "glCreateShaderProgramv"},
		{reinterpret_cast<void**>(&glCreateTextures), "glCreateTextures"},
		{reinterpret_cast<void**>(&glCreateTransformFeedbacks), "glCreateTransformFeedbacks"},
		{reinterpret_cast<void**>(&glCreateVertexArrays), "glCreateVertexArrays"},
		{reinterpret_cast<void**>(&glCullFace), "glCullFace"},
		{reinterpret_cast<void**>(&glDeleteBuffersARB), "glDeleteBuffersARB"},
		{reinterpret_cast<void**>(&glDeleteBuffers), "glDeleteBuffers"},
		{reinterpret_cast<void**>(&glDeleteFramebuffersEXT), "glDeleteFramebuffersEXT"},
		{reinterpret_cast<void**>(&glDeleteFramebuffers), "glDeleteFramebuffers"},
		{reinterpret_cast<void**>(&glDeleteLists), "glDeleteLists"},
		{reinterpret_cast<void**>(&glDeleteProgramPipelines), "glDeleteProgramPipelines"},
		{reinterpret_cast<void**>(&glDeleteProgram), "glDeleteProgram"},
		{reinterpret_cast<void**>(&glDeleteQueries), "glDeleteQueries"},
		{reinterpret_cast<void**>(&glDeleteRenderbuffersEXT), "glDeleteRenderbuffersEXT"},
		{reinterpret_cast<void**>(&glDeleteRenderbuffers), "glDeleteRenderbuffers"},
		{reinterpret_cast<void**>(&glDeleteSamplers), "glDeleteSamplers"},
		{reinterpret_cast<void**>(&glDeleteShader), "glDeleteShader"},
		{reinterpret_cast<void**>(&glDeleteSync), "glDeleteSync"},
		{reinterpret_cast<void**>(&glDeleteTextures), "glDeleteTextures"},
		{reinterpret_cast<void**>(&glDeleteVertexArrays), "glDeleteVertexArrays"},
		{reinterpret_cast<void**>(&glDepthFunc), "glDepthFunc"},
		{reinterpret_cast<void**>(&glDepthMask), "glDepthMask"},
		{reinterpret_cast<void**>(&glDepthRangef), "glDepthRangef"},
		{reinterpret_cast<void**>(&glDepthRange), "glDepthRange"},
		{reinterpret_cast<void**>(&glDetachShader), "glDetachShader"},
		{reinterpret_cast<void**>(&glDisableClientState), "glDisableClientState"},
		{reinterpret_cast<void**>(&glDisablei), "glDisablei"},
		{reinterpret_cast<void**>(&glDisable), "glDisable"},
		{reinterpret_cast<void**>(&glDisableVertexArrayAttrib), "glDisableVertexArrayAttrib"},
		{reinterpret_cast<void**>(&glDisableVertexAttribArray), "glDisableVertexAttribArray"},
		{reinterpret_cast<void**>(&glDrawArraysInstanced), "glDrawArraysInstanced"},
		{reinterpret_cast<void**>(&glDrawArrays), "glDrawArrays"},
		{reinterpret_cast<void**>(&glDrawBuffer), "glDrawBuffer"},
		{reinterpret_cast<void**>(&glDrawBuffers), "glDrawBuffers"},
		{reinterpret_cast<void**>(&glDrawElementsBaseVertex), "glDrawElementsBaseVertex"},
		{reinterpret_cast<void**>(&glDrawElementsInstancedBaseVertex), "glDrawElementsInstancedBaseVertex"},
		{reinterpret_cast<void**>(&glDrawElementsInstanced), "glDrawElementsInstanced"},
		{reinterpret_cast<void**>(&glDrawElements), "glDrawElements"},
		{reinterpret_cast<void**>(&glDrawPixels), "glDrawPixels"},
		{reinterpret_cast<void**>(&glDrawRangeElementsBaseVertex), "glDrawRangeElementsBaseVertex"},
		{reinterpret_cast<void**>(&glDrawRangeElements), "glDrawRangeElements"},
		{reinterpret_cast<void**>(&glEdgeFlagPointer), "glEdgeFlagPointer"},
		{reinterpret_cast<void**>(&glEdgeFlag), "glEdgeFlag"},
		{reinterpret_cast<void**>(&glEdgeFlagv), "glEdgeFlagv"},
		{reinterpret_cast<void**>(&glEnableClientState), "glEnableClientState"},
		{reinterpret_cast<void**>(&glEnablei), "glEnablei"},
		{reinterpret_cast<void**>(&glEnable), "glEnable"},
		{reinterpret_cast<void**>(&glEnableVertexArrayAttrib), "glEnableVertexArrayAttrib"},
		{reinterpret_cast<void**>(&glEnableVertexAttribArray), "glEnableVertexAttribArray"},
		{reinterpret_cast<void**>(&glEndConditionalRender), "glEndConditionalRender"},
		{reinterpret_cast<void**>(&glEndList), "glEndList"},
		{reinterpret_cast<void**>(&glEnd), "glEnd"},
		{reinterpret_cast<void**>(&glEndQuery), "glEndQuery"},
		{reinterpret_cast<void**>(&glEndTransformFeedback), "glEndTransformFeedback"},
		{reinterpret_cast<void**>(&glEvalCoord1d), "glEvalCoord1d"},
		{reinterpret_cast<void**>(&glEvalCoord1dv), "glEvalCoord1dv"},
		{reinterpret_cast<void**>(&glEvalCoord1f), "glEvalCoord1f"},
		{reinterpret_cast<void**>(&glEvalCoord1fv), "glEvalCoord1fv"},
		{reinterpret_cast<void**>(&glEvalCoord2d), "glEvalCoord2d"},
		{reinterpret_cast<void**>(&glEvalCoord2dv), "glEvalCoord2dv"},
		{reinterpret_cast<void**>(&glEvalCoord2f), "glEvalCoord2f"},
		{reinterpret_cast<void**>(&glEvalCoord2fv), "glEvalCoord2fv"},
		{reinterpret_cast<void**>(&glEvalMesh1), "glEvalMesh1"},
		{reinterpret_cast<void**>(&glEvalMesh2), "glEvalMesh2"},
		{reinterpret_cast<void**>(&glEvalPoint1), "glEvalPoint1"},
		{reinterpret_cast<void**>(&glEvalPoint2), "glEvalPoint2"},
		{reinterpret_cast<void**>(&glFeedbackBuffer), "glFeedbackBuffer"},
		{reinterpret_cast<void**>(&glFenceSync), "glFenceSync"},
		{reinterpret_cast<void**>(&glFinish), "glFinish"},
		{reinterpret_cast<void**>(&glFlushMappedBufferRange), "glFlushMappedBufferRange"},
		{reinterpret_cast<void**>(&glFlushMappedNamedBufferRange), "glFlushMappedNamedBufferRange"},
		{reinterpret_cast<void**>(&glFlush), "glFlush"},
		{reinterpret_cast<void**>(&glFogCoordd), "glFogCoordd"},
		{reinterpret_cast<void**>(&glFogCoorddv), "glFogCoorddv"},
		{reinterpret_cast<void**>(&glFogCoordf), "glFogCoordf"},
		{reinterpret_cast<void**>(&glFogCoordfv), "glFogCoordfv"},
		{reinterpret_cast<void**>(&glFogCoordPointer), "glFogCoordPointer"},
		{reinterpret_cast<void**>(&glFogf), "glFogf"},
		{reinterpret_cast<void**>(&glFogfv), "glFogfv"},
		{reinterpret_cast<void**>(&glFogi), "glFogi"},
		{reinterpret_cast<void**>(&glFogiv), "glFogiv"},
		{reinterpret_cast<void**>(&glFramebufferRenderbufferEXT), "glFramebufferRenderbufferEXT"},
		{reinterpret_cast<void**>(&glFramebufferRenderbuffer), "glFramebufferRenderbuffer"},
		{reinterpret_cast<void**>(&glFramebufferTexture1DEXT), "glFramebufferTexture1DEXT"},
		{reinterpret_cast<void**>(&glFramebufferTexture1D), "glFramebufferTexture1D"},
		{reinterpret_cast<void**>(&glFramebufferTexture2DEXT), "glFramebufferTexture2DEXT"},
		{reinterpret_cast<void**>(&glFramebufferTexture2D), "glFramebufferTexture2D"},
		{reinterpret_cast<void**>(&glFramebufferTexture3DEXT), "glFramebufferTexture3DEXT"},
		{reinterpret_cast<void**>(&glFramebufferTexture3D), "glFramebufferTexture3D"},
		{reinterpret_cast<void**>(&glFramebufferTextureLayer), "glFramebufferTextureLayer"},
		{reinterpret_cast<void**>(&glFramebufferTexture), "glFramebufferTexture"},
		{reinterpret_cast<void**>(&glFrontFace), "glFrontFace"},
		{reinterpret_cast<void**>(&glFrustum), "glFrustum"},
		{reinterpret_cast<void**>(&glGenBuffersARB), "glGenBuffersARB"},
		{reinterpret_cast<void**>(&glGenBuffers), "glGenBuffers"},
		{reinterpret_cast<void**>(&glGenerateMipmapEXT), "glGenerateMipmapEXT"},
		{reinterpret_cast<void**>(&glGenerateMipmap), "glGenerateMipmap"},
		{reinterpret_cast<void**>(&glGenerateTextureMipmap), "glGenerateTextureMipmap"},
		{reinterpret_cast<void**>(&glGenFramebuffersEXT), "glGenFramebuffersEXT"},
		{reinterpret_cast<void**>(&glGenFramebuffers), "glGenFramebuffers"},
		{reinterpret_cast<void**>(&glGenLists), "glGenLists"},
		{reinterpret_cast<void**>(&glGenProgramPipelines), "glGenProgramPipelines"},
		{reinterpret_cast<void**>(&glGenQueries), "glGenQueries"},
		{reinterpret_cast<void**>(&glGenRenderbuffersEXT), "glGenRenderbuffersEXT"},
		{reinterpret_cast<void**>(&glGenRenderbuffers), "glGenRenderbuffers"},
		{reinterpret_cast<void**>(&glGenSamplers), "glGenSamplers"},
		{reinterpret_cast<void**>(&glGenTextures), "glGenTextures"},
		{reinterpret_cast<void**>(&glGenVertexArrays), "glGenVertexArrays"},
		{reinterpret_cast<void**>(&glGetActiveAttrib), "glGetActiveAttrib"},
		{reinterpret_cast<void**>(&glGetActiveUniformBlockiv), "glGetActiveUniformBlockiv"},
		{reinterpret_cast<void**>(&glGetActiveUniformBlockName), "glGetActiveUniformBlockName"},
		{reinterpret_cast<void**>(&glGetActiveUniformName), "glGetActiveUniformName"},
		{reinterpret_cast<void**>(&glGetActiveUniform), "glGetActiveUniform"},
		{reinterpret_cast<void**>(&glGetActiveUniformsiv), "glGetActiveUniformsiv"},
		{reinterpret_cast<void**>(&glGetAttachedShaders), "glGetAttachedShaders"},
		{reinterpret_cast<void**>(&glGetAttribLocation), "glGetAttribLocation"},
		{reinterpret_cast<void**>(&glGetBooleani_v), "glGetBooleani_v"},
		{reinterpret_cast<void**>(&glGetBooleanv), "glGetBooleanv"},
		{reinterpret_cast<void**>(&glGetBufferParameteri64v), "glGetBufferParameteri64v"},
		{reinterpret_cast<void**>(&glGetBufferParameterivARB), "glGetBufferParameterivARB"},
		{reinterpret_cast<void**>(&glGetBufferParameteriv), "glGetBufferParameteriv"},
		{reinterpret_cast<void**>(&glGetBufferPointervARB), "glGetBufferPointervARB"},
		{reinterpret_cast<void**>(&glGetBufferPointerv), "glGetBufferPointerv"},
		{reinterpret_cast<void**>(&glGetBufferSubDataARB), "glGetBufferSubDataARB"},
		{reinterpret_cast<void**>(&glGetBufferSubData), "glGetBufferSubData"},
		{reinterpret_cast<void**>(&glGetClipPlane), "glGetClipPlane"},
		{reinterpret_cast<void**>(&glGetCompressedTexImage), "glGetCompressedTexImage"},
		{reinterpret_cast<void**>(&glGetCompressedTextureImage), "glGetCompressedTextureImage"},
		{reinterpret_cast<void**>(&glGetDoublev), "glGetDoublev"},
		{reinterpret_cast<void**>(&glGetError), "glGetError"},
		{reinterpret_cast<void**>(&glGetFloatv), "glGetFloatv"},
		{reinterpret_cast<void**>(&glGetFragDataLocation), "glGetFragDataLocation"},
		{reinterpret_cast<void**>(&glGetFramebufferAttachmentParameterivEXT), "glGetFramebufferAttachmentParameterivEXT"},
		{reinterpret_cast<void**>(&glGetFramebufferAttachmentParameteriv), "glGetFramebufferAttachmentParameteriv"},
		{reinterpret_cast<void**>(&glGetInteger64i_v), "glGetInteger64i_v"},
		{reinterpret_cast<void**>(&glGetInteger64v), "glGetInteger64v"},
		{reinterpret_cast<void**>(&glGetIntegeri_v), "glGetIntegeri_v"},
		{reinterpret_cast<void**>(&glGetIntegerv), "glGetIntegerv"},
		{reinterpret_cast<void**>(&glGetLightfv), "glGetLightfv"},
		{reinterpret_cast<void**>(&glGetLightiv), "glGetLightiv"},
		{reinterpret_cast<void**>(&glGetMapdv), "glGetMapdv"},
		{reinterpret_cast<void**>(&glGetMapfv), "glGetMapfv"},
		{reinterpret_cast<void**>(&glGetMapiv), "glGetMapiv"},
		{reinterpret_cast<void**>(&glGetMaterialfv), "glGetMaterialfv"},
		{reinterpret_cast<void**>(&glGetMaterialiv), "glGetMaterialiv"},
		{reinterpret_cast<void**>(&glGetMultisamplefv), "glGetMultisamplefv"},
		{reinterpret_cast<void**>(&glGetNamedBufferParameteri64v), "glGetNamedBufferParameteri64v"},
		{reinterpret_cast<void**>(&glGetNamedBufferParameteriv), "glGetNamedBufferParameteriv"},
		{reinterpret_cast<void**>(&glGetNamedBufferPointerv), "glGetNamedBufferPointerv"},
		{reinterpret_cast<void**>(&glGetNamedBufferSubData), "glGetNamedBufferSubData"},
		{reinterpret_cast<void**>(&glGetNamedFramebufferAttachmentParameteriv), "glGetNamedFramebufferAttachmentParameteriv"},
		{reinterpret_cast<void**>(&glGetNamedFramebufferParameteriv), "glGetNamedFramebufferParameteriv"},
		{reinterpret_cast<void**>(&glGetNamedRenderbufferParameteriv), "glGetNamedRenderbufferParameteriv"},
		{reinterpret_cast<void**>(&glGetPixelMapfv), "glGetPixelMapfv"},
		{reinterpret_cast<void**>(&glGetPixelMapuiv), "glGetPixelMapuiv"},
		{reinterpret_cast<void**>(&glGetPixelMapusv), "glGetPixelMapusv"},
		{reinterpret_cast<void**>(&glGetPointerv), "glGetPointerv"},
		{reinterpret_cast<void**>(&glGetPolygonStipple), "glGetPolygonStipple"},
		{reinterpret_cast<void**>(&glGetProgramInfoLog), "glGetProgramInfoLog"},
		{reinterpret_cast<void**>(&glGetProgramiv), "glGetProgramiv"},
		{reinterpret_cast<void**>(&glGetProgramPipelineInfoLog), "glGetProgramPipelineInfoLog"},
		{reinterpret_cast<void**>(&glGetProgramPipelineiv), "glGetProgramPipelineiv"},
		{reinterpret_cast<void**>(&glGetQueryBufferObjecti64v), "glGetQueryBufferObjecti64v"},
		{reinterpret_cast<void**>(&glGetQueryBufferObjectiv), "glGetQueryBufferObjectiv"},
		{reinterpret_cast<void**>(&glGetQueryBufferObjectui64v), "glGetQueryBufferObjectui64v"},
		{reinterpret_cast<void**>(&glGetQueryBufferObjectuiv), "glGetQueryBufferObjectuiv"},
		{reinterpret_cast<void**>(&glGetQueryiv), "glGetQueryiv"},
		{reinterpret_cast<void**>(&glGetQueryObjectiv), "glGetQueryObjectiv"},
		{reinterpret_cast<void**>(&glGetQueryObjectuiv), "glGetQueryObjectuiv"},
		{reinterpret_cast<void**>(&glGetRenderbufferParameterivEXT), "glGetRenderbufferParameterivEXT"},
		{reinterpret_cast<void**>(&glGetRenderbufferParameteriv), "glGetRenderbufferParameteriv"},
		{reinterpret_cast<void**>(&glGetSamplerParameterfv), "glGetSamplerParameterfv"},
		{reinterpret_cast<void**>(&glGetSamplerParameterIiv), "glGetSamplerParameterIiv"},
		{reinterpret_cast<void**>(&glGetSamplerParameterIuiv), "glGetSamplerParameterIuiv"},
		{reinterpret_cast<void**>(&glGetSamplerParameteriv), "glGetSamplerParameteriv"},
		{reinterpret_cast<void**>(&glGetShaderInfoLog), "glGetShaderInfoLog"},
		{reinterpret_cast<void**>(&glGetShaderiv), "glGetShaderiv"},
		{reinterpret_cast<void**>(&glGetShaderPrecisionFormat), "glGetShaderPrecisionFormat"},
		{reinterpret_cast<void**>(&glGetShaderSource), "glGetShaderSource"},
		{reinterpret_cast<void**>(&glGetStringi), "glGetStringi"},
		{reinterpret_cast<void**>(&glGetString), "glGetString"},
		{reinterpret_cast<void**>(&glGetSynciv), "glGetSynciv"},
		{reinterpret_cast<void**>(&glGetTexEnvfv), "glGetTexEnvfv"},
		{reinterpret_cast<void**>(&glGetTexEnviv), "glGetTexEnviv"},
		{reinterpret_cast<void**>(&glGetTexGendv), "glGetTexGendv"},
		{reinterpret_cast<void**>(&glGetTexGenfv), "glGetTexGenfv"},
		{reinterpret_cast<void**>(&glGetTexGeniv), "glGetTexGeniv"},
		{reinterpret_cast<void**>(&glGetTexImage), "glGetTexImage"},
		{reinterpret_cast<void**>(&glGetTexLevelParameterfv), "glGetTexLevelParameterfv"},
		{reinterpret_cast<void**>(&glGetTexLevelParameteriv), "glGetTexLevelParameteriv"},
		{reinterpret_cast<void**>(&glGetTexParameterfv), "glGetTexParameterfv"},
		{reinterpret_cast<void**>(&glGetTexParameterIiv), "glGetTexParameterIiv"},
		{reinterpret_cast<void**>(&glGetTexParameterIuiv), "glGetTexParameterIuiv"},
		{reinterpret_cast<void**>(&glGetTexParameteriv), "glGetTexParameteriv"},
		{reinterpret_cast<void**>(&glGetTextureImage), "glGetTextureImage"},
		{reinterpret_cast<void**>(&glGetTextureLevelParameterfv), "glGetTextureLevelParameterfv"},
		{reinterpret_cast<void**>(&glGetTextureLevelParameteriv), "glGetTextureLevelParameteriv"},
		{reinterpret_cast<void**>(&glGetTextureParameterfv), "glGetTextureParameterfv"},
		{reinterpret_cast<void**>(&glGetTextureParameterIiv), "glGetTextureParameterIiv"},
		{reinterpret_cast<void**>(&glGetTextureParameterIuiv), "glGetTextureParameterIuiv"},
		{reinterpret_cast<void**>(&glGetTextureParameteriv), "glGetTextureParameteriv"},
		{reinterpret_cast<void**>(&glGetTransformFeedbacki64_v), "glGetTransformFeedbacki64_v"},
		{reinterpret_cast<void**>(&glGetTransformFeedbackiv), "glGetTransformFeedbackiv"},
		{reinterpret_cast<void**>(&glGetTransformFeedbacki_v), "glGetTransformFeedbacki_v"},
		{reinterpret_cast<void**>(&glGetTransformFeedbackVarying), "glGetTransformFeedbackVarying"},
		{reinterpret_cast<void**>(&glGetUniformBlockIndex), "glGetUniformBlockIndex"},
		{reinterpret_cast<void**>(&glGetUniformfv), "glGetUniformfv"},
		{reinterpret_cast<void**>(&glGetUniformIndices), "glGetUniformIndices"},
		{reinterpret_cast<void**>(&glGetUniformiv), "glGetUniformiv"},
		{reinterpret_cast<void**>(&glGetUniformLocation), "glGetUniformLocation"},
		{reinterpret_cast<void**>(&glGetUniformuiv), "glGetUniformuiv"},
		{reinterpret_cast<void**>(&glGetVertexArrayIndexed64iv), "glGetVertexArrayIndexed64iv"},
		{reinterpret_cast<void**>(&glGetVertexArrayIndexediv), "glGetVertexArrayIndexediv"},
		{reinterpret_cast<void**>(&glGetVertexArrayiv), "glGetVertexArrayiv"},
		{reinterpret_cast<void**>(&glGetVertexAttribdv), "glGetVertexAttribdv"},
		{reinterpret_cast<void**>(&glGetVertexAttribfv), "glGetVertexAttribfv"},
		{reinterpret_cast<void**>(&glGetVertexAttribIiv), "glGetVertexAttribIiv"},
		{reinterpret_cast<void**>(&glGetVertexAttribIuiv), "glGetVertexAttribIuiv"},
		{reinterpret_cast<void**>(&glGetVertexAttribiv), "glGetVertexAttribiv"},
		{reinterpret_cast<void**>(&glGetVertexAttribPointerv), "glGetVertexAttribPointerv"},
		{reinterpret_cast<void**>(&glHint), "glHint"},
		{reinterpret_cast<void**>(&glIndexd), "glIndexd"},
		{reinterpret_cast<void**>(&glIndexdv), "glIndexdv"},
		{reinterpret_cast<void**>(&glIndexf), "glIndexf"},
		{reinterpret_cast<void**>(&glIndexfv), "glIndexfv"},
		{reinterpret_cast<void**>(&glIndexi), "glIndexi"},
		{reinterpret_cast<void**>(&glIndexiv), "glIndexiv"},
		{reinterpret_cast<void**>(&glIndexMask), "glIndexMask"},
		{reinterpret_cast<void**>(&glIndexPointer), "glIndexPointer"},
		{reinterpret_cast<void**>(&glIndexs), "glIndexs"},
		{reinterpret_cast<void**>(&glIndexsv), "glIndexsv"},
		{reinterpret_cast<void**>(&glIndexub), "glIndexub"},
		{reinterpret_cast<void**>(&glIndexubv), "glIndexubv"},
		{reinterpret_cast<void**>(&glInitNames), "glInitNames"},
		{reinterpret_cast<void**>(&glInterleavedArrays), "glInterleavedArrays"},
		{reinterpret_cast<void**>(&glInvalidateNamedFramebufferData), "glInvalidateNamedFramebufferData"},
		{reinterpret_cast<void**>(&glInvalidateNamedFramebufferSubData), "glInvalidateNamedFramebufferSubData"},
		{reinterpret_cast<void**>(&glIsBufferARB), "glIsBufferARB"},
		{reinterpret_cast<void**>(&glIsBuffer), "glIsBuffer"},
		{reinterpret_cast<void**>(&glIsEnabledi), "glIsEnabledi"},
		{reinterpret_cast<void**>(&glIsEnabled), "glIsEnabled"},
		{reinterpret_cast<void**>(&glIsFramebufferEXT), "glIsFramebufferEXT"},
		{reinterpret_cast<void**>(&glIsFramebuffer), "glIsFramebuffer"},
		{reinterpret_cast<void**>(&glIsList), "glIsList"},
		{reinterpret_cast<void**>(&glIsProgramPipeline), "glIsProgramPipeline"},
		{reinterpret_cast<void**>(&glIsProgram), "glIsProgram"},
		{reinterpret_cast<void**>(&glIsQuery), "glIsQuery"},
		{reinterpret_cast<void**>(&glIsRenderbufferEXT), "glIsRenderbufferEXT"},
		{reinterpret_cast<void**>(&glIsRenderbuffer), "glIsRenderbuffer"},
		{reinterpret_cast<void**>(&glIsSampler), "glIsSampler"},
		{reinterpret_cast<void**>(&glIsShader), "glIsShader"},
		{reinterpret_cast<void**>(&glIsSync), "glIsSync"},
		{reinterpret_cast<void**>(&glIsTexture), "glIsTexture"},
		{reinterpret_cast<void**>(&glIsVertexArray), "glIsVertexArray"},
		{reinterpret_cast<void**>(&glLightf), "glLightf"},
		{reinterpret_cast<void**>(&glLightfv), "glLightfv"},
		{reinterpret_cast<void**>(&glLighti), "glLighti"},
		{reinterpret_cast<void**>(&glLightiv), "glLightiv"},
		{reinterpret_cast<void**>(&glLightModelf), "glLightModelf"},
		{reinterpret_cast<void**>(&glLightModelfv), "glLightModelfv"},
		{reinterpret_cast<void**>(&glLightModeli), "glLightModeli"},
		{reinterpret_cast<void**>(&glLightModeliv), "glLightModeliv"},
		{reinterpret_cast<void**>(&glLineStipple), "glLineStipple"},
		{reinterpret_cast<void**>(&glLineWidth), "glLineWidth"},
		{reinterpret_cast<void**>(&glLinkProgram), "glLinkProgram"},
		{reinterpret_cast<void**>(&glListBase), "glListBase"},
		{reinterpret_cast<void**>(&glLoadIdentity), "glLoadIdentity"},
		{reinterpret_cast<void**>(&glLoadMatrixd), "glLoadMatrixd"},
		{reinterpret_cast<void**>(&glLoadMatrixf), "glLoadMatrixf"},
		{reinterpret_cast<void**>(&glLoadName), "glLoadName"},
		{reinterpret_cast<void**>(&glLoadTransposeMatrixd), "glLoadTransposeMatrixd"},
		{reinterpret_cast<void**>(&glLoadTransposeMatrixf), "glLoadTransposeMatrixf"},
		{reinterpret_cast<void**>(&glLogicOp), "glLogicOp"},
		{reinterpret_cast<void**>(&glMap1d), "glMap1d"},
		{reinterpret_cast<void**>(&glMap1f), "glMap1f"},
		{reinterpret_cast<void**>(&glMap2d), "glMap2d"},
		{reinterpret_cast<void**>(&glMap2f), "glMap2f"},
		{reinterpret_cast<void**>(&glMapBufferARB), "glMapBufferARB"},
		{reinterpret_cast<void**>(&glMapBuffer), "glMapBuffer"},
		{reinterpret_cast<void**>(&glMapBufferRange), "glMapBufferRange"},
		{reinterpret_cast<void**>(&glMapGrid1d), "glMapGrid1d"},
		{reinterpret_cast<void**>(&glMapGrid1f), "glMapGrid1f"},
		{reinterpret_cast<void**>(&glMapGrid2d), "glMapGrid2d"},
		{reinterpret_cast<void**>(&glMapGrid2f), "glMapGrid2f"},
		{reinterpret_cast<void**>(&glMapNamedBuffer), "glMapNamedBuffer"},
		{reinterpret_cast<void**>(&glMapNamedBufferRange), "glMapNamedBufferRange"},
		{reinterpret_cast<void**>(&glMaterialf), "glMaterialf"},
		{reinterpret_cast<void**>(&glMaterialfv), "glMaterialfv"},
		{reinterpret_cast<void**>(&glMateriali), "glMateriali"},
		{reinterpret_cast<void**>(&glMaterialiv), "glMaterialiv"},
		{reinterpret_cast<void**>(&glMatrixMode), "glMatrixMode"},
		{reinterpret_cast<void**>(&glMultiDrawArrays), "glMultiDrawArrays"},
		{reinterpret_cast<void**>(&glMultiDrawElementsBaseVertex), "glMultiDrawElementsBaseVertex"},
		{reinterpret_cast<void**>(&glMultiDrawElements), "glMultiDrawElements"},
		{reinterpret_cast<void**>(&glMultiTexCoord1d), "glMultiTexCoord1d"},
		{reinterpret_cast<void**>(&glMultiTexCoord1dv), "glMultiTexCoord1dv"},
		{reinterpret_cast<void**>(&glMultiTexCoord1f), "glMultiTexCoord1f"},
		{reinterpret_cast<void**>(&glMultiTexCoord1fv), "glMultiTexCoord1fv"},
		{reinterpret_cast<void**>(&glMultiTexCoord1i), "glMultiTexCoord1i"},
		{reinterpret_cast<void**>(&glMultiTexCoord1iv), "glMultiTexCoord1iv"},
		{reinterpret_cast<void**>(&glMultiTexCoord1s), "glMultiTexCoord1s"},
		{reinterpret_cast<void**>(&glMultiTexCoord1sv), "glMultiTexCoord1sv"},
		{reinterpret_cast<void**>(&glMultiTexCoord2d), "glMultiTexCoord2d"},
		{reinterpret_cast<void**>(&glMultiTexCoord2dv), "glMultiTexCoord2dv"},
		{reinterpret_cast<void**>(&glMultiTexCoord2f), "glMultiTexCoord2f"},
		{reinterpret_cast<void**>(&glMultiTexCoord2fv), "glMultiTexCoord2fv"},
		{reinterpret_cast<void**>(&glMultiTexCoord2i), "glMultiTexCoord2i"},
		{reinterpret_cast<void**>(&glMultiTexCoord2iv), "glMultiTexCoord2iv"},
		{reinterpret_cast<void**>(&glMultiTexCoord2s), "glMultiTexCoord2s"},
		{reinterpret_cast<void**>(&glMultiTexCoord2sv), "glMultiTexCoord2sv"},
		{reinterpret_cast<void**>(&glMultiTexCoord3d), "glMultiTexCoord3d"},
		{reinterpret_cast<void**>(&glMultiTexCoord3dv), "glMultiTexCoord3dv"},
		{reinterpret_cast<void**>(&glMultiTexCoord3f), "glMultiTexCoord3f"},
		{reinterpret_cast<void**>(&glMultiTexCoord3fv), "glMultiTexCoord3fv"},
		{reinterpret_cast<void**>(&glMultiTexCoord3i), "glMultiTexCoord3i"},
		{reinterpret_cast<void**>(&glMultiTexCoord3iv), "glMultiTexCoord3iv"},
		{reinterpret_cast<void**>(&glMultiTexCoord3s), "glMultiTexCoord3s"},
		{reinterpret_cast<void**>(&glMultiTexCoord3sv), "glMultiTexCoord3sv"},
		{reinterpret_cast<void**>(&glMultiTexCoord4d), "glMultiTexCoord4d"},
		{reinterpret_cast<void**>(&glMultiTexCoord4dv), "glMultiTexCoord4dv"},
		{reinterpret_cast<void**>(&glMultiTexCoord4f), "glMultiTexCoord4f"},
		{reinterpret_cast<void**>(&glMultiTexCoord4fv), "glMultiTexCoord4fv"},
		{reinterpret_cast<void**>(&glMultiTexCoord4i), "glMultiTexCoord4i"},
		{reinterpret_cast<void**>(&glMultiTexCoord4iv), "glMultiTexCoord4iv"},
		{reinterpret_cast<void**>(&glMultiTexCoord4s), "glMultiTexCoord4s"},
		{reinterpret_cast<void**>(&glMultiTexCoord4sv), "glMultiTexCoord4sv"},
		{reinterpret_cast<void**>(&glMultMatrixd), "glMultMatrixd"},
		{reinterpret_cast<void**>(&glMultMatrixf), "glMultMatrixf"},
		{reinterpret_cast<void**>(&glMultTransposeMatrixd), "glMultTransposeMatrixd"},
		{reinterpret_cast<void**>(&glMultTransposeMatrixf), "glMultTransposeMatrixf"},
		{reinterpret_cast<void**>(&glNamedBufferData), "glNamedBufferData"},
		{reinterpret_cast<void**>(&glNamedBufferStorage), "glNamedBufferStorage"},
		{reinterpret_cast<void**>(&glNamedBufferSubData), "glNamedBufferSubData"},
		{reinterpret_cast<void**>(&glNamedFramebufferDrawBuffer), "glNamedFramebufferDrawBuffer"},
		{reinterpret_cast<void**>(&glNamedFramebufferDrawBuffers), "glNamedFramebufferDrawBuffers"},
		{reinterpret_cast<void**>(&glNamedFramebufferParameteri), "glNamedFramebufferParameteri"},
		{reinterpret_cast<void**>(&glNamedFramebufferReadBuffer), "glNamedFramebufferReadBuffer"},
		{reinterpret_cast<void**>(&glNamedFramebufferRenderbuffer), "glNamedFramebufferRenderbuffer"},
		{reinterpret_cast<void**>(&glNamedFramebufferTextureLayer), "glNamedFramebufferTextureLayer"},
		{reinterpret_cast<void**>(&glNamedFramebufferTexture), "glNamedFramebufferTexture"},
		{reinterpret_cast<void**>(&glNamedRenderbufferStorageMultisample), "glNamedRenderbufferStorageMultisample"},
		{reinterpret_cast<void**>(&glNamedRenderbufferStorage), "glNamedRenderbufferStorage"},
		{reinterpret_cast<void**>(&glNewList), "glNewList"},
		{reinterpret_cast<void**>(&glNormal3b), "glNormal3b"},
		{reinterpret_cast<void**>(&glNormal3bv), "glNormal3bv"},
		{reinterpret_cast<void**>(&glNormal3d), "glNormal3d"},
		{reinterpret_cast<void**>(&glNormal3dv), "glNormal3dv"},
		{reinterpret_cast<void**>(&glNormal3f), "glNormal3f"},
		{reinterpret_cast<void**>(&glNormal3fv), "glNormal3fv"},
		{reinterpret_cast<void**>(&glNormal3i), "glNormal3i"},
		{reinterpret_cast<void**>(&glNormal3iv), "glNormal3iv"},
		{reinterpret_cast<void**>(&glNormal3s), "glNormal3s"},
		{reinterpret_cast<void**>(&glNormal3sv), "glNormal3sv"},
		{reinterpret_cast<void**>(&glNormalPointer), "glNormalPointer"},
		{reinterpret_cast<void**>(&glOrtho), "glOrtho"},
		{reinterpret_cast<void**>(&glPassThrough), "glPassThrough"},
		{reinterpret_cast<void**>(&glPixelMapfv), "glPixelMapfv"},
		{reinterpret_cast<void**>(&glPixelMapuiv), "glPixelMapuiv"},
		{reinterpret_cast<void**>(&glPixelMapusv), "glPixelMapusv"},
		{reinterpret_cast<void**>(&glPixelStoref), "glPixelStoref"},
		{reinterpret_cast<void**>(&glPixelStorei), "glPixelStorei"},
		{reinterpret_cast<void**>(&glPixelTransferf), "glPixelTransferf"},
		{reinterpret_cast<void**>(&glPixelTransferi), "glPixelTransferi"},
		{reinterpret_cast<void**>(&glPixelZoom), "glPixelZoom"},
		{reinterpret_cast<void**>(&glPointParameterf), "glPointParameterf"},
		{reinterpret_cast<void**>(&glPointParameterfv), "glPointParameterfv"},
		{reinterpret_cast<void**>(&glPointParameteri), "glPointParameteri"},
		{reinterpret_cast<void**>(&glPointParameteriv), "glPointParameteriv"},
		{reinterpret_cast<void**>(&glPointSize), "glPointSize"},
		{reinterpret_cast<void**>(&glPolygonMode), "glPolygonMode"},
		{reinterpret_cast<void**>(&glPolygonOffset), "glPolygonOffset"},
		{reinterpret_cast<void**>(&glPolygonStipple), "glPolygonStipple"},
		{reinterpret_cast<void**>(&glPopAttrib), "glPopAttrib"},
		{reinterpret_cast<void**>(&glPopClientAttrib), "glPopClientAttrib"},
		{reinterpret_cast<void**>(&glPopMatrix), "glPopMatrix"},
		{reinterpret_cast<void**>(&glPopName), "glPopName"},
		{reinterpret_cast<void**>(&glPrimitiveRestartIndex), "glPrimitiveRestartIndex"},
		{reinterpret_cast<void**>(&glPrioritizeTextures), "glPrioritizeTextures"},
		{reinterpret_cast<void**>(&glProgramParameteri), "glProgramParameteri"},
		{reinterpret_cast<void**>(&glProgramUniform1d), "glProgramUniform1d"},
		{reinterpret_cast<void**>(&glProgramUniform1dv), "glProgramUniform1dv"},
		{reinterpret_cast<void**>(&glProgramUniform1f), "glProgramUniform1f"},
		{reinterpret_cast<void**>(&glProgramUniform1fv), "glProgramUniform1fv"},
		{reinterpret_cast<void**>(&glProgramUniform1i), "glProgramUniform1i"},
		{reinterpret_cast<void**>(&glProgramUniform1iv), "glProgramUniform1iv"},
		{reinterpret_cast<void**>(&glProgramUniform1ui), "glProgramUniform1ui"},
		{reinterpret_cast<void**>(&glProgramUniform1uiv), "glProgramUniform1uiv"},
		{reinterpret_cast<void**>(&glProgramUniform2d), "glProgramUniform2d"},
		{reinterpret_cast<void**>(&glProgramUniform2dv), "glProgramUniform2dv"},
		{reinterpret_cast<void**>(&glProgramUniform2f), "glProgramUniform2f"},
		{reinterpret_cast<void**>(&glProgramUniform2fv), "glProgramUniform2fv"},
		{reinterpret_cast<void**>(&glProgramUniform2i), "glProgramUniform2i"},
		{reinterpret_cast<void**>(&glProgramUniform2iv), "glProgramUniform2iv"},
		{reinterpret_cast<void**>(&glProgramUniform2ui), "glProgramUniform2ui"},
		{reinterpret_cast<void**>(&glProgramUniform2uiv), "glProgramUniform2uiv"},
		{reinterpret_cast<void**>(&glProgramUniform3d), "glProgramUniform3d"},
		{reinterpret_cast<void**>(&glProgramUniform3dv), "glProgramUniform3dv"},
		{reinterpret_cast<void**>(&glProgramUniform3f), "glProgramUniform3f"},
		{reinterpret_cast<void**>(&glProgramUniform3fv), "glProgramUniform3fv"},
		{reinterpret_cast<void**>(&glProgramUniform3i), "glProgramUniform3i"},
		{reinterpret_cast<void**>(&glProgramUniform3iv), "glProgramUniform3iv"},
		{reinterpret_cast<void**>(&glProgramUniform3ui), "glProgramUniform3ui"},
		{reinterpret_cast<void**>(&glProgramUniform3uiv), "glProgramUniform3uiv"},
		{reinterpret_cast<void**>(&glProgramUniform4d), "glProgramUniform4d"},
		{reinterpret_cast<void**>(&glProgramUniform4dv), "glProgramUniform4dv"},
		{reinterpret_cast<void**>(&glProgramUniform4f), "glProgramUniform4f"},
		{reinterpret_cast<void**>(&glProgramUniform4fv), "glProgramUniform4fv"},
		{reinterpret_cast<void**>(&glProgramUniform4i), "glProgramUniform4i"},
		{reinterpret_cast<void**>(&glProgramUniform4iv), "glProgramUniform4iv"},
		{reinterpret_cast<void**>(&glProgramUniform4ui), "glProgramUniform4ui"},
		{reinterpret_cast<void**>(&glProgramUniform4uiv), "glProgramUniform4uiv"},
		{reinterpret_cast<void**>(&glProgramUniformMatrix2dv), "glProgramUniformMatrix2dv"},
		{reinterpret_cast<void**>(&glProgramUniformMatrix2fv), "glProgramUniformMatrix2fv"},
		{reinterpret_cast<void**>(&glProgramUniformMatrix2x3dv), "glProgramUniformMatrix2x3dv"},
		{reinterpret_cast<void**>(&glProgramUniformMatrix2x3fv), "glProgramUniformMatrix2x3fv"},
		{reinterpret_cast<void**>(&glProgramUniformMatrix2x4dv), "glProgramUniformMatrix2x4dv"},
		{reinterpret_cast<void**>(&glProgramUniformMatrix2x4fv), "glProgramUniformMatrix2x4fv"},
		{reinterpret_cast<void**>(&glProgramUniformMatrix3dv), "glProgramUniformMatrix3dv"},
		{reinterpret_cast<void**>(&glProgramUniformMatrix3fv), "glProgramUniformMatrix3fv"},
		{reinterpret_cast<void**>(&glProgramUniformMatrix3x2dv), "glProgramUniformMatrix3x2dv"},
		{reinterpret_cast<void**>(&glProgramUniformMatrix3x2fv), "glProgramUniformMatrix3x2fv"},
		{reinterpret_cast<void**>(&glProgramUniformMatrix3x4dv), "glProgramUniformMatrix3x4dv"},
		{reinterpret_cast<void**>(&glProgramUniformMatrix3x4fv), "glProgramUniformMatrix3x4fv"},
		{reinterpret_cast<void**>(&glProgramUniformMatrix4dv), "glProgramUniformMatrix4dv"},
		{reinterpret_cast<void**>(&glProgramUniformMatrix4fv), "glProgramUniformMatrix4fv"},
		{reinterpret_cast<void**>(&glProgramUniformMatrix4x2dv), "glProgramUniformMatrix4x2dv"},
		{reinterpret_cast<void**>(&glProgramUniformMatrix4x2fv), "glProgramUniformMatrix4x2fv"},
		{reinterpret_cast<void**>(&glProgramUniformMatrix4x3dv), "glProgramUniformMatrix4x3dv"},
		{reinterpret_cast<void**>(&glProgramUniformMatrix4x3fv), "glProgramUniformMatrix4x3fv"},
		{reinterpret_cast<void**>(&glProvokingVertex), "glProvokingVertex"},
		{reinterpret_cast<void**>(&glPushAttrib), "glPushAttrib"},
		{reinterpret_cast<void**>(&glPushClientAttrib), "glPushClientAttrib"},
		{reinterpret_cast<void**>(&glPushMatrix), "glPushMatrix"},
		{reinterpret_cast<void**>(&glPushName), "glPushName"},
		{reinterpret_cast<void**>(&glRasterPos2d), "glRasterPos2d"},
		{reinterpret_cast<void**>(&glRasterPos2dv), "glRasterPos2dv"},
		{reinterpret_cast<void**>(&glRasterPos2f), "glRasterPos2f"},
		{reinterpret_cast<void**>(&glRasterPos2fv), "glRasterPos2fv"},
		{reinterpret_cast<void**>(&glRasterPos2i), "glRasterPos2i"},
		{reinterpret_cast<void**>(&glRasterPos2iv), "glRasterPos2iv"},
		{reinterpret_cast<void**>(&glRasterPos2s), "glRasterPos2s"},
		{reinterpret_cast<void**>(&glRasterPos2sv), "glRasterPos2sv"},
		{reinterpret_cast<void**>(&glRasterPos3d), "glRasterPos3d"},
		{reinterpret_cast<void**>(&glRasterPos3dv), "glRasterPos3dv"},
		{reinterpret_cast<void**>(&glRasterPos3f), "glRasterPos3f"},
		{reinterpret_cast<void**>(&glRasterPos3fv), "glRasterPos3fv"},
		{reinterpret_cast<void**>(&glRasterPos3i), "glRasterPos3i"},
		{reinterpret_cast<void**>(&glRasterPos3iv), "glRasterPos3iv"},
		{reinterpret_cast<void**>(&glRasterPos3s), "glRasterPos3s"},
		{reinterpret_cast<void**>(&glRasterPos3sv), "glRasterPos3sv"},
		{reinterpret_cast<void**>(&glRasterPos4d), "glRasterPos4d"},
		{reinterpret_cast<void**>(&glRasterPos4dv), "glRasterPos4dv"},
		{reinterpret_cast<void**>(&glRasterPos4f), "glRasterPos4f"},
		{reinterpret_cast<void**>(&glRasterPos4fv), "glRasterPos4fv"},
		{reinterpret_cast<void**>(&glRasterPos4i), "glRasterPos4i"},
		{reinterpret_cast<void**>(&glRasterPos4iv), "glRasterPos4iv"},
		{reinterpret_cast<void**>(&glRasterPos4s), "glRasterPos4s"},
		{reinterpret_cast<void**>(&glRasterPos4sv), "glRasterPos4sv"},
		{reinterpret_cast<void**>(&glReadBuffer), "glReadBuffer"},
		{reinterpret_cast<void**>(&glReadPixels), "glReadPixels"},
		{reinterpret_cast<void**>(&glRectd), "glRectd"},
		{reinterpret_cast<void**>(&glRectdv), "glRectdv"},
		{reinterpret_cast<void**>(&glRectf), "glRectf"},
		{reinterpret_cast<void**>(&glRectfv), "glRectfv"},
		{reinterpret_cast<void**>(&glRecti), "glRecti"},
		{reinterpret_cast<void**>(&glRectiv), "glRectiv"},
		{reinterpret_cast<void**>(&glRects), "glRects"},
		{reinterpret_cast<void**>(&glRectsv), "glRectsv"},
		{reinterpret_cast<void**>(&glReleaseShaderCompiler), "glReleaseShaderCompiler"},
		{reinterpret_cast<void**>(&glRenderbufferStorageEXT), "glRenderbufferStorageEXT"},
		{reinterpret_cast<void**>(&glRenderbufferStorageMultisampleEXT), "glRenderbufferStorageMultisampleEXT"},
		{reinterpret_cast<void**>(&glRenderbufferStorageMultisample), "glRenderbufferStorageMultisample"},
		{reinterpret_cast<void**>(&glRenderbufferStorage), "glRenderbufferStorage"},
		{reinterpret_cast<void**>(&glRenderMode), "glRenderMode"},
		{reinterpret_cast<void**>(&glRotated), "glRotated"},
		{reinterpret_cast<void**>(&glRotatef), "glRotatef"},
		{reinterpret_cast<void**>(&glSampleCoverage), "glSampleCoverage"},
		{reinterpret_cast<void**>(&glSampleMaski), "glSampleMaski"},
		{reinterpret_cast<void**>(&glSamplerParameterf), "glSamplerParameterf"},
		{reinterpret_cast<void**>(&glSamplerParameterfv), "glSamplerParameterfv"},
		{reinterpret_cast<void**>(&glSamplerParameterIiv), "glSamplerParameterIiv"},
		{reinterpret_cast<void**>(&glSamplerParameteri), "glSamplerParameteri"},
		{reinterpret_cast<void**>(&glSamplerParameterIuiv), "glSamplerParameterIuiv"},
		{reinterpret_cast<void**>(&glSamplerParameteriv), "glSamplerParameteriv"},
		{reinterpret_cast<void**>(&glScaled), "glScaled"},
		{reinterpret_cast<void**>(&glScalef), "glScalef"},
		{reinterpret_cast<void**>(&glScissor), "glScissor"},
		{reinterpret_cast<void**>(&glSecondaryColor3b), "glSecondaryColor3b"},
		{reinterpret_cast<void**>(&glSecondaryColor3bv), "glSecondaryColor3bv"},
		{reinterpret_cast<void**>(&glSecondaryColor3d), "glSecondaryColor3d"},
		{reinterpret_cast<void**>(&glSecondaryColor3dv), "glSecondaryColor3dv"},
		{reinterpret_cast<void**>(&glSecondaryColor3f), "glSecondaryColor3f"},
		{reinterpret_cast<void**>(&glSecondaryColor3fv), "glSecondaryColor3fv"},
		{reinterpret_cast<void**>(&glSecondaryColor3i), "glSecondaryColor3i"},
		{reinterpret_cast<void**>(&glSecondaryColor3iv), "glSecondaryColor3iv"},
		{reinterpret_cast<void**>(&glSecondaryColor3s), "glSecondaryColor3s"},
		{reinterpret_cast<void**>(&glSecondaryColor3sv), "glSecondaryColor3sv"},
		{reinterpret_cast<void**>(&glSecondaryColor3ub), "glSecondaryColor3ub"},
		{reinterpret_cast<void**>(&glSecondaryColor3ubv), "glSecondaryColor3ubv"},
		{reinterpret_cast<void**>(&glSecondaryColor3ui), "glSecondaryColor3ui"},
		{reinterpret_cast<void**>(&glSecondaryColor3uiv), "glSecondaryColor3uiv"},
		{reinterpret_cast<void**>(&glSecondaryColor3us), "glSecondaryColor3us"},
		{reinterpret_cast<void**>(&glSecondaryColor3usv), "glSecondaryColor3usv"},
		{reinterpret_cast<void**>(&glSecondaryColorPointer), "glSecondaryColorPointer"},
		{reinterpret_cast<void**>(&glSelectBuffer), "glSelectBuffer"},
		{reinterpret_cast<void**>(&glShadeModel), "glShadeModel"},
		{reinterpret_cast<void**>(&glShaderBinary), "glShaderBinary"},
		{reinterpret_cast<void**>(&glShaderSource), "glShaderSource"},
		{reinterpret_cast<void**>(&glStencilFunc), "glStencilFunc"},
		{reinterpret_cast<void**>(&glStencilFuncSeparate), "glStencilFuncSeparate"},
		{reinterpret_cast<void**>(&glStencilMask), "glStencilMask"},
		{reinterpret_cast<void**>(&glStencilMaskSeparate), "glStencilMaskSeparate"},
		{reinterpret_cast<void**>(&glStencilOp), "glStencilOp"},
		{reinterpret_cast<void**>(&glStencilOpSeparate), "glStencilOpSeparate"},
		{reinterpret_cast<void**>(&glTexBuffer), "glTexBuffer"},
		{reinterpret_cast<void**>(&glTexCoord1d), "glTexCoord1d"},
		{reinterpret_cast<void**>(&glTexCoord1dv), "glTexCoord1dv"},
		{reinterpret_cast<void**>(&glTexCoord1f), "glTexCoord1f"},
		{reinterpret_cast<void**>(&glTexCoord1fv), "glTexCoord1fv"},
		{reinterpret_cast<void**>(&glTexCoord1i), "glTexCoord1i"},
		{reinterpret_cast<void**>(&glTexCoord1iv), "glTexCoord1iv"},
		{reinterpret_cast<void**>(&glTexCoord1s), "glTexCoord1s"},
		{reinterpret_cast<void**>(&glTexCoord1sv), "glTexCoord1sv"},
		{reinterpret_cast<void**>(&glTexCoord2d), "glTexCoord2d"},
		{reinterpret_cast<void**>(&glTexCoord2dv), "glTexCoord2dv"},
		{reinterpret_cast<void**>(&glTexCoord2f), "glTexCoord2f"},
		{reinterpret_cast<void**>(&glTexCoord2fv), "glTexCoord2fv"},
		{reinterpret_cast<void**>(&glTexCoord2i), "glTexCoord2i"},
		{reinterpret_cast<void**>(&glTexCoord2iv), "glTexCoord2iv"},
		{reinterpret_cast<void**>(&glTexCoord2s), "glTexCoord2s"},
		{reinterpret_cast<void**>(&glTexCoord2sv), "glTexCoord2sv"},
		{reinterpret_cast<void**>(&glTexCoord3d), "glTexCoord3d"},
		{reinterpret_cast<void**>(&glTexCoord3dv), "glTexCoord3dv"},
		{reinterpret_cast<void**>(&glTexCoord3f), "glTexCoord3f"},
		{reinterpret_cast<void**>(&glTexCoord3fv), "glTexCoord3fv"},
		{reinterpret_cast<void**>(&glTexCoord3i), "glTexCoord3i"},
		{reinterpret_cast<void**>(&glTexCoord3iv), "glTexCoord3iv"},
		{reinterpret_cast<void**>(&glTexCoord3s), "glTexCoord3s"},
		{reinterpret_cast<void**>(&glTexCoord3sv), "glTexCoord3sv"},
		{reinterpret_cast<void**>(&glTexCoord4d), "glTexCoord4d"},
		{reinterpret_cast<void**>(&glTexCoord4dv), "glTexCoord4dv"},
		{reinterpret_cast<void**>(&glTexCoord4f), "glTexCoord4f"},
		{reinterpret_cast<void**>(&glTexCoord4fv), "glTexCoord4fv"},
		{reinterpret_cast<void**>(&glTexCoord4i), "glTexCoord4i"},
		{reinterpret_cast<void**>(&glTexCoord4iv), "glTexCoord4iv"},
		{reinterpret_cast<void**>(&glTexCoord4s), "glTexCoord4s"},
		{reinterpret_cast<void**>(&glTexCoord4sv), "glTexCoord4sv"},
		{reinterpret_cast<void**>(&glTexCoordPointer), "glTexCoordPointer"},
		{reinterpret_cast<void**>(&glTexEnvf), "glTexEnvf"},
		{reinterpret_cast<void**>(&glTexEnvfv), "glTexEnvfv"},
		{reinterpret_cast<void**>(&glTexEnvi), "glTexEnvi"},
		{reinterpret_cast<void**>(&glTexEnviv), "glTexEnviv"},
		{reinterpret_cast<void**>(&glTexGend), "glTexGend"},
		{reinterpret_cast<void**>(&glTexGendv), "glTexGendv"},
		{reinterpret_cast<void**>(&glTexGenf), "glTexGenf"},
		{reinterpret_cast<void**>(&glTexGenfv), "glTexGenfv"},
		{reinterpret_cast<void**>(&glTexGeni), "glTexGeni"},
		{reinterpret_cast<void**>(&glTexGeniv), "glTexGeniv"},
		{reinterpret_cast<void**>(&glTexImage1D), "glTexImage1D"},
		{reinterpret_cast<void**>(&glTexImage2DMultisample), "glTexImage2DMultisample"},
		{reinterpret_cast<void**>(&glTexImage2D), "glTexImage2D"},
		{reinterpret_cast<void**>(&glTexImage3DMultisample), "glTexImage3DMultisample"},
		{reinterpret_cast<void**>(&glTexImage3D), "glTexImage3D"},
		{reinterpret_cast<void**>(&glTexParameterf), "glTexParameterf"},
		{reinterpret_cast<void**>(&glTexParameterfv), "glTexParameterfv"},
		{reinterpret_cast<void**>(&glTexParameterIiv), "glTexParameterIiv"},
		{reinterpret_cast<void**>(&glTexParameteri), "glTexParameteri"},
		{reinterpret_cast<void**>(&glTexParameterIuiv), "glTexParameterIuiv"},
		{reinterpret_cast<void**>(&glTexParameteriv), "glTexParameteriv"},
		{reinterpret_cast<void**>(&glTexSubImage1D), "glTexSubImage1D"},
		{reinterpret_cast<void**>(&glTexSubImage2D), "glTexSubImage2D"},
		{reinterpret_cast<void**>(&glTexSubImage3D), "glTexSubImage3D"},
		{reinterpret_cast<void**>(&glTextureBuffer), "glTextureBuffer"},
		{reinterpret_cast<void**>(&glTextureBufferRange), "glTextureBufferRange"},
		{reinterpret_cast<void**>(&glTextureParameterf), "glTextureParameterf"},
		{reinterpret_cast<void**>(&glTextureParameterfv), "glTextureParameterfv"},
		{reinterpret_cast<void**>(&glTextureParameterIiv), "glTextureParameterIiv"},
		{reinterpret_cast<void**>(&glTextureParameteri), "glTextureParameteri"},
		{reinterpret_cast<void**>(&glTextureParameterIuiv), "glTextureParameterIuiv"},
		{reinterpret_cast<void**>(&glTextureParameteriv), "glTextureParameteriv"},
		{reinterpret_cast<void**>(&glTextureStorage1D), "glTextureStorage1D"},
		{reinterpret_cast<void**>(&glTextureStorage2DMultisample), "glTextureStorage2DMultisample"},
		{reinterpret_cast<void**>(&glTextureStorage2D), "glTextureStorage2D"},
		{reinterpret_cast<void**>(&glTextureStorage3DMultisample), "glTextureStorage3DMultisample"},
		{reinterpret_cast<void**>(&glTextureStorage3D), "glTextureStorage3D"},
		{reinterpret_cast<void**>(&glTextureSubImage1D), "glTextureSubImage1D"},
		{reinterpret_cast<void**>(&glTextureSubImage2D), "glTextureSubImage2D"},
		{reinterpret_cast<void**>(&glTextureSubImage3D), "glTextureSubImage3D"},
		{reinterpret_cast<void**>(&glTransformFeedbackBufferBase), "glTransformFeedbackBufferBase"},
		{reinterpret_cast<void**>(&glTransformFeedbackBufferRange), "glTransformFeedbackBufferRange"},
		{reinterpret_cast<void**>(&glTransformFeedbackVaryings), "glTransformFeedbackVaryings"},
		{reinterpret_cast<void**>(&glTranslated), "glTranslated"},
		{reinterpret_cast<void**>(&glTranslatef), "glTranslatef"},
		{reinterpret_cast<void**>(&glUniform1f), "glUniform1f"},
		{reinterpret_cast<void**>(&glUniform1fv), "glUniform1fv"},
		{reinterpret_cast<void**>(&glUniform1i), "glUniform1i"},
		{reinterpret_cast<void**>(&glUniform1iv), "glUniform1iv"},
		{reinterpret_cast<void**>(&glUniform1ui), "glUniform1ui"},
		{reinterpret_cast<void**>(&glUniform1uiv), "glUniform1uiv"},
		{reinterpret_cast<void**>(&glUniform2f), "glUniform2f"},
		{reinterpret_cast<void**>(&glUniform2fv), "glUniform2fv"},
		{reinterpret_cast<void**>(&glUniform2i), "glUniform2i"},
		{reinterpret_cast<void**>(&glUniform2iv), "glUniform2iv"},
		{reinterpret_cast<void**>(&glUniform2ui), "glUniform2ui"},
		{reinterpret_cast<void**>(&glUniform2uiv), "glUniform2uiv"},
		{reinterpret_cast<void**>(&glUniform3f), "glUniform3f"},
		{reinterpret_cast<void**>(&glUniform3fv), "glUniform3fv"},
		{reinterpret_cast<void**>(&glUniform3i), "glUniform3i"},
		{reinterpret_cast<void**>(&glUniform3iv), "glUniform3iv"},
		{reinterpret_cast<void**>(&glUniform3ui), "glUniform3ui"},
		{reinterpret_cast<void**>(&glUniform3uiv), "glUniform3uiv"},
		{reinterpret_cast<void**>(&glUniform4f), "glUniform4f"},
		{reinterpret_cast<void**>(&glUniform4fv), "glUniform4fv"},
		{reinterpret_cast<void**>(&glUniform4i), "glUniform4i"},
		{reinterpret_cast<void**>(&glUniform4iv), "glUniform4iv"},
		{reinterpret_cast<void**>(&glUniform4ui), "glUniform4ui"},
		{reinterpret_cast<void**>(&glUniform4uiv), "glUniform4uiv"},
		{reinterpret_cast<void**>(&glUniformBlockBinding), "glUniformBlockBinding"},
		{reinterpret_cast<void**>(&glUniformMatrix2fv), "glUniformMatrix2fv"},
		{reinterpret_cast<void**>(&glUniformMatrix2x3fv), "glUniformMatrix2x3fv"},
		{reinterpret_cast<void**>(&glUniformMatrix2x4fv), "glUniformMatrix2x4fv"},
		{reinterpret_cast<void**>(&glUniformMatrix3fv), "glUniformMatrix3fv"},
		{reinterpret_cast<void**>(&glUniformMatrix3x2fv), "glUniformMatrix3x2fv"},
		{reinterpret_cast<void**>(&glUniformMatrix3x4fv), "glUniformMatrix3x4fv"},
		{reinterpret_cast<void**>(&glUniformMatrix4fv), "glUniformMatrix4fv"},
		{reinterpret_cast<void**>(&glUniformMatrix4x2fv), "glUniformMatrix4x2fv"},
		{reinterpret_cast<void**>(&glUniformMatrix4x3fv), "glUniformMatrix4x3fv"},
		{reinterpret_cast<void**>(&glUnmapBufferARB), "glUnmapBufferARB"},
		{reinterpret_cast<void**>(&glUnmapBuffer), "glUnmapBuffer"},
		{reinterpret_cast<void**>(&glUnmapNamedBuffer), "glUnmapNamedBuffer"},
		{reinterpret_cast<void**>(&glUseProgram), "glUseProgram"},
		{reinterpret_cast<void**>(&glUseProgramStages), "glUseProgramStages"},
		{reinterpret_cast<void**>(&glValidateProgramPipeline), "glValidateProgramPipeline"},
		{reinterpret_cast<void**>(&glValidateProgram), "glValidateProgram"},
		{reinterpret_cast<void**>(&glVertex2d), "glVertex2d"},
		{reinterpret_cast<void**>(&glVertex2dv), "glVertex2dv"},
		{reinterpret_cast<void**>(&glVertex2f), "glVertex2f"},
		{reinterpret_cast<void**>(&glVertex2fv), "glVertex2fv"},
		{reinterpret_cast<void**>(&glVertex2i), "glVertex2i"},
		{reinterpret_cast<void**>(&glVertex2iv), "glVertex2iv"},
		{reinterpret_cast<void**>(&glVertex2s), "glVertex2s"},
		{reinterpret_cast<void**>(&glVertex2sv), "glVertex2sv"},
		{reinterpret_cast<void**>(&glVertex3d), "glVertex3d"},
		{reinterpret_cast<void**>(&glVertex3dv), "glVertex3dv"},
		{reinterpret_cast<void**>(&glVertex3f), "glVertex3f"},
		{reinterpret_cast<void**>(&glVertex3fv), "glVertex3fv"},
		{reinterpret_cast<void**>(&glVertex3i), "glVertex3i"},
		{reinterpret_cast<void**>(&glVertex3iv), "glVertex3iv"},
		{reinterpret_cast<void**>(&glVertex3s), "glVertex3s"},
		{reinterpret_cast<void**>(&glVertex3sv), "glVertex3sv"},
		{reinterpret_cast<void**>(&glVertex4d), "glVertex4d"},
		{reinterpret_cast<void**>(&glVertex4dv), "glVertex4dv"},
		{reinterpret_cast<void**>(&glVertex4f), "glVertex4f"},
		{reinterpret_cast<void**>(&glVertex4fv), "glVertex4fv"},
		{reinterpret_cast<void**>(&glVertex4i), "glVertex4i"},
		{reinterpret_cast<void**>(&glVertex4iv), "glVertex4iv"},
		{reinterpret_cast<void**>(&glVertex4s), "glVertex4s"},
		{reinterpret_cast<void**>(&glVertex4sv), "glVertex4sv"},
		{reinterpret_cast<void**>(&glVertexArrayAttribBinding), "glVertexArrayAttribBinding"},
		{reinterpret_cast<void**>(&glVertexArrayAttribFormat), "glVertexArrayAttribFormat"},
		{reinterpret_cast<void**>(&glVertexArrayAttribIFormat), "glVertexArrayAttribIFormat"},
		{reinterpret_cast<void**>(&glVertexArrayAttribLFormat), "glVertexArrayAttribLFormat"},
		{reinterpret_cast<void**>(&glVertexArrayBindingDivisor), "glVertexArrayBindingDivisor"},
		{reinterpret_cast<void**>(&glVertexArrayElementBuffer), "glVertexArrayElementBuffer"},
		{reinterpret_cast<void**>(&glVertexArrayVertexBuffer), "glVertexArrayVertexBuffer"},
		{reinterpret_cast<void**>(&glVertexArrayVertexBuffers), "glVertexArrayVertexBuffers"},
		{reinterpret_cast<void**>(&glVertexAttrib1d), "glVertexAttrib1d"},
		{reinterpret_cast<void**>(&glVertexAttrib1dv), "glVertexAttrib1dv"},
		{reinterpret_cast<void**>(&glVertexAttrib1f), "glVertexAttrib1f"},
		{reinterpret_cast<void**>(&glVertexAttrib1fv), "glVertexAttrib1fv"},
		{reinterpret_cast<void**>(&glVertexAttrib1s), "glVertexAttrib1s"},
		{reinterpret_cast<void**>(&glVertexAttrib1sv), "glVertexAttrib1sv"},
		{reinterpret_cast<void**>(&glVertexAttrib2d), "glVertexAttrib2d"},
		{reinterpret_cast<void**>(&glVertexAttrib2dv), "glVertexAttrib2dv"},
		{reinterpret_cast<void**>(&glVertexAttrib2f), "glVertexAttrib2f"},
		{reinterpret_cast<void**>(&glVertexAttrib2fv), "glVertexAttrib2fv"},
		{reinterpret_cast<void**>(&glVertexAttrib2s), "glVertexAttrib2s"},
		{reinterpret_cast<void**>(&glVertexAttrib2sv), "glVertexAttrib2sv"},
		{reinterpret_cast<void**>(&glVertexAttrib3d), "glVertexAttrib3d"},
		{reinterpret_cast<void**>(&glVertexAttrib3dv), "glVertexAttrib3dv"},
		{reinterpret_cast<void**>(&glVertexAttrib3f), "glVertexAttrib3f"},
		{reinterpret_cast<void**>(&glVertexAttrib3fv), "glVertexAttrib3fv"},
		{reinterpret_cast<void**>(&glVertexAttrib3s), "glVertexAttrib3s"},
		{reinterpret_cast<void**>(&glVertexAttrib3sv), "glVertexAttrib3sv"},
		{reinterpret_cast<void**>(&glVertexAttrib4bv), "glVertexAttrib4bv"},
		{reinterpret_cast<void**>(&glVertexAttrib4d), "glVertexAttrib4d"},
		{reinterpret_cast<void**>(&glVertexAttrib4dv), "glVertexAttrib4dv"},
		{reinterpret_cast<void**>(&glVertexAttrib4f), "glVertexAttrib4f"},
		{reinterpret_cast<void**>(&glVertexAttrib4fv), "glVertexAttrib4fv"},
		{reinterpret_cast<void**>(&glVertexAttrib4iv), "glVertexAttrib4iv"},
		{reinterpret_cast<void**>(&glVertexAttrib4Nbv), "glVertexAttrib4Nbv"},
		{reinterpret_cast<void**>(&glVertexAttrib4Niv), "glVertexAttrib4Niv"},
		{reinterpret_cast<void**>(&glVertexAttrib4Nsv), "glVertexAttrib4Nsv"},
		{reinterpret_cast<void**>(&glVertexAttrib4Nub), "glVertexAttrib4Nub"},
		{reinterpret_cast<void**>(&glVertexAttrib4Nubv), "glVertexAttrib4Nubv"},
		{reinterpret_cast<void**>(&glVertexAttrib4Nuiv), "glVertexAttrib4Nuiv"},
		{reinterpret_cast<void**>(&glVertexAttrib4Nusv), "glVertexAttrib4Nusv"},
		{reinterpret_cast<void**>(&glVertexAttrib4s), "glVertexAttrib4s"},
		{reinterpret_cast<void**>(&glVertexAttrib4sv), "glVertexAttrib4sv"},
		{reinterpret_cast<void**>(&glVertexAttrib4ubv), "glVertexAttrib4ubv"},
		{reinterpret_cast<void**>(&glVertexAttrib4uiv), "glVertexAttrib4uiv"},
		{reinterpret_cast<void**>(&glVertexAttrib4usv), "glVertexAttrib4usv"},
		{reinterpret_cast<void**>(&glVertexAttribI1i), "glVertexAttribI1i"},
		{reinterpret_cast<void**>(&glVertexAttribI1iv), "glVertexAttribI1iv"},
		{reinterpret_cast<void**>(&glVertexAttribI1ui), "glVertexAttribI1ui"},
		{reinterpret_cast<void**>(&glVertexAttribI1uiv), "glVertexAttribI1uiv"},
		{reinterpret_cast<void**>(&glVertexAttribI2i), "glVertexAttribI2i"},
		{reinterpret_cast<void**>(&glVertexAttribI2iv), "glVertexAttribI2iv"},
		{reinterpret_cast<void**>(&glVertexAttribI2ui), "glVertexAttribI2ui"},
		{reinterpret_cast<void**>(&glVertexAttribI2uiv), "glVertexAttribI2uiv"},
		{reinterpret_cast<void**>(&glVertexAttribI3i), "glVertexAttribI3i"},
		{reinterpret_cast<void**>(&glVertexAttribI3iv), "glVertexAttribI3iv"},
		{reinterpret_cast<void**>(&glVertexAttribI3ui), "glVertexAttribI3ui"},
		{reinterpret_cast<void**>(&glVertexAttribI3uiv), "glVertexAttribI3uiv"},
		{reinterpret_cast<void**>(&glVertexAttribI4bv), "glVertexAttribI4bv"},
		{reinterpret_cast<void**>(&glVertexAttribI4i), "glVertexAttribI4i"},
		{reinterpret_cast<void**>(&glVertexAttribI4iv), "glVertexAttribI4iv"},
		{reinterpret_cast<void**>(&glVertexAttribI4sv), "glVertexAttribI4sv"},
		{reinterpret_cast<void**>(&glVertexAttribI4ubv), "glVertexAttribI4ubv"},
		{reinterpret_cast<void**>(&glVertexAttribI4ui), "glVertexAttribI4ui"},
		{reinterpret_cast<void**>(&glVertexAttribI4uiv), "glVertexAttribI4uiv"},
		{reinterpret_cast<void**>(&glVertexAttribI4usv), "glVertexAttribI4usv"},
		{reinterpret_cast<void**>(&glVertexAttribIPointer), "glVertexAttribIPointer"},
		{reinterpret_cast<void**>(&glVertexAttribPointer), "glVertexAttribPointer"},
		{reinterpret_cast<void**>(&glVertexPointer), "glVertexPointer"},
		{reinterpret_cast<void**>(&glViewport), "glViewport"},
		{reinterpret_cast<void**>(&glWaitSync), "glWaitSync"},
		{reinterpret_cast<void**>(&glWindowPos2d), "glWindowPos2d"},
		{reinterpret_cast<void**>(&glWindowPos2dv), "glWindowPos2dv"},
		{reinterpret_cast<void**>(&glWindowPos2f), "glWindowPos2f"},
		{reinterpret_cast<void**>(&glWindowPos2fv), "glWindowPos2fv"},
		{reinterpret_cast<void**>(&glWindowPos2i), "glWindowPos2i"},
		{reinterpret_cast<void**>(&glWindowPos2iv), "glWindowPos2iv"},
		{reinterpret_cast<void**>(&glWindowPos2s), "glWindowPos2s"},
		{reinterpret_cast<void**>(&glWindowPos2sv), "glWindowPos2sv"},
		{reinterpret_cast<void**>(&glWindowPos3d), "glWindowPos3d"},
		{reinterpret_cast<void**>(&glWindowPos3dv), "glWindowPos3dv"},
		{reinterpret_cast<void**>(&glWindowPos3f), "glWindowPos3f"},
		{reinterpret_cast<void**>(&glWindowPos3fv), "glWindowPos3fv"},
		{reinterpret_cast<void**>(&glWindowPos3i), "glWindowPos3i"},
		{reinterpret_cast<void**>(&glWindowPos3iv), "glWindowPos3iv"},
		{reinterpret_cast<void**>(&glWindowPos3s), "glWindowPos3s"},
		{reinterpret_cast<void**>(&glWindowPos3sv), "glWindowPos3sv"},
	};

	return gl_symbol_registry;
}

void Ren3dGlExtensionMgrImpl::clear_gl_symbols()
{
	auto& gl_symbol_registry = get_gl_symbol_registry();

	for (auto& gl_symbol_item : gl_symbol_registry)
	{
		*gl_symbol_item.first = nullptr;
	}
}

void Ren3dGlExtensionMgrImpl::resolve_gl_symbols()
{
	auto& gl_symbol_registry = get_gl_symbol_registry();

	for (auto& gl_symbol_item : gl_symbol_registry)
	{
		*gl_symbol_item.first = Ren3dGlUtils::resolve_symbol(gl_symbol_item.second);
	}
}

bool Ren3dGlExtensionMgrImpl::has_gl_symbol(
	const GlSymbolPtrs& gl_symbol_ptrs)
{
	return std::all_of(
		gl_symbol_ptrs.cbegin(),
		gl_symbol_ptrs.cend(),
		[](const auto item)
		{
			return *item;
		}
	);
}

Ren3dGlExtensionMgrImpl::GlSymbolPtrs& Ren3dGlExtensionMgrImpl::get_essentials_gl_symbol_ptrs()
{
	static auto gl_symbols = GlSymbolPtrs
	{
		reinterpret_cast<void**>(&glGetError),
		reinterpret_cast<void**>(&glGetIntegerv),
	};

	return gl_symbols;
}

Ren3dGlExtensionMgrImpl::GlSymbolPtrs& Ren3dGlExtensionMgrImpl::get_v2_0_gl_symbol_ptrs()
{
	static auto gl_symbols = GlSymbolPtrs
	{
		reinterpret_cast<void**>(&glAccum),
		reinterpret_cast<void**>(&glActiveTexture),
		reinterpret_cast<void**>(&glAlphaFunc),
		reinterpret_cast<void**>(&glAreTexturesResident),
		reinterpret_cast<void**>(&glArrayElement),
		reinterpret_cast<void**>(&glAttachShader),
		reinterpret_cast<void**>(&glBegin),
		reinterpret_cast<void**>(&glBeginQuery),
		reinterpret_cast<void**>(&glBindAttribLocation),
		reinterpret_cast<void**>(&glBindBuffer),
		reinterpret_cast<void**>(&glBindTexture),
		reinterpret_cast<void**>(&glBitmap),
		reinterpret_cast<void**>(&glBlendColor),
		reinterpret_cast<void**>(&glBlendEquation),
		reinterpret_cast<void**>(&glBlendEquationSeparate),
		reinterpret_cast<void**>(&glBlendFunc),
		reinterpret_cast<void**>(&glBlendFuncSeparate),
		reinterpret_cast<void**>(&glBufferData),
		reinterpret_cast<void**>(&glBufferSubData),
		reinterpret_cast<void**>(&glCallList),
		reinterpret_cast<void**>(&glCallLists),
		reinterpret_cast<void**>(&glClear),
		reinterpret_cast<void**>(&glClearAccum),
		reinterpret_cast<void**>(&glClearColor),
		reinterpret_cast<void**>(&glClearDepth),
		reinterpret_cast<void**>(&glClearIndex),
		reinterpret_cast<void**>(&glClearStencil),
		reinterpret_cast<void**>(&glClientActiveTexture),
		reinterpret_cast<void**>(&glClipPlane),
		reinterpret_cast<void**>(&glColor3b),
		reinterpret_cast<void**>(&glColor3bv),
		reinterpret_cast<void**>(&glColor3d),
		reinterpret_cast<void**>(&glColor3dv),
		reinterpret_cast<void**>(&glColor3f),
		reinterpret_cast<void**>(&glColor3fv),
		reinterpret_cast<void**>(&glColor3i),
		reinterpret_cast<void**>(&glColor3iv),
		reinterpret_cast<void**>(&glColor3s),
		reinterpret_cast<void**>(&glColor3sv),
		reinterpret_cast<void**>(&glColor3ub),
		reinterpret_cast<void**>(&glColor3ubv),
		reinterpret_cast<void**>(&glColor3ui),
		reinterpret_cast<void**>(&glColor3uiv),
		reinterpret_cast<void**>(&glColor3us),
		reinterpret_cast<void**>(&glColor3usv),
		reinterpret_cast<void**>(&glColor4b),
		reinterpret_cast<void**>(&glColor4bv),
		reinterpret_cast<void**>(&glColor4d),
		reinterpret_cast<void**>(&glColor4dv),
		reinterpret_cast<void**>(&glColor4f),
		reinterpret_cast<void**>(&glColor4fv),
		reinterpret_cast<void**>(&glColor4i),
		reinterpret_cast<void**>(&glColor4iv),
		reinterpret_cast<void**>(&glColor4s),
		reinterpret_cast<void**>(&glColor4sv),
		reinterpret_cast<void**>(&glColor4ub),
		reinterpret_cast<void**>(&glColor4ubv),
		reinterpret_cast<void**>(&glColor4ui),
		reinterpret_cast<void**>(&glColor4uiv),
		reinterpret_cast<void**>(&glColor4us),
		reinterpret_cast<void**>(&glColor4usv),
		reinterpret_cast<void**>(&glColorMask),
		reinterpret_cast<void**>(&glColorMaterial),
		reinterpret_cast<void**>(&glColorPointer),
		reinterpret_cast<void**>(&glCompileShader),
		reinterpret_cast<void**>(&glCompressedTexImage1D),
		reinterpret_cast<void**>(&glCompressedTexImage2D),
		reinterpret_cast<void**>(&glCompressedTexImage3D),
		reinterpret_cast<void**>(&glCompressedTexSubImage1D),
		reinterpret_cast<void**>(&glCompressedTexSubImage2D),
		reinterpret_cast<void**>(&glCompressedTexSubImage3D),
		reinterpret_cast<void**>(&glCopyPixels),
		reinterpret_cast<void**>(&glCopyTexImage1D),
		reinterpret_cast<void**>(&glCopyTexImage2D),
		reinterpret_cast<void**>(&glCopyTexSubImage1D),
		reinterpret_cast<void**>(&glCopyTexSubImage2D),
		reinterpret_cast<void**>(&glCopyTexSubImage3D),
		reinterpret_cast<void**>(&glCreateProgram),
		reinterpret_cast<void**>(&glCreateShader),
		reinterpret_cast<void**>(&glCullFace),
		reinterpret_cast<void**>(&glDeleteBuffers),
		reinterpret_cast<void**>(&glDeleteLists),
		reinterpret_cast<void**>(&glDeleteProgram),
		reinterpret_cast<void**>(&glDeleteQueries),
		reinterpret_cast<void**>(&glDeleteShader),
		reinterpret_cast<void**>(&glDeleteTextures),
		reinterpret_cast<void**>(&glDepthFunc),
		reinterpret_cast<void**>(&glDepthMask),
		reinterpret_cast<void**>(&glDepthRange),
		reinterpret_cast<void**>(&glDetachShader),
		reinterpret_cast<void**>(&glDisable),
		reinterpret_cast<void**>(&glDisableClientState),
		reinterpret_cast<void**>(&glDisableVertexAttribArray),
		reinterpret_cast<void**>(&glDrawArrays),
		reinterpret_cast<void**>(&glDrawBuffer),
		reinterpret_cast<void**>(&glDrawBuffers),
		reinterpret_cast<void**>(&glDrawElements),
		reinterpret_cast<void**>(&glDrawPixels),
		reinterpret_cast<void**>(&glDrawRangeElements),
		reinterpret_cast<void**>(&glEdgeFlag),
		reinterpret_cast<void**>(&glEdgeFlagPointer),
		reinterpret_cast<void**>(&glEdgeFlagv),
		reinterpret_cast<void**>(&glEnable),
		reinterpret_cast<void**>(&glEnableClientState),
		reinterpret_cast<void**>(&glEnableVertexAttribArray),
		reinterpret_cast<void**>(&glEnd),
		reinterpret_cast<void**>(&glEndList),
		reinterpret_cast<void**>(&glEndQuery),
		reinterpret_cast<void**>(&glEvalCoord1d),
		reinterpret_cast<void**>(&glEvalCoord1dv),
		reinterpret_cast<void**>(&glEvalCoord1f),
		reinterpret_cast<void**>(&glEvalCoord1fv),
		reinterpret_cast<void**>(&glEvalCoord2d),
		reinterpret_cast<void**>(&glEvalCoord2dv),
		reinterpret_cast<void**>(&glEvalCoord2f),
		reinterpret_cast<void**>(&glEvalCoord2fv),
		reinterpret_cast<void**>(&glEvalMesh1),
		reinterpret_cast<void**>(&glEvalMesh2),
		reinterpret_cast<void**>(&glEvalPoint1),
		reinterpret_cast<void**>(&glEvalPoint2),
		reinterpret_cast<void**>(&glFeedbackBuffer),
		reinterpret_cast<void**>(&glFinish),
		reinterpret_cast<void**>(&glFlush),
		reinterpret_cast<void**>(&glFogCoordPointer),
		reinterpret_cast<void**>(&glFogCoordd),
		reinterpret_cast<void**>(&glFogCoorddv),
		reinterpret_cast<void**>(&glFogCoordf),
		reinterpret_cast<void**>(&glFogCoordfv),
		reinterpret_cast<void**>(&glFogf),
		reinterpret_cast<void**>(&glFogfv),
		reinterpret_cast<void**>(&glFogi),
		reinterpret_cast<void**>(&glFogiv),
		reinterpret_cast<void**>(&glFrontFace),
		reinterpret_cast<void**>(&glFrustum),
		reinterpret_cast<void**>(&glGenBuffers),
		reinterpret_cast<void**>(&glGenLists),
		reinterpret_cast<void**>(&glGenQueries),
		reinterpret_cast<void**>(&glGenTextures),
		reinterpret_cast<void**>(&glGetActiveAttrib),
		reinterpret_cast<void**>(&glGetActiveUniform),
		reinterpret_cast<void**>(&glGetAttachedShaders),
		reinterpret_cast<void**>(&glGetAttribLocation),
		reinterpret_cast<void**>(&glGetBooleanv),
		reinterpret_cast<void**>(&glGetBufferParameteriv),
		reinterpret_cast<void**>(&glGetBufferPointerv),
		reinterpret_cast<void**>(&glGetBufferSubData),
		reinterpret_cast<void**>(&glGetClipPlane),
		reinterpret_cast<void**>(&glGetCompressedTexImage),
		reinterpret_cast<void**>(&glGetDoublev),
		reinterpret_cast<void**>(&glGetError),
		reinterpret_cast<void**>(&glGetFloatv),
		reinterpret_cast<void**>(&glGetIntegerv),
		reinterpret_cast<void**>(&glGetLightfv),
		reinterpret_cast<void**>(&glGetLightiv),
		reinterpret_cast<void**>(&glGetMapdv),
		reinterpret_cast<void**>(&glGetMapfv),
		reinterpret_cast<void**>(&glGetMapiv),
		reinterpret_cast<void**>(&glGetMaterialfv),
		reinterpret_cast<void**>(&glGetMaterialiv),
		reinterpret_cast<void**>(&glGetPixelMapfv),
		reinterpret_cast<void**>(&glGetPixelMapuiv),
		reinterpret_cast<void**>(&glGetPixelMapusv),
		reinterpret_cast<void**>(&glGetPointerv),
		reinterpret_cast<void**>(&glGetPolygonStipple),
		reinterpret_cast<void**>(&glGetProgramInfoLog),
		reinterpret_cast<void**>(&glGetProgramiv),
		reinterpret_cast<void**>(&glGetQueryObjectiv),
		reinterpret_cast<void**>(&glGetQueryObjectuiv),
		reinterpret_cast<void**>(&glGetQueryiv),
		reinterpret_cast<void**>(&glGetShaderInfoLog),
		reinterpret_cast<void**>(&glGetShaderSource),
		reinterpret_cast<void**>(&glGetShaderiv),
		reinterpret_cast<void**>(&glGetString),
		reinterpret_cast<void**>(&glGetTexEnvfv),
		reinterpret_cast<void**>(&glGetTexEnviv),
		reinterpret_cast<void**>(&glGetTexGendv),
		reinterpret_cast<void**>(&glGetTexGenfv),
		reinterpret_cast<void**>(&glGetTexGeniv),
		reinterpret_cast<void**>(&glGetTexImage),
		reinterpret_cast<void**>(&glGetTexLevelParameterfv),
		reinterpret_cast<void**>(&glGetTexLevelParameteriv),
		reinterpret_cast<void**>(&glGetTexParameterfv),
		reinterpret_cast<void**>(&glGetTexParameteriv),
		reinterpret_cast<void**>(&glGetUniformLocation),
		reinterpret_cast<void**>(&glGetUniformfv),
		reinterpret_cast<void**>(&glGetUniformiv),
		reinterpret_cast<void**>(&glGetVertexAttribPointerv),
		reinterpret_cast<void**>(&glGetVertexAttribdv),
		reinterpret_cast<void**>(&glGetVertexAttribfv),
		reinterpret_cast<void**>(&glGetVertexAttribiv),
		reinterpret_cast<void**>(&glHint),
		reinterpret_cast<void**>(&glIndexMask),
		reinterpret_cast<void**>(&glIndexPointer),
		reinterpret_cast<void**>(&glIndexd),
		reinterpret_cast<void**>(&glIndexdv),
		reinterpret_cast<void**>(&glIndexf),
		reinterpret_cast<void**>(&glIndexfv),
		reinterpret_cast<void**>(&glIndexi),
		reinterpret_cast<void**>(&glIndexiv),
		reinterpret_cast<void**>(&glIndexs),
		reinterpret_cast<void**>(&glIndexsv),
		reinterpret_cast<void**>(&glIndexub),
		reinterpret_cast<void**>(&glIndexubv),
		reinterpret_cast<void**>(&glInitNames),
		reinterpret_cast<void**>(&glInterleavedArrays),
		reinterpret_cast<void**>(&glIsBuffer),
		reinterpret_cast<void**>(&glIsEnabled),
		reinterpret_cast<void**>(&glIsList),
		reinterpret_cast<void**>(&glIsProgram),
		reinterpret_cast<void**>(&glIsQuery),
		reinterpret_cast<void**>(&glIsShader),
		reinterpret_cast<void**>(&glIsTexture),
		reinterpret_cast<void**>(&glLightModelf),
		reinterpret_cast<void**>(&glLightModelfv),
		reinterpret_cast<void**>(&glLightModeli),
		reinterpret_cast<void**>(&glLightModeliv),
		reinterpret_cast<void**>(&glLightf),
		reinterpret_cast<void**>(&glLightfv),
		reinterpret_cast<void**>(&glLighti),
		reinterpret_cast<void**>(&glLightiv),
		reinterpret_cast<void**>(&glLineStipple),
		reinterpret_cast<void**>(&glLineWidth),
		reinterpret_cast<void**>(&glLinkProgram),
		reinterpret_cast<void**>(&glListBase),
		reinterpret_cast<void**>(&glLoadIdentity),
		reinterpret_cast<void**>(&glLoadMatrixd),
		reinterpret_cast<void**>(&glLoadMatrixf),
		reinterpret_cast<void**>(&glLoadName),
		reinterpret_cast<void**>(&glLoadTransposeMatrixd),
		reinterpret_cast<void**>(&glLoadTransposeMatrixf),
		reinterpret_cast<void**>(&glLogicOp),
		reinterpret_cast<void**>(&glMap1d),
		reinterpret_cast<void**>(&glMap1f),
		reinterpret_cast<void**>(&glMap2d),
		reinterpret_cast<void**>(&glMap2f),
		reinterpret_cast<void**>(&glMapBuffer),
		reinterpret_cast<void**>(&glMapGrid1d),
		reinterpret_cast<void**>(&glMapGrid1f),
		reinterpret_cast<void**>(&glMapGrid2d),
		reinterpret_cast<void**>(&glMapGrid2f),
		reinterpret_cast<void**>(&glMaterialf),
		reinterpret_cast<void**>(&glMaterialfv),
		reinterpret_cast<void**>(&glMateriali),
		reinterpret_cast<void**>(&glMaterialiv),
		reinterpret_cast<void**>(&glMatrixMode),
		reinterpret_cast<void**>(&glMultMatrixd),
		reinterpret_cast<void**>(&glMultMatrixf),
		reinterpret_cast<void**>(&glMultTransposeMatrixd),
		reinterpret_cast<void**>(&glMultTransposeMatrixf),
		reinterpret_cast<void**>(&glMultiDrawArrays),
		reinterpret_cast<void**>(&glMultiDrawElements),
		reinterpret_cast<void**>(&glMultiTexCoord1d),
		reinterpret_cast<void**>(&glMultiTexCoord1dv),
		reinterpret_cast<void**>(&glMultiTexCoord1f),
		reinterpret_cast<void**>(&glMultiTexCoord1fv),
		reinterpret_cast<void**>(&glMultiTexCoord1i),
		reinterpret_cast<void**>(&glMultiTexCoord1iv),
		reinterpret_cast<void**>(&glMultiTexCoord1s),
		reinterpret_cast<void**>(&glMultiTexCoord1sv),
		reinterpret_cast<void**>(&glMultiTexCoord2d),
		reinterpret_cast<void**>(&glMultiTexCoord2dv),
		reinterpret_cast<void**>(&glMultiTexCoord2f),
		reinterpret_cast<void**>(&glMultiTexCoord2fv),
		reinterpret_cast<void**>(&glMultiTexCoord2i),
		reinterpret_cast<void**>(&glMultiTexCoord2iv),
		reinterpret_cast<void**>(&glMultiTexCoord2s),
		reinterpret_cast<void**>(&glMultiTexCoord2sv),
		reinterpret_cast<void**>(&glMultiTexCoord3d),
		reinterpret_cast<void**>(&glMultiTexCoord3dv),
		reinterpret_cast<void**>(&glMultiTexCoord3f),
		reinterpret_cast<void**>(&glMultiTexCoord3fv),
		reinterpret_cast<void**>(&glMultiTexCoord3i),
		reinterpret_cast<void**>(&glMultiTexCoord3iv),
		reinterpret_cast<void**>(&glMultiTexCoord3s),
		reinterpret_cast<void**>(&glMultiTexCoord3sv),
		reinterpret_cast<void**>(&glMultiTexCoord4d),
		reinterpret_cast<void**>(&glMultiTexCoord4dv),
		reinterpret_cast<void**>(&glMultiTexCoord4f),
		reinterpret_cast<void**>(&glMultiTexCoord4fv),
		reinterpret_cast<void**>(&glMultiTexCoord4i),
		reinterpret_cast<void**>(&glMultiTexCoord4iv),
		reinterpret_cast<void**>(&glMultiTexCoord4s),
		reinterpret_cast<void**>(&glMultiTexCoord4sv),
		reinterpret_cast<void**>(&glNewList),
		reinterpret_cast<void**>(&glNormal3b),
		reinterpret_cast<void**>(&glNormal3bv),
		reinterpret_cast<void**>(&glNormal3d),
		reinterpret_cast<void**>(&glNormal3dv),
		reinterpret_cast<void**>(&glNormal3f),
		reinterpret_cast<void**>(&glNormal3fv),
		reinterpret_cast<void**>(&glNormal3i),
		reinterpret_cast<void**>(&glNormal3iv),
		reinterpret_cast<void**>(&glNormal3s),
		reinterpret_cast<void**>(&glNormal3sv),
		reinterpret_cast<void**>(&glNormalPointer),
		reinterpret_cast<void**>(&glOrtho),
		reinterpret_cast<void**>(&glPassThrough),
		reinterpret_cast<void**>(&glPixelMapfv),
		reinterpret_cast<void**>(&glPixelMapuiv),
		reinterpret_cast<void**>(&glPixelMapusv),
		reinterpret_cast<void**>(&glPixelStoref),
		reinterpret_cast<void**>(&glPixelStorei),
		reinterpret_cast<void**>(&glPixelTransferf),
		reinterpret_cast<void**>(&glPixelTransferi),
		reinterpret_cast<void**>(&glPixelZoom),
		reinterpret_cast<void**>(&glPointParameterf),
		reinterpret_cast<void**>(&glPointParameterfv),
		reinterpret_cast<void**>(&glPointParameteri),
		reinterpret_cast<void**>(&glPointParameteriv),
		reinterpret_cast<void**>(&glPointSize),
		reinterpret_cast<void**>(&glPolygonMode),
		reinterpret_cast<void**>(&glPolygonOffset),
		reinterpret_cast<void**>(&glPolygonStipple),
		reinterpret_cast<void**>(&glPopAttrib),
		reinterpret_cast<void**>(&glPopClientAttrib),
		reinterpret_cast<void**>(&glPopMatrix),
		reinterpret_cast<void**>(&glPopName),
		reinterpret_cast<void**>(&glPrioritizeTextures),
		reinterpret_cast<void**>(&glPushAttrib),
		reinterpret_cast<void**>(&glPushClientAttrib),
		reinterpret_cast<void**>(&glPushMatrix),
		reinterpret_cast<void**>(&glPushName),
		reinterpret_cast<void**>(&glRasterPos2d),
		reinterpret_cast<void**>(&glRasterPos2dv),
		reinterpret_cast<void**>(&glRasterPos2f),
		reinterpret_cast<void**>(&glRasterPos2fv),
		reinterpret_cast<void**>(&glRasterPos2i),
		reinterpret_cast<void**>(&glRasterPos2iv),
		reinterpret_cast<void**>(&glRasterPos2s),
		reinterpret_cast<void**>(&glRasterPos2sv),
		reinterpret_cast<void**>(&glRasterPos3d),
		reinterpret_cast<void**>(&glRasterPos3dv),
		reinterpret_cast<void**>(&glRasterPos3f),
		reinterpret_cast<void**>(&glRasterPos3fv),
		reinterpret_cast<void**>(&glRasterPos3i),
		reinterpret_cast<void**>(&glRasterPos3iv),
		reinterpret_cast<void**>(&glRasterPos3s),
		reinterpret_cast<void**>(&glRasterPos3sv),
		reinterpret_cast<void**>(&glRasterPos4d),
		reinterpret_cast<void**>(&glRasterPos4dv),
		reinterpret_cast<void**>(&glRasterPos4f),
		reinterpret_cast<void**>(&glRasterPos4fv),
		reinterpret_cast<void**>(&glRasterPos4i),
		reinterpret_cast<void**>(&glRasterPos4iv),
		reinterpret_cast<void**>(&glRasterPos4s),
		reinterpret_cast<void**>(&glRasterPos4sv),
		reinterpret_cast<void**>(&glReadBuffer),
		reinterpret_cast<void**>(&glReadPixels),
		reinterpret_cast<void**>(&glRectd),
		reinterpret_cast<void**>(&glRectdv),
		reinterpret_cast<void**>(&glRectf),
		reinterpret_cast<void**>(&glRectfv),
		reinterpret_cast<void**>(&glRecti),
		reinterpret_cast<void**>(&glRectiv),
		reinterpret_cast<void**>(&glRects),
		reinterpret_cast<void**>(&glRectsv),
		reinterpret_cast<void**>(&glRenderMode),
		reinterpret_cast<void**>(&glRotated),
		reinterpret_cast<void**>(&glRotatef),
		reinterpret_cast<void**>(&glSampleCoverage),
		reinterpret_cast<void**>(&glScaled),
		reinterpret_cast<void**>(&glScalef),
		reinterpret_cast<void**>(&glScissor),
		reinterpret_cast<void**>(&glSecondaryColor3b),
		reinterpret_cast<void**>(&glSecondaryColor3bv),
		reinterpret_cast<void**>(&glSecondaryColor3d),
		reinterpret_cast<void**>(&glSecondaryColor3dv),
		reinterpret_cast<void**>(&glSecondaryColor3f),
		reinterpret_cast<void**>(&glSecondaryColor3fv),
		reinterpret_cast<void**>(&glSecondaryColor3i),
		reinterpret_cast<void**>(&glSecondaryColor3iv),
		reinterpret_cast<void**>(&glSecondaryColor3s),
		reinterpret_cast<void**>(&glSecondaryColor3sv),
		reinterpret_cast<void**>(&glSecondaryColor3ub),
		reinterpret_cast<void**>(&glSecondaryColor3ubv),
		reinterpret_cast<void**>(&glSecondaryColor3ui),
		reinterpret_cast<void**>(&glSecondaryColor3uiv),
		reinterpret_cast<void**>(&glSecondaryColor3us),
		reinterpret_cast<void**>(&glSecondaryColor3usv),
		reinterpret_cast<void**>(&glSecondaryColorPointer),
		reinterpret_cast<void**>(&glSelectBuffer),
		reinterpret_cast<void**>(&glShadeModel),
		reinterpret_cast<void**>(&glShaderSource),
		reinterpret_cast<void**>(&glStencilFunc),
		reinterpret_cast<void**>(&glStencilFuncSeparate),
		reinterpret_cast<void**>(&glStencilMask),
		reinterpret_cast<void**>(&glStencilMaskSeparate),
		reinterpret_cast<void**>(&glStencilOp),
		reinterpret_cast<void**>(&glStencilOpSeparate),
		reinterpret_cast<void**>(&glTexCoord1d),
		reinterpret_cast<void**>(&glTexCoord1dv),
		reinterpret_cast<void**>(&glTexCoord1f),
		reinterpret_cast<void**>(&glTexCoord1fv),
		reinterpret_cast<void**>(&glTexCoord1i),
		reinterpret_cast<void**>(&glTexCoord1iv),
		reinterpret_cast<void**>(&glTexCoord1s),
		reinterpret_cast<void**>(&glTexCoord1sv),
		reinterpret_cast<void**>(&glTexCoord2d),
		reinterpret_cast<void**>(&glTexCoord2dv),
		reinterpret_cast<void**>(&glTexCoord2f),
		reinterpret_cast<void**>(&glTexCoord2fv),
		reinterpret_cast<void**>(&glTexCoord2i),
		reinterpret_cast<void**>(&glTexCoord2iv),
		reinterpret_cast<void**>(&glTexCoord2s),
		reinterpret_cast<void**>(&glTexCoord2sv),
		reinterpret_cast<void**>(&glTexCoord3d),
		reinterpret_cast<void**>(&glTexCoord3dv),
		reinterpret_cast<void**>(&glTexCoord3f),
		reinterpret_cast<void**>(&glTexCoord3fv),
		reinterpret_cast<void**>(&glTexCoord3i),
		reinterpret_cast<void**>(&glTexCoord3iv),
		reinterpret_cast<void**>(&glTexCoord3s),
		reinterpret_cast<void**>(&glTexCoord3sv),
		reinterpret_cast<void**>(&glTexCoord4d),
		reinterpret_cast<void**>(&glTexCoord4dv),
		reinterpret_cast<void**>(&glTexCoord4f),
		reinterpret_cast<void**>(&glTexCoord4fv),
		reinterpret_cast<void**>(&glTexCoord4i),
		reinterpret_cast<void**>(&glTexCoord4iv),
		reinterpret_cast<void**>(&glTexCoord4s),
		reinterpret_cast<void**>(&glTexCoord4sv),
		reinterpret_cast<void**>(&glTexCoordPointer),
		reinterpret_cast<void**>(&glTexEnvf),
		reinterpret_cast<void**>(&glTexEnvfv),
		reinterpret_cast<void**>(&glTexEnvi),
		reinterpret_cast<void**>(&glTexEnviv),
		reinterpret_cast<void**>(&glTexGend),
		reinterpret_cast<void**>(&glTexGendv),
		reinterpret_cast<void**>(&glTexGenf),
		reinterpret_cast<void**>(&glTexGenfv),
		reinterpret_cast<void**>(&glTexGeni),
		reinterpret_cast<void**>(&glTexGeniv),
		reinterpret_cast<void**>(&glTexImage1D),
		reinterpret_cast<void**>(&glTexImage2D),
		reinterpret_cast<void**>(&glTexImage3D),
		reinterpret_cast<void**>(&glTexParameterf),
		reinterpret_cast<void**>(&glTexParameterfv),
		reinterpret_cast<void**>(&glTexParameteri),
		reinterpret_cast<void**>(&glTexParameteriv),
		reinterpret_cast<void**>(&glTexSubImage1D),
		reinterpret_cast<void**>(&glTexSubImage2D),
		reinterpret_cast<void**>(&glTexSubImage3D),
		reinterpret_cast<void**>(&glTranslated),
		reinterpret_cast<void**>(&glTranslatef),
		reinterpret_cast<void**>(&glUniform1f),
		reinterpret_cast<void**>(&glUniform1fv),
		reinterpret_cast<void**>(&glUniform1i),
		reinterpret_cast<void**>(&glUniform1iv),
		reinterpret_cast<void**>(&glUniform2f),
		reinterpret_cast<void**>(&glUniform2fv),
		reinterpret_cast<void**>(&glUniform2i),
		reinterpret_cast<void**>(&glUniform2iv),
		reinterpret_cast<void**>(&glUniform3f),
		reinterpret_cast<void**>(&glUniform3fv),
		reinterpret_cast<void**>(&glUniform3i),
		reinterpret_cast<void**>(&glUniform3iv),
		reinterpret_cast<void**>(&glUniform4f),
		reinterpret_cast<void**>(&glUniform4fv),
		reinterpret_cast<void**>(&glUniform4i),
		reinterpret_cast<void**>(&glUniform4iv),
		reinterpret_cast<void**>(&glUniformMatrix2fv),
		reinterpret_cast<void**>(&glUniformMatrix3fv),
		reinterpret_cast<void**>(&glUniformMatrix4fv),
		reinterpret_cast<void**>(&glUnmapBuffer),
		reinterpret_cast<void**>(&glUseProgram),
		reinterpret_cast<void**>(&glValidateProgram),
		reinterpret_cast<void**>(&glVertex2d),
		reinterpret_cast<void**>(&glVertex2dv),
		reinterpret_cast<void**>(&glVertex2f),
		reinterpret_cast<void**>(&glVertex2fv),
		reinterpret_cast<void**>(&glVertex2i),
		reinterpret_cast<void**>(&glVertex2iv),
		reinterpret_cast<void**>(&glVertex2s),
		reinterpret_cast<void**>(&glVertex2sv),
		reinterpret_cast<void**>(&glVertex3d),
		reinterpret_cast<void**>(&glVertex3dv),
		reinterpret_cast<void**>(&glVertex3f),
		reinterpret_cast<void**>(&glVertex3fv),
		reinterpret_cast<void**>(&glVertex3i),
		reinterpret_cast<void**>(&glVertex3iv),
		reinterpret_cast<void**>(&glVertex3s),
		reinterpret_cast<void**>(&glVertex3sv),
		reinterpret_cast<void**>(&glVertex4d),
		reinterpret_cast<void**>(&glVertex4dv),
		reinterpret_cast<void**>(&glVertex4f),
		reinterpret_cast<void**>(&glVertex4fv),
		reinterpret_cast<void**>(&glVertex4i),
		reinterpret_cast<void**>(&glVertex4iv),
		reinterpret_cast<void**>(&glVertex4s),
		reinterpret_cast<void**>(&glVertex4sv),
		reinterpret_cast<void**>(&glVertexAttrib1d),
		reinterpret_cast<void**>(&glVertexAttrib1dv),
		reinterpret_cast<void**>(&glVertexAttrib1f),
		reinterpret_cast<void**>(&glVertexAttrib1fv),
		reinterpret_cast<void**>(&glVertexAttrib1s),
		reinterpret_cast<void**>(&glVertexAttrib1sv),
		reinterpret_cast<void**>(&glVertexAttrib2d),
		reinterpret_cast<void**>(&glVertexAttrib2dv),
		reinterpret_cast<void**>(&glVertexAttrib2f),
		reinterpret_cast<void**>(&glVertexAttrib2fv),
		reinterpret_cast<void**>(&glVertexAttrib2s),
		reinterpret_cast<void**>(&glVertexAttrib2sv),
		reinterpret_cast<void**>(&glVertexAttrib3d),
		reinterpret_cast<void**>(&glVertexAttrib3dv),
		reinterpret_cast<void**>(&glVertexAttrib3f),
		reinterpret_cast<void**>(&glVertexAttrib3fv),
		reinterpret_cast<void**>(&glVertexAttrib3s),
		reinterpret_cast<void**>(&glVertexAttrib3sv),
		reinterpret_cast<void**>(&glVertexAttrib4Nbv),
		reinterpret_cast<void**>(&glVertexAttrib4Niv),
		reinterpret_cast<void**>(&glVertexAttrib4Nsv),
		reinterpret_cast<void**>(&glVertexAttrib4Nub),
		reinterpret_cast<void**>(&glVertexAttrib4Nubv),
		reinterpret_cast<void**>(&glVertexAttrib4Nuiv),
		reinterpret_cast<void**>(&glVertexAttrib4Nusv),
		reinterpret_cast<void**>(&glVertexAttrib4bv),
		reinterpret_cast<void**>(&glVertexAttrib4d),
		reinterpret_cast<void**>(&glVertexAttrib4dv),
		reinterpret_cast<void**>(&glVertexAttrib4f),
		reinterpret_cast<void**>(&glVertexAttrib4fv),
		reinterpret_cast<void**>(&glVertexAttrib4iv),
		reinterpret_cast<void**>(&glVertexAttrib4s),
		reinterpret_cast<void**>(&glVertexAttrib4sv),
		reinterpret_cast<void**>(&glVertexAttrib4ubv),
		reinterpret_cast<void**>(&glVertexAttrib4uiv),
		reinterpret_cast<void**>(&glVertexAttrib4usv),
		reinterpret_cast<void**>(&glVertexAttribPointer),
		reinterpret_cast<void**>(&glVertexPointer),
		reinterpret_cast<void**>(&glViewport),
		reinterpret_cast<void**>(&glWindowPos2d),
		reinterpret_cast<void**>(&glWindowPos2dv),
		reinterpret_cast<void**>(&glWindowPos2f),
		reinterpret_cast<void**>(&glWindowPos2fv),
		reinterpret_cast<void**>(&glWindowPos2i),
		reinterpret_cast<void**>(&glWindowPos2iv),
		reinterpret_cast<void**>(&glWindowPos2s),
		reinterpret_cast<void**>(&glWindowPos2sv),
		reinterpret_cast<void**>(&glWindowPos3d),
		reinterpret_cast<void**>(&glWindowPos3dv),
		reinterpret_cast<void**>(&glWindowPos3f),
		reinterpret_cast<void**>(&glWindowPos3fv),
		reinterpret_cast<void**>(&glWindowPos3i),
		reinterpret_cast<void**>(&glWindowPos3iv),
		reinterpret_cast<void**>(&glWindowPos3s),
		reinterpret_cast<void**>(&glWindowPos3sv),
	};

	return gl_symbols;
}

Ren3dGlExtensionMgrImpl::GlSymbolPtrs& Ren3dGlExtensionMgrImpl::get_v3_2_core_gl_symbol_ptrs()
{
	static auto gl_symbols = GlSymbolPtrs
	{
		reinterpret_cast<void**>(&glActiveTexture),
		reinterpret_cast<void**>(&glAttachShader),
		reinterpret_cast<void**>(&glBeginConditionalRender),
		reinterpret_cast<void**>(&glBeginQuery),
		reinterpret_cast<void**>(&glBeginTransformFeedback),
		reinterpret_cast<void**>(&glBindAttribLocation),
		reinterpret_cast<void**>(&glBindBuffer),
		reinterpret_cast<void**>(&glBindBufferBase),
		reinterpret_cast<void**>(&glBindBufferRange),
		reinterpret_cast<void**>(&glBindFragDataLocation),
		reinterpret_cast<void**>(&glBindFramebuffer),
		reinterpret_cast<void**>(&glBindRenderbuffer),
		reinterpret_cast<void**>(&glBindTexture),
		reinterpret_cast<void**>(&glBindVertexArray),
		reinterpret_cast<void**>(&glBlendColor),
		reinterpret_cast<void**>(&glBlendEquation),
		reinterpret_cast<void**>(&glBlendEquationSeparate),
		reinterpret_cast<void**>(&glBlendFunc),
		reinterpret_cast<void**>(&glBlendFuncSeparate),
		reinterpret_cast<void**>(&glBlitFramebuffer),
		reinterpret_cast<void**>(&glBufferData),
		reinterpret_cast<void**>(&glBufferSubData),
		reinterpret_cast<void**>(&glCheckFramebufferStatus),
		reinterpret_cast<void**>(&glClampColor),
		reinterpret_cast<void**>(&glClear),
		reinterpret_cast<void**>(&glClearBufferfi),
		reinterpret_cast<void**>(&glClearBufferfv),
		reinterpret_cast<void**>(&glClearBufferiv),
		reinterpret_cast<void**>(&glClearBufferuiv),
		reinterpret_cast<void**>(&glClearColor),
		reinterpret_cast<void**>(&glClearDepth),
		reinterpret_cast<void**>(&glClearStencil),
		reinterpret_cast<void**>(&glClientWaitSync),
		reinterpret_cast<void**>(&glColorMask),
		reinterpret_cast<void**>(&glColorMaski),
		reinterpret_cast<void**>(&glCompileShader),
		reinterpret_cast<void**>(&glCompressedTexImage1D),
		reinterpret_cast<void**>(&glCompressedTexImage2D),
		reinterpret_cast<void**>(&glCompressedTexImage3D),
		reinterpret_cast<void**>(&glCompressedTexSubImage1D),
		reinterpret_cast<void**>(&glCompressedTexSubImage2D),
		reinterpret_cast<void**>(&glCompressedTexSubImage3D),
		reinterpret_cast<void**>(&glCopyBufferSubData),
		reinterpret_cast<void**>(&glCopyTexImage1D),
		reinterpret_cast<void**>(&glCopyTexImage2D),
		reinterpret_cast<void**>(&glCopyTexSubImage1D),
		reinterpret_cast<void**>(&glCopyTexSubImage2D),
		reinterpret_cast<void**>(&glCopyTexSubImage3D),
		reinterpret_cast<void**>(&glCreateProgram),
		reinterpret_cast<void**>(&glCreateShader),
		reinterpret_cast<void**>(&glCullFace),
		reinterpret_cast<void**>(&glDeleteBuffers),
		reinterpret_cast<void**>(&glDeleteFramebuffers),
		reinterpret_cast<void**>(&glDeleteProgram),
		reinterpret_cast<void**>(&glDeleteQueries),
		reinterpret_cast<void**>(&glDeleteRenderbuffers),
		reinterpret_cast<void**>(&glDeleteShader),
		reinterpret_cast<void**>(&glDeleteSync),
		reinterpret_cast<void**>(&glDeleteTextures),
		reinterpret_cast<void**>(&glDeleteVertexArrays),
		reinterpret_cast<void**>(&glDepthFunc),
		reinterpret_cast<void**>(&glDepthMask),
		reinterpret_cast<void**>(&glDepthRange),
		reinterpret_cast<void**>(&glDetachShader),
		reinterpret_cast<void**>(&glDisable),
		reinterpret_cast<void**>(&glDisableVertexAttribArray),
		reinterpret_cast<void**>(&glDisablei),
		reinterpret_cast<void**>(&glDrawArrays),
		reinterpret_cast<void**>(&glDrawArraysInstanced),
		reinterpret_cast<void**>(&glDrawBuffer),
		reinterpret_cast<void**>(&glDrawBuffers),
		reinterpret_cast<void**>(&glDrawElements),
		reinterpret_cast<void**>(&glDrawElementsBaseVertex),
		reinterpret_cast<void**>(&glDrawElementsInstanced),
		reinterpret_cast<void**>(&glDrawElementsInstancedBaseVertex),
		reinterpret_cast<void**>(&glDrawRangeElements),
		reinterpret_cast<void**>(&glDrawRangeElementsBaseVertex),
		reinterpret_cast<void**>(&glEnable),
		reinterpret_cast<void**>(&glEnableVertexAttribArray),
		reinterpret_cast<void**>(&glEnablei),
		reinterpret_cast<void**>(&glEndConditionalRender),
		reinterpret_cast<void**>(&glEndQuery),
		reinterpret_cast<void**>(&glEndTransformFeedback),
		reinterpret_cast<void**>(&glFenceSync),
		reinterpret_cast<void**>(&glFinish),
		reinterpret_cast<void**>(&glFlush),
		reinterpret_cast<void**>(&glFlushMappedBufferRange),
		reinterpret_cast<void**>(&glFramebufferRenderbuffer),
		reinterpret_cast<void**>(&glFramebufferTexture),
		reinterpret_cast<void**>(&glFramebufferTexture1D),
		reinterpret_cast<void**>(&glFramebufferTexture2D),
		reinterpret_cast<void**>(&glFramebufferTexture3D),
		reinterpret_cast<void**>(&glFramebufferTextureLayer),
		reinterpret_cast<void**>(&glFrontFace),
		reinterpret_cast<void**>(&glGenBuffers),
		reinterpret_cast<void**>(&glGenFramebuffers),
		reinterpret_cast<void**>(&glGenQueries),
		reinterpret_cast<void**>(&glGenRenderbuffers),
		reinterpret_cast<void**>(&glGenTextures),
		reinterpret_cast<void**>(&glGenVertexArrays),
		reinterpret_cast<void**>(&glGenerateMipmap),
		reinterpret_cast<void**>(&glGetActiveAttrib),
		reinterpret_cast<void**>(&glGetActiveUniform),
		reinterpret_cast<void**>(&glGetActiveUniformBlockName),
		reinterpret_cast<void**>(&glGetActiveUniformBlockiv),
		reinterpret_cast<void**>(&glGetActiveUniformName),
		reinterpret_cast<void**>(&glGetActiveUniformsiv),
		reinterpret_cast<void**>(&glGetAttachedShaders),
		reinterpret_cast<void**>(&glGetAttribLocation),
		reinterpret_cast<void**>(&glGetBooleani_v),
		reinterpret_cast<void**>(&glGetBooleanv),
		reinterpret_cast<void**>(&glGetBufferParameteri64v),
		reinterpret_cast<void**>(&glGetBufferParameteriv),
		reinterpret_cast<void**>(&glGetBufferPointerv),
		reinterpret_cast<void**>(&glGetBufferSubData),
		reinterpret_cast<void**>(&glGetCompressedTexImage),
		reinterpret_cast<void**>(&glGetDoublev),
		reinterpret_cast<void**>(&glGetError),
		reinterpret_cast<void**>(&glGetFloatv),
		reinterpret_cast<void**>(&glGetFragDataLocation),
		reinterpret_cast<void**>(&glGetFramebufferAttachmentParameteriv),
		reinterpret_cast<void**>(&glGetInteger64i_v),
		reinterpret_cast<void**>(&glGetInteger64v),
		reinterpret_cast<void**>(&glGetIntegeri_v),
		reinterpret_cast<void**>(&glGetIntegerv),
		reinterpret_cast<void**>(&glGetMultisamplefv),
		reinterpret_cast<void**>(&glGetProgramInfoLog),
		reinterpret_cast<void**>(&glGetProgramiv),
		reinterpret_cast<void**>(&glGetQueryObjectiv),
		reinterpret_cast<void**>(&glGetQueryObjectuiv),
		reinterpret_cast<void**>(&glGetQueryiv),
		reinterpret_cast<void**>(&glGetRenderbufferParameteriv),
		reinterpret_cast<void**>(&glGetShaderInfoLog),
		reinterpret_cast<void**>(&glGetShaderSource),
		reinterpret_cast<void**>(&glGetShaderiv),
		reinterpret_cast<void**>(&glGetString),
		reinterpret_cast<void**>(&glGetStringi),
		reinterpret_cast<void**>(&glGetSynciv),
		reinterpret_cast<void**>(&glGetTexImage),
		reinterpret_cast<void**>(&glGetTexLevelParameterfv),
		reinterpret_cast<void**>(&glGetTexLevelParameteriv),
		reinterpret_cast<void**>(&glGetTexParameterIiv),
		reinterpret_cast<void**>(&glGetTexParameterIuiv),
		reinterpret_cast<void**>(&glGetTexParameterfv),
		reinterpret_cast<void**>(&glGetTexParameteriv),
		reinterpret_cast<void**>(&glGetTransformFeedbackVarying),
		reinterpret_cast<void**>(&glGetUniformBlockIndex),
		reinterpret_cast<void**>(&glGetUniformIndices),
		reinterpret_cast<void**>(&glGetUniformLocation),
		reinterpret_cast<void**>(&glGetUniformfv),
		reinterpret_cast<void**>(&glGetUniformiv),
		reinterpret_cast<void**>(&glGetUniformuiv),
		reinterpret_cast<void**>(&glGetVertexAttribIiv),
		reinterpret_cast<void**>(&glGetVertexAttribIuiv),
		reinterpret_cast<void**>(&glGetVertexAttribPointerv),
		reinterpret_cast<void**>(&glGetVertexAttribdv),
		reinterpret_cast<void**>(&glGetVertexAttribfv),
		reinterpret_cast<void**>(&glGetVertexAttribiv),
		reinterpret_cast<void**>(&glHint),
		reinterpret_cast<void**>(&glIsBuffer),
		reinterpret_cast<void**>(&glIsEnabled),
		reinterpret_cast<void**>(&glIsEnabledi),
		reinterpret_cast<void**>(&glIsFramebuffer),
		reinterpret_cast<void**>(&glIsProgram),
		reinterpret_cast<void**>(&glIsQuery),
		reinterpret_cast<void**>(&glIsRenderbuffer),
		reinterpret_cast<void**>(&glIsShader),
		reinterpret_cast<void**>(&glIsSync),
		reinterpret_cast<void**>(&glIsTexture),
		reinterpret_cast<void**>(&glIsVertexArray),
		reinterpret_cast<void**>(&glLineWidth),
		reinterpret_cast<void**>(&glLinkProgram),
		reinterpret_cast<void**>(&glLogicOp),
		reinterpret_cast<void**>(&glMapBuffer),
		reinterpret_cast<void**>(&glMapBufferRange),
		reinterpret_cast<void**>(&glMultiDrawArrays),
		reinterpret_cast<void**>(&glMultiDrawElements),
		reinterpret_cast<void**>(&glMultiDrawElementsBaseVertex),
		reinterpret_cast<void**>(&glPixelStoref),
		reinterpret_cast<void**>(&glPixelStorei),
		reinterpret_cast<void**>(&glPointParameterf),
		reinterpret_cast<void**>(&glPointParameterfv),
		reinterpret_cast<void**>(&glPointParameteri),
		reinterpret_cast<void**>(&glPointParameteriv),
		reinterpret_cast<void**>(&glPointSize),
		reinterpret_cast<void**>(&glPolygonMode),
		reinterpret_cast<void**>(&glPolygonOffset),
		reinterpret_cast<void**>(&glPrimitiveRestartIndex),
		reinterpret_cast<void**>(&glProvokingVertex),
		reinterpret_cast<void**>(&glReadBuffer),
		reinterpret_cast<void**>(&glReadPixels),
		reinterpret_cast<void**>(&glRenderbufferStorage),
		reinterpret_cast<void**>(&glRenderbufferStorageMultisample),
		reinterpret_cast<void**>(&glSampleCoverage),
		reinterpret_cast<void**>(&glSampleMaski),
		reinterpret_cast<void**>(&glScissor),
		reinterpret_cast<void**>(&glShaderSource),
		reinterpret_cast<void**>(&glStencilFunc),
		reinterpret_cast<void**>(&glStencilFuncSeparate),
		reinterpret_cast<void**>(&glStencilMask),
		reinterpret_cast<void**>(&glStencilMaskSeparate),
		reinterpret_cast<void**>(&glStencilOp),
		reinterpret_cast<void**>(&glStencilOpSeparate),
		reinterpret_cast<void**>(&glTexBuffer),
		reinterpret_cast<void**>(&glTexImage1D),
		reinterpret_cast<void**>(&glTexImage2D),
		reinterpret_cast<void**>(&glTexImage2DMultisample),
		reinterpret_cast<void**>(&glTexImage3D),
		reinterpret_cast<void**>(&glTexImage3DMultisample),
		reinterpret_cast<void**>(&glTexParameterIiv),
		reinterpret_cast<void**>(&glTexParameterIuiv),
		reinterpret_cast<void**>(&glTexParameterf),
		reinterpret_cast<void**>(&glTexParameterfv),
		reinterpret_cast<void**>(&glTexParameteri),
		reinterpret_cast<void**>(&glTexParameteriv),
		reinterpret_cast<void**>(&glTexSubImage1D),
		reinterpret_cast<void**>(&glTexSubImage2D),
		reinterpret_cast<void**>(&glTexSubImage3D),
		reinterpret_cast<void**>(&glTransformFeedbackVaryings),
		reinterpret_cast<void**>(&glUniform1f),
		reinterpret_cast<void**>(&glUniform1fv),
		reinterpret_cast<void**>(&glUniform1i),
		reinterpret_cast<void**>(&glUniform1iv),
		reinterpret_cast<void**>(&glUniform1ui),
		reinterpret_cast<void**>(&glUniform1uiv),
		reinterpret_cast<void**>(&glUniform2f),
		reinterpret_cast<void**>(&glUniform2fv),
		reinterpret_cast<void**>(&glUniform2i),
		reinterpret_cast<void**>(&glUniform2iv),
		reinterpret_cast<void**>(&glUniform2ui),
		reinterpret_cast<void**>(&glUniform2uiv),
		reinterpret_cast<void**>(&glUniform3f),
		reinterpret_cast<void**>(&glUniform3fv),
		reinterpret_cast<void**>(&glUniform3i),
		reinterpret_cast<void**>(&glUniform3iv),
		reinterpret_cast<void**>(&glUniform3ui),
		reinterpret_cast<void**>(&glUniform3uiv),
		reinterpret_cast<void**>(&glUniform4f),
		reinterpret_cast<void**>(&glUniform4fv),
		reinterpret_cast<void**>(&glUniform4i),
		reinterpret_cast<void**>(&glUniform4iv),
		reinterpret_cast<void**>(&glUniform4ui),
		reinterpret_cast<void**>(&glUniform4uiv),
		reinterpret_cast<void**>(&glUniformBlockBinding),
		reinterpret_cast<void**>(&glUniformMatrix2fv),
		reinterpret_cast<void**>(&glUniformMatrix2x3fv),
		reinterpret_cast<void**>(&glUniformMatrix2x4fv),
		reinterpret_cast<void**>(&glUniformMatrix3fv),
		reinterpret_cast<void**>(&glUniformMatrix3x2fv),
		reinterpret_cast<void**>(&glUniformMatrix3x4fv),
		reinterpret_cast<void**>(&glUniformMatrix4fv),
		reinterpret_cast<void**>(&glUniformMatrix4x2fv),
		reinterpret_cast<void**>(&glUniformMatrix4x3fv),
		reinterpret_cast<void**>(&glUnmapBuffer),
		reinterpret_cast<void**>(&glUseProgram),
		reinterpret_cast<void**>(&glValidateProgram),
		reinterpret_cast<void**>(&glVertexAttrib1d),
		reinterpret_cast<void**>(&glVertexAttrib1dv),
		reinterpret_cast<void**>(&glVertexAttrib1f),
		reinterpret_cast<void**>(&glVertexAttrib1fv),
		reinterpret_cast<void**>(&glVertexAttrib1s),
		reinterpret_cast<void**>(&glVertexAttrib1sv),
		reinterpret_cast<void**>(&glVertexAttrib2d),
		reinterpret_cast<void**>(&glVertexAttrib2dv),
		reinterpret_cast<void**>(&glVertexAttrib2f),
		reinterpret_cast<void**>(&glVertexAttrib2fv),
		reinterpret_cast<void**>(&glVertexAttrib2s),
		reinterpret_cast<void**>(&glVertexAttrib2sv),
		reinterpret_cast<void**>(&glVertexAttrib3d),
		reinterpret_cast<void**>(&glVertexAttrib3dv),
		reinterpret_cast<void**>(&glVertexAttrib3f),
		reinterpret_cast<void**>(&glVertexAttrib3fv),
		reinterpret_cast<void**>(&glVertexAttrib3s),
		reinterpret_cast<void**>(&glVertexAttrib3sv),
		reinterpret_cast<void**>(&glVertexAttrib4Nbv),
		reinterpret_cast<void**>(&glVertexAttrib4Niv),
		reinterpret_cast<void**>(&glVertexAttrib4Nsv),
		reinterpret_cast<void**>(&glVertexAttrib4Nub),
		reinterpret_cast<void**>(&glVertexAttrib4Nubv),
		reinterpret_cast<void**>(&glVertexAttrib4Nuiv),
		reinterpret_cast<void**>(&glVertexAttrib4Nusv),
		reinterpret_cast<void**>(&glVertexAttrib4bv),
		reinterpret_cast<void**>(&glVertexAttrib4d),
		reinterpret_cast<void**>(&glVertexAttrib4dv),
		reinterpret_cast<void**>(&glVertexAttrib4f),
		reinterpret_cast<void**>(&glVertexAttrib4fv),
		reinterpret_cast<void**>(&glVertexAttrib4iv),
		reinterpret_cast<void**>(&glVertexAttrib4s),
		reinterpret_cast<void**>(&glVertexAttrib4sv),
		reinterpret_cast<void**>(&glVertexAttrib4ubv),
		reinterpret_cast<void**>(&glVertexAttrib4uiv),
		reinterpret_cast<void**>(&glVertexAttrib4usv),
		reinterpret_cast<void**>(&glVertexAttribI1i),
		reinterpret_cast<void**>(&glVertexAttribI1iv),
		reinterpret_cast<void**>(&glVertexAttribI1ui),
		reinterpret_cast<void**>(&glVertexAttribI1uiv),
		reinterpret_cast<void**>(&glVertexAttribI2i),
		reinterpret_cast<void**>(&glVertexAttribI2iv),
		reinterpret_cast<void**>(&glVertexAttribI2ui),
		reinterpret_cast<void**>(&glVertexAttribI2uiv),
		reinterpret_cast<void**>(&glVertexAttribI3i),
		reinterpret_cast<void**>(&glVertexAttribI3iv),
		reinterpret_cast<void**>(&glVertexAttribI3ui),
		reinterpret_cast<void**>(&glVertexAttribI3uiv),
		reinterpret_cast<void**>(&glVertexAttribI4bv),
		reinterpret_cast<void**>(&glVertexAttribI4i),
		reinterpret_cast<void**>(&glVertexAttribI4iv),
		reinterpret_cast<void**>(&glVertexAttribI4sv),
		reinterpret_cast<void**>(&glVertexAttribI4ubv),
		reinterpret_cast<void**>(&glVertexAttribI4ui),
		reinterpret_cast<void**>(&glVertexAttribI4uiv),
		reinterpret_cast<void**>(&glVertexAttribI4usv),
		reinterpret_cast<void**>(&glVertexAttribIPointer),
		reinterpret_cast<void**>(&glVertexAttribPointer),
		reinterpret_cast<void**>(&glViewport),
		reinterpret_cast<void**>(&glWaitSync),
	};

	return gl_symbols;
}

Ren3dGlExtensionMgrImpl::GlSymbolPtrs& Ren3dGlExtensionMgrImpl::get_es_v2_0_gl_symbol_ptrs()
{
	static auto gl_symbols = GlSymbolPtrs
	{
		reinterpret_cast<void**>(&glActiveTexture),
		reinterpret_cast<void**>(&glAttachShader),
		reinterpret_cast<void**>(&glBindAttribLocation),
		reinterpret_cast<void**>(&glBindBuffer),
		reinterpret_cast<void**>(&glBindFramebuffer),
		reinterpret_cast<void**>(&glBindRenderbuffer),
		reinterpret_cast<void**>(&glBindTexture),
		reinterpret_cast<void**>(&glBlendColor),
		reinterpret_cast<void**>(&glBlendEquation),
		reinterpret_cast<void**>(&glBlendEquationSeparate),
		reinterpret_cast<void**>(&glBlendFunc),
		reinterpret_cast<void**>(&glBlendFuncSeparate),
		reinterpret_cast<void**>(&glBufferData),
		reinterpret_cast<void**>(&glBufferSubData),
		reinterpret_cast<void**>(&glCheckFramebufferStatus),
		reinterpret_cast<void**>(&glClear),
		reinterpret_cast<void**>(&glClearColor),
		reinterpret_cast<void**>(&glClearDepthf),
		reinterpret_cast<void**>(&glClearStencil),
		reinterpret_cast<void**>(&glColorMask),
		reinterpret_cast<void**>(&glCompileShader),
		reinterpret_cast<void**>(&glCompressedTexImage2D),
		reinterpret_cast<void**>(&glCompressedTexSubImage2D),
		reinterpret_cast<void**>(&glCopyTexImage2D),
		reinterpret_cast<void**>(&glCopyTexSubImage2D),
		reinterpret_cast<void**>(&glCreateProgram),
		reinterpret_cast<void**>(&glCreateShader),
		reinterpret_cast<void**>(&glCullFace),
		reinterpret_cast<void**>(&glDeleteBuffers),
		reinterpret_cast<void**>(&glDeleteFramebuffers),
		reinterpret_cast<void**>(&glDeleteProgram),
		reinterpret_cast<void**>(&glDeleteRenderbuffers),
		reinterpret_cast<void**>(&glDeleteShader),
		reinterpret_cast<void**>(&glDeleteTextures),
		reinterpret_cast<void**>(&glDepthFunc),
		reinterpret_cast<void**>(&glDepthMask),
		reinterpret_cast<void**>(&glDepthRangef),
		reinterpret_cast<void**>(&glDetachShader),
		reinterpret_cast<void**>(&glDisable),
		reinterpret_cast<void**>(&glDisableVertexAttribArray),
		reinterpret_cast<void**>(&glDrawArrays),
		reinterpret_cast<void**>(&glDrawElements),
		reinterpret_cast<void**>(&glEnable),
		reinterpret_cast<void**>(&glEnableVertexAttribArray),
		reinterpret_cast<void**>(&glFinish),
		reinterpret_cast<void**>(&glFlush),
		reinterpret_cast<void**>(&glFramebufferRenderbuffer),
		reinterpret_cast<void**>(&glFramebufferTexture2D),
		reinterpret_cast<void**>(&glFrontFace),
		reinterpret_cast<void**>(&glGenBuffers),
		reinterpret_cast<void**>(&glGenFramebuffers),
		reinterpret_cast<void**>(&glGenRenderbuffers),
		reinterpret_cast<void**>(&glGenTextures),
		reinterpret_cast<void**>(&glGenerateMipmap),
		reinterpret_cast<void**>(&glGetActiveAttrib),
		reinterpret_cast<void**>(&glGetActiveUniform),
		reinterpret_cast<void**>(&glGetAttachedShaders),
		reinterpret_cast<void**>(&glGetAttribLocation),
		reinterpret_cast<void**>(&glGetBooleanv),
		reinterpret_cast<void**>(&glGetBufferParameteriv),
		reinterpret_cast<void**>(&glGetError),
		reinterpret_cast<void**>(&glGetFloatv),
		reinterpret_cast<void**>(&glGetFramebufferAttachmentParameteriv),
		reinterpret_cast<void**>(&glGetIntegerv),
		reinterpret_cast<void**>(&glGetProgramInfoLog),
		reinterpret_cast<void**>(&glGetProgramiv),
		reinterpret_cast<void**>(&glGetRenderbufferParameteriv),
		reinterpret_cast<void**>(&glGetShaderInfoLog),
		reinterpret_cast<void**>(&glGetShaderPrecisionFormat),
		reinterpret_cast<void**>(&glGetShaderSource),
		reinterpret_cast<void**>(&glGetShaderiv),
		reinterpret_cast<void**>(&glGetString),
		reinterpret_cast<void**>(&glGetTexParameterfv),
		reinterpret_cast<void**>(&glGetTexParameteriv),
		reinterpret_cast<void**>(&glGetUniformLocation),
		reinterpret_cast<void**>(&glGetUniformfv),
		reinterpret_cast<void**>(&glGetUniformiv),
		reinterpret_cast<void**>(&glGetVertexAttribPointerv),
		reinterpret_cast<void**>(&glGetVertexAttribfv),
		reinterpret_cast<void**>(&glGetVertexAttribiv),
		reinterpret_cast<void**>(&glHint),
		reinterpret_cast<void**>(&glIsBuffer),
		reinterpret_cast<void**>(&glIsEnabled),
		reinterpret_cast<void**>(&glIsFramebuffer),
		reinterpret_cast<void**>(&glIsProgram),
		reinterpret_cast<void**>(&glIsRenderbuffer),
		reinterpret_cast<void**>(&glIsShader),
		reinterpret_cast<void**>(&glIsTexture),
		reinterpret_cast<void**>(&glLineWidth),
		reinterpret_cast<void**>(&glLinkProgram),
		reinterpret_cast<void**>(&glPixelStorei),
		reinterpret_cast<void**>(&glPolygonOffset),
		reinterpret_cast<void**>(&glReadPixels),
		reinterpret_cast<void**>(&glReleaseShaderCompiler),
		reinterpret_cast<void**>(&glRenderbufferStorage),
		reinterpret_cast<void**>(&glSampleCoverage),
		reinterpret_cast<void**>(&glScissor),
		reinterpret_cast<void**>(&glShaderBinary),
		reinterpret_cast<void**>(&glShaderSource),
		reinterpret_cast<void**>(&glStencilFunc),
		reinterpret_cast<void**>(&glStencilFuncSeparate),
		reinterpret_cast<void**>(&glStencilMask),
		reinterpret_cast<void**>(&glStencilMaskSeparate),
		reinterpret_cast<void**>(&glStencilOp),
		reinterpret_cast<void**>(&glStencilOpSeparate),
		reinterpret_cast<void**>(&glTexImage2D),
		reinterpret_cast<void**>(&glTexParameterf),
		reinterpret_cast<void**>(&glTexParameterfv),
		reinterpret_cast<void**>(&glTexParameteri),
		reinterpret_cast<void**>(&glTexParameteriv),
		reinterpret_cast<void**>(&glTexSubImage2D),
		reinterpret_cast<void**>(&glUniform1f),
		reinterpret_cast<void**>(&glUniform1fv),
		reinterpret_cast<void**>(&glUniform1i),
		reinterpret_cast<void**>(&glUniform1iv),
		reinterpret_cast<void**>(&glUniform2f),
		reinterpret_cast<void**>(&glUniform2fv),
		reinterpret_cast<void**>(&glUniform2i),
		reinterpret_cast<void**>(&glUniform2iv),
		reinterpret_cast<void**>(&glUniform3f),
		reinterpret_cast<void**>(&glUniform3fv),
		reinterpret_cast<void**>(&glUniform3i),
		reinterpret_cast<void**>(&glUniform3iv),
		reinterpret_cast<void**>(&glUniform4f),
		reinterpret_cast<void**>(&glUniform4fv),
		reinterpret_cast<void**>(&glUniform4i),
		reinterpret_cast<void**>(&glUniform4iv),
		reinterpret_cast<void**>(&glUniformMatrix2fv),
		reinterpret_cast<void**>(&glUniformMatrix3fv),
		reinterpret_cast<void**>(&glUniformMatrix4fv),
		reinterpret_cast<void**>(&glUseProgram),
		reinterpret_cast<void**>(&glValidateProgram),
		reinterpret_cast<void**>(&glVertexAttrib1f),
		reinterpret_cast<void**>(&glVertexAttrib1fv),
		reinterpret_cast<void**>(&glVertexAttrib2f),
		reinterpret_cast<void**>(&glVertexAttrib2fv),
		reinterpret_cast<void**>(&glVertexAttrib3f),
		reinterpret_cast<void**>(&glVertexAttrib3fv),
		reinterpret_cast<void**>(&glVertexAttrib4f),
		reinterpret_cast<void**>(&glVertexAttrib4fv),
		reinterpret_cast<void**>(&glVertexAttribPointer),
		reinterpret_cast<void**>(&glViewport),
	};

	return gl_symbols;
}

Ren3dGlExtensionMgrImpl::GlSymbolPtrs& Ren3dGlExtensionMgrImpl::get_arb_buffer_storage_gl_symbol_ptrs()
{
	static auto gl_symbols = GlSymbolPtrs
	{
		reinterpret_cast<void**>(&glBufferStorage),
	};

	return gl_symbols;
}

Ren3dGlExtensionMgrImpl::GlSymbolPtrs& Ren3dGlExtensionMgrImpl::get_arb_direct_state_access_gl_symbol_ptrs()
{
	static auto gl_symbols = GlSymbolPtrs
	{
		reinterpret_cast<void**>(&glBindTextureUnit),
		reinterpret_cast<void**>(&glBlitNamedFramebuffer),
		reinterpret_cast<void**>(&glCheckNamedFramebufferStatus),
		reinterpret_cast<void**>(&glClearNamedBufferData),
		reinterpret_cast<void**>(&glClearNamedBufferSubData),
		reinterpret_cast<void**>(&glClearNamedFramebufferfi),
		reinterpret_cast<void**>(&glClearNamedFramebufferfv),
		reinterpret_cast<void**>(&glClearNamedFramebufferiv),
		reinterpret_cast<void**>(&glClearNamedFramebufferuiv),
		reinterpret_cast<void**>(&glCompressedTextureSubImage1D),
		reinterpret_cast<void**>(&glCompressedTextureSubImage2D),
		reinterpret_cast<void**>(&glCompressedTextureSubImage3D),
		reinterpret_cast<void**>(&glCopyNamedBufferSubData),
		reinterpret_cast<void**>(&glCopyTextureSubImage1D),
		reinterpret_cast<void**>(&glCopyTextureSubImage2D),
		reinterpret_cast<void**>(&glCopyTextureSubImage3D),
		reinterpret_cast<void**>(&glCreateBuffers),
		reinterpret_cast<void**>(&glCreateFramebuffers),
		reinterpret_cast<void**>(&glCreateProgramPipelines),
		reinterpret_cast<void**>(&glCreateQueries),
		reinterpret_cast<void**>(&glCreateRenderbuffers),
		reinterpret_cast<void**>(&glCreateSamplers),
		reinterpret_cast<void**>(&glCreateTextures),
		reinterpret_cast<void**>(&glCreateTransformFeedbacks),
		reinterpret_cast<void**>(&glCreateVertexArrays),
		reinterpret_cast<void**>(&glDisableVertexArrayAttrib),
		reinterpret_cast<void**>(&glEnableVertexArrayAttrib),
		reinterpret_cast<void**>(&glFlushMappedNamedBufferRange),
		reinterpret_cast<void**>(&glGenerateTextureMipmap),
		reinterpret_cast<void**>(&glGetCompressedTextureImage),
		reinterpret_cast<void**>(&glGetNamedBufferParameteri64v),
		reinterpret_cast<void**>(&glGetNamedBufferParameteriv),
		reinterpret_cast<void**>(&glGetNamedBufferPointerv),
		reinterpret_cast<void**>(&glGetNamedBufferSubData),
		reinterpret_cast<void**>(&glGetNamedFramebufferAttachmentParameteriv),
		reinterpret_cast<void**>(&glGetNamedFramebufferParameteriv),
		reinterpret_cast<void**>(&glGetNamedRenderbufferParameteriv),
		reinterpret_cast<void**>(&glGetQueryBufferObjecti64v),
		reinterpret_cast<void**>(&glGetQueryBufferObjectiv),
		reinterpret_cast<void**>(&glGetQueryBufferObjectui64v),
		reinterpret_cast<void**>(&glGetQueryBufferObjectuiv),
		reinterpret_cast<void**>(&glGetTextureImage),
		reinterpret_cast<void**>(&glGetTextureLevelParameterfv),
		reinterpret_cast<void**>(&glGetTextureLevelParameteriv),
		reinterpret_cast<void**>(&glGetTextureParameterIiv),
		reinterpret_cast<void**>(&glGetTextureParameterIuiv),
		reinterpret_cast<void**>(&glGetTextureParameterfv),
		reinterpret_cast<void**>(&glGetTextureParameteriv),
		reinterpret_cast<void**>(&glGetTransformFeedbacki64_v),
		reinterpret_cast<void**>(&glGetTransformFeedbacki_v),
		reinterpret_cast<void**>(&glGetTransformFeedbackiv),
		reinterpret_cast<void**>(&glGetVertexArrayIndexed64iv),
		reinterpret_cast<void**>(&glGetVertexArrayIndexediv),
		reinterpret_cast<void**>(&glGetVertexArrayiv),
		reinterpret_cast<void**>(&glInvalidateNamedFramebufferData),
		reinterpret_cast<void**>(&glInvalidateNamedFramebufferSubData),
		reinterpret_cast<void**>(&glMapNamedBuffer),
		reinterpret_cast<void**>(&glMapNamedBufferRange),
		reinterpret_cast<void**>(&glNamedBufferData),
		reinterpret_cast<void**>(&glNamedBufferStorage),
		reinterpret_cast<void**>(&glNamedBufferSubData),
		reinterpret_cast<void**>(&glNamedFramebufferDrawBuffer),
		reinterpret_cast<void**>(&glNamedFramebufferDrawBuffers),
		reinterpret_cast<void**>(&glNamedFramebufferParameteri),
		reinterpret_cast<void**>(&glNamedFramebufferReadBuffer),
		reinterpret_cast<void**>(&glNamedFramebufferRenderbuffer),
		reinterpret_cast<void**>(&glNamedFramebufferTexture),
		reinterpret_cast<void**>(&glNamedFramebufferTextureLayer),
		reinterpret_cast<void**>(&glNamedRenderbufferStorage),
		reinterpret_cast<void**>(&glNamedRenderbufferStorageMultisample),
		reinterpret_cast<void**>(&glTextureBuffer),
		reinterpret_cast<void**>(&glTextureBufferRange),
		reinterpret_cast<void**>(&glTextureParameterIiv),
		reinterpret_cast<void**>(&glTextureParameterIuiv),
		reinterpret_cast<void**>(&glTextureParameterf),
		reinterpret_cast<void**>(&glTextureParameterfv),
		reinterpret_cast<void**>(&glTextureParameteri),
		reinterpret_cast<void**>(&glTextureParameteriv),
		reinterpret_cast<void**>(&glTextureStorage1D),
		reinterpret_cast<void**>(&glTextureStorage2D),
		reinterpret_cast<void**>(&glTextureStorage2DMultisample),
		reinterpret_cast<void**>(&glTextureStorage3D),
		reinterpret_cast<void**>(&glTextureStorage3DMultisample),
		reinterpret_cast<void**>(&glTextureSubImage1D),
		reinterpret_cast<void**>(&glTextureSubImage2D),
		reinterpret_cast<void**>(&glTextureSubImage3D),
		reinterpret_cast<void**>(&glTransformFeedbackBufferBase),
		reinterpret_cast<void**>(&glTransformFeedbackBufferRange),
		reinterpret_cast<void**>(&glUnmapNamedBuffer),
		reinterpret_cast<void**>(&glVertexArrayAttribBinding),
		reinterpret_cast<void**>(&glVertexArrayAttribFormat),
		reinterpret_cast<void**>(&glVertexArrayAttribIFormat),
		reinterpret_cast<void**>(&glVertexArrayAttribLFormat),
		reinterpret_cast<void**>(&glVertexArrayBindingDivisor),
		reinterpret_cast<void**>(&glVertexArrayElementBuffer),
		reinterpret_cast<void**>(&glVertexArrayVertexBuffer),
		reinterpret_cast<void**>(&glVertexArrayVertexBuffers),
	};

	return gl_symbols;
}

Ren3dGlExtensionMgrImpl::GlSymbolPtrs& Ren3dGlExtensionMgrImpl::get_arb_framebuffer_object_gl_symbol_ptrs()
{
	static auto gl_symbols = GlSymbolPtrs
	{
		reinterpret_cast<void**>(&glBindFramebuffer),
		reinterpret_cast<void**>(&glBindRenderbuffer),
		reinterpret_cast<void**>(&glBlitFramebuffer),
		reinterpret_cast<void**>(&glCheckFramebufferStatus),
		reinterpret_cast<void**>(&glDeleteFramebuffers),
		reinterpret_cast<void**>(&glDeleteRenderbuffers),
		reinterpret_cast<void**>(&glFramebufferRenderbuffer),
		reinterpret_cast<void**>(&glFramebufferTexture1D),
		reinterpret_cast<void**>(&glFramebufferTexture2D),
		reinterpret_cast<void**>(&glFramebufferTexture3D),
		reinterpret_cast<void**>(&glFramebufferTextureLayer),
		reinterpret_cast<void**>(&glGenFramebuffers),
		reinterpret_cast<void**>(&glGenRenderbuffers),
		reinterpret_cast<void**>(&glGenerateMipmap),
		reinterpret_cast<void**>(&glGetFramebufferAttachmentParameteriv),
		reinterpret_cast<void**>(&glGetRenderbufferParameteriv),
		reinterpret_cast<void**>(&glIsFramebuffer),
		reinterpret_cast<void**>(&glIsRenderbuffer),
		reinterpret_cast<void**>(&glRenderbufferStorage),
		reinterpret_cast<void**>(&glRenderbufferStorageMultisample),
	};

	return gl_symbols;
}

Ren3dGlExtensionMgrImpl::GlSymbolPtrs& Ren3dGlExtensionMgrImpl::get_arb_sampler_objects_gl_symbol_ptrs()
{
	static auto gl_symbols = GlSymbolPtrs
	{
		reinterpret_cast<void**>(&glBindSampler),
		reinterpret_cast<void**>(&glDeleteSamplers),
		reinterpret_cast<void**>(&glGenSamplers),
		reinterpret_cast<void**>(&glGetSamplerParameterIiv),
		reinterpret_cast<void**>(&glGetSamplerParameterIuiv),
		reinterpret_cast<void**>(&glGetSamplerParameterfv),
		reinterpret_cast<void**>(&glGetSamplerParameteriv),
		reinterpret_cast<void**>(&glIsSampler),
		reinterpret_cast<void**>(&glSamplerParameterIiv),
		reinterpret_cast<void**>(&glSamplerParameterIuiv),
		reinterpret_cast<void**>(&glSamplerParameterf),
		reinterpret_cast<void**>(&glSamplerParameterfv),
		reinterpret_cast<void**>(&glSamplerParameteri),
		reinterpret_cast<void**>(&glSamplerParameteriv),
	};

	return gl_symbols;
}

Ren3dGlExtensionMgrImpl::GlSymbolPtrs& Ren3dGlExtensionMgrImpl::get_arb_separate_shader_objects_gl_symbol_ptrs()
{
	static auto gl_symbols = GlSymbolPtrs
	{
		reinterpret_cast<void**>(&glActiveShaderProgram),
		reinterpret_cast<void**>(&glBindProgramPipeline),
		reinterpret_cast<void**>(&glCreateShaderProgramv),
		reinterpret_cast<void**>(&glDeleteProgramPipelines),
		reinterpret_cast<void**>(&glGenProgramPipelines),
		reinterpret_cast<void**>(&glGetProgramPipelineInfoLog),
		reinterpret_cast<void**>(&glGetProgramPipelineiv),
		reinterpret_cast<void**>(&glIsProgramPipeline),
		reinterpret_cast<void**>(&glProgramParameteri),
		reinterpret_cast<void**>(&glProgramUniform1d),
		reinterpret_cast<void**>(&glProgramUniform1dv),
		reinterpret_cast<void**>(&glProgramUniform1f),
		reinterpret_cast<void**>(&glProgramUniform1fv),
		reinterpret_cast<void**>(&glProgramUniform1i),
		reinterpret_cast<void**>(&glProgramUniform1iv),
		reinterpret_cast<void**>(&glProgramUniform1ui),
		reinterpret_cast<void**>(&glProgramUniform1uiv),
		reinterpret_cast<void**>(&glProgramUniform2d),
		reinterpret_cast<void**>(&glProgramUniform2dv),
		reinterpret_cast<void**>(&glProgramUniform2f),
		reinterpret_cast<void**>(&glProgramUniform2fv),
		reinterpret_cast<void**>(&glProgramUniform2i),
		reinterpret_cast<void**>(&glProgramUniform2iv),
		reinterpret_cast<void**>(&glProgramUniform2ui),
		reinterpret_cast<void**>(&glProgramUniform2uiv),
		reinterpret_cast<void**>(&glProgramUniform3d),
		reinterpret_cast<void**>(&glProgramUniform3dv),
		reinterpret_cast<void**>(&glProgramUniform3f),
		reinterpret_cast<void**>(&glProgramUniform3fv),
		reinterpret_cast<void**>(&glProgramUniform3i),
		reinterpret_cast<void**>(&glProgramUniform3iv),
		reinterpret_cast<void**>(&glProgramUniform3ui),
		reinterpret_cast<void**>(&glProgramUniform3uiv),
		reinterpret_cast<void**>(&glProgramUniform4d),
		reinterpret_cast<void**>(&glProgramUniform4dv),
		reinterpret_cast<void**>(&glProgramUniform4f),
		reinterpret_cast<void**>(&glProgramUniform4fv),
		reinterpret_cast<void**>(&glProgramUniform4i),
		reinterpret_cast<void**>(&glProgramUniform4iv),
		reinterpret_cast<void**>(&glProgramUniform4ui),
		reinterpret_cast<void**>(&glProgramUniform4uiv),
		reinterpret_cast<void**>(&glProgramUniformMatrix2dv),
		reinterpret_cast<void**>(&glProgramUniformMatrix2fv),
		reinterpret_cast<void**>(&glProgramUniformMatrix2x3dv),
		reinterpret_cast<void**>(&glProgramUniformMatrix2x3fv),
		reinterpret_cast<void**>(&glProgramUniformMatrix2x4dv),
		reinterpret_cast<void**>(&glProgramUniformMatrix2x4fv),
		reinterpret_cast<void**>(&glProgramUniformMatrix3dv),
		reinterpret_cast<void**>(&glProgramUniformMatrix3fv),
		reinterpret_cast<void**>(&glProgramUniformMatrix3x2dv),
		reinterpret_cast<void**>(&glProgramUniformMatrix3x2fv),
		reinterpret_cast<void**>(&glProgramUniformMatrix3x4dv),
		reinterpret_cast<void**>(&glProgramUniformMatrix3x4fv),
		reinterpret_cast<void**>(&glProgramUniformMatrix4dv),
		reinterpret_cast<void**>(&glProgramUniformMatrix4fv),
		reinterpret_cast<void**>(&glProgramUniformMatrix4x2dv),
		reinterpret_cast<void**>(&glProgramUniformMatrix4x2fv),
		reinterpret_cast<void**>(&glProgramUniformMatrix4x3dv),
		reinterpret_cast<void**>(&glProgramUniformMatrix4x3fv),
		reinterpret_cast<void**>(&glUseProgramStages),
		reinterpret_cast<void**>(&glValidateProgramPipeline),
	};

	return gl_symbols;
}

Ren3dGlExtensionMgrImpl::GlSymbolPtrs& Ren3dGlExtensionMgrImpl::get_arb_vertex_array_object_gl_symbol_ptrs()
{
	static auto gl_symbols = GlSymbolPtrs
	{
		reinterpret_cast<void**>(&glBindVertexArray),
		reinterpret_cast<void**>(&glDeleteVertexArrays),
		reinterpret_cast<void**>(&glGenVertexArrays),
		reinterpret_cast<void**>(&glIsVertexArray),
	};

	return gl_symbols;
}

Ren3dGlExtensionMgrImpl::GlSymbolPtrs& Ren3dGlExtensionMgrImpl::get_ext_framebuffer_blit_gl_symbol_ptrs()
{
	static auto gl_symbols = GlSymbolPtrs
	{
		reinterpret_cast<void**>(&glBlitFramebufferEXT),
	};

	return gl_symbols;
}

Ren3dGlExtensionMgrImpl::GlSymbolPtrs& Ren3dGlExtensionMgrImpl::get_ext_framebuffer_multisample_gl_symbol_ptrs()
{
	static auto gl_symbols = GlSymbolPtrs
	{
		reinterpret_cast<void**>(&glRenderbufferStorageMultisampleEXT),
	};

	return gl_symbols;
}

Ren3dGlExtensionMgrImpl::GlSymbolPtrs& Ren3dGlExtensionMgrImpl::get_ext_framebuffer_object_gl_symbol_ptrs()
{
	static auto gl_symbols = GlSymbolPtrs
	{
		reinterpret_cast<void**>(&glBindFramebufferEXT),
		reinterpret_cast<void**>(&glBindRenderbufferEXT),
		reinterpret_cast<void**>(&glCheckFramebufferStatusEXT),
		reinterpret_cast<void**>(&glDeleteFramebuffersEXT),
		reinterpret_cast<void**>(&glDeleteRenderbuffersEXT),
		reinterpret_cast<void**>(&glFramebufferRenderbufferEXT),
		reinterpret_cast<void**>(&glFramebufferTexture1DEXT),
		reinterpret_cast<void**>(&glFramebufferTexture2DEXT),
		reinterpret_cast<void**>(&glFramebufferTexture3DEXT),
		reinterpret_cast<void**>(&glGenFramebuffersEXT),
		reinterpret_cast<void**>(&glGenRenderbuffersEXT),
		reinterpret_cast<void**>(&glGenerateMipmapEXT),
		reinterpret_cast<void**>(&glGetFramebufferAttachmentParameterivEXT),
		reinterpret_cast<void**>(&glGetRenderbufferParameterivEXT),
		reinterpret_cast<void**>(&glIsFramebufferEXT),
		reinterpret_cast<void**>(&glIsRenderbufferEXT),
		reinterpret_cast<void**>(&glRenderbufferStorageEXT),
	};

	return gl_symbols;
}

const std::string& Ren3dGlExtensionMgrImpl::get_empty_extension_name() noexcept
{
	static const auto result = std::string{};

	return result;
}

int Ren3dGlExtensionMgrImpl::get_registered_extension_count() noexcept
{
	return static_cast<int>(Ren3dGlExtensionId::count_);
}

int Ren3dGlExtensionMgrImpl::get_extension_index(
	const Ren3dGlExtensionId extension_id) noexcept
{
	const auto extension_index = static_cast<int>(extension_id);

	if (extension_index < 0 || extension_index >= get_registered_extension_count())
	{
		return -1;
	}

	return extension_index;
}

void Ren3dGlExtensionMgrImpl::initialize_registry()
{
	registry_.clear();
	registry_.resize(static_cast<int>(Ren3dGlExtensionId::count_));

	{
		auto& registry_item = registry_[static_cast<int>(Ren3dGlExtensionId::essentials)];
		registry_item.is_virtual_ = true;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.extension_name_ = "essentials";
		registry_item.gl_symbol_ptrs_ = &get_essentials_gl_symbol_ptrs();
	}

	{
		auto& registry_item = registry_[static_cast<int>(Ren3dGlExtensionId::v2_0)];
		registry_item.is_virtual_ = true;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.extension_name_ = "v2.0";
		registry_item.gl_symbol_ptrs_ = &get_v2_0_gl_symbol_ptrs();
	}

	{
		auto& registry_item = registry_[static_cast<int>(Ren3dGlExtensionId::v3_2_core)];
		registry_item.is_virtual_ = true;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.extension_name_ = "v3.2 core";
		registry_item.gl_symbol_ptrs_ = &get_v3_2_core_gl_symbol_ptrs();
	}

	{
		auto& registry_item = registry_[static_cast<int>(Ren3dGlExtensionId::es_v2_0)];
		registry_item.is_virtual_ = true;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.extension_name_ = "ES v2.0";
		registry_item.gl_symbol_ptrs_ = &get_es_v2_0_gl_symbol_ptrs();
	}

	{
		auto& registry_item = registry_[static_cast<int>(Ren3dGlExtensionId::arb_buffer_storage)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.extension_name_ = "GL_ARB_buffer_storage";
		registry_item.gl_symbol_ptrs_ = &get_arb_buffer_storage_gl_symbol_ptrs();
	}

	{
		auto& registry_item = registry_[static_cast<int>(Ren3dGlExtensionId::arb_direct_state_access)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.extension_name_ = "GL_ARB_direct_state_access";
		registry_item.gl_symbol_ptrs_ = &get_arb_direct_state_access_gl_symbol_ptrs();
	}

	{
		auto& registry_item = registry_[static_cast<int>(Ren3dGlExtensionId::arb_framebuffer_object)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.extension_name_ = "GL_ARB_framebuffer_object";
		registry_item.gl_symbol_ptrs_ = &get_arb_framebuffer_object_gl_symbol_ptrs();
	}

	{
		auto& registry_item = registry_[static_cast<int>(Ren3dGlExtensionId::arb_texture_filter_anisotropic)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.extension_name_ = "GL_ARB_texture_filter_anisotropic";
		registry_item.gl_symbol_ptrs_ = nullptr;
	}

	{
		auto& registry_item = registry_[static_cast<int>(Ren3dGlExtensionId::arb_texture_non_power_of_two)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.extension_name_ = "GL_ARB_texture_non_power_of_two";
		registry_item.gl_symbol_ptrs_ = nullptr;
	}

	{
		auto& registry_item = registry_[static_cast<int>(Ren3dGlExtensionId::arb_sampler_objects)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.extension_name_ = "GL_ARB_sampler_objects";
		registry_item.gl_symbol_ptrs_ = &get_arb_sampler_objects_gl_symbol_ptrs();
	}

	{
		auto& registry_item = registry_[static_cast<int>(Ren3dGlExtensionId::arb_separate_shader_objects)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.extension_name_ = "GL_ARB_separate_shader_objects";
		registry_item.gl_symbol_ptrs_ = &get_arb_separate_shader_objects_gl_symbol_ptrs();
	}

	{
		auto& registry_item = registry_[static_cast<int>(Ren3dGlExtensionId::arb_vertex_array_object)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.extension_name_ = "GL_ARB_vertex_array_object";
		registry_item.gl_symbol_ptrs_ = &get_arb_vertex_array_object_gl_symbol_ptrs();
	}

	{
		auto& registry_item = registry_[static_cast<int>(Ren3dGlExtensionId::ext_framebuffer_blit)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.extension_name_ = "GL_EXT_framebuffer_blit";
		registry_item.gl_symbol_ptrs_ = &get_ext_framebuffer_blit_gl_symbol_ptrs();
	}

	{
		auto& registry_item = registry_[static_cast<int>(Ren3dGlExtensionId::ext_framebuffer_multisample)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.extension_name_ = "GL_EXT_framebuffer_multisample";
		registry_item.gl_symbol_ptrs_ = &get_ext_framebuffer_multisample_gl_symbol_ptrs();
	}

	{
		auto& registry_item = registry_[static_cast<int>(Ren3dGlExtensionId::ext_packed_depth_stencil)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.extension_name_ = "GL_EXT_packed_depth_stencil";
		registry_item.gl_symbol_ptrs_ = nullptr;
	}

	{
		auto& registry_item = registry_[static_cast<int>(Ren3dGlExtensionId::ext_texture_filter_anisotropic)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.extension_name_ = "GL_EXT_texture_filter_anisotropic";
		registry_item.gl_symbol_ptrs_ = nullptr;
	}

	{
		auto& registry_item = registry_[static_cast<int>(Ren3dGlExtensionId::oes_texture_npot)];
		registry_item.is_virtual_ = false;
		registry_item.is_probed_ = false;
		registry_item.is_available_ = false;
		registry_item.extension_name_ = "GL_OES_texture_npot";
		registry_item.gl_symbol_ptrs_ = nullptr;
	}
}

int Ren3dGlExtensionMgrImpl::parse_version_number(
	const std::string& string)
{
	if (string.empty())
	{
		throw Ren3dGlExtensionMgrImplGlVersionException{"Expected a digit."};
	}

	constexpr auto max_digit_count = 10;

	auto number = 0;
	auto digit_count = 0;

	for (const auto ch : string)
	{
		++digit_count;

		if (digit_count > max_digit_count)
		{
			throw Ren3dGlExtensionMgrImplGlVersionException{"Too many digits."};
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
				throw Ren3dGlExtensionMgrImplGlVersionException{"Non-digit character."};
		}
	}

	return number;
}

void Ren3dGlExtensionMgrImpl::parse_version(
	const std::string& version_string,
	Ren3dGlVersion& gl_version)
{
	static const auto digits = std::string{"0123456789"};
	static const auto gles_prefix = std::string{"OpenGL ES "};

	const auto digits_pos = version_string.find_first_of(digits);

	if (digits_pos == std::string::npos)
	{
		throw Ren3dGlExtensionMgrImplGlVersionException{"Unsupported format."};
	}

	if (digits_pos > 0)
	{
		if (version_string.find(gles_prefix) == 0)
		{
			gl_version.is_es_ = true;
		}
		else
		{
			throw Ren3dGlExtensionMgrImplGlVersionException{"Unsupported prefix."};
		}
	}

	const auto vendor_info_pos = version_string.find(' ', digits_pos);
	const auto has_vendor = (vendor_info_pos != std::string::npos);

	const auto first_dot_pos = version_string.find('.', digits_pos);

	if (first_dot_pos == std::string::npos)
	{
		throw Ren3dGlExtensionMgrImplGlVersionException{"Expected the dot separator."};
	}

	const auto second_dot_pos = version_string.rfind('.', vendor_info_pos);
	const auto has_release = (first_dot_pos != second_dot_pos);

	// Major.
	//
	const auto major_version_string_view = version_string.substr(
		digits_pos,
		first_dot_pos - digits_pos
	);

	gl_version.major_ = parse_version_number(major_version_string_view);


	// Minor.
	//
	const auto minor_begin_pos = first_dot_pos + 1;
	const auto minor_end_pos = (
		has_release ?
		second_dot_pos :
		(has_vendor ? vendor_info_pos : static_cast<int>(version_string.size()))
	);

	const auto minor_version_string_view = version_string.substr(
		minor_begin_pos,
		minor_end_pos - minor_begin_pos
	);

	gl_version.minor_ = parse_version_number(minor_version_string_view);


	// Release.
	//
	if (has_release)
	{
		const auto release_begin_pos = second_dot_pos + 1;

		const auto release_version_str_view = version_string.substr(
			release_begin_pos,
			vendor_info_pos - release_begin_pos
		);

		gl_version.release_ = parse_version_number(release_version_str_view);
	}


	// Vendor information.
	//
	if (has_vendor)
	{
		gl_version.vendor_ = version_string.substr(vendor_info_pos + 1);
	}
}

void Ren3dGlExtensionMgrImpl::get_version(
	const GLenum version_enum,
	Ren3dGlVersion& gl_version)
{
	if (!glGetString)
	{
		throw Ren3dGlExtensionMgrImplGlVersionException{"Null \"glGetString\"."};
	}

	const auto gl_version_string = reinterpret_cast<const char*>(glGetString(version_enum));

	if (!gl_version_string)
	{
		throw Ren3dGlExtensionMgrImplGlVersionException{"Null version string."};
	}

	const auto version_string = std::string{gl_version_string};

	parse_version(version_string, gl_version);
}

void Ren3dGlExtensionMgrImpl::get_version()
{
	get_version(GL_VERSION, gl_version_);
	get_version(GL_SHADING_LANGUAGE_VERSION, glsl_version_);
}

void Ren3dGlExtensionMgrImpl::get_context_attributes()
{
	context_kind_ = Ren3dGlUtils::get_context_kind();

	if (context_kind_ == Ren3dGlContextKind::invalid)
	{
		throw Ren3dGlExtensionMgrImplException{"Invalid context kind."};
	}
}

void Ren3dGlExtensionMgrImpl::get_names_from_multiple_strings()
{
	if (!glGetIntegerv)
	{
		throw Ren3dGlExtensionMgrImplMissingSymbolException{"glGetIntegerv"};
	}

	if (!glGetStringi)
	{
		throw Ren3dGlExtensionMgrImplMissingSymbolException{"glGetStringi"};
	}

	auto gl_extension_count = GLint{};

	glGetIntegerv(GL_NUM_EXTENSIONS, &gl_extension_count);
	Ren3dGlError::ensure_debug();

	if (gl_extension_count == 0)
	{
		return;
	}

	extension_names_.resize(gl_extension_count);

	for (auto i = GLint{}; i < gl_extension_count; ++i)
	{
		const auto extension_name = glGetStringi(GL_EXTENSIONS, static_cast<GLuint>(i));

		if (!extension_name)
		{
			throw Ren3dGlExtensionMgrImplException{"Null extension name."};
		}

		extension_names_[i] = reinterpret_cast<const char*>(extension_name);
	}
}

void Ren3dGlExtensionMgrImpl::get_names_from_one_string()
{
	if (!glGetString)
	{
		throw Ren3dGlExtensionMgrImplMissingSymbolException{"glGetString"};
	}

	const auto gl_extensions_c_string = glGetString(GL_EXTENSIONS);
	Ren3dGlError::ensure_debug();

	if (!gl_extensions_c_string)
	{
		throw Ren3dGlExtensionMgrImplException{"Null extensions string."};
	}

	const auto gl_extensions_std_string = std::string
	{
		reinterpret_cast<const char*>(gl_extensions_c_string)
	};

	const auto extension_count = 1 + std::count(
		gl_extensions_std_string.cbegin(),
		gl_extensions_std_string.cend(),
		' '
	);

	auto iss = std::istringstream{gl_extensions_std_string};

	extension_names_.reserve(extension_count);

	extension_names_.assign(
		std::istream_iterator<std::string>{iss},
		std::istream_iterator<std::string>{}
	);
}

void Ren3dGlExtensionMgrImpl::get_names()
{
	if (gl_version_.major_ >= 3)
	{
		get_names_from_multiple_strings();
	}
	else
	{
		get_names_from_one_string();
	}

	std::sort(extension_names_.begin(), extension_names_.end());
}

void Ren3dGlExtensionMgrImpl::probe_generic(
	const Ren3dGlExtensionId extension_id)
{
	const auto extension_index = get_extension_index(extension_id);

	if (extension_index < 0)
	{
		throw Ren3dGlExtensionMgrImplException{"Invalid extension id."};
	}

	auto& registry_item = registry_[extension_index];

	if (registry_item.is_probed_)
	{
		return;
	}

	registry_item.is_probed_ = true;

	if (registry_item.is_virtual_ && !registry_item.gl_symbol_ptrs_)
	{
		throw Ren3dGlExtensionMgrImplException{"Expected symbol pointer for virtual extension."};
	}

	if (!registry_item.is_virtual_)
	{
		const auto has_extension_name = std::any_of(
			extension_names_.cbegin(),
			extension_names_.cend(),
			[&](const auto& item)
			{
				return item == registry_item.extension_name_;
			}
		);

		if (!has_extension_name)
		{
			return;
		}
	}

	if (registry_item.gl_symbol_ptrs_)
	{
		const auto has_symbols = has_gl_symbol(*registry_item.gl_symbol_ptrs_);

		if (!has_symbols)
		{
			return;
		}
	}

	registry_item.is_available_ = true;
}

//
// Ren3dGlExtensionMgrImpl
// ==========================================================================


// ==========================================================================
// Ren3dGlExtensionMgrFactory
//

Ren3dGlExtensionMgrUPtr Ren3dGlExtensionMgrFactory::create()
{
	return std::make_unique<Ren3dGlExtensionMgrImpl>();
}

//
// Ren3dGlExtensionMgrFactory
// ==========================================================================


} // detail
} // bstone
