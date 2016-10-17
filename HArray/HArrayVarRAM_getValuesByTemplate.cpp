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

#include "StdAfx.h"
#include "HArrayVarRAM.h"

void HArrayVarRAM::scanKeysAndValuesFromBlock(uint* key,
											  uint contentOffset,
											  uint keyOffset,
											  uint blockOffset,
											  HARRAY_ITEM_VISIT_FUNC visitor)
{
	//printf("getValuesByRangeFromBlock count=%d size=%d contentOffset=%d keyOffset=%d blockOffset=%d\n", count, size, contentOffset, keyOffset, blockOffset);

	uint maxOffset = blockOffset + BLOCK_ENGINE_SIZE;

	for(uint offset = blockOffset; offset < maxOffset; offset++)
	{
		BlockPage* pBlockPage = pBlockPages[offset >> 16];
		BlockCell& blockCell = pBlockPage->pBlock[offset & 0xFFFF];

		uchar& blockCellType = blockCell.Type;

		if(blockCellType == EMPTY_TYPE)
		{
			continue;
		}
		else if(blockCellType == CURRENT_VALUE_TYPE) //current value
		{
			uint& keyValue = blockCell.ValueOrOffset;

			key[keyOffset] = keyValue;

			scanKeysAndValues(key, keyOffset + 1,  blockCell.Offset, visitor);
		}
		else if(blockCellType <= MAX_BRANCH_TYPE1) //branch cell
		{
			BranchPage* pBranchPage = pBranchPages[blockCell.Offset >> 16];
			BranchCell& branchCell1 = pBranchPage->pBranch[blockCell.Offset & 0xFFFF];

			//try find value in the list
			for(uint i=0; i<blockCellType; i++)
			{
				uint& keyValue = branchCell1.Values[i];

				key[keyOffset] = keyValue;

				scanKeysAndValues(key, keyOffset + 1, branchCell1.Offsets[i], visitor);
			}
		}
		else if(blockCellType <= MAX_BRANCH_TYPE2) //branch cell
		{
			BranchPage* pBranchPage1 = pBranchPages[blockCell.Offset >> 16];
			BranchCell branchCell1 = pBranchPage1->pBranch[blockCell.Offset & 0xFFFF];

			//try find value in the list
			for(uint i=0; i < BRANCH_ENGINE_SIZE; i++)
			{
				uint& keyValue = branchCell1.Values[i];

				key[keyOffset] = keyValue;

				scanKeysAndValues(key, keyOffset + 1, branchCell1.Offsets[i], visitor);
			}

			BranchPage* pBranchPage2 = pBranchPages[blockCell.ValueOrOffset >> 16];
			BranchCell branchCell2 = pBranchPage2->pBranch[blockCell.ValueOrOffset & 0xFFFF];

			//try find value in the list
			uint countValues = blockCellType - MAX_BRANCH_TYPE1;

			for(uint i=0; i<countValues; i++)
			{
				uint& keyValue = branchCell2.Values[i];

				key[keyOffset] = keyValue;

				scanKeysAndValues(key, keyOffset + 1, branchCell2.Offsets[i], visitor);
			}
		}
		else if(blockCell.Type <= MAX_BLOCK_TYPE)
		{
			//go to block
			scanKeysAndValuesFromBlock(key,
									   contentOffset,
									   keyOffset,
									   blockCell.Offset,
									   visitor);
		}
	}
}

