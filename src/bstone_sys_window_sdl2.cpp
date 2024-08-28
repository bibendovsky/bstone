/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_sys_window_sdl2.h"

#include <string>

#ifdef _WIN32
#include "SDL_syswm.h"
#endif

#include "bstone_char_conv.h"
#include "bstone_exception.h"
#include "bstone_single_pool_resource.h"
#include "bstone_sys_detail_sdl2.h"
#include "bstone_sys_exception_sdl2.h"
#include "bstone_sys_gl_context_sdl2.h"
#include "bstone_sys_renderer_sdl2.h"

namespace bstone {
namespace sys {

Sdl2WindowInternal::Sdl2WindowInternal() = default;

Sdl2WindowInternal::~Sdl2WindowInternal() = default;

void* Sdl2WindowInternal::get_native_handle() const noexcept
{
	return do_get_native_handle();
}

// ==========================================================================

namespace {

struct Sdl2WindowDeleter
{
	void operator()(SDL_Window* sdl_window) const noexcept
	{
		SDL_DestroyWindow(sdl_window);
	}
};

using Sdl2WindowUPtr = std::unique_ptr<SDL_Window, Sdl2WindowDeleter>;

// ==========================================================================

class Sdl2Window final : public Sdl2WindowInternal
{
public:
	Sdl2Window(
		Logger& logger,
		WindowRoundedCornerMgr& rounded_corner_mgr,
		const WindowInitParam& param);

	~Sdl2Window() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	Logger& logger_;
	WindowRoundedCornerMgr& rounded_corner_mgr_;
	Sdl2WindowUPtr sdl_window_{};
	Uint32 sdl_window_id_{};
	void* native_window_handle_{};

private:
	void* do_get_native_handle() const noexcept override;

private:
	const char* do_get_title() override;
	void do_set_title(const char* title) override;

	WindowPosition do_get_position() override;
	void do_set_position(WindowPosition position) override;

	WindowSize do_get_size() override;
	void do_set_size(WindowSize size) override;

	void do_show(bool is_visible) override;

	void do_set_rounded_corner_type(WindowRoundedCornerType value) override;

	bool do_is_fake_fullscreen() override;
	void do_set_fake_fullscreen(bool is_fake_fullscreen) override;

	GlContextUPtr do_make_gl_context() override;

	WindowSize do_gl_get_drawable_size() override;
	void do_gl_swap_buffers() override;

	RendererUPtr do_make_renderer(const RendererInitParam& param) override;

private:
	static MemoryResource& get_memory_resource();

	static void log_offset(WindowOffset offset, std::string& message);
	static void log_rect(const WindowInitParam& param, std::string& message);
	static void log_flag(const char* flag_name, std::string& message);
	static void log_flags(const WindowInitParam& param, std::string& message);
	static void log_rounded_corner_type(const WindowInitParam& param, std::string& message);
	static void log_gl_attributes(const GlContextAttributes* gl_attributes, std::string& message);
	static void log_input(const WindowInitParam& param, std::string& message);
	void log_output(std::string& message);

