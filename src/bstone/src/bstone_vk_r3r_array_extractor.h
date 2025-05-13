/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer: Array extractor for the most of vkEnumerate-like functions.

#ifndef BSTONE_VK_R3R_ENUMERATOR_INCLUDED
#define BSTONE_VK_R3R_ENUMERATOR_INCLUDED

#include "vulkan/vk_platform.h"
#include <cstdint>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

namespace bstone {

namespace detail {

template<typename TResult, typename... TArgs>
struct VkR3rEnumeratorFuncTraits;

template<typename TResult, typename... TArgs>
struct VkR3rEnumeratorFuncTraits<auto (VKAPI_PTR *)(TArgs...) -> TResult>
{
	using Result = TResult;
	using ArgsTuple = std::tuple<TArgs...>;
};

// ======================================

template<typename TFunc>
struct VkR3rArrayExtractorHelper
{
	using Element = std::remove_pointer_t<std::tuple_element_t<
		std::tuple_size<typename detail::VkR3rEnumeratorFuncTraits<TFunc>::ArgsTuple>::value - 1,
		typename detail::VkR3rEnumeratorFuncTraits<TFunc>::ArgsTuple>>;

	using Array = std::vector<Element>;
};

template<typename TFunc>
using VkR3rArrayExtractorArray = typename detail::VkR3rArrayExtractorHelper<TFunc>::Array;

} // namespace detail

// ======================================

template<typename TFunc, typename... TArgs>
void vk_r3r_extract_array(
	TFunc func,
	detail::VkR3rArrayExtractorArray<TFunc>& dst_array,
	TArgs&&... args)
{
	std::uint32_t count = 0;
	func(std::forward<TArgs>(args)..., &count, nullptr);
	if (count == 0)
	{
		return;
	}
	dst_array.resize(count);
	func(std::forward<TArgs>(args)..., &count, dst_array.data());
	dst_array.resize(count);
}

template<typename TFunc, typename... TArgs>
auto vk_r3r_extract_array(
	TFunc func,
	TArgs&&... args) -> detail::VkR3rArrayExtractorArray<TFunc>
{
	typename detail::VkR3rArrayExtractorHelper<TFunc>::Array array{};
	vk_r3r_extract_array(func, array, std::forward<TArgs>(args)...);
	return array;
}

} // namespace bstone

#endif // BSTONE_VK_R3R_ENUMERATOR_INCLUDED
