/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_sys_window_sdl.h"

#include <string>
#include "SDL3/SDL_version.h"

#include "bstone_char_conv.h"
#include "bstone_exception.h"
#include "bstone_single_pool_resource.h"
#include "bstone_sys_detail_sdl.h"
#include "bstone_sys_exception_sdl.h"
#include "bstone_sys_gl_context_sdl.h"
#include "bstone_sys_renderer_sdl.h"

namespace bstone {
namespace sys {

SdlWindowInternal::SdlWindowInternal() = default;

SdlWindowInternal::~SdlWindowInternal() = default;

void* SdlWindowInternal::get_native_handle() const noexcept
{
	return do_get_native_handle();
}

void* SdlWindowInternal::get_sdl_window() const
{
	return do_get_sdl_window();
}

// ==========================================================================

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

class SdlWindow final : public SdlWindowInternal
{
public:
	SdlWindow(
		Logger& logger,
		WindowRoundedCornerMgr& rounded_corner_mgr,
		const WindowInitParam& param);

	~SdlWindow() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	Logger& logger_;
	WindowRoundedCornerMgr& rounded_corner_mgr_;
	SdlWindowUPtr sdl_window_{};
	Uint32 sdl_window_id_{};
	void* native_window_handle_{};

private:
	void* do_get_native_handle() const noexcept override;
	void* do_get_sdl_window() const override;

private:
	const char* do_get_title() override;
	void do_set_title(const char* title) override;

	WindowPosition do_get_position() override;
	void do_set_position(WindowPosition position) override;

	WindowSize do_get_size() override;
	void do_set_size(WindowSize size) override;

	DisplayMode do_get_display_mode() override;
	void do_set_display_mode(const DisplayMode& display_mode) override;

	void do_show(bool is_visible) override;

	void do_set_rounded_corner_type(WindowRoundedCornerType value) override;

