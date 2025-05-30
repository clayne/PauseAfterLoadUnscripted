﻿/*************************************************************************
PauseAfterLoadUnscripted
Copyright (c) Steve Townsend 2021

>>> SOURCE LICENSE >>>
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation (www.fsf.org); either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

A copy of the GNU General Public License is available at
http://www.fsf.org/licensing/licenses
>>> END OF LICENSE >>>
*************************************************************************/
#include "PrecompiledHeaders.h"

#include "Utilities/utils.h"

#include <psapi.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <algorithm> //Trim
#include <windows.h>
#include <math.h>	// pow
#include <locale>

#undef GetEnvironmentVariable
#undef GetFileVersionInfo
#undef GetFileVersionInfoSize
#undef GetModuleFileName
#undef GetModuleHandle
#undef LoadLibrary
#undef MessageBox
#undef OutputDebugString
#undef RegQueryValueEx
#undef VerQueryValue

namespace FileUtils
{
	std::wstring GetGamePath(void)
	{
		static std::wstring s_runtimeDirectory;
		if (s_runtimeDirectory.empty())
		{
			wchar_t	runtimePathBuf[MAX_PATH];
			uint32_t	runtimePathLength = SKSE::WinAPI::GetModuleFileName(
				SKSE::WinAPI::GetModuleHandle((const wchar_t *)(NULL)), runtimePathBuf, sizeof(runtimePathBuf));
			if (runtimePathLength && (runtimePathLength < sizeof(runtimePathBuf)))
			{
				std::wstring runtimePath(runtimePathBuf, runtimePathLength);
				std::wstring::size_type	lastSlash = runtimePath.rfind('\\');
				if (lastSlash != std::string::npos)
					s_runtimeDirectory = runtimePath.substr(0, lastSlash + 1);
			}
			REL_MESSAGE("GetGamePath result: {}", StringUtils::FromUnicode(s_runtimeDirectory));
		}
		return s_runtimeDirectory;
	}

	std::wstring GetPluginFileName() noexcept
	{
		static std::wstring s_pluginFileName;
		if (s_pluginFileName.empty())
		{
			HMODULE hm = NULL;
			wchar_t path[MAX_PATH];
			if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
				GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
				(LPCWSTR)&GetPluginPath, &hm) == 0)
			{
				REL_ERROR("GetModuleHandle failed, error = {}\n", GetLastError());
			}
			else if (SKSE::WinAPI::GetModuleFileName(hm, path, sizeof(path)) == 0)
			{
				REL_ERROR("GetModuleFileName failed, error = {}\n", GetLastError());
			}
			else
			{
				s_pluginFileName = path;
			}
		}
		return s_pluginFileName;
	}

	std::wstring GetPluginPath() noexcept
	{
		static std::wstring s_skseDirectory;
		if (s_skseDirectory.empty())
		{
			std::wstring pluginFileName(GetPluginFileName());
			if (!pluginFileName.empty())
			{
			    wchar_t drive[MAX_PATH];
				memset(drive, 0, MAX_PATH);
				wchar_t dir[MAX_PATH];
				memset(dir, 0, MAX_PATH);
				if (_wsplitpath_s(pluginFileName.c_str(), drive, MAX_PATH, dir, MAX_PATH, nullptr, 0, nullptr, 0) == 0)
				{
					wchar_t path[MAX_PATH];
					memset(path, 0, MAX_PATH);
					if (_wmakepath_s(path, drive, dir, nullptr, nullptr) == 0)
					{
					    s_skseDirectory = path;
						REL_MESSAGE("GetPluginPath result: {}", StringUtils::FromUnicode(s_skseDirectory));
					}
				}
			}
		}
		return s_skseDirectory;
	}
}

namespace utils
{
	double GetGameSettingFloat(const RE::BSFixedString& name)
	{
		RE::Setting* setting(nullptr);
		RE::GameSettingCollection* settings(RE::GameSettingCollection::GetSingleton());
		if (settings)
		{
			setting = settings->GetSetting(name.c_str());
		}

		if (!setting || setting->GetType() != RE::Setting::Type::kFloat)
			return 0.0;

		double result(setting->GetFloat());
		REL_MESSAGE("Game Setting({})={:0.3f}", name.c_str(), result);
		return result;
	}
}

