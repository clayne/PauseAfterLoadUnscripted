/*************************************************************************
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
#pragma once

#include <spdlog/sinks/basic_file_sink.h>

extern std::shared_ptr<spdlog::logger> PALULogger;

// wrappers for spdLog to make release/debug logging easier
// Debug build only
#if _DEBUG || defined(_FULL_LOGGING)
#define DBG_DMESSAGE(a_fmt, ...) PALULogger->debug(a_fmt __VA_OPT__(,) __VA_ARGS__)
#define DBG_VMESSAGE(a_fmt, ...) PALULogger->trace(a_fmt __VA_OPT__(,) __VA_ARGS__)
#define DBG_MESSAGE(a_fmt, ...) PALULogger->info(a_fmt __VA_OPT__(,) __VA_ARGS__)
#define DBG_WARNING(a_fmt, ...) PALULogger->warn(a_fmt __VA_OPT__(,) __VA_ARGS__)
#define DBG_ERROR(a_fmt, ...) PALULogger->error(a_fmt __VA_OPT__(,) __VA_ARGS__)
#define DBG_FATALERROR(a_fmt, ...) PALULogger->critical(a_fmt __VA_OPT__(,) __VA_ARGS__)
#else
#define DBG_DMESSAGE(a_fmt, ...)
#define DBG_VMESSAGE(a_fmt, ...)
#define DBG_MESSAGE(a_fmt, ...)
#define DBG_WARNING(a_fmt, ...)
#define DBG_ERROR(a_fmt, ...)
#define DBG_FATALERROR(a_fmt, ...)
#endif

// Always log
#define REL_DMESSAGE(a_fmt, ...) PALULogger->debug(a_fmt __VA_OPT__(,) __VA_ARGS__)
#define REL_VMESSAGE(a_fmt, ...) PALULogger->trace(a_fmt __VA_OPT__(,) __VA_ARGS__)
#define REL_MESSAGE(a_fmt, ...) PALULogger->info(a_fmt __VA_OPT__(,) __VA_ARGS__)
#define REL_WARNING(a_fmt, ...) PALULogger->warn(a_fmt __VA_OPT__(,) __VA_ARGS__)
#define REL_ERROR(a_fmt, ...) PALULogger->error(a_fmt __VA_OPT__(,) __VA_ARGS__)
#define REL_FATALERROR(a_fmt, ...) PALULogger->critical(a_fmt __VA_OPT__(,) __VA_ARGS__)
