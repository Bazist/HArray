/*
# Copyright(C) 2010-2017 Viacheslav Makoveichuk (email: slv709@gmail.com, skype: vyacheslavm81)
# This file is part of HArray.
#
# HArray is free software : you can redistribute it and / or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# HArray is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif

#include <string>
#include <cstring>
#include <stdio.h>
#include <inttypes.h>

#ifdef _WIN32
typedef __int32_t int32_t;
typedef __int64 long64;
typedef unsigned __int32_t int32_t;
typedef unsigned __int64 uint64_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int8 ucode8;
#endif

#ifdef linux
typedef int32_t_t int32_t;
typedef int64_t long64;
typedef int32_t_t int32_t;
typedef uint64_t uint64_t;
typedef uint16_t uint16_t;
typedef uint8_t uint8_t;
typedef uint8_t ucode8;
#endif

const int32_t MAX_CHAR = 256;
const int32_t MAX_SHORT = 65536;

const int32_t INIT_MAX_PAGES = 256;
const int32_t PAGE_SIZE = MAX_SHORT*2;
const uint8_t BLOCK_SIZE = 16;
const uint8_t ROW_LEN = 3;

// TODO: reference additional headers your program requires here
