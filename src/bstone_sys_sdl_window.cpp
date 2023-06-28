/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <string>
#include "bstone_char_conv.h"
#include "bstone_exception.h"
#include "bstone_single_memory_pool.h"
#include "bstone_sys_sdl_detail.h"
#include "bstone_sys_sdl_exception.h"
#include "bstone_sys_sdl_gl_context.h"
#include "bstone_sys_sdl_renderer.h"
#include "bstone_sys_sdl_window.h"

namespace bstone {
namespace sys {

namespace {

struct SdlWindowDeleter
{
	void operator()(SDL_Window* sdl_window) const noexcept
	{
		SDL_DestroyWindow(sdl_window);
	}
};

using SdlWindowUPtr = std::unique_ptr<SDL_Window, SdlWindowDeleter>;

// ==========================================================================

class SdlWindow final : public Window
{
public:
	SdlWindow(Logger& logger, const WindowInitParam& param);
	~SdlWindow() override;

	static void* operator new(std::size_t size);
	static void operator delete(void* ptr);

private:
	Logger& logger_;
	SdlWindowUPtr sdl_window_{};
	Uint32 sdl_window_id_{};

private:
	const char* do_get_title() override;
	void do_set_title(const char* title) override;

	WindowPosition do_get_position() override;
	void do_set_position(WindowPosition position) override;

	WindowSize do_get_size() override;
	void do_set_size(WindowSize size) override;

	void do_show(bool is_visible) override;

	bool do_is_fake_fullscreen() override;
	void do_set_fake_fullscreen(bool is_fake_fullscreen) override;

	GlContextUPtr do_make_gl_context() override;

	WindowSize do_gl_get_drawable_size() override;
	void do_gl_swap_buffers() override;

	RendererUPtr do_make_renderer(const RendererInitParam& param) override;

private:
	static void log_position(int position, std::string& message);
	static void log_rect(const WindowInitParam& param, std::string& message);
	static void log_flag(const char* flag_name, std::string& message);
	static void log_flags(const WindowInitParam& param, std::string& message);
	static void log_gl_attributes(const GlContextAttributes* gl_attributes, std::string& message);
	static void log_input(const WindowInitParam& param, std::string& message);
	void log_output(std::string& message);

