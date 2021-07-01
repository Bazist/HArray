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

//SCAN BY VISITOR ======================================================================================================================
void HArray::scanKeysAndValuesFromBlock(int32_t* key,
											  int32_t contentOffset,
											  int32_t keyOffset,
											  int32_t blockOffset,
											  HARRAY_ITEM_VISIT_FUNC visitor,
											  void* pData)
{
	//printf("getValuesByRangeFromBlock count=%d size=%d contentOffset=%d keyOffset=%d blockOffset=%d\n", count, size, contentOffset, keyOffset, blockOffset);

	int32_t maxOffset = blockOffset + BLOCK_ENGINE_SIZE;

	for(int32_t offset = blockOffset; offset < maxOffset; offset++)
	{
		BlockPage* pBlockPage = pBlockPages[offset >> 16];
		BlockCell& blockCell = pBlockPage->pBlock[offset & 0xFFFF];

		uint8_t& blockCellType = blockCell.Type;

		if(blockCellType == EMPTY_TYPE)
		{
			continue;
		}
		else if(blockCellType == CURRENT_VALUE_TYPE) //current value
		{
			int32_t& keyValue = blockCell.ValueOrOffset;

			key[keyOffset] = keyValue;

			scanKeysAndValues(key, keyOffset + 1,  blockCell.Offset, visitor, pData);
		}
		else if(blockCellType <= MAX_BRANCH_TYPE1) //branch cell
		{
			BranchPage* pBranchPage = pBranchPages[blockCell.Offset >> 16];
			BranchCell& branchCell1 = pBranchPage->pBranch[blockCell.Offset & 0xFFFF];

			//try find value in the list
			for(int32_t i=0; i<blockCellType; i++)
			{
				int32_t& keyValue = branchCell1.Values[i];

				key[keyOffset] = keyValue;

				scanKeysAndValues(key, keyOffset + 1, branchCell1.Offsets[i], visitor, pData);
			}
		}
		else if(blockCellType <= MAX_BRANCH_TYPE2) //branch cell
		{
			BranchPage* pBranchPage1 = pBranchPages[blockCell.Offset >> 16];
			BranchCell branchCell1 = pBranchPage1->pBranch[blockCell.Offset & 0xFFFF];

			//try find value in the list
			for(int32_t i=0; i < BRANCH_ENGINE_SIZE; i++)
			{
				int32_t& keyValue = branchCell1.Values[i];

				key[keyOffset] = keyValue;

				scanKeysAndValues(key, keyOffset + 1, branchCell1.Offsets[i], visitor, pData);
			}

			BranchPage* pBranchPage2 = pBranchPages[blockCell.ValueOrOffset >> 16];
			BranchCell branchCell2 = pBranchPage2->pBranch[blockCell.ValueOrOffset & 0xFFFF];

			//try find value in the list
			int32_t countValues = blockCellType - MAX_BRANCH_TYPE1;

			for(int32_t i=0; i<countValues; i++)
			{
				int32_t& keyValue = branchCell2.Values[i];

				key[keyOffset] = keyValue;

				scanKeysAndValues(key, keyOffset + 1, branchCell2.Offsets[i], visitor, pData);
			}
		}
		else if(blockCell.Type <= MAX_BLOCK_TYPE)
		{
			//go to block
			scanKeysAndValuesFromBlock(key,
									   contentOffset,
									   keyOffset,
									   blockCell.Offset,
									   visitor,
									   pData);
		}
	}
}