void HArrayVarRAM::scanKeysAndValues(uint* key,
									 uint keyOffset, 
									 uint contentOffset,
									 HARRAY_ITEM_VISIT_FUNC visitor)
{
	//printf("getValuesByRange count=%d size=%d contentOffset=%d keyOffset=%d\n", count, size, contentOffset, keyOffset);

	for(;; keyOffset++, contentOffset++)
	{

NEXT_KEY_PART:

		ContentPage* pContentPage = pContentPages[contentOffset>>16];
		ushort contentIndex = contentOffset&0xFFFF;
		
		uint contentCellValueOrOffset = pContentPage->pContent[contentIndex].Value;
		uchar contentCellType = pContentPage->pContent[contentIndex].Type; //move to type part

		if(contentCellType >= ONLY_CONTENT_TYPE) //ONLY CONTENT =========================================================================================
		{
			uint keyLen =  contentCellType - ONLY_CONTENT_TYPE;
	
			for(uint i = 0; i < keyLen; i++, keyOffset++, contentOffset++)
			{
				uint& keyValue = pContentPages[contentOffset>>16]->pContent[contentOffset&0xFFFF].Value;
					
				key[keyOffset] = keyValue;
			}

			//contentOffset += (keyLen - keyOffset);

			(*visitor)(key, keyOffset, pContentPages[contentOffset>>16]->pContent[contentOffset&0xFFFF].Value);

			return;
		}

		if(contentCellType == VAR_TYPE) //VAR =====================================================================
		{
			VarPage* pVarPage = pVarPages[contentCellValueOrOffset >> 16];
			VarCell& varCell = pVarPage->pVar[contentCellValueOrOffset & 0xFFFF];

			//save value
			//(*visitor)(key, keyOffset, varCell.Value);
			
			contentCellType = varCell.ContentCell.Type; //read from var cell
			contentCellValueOrOffset = varCell.ContentCell.Value;

			if(contentCellType == CONTINUE_VAR_TYPE) //CONTINUE VAR =====================================================================
			{
				contentOffset = contentCellValueOrOffset;

				//goto
				scanKeysAndValues(key, keyOffset, contentOffset, visitor);

				return;
			}
		}
		
		if(contentCellType <= MAX_BRANCH_TYPE1) //BRANCH =====================================================================
		{
			BranchPage* pBranchPage = pBranchPages[contentCellValueOrOffset >> 16];
			BranchCell& branchCell = pBranchPage->pBranch[contentCellValueOrOffset & 0xFFFF];

			//check other
			for(uint i = 0; i<contentCellType; i++) //from 1
			{
				uint& keyValue = branchCell.Values[i];

				key[keyOffset] = keyValue;

				scanKeysAndValues(key, keyOffset + 1, branchCell.Offsets[i], visitor);
			}

			return;
		}
		else if(contentCellType == VALUE_TYPE)
		{
			(*visitor)(key, keyOffset, contentCellValueOrOffset);

			return;
		}
		else if(contentCellType <= MAX_BLOCK_TYPE) //VALUE IN BLOCK ===================================================================
		{
			scanKeysAndValuesFromBlock(	key,
										contentOffset,
										keyOffset,
										contentCellValueOrOffset,
										visitor );

			return;
		}
		else if(contentCellType == CURRENT_VALUE_TYPE)
		{
			uint& keyValue = contentCellValueOrOffset;
			
			key[keyOffset] = keyValue;
		}
	}
}

