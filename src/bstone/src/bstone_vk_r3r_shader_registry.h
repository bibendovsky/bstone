/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer: Shader registry

#ifndef BSTONE_VK_R3R_SHADER_REGISTRY_INCLUDED
#define BSTONE_VK_R3R_SHADER_REGISTRY_INCLUDED

namespace bstone {

struct VkR3rShaderCodeInfo
{
	const unsigned char* bytes;
	int byte_count;
};

class VkR3rShaderRegistry
{
public:
	static VkR3rShaderCodeInfo get_post_vert();
	static VkR3rShaderCodeInfo get_post_frag();

	static VkR3rShaderCodeInfo get_post_ms_vert();
	static VkR3rShaderCodeInfo get_post_ms_frag();
};

} // namespace bstone

#endif // BSTONE_VK_R3R_SHADER_REGISTRY_INCLUDED