	WindowFullscreenType do_get_fullscreen_mode() override;
	void do_set_fullscreen_mode(WindowFullscreenType fullscreen_mode) override;

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

SdlWindow::SdlWindow(
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

	sdl_window_.reset(sdl_ensure_result(SDL_CreateWindow(
		param.title,
		param.width,
		param.height,
		sdl_flags)));
	sdl_ensure_result(SDL_SetWindowPosition(sdl_window_.get(), sdl_x, sdl_y));

	sdl_window_id_ = SDL_GetWindowID(sdl_window_.get());

#ifdef _WIN32
	const SDL_PropertiesID sdl_window_properties_id = SDL_GetWindowProperties(sdl_window_.get());
	if (sdl_window_properties_id == 0)
	{
		BSTONE_THROW_STATIC_SOURCE("SDL_GetWindowProperties");
	}
	native_window_handle_ = SDL_GetPointerProperty(sdl_window_properties_id, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
	if (native_window_handle_ == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("SDL_GetPointerProperty(SDL_PROP_WINDOW_WIN32_HWND_POINTER)");
	}
#endif

	rounded_corner_mgr_.set_round_corner_type(*this, param.rounded_corner_type);

	message.clear();
	log_output(message);
	logger_.log_information(message.c_str());

	logger_.log_information(">>> SDL window created.");
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

SdlWindow::~SdlWindow()
{
	auto message = std::string{};
	message.reserve(256);
	
	message.clear();
	message += "Destroy SDL window (id:";
	detail::sdl_log_xint(sdl_window_id_, message);
	message += "; ptr:";
	detail::sdl_log_xint_hex(reinterpret_cast<std::uintptr_t>(sdl_window_.get()), message);
	message += ").";

	logger_.log_information(message.c_str());
}

void* SdlWindow::operator new(std::size_t size)
try {
	return get_memory_resource().allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SdlWindow::operator delete(void* ptr)
{
	get_memory_resource().deallocate(ptr);
}

void* SdlWindow::do_get_native_handle() const noexcept
{
#ifdef _WIN32
	return native_window_handle_;
#else
	return nullptr;
#endif
}

void* SdlWindow::do_get_sdl_window() const
{
	return sdl_window_.get();
}

const char* SdlWindow::do_get_title()
try {
	return sdl_ensure_result(SDL_GetWindowTitle(sdl_window_.get()));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SdlWindow::do_set_title(const char* title)
try {
	SDL_SetWindowTitle(sdl_window_.get(), title);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

WindowPosition SdlWindow::do_get_position()
try {
	auto x = 0;
	auto y = 0;
	SDL_GetWindowPosition(sdl_window_.get(), &x, &y);
	return WindowPosition{WindowOffset{x}, WindowOffset{y}};
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SdlWindow::do_set_position(WindowPosition position)
try {
	const auto sdl_x = map_offset(position.x);
	const auto sdl_y = map_offset(position.y);
	SDL_SetWindowPosition(sdl_window_.get(), sdl_x, sdl_y);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

WindowSize SdlWindow::do_get_size()
try {
	auto width = 0;
	auto height = 0;
	SDL_GetWindowSize(sdl_window_.get(), &width, &height);
	return WindowSize{width, height};
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SdlWindow::do_set_size(WindowSize size)
try {
	sdl_ensure_result(SDL_SetWindowSize(sdl_window_.get(), size.width, size.height));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

DisplayMode SdlWindow::do_get_display_mode()
try {
	SDL_Window* const sdl_window = sdl_window_.get();
	if (const Uint32 sdl_flags = SDL_GetWindowFlags(sdl_window);
		(sdl_flags & SDL_WINDOW_FULLSCREEN) != 0)
	{
		if (const SDL_DisplayMode* sdl_display_mode = SDL_GetWindowFullscreenMode(sdl_window);
			sdl_display_mode != nullptr)
		{
			return DisplayMode{
				.width = sdl_display_mode->w,
				.height = sdl_display_mode->h,
				.refresh_rate = static_cast<int>(sdl_display_mode->refresh_rate),
			};
		}
	}
	const SDL_DisplayMode* const sdl_display_mode = sdl_ensure_result(SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay()));
	int sdl_w;
	int sdl_h;
	sdl_ensure_result(SDL_GetWindowSizeInPixels(sdl_window, &sdl_w, &sdl_h));
	return DisplayMode{
		.width = sdl_w,
		.height = sdl_h,
		.refresh_rate = static_cast<int>(sdl_display_mode->refresh_rate + 0.5F),
	};
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SdlWindow::do_set_display_mode(const DisplayMode& display_mode)
try {
	SDL_DisplayMode sdl_display_mode{};
	sdl_ensure_result(SDL_GetClosestFullscreenDisplayMode(
		SDL_GetDisplayForWindow(sdl_window_.get()),
		display_mode.width,
		display_mode.height,
		display_mode.refresh_rate,
		false,
		&sdl_display_mode));
	sdl_ensure_result(SDL_SetWindowFullscreenMode(sdl_window_.get(), &sdl_display_mode));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SdlWindow::do_show(bool is_visible)
try {
	const auto sdl_func = is_visible ? SDL_ShowWindow : SDL_HideWindow;
	sdl_func(sdl_window_.get());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SdlWindow::do_set_rounded_corner_type(WindowRoundedCornerType value)
try {
	rounded_corner_mgr_.set_round_corner_type(*this, value);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

WindowFullscreenType SdlWindow::do_get_fullscreen_mode()
{
	const Uint32 sdl_flags = SDL_GetWindowFlags(sdl_window_.get());
	if ((sdl_flags & SDL_WINDOW_FULLSCREEN) != 0)
	{
		if (SDL_GetWindowFullscreenMode(sdl_window_.get()) != nullptr)
		{
			return WindowFullscreenType::exclusive;
		}
		return WindowFullscreenType::fake;
	}
	return WindowFullscreenType::none;
}

void SdlWindow::do_set_fullscreen_mode(WindowFullscreenType fullscreen_mode)
try {
	bool is_fullscreen = false;
	bool is_exclusive_fullscreen = false;
	switch (fullscreen_mode)
	{
		case WindowFullscreenType::none:
			break;
		case WindowFullscreenType::fake:
			is_fullscreen = true;
			break;
		case WindowFullscreenType::exclusive:
			is_fullscreen = true;
			is_exclusive_fullscreen = true;
			break;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown fullscreen mode.");
	}
	if (!is_exclusive_fullscreen)
	{
		sdl_ensure_result(SDL_SetWindowFullscreenMode(sdl_window_.get(), nullptr));
	}
	sdl_ensure_result(SDL_SetWindowFullscreen(sdl_window_.get(), is_fullscreen));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

GlContextUPtr SdlWindow::do_make_gl_context()
try {
	return make_gl_context_sdl(logger_, *sdl_window_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

WindowSize SdlWindow::do_gl_get_drawable_size()
try {
	int sdl_width;
	int sdl_height;
	sdl_ensure_result(SDL_GetWindowSizeInPixels(sdl_window_.get(), &sdl_width, &sdl_height));
	return WindowSize{sdl_width, sdl_height};
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void SdlWindow::do_gl_swap_buffers()
try {
	SDL_GL_SwapWindow(sdl_window_.get());
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

RendererUPtr SdlWindow::do_make_renderer(const RendererInitParam& param)
try {
	return make_renderer_sdl(logger_, *sdl_window_, param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

MemoryResource& SdlWindow::get_memory_resource()
{
	static SinglePoolResource<SdlWindow> memory_pool{};

	return memory_pool;
}

void SdlWindow::log_offset(WindowOffset offset, std::string& message)
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
			detail::sdl_log_xint(offset.get(), message);
			break;
	}
}

void SdlWindow::log_rect(const WindowInitParam& param, std::string& message)
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

	if (param.renderer_type == WindowRendererType::open_gl)
	{
		log_flag("opengl", message);
	}
	else if (param.renderer_type == WindowRendererType::vulkan)
	{
		log_flag("vulkan", message);
	}

	log_flag(param.is_visible ? "shown" : "hidden", message);
}

void SdlWindow::log_rounded_corner_type(const WindowInitParam& param, std::string& message)
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
	detail::sdl_log_eol(message);
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

void SdlWindow::log_output(std::string& message)
{
	//
	message += "Effective parameters:";
	detail::sdl_log_eol(message);

	// window pointer
	//
	message += "  Ptr: ";
	detail::sdl_log_xint_hex(reinterpret_cast<std::uintptr_t>(sdl_window_.get()), message);
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

int SdlWindow::map_offset(WindowOffset offset)
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

Uint32 SdlWindow::map_flags(const WindowInitParam& param) noexcept
{
	Uint32 sdl_flags = 0;

	if (param.renderer_type == WindowRendererType::open_gl)
	{
		sdl_flags |= SDL_WINDOW_OPENGL;
	}
	else if (param.renderer_type == WindowRendererType::vulkan)
	{
		sdl_flags |= SDL_WINDOW_VULKAN;
	}

	sdl_flags |= param.is_visible ? 0 : SDL_WINDOW_HIDDEN;

	return sdl_flags;
}

int SdlWindow::map_gl_context_profile(GlContextProfile context_profile)
try {
	switch (context_profile)
	{
		case GlContextProfile::compatibility: return SDL_GL_CONTEXT_PROFILE_COMPATIBILITY;
		case GlContextProfile::core: return SDL_GL_CONTEXT_PROFILE_CORE;
		case GlContextProfile::es: return SDL_GL_CONTEXT_PROFILE_ES;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown context profile.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

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
	const int sdl_accelerated = gl_attribs.is_accelerated;

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

WindowUPtr make_sdl_window(
	Logger& logger,
	WindowRoundedCornerMgr& rounded_corner_mgr,
	const WindowInitParam& param)
{
	return std::make_unique<SdlWindow>(logger, rounded_corner_mgr, param);
}

} // namespace sys
} // namespace bstone