void HArray::scanKeysAndValues(int32_t* key,
									 int32_t keyOffset,
									 int32_t contentOffset,
									 HARRAY_ITEM_VISIT_FUNC visitor,
									 void* pData)
{
	//printf("getValuesByRange count=%d size=%d contentOffset=%d keyOffset=%d\n", count, size, contentOffset, keyOffset);

	for(;; keyOffset++, contentOffset++)
	{
		ContentPage* pContentPage = pContentPages[contentOffset>>16];
		uint16_t contentIndex = contentOffset&0xFFFF;

		int32_t contentCellValueOrOffset = pContentPage->pContent[contentIndex];
		uint8_t contentCellType = pContentPage->pType[contentIndex]; //move to type part

		if(contentCellType >= ONLY_CONTENT_TYPE) //ONLY CONTENT =========================================================================================
		{
			int32_t keyLen =  contentCellType - ONLY_CONTENT_TYPE;

			for(int32_t i = 0; i < keyLen; i++, keyOffset++, contentOffset++)
			{
				int32_t& keyValue = pContentPages[contentOffset>>16]->pContent[contentOffset&0xFFFF];

				key[keyOffset] = keyValue;
			}

			//contentOffset += (keyLen - keyOffset);

			(*visitor)(key,
					   keyOffset,
					   pContentPages[contentOffset>>16]->pContent[contentOffset&0xFFFF],
					   pData);

			return;
		}

		if(contentCellType == VAR_TYPE) //VAR =====================================================================
		{
			VarPage* pVarPage = pVarPages[contentCellValueOrOffset >> 16];
			VarCell& varCell = pVarPage->pVar[contentCellValueOrOffset & 0xFFFF];

			//save value
			contentCellType = varCell.ContCellType; //read from var cell
			contentCellValueOrOffset = varCell.ContCellValue;

			(*visitor)(key,
					   keyOffset,
					   varCell.ValueContCellValue,
					   pData);

			if(contentCellType == CONTINUE_VAR_TYPE) //CONTINUE VAR =====================================================================
			{
				contentOffset = contentCellValueOrOffset;

				//goto
				scanKeysAndValues(key, keyOffset, contentOffset, visitor, pData);

				return;
			}
		}

		if(contentCellType <= MAX_BRANCH_TYPE1) //BRANCH =====================================================================
		{
			BranchPage* pBranchPage = pBranchPages[contentCellValueOrOffset >> 16];
			BranchCell& branchCell = pBranchPage->pBranch[contentCellValueOrOffset & 0xFFFF];

			//check other
			for(int32_t i = 0; i<contentCellType; i++) //from 1
			{
				int32_t& keyValue = branchCell.Values[i];

				key[keyOffset] = keyValue;

				scanKeysAndValues(key, keyOffset + 1, branchCell.Offsets[i], visitor, pData);
			}

			return;
		}
		else if(contentCellType == VALUE_TYPE)
		{
			(*visitor)(key,
					   keyOffset,
					   contentCellValueOrOffset,
					   pData);

			return;
		}
		else if(contentCellType <= MAX_BLOCK_TYPE) //VALUE IN BLOCK ===================================================================
		{
			scanKeysAndValuesFromBlock(key,
										contentOffset,
										keyOffset,
										contentCellValueOrOffset,
										visitor,
										pData);

			return;
		}
		else if(contentCellType == CURRENT_VALUE_TYPE)
		{
			int32_t& keyValue = contentCellValueOrOffset;

			key[keyOffset] = keyValue;
		}
	}
}

