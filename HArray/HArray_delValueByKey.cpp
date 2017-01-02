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

#include "stdafx.h"
#include "HArray.h"

/*

Main strategy
1. OnlyContentType => zero content, zero header
2. CurrentValue => ???
3. Branch => Decrease branch => Remove branch + Current Value in content
4. Branch1 in Block => Decrease branch => Remove branch + inject in block
5. Branch2 in Block => Decrease branch => Remove branch + inject in block
6. In Low Block less than 8 values => Remove block => create branches in block above level
7. In Top Block less than 4 values => Create branch
8. VarBranch shunt (remove value) => Remove VarBranch + inject ContentCell
9. VarBranch shunt (remove transit key) => Remove VarBranch + inject ContentCell.Value
10. VarBranch continue (remove value) => Remove VarBranch + inject ContentCell
//11. VarBranch continue (remove value) => Remove VarBranch + inject ContentCell

Pools
1. Table of content holes
2. List of free branches
3. List of free blocks
4. List of varbranches

Stats

*/


bool HArray::delValueByKey(uint32* key,
						   uint32 keyLen)
{
	uint32* pValue = getValueByKey(key, keyLen);

	if (pValue)
	{
		*pValue = 0;

		return true;
	}
	else
	{
		return false;
	}
}

//Type
//ContentCell
//BlockCell

const uchar8 CURRENT_VALUE_SEGMENT_TYPE = 1;
const uchar8 BRANCH_SEGMENT_TYPE = CURRENT_VALUE_SEGMENT_TYPE + 1;
const uchar8 BLOCK_VALUE_SEGMENT_TYPE = BRANCH_SEGMENT_TYPE + 1;
const uchar8 BLOCK_BRANCH1_SEGMENT_TYPE = BLOCK_VALUE_SEGMENT_TYPE + 1;
const uchar8 BLOCK_BRANCH2_SEGMENT_TYPE = BLOCK_BRANCH1_SEGMENT_TYPE + 1;
const uchar8 BLOCK_OFFSET_SEGMENT_TYPE = BLOCK_BRANCH2_SEGMENT_TYPE + 1;
const uchar8 VAR_SEGMENT_TYPE = BLOCK_OFFSET_SEGMENT_TYPE + 1;

struct SegmentPath
{
	uchar8 Type;

	ContentCell* pContentCell;
	BlockCell* pBlockCell;
	BranchCell* pBranchCell;
	VarCell* pVarCell;

	uint32 Index;
};

