
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

bool HArray::insert(uint32_t* key,
	uint32_t keyLen,
	uint32_t value) //Type of value parameter: VALUE_TYPE_1 (by default), VALUE_TYPE_2 ... VALUE_TYPE_5
{
	try
	{
		uint32_t maxSafeShort = MAX_SAFE_SHORT - keyLen;

		//create new page =======================================================================================
		if (!pContentPages[(lastContentOffset + keyLen + ValueLen) >> 16])
		{
			pContentPages[ContentPagesCount++] = new ContentPage();

			if (ContentPagesCount == ContentPagesSize)
			{
				reallocateContentPages();
			}
		}

		//insert value ==========================================================================================
		uint32_t keyOffset = 0;
		uint32_t headerOffset;
		uint32_t keyValue;

		uint8_t* pContentCellType;
		uint32_t* pContentCellValue;

		if (!normalizeFunc)
		{
			headerOffset = key[0] >> HeaderBits;
		}
		else
		{
			headerOffset = (*normalizeFunc)(key) >> HeaderBits;
		}

		uint32_t contentOffset = pHeader[headerOffset];

		ContentPage* pContentPage;
		uint32_t contentIndex;

		uint8_t contentCellType;
		//uint32_t contentCellValue;

		uint32_t* pSetLastContentOffset = 0;

		if (!contentOffset)
		{
#ifndef _RELEASE
			tempValues[SHORT_WAY_STAT]++;
#endif
			//insert key in free slot
			if (tailReleasedContentOffsets[keyLen])
			{
				uint32_t startContentOffset = tailReleasedContentOffsets[keyLen];

				pHeader[headerOffset] = startContentOffset;

				ContentPage* pContentPage = pContentPages[startContentOffset >> 16];
				uint32_t contentIndex = startContentOffset & 0xFFFF;

				pContentPage->pType[contentIndex] = (ONLY_CONTENT_TYPE + keyLen);
				tailReleasedContentOffsets[keyLen] = pContentPage->pContent[contentIndex];

				countReleasedContentCells -= (keyLen + ValueLen);

				//fill key
				for (; keyOffset < keyLen; contentIndex++, keyOffset++)
				{
					pContentPage->pContent[contentIndex] = key[keyOffset];
				}

				pContentPage->pType[contentIndex] = VALUE_TYPE;
				pContentPage->pContent[contentIndex] = value;

				return true;
			}
			else
			{
				ContentPage* pContentPage = pContentPages[lastContentOffset >> 16];
				uint32_t contentIndex = lastContentOffset & 0xFFFF;

				if (contentIndex > maxSafeShort) //content in one page
				{
					//release content cells
					uint32_t spaceLen = MAX_SHORT - contentIndex - 1;

					pContentPage->pContent[contentIndex] = tailReleasedContentOffsets[spaceLen];

					tailReleasedContentOffsets[spaceLen] = lastContentOffset;

					countReleasedContentCells += (spaceLen + ValueLen);

					//move to next page
					uint32_t page = (lastContentOffset >> 16) + 1;

					pContentPage = pContentPages[page];

					lastContentOffset = (page << 16);

					contentIndex = 0;
				}

				pHeader[headerOffset] = lastContentOffset;

				pContentPage->pType[contentIndex] = (ONLY_CONTENT_TYPE + keyLen);

				//fill key
				for (; keyOffset < keyLen; contentIndex++, keyOffset++, lastContentOffset++)
				{
					pContentPage->pContent[contentIndex] = key[keyOffset];
				}

				pContentPage->pType[contentIndex] = VALUE_TYPE;
				pContentPage->pContent[contentIndex] = value;

				lastContentOffset++;

				return true;
			}
		}

#ifndef _RELEASE
		tempValues[LONG_WAY_STAT]++;
#endif

		//TWO KEYS =============================================================================================
	NEXT_KEY_PART:

		pContentPage = pContentPages[contentOffset >> 16];
		contentIndex = contentOffset & 0xFFFF;

		contentCellType = pContentPage->pType[contentIndex];
		//contentCellValue = pContentPage->pContent[contentIndex];

		if (contentCellType >= ONLY_CONTENT_TYPE) //ONLY CONTENT =========================================================================
		{
			uint32_t originKeyLen = keyOffset + contentCellType - ONLY_CONTENT_TYPE;

			if (contentIndex < maxSafeShort) //content in one page
			{
				for (; keyOffset < originKeyLen; contentOffset++, contentIndex++, keyOffset++)
				{
					uint8_t& contentCellType = pContentPage->pType[contentIndex];
					uint32_t& contentCellValue = pContentPage->pContent[contentIndex];

					//key shorter than already inserted, insert
					if (keyOffset == keyLen)
					{
						VarCell* pVarCell;

						if (countReleasedVarCells)
						{
							uint32_t varOffset = tailReleasedVarOffset;

							pVarCell = &pVarPages[varOffset >> 16]->pVar[varOffset & 0xFFFF];

							tailReleasedVarOffset = pVarCell->ValueContCellValue;

							countReleasedVarCells--;
						}
						else
						{
							VarPage* pVarPage = pVarPages[lastVarOffset >> 16];
							if (!pVarPage)
							{
								pVarPage = new VarPage();
								pVarPages[VarPagesCount++] = pVarPage;

								if (VarPagesCount == VarPagesSize)
								{
									reallocateVarPages();
								}
							}

							pVarCell = &pVarPage->pVar[lastVarOffset & 0xFFFF];
						}

						pVarCell->ContCellType = CURRENT_VALUE_TYPE;
						pVarCell->ContCellValue = contentCellValue;

						pVarCell->ValueContCellType = VALUE_TYPE;
						pVarCell->ValueContCellValue = value;

						contentCellType = VAR_TYPE;
						contentCellValue = lastVarOffset++;

						//set rest of key and value
						for (contentIndex++, keyOffset++; keyOffset < originKeyLen; contentIndex++, keyOffset++)
						{
							pContentPage->pType[contentIndex] = CURRENT_VALUE_TYPE;
						}

						//pContentPage->pType[contentIndex] = VALUE_TYPE;

						return true;
					}
					else if (contentCellValue != key[keyOffset])
					{
#ifndef _RELEASE
						tempValues[CONTENT_BRANCH_STAT]++;
#endif

						//create branch
						contentCellType = MIN_BRANCH_TYPE1 + 1;

						//get free branch cell
						BranchCell* pBranchCell;
						if (countReleasedBranchCells)
						{
							uint32_t branchOffset = tailReleasedBranchOffset;

							pBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];

							tailReleasedBranchOffset = pBranchCell->Values[0];

							countReleasedBranchCells--;

							pBranchCell->Values[0] = contentCellValue;
							contentCellValue = branchOffset;
						}
						else
						{
							BranchPage* pBranchPage = pBranchPages[lastBranchOffset >> 16];
							if (!pBranchPage)
							{
								pBranchPage = new BranchPage();
								pBranchPages[BranchPagesCount++] = pBranchPage;

								if (BranchPagesCount == BranchPagesSize)
								{
									reallocateBranchPages();
								}
							}

							pBranchCell = &pBranchPage->pBranch[lastBranchOffset & 0xFFFF];

							pBranchCell->Values[0] = contentCellValue;
							contentCellValue = lastBranchOffset++;
						}

						pBranchCell->Offsets[0] = contentOffset + 1;

						pBranchCell->Values[1] = key[keyOffset];
						pSetLastContentOffset = &pBranchCell->Offsets[1];

						if ((keyOffset + 1) < originKeyLen)
						{
							pContentPage->pType[contentIndex + 1] = (ONLY_CONTENT_TYPE + originKeyLen - keyOffset - 1);
						}

						goto FILL_KEY;
					}
					else
					{
						contentCellType = CURRENT_VALUE_TYPE; //reset to current value
					}
				}

				if (keyLen > originKeyLen) //key longer than already inserted
				{
					VarCell* pVarCell;

					if (countReleasedVarCells)
					{
						uint32_t varOffset = tailReleasedVarOffset;

						pVarCell = &pVarPages[varOffset >> 16]->pVar[varOffset & 0xFFFF];

						tailReleasedVarOffset = pVarCell->ValueContCellValue;

						countReleasedVarCells--;
					}
					else
					{
						VarPage* pVarPage = pVarPages[lastVarOffset >> 16];
						if (!pVarPage)
						{
							pVarPage = new VarPage();
							pVarPages[VarPagesCount++] = pVarPage;

							if (VarPagesCount == VarPagesSize)
							{
								reallocateVarPages();
							}
						}

						pVarCell = &pVarPage->pVar[lastVarOffset & 0xFFFF];
					}

					pVarCell->ValueContCellType = VALUE_TYPE;
					pVarCell->ValueContCellValue = pContentPage->pContent[contentIndex];

					pVarCell->ContCellType = CONTINUE_VAR_TYPE;
					pSetLastContentOffset = &pVarCell->ContCellValue;

					pContentPage->pType[contentIndex] = VAR_TYPE;
					pContentPage->pContent[contentIndex] = lastVarOffset++;

					goto FILL_KEY2;
				}
				else //key is exists, update
				{
					pContentPage->pType[contentIndex] = VALUE_TYPE;
					pContentPage->pContent[contentIndex] = value;

					return false;
				}
			}
			else  //content in two pages
			{
				for (; keyOffset < originKeyLen; contentOffset++, keyOffset++)
				{
					pContentPage = pContentPages[contentOffset >> 16];
					contentIndex = contentOffset & 0xFFFF;

					//key less than original, insert
					if (keyOffset == keyLen)
					{
						VarCell* pVarCell;

						if (countReleasedVarCells)
						{
							uint32_t varOffset = tailReleasedVarOffset;

							pVarCell = &pVarPages[varOffset >> 16]->pVar[varOffset & 0xFFFF];

							tailReleasedVarOffset = pVarCell->ValueContCellValue;

							countReleasedVarCells--;
						}
						else
						{
							VarPage* pVarPage = pVarPages[lastVarOffset >> 16];
							if (!pVarPage)
							{
								pVarPage = new VarPage();
								pVarPages[VarPagesCount++] = pVarPage;

								if (VarPagesCount == VarPagesSize)
								{
									reallocateVarPages();
								}
							}

							pVarCell = &pVarPage->pVar[lastVarOffset & 0xFFFF];
						}

						pVarCell->ContCellType = CURRENT_VALUE_TYPE;
						pVarCell->ContCellValue = pContentPage->pContent[contentIndex];

						pVarCell->ValueContCellType = VALUE_TYPE;
						pVarCell->ValueContCellValue = value;

						pContentPage->pType[contentIndex] = VAR_TYPE;
						pContentPage->pContent[contentIndex] = lastVarOffset++;

						//set rest of key and value
						for (contentOffset++, keyOffset++; keyOffset < originKeyLen; contentOffset++, keyOffset++)
						{
							pContentPages[contentOffset >> 16]->pType[contentOffset & 0xFFFF] = CURRENT_VALUE_TYPE;
						}

						//pContentPages[contentOffset >> 16]->pType[contentOffset & 0xFFFF] = VALUE_TYPE;

						return true;
					}
					else if (pContentPage->pContent[contentIndex] != key[keyOffset])
					{
#ifndef _RELEASE
						tempValues[CONTENT_BRANCH_STAT]++;
#endif

						//create branch
						pContentPage->pType[contentIndex] = MIN_BRANCH_TYPE1 + 1;

						//get free branch cell
						BranchCell* pBranchCell;
						if (countReleasedBranchCells)
						{
							uint32_t branchOffset = tailReleasedBranchOffset;

							pBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];

							tailReleasedBranchOffset = pBranchCell->Values[0];

							countReleasedBranchCells--;

							pBranchCell->Values[0] = pContentPage->pContent[contentIndex];
							pContentPage->pContent[contentIndex] = branchOffset;
						}
						else
						{
							BranchPage* pBranchPage = pBranchPages[lastBranchOffset >> 16];
							if (!pBranchPage)
							{
								pBranchPage = new BranchPage();
								pBranchPages[BranchPagesCount++] = pBranchPage;

								if (BranchPagesCount == BranchPagesSize)
								{
									reallocateBranchPages();
								}
							}

							pBranchCell = &pBranchPage->pBranch[lastBranchOffset & 0xFFFF];

							pBranchCell->Values[0] = pContentPage->pContent[contentIndex];
							pContentPage->pContent[contentIndex] = lastBranchOffset++;
						}

						pBranchCell->Offsets[0] = contentOffset + 1;

						pBranchCell->Values[1] = key[keyOffset];
						pSetLastContentOffset = &pBranchCell->Offsets[1];

						if ((keyOffset + 1) < originKeyLen)
						{
							//set rest of key
							contentOffset++;

							pContentPage = pContentPages[contentOffset >> 16];
							contentIndex = contentOffset & 0xFFFF;

							pContentPage->pType[contentIndex] = (ONLY_CONTENT_TYPE + originKeyLen - keyOffset - 1);
						}

						goto FILL_KEY;
					}
					else
					{
						pContentPage->pType[contentIndex] = CURRENT_VALUE_TYPE; //reset to current value
					}
				}

				pContentPage = pContentPages[contentOffset >> 16];
				contentIndex = contentOffset & 0xFFFF;

				if (keyLen > originKeyLen) //key more than original
				{
					VarCell* pVarCell;

					if (countReleasedVarCells)
					{
						uint32_t varOffset = tailReleasedVarOffset;

						pVarCell = &pVarPages[varOffset >> 16]->pVar[varOffset & 0xFFFF];

						tailReleasedVarOffset = pVarCell->ValueContCellValue;

						countReleasedVarCells--;
					}
					else
					{
						VarPage* pVarPage = pVarPages[lastVarOffset >> 16];
						if (!pVarPage)
						{
							pVarPage = new VarPage();
							pVarPages[VarPagesCount++] = pVarPage;

							if (VarPagesCount == VarPagesSize)
							{
								reallocateVarPages();
							}
						}

						pVarCell = &pVarPage->pVar[lastVarOffset & 0xFFFF];
					}

					pVarCell->ValueContCellType = VALUE_TYPE;
					pVarCell->ValueContCellValue = pContentPage->pContent[contentIndex];

					pVarCell->ContCellType = CONTINUE_VAR_TYPE;
					pSetLastContentOffset = &pVarCell->ContCellValue;

					pContentPage->pType[contentIndex] = VAR_TYPE;
					pContentPage->pContent[contentIndex] = lastVarOffset++;

					goto FILL_KEY2;
				}
				else //key is exists, update
				{
					pContentPage->pContent[contentIndex] = value;

					return false;
				}
			}

			return true;
		}

		keyValue = key[keyOffset];

		pContentCellType = &pContentPage->pType[contentIndex];
		pContentCellValue = &pContentPage->pContent[contentIndex];

		uint32_t contentCellValueOrOffset;

		contentCellValueOrOffset = pContentPage->pContent[contentIndex];

		if (contentCellType == VAR_TYPE) //VAR =====================================================================
		{
			VarPage* pVarPage = pVarPages[contentCellValueOrOffset >> 16];
			VarCell& varCell = pVarPage->pVar[contentCellValueOrOffset & 0xFFFF];

			if (keyOffset < keyLen)
			{
				contentCellType = varCell.ContCellType; //read from var cell
				contentCellValueOrOffset = varCell.ContCellValue;

				if (contentCellType == CONTINUE_VAR_TYPE) //CONTINUE VAR =====================================================================
				{
					contentOffset = contentCellValueOrOffset;

					goto NEXT_KEY_PART;
				}

				pContentCellType = &varCell.ContCellType;
				pContentCellValue = &varCell.ContCellValue;
			}
			else
			{
				//update existing value
				varCell.ValueContCellType = VALUE_TYPE;
				varCell.ValueContCellValue = value;

				return false;
			}
		}
		else if (contentCellType == VALUE_TYPE) //update existing value
		{
			if (keyOffset < keyLen)
			{
				VarCell* pVarCell;

				if (countReleasedVarCells)
				{
					uint32_t varOffset = tailReleasedVarOffset;

					pVarCell = &pVarPages[varOffset >> 16]->pVar[varOffset & 0xFFFF];

					tailReleasedVarOffset = pVarCell->ValueContCellValue;

					countReleasedVarCells--;
				}
				else
				{
					VarPage* pVarPage = pVarPages[lastVarOffset >> 16];
					if (!pVarPage)
					{
						pVarPage = new VarPage();
						pVarPages[VarPagesCount++] = pVarPage;

						if (VarPagesCount == VarPagesSize)
						{
							reallocateVarPages();
						}
					}

					pVarCell = &pVarPage->pVar[lastVarOffset & 0xFFFF];
				}

				pVarCell->ValueContCellType = VALUE_TYPE;
				pVarCell->ValueContCellValue = pContentPage->pContent[contentIndex];

				pVarCell->ContCellType = CONTINUE_VAR_TYPE;
				pSetLastContentOffset = &pVarCell->ContCellValue;

				pContentPage->pType[contentIndex] = VAR_TYPE;
				pContentPage->pContent[contentIndex] = lastVarOffset++;

				goto FILL_KEY2;
			}
			else
			{
				*pContentCellValue = value;

				return false;
			}
		}
		else if (keyOffset == keyLen) //STOP =====================================================================
		{
			VarCell* pVarCell;

			if (countReleasedVarCells)
			{
				uint32_t varOffset = tailReleasedVarOffset;

				pVarCell = &pVarPages[varOffset >> 16]->pVar[varOffset & 0xFFFF];

				tailReleasedVarOffset = pVarCell->ValueContCellValue;

				countReleasedVarCells--;
			}
			else
			{
				VarPage* pVarPage = pVarPages[lastVarOffset >> 16];
				if (!pVarPage)
				{
					pVarPage = new VarPage();
					pVarPages[VarPagesCount++] = pVarPage;

					if (VarPagesCount == VarPagesSize)
					{
						reallocateVarPages();
					}
				}

				pVarCell = &pVarPage->pVar[lastVarOffset & 0xFFFF];
			}

			pVarCell->ContCellType = pContentPage->pType[contentIndex];
			pVarCell->ContCellValue = pContentPage->pContent[contentIndex];

			pVarCell->ValueContCellType = VALUE_TYPE;
			pVarCell->ValueContCellValue = value;

			pContentPage->pType[contentIndex] = VAR_TYPE;
			pContentPage->pContent[contentIndex] = lastVarOffset++;

			return true;
		}

		if (contentCellType <= MAX_BRANCH_TYPE1) //BRANCH =====================================================================
		{
			BranchPage* pBranchPage = pBranchPages[contentCellValueOrOffset >> 16];
			BranchCell& branchCell = pBranchPage->pBranch[contentCellValueOrOffset & 0xFFFF];

			//try find value in the list
			for (uint32_t i = 0; i < contentCellType; i++)
			{
				if (branchCell.Values[i] == keyValue)
				{
					contentOffset = branchCell.Offsets[i];
					keyOffset++;

					goto NEXT_KEY_PART;
				}
			}

			//create value in branch
			if (contentCellType < MAX_BRANCH_TYPE1)
			{
				branchCell.Values[contentCellType] = keyValue;
				pSetLastContentOffset = &branchCell.Offsets[contentCellType];

				(*pContentCellType)++;

				goto FILL_KEY;
			}
			else
			{
#ifndef _RELEASE
				tempValues[CONTENT_BRANCH_STAT]--;
				tempValues[MOVES_LEVEL1_STAT]++;
#endif

				//EXTRACT BRANCH AND CREATE BLOCK =========================================================
				uint8_t idxKeyValue = 0;
				uint8_t currContentCellType = MIN_BLOCK_TYPE;

				const uint16_t countCell = BRANCH_ENGINE_SIZE + 1;

			EXTRACT_BRANCH:

				BlockCell blockCells[countCell];
				uint8_t indexes[countCell];

				for (uint32_t i = 0; i < MAX_BRANCH_TYPE1; i++)
				{
					BlockCell& currBlockCell = blockCells[i];
					currBlockCell.Type = CURRENT_VALUE_TYPE;
					currBlockCell.Offset = branchCell.Offsets[i];

					uint32_t& value = branchCell.Values[i];
					currBlockCell.ValueOrOffset = value;

					indexes[i] = ((value << idxKeyValue) >> BLOCK_ENGINE_SHIFT); //((uint8_t*)&value)[idxKeyValue];
				}

				//add current value
				BlockCell& currBlockCell4 = blockCells[BRANCH_ENGINE_SIZE];
				currBlockCell4.Type = CURRENT_VALUE_TYPE;
				//currBlockCell4.Offset = lastContentOffset; //will be set later
				currBlockCell4.ValueOrOffset = keyValue;

				indexes[BRANCH_ENGINE_SIZE] = ((keyValue << idxKeyValue) >> BLOCK_ENGINE_SHIFT);

				//clear map
				uint8_t mapIndexes[BLOCK_ENGINE_SIZE];
				for (uint32_t i = 0; i < countCell; i++)
				{
					mapIndexes[indexes[i]] = 0;
				}

				//fill map
				for (uint32_t i = 0; i < countCell; i++)
				{
					uint8_t& countIndexes = mapIndexes[indexes[i]];
					countIndexes++;

					if (countIndexes > BRANCH_ENGINE_SIZE)
					{
						idxKeyValue += BLOCK_ENGINE_STEP;
						currContentCellType++;
						goto EXTRACT_BRANCH; //use another byte
					}
				}

				//release branch cell
				memset(&branchCell, 0, sizeof(BranchCell));

				branchCell.Values[0] = tailReleasedBranchOffset;

				tailReleasedBranchOffset = contentCellValueOrOffset;

				countReleasedBranchCells++;

				//get free block
				uint32_t startBlockOffset;

				if (countReleasedBlockCells)
				{
					startBlockOffset = tailReleasedBlockOffset;

					tailReleasedBlockOffset = pBlockPages[startBlockOffset >> 16]->pBlock[startBlockOffset & 0xFFFF].Offset;

					countReleasedBlockCells -= BLOCK_ENGINE_SIZE;
				}
				else
				{
					startBlockOffset = lastBlockOffset;

					uint32_t maxLastBlockOffset = startBlockOffset + BLOCK_ENGINE_SIZE * 2;
					if (!pBlockPages[maxLastBlockOffset >> 16])
					{
						pBlockPages[BlockPagesCount++] = new BlockPage();

						if (BlockPagesCount == BlockPagesSize)
						{
							reallocateBlockPages();
						}
					}

					lastBlockOffset += BLOCK_ENGINE_SIZE;
				}

				//fill block
				*pContentCellType = currContentCellType;
				*pContentCellValue = startBlockOffset;

				for (uint32_t i = 0; i < countCell; i++)
				{
					uint8_t idx = indexes[i];
					uint32_t offset = startBlockOffset + idx;

					BlockPage* pBlockPage = pBlockPages[offset >> 16];
					BlockCell& currBlockCell = pBlockPage->pBlock[offset & 0xFFFF];

					uint8_t count = mapIndexes[idx];
					if (count == 1) //one value in block cell
					{
						currBlockCell = blockCells[i];

						if (i == BRANCH_ENGINE_SIZE) //last cell
						{
							pSetLastContentOffset = &currBlockCell.Offset;
						}
					}
					else if (count <= BRANCH_ENGINE_SIZE) //create branch cell
					{
						if (currBlockCell.Type == 0) //create branch
						{
							//get free branch cell
							BranchCell* pCurrBranchCell;
							if (countReleasedBranchCells)
							{
								uint32_t branchOffset = tailReleasedBranchOffset;

								pCurrBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];

								tailReleasedBranchOffset = pCurrBranchCell->Values[0];

								countReleasedBranchCells--;

								currBlockCell.Offset = branchOffset;
							}
							else
							{
								BranchPage* pBranchPage = pBranchPages[lastBranchOffset >> 16];
								if (!pBranchPage)
								{
									pBranchPage = new BranchPage();
									pBranchPages[BranchPagesCount++] = pBranchPage;

									if (BranchPagesCount == BranchPagesSize)
									{
										reallocateBranchPages();
									}
								}

								pCurrBranchCell = &pBranchPage->pBranch[lastBranchOffset & 0xFFFF];
								currBlockCell.Offset = lastBranchOffset++;
							}

							currBlockCell.Type = MIN_BRANCH_TYPE1;

							pCurrBranchCell->Values[0] = blockCells[i].ValueOrOffset;

							if (i < BRANCH_ENGINE_SIZE) //last cell ?
							{
								pCurrBranchCell->Offsets[0] = blockCells[i].Offset;
							}
							else
							{
								pSetLastContentOffset = &pCurrBranchCell->Offsets[0];
							}
						}
						else //types 1..4
						{
							BranchPage* pBranchPage = pBranchPages[currBlockCell.Offset >> 16];
							BranchCell& currBranchCell = pBranchPage->pBranch[currBlockCell.Offset & 0xFFFF];

							currBranchCell.Values[currBlockCell.Type] = blockCells[i].ValueOrOffset;

							if (i < BRANCH_ENGINE_SIZE) //last cell ?
							{
								currBranchCell.Offsets[currBlockCell.Type] = blockCells[i].Offset;
							}
							else
							{
								pSetLastContentOffset = &currBranchCell.Offsets[currBlockCell.Type];
							}

							currBlockCell.Type++;
						}
					}
					else
					{
						printf("FAIL STATE.");
					}
				}

				goto FILL_KEY;
			}
		}
		else if (contentCellType <= MAX_BLOCK_TYPE) //VALUE IN BLOCK ===================================================================
		{
			uint32_t level = 1;

			uint8_t idxKeyValue = (contentCellType - MIN_BLOCK_TYPE) * BLOCK_ENGINE_STEP;

			uint32_t startOffset = contentCellValueOrOffset;

		NEXT_BLOCK:
			uint32_t subOffset = ((keyValue << idxKeyValue) >> BLOCK_ENGINE_SHIFT);
			uint32_t blockOffset = startOffset + subOffset;

			BlockCell& blockCell = pBlockPages[blockOffset >> 16]->pBlock[blockOffset & 0xFFFF];

			uint8_t& blockCellType = blockCell.Type;

			if (blockCellType == EMPTY_TYPE) //release cell, fill
			{
				blockCellType = CURRENT_VALUE_TYPE;
				blockCell.ValueOrOffset = keyValue;
				pSetLastContentOffset = &blockCell.Offset;

				goto FILL_KEY;
			}
			else //block cell
			{
				if (blockCellType == CURRENT_VALUE_TYPE) //current value
				{
					if (blockCell.ValueOrOffset == keyValue) //value is exists
					{
						contentOffset = blockCell.Offset;
						keyOffset++;

						goto NEXT_KEY_PART;
					}
					else //create branch with two values
					{
						blockCellType = MIN_BRANCH_TYPE1 + 1;

						//get free branch cell
						BranchCell* pBranchCell;
						if (countReleasedBranchCells)
						{
							uint32_t branchOffset = tailReleasedBranchOffset;

							pBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];

							tailReleasedBranchOffset = pBranchCell->Values[0];

							countReleasedBranchCells--;

							pBranchCell->Offsets[0] = blockCell.Offset;
							blockCell.Offset = branchOffset;
						}
						else
						{
							BranchPage* pBranchPage = pBranchPages[lastBranchOffset >> 16];
							if (!pBranchPage)
							{
								pBranchPage = new BranchPage();
								pBranchPages[BranchPagesCount++] = pBranchPage;

								if (BranchPagesCount == BranchPagesSize)
								{
									reallocateBranchPages();
								}
							}

							pBranchCell = &pBranchPage->pBranch[lastBranchOffset & 0xFFFF];

							pBranchCell->Offsets[0] = blockCell.Offset;
							blockCell.Offset = lastBranchOffset++;
						}

						pBranchCell->Values[0] = blockCell.ValueOrOffset;

						pBranchCell->Values[1] = keyValue;
						pSetLastContentOffset = &pBranchCell->Offsets[1];

						goto FILL_KEY;
					}
				}
				else if (blockCellType <= MAX_BRANCH_TYPE1) //branch cell 1
				{
					BranchPage* pBranchPage1 = pBranchPages[blockCell.Offset >> 16];
					BranchCell& branchCell1 = pBranchPage1->pBranch[blockCell.Offset & 0xFFFF];

					//first branch
					//try find value in the list
					for (uint32_t i = 0; i < blockCellType; i++)
					{
						if (branchCell1.Values[i] == keyValue)
						{
							contentOffset = branchCell1.Offsets[i];
							keyOffset++;

							goto NEXT_KEY_PART;
						}
					}

					if (blockCellType < MAX_BRANCH_TYPE1)
					{
						branchCell1.Values[blockCellType] = keyValue;
						pSetLastContentOffset = &branchCell1.Offsets[blockCellType];

						blockCellType++;

						goto FILL_KEY;
					}

					//create second branch
					blockCellType = MIN_BRANCH_TYPE2;

					//get free branch cell
					BranchCell* pBranchCell;
					if (countReleasedBranchCells)
					{
						uint32_t branchOffset = tailReleasedBranchOffset;

						pBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];

						tailReleasedBranchOffset = pBranchCell->Values[0];

						countReleasedBranchCells--;

						blockCell.ValueOrOffset = branchOffset;
					}
					else
					{
						BranchPage* pBranchPage = pBranchPages[lastBranchOffset >> 16];
						if (!pBranchPage)
						{
							pBranchPage = new BranchPage();
							pBranchPages[BranchPagesCount++] = pBranchPage;

							if (BranchPagesCount == BranchPagesSize)
							{
								reallocateBranchPages();
							}
						}

						pBranchCell = &pBranchPage->pBranch[lastBranchOffset & 0xFFFF];

						blockCell.ValueOrOffset = lastBranchOffset++;
					}

					pBranchCell->Values[0] = keyValue;
					pSetLastContentOffset = &pBranchCell->Offsets[0];

					goto FILL_KEY;
				}
				else if (blockCellType <= MAX_BRANCH_TYPE2) //branch cell 2
				{
					BranchPage* pBranchPage1 = pBranchPages[blockCell.Offset >> 16];
					BranchCell& branchCell1 = pBranchPage1->pBranch[blockCell.Offset & 0xFFFF];

					//first branch
					//try find value in the list
					for (uint32_t i = 0; i < BRANCH_ENGINE_SIZE; i++)
					{
						if (branchCell1.Values[i] == keyValue)
						{
							contentOffset = branchCell1.Offsets[i];
							keyOffset++;

							goto NEXT_KEY_PART;
						}
					}

					//second branch
					BranchPage* pBranchPage2 = pBranchPages[blockCell.ValueOrOffset >> 16];
					BranchCell& branchCell2 = pBranchPage2->pBranch[blockCell.ValueOrOffset & 0xFFFF];

					//try find value in the list
					uint32_t countValues = blockCellType - MAX_BRANCH_TYPE1;

					for (uint8_t i = 0; i < countValues; i++)
					{
						if (branchCell2.Values[i] == keyValue)
						{
							contentOffset = branchCell2.Offsets[i];
							keyOffset++;

							goto NEXT_KEY_PART;
						}
					}

					//create value in branch
					if (blockCellType < MAX_BRANCH_TYPE2) //add to branch value
					{
						branchCell2.Values[countValues] = keyValue;
						pSetLastContentOffset = &branchCell2.Offsets[countValues];

						blockCellType++;

						goto FILL_KEY;
					}
					else
					{
						//CREATE NEXT BLOCK ==========================================================
#ifndef _RELEASE
						switch (level)
						{
						case 1:
							tempValues[MOVES_LEVEL1_STAT]++;
							break;
						case 2:
							tempValues[MOVES_LEVEL2_STAT]++;
							break;
						case 3:
							tempValues[MOVES_LEVEL3_STAT]++;
							break;
						case 4:
							tempValues[MOVES_LEVEL4_STAT]++;
							break;
						case 5:
							tempValues[MOVES_LEVEL5_STAT]++;
							break;
						case 6:
							tempValues[MOVES_LEVEL6_STAT]++;
							break;
						case 7:
							tempValues[MOVES_LEVEL7_STAT]++;
							break;
						case 8:
							tempValues[MOVES_LEVEL8_STAT]++;
							break;
						default:
							break;
						}
#endif

						const uint16_t branchesSize = BRANCH_ENGINE_SIZE * 2;
						const uint16_t countCell = branchesSize + 1;

						BlockCell blockCells[countCell];
						uint8_t indexes[countCell];

					EXTRACT_BRANCH2:
						idxKeyValue += BLOCK_ENGINE_STEP;
						if (idxKeyValue > BLOCK_ENGINE_SHIFT)
							idxKeyValue = 0;

						//idxKeyValue &= BLOCK_ENGINE_MASK;

						uint8_t j = 0;

						//first branch
						for (uint32_t i = 0; i < BRANCH_ENGINE_SIZE; i++, j++)
						{
							BlockCell& currBlockCell = blockCells[j];
							currBlockCell.Type = CURRENT_VALUE_TYPE;
							currBlockCell.Offset = branchCell1.Offsets[i];

							uint32_t& value = branchCell1.Values[i];
							currBlockCell.ValueOrOffset = value;

							indexes[j] = ((value << idxKeyValue) >> BLOCK_ENGINE_SHIFT);
						}

						//second branch
						for (uint32_t i = 0; i < BRANCH_ENGINE_SIZE; i++, j++)
						{
							BlockCell& currBlockCell = blockCells[j];
							currBlockCell.Type = CURRENT_VALUE_TYPE;
							currBlockCell.Offset = branchCell2.Offsets[i];

							uint32_t& value = branchCell2.Values[i];
							currBlockCell.ValueOrOffset = value;

							indexes[j] = ((value << idxKeyValue) >> BLOCK_ENGINE_SHIFT);
						}

						//add current value
						BlockCell& currBlockCell8 = blockCells[branchesSize];
						currBlockCell8.Type = CURRENT_VALUE_TYPE;
						//currBlockCell8.Offset = lastContentOffset; //will be set later
						currBlockCell8.ValueOrOffset = keyValue;

						indexes[branchesSize] = ((keyValue << idxKeyValue) >> BLOCK_ENGINE_SHIFT);

						//clear map
						uint8_t mapIndexes[BLOCK_ENGINE_SIZE];
						for (uint8_t i = 0; i < countCell; i++)
						{
							mapIndexes[indexes[i]] = 0;
						}

						//fill map
						for (uint32_t i = 0; i < countCell; i++)
						{
							uint8_t& countIndexes = mapIndexes[indexes[i]];
							countIndexes++;

							if (countIndexes > branchesSize)
							{
								goto EXTRACT_BRANCH2; //use another byte
							}
						}

						//release branch cell 1
						memset(&branchCell1, 0, sizeof(BranchCell));

						branchCell1.Values[0] = tailReleasedBranchOffset;

						tailReleasedBranchOffset = blockCell.Offset;
						blockCell.Offset = 0;

						countReleasedBranchCells++;

						//release branch cell 2
						memset(&branchCell2, 0, sizeof(BranchCell));

						branchCell2.Values[0] = tailReleasedBranchOffset;

						tailReleasedBranchOffset = blockCell.ValueOrOffset;
						blockCell.ValueOrOffset = 0;

						countReleasedBranchCells++;

						//get free block
						uint32_t startBlockOffset;

						if (countReleasedBlockCells)
						{
							startBlockOffset = tailReleasedBlockOffset;

							tailReleasedBlockOffset = pBlockPages[startBlockOffset >> 16]->pBlock[startBlockOffset & 0xFFFF].Offset;

							countReleasedBlockCells -= BLOCK_ENGINE_SIZE;
						}
						else
						{
							startBlockOffset = lastBlockOffset;

							uint32_t maxLastBlockOffset = startBlockOffset + BLOCK_ENGINE_SIZE * 2;
							if (!pBlockPages[maxLastBlockOffset >> 16])
							{
								pBlockPages[BlockPagesCount++] = new BlockPage();

								if (BlockPagesCount == BlockPagesSize)
								{
									reallocateBlockPages();
								}
							}

							lastBlockOffset += BLOCK_ENGINE_SIZE;
						}

						//fill block
						blockCellType = MIN_BLOCK_TYPE + (idxKeyValue / BLOCK_ENGINE_STEP);
						blockCell.Offset = startBlockOffset;

						for (uint32_t i = 0; i < countCell; i++)
						{
							uint8_t idx = indexes[i];
							uint32_t offset = startBlockOffset + idx;

							BlockPage* pBlockPage = pBlockPages[offset >> 16];
							BlockCell& currBlockCell = pBlockPage->pBlock[offset & 0xFFFF];

							uint8_t count = mapIndexes[idx];
							if (count == 1) //one value in block cell
							{
								currBlockCell = blockCells[i];

								if (i == branchesSize) //last cell
								{
									pSetLastContentOffset = &currBlockCell.Offset;
								}
							}
							else
							{
								if (currBlockCell.Type == 0) //create branch
								{
									//get free branch cell
									BranchCell* pCurrBranchCell;
									if (countReleasedBranchCells)
									{
										uint32_t branchOffset = tailReleasedBranchOffset;

										pCurrBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];

										tailReleasedBranchOffset = pCurrBranchCell->Values[0];

										countReleasedBranchCells--;

										currBlockCell.Offset = branchOffset;
									}
									else
									{
										BranchPage* pBranchPage = pBranchPages[lastBranchOffset >> 16];
										if (!pBranchPage)
										{
											pBranchPage = new BranchPage();
											pBranchPages[BranchPagesCount++] = pBranchPage;

											if (BranchPagesCount == BranchPagesSize)
											{
												reallocateBranchPages();
											}
										}

										pCurrBranchCell = &pBranchPage->pBranch[lastBranchOffset & 0xFFFF];

										currBlockCell.Offset = lastBranchOffset++;
									}

									currBlockCell.Type = MIN_BRANCH_TYPE1;

									pCurrBranchCell->Values[0] = blockCells[i].ValueOrOffset;

									if (i < branchesSize) //last cell ?
									{
										pCurrBranchCell->Offsets[0] = blockCells[i].Offset;
									}
									else
									{
										pSetLastContentOffset = &pCurrBranchCell->Offsets[0];
									}
								}
								else if (currBlockCell.Type < MAX_BRANCH_TYPE1)
								{
									BranchPage* pBranchPage = pBranchPages[currBlockCell.Offset >> 16];
									BranchCell& currBranchCell = pBranchPage->pBranch[currBlockCell.Offset & 0xFFFF];

									currBranchCell.Values[currBlockCell.Type] = blockCells[i].ValueOrOffset;

									if (i < branchesSize) //last cell ?
									{
										currBranchCell.Offsets[currBlockCell.Type] = blockCells[i].Offset;
									}
									else
									{
										pSetLastContentOffset = &currBranchCell.Offsets[currBlockCell.Type];
									}

									currBlockCell.Type++;
								}
								else if (currBlockCell.Type == MAX_BRANCH_TYPE1)
								{
									//get free branch cell
									BranchCell* pCurrBranchCell;
									if (countReleasedBranchCells)
									{
										uint32_t branchOffset = tailReleasedBranchOffset;

										pCurrBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];

										tailReleasedBranchOffset = pCurrBranchCell->Values[0];

										countReleasedBranchCells--;

										currBlockCell.ValueOrOffset = branchOffset;
									}
									else
									{
										BranchPage* pBranchPage = pBranchPages[lastBranchOffset >> 16];
										if (!pBranchPage)
										{
											pBranchPage = new BranchPage();
											pBranchPages[BranchPagesCount++] = pBranchPage;

											if (BranchPagesCount == BranchPagesSize)
											{
												reallocateBranchPages();
											}
										}

										pCurrBranchCell = &pBranchPage->pBranch[lastBranchOffset & 0xFFFF];

										currBlockCell.ValueOrOffset = lastBranchOffset++;
									}

									currBlockCell.Type = MIN_BRANCH_TYPE2;

									pCurrBranchCell->Values[0] = blockCells[i].ValueOrOffset;

									if (i < branchesSize) //last cell ?
									{
										pCurrBranchCell->Offsets[0] = blockCells[i].Offset;
									}
									else
									{
										pSetLastContentOffset = &pCurrBranchCell->Offsets[0];
									}
								}
								else
								{
									uint32_t countValues = currBlockCell.Type - MAX_BRANCH_TYPE1;

									BranchPage* pBranchPage = pBranchPages[currBlockCell.ValueOrOffset >> 16];
									BranchCell& currBranchCell = pBranchPage->pBranch[currBlockCell.ValueOrOffset & 0xFFFF];

									currBranchCell.Values[countValues] = blockCells[i].ValueOrOffset;

									if (i < branchesSize) //last cell ?
									{
										currBranchCell.Offsets[countValues] = blockCells[i].Offset;
									}
									else
									{
										pSetLastContentOffset = &currBranchCell.Offsets[countValues];
									}

									currBlockCell.Type++;
								}
							}
						}

						goto FILL_KEY;
					}
				}
				else if (blockCell.Type <= MAX_BLOCK_TYPE)
				{
					//go to block
					idxKeyValue = (blockCell.Type - MIN_BLOCK_TYPE) * BLOCK_ENGINE_STEP;
					startOffset = blockCell.Offset;

					level++;

					goto NEXT_BLOCK;
				}
			}
		}
		else if (contentCellType == CURRENT_VALUE_TYPE) //PART OF KEY =========================================================================
		{
			if (contentCellValueOrOffset == keyValue)
			{
				contentOffset++;
				keyOffset++;

				goto NEXT_KEY_PART;
			}
			else //create branch
			{
				*pContentCellType = MIN_BRANCH_TYPE1 + 1;

				//get free branch cell
				BranchCell* pBranchCell;
				if (countReleasedBranchCells)
				{
					uint32_t branchOffset = tailReleasedBranchOffset;

					pBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];

					tailReleasedBranchOffset = pBranchCell->Values[0];

					countReleasedBranchCells--;

					pBranchCell->Values[0] = contentCellValueOrOffset;
					*pContentCellValue = branchOffset;
				}
				else
				{
					BranchPage* pBranchPage = pBranchPages[lastBranchOffset >> 16];
					if (!pBranchPage)
					{
						pBranchPage = new BranchPage();
						pBranchPages[BranchPagesCount++] = pBranchPage;

						if (BranchPagesCount == BranchPagesSize)
						{
							reallocateBranchPages();
						}
					}

					pBranchCell = &pBranchPage->pBranch[lastBranchOffset & 0xFFFF];

					pBranchCell->Values[0] = contentCellValueOrOffset;
					*pContentCellValue = lastBranchOffset++;
				}

				pBranchCell->Offsets[0] = contentOffset + 1;

				pBranchCell->Values[1] = keyValue;
				pSetLastContentOffset = &pBranchCell->Offsets[1];

				goto FILL_KEY;
			}
		}

	FILL_KEY:

		keyOffset++;

	FILL_KEY2:

		uint32_t restKeyLen = keyLen - keyOffset;

		if (tailReleasedContentOffsets[restKeyLen])
		{
			uint32_t startContentOffset = *pSetLastContentOffset = tailReleasedContentOffsets[restKeyLen];

			//fill key
			pContentPage = pContentPages[startContentOffset >> 16];
			contentIndex = startContentOffset & 0xFFFF;

			pContentPage->pType[contentIndex] = ONLY_CONTENT_TYPE + restKeyLen;
			tailReleasedContentOffsets[restKeyLen] = pContentPage->pContent[contentIndex];

			countReleasedContentCells -= (restKeyLen + ValueLen);

			for (; keyOffset < keyLen; contentIndex++, keyOffset++)
			{
				pContentPage->pContent[contentIndex] = key[keyOffset];
			}

			pContentPage->pType[contentIndex] = VALUE_TYPE;
			pContentPage->pContent[contentIndex] = value;

			return true;
		}
		else
		{
			pContentPage = pContentPages[lastContentOffset >> 16];
			contentIndex = lastContentOffset & 0xFFFF;

			if (contentIndex > maxSafeShort) //content in next page
			{
				//release content cells
				uint32_t spaceLen = MAX_SHORT - contentIndex - 1;

				pContentPage->pContent[contentIndex] = tailReleasedContentOffsets[spaceLen];

				tailReleasedContentOffsets[spaceLen] = lastContentOffset;

				countReleasedContentCells += (spaceLen + ValueLen);

				//move to next page
				uint32_t page = (lastContentOffset >> 16) + 1;

				pContentPage = pContentPages[page];

				lastContentOffset = (page << 16);

				contentIndex = 0;
			}

			//set last content offset in cell
			if (pSetLastContentOffset)
			{
				*pSetLastContentOffset = lastContentOffset;
			}

			//fill key
			pContentPage->pType[contentIndex] = ONLY_CONTENT_TYPE + restKeyLen;

			for (; keyOffset < keyLen; contentIndex++, keyOffset++, lastContentOffset++)
			{
				pContentPage->pContent[contentIndex] = key[keyOffset];
			}

			pContentPage->pType[contentIndex] = VALUE_TYPE;
			pContentPage->pContent[contentIndex] = value;

			lastContentOffset++;

			return true;
		}
	}
	catch (...)
	{
		destroy();

		throw;
	}
}
