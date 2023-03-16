/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_exception.h"
#include "bstone_sys_window.h"

namespace bstone {
namespace sys {

const char* Window::get_title()
try
{
	return do_get_title();
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Window::set_title(const char* title)
try
{
	do_set_title(title);
}
BSTONE_STATIC_THROW_NESTED_FUNC

WindowPosition Window::get_position()
try
{
	return do_get_position();
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Window::set_position(WindowPosition position)
try
{
	do_set_position(position);
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Window::center()
try
{
	do_set_position(WindowPosition{window_position_centered, window_position_centered});
}
BSTONE_STATIC_THROW_NESTED_FUNC

WindowSize Window::get_size()
try
{
	return do_get_size();
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Window::set_size(WindowSize size)
try
{
	do_set_size(size);
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Window::show(bool is_visible)
try
{
	do_show(is_visible);
}
BSTONE_STATIC_THROW_NESTED_FUNC

bool Window::is_fake_fullscreen()
try
{
	return do_is_fake_fullscreen();
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Window::set_fake_fullscreen(bool is_fake_fullscreen)
try
{
	do_set_fake_fullscreen(is_fake_fullscreen);
}
BSTONE_STATIC_THROW_NESTED_FUNC

GlContextUPtr Window::make_gl_context()
try
{
	return do_make_gl_context();
}
BSTONE_STATIC_THROW_NESTED_FUNC

WindowSize Window::gl_get_drawable_size()
try
{
	return do_gl_get_drawable_size();
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Window::gl_swap_buffers()
try
{
	do_gl_swap_buffers();
}
BSTONE_STATIC_THROW_NESTED_FUNC

RendererUPtr Window::make_renderer(const RendererInitParam& param)
try
{
	return do_make_renderer(param);
}
BSTONE_STATIC_THROW_NESTED_FUNC

} // namespace sys
} // namespace bstone