	static int map_position(int position);
	static Uint32 map_flags(const WindowInitParam& param) noexcept;
	static int map_gl_context_profile(GlContextProfile context_profile);
	static GlContextAttributes make_default_gl_attributes() noexcept;
	static void set_gl_attributes(const GlContextAttributes& gl_attributes);
};

// ==========================================================================

using SdlWindowPool = SingleMemoryPool<SdlWindow>;
SdlWindowPool sdl_window_pool{};

// ==========================================================================

SdlWindow::SdlWindow(Logger& logger, const WindowInitParam& param)
BSTONE_BEGIN_CTOR_TRY
	:
	logger_{logger}
{
	logger_.log_information("<<< Create SDL window.");

	auto message = std::string{};
	message.reserve(4096);

	message.clear();
	log_input(param, message);
	logger_.log_information(message);

	const auto sdl_x = map_position(param.x);
	const auto sdl_y = map_position(param.y);
	const auto sdl_flags = map_flags(param);

	if (param.is_opengl)
	{
		if (param.gl_attributes != nullptr)
		{
			set_gl_attributes(*param.gl_attributes);
		}
		else
		{
			auto gl_attributes = make_default_gl_attributes();
			set_gl_attributes(gl_attributes);
		}
	}

	sdl_window_.reset(sdl_ensure_result(SDL_CreateWindow(
		param.title,
		sdl_x,
		sdl_y,
		param.width,
		param.height,
		sdl_flags)));

	sdl_window_id_ = SDL_GetWindowID(sdl_window_.get());

	message.clear();
	log_output(message);
	logger_.log_information(message);

	logger_.log_information(">>> SDL window created.");
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

SdlWindow::~SdlWindow()
{
	auto message = std::string{};
	message.reserve(256);
	
	message.clear();
	message += "Destroy SDL window (id:";
	detail::sdl_log_xint(sdl_window_id_, message);
	message += "; ptr:";
	detail::sdl_log_xint_hex(reinterpret_cast<UIntP>(sdl_window_.get()), message);
	message += ").";

	logger_.log_information(message);
}

void* SdlWindow::operator new(std::size_t size)
BSTONE_BEGIN_FUNC_TRY
	return sdl_window_pool.allocate(size);
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SdlWindow::operator delete(void* ptr)
BSTONE_BEGIN_FUNC_TRY
	sdl_window_pool.deallocate(ptr);
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const char* SdlWindow::do_get_title()
BSTONE_BEGIN_FUNC_TRY
	return sdl_ensure_result(SDL_GetWindowTitle(sdl_window_.get()));
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SdlWindow::do_set_title(const char* title)
BSTONE_BEGIN_FUNC_TRY
	SDL_SetWindowTitle(sdl_window_.get(), title);
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

WindowPosition SdlWindow::do_get_position()
BSTONE_BEGIN_FUNC_TRY
	auto x = 0;
	auto y = 0;
	SDL_GetWindowPosition(sdl_window_.get(), &x, &y);
	return WindowPosition{x, y};
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SdlWindow::do_set_position(WindowPosition position)
BSTONE_BEGIN_FUNC_TRY
	const auto sdl_x = map_position(position.x);
	const auto sdl_y = map_position(position.y);
	SDL_SetWindowPosition(sdl_window_.get(), sdl_x, sdl_y);
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

WindowSize SdlWindow::do_get_size()
BSTONE_BEGIN_FUNC_TRY
	auto width = 0;
	auto height = 0;
	SDL_GetWindowSize(sdl_window_.get(), &width, &height);
	return WindowSize{width, height};
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SdlWindow::do_set_size(WindowSize size)
BSTONE_BEGIN_FUNC_TRY
	SDL_SetWindowSize(sdl_window_.get(), size.width, size.height);
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SdlWindow::do_show(bool is_visible)
BSTONE_BEGIN_FUNC_TRY
	const auto sdl_func = is_visible ? SDL_ShowWindow : SDL_HideWindow;
	sdl_func(sdl_window_.get());
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

bool SdlWindow::do_is_fake_fullscreen()
BSTONE_BEGIN_FUNC_TRY
	const auto sdl_flags = SDL_GetWindowFlags(sdl_window_.get());
	return (sdl_flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0;
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SdlWindow::do_set_fake_fullscreen(bool is_fake_fullscreen)
BSTONE_BEGIN_FUNC_TRY
	const auto sdl_flags = Uint32{is_fake_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0U};
	sdl_ensure_result(SDL_SetWindowFullscreen(sdl_window_.get(), sdl_flags));
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlContextUPtr SdlWindow::do_make_gl_context()
BSTONE_BEGIN_FUNC_TRY
	return make_sdl_gl_context(logger_, *sdl_window_);
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

WindowSize SdlWindow::do_gl_get_drawable_size()
BSTONE_BEGIN_FUNC_TRY
	auto width = 0;
	auto height = 0;
	SDL_GL_GetDrawableSize(sdl_window_.get(), &width, &height);
	return WindowSize{width, height};
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SdlWindow::do_gl_swap_buffers()
BSTONE_BEGIN_FUNC_TRY
	SDL_GL_SwapWindow(sdl_window_.get());
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

RendererUPtr SdlWindow::do_make_renderer(const RendererInitParam& param)
BSTONE_BEGIN_FUNC_TRY
	return make_sdl_renderer(logger_, *sdl_window_, param);
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SdlWindow::log_position(int position, std::string& message)
{
	switch (position)
	{
		case window_position_centered:
			message += "centered";
			break;

		case window_position_undefined:
			message += "undefined";
			break;

		default:
			detail::sdl_log_xint(position, message);
			break;
	}
}

void SdlWindow::log_rect(const WindowInitParam& param, std::string& message)
{
	// x
	//
	message += "(x:";
	log_position(param.x, message);
	message += "; y:";

	// y
	//
	log_position(param.y, message);
	message += "; w:";

	// width
	//
	detail::sdl_log_xint(param.width, message);
	message += "; h:";

	// height
	//
	detail::sdl_log_xint(param.height, message);
	message += ')';
}

void SdlWindow::log_flag(const char* flag_name, std::string& message)
{
	constexpr auto spaces = "    ";

	message += spaces;
	message += flag_name;
	detail::sdl_log_eol(message);
}

void SdlWindow::log_flags(const WindowInitParam& param, std::string& message)
{
	const auto log_flag = [](const char* flag_name, std::string& message)
	{
		message += "    ";
		message += flag_name;
		detail::sdl_log_eol(message);
	};

	message += "  Flags:";
	detail::sdl_log_eol(message);

	if (param.is_opengl)
	{
		log_flag("opengl", message);
	}

	log_flag(param.is_visible ? "shown" : "hidden", message);

	if (param.is_fake_fullscreen)
	{
		log_flag("fake fullscreen", message);
	}
}

void SdlWindow::log_gl_attributes(const GlContextAttributes* gl_attributes, std::string& message)
{
	message += "  ";
	message += gl_attributes != nullptr ? "Custom" : "Default";
	message += " GL context attributes:";
	detail::sdl_log_eol(message);

	if (gl_attributes != nullptr)
	{
		detail::sdl_log_gl_attributes(*gl_attributes, message);
	}
	else
	{
		const auto default_gl_attribs = make_default_gl_attributes();
		detail::sdl_log_gl_attributes(default_gl_attribs, message);
	}
}

void SdlWindow::log_input(const WindowInitParam& param, std::string& message)
{
	message += "Input parameters:";
	detail::sdl_log_eol(message);

	// title
	//
	message += "  Title: \"";
	message += param.title != nullptr ? param.title : "";
	message += '"';
	detail::sdl_log_eol(message);

	// rect
	//
	message += "  Rect: ";
	log_rect(param, message);
	detail::sdl_log_eol(message);

	// flags
	//
	log_flags(param, message);

	// gl attributes
	//
	if (param.is_opengl)
	{
		log_gl_attributes(param.gl_attributes, message);
	}
}

void SdlWindow::log_output(std::string& message)
{
	//
	message += "Effective parameters:";
	detail::sdl_log_eol(message);

	// window pointer
	//
	message += "  Ptr: ";
	detail::sdl_log_xint_hex(reinterpret_cast<UIntP>(sdl_window_.get()), message);
	detail::sdl_log_eol(message);

	// window id
	//
	message += "  ID: ";
	detail::sdl_log_xint(sdl_window_id_, message);
	detail::sdl_log_eol(message);

	// x, y
	//
	auto x = 0;
	auto y = 0;
	SDL_GetWindowPosition(sdl_window_.get(), &x, &y);

	message += "  Rect: (x:";
	detail::sdl_log_xint(x, message);
	message += "; y:";
	detail::sdl_log_xint(y, message);

	// width, height
	auto width = 0;
	auto height = 0;
	SDL_GetWindowSize(sdl_window_.get(), &width, &height);

	message += "; w:";
	detail::sdl_log_xint(width, message);
	message += "; h:";
	detail::sdl_log_xint(height, message);
	message += ')';
	detail::sdl_log_eol(message);
}

int SdlWindow::map_position(int position)
BSTONE_BEGIN_FUNC_TRY
	switch (position)
	{
		case window_position_centered: return SDL_WINDOWPOS_CENTERED;
		case window_position_undefined: return SDL_WINDOWPOS_UNDEFINED;

		default:
			if (position < window_min_position || position > window_max_position)
			{
				BSTONE_THROW_STATIC_SOURCE("Position out of range.");
			}

			return position;
	}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

Uint32 SdlWindow::map_flags(const WindowInitParam& param) noexcept
{
	auto sdl_flags = Uint32{SDL_WINDOW_ALLOW_HIGHDPI};

	if (param.is_opengl)
	{
		sdl_flags |= SDL_WINDOW_OPENGL;
	}

	sdl_flags |= param.is_visible ? SDL_WINDOW_SHOWN : SDL_WINDOW_HIDDEN;

	if (param.is_fake_fullscreen)
	{
		sdl_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	return sdl_flags;
}

int SdlWindow::map_gl_context_profile(GlContextProfile context_profile)
BSTONE_BEGIN_FUNC_TRY
	switch (context_profile)
	{
		case GlContextProfile::compatibility: return SDL_GL_CONTEXT_PROFILE_COMPATIBILITY;
		case GlContextProfile::core: return SDL_GL_CONTEXT_PROFILE_CORE;
		case GlContextProfile::es: return SDL_GL_CONTEXT_PROFILE_ES;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown context profile.");
	}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlContextAttributes SdlWindow::make_default_gl_attributes() noexcept
{
	auto gl_attributes = GlContextAttributes{};
	gl_attributes.is_accelerated = true;
	gl_attributes.profile = GlContextProfile::compatibility;
	gl_attributes.major_version = 1;
	gl_attributes.minor_version = 1;
	gl_attributes.multisample_buffer_count = 0;
	gl_attributes.multisample_sample_count = 0;
	gl_attributes.red_bit_count = 0;
	gl_attributes.green_bit_count = 0;
	gl_attributes.blue_bit_count = 0;
	gl_attributes.alpha_bit_count = 0;
	gl_attributes.depth_bit_count = 0;
	return gl_attributes;
}

void SdlWindow::set_gl_attributes(const GlContextAttributes& gl_attribs)
{
	const auto sdl_profile = map_gl_context_profile(gl_attribs.profile);
	const auto sdl_accelerated = gl_attribs.is_accelerated ? SDL_TRUE : SDL_FALSE;

	SDL_GL_ResetAttributes();
	sdl_ensure_result(SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, sdl_accelerated));
	sdl_ensure_result(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, sdl_profile));
	sdl_ensure_result(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, gl_attribs.major_version));
	sdl_ensure_result(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, gl_attribs.minor_version));
	sdl_ensure_result(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, gl_attribs.multisample_buffer_count));
	sdl_ensure_result(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, gl_attribs.multisample_sample_count));
	sdl_ensure_result(SDL_GL_SetAttribute(SDL_GL_RED_SIZE, gl_attribs.red_bit_count));
	sdl_ensure_result(SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, gl_attribs.green_bit_count));
	sdl_ensure_result(SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, gl_attribs.blue_bit_count));
	sdl_ensure_result(SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, gl_attribs.alpha_bit_count));
	sdl_ensure_result(SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, gl_attribs.depth_bit_count));
}

} // namespace

// ==========================================================================

WindowUPtr make_sdl_window(Logger& logger, const WindowInitParam& param)
{
	return std::make_unique<SdlWindow>(logger, param);
}

} // namespace sys
} // namespace bstone
