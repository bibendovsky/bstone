/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// Content path.
//


#ifndef BSTONE_CONTENT_PATH_INCLUDED
#define BSTONE_CONTENT_PATH_INCLUDED


#include <string>


namespace bstone
{


enum class ContentPathProvider
{
	none,
	gog,
	steam,
}; // ContentPathProvider


struct AssetPath
{
	ContentPathProvider provider_;

	std::string aog_;
	std::string ps_;
}; // AssetProvider


AssetPath make_content_path(
	const ContentPathProvider type);


} // bstone


#endif // !BSTONE_CONTENT_PATH_INCLUDED
