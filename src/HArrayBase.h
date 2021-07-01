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

#pragma once

#ifndef _HARRAY_FIX_BASE		 // Allow use of features specific to Windows XP or later.
#define _HARRAY_FIX_BASE 0x709 // Change this to the appropriate value to target other versions of Windows.

#endif

#include "stdafx.h"
#define _RELEASE 0x1234567

const int32_t REPOSITORY_VERSION = 1;

const int32_t COUNT_TEMPS = 50;

const int32_t BLOCK_ENGINE_BITS = 4; //bits of block
const int32_t BLOCK_ENGINE_SIZE = 16; //size of block
const int32_t BLOCK_ENGINE_SHIFT = 32 - BLOCK_ENGINE_BITS;
const int32_t BLOCK_ENGINE_STEP = 4;

//const int32_t BLOCK_ENGINE_BITS = 16; //bits of block
//const int32_t BLOCK_ENGINE_SIZE = 65536; //size of block
//const int32_t BLOCK_ENGINE_SHIFT = 32 - BLOCK_ENGINE_BITS;
//const int32_t BLOCK_ENGINE_STEP = 16;

const int32_t BRANCH_ENGINE_SIZE = 4; //can be changed

const uint8_t EMPTY_TYPE = 0;
const uint8_t MIN_BRANCH_TYPE1 = 1;
const uint8_t MAX_BRANCH_TYPE1 = BRANCH_ENGINE_SIZE;
const uint8_t MIN_BRANCH_TYPE2 = BRANCH_ENGINE_SIZE + 1;
const uint8_t MAX_BRANCH_TYPE2 = BRANCH_ENGINE_SIZE * 2;
const uint8_t MIN_BLOCK_TYPE = MAX_BRANCH_TYPE2 + 1;
const uint8_t MAX_BLOCK_TYPE = MIN_BLOCK_TYPE + (32 / BLOCK_ENGINE_STEP) - 1;
const uint8_t VAR_TYPE = MAX_BLOCK_TYPE + 1; //for var value
const uint8_t CONTINUE_VAR_TYPE = VAR_TYPE + 1; //for continue var value
const uint8_t CURRENT_VALUE_TYPE = CONTINUE_VAR_TYPE + 1;
const uint8_t VALUE_TYPE = CURRENT_VALUE_TYPE + 1;
const uint8_t ONLY_CONTENT_TYPE = VALUE_TYPE + 1;

const uint8_t MOVES_LEVEL1_STAT = 0;
const uint8_t MOVES_LEVEL2_STAT = 1;
const uint8_t MOVES_LEVEL3_STAT = 2;
const uint8_t MOVES_LEVEL4_STAT = 3;
const uint8_t MOVES_LEVEL5_STAT = 4;
const uint8_t MOVES_LEVEL6_STAT = 5;
const uint8_t MOVES_LEVEL7_STAT = 6;
const uint8_t MOVES_LEVEL8_STAT = 7;
const uint8_t SHORT_WAY_STAT = 8;
const uint8_t LONG_WAY_STAT = 9;
const uint8_t CONTENT_BRANCH_STAT = 10;

const uint8_t CURRENT_VALUE_SEGMENT_TYPE = 1;
const uint8_t BRANCH_SEGMENT_TYPE = CURRENT_VALUE_SEGMENT_TYPE + 1;
const uint8_t BLOCK_VALUE_SEGMENT_TYPE = BRANCH_SEGMENT_TYPE + 1;
const uint8_t BLOCK_BRANCH1_SEGMENT_TYPE = BLOCK_VALUE_SEGMENT_TYPE + 1;
const uint8_t BLOCK_BRANCH2_SEGMENT_TYPE = BLOCK_BRANCH1_SEGMENT_TYPE + 1;
const uint8_t BLOCK_OFFSET_SEGMENT_TYPE = BLOCK_BRANCH2_SEGMENT_TYPE + 1;
const uint8_t VAR_SHUNT_SEGMENT_TYPE = BLOCK_OFFSET_SEGMENT_TYPE + 1;
const uint8_t VAR_VALUE_SEGMENT_TYPE = VAR_SHUNT_SEGMENT_TYPE + 1;

const int32_t MIN_COUNT_RELEASED_CONTENT_CELLS = MAX_SHORT * 2; //two pages
const int32_t MIN_COUNT_RELEASED_BRANCH_CELLS = MAX_SHORT;
const int32_t MIN_COUNT_RELEASED_BLOCK_CELLS = MAX_SHORT;
const int32_t MIN_COUNT_RELEASED_VAR_CELLS = MAX_SHORT;

