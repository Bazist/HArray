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

bool HArray::getValueByKey(const char* key,
						   uint32 keyLen,
						   uint32& value)
{
	uint32 lastSegmentKeyLen = keyLen & 0x3;

	if (!lastSegmentKeyLen) //key is aligned by 4 bytes, just pass as is
	{
		return getValueByKey((uint32*)key, keyLen, value);
	}
	else
	{
		uint32* keyInSegments = (uint32*)key;
		uint32 keyLenInSegments = keyLen >> 2;

		uint32 newKey[MAX_KEY_SEGMENTS];

		uint32 i = 0;

		for (; i < keyLenInSegments; i++)
		{
			newKey[i] = keyInSegments[i];
		}

		newKey[i] = 0;

		char* lastSegmentNewKey = (char*)&newKey[i];
		char* lastSegmentKey = (char*)&keyInSegments[i];

		for (uint32 j = 0; j < lastSegmentKeyLen; j++)
		{
			lastSegmentNewKey[j] = lastSegmentKey[j];
		}

		return getValueByKey((uint32*)newKey, (keyLen | 0x3) + 1, value);
	}
}

bool HArray::getValueByKey(uint32* key,
	 					   uint32 keyLen,
						   uint32& value)
{
	value = 0;

	keyLen >>= 2; //in 4 bytes

	uint32 headerOffset;

	if (!normalizeFunc)
	{
		headerOffset = key[0] >> HeaderBits;
	}
	else
	{
		headerOffset = (*normalizeFunc)(key);
	}

	uint32 contentOffset = pHeader[headerOffset];

	if(contentOffset)
	{
		uint32 keyOffset = 0;

NEXT_KEY_PART:
		ContentPage* pContentPage = pContentPages[contentOffset>>16];
		ushort16 contentIndex = contentOffset & 0xFFFF;

		uchar8 contentCellType = pContentPage->pType[contentIndex]; //move to type part

		if(contentCellType >= ONLY_CONTENT_TYPE) //ONLY CONTENT =========================================================================================
		{
			if((keyLen - keyOffset) != (uint32)(contentCellType - ONLY_CONTENT_TYPE))
			{
				return false;
			}

			for(; keyOffset < keyLen; contentIndex++, keyOffset++)
			{
				if (pContentPage->pContent[contentIndex] != key[keyOffset])
				{
					return false;
				}
			}

			value = pContentPage->pContent[contentIndex]; //return value

			return true;
		}

		uint32& keyValue = key[keyOffset];
		uint32* pContentCellValueOrOffset = &pContentPage->pContent[contentIndex];

		if(contentCellType == VAR_TYPE) //VAR =====================================================================
		{
			VarPage* pVarPage = pVarPages[(*pContentCellValueOrOffset) >> 16];
			VarCell& varCell = pVarPage->pVar[(*pContentCellValueOrOffset) & 0xFFFF];

			if(keyOffset < keyLen)
			{
				contentCellType = varCell.ContCellType; //read from var cell

				if(contentCellType == CONTINUE_VAR_TYPE) //CONTINUE VAR =====================================================================
				{
					contentOffset = varCell.ContCellValue;

					goto NEXT_KEY_PART;
				}
				else
				{
					pContentCellValueOrOffset = &varCell.ContCellValue;
				}
			}
			else
			{
				if(varCell.ValueContCellType)
                {
                	value = varCell.ValueContCellValue;

					return true;
                }
                else
                {
					return false;
                }
			}
		}
		else if(keyOffset == keyLen)
		{
			if(contentCellType == VALUE_TYPE)
			{
                return pContentCellValueOrOffset;
			}
			else
			{
				return false;
			}
		}

		if(contentCellType <= MAX_BRANCH_TYPE1) //BRANCH =====================================================================
		{
			BranchPage* pBranchPage = pBranchPages[(*pContentCellValueOrOffset) >> 16];
			BranchCell& branchCell = pBranchPage->pBranch[(*pContentCellValueOrOffset) & 0xFFFF];

			//try find value in the list
			uint32* values = branchCell.Values;

			for(uint32 i=0; i<contentCellType; i++)
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
			if(keyOffset == keyLen)
			{
                return pContentCellValueOrOffset;
			}
			else
			{
				return false;
			}
		}
		else if(contentCellType <= MAX_BLOCK_TYPE) //VALUE IN BLOCK ===================================================================
		{
			uchar8 idxKeyValue = (contentCellType - MIN_BLOCK_TYPE) * BLOCK_ENGINE_STEP;

			uint32 startOffset = *pContentCellValueOrOffset;

	NEXT_BLOCK:
			uint32 subOffset = ((keyValue << idxKeyValue) >> BLOCK_ENGINE_SHIFT);
			uint32 blockOffset = startOffset + subOffset;

			BlockPage* pBlockPage = pBlockPages[blockOffset >> 16];
			BlockCell& blockCell = pBlockPage->pBlock[blockOffset & 0xFFFF];

			uchar8& blockCellType = blockCell.Type;

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
				BranchCell& branchCell1 = pBranchPages[blockCell.Offset >> 16]->pBranch[blockCell.Offset & 0xFFFF];

				//try find value in the list
				for(uint32 i=0; i<blockCellType; i++)
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
				BranchCell& branchCell1 = pBranchPages[blockCell.Offset >> 16]->pBranch[blockCell.Offset & 0xFFFF];

				//try find value in the list
				for(uint32 i=0; i < BRANCH_ENGINE_SIZE; i++)
				{
					if(branchCell1.Values[i] == keyValue)
					{
						contentOffset = branchCell1.Offsets[i];
						keyOffset++;

						goto NEXT_KEY_PART;
					}
				}

				BranchCell& branchCell2 = pBranchPages[blockCell.ValueOrOffset >> 16]->pBranch[blockCell.ValueOrOffset & 0xFFFF];

				//try find value in the list
				uint32 countValues = blockCellType - MAX_BRANCH_TYPE1;

				for(uint32 i=0; i<countValues; i++)
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
			if(*pContentCellValueOrOffset == keyValue)
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
