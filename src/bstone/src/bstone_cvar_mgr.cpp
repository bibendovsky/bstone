/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <cassert>
#include <string>
#include <vector>
#include <unordered_map>
#include "bstone_cvar_mgr.h"
#include "bstone_exception.h"
#include "bstone_string_view_hasher.h"

namespace bstone {

CVarMgr::CVarMgr() = default;

CVarMgr::~CVarMgr() = default;

// ==========================================================================

class CVarMgrImpl final : public CVarMgr
{
public:
	CVarMgrImpl(std::intptr_t max_cvars);
	CVarMgrImpl(const CVarMgrImpl&) = delete;
	CVarMgrImpl& operator=(const CVarMgrImpl&) = delete;
	~CVarMgrImpl() override;

	CVar* find(StringView name) const noexcept override;
	CVarMgrCVars get_all() noexcept override;

	void add(CVar& cvar) override;

private:
	using CVars = std::vector<CVar*>;
	using NameToIndex = std::unordered_map<StringView, CVars::size_type, StringViewHasher>;

private:
	std::intptr_t max_cvars_{};
	CVars cvars_{};
	NameToIndex name_to_index_map_{};
};

// --------------------------------------------------------------------------

CVarMgrImpl::CVarMgrImpl(std::intptr_t max_cvars)
	:
	max_cvars_{max_cvars}
{}

CVarMgrImpl::~CVarMgrImpl() = default;

CVar* CVarMgrImpl::find(StringView name) const noexcept
{
	const auto item_iter = name_to_index_map_.find(name);

	if (item_iter == name_to_index_map_.cend())
	{
		return nullptr;
	}

	return cvars_[item_iter->second];
}

CVarMgrCVars CVarMgrImpl::get_all() noexcept
{
	return CVarMgrCVars{cvars_.data(), static_cast<std::intptr_t>(cvars_.size())};
}

void CVarMgrImpl::add(CVar& cvar)
try {
	if (cvars_.size() == static_cast<CVars::size_type>(max_cvars_))
	{
		BSTONE_THROW_STATIC_SOURCE("Too many CVARs.");
	}

	const auto name = cvar.get_name();
	const auto existing_cvar = find(name);

	if (existing_cvar != nullptr)
	{
		auto message = std::string{};
		message.reserve(128);
		message += "CVAR \"";
		message.append(name.get_data(), name.get_size());
		message += "\" already registered.";
		BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
	}

	cvars_.emplace_back(&cvar);
	name_to_index_map_[name] = cvars_.size() - 1;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

CVarMgrUPtr make_cvar_mgr(std::intptr_t max_cvars)
{
	return std::make_unique<CVarMgrImpl>(max_cvars);
}

} // namespace bstone