uint HArrayVarRAM::scanKeysAndValues(uint* key, 
									 uint keyLen,
									 HARRAY_ITEM_VISIT_FUNC visitor)
{
	keyLen >>= 2; //in 4 bytes
	uint maxSafeShort = MAX_SAFE_SHORT - keyLen;

	uint contentOffset = pHeader[key[0]>>HeaderBits];
	
	if(contentOffset)
	{
		uint keyOffset = 0;
	
NEXT_KEY_PART:
		ContentPage* pContentPage = pContentPages[contentOffset>>16];
		ushort contentIndex = contentOffset&0xFFFF;

		uchar contentCellType = pContentPage->pContent[contentIndex].Type; //move to type part
	
		if(contentCellType >= ONLY_CONTENT_TYPE) //ONLY CONTENT =========================================================================================
		{
			if(contentIndex < maxSafeShort) //content in one page
			{
				for(; keyOffset < keyLen; contentIndex++, keyOffset++)
				{
					if(pContentPage->pContent[contentIndex].Value != key[keyOffset])
						return 0;
				}

				keyLen = contentCellType - ONLY_CONTENT_TYPE;
			
				for(; keyOffset < keyLen; contentIndex++, keyOffset++)
				{
					key[keyOffset] = pContentPage->pContent[contentIndex].Value;
				}

				(*visitor)(key, keyOffset, pContentPage->pContent[contentIndex].Value); //return value

				return 0;
			}
			else //content in two pages
			{
				for(; keyOffset < keyLen; contentOffset++, keyOffset++)
				{
					if(pContentPages[contentOffset>>16]->pContent[contentOffset&0xFFFF].Value != key[keyOffset])
						return 0;
				}

				keyLen = contentCellType - ONLY_CONTENT_TYPE;
			
				for(; keyOffset < keyLen; contentIndex++, keyOffset++)
				{
					key[keyOffset] = pContentPage->pContent[contentIndex].Value;
				}

				(*visitor)(key, keyOffset, pContentPages[contentOffset>>16]->pContent[contentOffset&0xFFFF].Value);

				return 0;
			}
		}

		uint& keyValue = key[keyOffset];
		uint contentCellValueOrOffset = pContentPage->pContent[contentIndex].Value;
	
		if(contentCellType == VAR_TYPE) //VAR =====================================================================
		{
			VarPage* pVarPage = pVarPages[contentCellValueOrOffset >> 16];
			VarCell& varCell = pVarPage->pVar[contentCellValueOrOffset & 0xFFFF];

			if(keyOffset < keyLen)
			{
				contentCellType = varCell.ContentCell.Type; //read from var cell
				contentCellValueOrOffset = varCell.ContentCell.Value;

				if(contentCellType == CONTINUE_VAR_TYPE) //CONTINUE VAR =====================================================================
				{
					contentOffset = contentCellValueOrOffset;

					goto NEXT_KEY_PART;
				}
			}
			else
			{
				scanKeysAndValues(key, keyOffset, contentOffset, visitor);

				//return varCell.Value;

				return 0;
			}
		}
		else if(keyOffset == keyLen)
		{
			if(contentCellType == VALUE_TYPE)
			{
				return contentCellValueOrOffset;
			}
			else
			{
				scanKeysAndValues(key, keyOffset, contentOffset, visitor);

				return 0;
			}
		}
		
		if(contentCellType <= MAX_BRANCH_TYPE1) //BRANCH =====================================================================
		{
			BranchPage* pBranchPage = pBranchPages[contentCellValueOrOffset >> 16];
			BranchCell& branchCell = pBranchPage->pBranch[contentCellValueOrOffset & 0xFFFF];

			//try find value in the list
			uint* values = branchCell.Values;

			for(uint i=0; i<contentCellType; i++)
			{
				if(values[i] == keyValue)
				{
					contentOffset = branchCell.Offsets[i];
					keyOffset++;

					goto NEXT_KEY_PART;
				}
			}

			return 0;
		}
		else if(contentCellType == VALUE_TYPE)
		{
			return contentCellValueOrOffset;
		}
		else if(contentCellType <= MAX_BLOCK_TYPE) //VALUE IN BLOCK ===================================================================
		{
			uchar idxKeyValue = (contentCellType - MIN_BLOCK_TYPE) * BLOCK_ENGINE_STEP;

			uint startOffset = contentCellValueOrOffset;

	NEXT_BLOCK:
			uint subOffset = ((keyValue << idxKeyValue) >> BLOCK_ENGINE_SHIFT);
			uint blockOffset = startOffset + subOffset;
		
			BlockPage* pBlockPage = pBlockPages[blockOffset >> 16];
			BlockCell& blockCell = pBlockPage->pBlock[blockOffset & 0xFFFF];

			uchar& blockCellType = blockCell.Type;

			if(blockCellType == EMPTY_TYPE)
			{
				return 0;
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
					return 0;
				}
			}
			else if(blockCellType <= MAX_BRANCH_TYPE1) //branch cell
			{
				BranchPage* pBranchPage = pBranchPages[blockCell.Offset >> 16];
				BranchCell& branchCell1 = pBranchPage->pBranch[blockCell.Offset & 0xFFFF];

				//try find value in the list
				for(uint i=0; i<blockCellType; i++)
				{
					if(branchCell1.Values[i] == keyValue)
					{
						contentOffset = branchCell1.Offsets[i];
						keyOffset++;

						goto NEXT_KEY_PART;
					}
				}

				return 0;
			}
			else if(blockCellType <= MAX_BRANCH_TYPE2) //branch cell
			{
				BranchPage* pBranchPage1 = pBranchPages[blockCell.Offset >> 16];
				BranchCell branchCell1 = pBranchPage1->pBranch[blockCell.Offset & 0xFFFF];

				//try find value in the list
				for(uint i=0; i < BRANCH_ENGINE_SIZE; i++)
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
				uint countValues = blockCellType - MAX_BRANCH_TYPE1;

				for(uint i=0; i<countValues; i++)
				{
					if(branchCell2.Values[i] == keyValue)
					{
						contentOffset = branchCell2.Offsets[i];
						keyOffset++;

						goto NEXT_KEY_PART;
					}
				}

				return 0;
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
				return 0;
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
				return 0;
			}
		}
	}
	
	return 0;
}