void HArray::scanKeysAndValues(int32_t* key,
								int32_t keyLen,
								HARRAY_ITEM_VISIT_FUNC visitor,
								void* pData)
{
	int32_t maxSafeShort = MAX_SAFE_SHORT - keyLen;

	int32_t headerOffset;

	if (!normalizeFunc)
	{
		headerOffset = key[0] >> HeaderBits;
	}
	else
	{
		headerOffset = (*normalizeFunc)(key);
	}

	int32_t contentOffset = pHeader[headerOffset];

	if(contentOffset)
	{
		int32_t keyOffset = 0;

NEXT_KEY_PART:
		ContentPage* pContentPage = pContentPages[contentOffset>>16];
		uint16_t contentIndex = contentOffset&0xFFFF;

		uint8_t contentCellType = pContentPage->pType[contentIndex]; //move to type part

		if(contentCellType >= ONLY_CONTENT_TYPE) //ONLY CONTENT =========================================================================================
		{
			int32_t fullKeyLen = keyOffset + contentCellType - ONLY_CONTENT_TYPE;

			if(contentIndex < maxSafeShort) //content in one page
			{
				for(; keyOffset < keyLen; contentIndex++, keyOffset++)
				{
					if(pContentPage->pContent[contentIndex] != key[keyOffset])
						return;
				}

				for(; keyOffset < fullKeyLen; contentIndex++, keyOffset++)
				{
					key[keyOffset] = pContentPage->pContent[contentIndex];
				}

				if (contentIndex < MAX_SHORT) //remove warning of compilator
				{
					(*visitor)(key,
						keyOffset,
						pContentPage->pContent[contentIndex],
						pData); //return value
				}
				else
				{
					printf("!!! FAIL STATE !!!");
				}

				return;
			}
			else //content in two pages
			{
				for(; keyOffset < keyLen; contentOffset++, keyOffset++)
				{
					if(pContentPages[contentOffset>>16]->pContent[contentOffset&0xFFFF] != key[keyOffset])
						return;
				}

				for(; keyOffset < fullKeyLen; contentIndex++, keyOffset++)
				{
					key[keyOffset] = pContentPage->pContent[contentIndex];
				}

				(*visitor)(key,
						   keyOffset,
						   pContentPages[contentOffset>>16]->pContent[contentOffset&0xFFFF],
						   pData);

				return;
			}
		}

		int32_t& keyValue = key[keyOffset];
		int32_t contentCellValueOrOffset = pContentPage->pContent[contentIndex];

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
				scanKeysAndValues(key, keyOffset, contentOffset, visitor, pData);

				//return varCell.Value;

				return;
			}
		}
		else if(keyOffset == keyLen)
		{
			if(contentCellType == VALUE_TYPE)
			{
				(*visitor)(key,
						   keyOffset,
					       contentCellValueOrOffset,
						   pData);

				return;

			}
			else
			{
				scanKeysAndValues(key, keyOffset, contentOffset, visitor, pData);

				return;
			}
		}

		if(contentCellType <= MAX_BRANCH_TYPE1) //BRANCH =====================================================================
		{
			BranchPage* pBranchPage = pBranchPages[contentCellValueOrOffset >> 16];
			BranchCell& branchCell = pBranchPage->pBranch[contentCellValueOrOffset & 0xFFFF];

			//try find value in the list
			int32_t* values = branchCell.Values;

			for(int32_t i=0; i<contentCellType; i++)
			{
				if(values[i] == keyValue)
				{
					contentOffset = branchCell.Offsets[i];
					keyOffset++;

					goto NEXT_KEY_PART;
				}
			}

			return;
		}
		else if(contentCellType == VALUE_TYPE)
		{
			(*visitor)(key,
					   keyOffset,
					   contentCellValueOrOffset,
					   pData);

			return;
		}
		else if(contentCellType <= MAX_BLOCK_TYPE) //VALUE IN BLOCK ===================================================================
		{
			uint8_t idxKeyValue = (contentCellType - MIN_BLOCK_TYPE) * BLOCK_ENGINE_STEP;

			int32_t startOffset = contentCellValueOrOffset;

	NEXT_BLOCK:
			int32_t subOffset = ((keyValue << idxKeyValue) >> BLOCK_ENGINE_SHIFT);
			int32_t blockOffset = startOffset + subOffset;

			BlockPage* pBlockPage = pBlockPages[blockOffset >> 16];
			BlockCell& blockCell = pBlockPage->pBlock[blockOffset & 0xFFFF];

			uint8_t& blockCellType = blockCell.Type;

			if(blockCellType == EMPTY_TYPE)
			{
				return;
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
					return;
				}
			}
			else if(blockCellType <= MAX_BRANCH_TYPE1) //branch cell
			{
				BranchPage* pBranchPage = pBranchPages[blockCell.Offset >> 16];
				BranchCell& branchCell1 = pBranchPage->pBranch[blockCell.Offset & 0xFFFF];

				//try find value in the list
				for(int32_t i=0; i<blockCellType; i++)
				{
					if(branchCell1.Values[i] == keyValue)
					{
						contentOffset = branchCell1.Offsets[i];
						keyOffset++;

						goto NEXT_KEY_PART;
					}
				}

				return;
			}
			else if(blockCellType <= MAX_BRANCH_TYPE2) //branch cell
			{
				BranchPage* pBranchPage1 = pBranchPages[blockCell.Offset >> 16];
				BranchCell branchCell1 = pBranchPage1->pBranch[blockCell.Offset & 0xFFFF];

				//try find value in the list
				for(int32_t i=0; i < BRANCH_ENGINE_SIZE; i++)
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
				int32_t countValues = blockCellType - MAX_BRANCH_TYPE1;

				for(int32_t i=0; i<countValues; i++)
				{
					if(branchCell2.Values[i] == keyValue)
					{
						contentOffset = branchCell2.Offsets[i];
						keyOffset++;

						goto NEXT_KEY_PART;
					}
				}

				return;
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
				return;
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
				return;
			}
		}
	}

	return;
}