	static int map_offset(WindowOffset offset);
	static Uint32 map_flags(const WindowInitParam& param) noexcept;
	static int map_gl_context_profile(GlContextProfile context_profile);
	static GlContextAttributes make_default_gl_attributes() noexcept;
	static void set_gl_attributes(const GlContextAttributes& gl_attributes);
};

// ==========================================================================

Sdl2Window::Sdl2Window(
	Logger& logger,
	WindowRoundedCornerMgr& rounded_corner_mgr,
	const WindowInitParam& param)
try
	:
	logger_{logger},
	rounded_corner_mgr_{rounded_corner_mgr}
{
	logger_.log_information("<<< Create SDL window.");

	auto message = std::string{};
	message.reserve(4096);

	message.clear();
	log_input(param, message);
	logger_.log_information(message.c_str());

	const auto sdl_x = map_offset(param.x);
	const auto sdl_y = map_offset(param.y);
	const auto sdl_flags = map_flags(param);

	if (param.renderer_type == WindowRendererType::open_gl)
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

	sdl_window_.reset(sdl2_ensure_result(SDL_CreateWindow(
		param.title,
		sdl_x,
		sdl_y,
		param.width,
		param.height,
		sdl_flags)));

	sdl_window_id_ = SDL_GetWindowID(sdl_window_.get());

#ifdef _WIN32
	auto sdl_sys_wm_info = SDL_SysWMinfo{};
	SDL_VERSION(&sdl_sys_wm_info.version);

	if (SDL_GetWindowWMInfo(sdl_window_.get(), &sdl_sys_wm_info) == SDL_FALSE)
	{
		sdl2_fail();
	}

	native_window_handle_ = sdl_sys_wm_info.info.win.window;
#endif

	rounded_corner_mgr_.set_round_corner_type(*this, param.rounded_corner_type);

	message.clear();
	log_output(message);
	logger_.log_information(message.c_str());

	logger_.log_information(">>> SDL window created.");
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

Sdl2Window::~Sdl2Window()
{
	auto message = std::string{};
	message.reserve(256);
	
	message.clear();
	message += "Destroy SDL window (id:";
	detail::sdl2_log_xint(sdl_window_id_, message);
	message += "; ptr:";
	detail::sdl2_log_xint_hex(reinterpret_cast<std::uintptr_t>(sdl_window_.get()), message);
	message += ").";

	logger_.log_information(message.c_str());
}

void* Sdl2Window::operator new(std::size_t size)
try {
	return get_memory_resource().allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl2Window::operator delete(void* ptr)
{
	get_memory_resource().deallocate(ptr);
}

void* Sdl2Window::do_get_native_handle() const noexcept
{
#ifdef _WIN32
	return native_window_handle_;
#else
	return nullptr;
#endif
}

const char* Sdl2Window::do_get_title()
try {
	return sdl2_ensure_result(SDL_GetWindowTitle(sdl_window_.get()));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl2Window::do_set_title(const char* title)
try {
	SDL_SetWindowTitle(sdl_window_.get(), title);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

WindowPosition Sdl2Window::do_get_position()
try {
	auto x = 0;
	auto y = 0;
	SDL_GetWindowPosition(sdl_window_.get(), &x, &y);
	return WindowPosition{WindowOffset{x}, WindowOffset{y}};
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl2Window::do_set_position(WindowPosition position)
try {
	const auto sdl_x = map_offset(position.x);
	const auto sdl_y = map_offset(position.y);
	SDL_SetWindowPosition(sdl_window_.get(), sdl_x, sdl_y);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

WindowSize Sdl2Window::do_get_size()
try {
	auto width = 0;
	auto height = 0;
	SDL_GetWindowSize(sdl_window_.get(), &width, &height);
	return WindowSize{width, height};
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl2Window::do_set_size(WindowSize size)
try {
	SDL_SetWindowSize(sdl_window_.get(), size.width, size.height);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl2Window::do_show(bool is_visible)
try {
	const auto sdl_func = is_visible ? SDL_ShowWindow : SDL_HideWindow;
	sdl_func(sdl_window_.get());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl2Window::do_set_rounded_corner_type(WindowRoundedCornerType value)
try {
	rounded_corner_mgr_.set_round_corner_type(*this, value);
}
 BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

bool Sdl2Window::do_is_fake_fullscreen()
try {
	const auto sdl_flags = SDL_GetWindowFlags(sdl_window_.get());
	return (sdl_flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl2Window::do_set_fake_fullscreen(bool is_fake_fullscreen)
try {
	const auto sdl_flags = static_cast<Uint32>(
		is_fake_fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WindowFlags{});
	sdl2_ensure_result(SDL_SetWindowFullscreen(sdl_window_.get(), sdl_flags));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlContextUPtr Sdl2Window::do_make_gl_context()
try {
	return make_sdl2_gl_context(logger_, *sdl_window_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

WindowSize Sdl2Window::do_gl_get_drawable_size()
try {
	auto width = 0;
	auto height = 0;
	SDL_GL_GetDrawableSize(sdl_window_.get(), &width, &height);
	return WindowSize{width, height};
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Sdl2Window::do_gl_swap_buffers()
try {
	SDL_GL_SwapWindow(sdl_window_.get());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

RendererUPtr Sdl2Window::do_make_renderer(const RendererInitParam& param)
try {
	return make_sdl2_renderer(logger_, *sdl_window_, param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

MemoryResource& Sdl2Window::get_memory_resource()
{
	static SinglePoolResource<Sdl2Window> memory_pool{};

	return memory_pool;
}

void Sdl2Window::log_offset(WindowOffset offset, std::string& message)
{
	switch (offset.get_type())
	{
		case WindowOffsetType::centered:
			message += "centered";
			break;

		case WindowOffsetType::undefined:
			message += "undefined";
			break;

		default:
			detail::sdl2_log_xint(offset.get(), message);
			break;
	}
}

void Sdl2Window::log_rect(const WindowInitParam& param, std::string& message)
{
	// x
	//
	message += "(x:";
	log_offset(param.x, message);
	message += "; y:";

	// y
	//
	log_offset(param.y, message);
	message += "; w:";

	// width
	//
	detail::sdl2_log_xint(param.width, message);
	message += "; h:";

	// height
	//
	detail::sdl2_log_xint(param.height, message);
	message += ')';
}

void Sdl2Window::log_flag(const char* flag_name, std::string& message)
{
	constexpr auto spaces = "    ";

	message += spaces;
	message += flag_name;
	detail::sdl2_log_eol(message);
}

void Sdl2Window::log_flags(const WindowInitParam& param, std::string& message)
{
	const auto log_flag = [](const char* flag_name, std::string& message)
	{
		message += "    ";
		message += flag_name;
		detail::sdl2_log_eol(message);
	};

	message += "  Flags:";
	detail::sdl2_log_eol(message);

	if (param.renderer_type == WindowRendererType::open_gl)
	{
		log_flag("opengl", message);
	}

	log_flag(param.is_visible ? "shown" : "hidden", message);

	if (param.fullscreen_type == WindowFullscreenType::fake)
	{
		log_flag("fake fullscreen", message);
	}
}

void Sdl2Window::log_rounded_corner_type(const WindowInitParam& param, std::string& message)
{
	auto rounded_corner_type_string = "unknown";

	switch (param.rounded_corner_type)
	{
		case WindowRoundedCornerType::none: rounded_corner_type_string = "none"; break;
		case WindowRoundedCornerType::system: rounded_corner_type_string = "system"; break;
		case WindowRoundedCornerType::round: rounded_corner_type_string = "round"; break;
		case WindowRoundedCornerType::round_small: rounded_corner_type_string = "round small"; break;
	}

	message += "  Rounded corner type: ";
	message += rounded_corner_type_string;
	detail::sdl2_log_eol(message);
}

void Sdl2Window::log_gl_attributes(const GlContextAttributes* gl_attributes, std::string& message)
{
	message += "  ";
	message += gl_attributes != nullptr ? "Custom" : "Default";
	message += " GL context attributes:";
	detail::sdl2_log_eol(message);

	if (gl_attributes != nullptr)
	{
		detail::sdl2_log_gl_attributes(*gl_attributes, message);
	}
	else
	{
		const auto default_gl_attribs = make_default_gl_attributes();
		detail::sdl2_log_gl_attributes(default_gl_attribs, message);
	}
}

void Sdl2Window::log_input(const WindowInitParam& param, std::string& message)
{
	message += "Input parameters:";
	detail::sdl2_log_eol(message);

	// title
	//
	message += "  Title: \"";
	message += param.title != nullptr ? param.title : "";
	message += '"';
	detail::sdl2_log_eol(message);

	// rect
	//
	message += "  Rect: ";
	log_rect(param, message);
	detail::sdl2_log_eol(message);

	// flags
	//
	log_flags(param, message);

	// rounded corner type
	//
	log_rounded_corner_type(param, message);

	// gl attributes
	//
	if (param.renderer_type == WindowRendererType::open_gl)
	{
		log_gl_attributes(param.gl_attributes, message);
	}
}

void Sdl2Window::log_output(std::string& message)
{
	//
	message += "Effective parameters:";
	detail::sdl2_log_eol(message);

	// window pointer
	//
	message += "  Ptr: ";
	detail::sdl2_log_xint_hex(reinterpret_cast<std::uintptr_t>(sdl_window_.get()), message);
	detail::sdl2_log_eol(message);

	// window id
	//
	message += "  ID: ";
	detail::sdl2_log_xint(sdl_window_id_, message);
	detail::sdl2_log_eol(message);

	// x, y
	//
	auto x = 0;
	auto y = 0;
	SDL_GetWindowPosition(sdl_window_.get(), &x, &y);

	message += "  Rect: (x:";
	detail::sdl2_log_xint(x, message);
	message += "; y:";
	detail::sdl2_log_xint(y, message);

	// width, height
	auto width = 0;
	auto height = 0;
	SDL_GetWindowSize(sdl_window_.get(), &width, &height);

	message += "; w:";
	detail::sdl2_log_xint(width, message);
	message += "; h:";
	detail::sdl2_log_xint(height, message);
	message += ')';
	detail::sdl2_log_eol(message);
}

int Sdl2Window::map_offset(WindowOffset offset)
{
	switch (offset.get_type())
	{
		case WindowOffsetType::centered: return SDL_WINDOWPOS_CENTERED;
		case WindowOffsetType::undefined: return SDL_WINDOWPOS_UNDEFINED;

		case WindowOffsetType::custom:
			{
				const auto value = offset.get();

				if (value < window_min_position || value > window_max_position)
				{
					BSTONE_THROW_STATIC_SOURCE("Position out of range.");
				}

				return value;
			}

		default: BSTONE_THROW_STATIC_SOURCE("Unknown offset type.");
	}
}

Uint32 Sdl2Window::map_flags(const WindowInitParam& param) noexcept
{
	auto sdl_flags = Uint32{SDL_WINDOW_ALLOW_HIGHDPI};

	if (param.renderer_type == WindowRendererType::open_gl)
	{
		sdl_flags |= SDL_WINDOW_OPENGL;
	}

	sdl_flags |= param.is_visible ? SDL_WINDOW_SHOWN : SDL_WINDOW_HIDDEN;

	if (param.fullscreen_type == WindowFullscreenType::fake)
	{
		sdl_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	return sdl_flags;
}

int Sdl2Window::map_gl_context_profile(GlContextProfile context_profile)
try {
	switch (context_profile)
	{
		case GlContextProfile::compatibility: return SDL_GL_CONTEXT_PROFILE_COMPATIBILITY;
		case GlContextProfile::core: return SDL_GL_CONTEXT_PROFILE_CORE;
		case GlContextProfile::es: return SDL_GL_CONTEXT_PROFILE_ES;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown context profile.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlContextAttributes Sdl2Window::make_default_gl_attributes() noexcept
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

void Sdl2Window::set_gl_attributes(const GlContextAttributes& gl_attribs)
{
	const auto sdl_profile = map_gl_context_profile(gl_attribs.profile);
	const auto sdl_accelerated = gl_attribs.is_accelerated ? SDL_TRUE : SDL_FALSE;

	SDL_GL_ResetAttributes();
	sdl2_ensure_result(SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, sdl_accelerated));
	sdl2_ensure_result(SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, sdl_profile));
	sdl2_ensure_result(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, gl_attribs.major_version));
	sdl2_ensure_result(SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, gl_attribs.minor_version));
	sdl2_ensure_result(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, gl_attribs.multisample_buffer_count));
	sdl2_ensure_result(SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, gl_attribs.multisample_sample_count));
	sdl2_ensure_result(SDL_GL_SetAttribute(SDL_GL_RED_SIZE, gl_attribs.red_bit_count));
	sdl2_ensure_result(SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, gl_attribs.green_bit_count));
	sdl2_ensure_result(SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, gl_attribs.blue_bit_count));
	sdl2_ensure_result(SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, gl_attribs.alpha_bit_count));
	sdl2_ensure_result(SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, gl_attribs.depth_bit_count));
}

} // namespace

// ==========================================================================

WindowUPtr make_sdl2_window(
	Logger& logger,
	WindowRoundedCornerMgr& rounded_corner_mgr,
	const WindowInitParam& param)
{
	return std::make_unique<Sdl2Window>(logger, rounded_corner_mgr, param);
}

} // namespace sys
} // namespace bstone
