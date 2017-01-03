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

void HArray::releaseContentCells(uint32 contentOffset, uint32 len)
{

}

void HArray::releaseBranchCell(uint32 branchOffset, BranchCell* pBranchCell)
{

}

void HArray::releaseVarCell(uint32 varOffset, VarCell* pVarCell)
{

}

void HArray::releaseBlockCell(uint32 startBlockOffset, BlockCell* pBlockCell)
{

}

void HArray::defragmentContentPages()
{

}

void HArray::defragmentBranchPages()
{

}

void HArray::defragmentBlockPages()
{

}	

void HArray::defragmentVarPages()
{
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
const uchar8 VAR_SHUNT_SEGMENT_TYPE = BLOCK_OFFSET_SEGMENT_TYPE + 1;
const uchar8 VAR_VALUE_SEGMENT_TYPE = VAR_SHUNT_SEGMENT_TYPE + 1;

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
	uint32 ContentOrBlockSubOffset;
};

bool HArray::delValueByKey2(uint32* key,
							uint32 keyLen)
{
	//EXTRACT PATH =============================================================================================

	SegmentPath path[128];
	int32 pathLen = 0;

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
				releaseContentCells(contentOffset, keyLen + 1);

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
				releaseContentCells(origContentOffset, keyLen + 1);

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
				//save path
				SegmentPath& sp = path[pathLen++];
				sp.Type = VAR_SHUNT_SEGMENT_TYPE;
				sp.pContentCell = &contentCell;
				sp.pVarCell = &varCell;
				sp.VarOffset = *pContentCellValueOrOffset;
										
				contentCellType = varCell.ContCell.Type; //read from var cell

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
				//save path
				SegmentPath& sp = path[pathLen++];
				sp.Type = VAR_VALUE_SEGMENT_TYPE;
				sp.pContentCell = &contentCell;
				sp.pVarCell = &varCell;
				sp.VarOffset = *pContentCellValueOrOffset;

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
					sp.pBranchCell1 = &branchCell;
					sp.BranchOffset1 = *pContentCellValueOrOffset;
					sp.BranchIndex = i;
					sp.ContentOrBlockSubOffset = contentOffset;

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
					sp.StartBlockOffset = startOffset;
					sp.ContentOrBlockSubOffset = subOffset;
					//sp.pBranchCell = 0;
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
						sp.StartBlockOffset = startOffset;
						sp.pBranchCell1 = &branchCell1;
						sp.BranchOffset1 = blockCell.Offset;
						sp.BranchIndex = i;
						sp.ContentOrBlockSubOffset = subOffset;

						goto NEXT_KEY_PART;
					}
				}

				return 0;
			}
			else if (blockCellType <= MAX_BRANCH_TYPE2) //branch cell
			{
				BranchPage* pBranchPage1 = pBranchPages[blockCell.Offset >> 16];
				BranchCell branchCell1 = pBranchPage1->pBranch[blockCell.Offset & 0xFFFF];

				BranchPage* pBranchPage2 = pBranchPages[blockCell.ValueOrOffset >> 16];
				BranchCell branchCell2 = pBranchPage2->pBranch[blockCell.ValueOrOffset & 0xFFFF];

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
						sp.StartBlockOffset = startOffset;
						sp.pBranchCell1 = &branchCell1;
						sp.BranchOffset1 = blockCell.Offset;
						sp.pBranchCell2 = &branchCell2;
						sp.BranchOffset2 = blockCell.ValueOrOffset;
						sp.BranchIndex = i;
						sp.ContentOrBlockSubOffset = subOffset;

						goto NEXT_KEY_PART;
					}
				}

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
						sp.StartBlockOffset = startOffset;
						sp.pBranchCell1 = &branchCell1;
						sp.BranchOffset1 = blockCell.Offset;
						sp.pBranchCell2 = &branchCell2;
						sp.BranchOffset2 = blockCell.ValueOrOffset;
						sp.BranchIndex = i;
						sp.ContentOrBlockSubOffset = subOffset;

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
				sp.StartBlockOffset = startOffset;

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
				sp.Type = CURRENT_VALUE_SEGMENT_TYPE;
				sp.pContentCell = &contentCell;
				sp.ContentOrBlockSubOffset = contentOffset;
	
				goto NEXT_KEY_PART;
			}
			else
			{
				return false;
			}
		}
	}

	DISMANTLING:

	//DISMANTLING =============================================================================================
	for(int32 currPathLen = pathLen - 1; currPathLen >= 0; currPathLen--)
	{
		SegmentPath& sp = path[currPathLen];
				
		switch(sp.Type)
		{
			case CURRENT_VALUE_SEGMENT_TYPE:
			{
				uint32 contentOffsetLen = 1;

				while(true)
				{
					//just remove
					path[currPathLen].pContentCell->Type = 0;
					path[currPathLen].pContentCell->Value = 0;

					if(currPathLen > 0)
					{
						if(path[currPathLen - 1].Type == CURRENT_VALUE_SEGMENT_TYPE)
						{
							currPathLen--;
							contentOffsetLen++;
						}
						else
						{
							releaseContentCells(sp.ContentOrBlockSubOffset - contentOffsetLen + 1, contentOffsetLen);

							break;
						}
					}
					else
					{
						releaseContentCells(sp.ContentOrBlockSubOffset - contentOffsetLen + 1, contentOffsetLen);

						goto DEFRAGMENT;
					}
				}

				break;
			}
		
			case BRANCH_SEGMENT_TYPE:
			{
				if(sp.pContentCell->Type > MIN_BRANCH_TYPE1) //not last way
				{
					//remove item
					uint32 lastIndex = sp.pContentCell->Type - 1;

					sp.pBranchCell1->Values[sp.BranchIndex] = sp.pBranchCell1->Values[lastIndex];
					sp.pBranchCell1->Offsets[sp.BranchIndex] = sp.pBranchCell1->Offsets[lastIndex];

					sp.pContentCell->Type--;

					if(sp.pContentCell->Type == MIN_BRANCH_TYPE1) //last way is original ?
					{
						if(sp.pBranchCell1->Offsets[0] == sp.ContentOrBlockSubOffset + 1)
						{
							sp.pContentCell->Type = CURRENT_VALUE_TYPE;
							sp.pContentCell->Value = sp.pBranchCell1->Values[0];

							releaseBranchCell(sp.BranchOffset1, sp.pBranchCell1);		
						}
					}

					goto DEFRAGMENT;
				}
				else //last way
				{
					releaseBranchCell(sp.BranchOffset1, sp.pBranchCell1);

					break;	
				}
			}

			case BLOCK_VALUE_SEGMENT_TYPE:
			{
				sp.pBlockCell->Type = 0;
				sp.pBlockCell->Offset = 0;
				sp.pBlockCell->ValueOrOffset = 0;

				//check block
				uint32 subOffset = 0;
				BlockCell* pCurrBlockCell = sp.pBlockCell - sp.ContentOrBlockSubOffset;

				uint32 count = 0;

				for(; subOffset < BLOCK_ENGINE_SIZE; subOffset++, pCurrBlockCell++)
				{
					if(pCurrBlockCell->Type == CURRENT_VALUE_TYPE)
					{
						count++;
					}
					else
					{
						count += pCurrBlockCell->Type;
					}

					if(count > BRANCH_ENGINE_SIZE * 2)
					{
						break;
					}
				}

				if(count <= BRANCH_ENGINE_SIZE * 2)
				{
					if(count > 0)
					{
						if(path[currPathLen - 1].Type == BLOCK_OFFSET_SEGMENT_TYPE)
						{
							
						}
						else
						{
							
						}
					}
					else //only one value in block
					{
						if(path[currPathLen - 1].Type == BLOCK_OFFSET_SEGMENT_TYPE)
						{
							
						}
						else //inject to content last value from block, release block
						{

						}
					}
				}

				break;
			}

			case BLOCK_BRANCH1_SEGMENT_TYPE:
			{
				//remove item
				if(sp.pBlockCell->Type <= MAX_BRANCH_TYPE1) //in one branch
				{
					uint32 lastIndex = sp.pBlockCell->Type - 1;

					sp.pBranchCell1->Values[sp.BranchIndex] = sp.pBranchCell1->Values[lastIndex];
					sp.pBranchCell1->Offsets[sp.BranchIndex] = sp.pBranchCell1->Offsets[lastIndex];

					sp.pBlockCell->Type--;

					if(sp.pBlockCell->Type == MIN_BRANCH_TYPE1) //only one item, inject to block
					{
						sp.pBlockCell->Type = CURRENT_VALUE_TYPE;
						sp.pBlockCell->ValueOrOffset = sp.pBranchCell1->Values[0];

						releaseBranchCell(sp.BranchOffset1, sp.pBranchCell1);
					}

					//check block ?
				}
				else //in two branches
				{
					uint32 lastIndex = sp.pBlockCell->Type - MAX_BRANCH_TYPE1 - 1;

					sp.pBranchCell1->Values[sp.BranchIndex] = sp.pBranchCell2->Values[lastIndex];
					sp.pBranchCell1->Offsets[sp.BranchIndex] = sp.pBranchCell2->Offsets[lastIndex];

					sp.pBlockCell->Type--;

					//if it was last item in branch2 then release branch2
					if(sp.pBlockCell->Type < MIN_BRANCH_TYPE2) //not last item in branch2
					{
						releaseBranchCell(sp.BranchOffset2, sp.pBranchCell2);
					}
				}

				goto DEFRAGMENT;
			}

			case BLOCK_BRANCH2_SEGMENT_TYPE:
			{
				if(sp.pBlockCell->Type > MIN_BRANCH_TYPE2) //not last item
				{
					uint32 lastIndex = sp.pBlockCell->Type - MAX_BRANCH_TYPE1 - 1;

					sp.pBranchCell2->Values[sp.BranchIndex] = sp.pBranchCell2->Values[lastIndex];
					sp.pBranchCell2->Offsets[sp.BranchIndex] = sp.pBranchCell2->Offsets[lastIndex];
				}
				else //last item
				{
					releaseBranchCell(sp.BranchOffset2, sp.pBranchCell2);
				}

				sp.pBlockCell->Type--;

				goto DEFRAGMENT;
			}

			case BLOCK_OFFSET_SEGMENT_TYPE:
			{
				goto DEFRAGMENT;
			}

			case VAR_SHUNT_SEGMENT_TYPE:
			{
				if(sp.pVarCell->ValueContCell.Type)
				{
					//delete continue way, inject value
					*sp.pContentCell = sp.pVarCell->ValueContCell;

					releaseVarCell(sp.VarOffset, sp.pVarCell);

					goto DEFRAGMENT;
				}
				else
				{
					//delete continue way, value was deleted earlier

					releaseVarCell(sp.VarOffset, sp.pVarCell);

					break;			
				}
			}

			case VAR_VALUE_SEGMENT_TYPE:
			{
				if(sp.pVarCell->ContCell.Type == CONTINUE_VAR_TYPE)
				{
					if(sp.pVarCell->ValueContCell.Type)
					{
						sp.pVarCell->ValueContCell.Type = 0;

						goto DEFRAGMENT;
					}
					else
					{
						return false;
					}
				}
				else
				{
					//was only shunted value, remove var cell
					*sp.pContentCell = sp.pVarCell->ContCell;

					releaseVarCell(sp.VarOffset, sp.pVarCell);

					goto DEFRAGMENT;
				}
			}

			default: //fail state
				break;
		}
	}

	//RESET HEADER ====================================================================
	headerCell.Type = 0;
	headerCell.Offset = 0;

DEFRAGMENT:

//DEFRAGMENT =============================================================================================
	
	defragmentContentPages();
	defragmentBranchPages();
	defragmentBlockPages();	
	defragmentVarPages();

	return true;
}
