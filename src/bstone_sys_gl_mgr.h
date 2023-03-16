/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_SYS_GL_MGR_INCLUDED)
#define BSTONE_SYS_GL_MGR_INCLUDED

#include <memory>

namespace bstone {
namespace sys {

class GlMgr
{
public:
	GlMgr() = default;
	virtual ~GlMgr() = default;

	void load_default_library();
	void load_library(const char* path);
	void unload_library() noexcept;

	void* get_symbol(const char* symbol_name);

	template<typename T>
	T get_symbol(const char* symbol_name)
	{
		return reinterpret_cast<T>(get_symbol(symbol_name));
	}

	int get_swap_interval() noexcept;
	void set_swap_interval(int swap_interval);

private:
	virtual void do_load_library(const char* path) = 0;
	virtual void do_unload_library() noexcept = 0;

	virtual bool do_has_extension(const char* extension_name) = 0;
	virtual void* do_get_symbol(const char* symbol_name) = 0;

	virtual int do_get_swap_interval() noexcept = 0;
	virtual void do_set_swap_interval(int swap_interval) = 0;
};

// ==========================================================================

using GlMgrUPtr = std::unique_ptr<GlMgr>;

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_GL_MGR_INCLUDED
