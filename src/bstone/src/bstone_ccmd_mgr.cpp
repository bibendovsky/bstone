/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <cassert>
#include <string>
#include <vector>
#include <unordered_map>
#include "bstone_ccmd_mgr.h"
#include "bstone_exception.h"
#include "bstone_string_view_hasher.h"

namespace bstone {

CCmdMgr::CCmdMgr() = default;

CCmdMgr::~CCmdMgr() = default;

// ==========================================================================

class CCmdMgrImpl final : public CCmdMgr
{
public:
	CCmdMgrImpl(std::intptr_t max_ccmds);
	CCmdMgrImpl(const CCmdMgrImpl&) = delete;
	CCmdMgrImpl& operator=(const CCmdMgrImpl&) = delete;
	~CCmdMgrImpl() override;

	CCmd* find(StringView name) const noexcept override;
	CCmdMgrCCmds get_all() noexcept override;

	void add(CCmd& ccmd) override;

private:
	using CCmds = std::vector<CCmd*>;
	using NameToIndex = std::unordered_map<StringView, CCmds::size_type, StringViewHasher>;

private:
	std::intptr_t max_ccmds_{};
	CCmds ccmds_{};
	NameToIndex name_to_index_map_{};
};

// --------------------------------------------------------------------------

CCmdMgrImpl::CCmdMgrImpl(std::intptr_t max_ccmds)
	:
	max_ccmds_{max_ccmds}
{}

CCmdMgrImpl::~CCmdMgrImpl() = default;

CCmd* CCmdMgrImpl::find(StringView name) const noexcept
{
	const auto item_iter = name_to_index_map_.find(name);

	if (item_iter == name_to_index_map_.cend())
	{
		return nullptr;
	}

	return ccmds_[item_iter->second];
}

CCmdMgrCCmds CCmdMgrImpl::get_all() noexcept
{
	return CCmdMgrCCmds{ccmds_.data(), static_cast<std::intptr_t>(ccmds_.size())};
}

void CCmdMgrImpl::add(CCmd& ccmd)
try {
	if (ccmds_.size() == static_cast<CCmds::size_type>(max_ccmds_))
	{
		BSTONE_THROW_STATIC_SOURCE("Too many CCMDs.");
	}

	const auto name = ccmd.get_name();
	const auto existing_ccmd = find(name);

	if (existing_ccmd != nullptr)
	{
		auto message = std::string{};
		message.reserve(64);
		message += "CCMD \"";
		message.append(name.get_data(), name.get_size());
		message += "\" already registered.";
		BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
	}

	ccmds_.emplace_back(&ccmd);
	name_to_index_map_[name] = ccmds_.size() - 1;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

CCmdMgrUPtr make_ccmd_mgr(std::intptr_t max_ccmds)
{
	return std::make_unique<CCmdMgrImpl>(max_ccmds);
}

} // namespace bstone