const uint8_t MAX_KEY_SEGMENTS = MAX_CHAR - ONLY_CONTENT_TYPE;
const uint8_t MIN_HEADER_BASE_BITS = 14;		  //16384 slots
const uint8_t MAX_HEADER_FILL_FACTOR_BITS = 4; //fill factor 1/16 of header size

typedef bool HARRAY_ITEM_VISIT_FUNC(int32_t* key, int32_t keyLen, int32_t value, void* pData);

struct HArrayFixBaseInfo
{
	int32_t Version;

	int32_t KeyLen;
	int32_t ValueLen;

	int32_t HeaderBase;

	int32_t ContentPagesCount;
	int32_t VarPagesCount;
	int32_t BranchPagesCount;
	int32_t BlockPagesCount;

	int32_t LastContentOffset;
	int32_t LastVarOffset;
	int32_t LastBranchOffset;
	int32_t LastBlockOffset;
};

struct HACursor
{
	int32_t CountFullContentPage;
	int32_t SizeLastContentPage;

	int32_t Page;
	int32_t Index;

	int32_t* Value;
};

//struct ContentTypeCell
//{
//	uint8_t Type;
//};

struct HArrayPair
{
public:
	HArrayPair(int32_t maxKeyLen)
	{
		Key = new int32_t[maxKeyLen];
		KeyLen = 0;
		Value = 0;
		ValueType = 0;
	}

	int32_t* Key;
	int32_t KeyLen;
	int32_t Value;
	uint8_t ValueType;

	inline void setPair(int32_t skipKeyLen,
						int32_t* key,
						int32_t keyLen,
						int32_t value,
						uint8_t valueType)
	{
		for (int32_t i=0; skipKeyLen < keyLen; i++, skipKeyLen++)
		{
			Key[i] = key[skipKeyLen];
		}

		KeyLen = keyLen;
		Value = value;
		ValueType = valueType;
	}

	void print()
	{
		for(int32_t i = 0; i < KeyLen; i++)
		{
			printf("%u ", Key[i]);
		}

		printf("=> %u\n", Value);
	}

	~HArrayPair()
	{
		delete[] Key;
	}
};

struct BranchCell
{
	int32_t Values[BRANCH_ENGINE_SIZE];
	int32_t Offsets[BRANCH_ENGINE_SIZE];
};

struct BlockCell
{
	uint8_t Type;
	int32_t Offset;
	int32_t ValueOrOffset;
};

//struct ContentCell
//{
//	uint8_t Type;
//	int32_t Value;
//};

struct VarCell
{
	uint8_t ValueContCellType;
	int32_t ValueContCellValue;

	uint8_t ContCellType;
	int32_t ContCellValue;
};

struct ContentPage
{
	uint8_t pType[MAX_SHORT];
	int32_t pContent[MAX_SHORT];
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
		memset(this, 0, sizeof(BlockPage));
	}

	BlockCell pBlock[MAX_SHORT];
};

struct CompactPage
{
	CompactPage()
	{
		memset(this, 0, sizeof(CompactPage));
	}

	int32_t Values[MAX_CHAR];
	int32_t Offsets[MAX_CHAR];

	int32_t Count;

	CompactPage* pNextPage;
};

struct SegmentPath
{
	uint8_t Type;

	uint8_t* pContentCellType;
	int32_t* pContentCellValue;

	BlockCell* pBlockCell;
	int32_t StartBlockOffset;

	BranchCell* pBranchCell1;
	int32_t BranchOffset1;

	BranchCell* pBranchCell2;
	int32_t BranchOffset2;

	VarCell* pVarCell;
	int32_t VarOffset;

	int32_t BranchIndex;
	int32_t ContentOffset;
	int32_t BlockSubOffset;

	void print()
	{
		/*
		printf("Type: %u, ", Type);

		if(pContentCell)
			printf("ContentCell: Type=%u, Value=%u, ", pContentCell->Type, pContentCell->Value);

		if(pBlockCell)
			printf("BlockCell: Type=%u, Offset=%u, ValueOrOffset=%u, ", pBlockCell->Type, pBlockCell->Offset, pBlockCell->ValueOrOffset);

		printf("StartBlockOffset: %u, ", StartBlockOffset);

		if (pBranchCell1)
			printf("pBranchCell1: Type=%u, Offset=%u, ValueOrOffset=%u, ", pBlockCell->Type, pBlockCell->Offset, pBlockCell->ValueOrOffset);
		*/
	}
};

typedef int32_t (*NormalizeFunc)(void* key);

typedef int (*CompareFunc)(void* key1, int32_t keyLen1, void* key2, int32_t keyLen2);

typedef int(*CompareSegmentFunc)(void* keySeg1, void* keySeg2, int32_t index);