bool HArray::delValueByKey2(uint32* key,
							uint32 keyLen)
{
	SegmentPath path[128];
	uint32 pathLen = 0;

	keyLen >>= 2; //in 4 bytes
	uint32 maxSafeShort = MAX_SAFE_SHORT - keyLen;

	uint32 headerOffset;

	if (!normalizeFunc)
	{
		headerOffset = key[0] >> HeaderBits;
	}
	else
	{
		headerOffset = (*normalizeFunc)(key);
	}

	HeaderCell& headerCell = pHeader[headerOffset];

	if (headerCell.Type)
	{
		uint32 contentOffset = headerCell.Offset;

		uint32 keyOffset = 0;

	NEXT_KEY_PART:
		ContentPage* pContentPage = pContentPages[contentOffset >> 16];
		ushort16 contentIndex = contentOffset & 0xFFFF;

		ContentCell& contentCell = pContentPage->pContent[contentIndex];

		uchar8 contentCellType = contentCell.Type; //move to type part

		if (contentCellType >= ONLY_CONTENT_TYPE) //ONLY CONTENT =========================================================================================
		{
			if ((keyLen - keyOffset) != (contentCellType - ONLY_CONTENT_TYPE))
			{
				return false;
			}

			uint32 origKeyOffset = keyOffset;
				
			if (contentIndex < maxSafeShort) //content in one page
			{
				uint32 origContentIndex = contentIndex;

				for (; keyOffset < keyLen; contentIndex++, keyOffset++)
				{
					if (pContentPage->pContent[contentIndex].Value != key[keyOffset])
						return false;
				}

				//remove
				for (; origKeyOffset <= keyLen; origContentIndex++, origKeyOffset++)
				{
					pContentPage->pContent[origContentIndex].Type = 0;
				}
			}
			else //content in two pages
			{
				uint32 origContentOffset = contentOffset;

				for (; keyOffset < keyLen; contentOffset++, keyOffset++)
				{
					if (pContentPages[contentOffset >> 16]->pContent[contentOffset & 0xFFFF].Value != key[keyOffset])
						return false;
				}

				//remove
				for (; origKeyOffset <= keyLen; origContentOffset++, origKeyOffset++)
				{
					pContentPages[origContentOffset >> 16]->pContent[origContentOffset & 0xFFFF].Type = 0;
				}
			}

			goto DISMANTLING;
		}

		uint32& keyValue = key[keyOffset];
		uint32* pContentCellValueOrOffset = &contentCell.Value;

		if (contentCellType == VAR_TYPE) //VAR =====================================================================
		{
			VarPage* pVarPage = pVarPages[(*pContentCellValueOrOffset) >> 16];
			VarCell& varCell = pVarPage->pVar[(*pContentCellValueOrOffset) & 0xFFFF];

			if (keyOffset < keyLen)
			{
				contentCellType = varCell.ContCell.Type; //read from var cell

				//save path
				SegmentPath& sp = path[pathLen++];
				sp.Type = VAR_SEGMENT_TYPE;
				sp.pContentCell = &contentCell;
				sp.pVarCell = &varCell;
				//sp.pBranchCell = &branchCell1;
				//sp.Index = i;

				if (contentCellType == CONTINUE_VAR_TYPE) //CONTINUE VAR =====================================================================
				{
					contentOffset = varCell.ContCell.Value;

					goto NEXT_KEY_PART;
				}
				else
				{
					pContentCellValueOrOffset = &varCell.ContCell.Value;
				}
			}
			else
			{
				//return &varCell.ValueContentCell.Value;
				goto DISMANTLING;
			}
		}
		else if (keyOffset == keyLen)
		{
			if (contentCellType == VALUE_TYPE)
			{
				//remove
				*pContentCellValueOrOffset = 0;

				goto DISMANTLING;
			}
			else
			{
				return false;
			}
		}

		if (contentCellType <= MAX_BRANCH_TYPE1) //BRANCH =====================================================================
		{
			BranchPage* pBranchPage = pBranchPages[(*pContentCellValueOrOffset) >> 16];
			BranchCell& branchCell = pBranchPage->pBranch[(*pContentCellValueOrOffset) & 0xFFFF];

			//try find value in the list
			uint32* values = branchCell.Values;

			for (uint32 i = 0; i<contentCellType; i++)
			{
				if (values[i] == keyValue)
				{
					contentOffset = branchCell.Offsets[i];
					keyOffset++;

					//save path
					SegmentPath& sp = path[pathLen++];
					sp.Type = BRANCH_SEGMENT_TYPE;
					sp.pContentCell = &contentCell;
					sp.pBranchCell = &branchCell;
					sp.Index = i;

					goto NEXT_KEY_PART;
				}
			}

			return 0;
		}
		else if (contentCellType == VALUE_TYPE)
		{
			if (keyOffset == keyLen)
			{
				//remove
				*pContentCellValueOrOffset = 0;

				goto DISMANTLING;
			}
			else
			{
				return false;
			}
		}
		else if (contentCellType <= MAX_BLOCK_TYPE) //VALUE IN BLOCK ===================================================================
		{
			uchar8 idxKeyValue = (contentCellType - MIN_BLOCK_TYPE) * BLOCK_ENGINE_STEP;

			uint32 startOffset = *pContentCellValueOrOffset;

		NEXT_BLOCK:
			uint32 subOffset = ((keyValue << idxKeyValue) >> BLOCK_ENGINE_SHIFT);
			uint32 blockOffset = startOffset + subOffset;

			BlockPage* pBlockPage = pBlockPages[blockOffset >> 16];
			BlockCell& blockCell = pBlockPage->pBlock[blockOffset & 0xFFFF];

			uchar8& blockCellType = blockCell.Type;

			if (blockCellType == EMPTY_TYPE)
			{
				return false;
			}
			else if (blockCellType == CURRENT_VALUE_TYPE) //current value
			{
				if (blockCell.ValueOrOffset == keyValue) //value is exists
				{
					contentOffset = blockCell.Offset;
					keyOffset++;

					//save path
					SegmentPath& sp = path[pathLen++];
					sp.Type = BLOCK_VALUE_SEGMENT_TYPE;
					sp.pContentCell = &contentCell;
					sp.pBlockCell = &blockCell;
					sp.pBranchCell = 0;
					//sp.Index = i;

					goto NEXT_KEY_PART;
				}
				else
				{
					return false;
				}
			}
			else if (blockCellType <= MAX_BRANCH_TYPE1) //branch cell
			{
				BranchPage* pBranchPage = pBranchPages[blockCell.Offset >> 16];
				BranchCell& branchCell1 = pBranchPage->pBranch[blockCell.Offset & 0xFFFF];

				//try find value in the list
				for (uint32 i = 0; i<blockCellType; i++)
				{
					if (branchCell1.Values[i] == keyValue)
					{
						contentOffset = branchCell1.Offsets[i];
						keyOffset++;

						//save path
						SegmentPath& sp = path[pathLen++];
						sp.Type = BLOCK_BRANCH1_SEGMENT_TYPE;
						sp.pContentCell = &contentCell;
						sp.pBlockCell = &blockCell;
						sp.pBranchCell = &branchCell1;
						sp.Index = i;

						goto NEXT_KEY_PART;
					}
				}

				return 0;
			}
			else if (blockCellType <= MAX_BRANCH_TYPE2) //branch cell
			{
				BranchPage* pBranchPage1 = pBranchPages[blockCell.Offset >> 16];
				BranchCell branchCell1 = pBranchPage1->pBranch[blockCell.Offset & 0xFFFF];

				//try find value in the list
				for (uint32 i = 0; i < BRANCH_ENGINE_SIZE; i++)
				{
					if (branchCell1.Values[i] == keyValue)
					{
						contentOffset = branchCell1.Offsets[i];
						keyOffset++;

						//save path
						SegmentPath& sp = path[pathLen++];
						sp.Type = BLOCK_BRANCH1_SEGMENT_TYPE;
						sp.pContentCell = &contentCell;
						sp.pBlockCell = &blockCell;
						sp.pBranchCell = &branchCell1;
						sp.Index = i;

						goto NEXT_KEY_PART;
					}
				}

				BranchPage* pBranchPage2 = pBranchPages[blockCell.ValueOrOffset >> 16];
				BranchCell branchCell2 = pBranchPage2->pBranch[blockCell.ValueOrOffset & 0xFFFF];

				//try find value in the list
				uint32 countValues = blockCellType - MAX_BRANCH_TYPE1;

				for (uint32 i = 0; i<countValues; i++)
				{
					if (branchCell2.Values[i] == keyValue)
					{
						contentOffset = branchCell2.Offsets[i];
						keyOffset++;

						//save path
						SegmentPath& sp = path[pathLen++];
						sp.Type = BLOCK_BRANCH2_SEGMENT_TYPE;
						sp.pContentCell = &contentCell;
						sp.pBlockCell = &blockCell;
						sp.pBranchCell = &branchCell2;
						sp.Index = i;

						goto NEXT_KEY_PART;
					}
				}

				return 0;
			}
			else if (blockCell.Type <= MAX_BLOCK_TYPE)
			{
				//go to block
				idxKeyValue = (blockCell.Type - MIN_BLOCK_TYPE) * BLOCK_ENGINE_STEP;
				startOffset = blockCell.Offset;

				//save path
				SegmentPath& sp = path[pathLen++];
				sp.Type = BLOCK_OFFSET_SEGMENT_TYPE;
				sp.pContentCell = &contentCell;
				sp.pBlockCell = &blockCell;
				//sp.pBranchCell = &branchCell1;
				//sp.Index = i;

				goto NEXT_BLOCK;
			}
			else
			{
				return false;
			}
		}
		else if (contentCellType == CURRENT_VALUE_TYPE) //PART OF KEY =========================================================================
		{
			if (*pContentCellValueOrOffset == keyValue)
			{
				contentOffset++;
				keyOffset++;

				//save path
				SegmentPath& sp = path[pathLen++];
				sp.Type = CURRENT_VALUE_TYPE;
				sp.pContentCell = &contentCell;
					
				goto NEXT_KEY_PART;
			}
			else
			{
				return false;
			}
		}
	}

	DISMANTLING:;

	return false;
}
