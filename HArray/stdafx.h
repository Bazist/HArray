/*
# Copyright(C) 2010-2016 Vyacheslav Makoveichuk
# This file is part of VyMa\Trie.
#
# VyMa\Trie is free software : you can redistribute it and / or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Vyma\Trie is distributed in the hope that it will be useful,
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

#include <stdio.h>
#include <tchar.h>
#include <string.h>

#define uint unsigned int
#define ulong unsigned __int64
#define ushort unsigned short
#define uchar unsigned char
#define ucode unsigned char

const uint MAX_SHORT = 65536;
const uint MAX_CHAR = 256;
const uint INIT_MAX_PAGES = 1024;
const uint PAGE_SIZE = MAX_SHORT*2;
const uchar BLOCK_SIZE = 16;
const uchar ROW_LEN = 3;

// TODO: reference additional headers your program requires here
