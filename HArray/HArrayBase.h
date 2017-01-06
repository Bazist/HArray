/*
# Copyright(C) 2010-2017 Vyacheslav Makoveychuk (email: slv709@gmail.com, skype: vyacheslavm81)
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

#define _RELEASE 0x1234567

const uint32 REPOSITORY_VERSION = 1;

const uint32 COUNT_TEMPS = 50;

const uint32 BLOCK_ENGINE_BITS = 4; //bits of block
const uint32 BLOCK_ENGINE_SIZE = 16; //size of block
const uint32 BLOCK_ENGINE_SHIFT = 32 - BLOCK_ENGINE_BITS;
const uint32 BLOCK_ENGINE_STEP = 4;

//const uint32 BLOCK_ENGINE_BITS = 16; //bits of block
//const uint32 BLOCK_ENGINE_SIZE = 65536; //size of block
//const uint32 BLOCK_ENGINE_SHIFT = 32 - BLOCK_ENGINE_BITS;
//const uint32 BLOCK_ENGINE_STEP = 16;

const uint32 BRANCH_ENGINE_SIZE = 4; //can be changed

const uchar8 HEADER_JUMP_TYPE = 1;
const uchar8 HEADER_BRANCH_TYPE = HEADER_JUMP_TYPE + 1;
const uchar8 HEADER_CURRENT_VALUE_TYPE = HEADER_BRANCH_TYPE + 1;

const uchar8 EMPTY_TYPE = 0;
const uchar8 MIN_BRANCH_TYPE1 = 1;
const uchar8 MAX_BRANCH_TYPE1 = BRANCH_ENGINE_SIZE;
const uchar8 MIN_BRANCH_TYPE2 = BRANCH_ENGINE_SIZE + 1;
const uchar8 MAX_BRANCH_TYPE2 = BRANCH_ENGINE_SIZE * 2;
const uchar8 MIN_BLOCK_TYPE = MAX_BRANCH_TYPE2 + 1;
const uchar8 MAX_BLOCK_TYPE = MIN_BLOCK_TYPE + (32 / BLOCK_ENGINE_STEP) - 1;
const uchar8 VAR_TYPE = MAX_BLOCK_TYPE + 1; //for var value
const uchar8 CONTINUE_VAR_TYPE = VAR_TYPE + 1; //for continue var value
const uchar8 CURRENT_VALUE_TYPE = CONTINUE_VAR_TYPE + 1;
const uchar8 VALUE_TYPE = CURRENT_VALUE_TYPE + 1;
const uchar8 MIN_HEADER_BLOCK_TYPE = VALUE_TYPE + 1;
const uchar8 MAX_HEADER_BLOCK_TYPE = MIN_HEADER_BLOCK_TYPE + 71;
const uchar8 ONLY_CONTENT_TYPE = MAX_HEADER_BLOCK_TYPE + 1;

const uchar8 MOVES_LEVEL1_STAT = 0;
const uchar8 MOVES_LEVEL2_STAT = 1;
const uchar8 MOVES_LEVEL3_STAT = 2;
const uchar8 MOVES_LEVEL4_STAT = 3;
const uchar8 MOVES_LEVEL5_STAT = 4;
const uchar8 MOVES_LEVEL6_STAT = 5;
const uchar8 MOVES_LEVEL7_STAT = 6;
const uchar8 MOVES_LEVEL8_STAT = 7;
const uchar8 SHORT_WAY_STAT = 8;
const uchar8 LONG_WAY_STAT = 9;
const uchar8 CONTENT_BRANCH_STAT = 10;

const uchar8 CURRENT_VALUE_SEGMENT_TYPE = 1;
const uchar8 BRANCH_SEGMENT_TYPE = CURRENT_VALUE_SEGMENT_TYPE + 1;
const uchar8 BLOCK_VALUE_SEGMENT_TYPE = BRANCH_SEGMENT_TYPE + 1;
const uchar8 BLOCK_BRANCH1_SEGMENT_TYPE = BLOCK_VALUE_SEGMENT_TYPE + 1;
const uchar8 BLOCK_BRANCH2_SEGMENT_TYPE = BLOCK_BRANCH1_SEGMENT_TYPE + 1;
const uchar8 BLOCK_OFFSET_SEGMENT_TYPE = BLOCK_BRANCH2_SEGMENT_TYPE + 1;
const uchar8 VAR_SHUNT_SEGMENT_TYPE = BLOCK_OFFSET_SEGMENT_TYPE + 1;
const uchar8 VAR_VALUE_SEGMENT_TYPE = VAR_SHUNT_SEGMENT_TYPE + 1;

const uint32 MAX_COUNT_RELEASED_CONTENT_CELLS = MAX_SHORT * 2; //two pages
const uint32 MAX_COUNT_RELEASED_BRANCH_CELLS = MAX_SHORT;
const uint32 MAX_COUNT_RELEASED_BLOCK_CELLS = MAX_SHORT;
const uint32 MAX_COUNT_RELEASED_VAR_CELLS = MAX_SHORT;

const uchar8 MAX_KEY_SEGMENTS = MAX_CHAR - ONLY_CONTENT_TYPE;

typedef bool HARRAY_ITEM_VISIT_FUNC(uint32* key, uint32 keyLen, uint32 value, uchar8 valueType, void* pData);

struct HArrayFixBaseInfo
{
	uint32 Version;

	uint32 KeyLen;
	uint32 ValueLen;

	uint32 HeaderBase;

	uint32 ContentPagesCount;
	uint32 VarPagesCount;
	uint32 BranchPagesCount;
	uint32 BlockPagesCount;

	uint32 LastContentOffset;
	uint32 LastVarOffset;
	uint32 LastBranchOffset;
	uint32 LastBlockOffset;
};

struct HACursor
{
	uint32 CountFullContentPage;
	uint32 SizeLastContentPage;

	uint32 Page;
	uint32 Index;

	uint32* Value;
};

//struct ContentTypeCell
//{
//	uchar8 Type;
//};

struct HArrayPair
{
public:
	uint32 Key[16];
	uint32 Value;
	uint32 KeyLen;

	void print()
	{
		for(int i=0; i<KeyLen; i++)
		{
			printf("%u ", Key[i]);
		}

		printf("=> %u\n", Value);
	}
};

struct BranchCell
{
	uint32 Values[BRANCH_ENGINE_SIZE];
	uint32 Offsets[BRANCH_ENGINE_SIZE];
};

struct BlockCell
{
	uchar8 Type;
	uint32 Offset;
	uint32 ValueOrOffset;
};

struct ContentCell
{
	uchar8 Type;
	uint32 Value;
};

struct HeaderCell
{
	uchar8 Type;
	uint32 Offset;
};

struct HeaderBranchCell
{
	HeaderBranchCell()
	{
		HeaderOffset = 0;

		for (uint32 i = 0; i < BRANCH_ENGINE_SIZE; i++)
		{
			ParentIDs[i] = 0;
		}

		pNextHeaderBranhCell = 0;
	}

	uint32 HeaderOffset;

	uchar8 ParentIDs[BRANCH_ENGINE_SIZE];
	uint32 Offsets[BRANCH_ENGINE_SIZE];

	HeaderBranchCell* pNextHeaderBranhCell;
};

struct HeaderBranchPage
{
	HeaderBranchCell pHeaderBranch[MAX_SHORT];
};

struct VarCell
{
	ContentCell ValueContCell;
	ContentCell ContCell;
};

struct ContentPage
{
	ContentCell pContent[MAX_SHORT];
};

struct VarPage
{
	VarCell pVar[MAX_SHORT];
};

struct BranchPage
{
	BranchCell pBranch[MAX_SHORT];
};

struct BlockPage
{
	BlockPage()
	{
		for (uint32 i = 0; i < MAX_SHORT; i++)
		{
			pBlock[i].Type = 0;
		}
	}

	BlockCell pBlock[MAX_SHORT];
};

struct CompactPage
{
	CompactPage()
	{
		for (uint32 i = 0; i < MAX_CHAR; i++)
		{
			Values[i] = 0;
			Offsets[i] = 0;
		}

		Count = 0;
	}

	uint32 Values[MAX_CHAR];
	uint32 Offsets[MAX_CHAR];

	uint32 Count;

	CompactPage* pNextPage;
};

struct SegmentPath
{
	uchar8 Type;

	ContentCell* pContentCell;

	BlockCell* pBlockCell;
	uint32 StartBlockOffset;

	BranchCell* pBranchCell1;
	uint32 BranchOffset1;

	BranchCell* pBranchCell2;
	uint32 BranchOffset2;

	VarCell* pVarCell;
	uint32 VarOffset;

	uint32 BranchIndex;
	uint32 ContentOffset;
	uint32 BlockSubOffset;
};

typedef uint32 (*NormalizeFunc)(void* key);

typedef int (*CompareFunc)(void* key1, uint32 keyLen1, void* key2, uint32 keyLen2);

typedef int(*CompareSegmentFunc)(void* keySeg1, void* keySeg2, uint32 index);
