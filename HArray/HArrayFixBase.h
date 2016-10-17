/*
# Copyright(C) 2010-2016 Vyacheslav Makoveychuk (email: slv709@gmail.com, skype: vyacheslavm81)
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


#pragma once

#ifndef _HARRAY_FIX_BASE		 // Allow use of features specific to Windows XP or later.                   
#define _HARRAY_FIX_BASE 0x709 // Change this to the appropriate value to target other versions of Windows.

#endif	

#include "stdafx.h"

//#define _RELEASE 0x1234567

const uint REPOSITORY_VERSION = 1;

const uint COUNT_TEMPS = 50;

const uint BLOCK_ENGINE_BITS = 4; //bits of block
const uint BLOCK_ENGINE_SIZE = 16; //size of block
const uint BLOCK_ENGINE_SHIFT = 32 - BLOCK_ENGINE_BITS;
const uint BLOCK_ENGINE_STEP = 4;

//const uint BLOCK_ENGINE_BITS = 16; //bits of block
//const uint BLOCK_ENGINE_SIZE = 65536; //size of block
//const uint BLOCK_ENGINE_SHIFT = 32 - BLOCK_ENGINE_BITS;
//const uint BLOCK_ENGINE_STEP = 16;

const uint BRANCH_ENGINE_SIZE = 4; //can be changed

const uchar EMPTY_TYPE = 0;
const uchar MIN_BRANCH_TYPE1 = 1;
const uchar MAX_BRANCH_TYPE1 = BRANCH_ENGINE_SIZE;
const uchar MIN_BRANCH_TYPE2 = BRANCH_ENGINE_SIZE + 1;
const uchar MAX_BRANCH_TYPE2 = BRANCH_ENGINE_SIZE * 2;
const uchar MIN_BLOCK_TYPE = 50;
const uchar MAX_BLOCK_TYPE = MIN_BLOCK_TYPE + (32 / BLOCK_ENGINE_STEP) - 1;
const uchar VAR_TYPE = 125; //for var value
const uchar CONTINUE_VAR_TYPE = 126; //for continue var value
const uchar CURRENT_VALUE_TYPE = 127;
const uchar VALUE_TYPE = 128;
const uchar ONLY_CONTENT_TYPE = 129;

typedef bool HARRAY_ITEM_VISIT_FUNC(uint* key, uint keyLen, uint value);

struct HArrayFixBaseInfo
{
	uint Version;
	
	uint KeyLen;
	uint ValueLen;

	uint HeaderBase;
	
	uint ContentPagesCount;
	uint VarPagesCount;
	uint BranchPagesCount;
	uint BlockPagesCount;

	uint LastContentOffset;
	uint LastVarOffset;
	uint LastBranchOffset;
	uint LastBlockOffset;
};

//struct ContentTypeCell
//{
//	uchar Type;
//};

struct HArrayFixPair
{
	uint Key[16];
	uint Value;
	uint KeyLen;

	int compareTo(HArrayFixPair& pair)
	{
		for(uint i=0; i<KeyLen; i++)
		{
			if(Key[i] < pair.Key[i])
				return -1;
			
			if(Key[i] > pair.Key[i])
				return 1;
		}

		return 0;
	}
};

struct BranchCell
{
	uint Values[BRANCH_ENGINE_SIZE];
	uint Offsets[BRANCH_ENGINE_SIZE];
};

struct BlockCell
{
	uchar Type;
	uint Offset;
	uint ValueOrOffset;
};

struct ContentCell
{
	uchar Type;
	uint Value;
};

struct VarCell
{
	ContentCell ContentCell;
	uint Value;
};

class ContentPage
{
public:
	ContentCell pContent[MAX_SHORT];
};

class VarPage
{
public:
	VarCell pVar[MAX_SHORT];
};

class BranchPage
{
public:
	BranchCell pBranch[MAX_SHORT];
};

class BlockPage
{
public:
	BlockPage()
	{
		for(uint i=0; i<MAX_SHORT; i++)
		{
			pBlock[i].Type = 0;
		}
	}

	BlockCell pBlock[MAX_SHORT];
};