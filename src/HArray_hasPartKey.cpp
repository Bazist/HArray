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

#include "stdafx.h"
#include "HArray.h"

bool HArray::hasPartKey(uint32_t* key, uint32_t keyLen)
{
	uint32_t maxSafeShort = MAX_SAFE_SHORT - keyLen;

	uint32_t headerOffset;

	if (!normalizeFunc)
	{
		headerOffset = key[0] >> HeaderBits;
	}
	else
	{
		headerOffset = (*normalizeFunc)(key);
	}

	uint32_t contentOffset = pHeader[headerOffset];

	if(contentOffset)
	{
		uint32_t keyOffset = 0;

NEXT_KEY_PART:
		ContentPage* pContentPage = pContentPages[contentOffset>>16];
		uint16_t contentIndex = contentOffset&0xFFFF;

		uint8_t contentCellType = pContentPage->pType[contentIndex]; //move to type part

		if(contentCellType >= ONLY_CONTENT_TYPE) //ONLY CONTENT =========================================================================================
		{
			if(contentIndex < maxSafeShort) //content in one page
			{
				for(; keyOffset < keyLen; contentIndex++, keyOffset++)
				{
					if(pContentPage->pContent[contentIndex] != key[keyOffset])
						return false;
				}

				return true;
			}
			else //content in two pages
			{
				for(; keyOffset < keyLen; contentOffset++, keyOffset++)
				{
					if(pContentPages[contentOffset>>16]->pContent[contentOffset&0xFFFF] != key[keyOffset])
						return false;
				}

				return true;
			}
		}

		uint32_t& keyValue = key[keyOffset];
		uint32_t contentCellValueOrOffset = pContentPage->pContent[contentIndex];

		if(contentCellType == VAR_TYPE) //VAR =====================================================================
		{
			VarPage* pVarPage = pVarPages[contentCellValueOrOffset >> 16];
			VarCell& varCell = pVarPage->pVar[contentCellValueOrOffset & 0xFFFF];

			if(keyOffset < keyLen)
			{
				contentCellType = varCell.ContCellType; //read from var cell
				contentCellValueOrOffset = varCell.ContCellValue;

				if(contentCellType == CONTINUE_VAR_TYPE) //CONTINUE VAR =====================================================================
				{
					contentOffset = contentCellValueOrOffset;

					goto NEXT_KEY_PART;
				}
			}
			else
			{
				return true;
			}
		}
		else if(keyOffset == keyLen)
		{
			return true;
		}

		if(contentCellType <= MAX_BRANCH_TYPE1) //BRANCH =====================================================================
		{
			BranchPage* pBranchPage = pBranchPages[contentCellValueOrOffset >> 16];
			BranchCell& branchCell = pBranchPage->pBranch[contentCellValueOrOffset & 0xFFFF];

			//try find value in the list
			uint32_t* values = branchCell.Values;

			for(uint32_t i=0; i<contentCellType; i++)
			{
				if(values[i] == keyValue)
				{
					contentOffset = branchCell.Offsets[i];
					keyOffset++;

					goto NEXT_KEY_PART;
				}
			}

			return false;
		}
		else if(contentCellType == VALUE_TYPE)
		{
			return true;
		}
		else if(contentCellType <= MAX_BLOCK_TYPE) //VALUE IN BLOCK ===================================================================
		{
			uint8_t idxKeyValue = (contentCellType - MIN_BLOCK_TYPE) * BLOCK_ENGINE_STEP;

			uint32_t startOffset = contentCellValueOrOffset;

	NEXT_BLOCK:
			uint32_t subOffset = ((keyValue << idxKeyValue) >> BLOCK_ENGINE_SHIFT);
			uint32_t blockOffset = startOffset + subOffset;

			BlockPage* pBlockPage = pBlockPages[blockOffset >> 16];
			BlockCell& blockCell = pBlockPage->pBlock[blockOffset & 0xFFFF];

			uint8_t& blockCellType = blockCell.Type;

			if(blockCellType == EMPTY_TYPE)
			{
				return false;
			}
			else if(blockCellType == CURRENT_VALUE_TYPE) //current value
			{
				if(blockCell.ValueOrOffset == keyValue) //value is exists
				{
					contentOffset = blockCell.Offset;
					keyOffset++;

					goto NEXT_KEY_PART;
				}
				else
				{
					return false;
				}
			}
			else if(blockCellType <= MAX_BRANCH_TYPE1) //branch cell
			{
				BranchPage* pBranchPage = pBranchPages[blockCell.Offset >> 16];
				BranchCell& branchCell1 = pBranchPage->pBranch[blockCell.Offset & 0xFFFF];

				//try find value in the list
				for(uint32_t i=0; i<blockCellType; i++)
				{
					if(branchCell1.Values[i] == keyValue)
					{
						contentOffset = branchCell1.Offsets[i];
						keyOffset++;

						goto NEXT_KEY_PART;
					}
				}

				return false;
			}
			else if(blockCellType <= MAX_BRANCH_TYPE2) //branch cell
			{
				BranchPage* pBranchPage1 = pBranchPages[blockCell.Offset >> 16];
				BranchCell branchCell1 = pBranchPage1->pBranch[blockCell.Offset & 0xFFFF];

				//try find value in the list
				for(uint32_t i=0; i < BRANCH_ENGINE_SIZE; i++)
				{
					if(branchCell1.Values[i] == keyValue)
					{
						contentOffset = branchCell1.Offsets[i];
						keyOffset++;

						goto NEXT_KEY_PART;
					}
				}

				BranchPage* pBranchPage2 = pBranchPages[blockCell.ValueOrOffset >> 16];
				BranchCell branchCell2 = pBranchPage2->pBranch[blockCell.ValueOrOffset & 0xFFFF];

				//try find value in the list
				uint32_t countValues = blockCellType - MAX_BRANCH_TYPE1;

				for(uint32_t i=0; i<countValues; i++)
				{
					if(branchCell2.Values[i] == keyValue)
					{
						contentOffset = branchCell2.Offsets[i];
						keyOffset++;

						goto NEXT_KEY_PART;
					}
				}

				return false;
			}
			else if(blockCell.Type <= MAX_BLOCK_TYPE)
			{
				//go to block
				idxKeyValue = (blockCell.Type - MIN_BLOCK_TYPE) * BLOCK_ENGINE_STEP;
				startOffset = blockCell.Offset;

				goto NEXT_BLOCK;
			}
			else
			{
				return false;
			}
		}
		else if(contentCellType == CURRENT_VALUE_TYPE) //PART OF KEY =========================================================================
		{
			if(contentCellValueOrOffset == keyValue)
			{
				contentOffset++;
				keyOffset++;

				goto NEXT_KEY_PART;
			}
			else
			{
				return false;
			}
		}
	}

	return false;
}