//scan all
void HArray::scanKeysAndValues(HARRAY_ITEM_VISIT_FUNC visitor,
								void* pData)
{
	int32_t key[1024];

	for (int32_t i = 0; i < HeaderSize; i++)
	{
		int32_t contentOffset = pHeader[i];

		if (contentOffset)
		{
			key[0] = (i << HeaderBits);

			scanKeysAndValues(key, 0, visitor, pData);
		}
	}

	return;
}

////RETURN ARRAY ======================================================================================================================
//void HArray::scanKeysAndValuesFromBlock(int32_t* key,
//	int32_t keyLen,
//	int32_t contentOffset,
//	int32_t keyOffset,
//	int32_t blockOffset,
//	HArrayPair* pairs,
//	int32_t& countPairs)
//{
//	//printf("getValuesByRangeFromBlock count=%d size=%d contentOffset=%d keyOffset=%d blockOffset=%d\n", count, size, contentOffset, keyOffset, blockOffset);
//
//	int32_t maxOffset = blockOffset + BLOCK_ENGINE_SIZE;
//
//	for (int32_t offset = blockOffset; offset < maxOffset; offset++)
//	{
//		BlockPage* pBlockPage = pBlockPages[offset >> 16];
//		BlockCell& blockCell = pBlockPage->pBlock[offset & 0xFFFF];
//
//		uint8_t& blockCellType = blockCell.Type;
//
//		if (blockCellType == EMPTY_TYPE)
//		{
//			continue;
//		}
//		else if (blockCellType == CURRENT_VALUE_TYPE) //current value
//		{
//			int32_t& keyValue = blockCell.ValueOrOffset;
//
//			key[keyOffset] = keyValue;
//
//			scanKeysAndValues(key, keyLen, keyOffset + 1, blockCell.Offset, pairs, countPairs);
//		}
//		else if (blockCellType <= MAX_BRANCH_TYPE1) //branch cell
//		{
//			BranchPage* pBranchPage = pBranchPages[blockCell.Offset >> 16];
//			BranchCell& branchCell1 = pBranchPage->pBranch[blockCell.Offset & 0xFFFF];
//
//			//try find value in the list
//			for (int32_t i = 0; i < blockCellType; i++)
//			{
//				int32_t& keyValue = branchCell1.Values[i];
//
//				key[keyOffset] = keyValue;
//
//				scanKeysAndValues(key, keyLen, keyOffset + 1, branchCell1.Offsets[i], pairs, countPairs);
//			}
//		}
//		else if (blockCellType <= MAX_BRANCH_TYPE2) //branch cell
//		{
//			BranchPage* pBranchPage1 = pBranchPages[blockCell.Offset >> 16];
//			BranchCell branchCell1 = pBranchPage1->pBranch[blockCell.Offset & 0xFFFF];
//
//			//try find value in the list
//			for (int32_t i = 0; i < BRANCH_ENGINE_SIZE; i++)
//			{
//				int32_t& keyValue = branchCell1.Values[i];
//
//				key[keyOffset] = keyValue;
//
//				scanKeysAndValues(key, keyLen, keyOffset + 1, branchCell1.Offsets[i], pairs, countPairs);
//			}
//
//			BranchPage* pBranchPage2 = pBranchPages[blockCell.ValueOrOffset >> 16];
//			BranchCell branchCell2 = pBranchPage2->pBranch[blockCell.ValueOrOffset & 0xFFFF];
//
//			//try find value in the list
//			int32_t countValues = blockCellType - MAX_BRANCH_TYPE1;
//
//			for (int32_t i = 0; i < countValues; i++)
//			{
//				int32_t& keyValue = branchCell2.Values[i];
//
//				key[keyOffset] = keyValue;
//
//				scanKeysAndValues(key, keyLen, keyOffset + 1, branchCell2.Offsets[i], pairs, countPairs);
//			}
//		}
//		else if (blockCell.Type <= MAX_BLOCK_TYPE)
//		{
//			//go to block
//			scanKeysAndValuesFromBlock(key,
//				keyLen,
//				contentOffset,
//				keyOffset,
//				blockCell.Offset,
//				pairs,
//				countPairs);
//		}
//	}
//}
//
//void HArray::scanKeysAndValues(int32_t* key,
//	int32_t keyLen,
//	int32_t keyOffset,
//	int32_t contentOffset,
//	int32_t* values,
//	int32_t& countValues)
//{
//	//printf("getValuesByRange count=%d size=%d contentOffset=%d keyOffset=%d\n", count, size, contentOffset, keyOffset);
//
//	for (;; keyOffset++, contentOffset++)
//	{
//		ContentPage* pContentPage = pContentPages[contentOffset >> 16];
//		uint16_t contentIndex = contentOffset & 0xFFFF;
//
//		int32_t contentCellValueOrOffset = pContentPage->pContent[contentIndex];
//		uint8_t contentCellType = pContentPage->pType[contentIndex]; //move to type part
//
//		if (contentCellType >= ONLY_CONTENT_TYPE) //ONLY CONTENT =========================================================================================
//		{
//			int32_t keyLen = contentCellType - ONLY_CONTENT_TYPE;
//
//			for (int32_t i = 0; i < keyLen; i++, keyOffset++, contentOffset++)
//			{
//				int32_t& keyValue = pContentPages[contentOffset >> 16]->pContent[contentOffset & 0xFFFF];
//
//				key[keyOffset] = keyValue;
//			}
//
//			pairs[countPairs++].setPair(keyLen,
//				key,
//				keyOffset,
//				pContentPages[contentOffset >> 16]->pContent[contentOffset & 0xFFFF],
//				pContentPages[contentOffset >> 16]->pType[contentOffset & 0xFFFF]);
//
//			return;
//		}
//
//		if (contentCellType == VAR_TYPE) //VAR =====================================================================
//		{
//			VarPage* pVarPage = pVarPages[contentCellValueOrOffset >> 16];
//			VarCell& varCell = pVarPage->pVar[contentCellValueOrOffset & 0xFFFF];
//
//			//save value
//			contentCellType = varCell.ContCellType; //read from var cell
//			contentCellValueOrOffset = varCell.ContCellValue;
//
//			pairs[countPairs++].setPair(keyLen,
//				key,
//				keyOffset,
//				varCell.ValueContCellValue,
//				varCell.ValueContCellType);
//
//			if (contentCellType == CONTINUE_VAR_TYPE) //CONTINUE VAR =====================================================================
//			{
//				contentOffset = contentCellValueOrOffset;
//
//				//goto
//				scanKeysAndValues(key, keyLen, keyOffset, contentOffset, pairs, countPairs);
//
//				return;
//			}
//		}
//
//		if (contentCellType <= MAX_BRANCH_TYPE1) //BRANCH =====================================================================
//		{
//			BranchPage* pBranchPage = pBranchPages[contentCellValueOrOffset >> 16];
//			BranchCell& branchCell = pBranchPage->pBranch[contentCellValueOrOffset & 0xFFFF];
//
//			//check other
//			for (int32_t i = 0; i < contentCellType; i++) //from 1
//			{
//				int32_t& keyValue = branchCell.Values[i];
//
//				key[keyOffset] = keyValue;
//
//				scanKeysAndValues(key, keyLen, keyOffset + 1, branchCell.Offsets[i], pairs, countPairs);
//			}
//
//			return;
//		}
//		else if (VALUE_TYPE_1 <= contentCellType && contentCellType <= VALUE_TYPE_5)
//		{
//			pairs[countPairs++].setPair(keyLen,
//				key,
//				keyOffset,
//				contentCellValueOrOffset,
//				contentCellType);
//
//			return;
//		}
//		else if (contentCellType <= MAX_BLOCK_TYPE) //VALUE IN BLOCK ===================================================================
//		{
//			scanKeysAndValuesFromBlock(key,
//				keyLen,
//				contentOffset,
//				keyOffset,
//				contentCellValueOrOffset,
//				pairs,
//				countPairs);
//
//			return;
//		}
//		else if (contentCellType == CURRENT_VALUE_TYPE)
//		{
//			int32_t& keyValue = contentCellValueOrOffset;
//
//			key[keyOffset] = keyValue;
//		}
//	}
//}
//
//void HArray::scanKeysAndValues(int32_t* key,
//	int32_t keyLen,
//	int32_t filterKeyLen, //set 
//	int32_t* values,
//	int32_t& countValues)
//{
//	int32_t maxSafeShort = MAX_SAFE_SHORT - keyLen;
//
//	int32_t headerOffset;
//
//	if (!normalizeFunc)
//	{
//		headerOffset = key[0] >> HeaderBits;
//	}
//	else
//	{
//		headerOffset = (*normalizeFunc)(key);
//	}
//
//	int32_t contentOffset = pHeader[headerOffset];
//
//	if (contentOffset)
//	{
//		int32_t keyOffset = 0;
//
//	NEXT_KEY_PART:
//		ContentPage* pContentPage = pContentPages[contentOffset >> 16];
//		uint16_t contentIndex = contentOffset & 0xFFFF;
//
//		uint8_t contentCellType = pContentPage->pType[contentIndex]; //move to type part
//
//		if (contentCellType >= ONLY_CONTENT_TYPE) //ONLY CONTENT =========================================================================================
//		{
//			int32_t fullKeyLen = keyOffset + contentCellType - ONLY_CONTENT_TYPE;
//
//			if (contentIndex < maxSafeShort) //content in one page
//			{
//				for (; keyOffset < keyLen; contentIndex++, keyOffset++)
//				{
//					if (pContentPage->pContent[contentIndex] != key[keyOffset])
//						return;
//				}
//
//				for (; keyOffset < fullKeyLen; contentIndex++, keyOffset++)
//				{
//					key[keyOffset] = pContentPage->pContent[contentIndex];
//				}
//
//				if (contentIndex < MAX_SHORT) //remove warning of compilator
//				{
//					pairs[countPairs++].setPair(keyLen, 
//						key,
//						keyOffset,
//						pContentPage->pContent[contentIndex],
//						pContentPage->pType[contentIndex]);
//				}
//				else
//				{
//					printf("!!! FAIL STATE !!!");
//				}
//
//				return;
//			}
//			else //content in two pages
//			{
//				for (; keyOffset < keyLen; contentOffset++, keyOffset++)
//				{
//					if (pContentPages[contentOffset >> 16]->pContent[contentOffset & 0xFFFF] != key[keyOffset])
//						return;
//				}
//
//				for (; keyOffset < fullKeyLen; contentIndex++, keyOffset++)
//				{
//					key[keyOffset] = pContentPage->pContent[contentIndex];
//				}
//
//				pairs[countPairs++].setPair(keyLen,
//					key,
//					keyOffset,
//					pContentPages[contentOffset >> 16]->pContent[contentOffset & 0xFFFF],
//					pContentPages[contentOffset >> 16]->pType[contentOffset & 0xFFFF]);
//
//				return;
//			}
//		}
//
//		int32_t& keyValue = key[keyOffset];
//		int32_t contentCellValueOrOffset = pContentPage->pContent[contentIndex];
//
//		if (contentCellType == VAR_TYPE) //VAR =====================================================================
//		{
//			VarPage* pVarPage = pVarPages[contentCellValueOrOffset >> 16];
//			VarCell& varCell = pVarPage->pVar[contentCellValueOrOffset & 0xFFFF];
//
//			if (keyOffset < keyLen)
//			{
//				contentCellType = varCell.ContCellType; //read from var cell
//				contentCellValueOrOffset = varCell.ContCellValue;
//
//				if (contentCellType == CONTINUE_VAR_TYPE) //CONTINUE VAR =====================================================================
//				{
//					contentOffset = contentCellValueOrOffset;
//
//					goto NEXT_KEY_PART;
//				}
//			}
//			else
//			{
//				scanKeysAndValues(key, keyLen, keyOffset, contentOffset, pairs, countPairs);
//
//				//return varCell.Value;
//
//				return;
//			}
//		}
//		else if (keyOffset == keyLen)
//		{
//			if (VALUE_TYPE_1 <= contentCellType && contentCellType <= VALUE_TYPE_5)
//			{
//				pairs[countPairs++].setPair(keyLen,
//					key,
//					keyOffset,
//					contentCellValueOrOffset,
//					contentCellType);
//
//				return;
//
//			}
//			else
//			{
//				scanKeysAndValues(key, keyLen, keyOffset, contentOffset, pairs, countPairs);
//
//				return;
//			}
//		}
//
//		if (contentCellType <= MAX_BRANCH_TYPE1) //BRANCH =====================================================================
//		{
//			BranchPage* pBranchPage = pBranchPages[contentCellValueOrOffset >> 16];
//			BranchCell& branchCell = pBranchPage->pBranch[contentCellValueOrOffset & 0xFFFF];
//
//			//try find value in the list
//			int32_t* values = branchCell.Values;
//
//			for (int32_t i = 0; i < contentCellType; i++)
//			{
//				if (values[i] == keyValue)
//				{
//					contentOffset = branchCell.Offsets[i];
//					keyOffset++;
//
//					goto NEXT_KEY_PART;
//				}
//			}
//
//			return;
//		}
//		else if (VALUE_TYPE_1 <= contentCellType && contentCellType <= VALUE_TYPE_5)
//		{
//			pairs[countPairs++].setPair(keyLen,
//				key,
//				keyOffset,
//				contentCellValueOrOffset,
//				contentCellType);
//
//			return;
//		}
//		else if (contentCellType <= MAX_BLOCK_TYPE) //VALUE IN BLOCK ===================================================================
//		{
//			uint8_t idxKeyValue = (contentCellType - MIN_BLOCK_TYPE) * BLOCK_ENGINE_STEP;
//
//			int32_t startOffset = contentCellValueOrOffset;
//
//		NEXT_BLOCK:
//			int32_t subOffset = ((keyValue << idxKeyValue) >> BLOCK_ENGINE_SHIFT);
//			int32_t blockOffset = startOffset + subOffset;
//
//			BlockPage* pBlockPage = pBlockPages[blockOffset >> 16];
//			BlockCell& blockCell = pBlockPage->pBlock[blockOffset & 0xFFFF];
//
//			uint8_t& blockCellType = blockCell.Type;
//
//			if (blockCellType == EMPTY_TYPE)
//			{
//				return;
//			}
//			else if (blockCellType == CURRENT_VALUE_TYPE) //current value
//			{
//				if (blockCell.ValueOrOffset == keyValue) //value is exists
//				{
//					contentOffset = blockCell.Offset;
//					keyOffset++;
//
//					goto NEXT_KEY_PART;
//				}
//				else
//				{
//					return;
//				}
//			}
//			else if (blockCellType <= MAX_BRANCH_TYPE1) //branch cell
//			{
//				BranchPage* pBranchPage = pBranchPages[blockCell.Offset >> 16];
//				BranchCell& branchCell1 = pBranchPage->pBranch[blockCell.Offset & 0xFFFF];
//
//				//try find value in the list
//				for (int32_t i = 0; i < blockCellType; i++)
//				{
//					if (branchCell1.Values[i] == keyValue)
//					{
//						contentOffset = branchCell1.Offsets[i];
//						keyOffset++;
//
//						goto NEXT_KEY_PART;
//					}
//				}
//
//				return;
//			}
//			else if (blockCellType <= MAX_BRANCH_TYPE2) //branch cell
//			{
//				BranchPage* pBranchPage1 = pBranchPages[blockCell.Offset >> 16];
//				BranchCell branchCell1 = pBranchPage1->pBranch[blockCell.Offset & 0xFFFF];
//
//				//try find value in the list
//				for (int32_t i = 0; i < BRANCH_ENGINE_SIZE; i++)
//				{
//					if (branchCell1.Values[i] == keyValue)
//					{
//						contentOffset = branchCell1.Offsets[i];
//						keyOffset++;
//
//						goto NEXT_KEY_PART;
//					}
//				}
//
//				BranchPage* pBranchPage2 = pBranchPages[blockCell.ValueOrOffset >> 16];
//				BranchCell branchCell2 = pBranchPage2->pBranch[blockCell.ValueOrOffset & 0xFFFF];
//
//				//try find value in the list
//				int32_t countValues = blockCellType - MAX_BRANCH_TYPE1;
//
//				for (int32_t i = 0; i < countValues; i++)
//				{
//					if (branchCell2.Values[i] == keyValue)
//					{
//						contentOffset = branchCell2.Offsets[i];
//						keyOffset++;
//
//						goto NEXT_KEY_PART;
//					}
//				}
//
//				return;
//			}
//			else if (blockCell.Type <= MAX_BLOCK_TYPE)
//			{
//				//go to block
//				idxKeyValue = (blockCell.Type - MIN_BLOCK_TYPE) * BLOCK_ENGINE_STEP;
//				startOffset = blockCell.Offset;
//
//				goto NEXT_BLOCK;
//			}
//			else
//			{
//				return;
//			}
//		}
//		else if (contentCellType == CURRENT_VALUE_TYPE) //PART OF KEY =========================================================================
//		{
//			if (contentCellValueOrOffset == keyValue)
//			{
//				contentOffset++;
//				keyOffset++;
//
//				goto NEXT_KEY_PART;
//			}
//			else
//			{
//				return;
//			}
//		}
//	}
//
//	return;
//}