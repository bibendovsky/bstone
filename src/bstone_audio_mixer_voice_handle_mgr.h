/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_AUDIO_MIXER_VOICE_HANDLE_MGR_INCLUDED
#define BSTONE_AUDIO_MIXER_VOICE_HANDLE_MGR_INCLUDED

#include <cstddef>
#include <limits>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include "bstone_audio_mixer_voice_handle.h"
#include "bstone_spinlock.h"

namespace bstone
{

[[noreturn]] void audio_mixer_voice_handle_mgr_fail(const char* message);
[[noreturn]] void audio_mixer_voice_handle_mgr_fail_nested(const char* message);

// ==========================================================================

template<typename TVoice>
class AudioMixerVoiceHandleMgr
{
public:
	using Voice = TVoice;

	AudioMixerVoiceHandleMgr() noexcept
	{
	}

	void set_cache_capacity(int capacity)
	try
	{
		if (capacity < 0)
		{
			audio_mixer_voice_handle_mgr_fail("Capacity out of range.");
		}

		const auto guard = MutexLock{mutex_};
		cache_.reserve(capacity);
	}
	catch (...)
	{
		audio_mixer_voice_handle_mgr_fail_nested(__func__);
	}

	void set_map_capacity(int capacity)
	try
	{
		if (capacity < 0)
		{
			audio_mixer_voice_handle_mgr_fail("Capacity out of range.");
		}

		const auto guard = MutexLock{mutex_};
		map_.reserve(capacity);
	}
	catch (...)
	{
		audio_mixer_voice_handle_mgr_fail_nested(__func__);
	}

	AudioMixerVoiceHandle generate()
	try
	{
		const auto guard = MutexLock{mutex_};
		handle_ = ++handle_;
		return handle_;
	}
	catch (...)
	{
		audio_mixer_voice_handle_mgr_fail_nested(__func__);
	}

	void cache(AudioMixerVoiceHandle handle)
	try
	{
		validate_handle(handle);
		const auto guard = MutexLock{mutex_};
		cache_.insert(handle);
	}
	catch (...)
	{
		audio_mixer_voice_handle_mgr_fail_nested(__func__);
	}

	void uncache(AudioMixerVoiceHandle handle)
	try
	{
		const auto guard = MutexLock{mutex_};
		cache_.erase(handle);
	}
	catch (...)
	{
		audio_mixer_voice_handle_mgr_fail_nested(__func__);
	}

	void uncache_and_map(AudioMixerVoiceHandle handle, Voice* voice)
	try
	{
		validate_voice(voice);
		const auto guard = MutexLock{mutex_};
		const auto erased_count = cache_.erase(handle);

		if (erased_count == 0)
		{
			audio_mixer_voice_handle_mgr_fail("Uncached handle.");
		}

		map_.emplace(handle, voice);
	}
	catch (...)
	{
		audio_mixer_voice_handle_mgr_fail_nested(__func__);
	}

	void unmap(AudioMixerVoiceHandle handle)
	try
	{
		const auto guard = MutexLock{mutex_};
		map_.erase(handle);
	}
	catch (...)
	{
		audio_mixer_voice_handle_mgr_fail_nested(__func__);
	}

	bool is_valid_handle(AudioMixerVoiceHandle handle) const noexcept
	{
		if (!handle.is_valid())
		{
			return false;
		}

		const auto guard = MutexLock{mutex_};
		return cache_.count(handle) > 0 || map_.count(handle) > 0;
	}

	void invalidate(AudioMixerVoiceHandle handle) noexcept
	{
		if (!handle.is_valid())
		{
			return;
		}

		const auto guard = MutexLock{mutex_};
		cache_.erase(handle);
		map_.erase(handle);
	}

	Voice* get_voice(AudioMixerVoiceHandle handle) noexcept
	{
		if (!handle.is_valid())
		{
			return nullptr;
		}

		const auto guard = MutexLock{mutex_};
		const auto map_it = map_.find(handle);

		if (map_it == map_.cend())
		{
			return nullptr;
		}

		return map_it->second;
	}

	Voice* get_voice_and_invalidate(AudioMixerVoiceHandle handle) noexcept
	{
		if (!handle.is_valid())
		{
			return nullptr;
		}

		const auto guard = MutexLock{mutex_};

		if (cache_.count(handle) > 0)
		{
			cache_.erase(handle);
			return nullptr;
		}

		const auto map_it = map_.find(handle);

		if (map_it == map_.cend())
		{
			return nullptr;
		}

		const auto voice = map_it->second;
		map_.erase(map_it);
		return voice;
	}

private:
	using Mutex = Spinlock;
	using MutexLock = std::unique_lock<Mutex>;

	using Cache = std::unordered_set<AudioMixerVoiceHandle, AudioMixerVoiceHandleStdHasher>;
	using Map = std::unordered_map<AudioMixerVoiceHandle, Voice*, AudioMixerVoiceHandleStdHasher>;

	mutable Mutex mutex_{};
	AudioMixerVoiceHandle handle_{};
	Cache cache_{};
	Map map_{};

	static void validate_handle(AudioMixerVoiceHandle handle)
	{
		if (!handle.is_valid())
		{
			audio_mixer_voice_handle_mgr_fail("Invalid handle.");
		}
	}

	static void validate_voice(Voice* voice)
	{
		if (!voice)
		{
			audio_mixer_voice_handle_mgr_fail("Null voice.");
		}
	}
}; // AudioMixerVoiceHandleMgr

} // bstone

#endif // !BSTONE_AUDIO_MIXER_VOICE_HANDLE_MGR_INCLUDED