namespace WindowsUtils
{
	unsigned long long microsecondsNow() {
		static LARGE_INTEGER s_frequency;
		static BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);
		if (s_use_qpc) {
			LARGE_INTEGER now;
			QueryPerformanceCounter(&now);
			// To guard against loss-of-precision, we convert
			// to microseconds *before* dividing by ticks-per-second.
			return static_cast<unsigned long long>((1000000LL * now.QuadPart) / s_frequency.QuadPart);
		}
		else {
			return GetTickCount64() * 1000ULL;
		}
	}

	void LogProcessWorkingSet()
	{
		PROCESS_MEMORY_COUNTERS pmc;
		if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
		{
			REL_MESSAGE("Working Set is {:0.3f} MB", double(pmc.WorkingSetSize) / (1024.0 * 1024.0));
		}
	}

	void TakeNap(const double delaySeconds)
	{
		constexpr double PragmaticLongestWait = 10.0;
		if (delaySeconds < 0.0 || delaySeconds > PragmaticLongestWait)
		{
			REL_ERROR("Adjust requested wait of {:0.2f} seconds to {:0.2f}", delaySeconds, PragmaticLongestWait);
		}
		REL_MESSAGE("wait for {} milliseconds", static_cast<long long>(delaySeconds * 1000.0));

		// flush log output here
		PALULogger->flush();

		auto nextRunTime = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(static_cast<long long>(delaySeconds * 1000.0));
		std::this_thread::sleep_until(nextRunTime);
	}

	ScopedTimer::ScopedTimer(const std::string& context) : m_startTime(microsecondsNow()), m_context(context) {}

	ScopedTimer::~ScopedTimer()
	{
		unsigned long long endTime(microsecondsNow());
		REL_MESSAGE("TIME({})={} micros", m_context.c_str(), endTime - m_startTime);
	}

	std::unique_ptr<ScopedTimerFactory> ScopedTimerFactory::m_instance;

	ScopedTimerFactory& ScopedTimerFactory::Instance()
	{
		if (!m_instance)
		{
			m_instance = std::make_unique<ScopedTimerFactory>();
		}
		return *m_instance;
	}

	int ScopedTimerFactory::StartTimer(const std::string& context)
	{
#ifdef _PROFILING
		RecursiveLockGuard guard(m_timerLock);
		int handle(++m_nextHandle);
		m_timerByHandle.insert(std::make_pair(handle, std::make_unique<ScopedTimer>(context)));
		return handle;
#else
		return 0;
#endif
	}

	// fails silently if invalid, otherwise stops the timer and records result
	void ScopedTimerFactory::StopTimer(const int handle)
	{
#ifdef _PROFILING
		RecursiveLockGuard guard(m_timerLock);
		m_timerByHandle.erase(handle);
#endif
	}
}

namespace StringUtils
{
	std::locale DefaultLocale;
	void ToLower(std::string &str)
	{
		for (auto &c : str)
			c = std::tolower(c, DefaultLocale);
	}
	void ToUpper(std::string& str)
	{
		for (auto& c : str)
			c = std::toupper(c, DefaultLocale);
	}

	bool Replace(std::string &str, const std::string target, const std::string replacement)
	{
		if (str.empty() || target.empty())
			return false;

		std::string::size_type pos = 0;
		while ((pos = str.find(target, pos)) != std::string::npos)
		{
			str.replace(pos, target.length(), replacement);
			pos += replacement.length();
		}
		return true;
	}

	// see https://stackoverflow.com/questions/215963/how-do-you-properly-use-widechartomultibyte
	std::string FromUnicode(const std::wstring& input) {
		if (input.empty()) return std::string();

		int size_needed = WideCharToMultiByte(CP_UTF8, 0, &input[0], static_cast<int>(input.size()), NULL, 0, 0, 0);
		if (size_needed == 0) return std::string();

		std::string output(static_cast<size_t>(size_needed), 0);
		int result(WideCharToMultiByte(CP_UTF8, 0, &input[0], static_cast<int>(input.size()), &output[0], size_needed, 0, 0));
		if (result == 0) return std::string();

		return output;
	}

	std::string FormIDString(const RE::FormID formID)
	{
		std::ostringstream formIDStr;
		formIDStr << "0x" << std::hex << std::setw(8) << std::setfill('0') << formID;
		std::string result(formIDStr.str());
		REL_VMESSAGE("FormID 0x{:08x} mapped to {}", formID, result.c_str());
		return result;
	}
}

namespace GameSettingUtils
{
	std::string GetGameLanguage()
	{
		RE::Setting	* setting = RE::GetINISetting("sLanguage:General");
		std::string sLanguage = (setting && setting->GetType() == RE::Setting::Type::kString) ? setting->GetString() : "ENGLISH";
		return sLanguage;
	}
}
