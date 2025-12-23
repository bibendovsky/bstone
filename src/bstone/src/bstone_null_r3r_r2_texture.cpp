/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Null 3D renderer: 2D texture

#include "bstone_null_r3r_r2_texture.h"
#include <stddef.h>
#include <stdint.h>
#include "bstone_exception.h"
#include "bstone_fixed_pool_resource.h"
#include "bstone_r3r_limits.h"

// ==========================================================================

namespace bstone {

namespace {

class NullR3rR2TextureImpl final : public R3rR2Texture
{
public:
	NullR3rR2TextureImpl(const R3rR2TextureInitParam& param);
	~NullR3rR2TextureImpl() override {}

	void* operator new(size_t size);
	void operator delete(void* ptr);

private:
	void do_update(const R3rR2TextureUpdateParam& param) override;
	void do_generate_mipmaps() override;

private:
	using MemoryPool = FixedPoolResource<NullR3rR2TextureImpl, R3rLimits::max_textures()>;

private:
	static MemoryPool memory_pool_;
};

// --------------------------------------------------------------------------

NullR3rR2TextureImpl::MemoryPool NullR3rR2TextureImpl::memory_pool_{};

// --------------------------------------------------------------------------

NullR3rR2TextureImpl::NullR3rR2TextureImpl([[maybe_unused]] const R3rR2TextureInitParam& param)
{}

void* NullR3rR2TextureImpl::operator new(size_t size)
try {
	return memory_pool_.allocate(static_cast<intptr_t>(size));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void NullR3rR2TextureImpl::operator delete(void* ptr)
{
	memory_pool_.deallocate(ptr);
}

void NullR3rR2TextureImpl::do_update([[maybe_unused]] const R3rR2TextureUpdateParam& param)
{}

void NullR3rR2TextureImpl::do_generate_mipmaps()
{}

} // namespace

// ==========================================================================

NullR3rR2TextureUPtr make_null_r3r_r2_texture(const R3rR2TextureInitParam& param)
{
	return std::make_unique<NullR3rR2TextureImpl>(param);
}

} // namespace bstone
