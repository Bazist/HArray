
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
# You should have received a copy of the GNU General Public LicenseЕ
# along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "HArray.h"

uint32 HArray::moveContentCells(uint32& startContentOffset,
	ContentPage** newContentPages,
	uint32& countNewContentPages,
	uint32 shrinkLastContentOffset,
	uint32* lastContentOffsetOnNewPages)
{
	uchar8* pSourceStartContentCellType = &pContentPages[startContentOffset >> 16]->pType[startContentOffset & 0xFFFF];
	uint32* pSourceStartContentCellValue = &pContentPages[startContentOffset >> 16]->pContent[startContentOffset & 0xFFFF];

	//identify key len
	uint32 keyLen;

	if (*pSourceStartContentCellType >= ONLY_CONTENT_TYPE)
	{
		keyLen = *pSourceStartContentCellType - ONLY_CONTENT_TYPE;
	}
	else
	{
		//detect key len
		uchar8* pEndContentCellType = pSourceStartContentCellType;
		uint32* pEndContentCellValue = pSourceStartContentCellValue;

		while (true)
		{
			if (*pEndContentCellType == VALUE_TYPE ||
				(MIN_BRANCH_TYPE1 <= *pEndContentCellType && *pEndContentCellType <= MAX_BRANCH_TYPE1) ||
				(MIN_BLOCK_TYPE <= *pEndContentCellType && *pEndContentCellType <= MAX_BLOCK_TYPE))
			{
				break;
			}
			else if (*pEndContentCellType == VAR_TYPE) //check shunt
			{
				VarCell& varCell = pVarPages[*pEndContentCellValue >> 16]->pVar[*pEndContentCellValue & 0xFFFF];

				if (varCell.ContCellType == CONTINUE_VAR_TYPE ||
					varCell.ContCellType == VALUE_TYPE ||
					(MIN_BRANCH_TYPE1 <= varCell.ContCellType && varCell.ContCellType <= MAX_BRANCH_TYPE1) ||
					(MIN_BLOCK_TYPE <= varCell.ContCellType && varCell.ContCellType <= MAX_BLOCK_TYPE))
				{
					break; //end of key
				}
			}

			pEndContentCellType++;
			pEndContentCellValue++;
		}

		keyLen = pEndContentCellValue - pSourceStartContentCellValue;
	}

	uint32 dataLen = (keyLen + ValueLen);

	//get key from pool
	uchar8* pDestStartContentCellType = 0;
	uint32* pDestStartContentCellValue = 0;

	uint32 index;
	uint32 subOffset = 0;

	//key not found, try get key from bigger slots
	for (uint32 currKeyLen = keyLen; currKeyLen < MAX_KEY_SEGMENTS; currKeyLen++)
	{
		while (tailReleasedContentOffsets[keyLen]) //to existing page
		{
			//release content cells
			startContentOffset = tailReleasedContentOffsets[keyLen];

			ContentPage* pContentPage = pContentPages[startContentOffset >> 16];
			
			uchar8& contentCellType = pContentPage->pType[startContentOffset & 0xFFFF];
			uint32& contentCellValue = pContentPage->pContent[startContentOffset & 0xFFFF];

			tailReleasedContentOffsets[keyLen] = contentCellValue;

			countReleasedContentCells -= dataLen;

			if (startContentOffset < shrinkLastContentOffset) //skip spaces on shrink page
			{
				pDestStartContentCellType = &contentCellType;
				pDestStartContentCellValue = &contentCellValue;

				if (keyLen < currKeyLen)
				{
					uint32 len = keyLen + 1;

					releaseContentCells(pDestStartContentCellValue + len,
						startContentOffset + len,
						currKeyLen - len);
				}

				goto MOVE_KEY;
			}
		}
	}

	//slot not found, insert in new page
	//create first new page
	if (!countNewContentPages) //create new page
	{
		newContentPages[countNewContentPages] = new ContentPage();
		lastContentOffsetOnNewPages[countNewContentPages] = 0;

		countNewContentPages++;
	}

	//create next new page
	index = countNewContentPages - 1;

	if (lastContentOffsetOnNewPages[index] + dataLen > MAX_SHORT)
	{
		newContentPages[countNewContentPages] = new ContentPage();
		lastContentOffsetOnNewPages[countNewContentPages] = 0;

		countNewContentPages++;
		index++;
	}

	//modify
	if (index)
	{
		subOffset += index * MAX_SHORT;
	}

	subOffset += lastContentOffsetOnNewPages[index];

	startContentOffset = shrinkLastContentOffset + subOffset;

	pDestStartContentCellType = &newContentPages[index]->pType[lastContentOffsetOnNewPages[index]];
	pDestStartContentCellValue = &newContentPages[index]->pContent[lastContentOffsetOnNewPages[index]];

	lastContentOffsetOnNewPages[index] += dataLen;

MOVE_KEY:

	//copy data
	memcpy(pDestStartContentCellType, pSourceStartContentCellType, dataLen);
	memcpy(pDestStartContentCellValue, pSourceStartContentCellValue, dataLen * sizeof(uint32));
	
	memset(pSourceStartContentCellType, 0, dataLen);
	memset(pSourceStartContentCellValue, 0, dataLen * sizeof(uint32));

	return (keyLen + ValueLen);
}

void HArray::shrinkContentPages()
{
	uint32 amountShrinkPages = countReleasedContentCells / MAX_SHORT;
	uint32 shrinkLastPage = (((lastContentOffset - 1) >> 16) - (amountShrinkPages - 1));
	uint32 shrinkLastContentOffset = shrinkLastPage << 16; //begin of previous page

	uint32 currMovedLen = 0;
	uint32 totalMovedLen = lastContentOffset - shrinkLastContentOffset;

	ContentPage** newContentPages = new ContentPage*[amountShrinkPages];
	memset(newContentPages, 0, amountShrinkPages * sizeof(ContentPage*));

	uint32 countNewContentPages = 0;

	uint32* lastContentOffsetOnNewPages = new uint32[amountShrinkPages];
	memset(lastContentOffsetOnNewPages, 0, amountShrinkPages * sizeof(uint32));

	uint32 lastPage;

	//1. scan header ==============================================================================================
	for (uint32 cell = 0; cell < HeaderSize; cell++)
	{
		uint32 contentOffset = pHeader[cell];

		if (contentOffset >= shrinkLastContentOffset)
		{
			currMovedLen += moveContentCells(contentOffset, //changed
				newContentPages,
				countNewContentPages,
				shrinkLastContentOffset,
				lastContentOffsetOnNewPages);

			if (currMovedLen >= totalMovedLen)
			{
				goto EXIT;
			}
		}
	}

	//2. scan branches =============================================================================================
	lastPage = BranchPagesCount - 1;

	for (uint32 page = 0; page < BranchPagesCount; page++)
	{
		BranchPage* pBranchPage = pBranchPages[page];

		uint32 countCells;

		if (page < lastPage) //not last page
		{
			countCells = MAX_SHORT;
		}
		else //last page
		{
			countCells = ((lastBranchOffset - 1) & 0xFFFF) + 1;
		}

		for (uint32 cell = 0; cell < countCells; cell++)
		{
			BranchCell& branchCell = pBranchPage->pBranch[cell];

			for (uint32 i = 0; i < BRANCH_ENGINE_SIZE; i++)
			{
				if (branchCell.Offsets[i]) //not empty
				{
					if (branchCell.Offsets[i] >= shrinkLastContentOffset)
					{
						currMovedLen += moveContentCells(branchCell.Offsets[i], //changed
							newContentPages,
							countNewContentPages,
							shrinkLastContentOffset,
							lastContentOffsetOnNewPages);

						if (currMovedLen >= totalMovedLen)
						{
							goto EXIT;
						}
					}
				}
				else
				{
					break;
				}
			}
		}
	}

	//3. scan blocks ===============================================================================================
	lastPage = BlockPagesCount - 1;

	for (uint32 page = 0; page < BlockPagesCount; page++)
	{
		BlockPage* pBlockPage = pBlockPages[page];

		uint32 countCells;

		if (page < lastPage) //not last page
		{
			countCells = MAX_SHORT;
		}
		else //last page
		{
			countCells = ((lastBlockOffset - BLOCK_ENGINE_SIZE) & 0xFFFF) + BLOCK_ENGINE_SIZE;
		}

		for (uint32 cell = 0; cell < countCells; cell++)
		{
			BlockCell& blockCell = pBlockPage->pBlock[cell];

			if (blockCell.Type == CURRENT_VALUE_TYPE &&
				blockCell.Offset >= shrinkLastContentOffset)
			{
				currMovedLen += moveContentCells(blockCell.Offset, //changed
					newContentPages,
					countNewContentPages,
					shrinkLastContentOffset,
					lastContentOffsetOnNewPages);

				if (currMovedLen >= totalMovedLen)
				{
					goto EXIT;
				}
			}
		}
	}

	//4. var cells =================================================================================================
	lastPage = VarPagesCount - 1;

	for (uint32 page = 0; page < VarPagesCount; page++)
	{
		VarPage* pVarPage = pVarPages[page];

		uint32 countCells;

		if (page < lastPage) //not last page
		{
			countCells = MAX_SHORT;
		}
		else //last page
		{
			countCells = ((lastVarOffset - 1) & 0xFFFF) + 1;
		}

		for (uint32 cell = 0; cell < countCells; cell++)
		{
			VarCell& varCell = pVarPage->pVar[cell];

			if (varCell.ContCellType == CONTINUE_VAR_TYPE &&
				varCell.ContCellValue >= shrinkLastContentOffset)
			{
				currMovedLen += moveContentCells(varCell.ContCellValue, //changed
					newContentPages,
					countNewContentPages,
					shrinkLastContentOffset,
					lastContentOffsetOnNewPages);

				if (currMovedLen >= totalMovedLen)
				{
					goto EXIT;
				}
			}
		}
	}

	//5. clear shrinked spaces in pool ===========================================================================
	for (uint32 i = 0; i < MAX_KEY_SEGMENTS; i++)
	{
		uint32 contentOffset = tailReleasedContentOffsets[i];
		uint32 prevContentOffset = 0;

		while (contentOffset)
		{
			if (contentOffset < shrinkLastContentOffset) //next
			{
				prevContentOffset = contentOffset;

				contentOffset = pContentPages[contentOffset >> 16]->pContent[contentOffset & 0xFFFF];
			}
			else
			{
				if (contentOffset == tailReleasedContentOffsets[i]) //remove tail
				{
					prevContentOffset = contentOffset;

					contentOffset = tailReleasedContentOffsets[i] = pContentPages[contentOffset >> 16]->pContent[contentOffset & 0xFFFF];
				}
				else //remove in middle
				{
					contentOffset = pContentPages[contentOffset >> 16]->pContent[contentOffset & 0xFFFF];

					pContentPages[prevContentOffset >> 16]->
						pContent[prevContentOffset & 0xFFFF] = contentOffset;
				}

				countReleasedContentCells -= (i + 1);
			}
		}
	}

EXIT:

	//6. remove pages ==============================================================================================
	for (uint32 i = 0, currPage = shrinkLastPage; i < amountShrinkPages; i++, currPage++)
	{
		delete pContentPages[currPage];
		pContentPages[currPage] = 0;
	}

	if (countNewContentPages) //not all content were moved to existing pages
	{
		for (uint32 i = 0, currPage = shrinkLastPage; i < countNewContentPages; i++, currPage++)
		{
			pContentPages[currPage] = newContentPages[i];
		}

		ContentPagesCount = shrinkLastPage + countNewContentPages;

		lastContentOffset = ((ContentPagesCount - 1) << 16) | lastContentOffsetOnNewPages[countNewContentPages - 1];

		notMovedContentCellsAfterLastShrink = lastContentOffset - shrinkLastContentOffset;

		//release rest content cells
		for (uint32 i = 0, currPage = shrinkLastPage; i < countNewContentPages - 1; i++, currPage++)
		{
			uint32 restLen = MAX_SHORT - lastContentOffsetOnNewPages[i];

			if (restLen)
			{
				releaseContentCells(newContentPages[i]->pContent + lastContentOffsetOnNewPages[i],
					(currPage << 16) | lastContentOffsetOnNewPages[i],
					restLen - 1);
			}
		}



	}
	else //all content were moved to existing pages
	{
		notMovedContentCellsAfterLastShrink = 0;

		ContentPagesCount = shrinkLastPage;

		lastContentOffset = shrinkLastContentOffset;
	}
}

void HArray::shrinkBranchPages()
{
	uint32 shrinkLastBranchOffset = lastBranchOffset - countReleasedBranchCells;

	//the are no branches in ha, delete all branch pages
	if (!shrinkLastBranchOffset)
	{
		tailReleasedBranchOffset = 0;
		countReleasedBranchCells = 0;

		for (uint32 page = 0; page < BranchPagesCount; page++)
		{
			if (pBranchPages[page])
			{
				delete pBranchPages[page];

				pBranchPages[page] = 0;
			}
		}

		lastBranchOffset = 0;
		BranchPagesCount = 0;

		return;
	}

	//content ===================================================================================================================
	memset(tailReleasedContentOffsets, 0, sizeof(uint32) * MAX_KEY_SEGMENTS);
	countReleasedContentCells = 0;

	uchar8* pStartReleasedContentCellsType = 0;
	uint32* pStartReleasedContentCellsValue = 0;

	uint32 countReleasedContentCells = 0;
	uint32 startReleasedContentCellsOffset = 0;

	int32 lastPage = ContentPagesCount - 1;

	for (uint32 page = 0; page < ContentPagesCount; page++)
	{
		ContentPage* pContentPage = pContentPages[page];

		uint32 countCells;

		if (page < lastPage) //not last page
		{
			countCells = MAX_SHORT;
		}
		else //last page
		{
			countCells = ((lastContentOffset - 1) & 0xFFFF) + 1;
		}

		for (uint32 cell = (page == 0 ? 1 : 0); cell < countCells; cell++)
		{
			uchar8& contentCellType = pContentPage->pType[cell];
			uint32& contentCellValue = pContentPage->pContent[cell];

			if (MIN_BRANCH_TYPE1 <= contentCellType && contentCellType <= MAX_BRANCH_TYPE1) //in content
			{
				if (contentCellValue >= shrinkLastBranchOffset)
				{
					uint32 newBranchOffset = 0xFFFFFFFF;

					//find free var cell
					while (countReleasedBranchCells)
					{
						countReleasedBranchCells--;

						if (tailReleasedBranchOffset < shrinkLastBranchOffset)
						{
							newBranchOffset = tailReleasedBranchOffset;

							tailReleasedBranchOffset = pBranchPages[tailReleasedBranchOffset >> 16]->pBranch[tailReleasedBranchOffset & 0xFFFF].Values[0];

							break;
						}
						else
						{
							tailReleasedBranchOffset = pBranchPages[tailReleasedBranchOffset >> 16]->pBranch[tailReleasedBranchOffset & 0xFFFF].Values[0];
						}
					}

					if (newBranchOffset == 0xFFFFFFFF)
					{
						printf("\nFAIL STATE (shrinkBranchPages)\n");

						return;
					}

					//get cells
					BranchCell& srcBranchCell = pBranchPages[contentCellValue >> 16]->pBranch[contentCellValue & 0xFFFF];
					BranchCell& destBranchCell = pBranchPages[newBranchOffset >> 16]->pBranch[newBranchOffset & 0xFFFF];

					//copy
					destBranchCell = srcBranchCell;
					memset(&srcBranchCell, 0, sizeof(BranchCell));

					//set content cell
					contentCellValue = newBranchOffset;

					if (!countReleasedBranchCells)
					{
						goto EXIT;
					}
				}
			}
			else if (contentCellType == VAR_TYPE) //shunted in var cell
			{
				VarCell& varCell = pVarPages[contentCellValue >> 16]->pVar[contentCellValue & 0xFFFF];

				if (MIN_BRANCH_TYPE1 <= varCell.ContCellType && varCell.ContCellType <= MAX_BRANCH_TYPE1) //in content
				{
					if (varCell.ContCellValue >= shrinkLastBranchOffset)
					{
						uint32 newBranchOffset = 0xFFFFFFFF;

						//find free var cell
						while (countReleasedBranchCells)
						{
							countReleasedBranchCells--;

							if (tailReleasedBranchOffset < shrinkLastBranchOffset)
							{
								newBranchOffset = tailReleasedBranchOffset;

								tailReleasedBranchOffset = pBranchPages[tailReleasedBranchOffset >> 16]->pBranch[tailReleasedBranchOffset & 0xFFFF].Values[0];

								break;
							}
							else
							{
								tailReleasedBranchOffset = pBranchPages[tailReleasedBranchOffset >> 16]->pBranch[tailReleasedBranchOffset & 0xFFFF].Values[0];
							}
						}

						if (newBranchOffset == 0xFFFFFFFF)
						{
							printf("\nFAIL STATE (shrinkBranchPages)\n");

							return;
						}

						//get cells
						BranchCell& srcBranchCell = pBranchPages[varCell.ContCellValue >> 16]->pBranch[varCell.ContCellValue & 0xFFFF];
						BranchCell& destBranchCell = pBranchPages[newBranchOffset >> 16]->pBranch[newBranchOffset & 0xFFFF];

						//copy
						destBranchCell = srcBranchCell;
						memset(&srcBranchCell, 0, sizeof(BranchCell));

						//set content cell
						varCell.ContCellValue = newBranchOffset;
					}
				}
			}

			//scan released content cells
			if (contentCellType == EMPTY_TYPE)
			{
				if (!pStartReleasedContentCellsType)
				{
					pStartReleasedContentCellsType = &contentCellType;
					pStartReleasedContentCellsValue = &contentCellValue;

					startReleasedContentCellsOffset = (page << 16) | cell;
					countReleasedContentCells = 0;
				}
				else
				{
					countReleasedContentCells++;
				}

				if (countReleasedContentCells == MAX_KEY_SEGMENTS - 1 ||
					cell == MAX_SHORT - 1)
				{
					releaseContentCells(pStartReleasedContentCellsValue,
						startReleasedContentCellsOffset,
						countReleasedContentCells);

					pStartReleasedContentCellsType = 0;
					pStartReleasedContentCellsValue = 0;
				}
			}
			else if (pStartReleasedContentCellsType)
			{
				releaseContentCells(pStartReleasedContentCellsValue,
					startReleasedContentCellsOffset,
					countReleasedContentCells);

				pStartReleasedContentCellsType = 0;
				pStartReleasedContentCellsValue = 0;
			}
		}
	}

	//last release
	if (pStartReleasedContentCellsType)
	{
		releaseContentCells(pStartReleasedContentCellsValue,
							startReleasedContentCellsOffset,
							countReleasedContentCells);

		pStartReleasedContentCellsType = 0;
		pStartReleasedContentCellsValue = 0;
	}

	//blocks ===================================================================================================================
	lastPage = BlockPagesCount - 1;

	for (uint32 page = 0; page < BlockPagesCount; page++)
	{
		BlockPage* pBlockPage = pBlockPages[page];

		uint32 countCells;

		if (page < lastPage) //not last page
		{
			countCells = MAX_SHORT;
		}
		else //last page
		{
			countCells = ((lastBlockOffset - BLOCK_ENGINE_SIZE) & 0xFFFF) + BLOCK_ENGINE_SIZE;
		}

		for (uint32 cell = 0; cell < countCells; cell++)
		{
			BlockCell& blockCell = pBlockPage->pBlock[cell];

			if (blockCell.Type >= MIN_BRANCH_TYPE1)
			{
				if (blockCell.Type <= MAX_BRANCH_TYPE1) //one branch found
				{
					//first barnch
					if (blockCell.Offset >= shrinkLastBranchOffset)
					{
						uint32 newBranchOffset = 0xFFFFFFFF;

						//find free branch cell
						while (countReleasedBranchCells)
						{
							countReleasedBranchCells--;

							if (tailReleasedBranchOffset < shrinkLastBranchOffset)
							{
								newBranchOffset = tailReleasedBranchOffset;

								tailReleasedBranchOffset = pBranchPages[tailReleasedBranchOffset >> 16]->pBranch[tailReleasedBranchOffset & 0xFFFF].Values[0];

								break;
							}
							else
							{
								tailReleasedBranchOffset = pBranchPages[tailReleasedBranchOffset >> 16]->pBranch[tailReleasedBranchOffset & 0xFFFF].Values[0];
							}
						}

						if (newBranchOffset == 0xFFFFFFFF)
						{
							printf("\nFAIL STATE (shrinkBranchPages)\n");

							return;
						}

						//get cells
						BranchCell& srcBranchCell = pBranchPages[blockCell.Offset >> 16]->pBranch[blockCell.Offset & 0xFFFF];
						BranchCell& destBranchCell = pBranchPages[newBranchOffset >> 16]->pBranch[newBranchOffset & 0xFFFF];

						//copy
						destBranchCell = srcBranchCell;
						memset(&srcBranchCell, 0, sizeof(BranchCell));

						//set block cell
						blockCell.Offset = newBranchOffset;

						if (!countReleasedBranchCells)
						{
							goto EXIT;
						}
					}
				}
				else if (blockCell.Type <= MAX_BRANCH_TYPE2) //one branch found
				{
					//first branch
					if (blockCell.Offset >= shrinkLastBranchOffset)
					{
						uint32 newBranchOffset = 0xFFFFFFFF;

						//find free branch cell
						while (countReleasedBranchCells)
						{
							countReleasedBranchCells--;

							if (tailReleasedBranchOffset < shrinkLastBranchOffset)
							{
								newBranchOffset = tailReleasedBranchOffset;

								tailReleasedBranchOffset = pBranchPages[tailReleasedBranchOffset >> 16]->pBranch[tailReleasedBranchOffset & 0xFFFF].Values[0];

								break;
							}
							else
							{
								tailReleasedBranchOffset = pBranchPages[tailReleasedBranchOffset >> 16]->pBranch[tailReleasedBranchOffset & 0xFFFF].Values[0];
							}
						}

						if (newBranchOffset == 0xFFFFFFFF)
						{
							printf("\nFAIL STATE (shrinkBranchPages)\n");

							return;
						}

						//get cells
						BranchCell& srcBranchCell = pBranchPages[blockCell.Offset >> 16]->pBranch[blockCell.Offset & 0xFFFF];
						BranchCell& destBranchCell = pBranchPages[newBranchOffset >> 16]->pBranch[newBranchOffset & 0xFFFF];

						//copy
						destBranchCell = srcBranchCell;
						memset(&srcBranchCell, 0, sizeof(BranchCell));

						//set block cell
						blockCell.Offset = newBranchOffset;

						if (!countReleasedBranchCells)
						{
							goto EXIT;
						}
					}

					//second branch
					if (blockCell.ValueOrOffset >= shrinkLastBranchOffset)
					{
						uint32 newBranchOffset = 0xFFFFFFFF;

						//find free branch cell
						while (countReleasedBranchCells)
						{
							countReleasedBranchCells--;

							if (tailReleasedBranchOffset < shrinkLastBranchOffset)
							{
								newBranchOffset = tailReleasedBranchOffset;

								tailReleasedBranchOffset = pBranchPages[tailReleasedBranchOffset >> 16]->pBranch[tailReleasedBranchOffset & 0xFFFF].Values[0];

								break;
							}
							else
							{
								tailReleasedBranchOffset = pBranchPages[tailReleasedBranchOffset >> 16]->pBranch[tailReleasedBranchOffset & 0xFFFF].Values[0];
							}
						}

						if (newBranchOffset == 0xFFFFFFFF)
						{
							printf("\nFAIL STATE (shrinkBranchPages)\n");

							return;
						}

						//get cells
						BranchCell& srcBranchCell = pBranchPages[blockCell.ValueOrOffset >> 16]->pBranch[blockCell.ValueOrOffset & 0xFFFF];
						BranchCell& destBranchCell = pBranchPages[newBranchOffset >> 16]->pBranch[newBranchOffset & 0xFFFF];

						//copy
						destBranchCell = srcBranchCell;
						memset(&srcBranchCell, 0, sizeof(BranchCell));

						//set block cell
						blockCell.ValueOrOffset = newBranchOffset;

						if (!countReleasedBranchCells)
						{
							goto EXIT;
						}
					}
				}
			}
		}
	}

	//check state
	for (uint32 i = 0; i < countReleasedBranchCells; i++)
	{
		if (tailReleasedBranchOffset < shrinkLastBranchOffset)
		{
			printf("\nFAIL STATE (shrinkBranchPages)\n");

			return;
		}
		else
		{
			tailReleasedBranchOffset = pBranchPages[tailReleasedBranchOffset >> 16]->pBranch[tailReleasedBranchOffset & 0xFFFF].Values[0];
		}
	}

	tailReleasedBranchOffset = 0;
	countReleasedBranchCells = 0;

EXIT:

	//delete shrinked pages
	uint32 startPage = (shrinkLastBranchOffset >> 16) + 1;

	uint32 endPage = (lastBranchOffset >> 16);

	for (uint32 page = startPage; page <= endPage; page++)
	{
		if (pBranchPages[page])
		{
			delete pBranchPages[page];

			pBranchPages[page] = 0;
		}
	}

	lastBranchOffset = shrinkLastBranchOffset;

	BranchPagesCount = startPage;

	return;
}


bool HArray::shrinkBlock(uint32 startBlockOffset,
	uint32 shrinkLastBlockOffset)
{
	//sub blocks ===================================================================================================================
	BlockPage* pBlockPage = pBlockPages[startBlockOffset >> 16];
	uint32 startOffset = startBlockOffset & 0xFFFF;
	uint32 endOffset = startOffset + BLOCK_ENGINE_SIZE;

	for (uint32 cell = startOffset; cell < endOffset; cell++)
	{
		BlockCell& blockCell = pBlockPage->pBlock[cell];

		if (MIN_BLOCK_TYPE <= blockCell.Type && blockCell.Type <= MAX_BLOCK_TYPE) //in block
		{
			if (blockCell.Offset >= shrinkLastBlockOffset)
			{
				uint32 newBlockOffset = 0xFFFFFFFF;

				//find free block cell
				while (countReleasedBlockCells)
				{
					countReleasedBlockCells -= BLOCK_ENGINE_SIZE;

					if (tailReleasedBlockOffset < shrinkLastBlockOffset)
					{
						newBlockOffset = tailReleasedBlockOffset;

						tailReleasedBlockOffset = pBlockPages[tailReleasedBlockOffset >> 16]->pBlock[tailReleasedBlockOffset & 0xFFFF].Offset;

						break;
					}
					else
					{
						tailReleasedBlockOffset = pBlockPages[tailReleasedBlockOffset >> 16]->pBlock[tailReleasedBlockOffset & 0xFFFF].Offset;
					}
				}

				if (newBlockOffset == 0xFFFFFFFF)
				{
					printf("\nFAIL STATE (shrinkBlockPages)\n");

					return false;
				}

				uint32 oldBlockOffset = blockCell.Offset;
				blockCell.Offset = newBlockOffset;

				//get cells
				BlockCell& srcBlockCell = pBlockPages[oldBlockOffset >> 16]->pBlock[oldBlockOffset & 0xFFFF];
				BlockCell& destBlockCell = pBlockPages[newBlockOffset >> 16]->pBlock[newBlockOffset & 0xFFFF];

				//copy data
				memcpy(&destBlockCell, &srcBlockCell, sizeof(BlockCell) * BLOCK_ENGINE_SIZE);
				memset(&srcBlockCell, 0, sizeof(BlockCell) * BLOCK_ENGINE_SIZE);

				if (!countReleasedBlockCells)
				{
					return true;
				}

				//after move block, we need check position and rescan range
				if (newBlockOffset < startBlockOffset)
				{
					if (shrinkBlock(newBlockOffset, shrinkLastBlockOffset))
						return true;
				}
			}
		}
	}

	return false;
}

void HArray::shrinkBlockPages()
{
	uint32 shrinkLastBlockOffset = lastBlockOffset - countReleasedBlockCells;

	//the are no blocks in ha, delete all block pages
	if (!shrinkLastBlockOffset)
	{
		tailReleasedBlockOffset = 0;
		countReleasedBlockCells = 0;

		for (uint32 page = 0; page < BlockPagesCount; page++)
		{
			if (pBlockPages[page])
			{
				delete pBlockPages[page];

				pBlockPages[page] = 0;
			}
		}

		lastBlockOffset = 0;
		BlockPagesCount = 0;

		return;
	}

	//content ===================================================================================================================
	memset(tailReleasedContentOffsets, 0, sizeof(uint32) * MAX_KEY_SEGMENTS);
	countReleasedContentCells = 0;

	uchar8* pStartReleasedContentCellsType = 0;
	uint32* pStartReleasedContentCellsValue = 0;

	uint32 countReleasedContentCells = 0;
	uint32 startReleasedContentCellsOffset = 0;

	int32 lastPage = ContentPagesCount - 1;

	for (uint32 page = 0; page < ContentPagesCount; page++)
	{
		ContentPage* pContentPage = pContentPages[page];

		uint32 countCells;

		if (page < lastPage) //not last page
		{
			countCells = MAX_SHORT;
		}
		else //last page
		{
			countCells = ((lastContentOffset - 1) & 0xFFFF) + 1;
		}

		for (uint32 cell = (page == 0 ? 1 : 0); cell < countCells; cell++)
		{
			uchar8& contentCellType = pContentPage->pType[cell];
			uint32& contentCellValue = pContentPage->pContent[cell];

			if (MIN_BLOCK_TYPE <= contentCellType && contentCellType <= MAX_BLOCK_TYPE) //in content
			{
				if (contentCellValue >= shrinkLastBlockOffset)
				{
					uint32 newBlockOffset = 0xFFFFFFFF;

					//find free block cell
					while (countReleasedBlockCells)
					{
						countReleasedBlockCells -= BLOCK_ENGINE_SIZE;

						if (tailReleasedBlockOffset < shrinkLastBlockOffset)
						{
							newBlockOffset = tailReleasedBlockOffset;

							tailReleasedBlockOffset = pBlockPages[tailReleasedBlockOffset >> 16]->pBlock[tailReleasedBlockOffset & 0xFFFF].Offset;

							break;
						}
						else
						{
							tailReleasedBlockOffset = pBlockPages[tailReleasedBlockOffset >> 16]->pBlock[tailReleasedBlockOffset & 0xFFFF].Offset;
						}
					}

					if (newBlockOffset == 0xFFFFFFFF)
					{
						printf("\nFAIL STATE (shrinkBlockPages)\n");

						return;
					}

					uint32 oldBlockOffset = contentCellValue;
					contentCellValue = newBlockOffset;

					//get cells
					BlockCell& srcBlockCell = pBlockPages[oldBlockOffset >> 16]->pBlock[oldBlockOffset & 0xFFFF];
					BlockCell& destBlockCell = pBlockPages[newBlockOffset >> 16]->pBlock[newBlockOffset & 0xFFFF];

					//copy data
					memcpy(&destBlockCell, &srcBlockCell, sizeof(BlockCell) * BLOCK_ENGINE_SIZE);
					memset(&srcBlockCell, 0, sizeof(BlockCell) * BLOCK_ENGINE_SIZE);
				}
			}

			//scan released content cells
			if (contentCellType == EMPTY_TYPE)
			{
				if (!pStartReleasedContentCellsType)
				{
					pStartReleasedContentCellsType = &contentCellType;
					pStartReleasedContentCellsValue = &contentCellValue;

					startReleasedContentCellsOffset = (page << 16) | cell;
					countReleasedContentCells = 0;
				}
				else
				{
					countReleasedContentCells++;
				}

				if (countReleasedContentCells == MAX_KEY_SEGMENTS - 1 ||
					cell == MAX_SHORT - 1)
				{
					releaseContentCells(pStartReleasedContentCellsValue,
										startReleasedContentCellsOffset,
										countReleasedContentCells);

					pStartReleasedContentCellsType = 0;
					pStartReleasedContentCellsValue = 0;
				}
			}
			else if (pStartReleasedContentCellsType)
			{
				releaseContentCells(pStartReleasedContentCellsValue,
									startReleasedContentCellsOffset,
									countReleasedContentCells);

				pStartReleasedContentCellsType = 0;
				pStartReleasedContentCellsValue = 0;
			}
		}
	}

	//last release
	if (pStartReleasedContentCellsType)
	{
		releaseContentCells(pStartReleasedContentCellsValue,
							startReleasedContentCellsOffset,
							countReleasedContentCells);

		pStartReleasedContentCellsType = 0;
		pStartReleasedContentCellsValue = 0;
	}

	//sub blocks ===================================================================================================================
	lastPage = BlockPagesCount - 1;

	for (uint32 page = 0; page < BlockPagesCount; page++)
	{
		BlockPage* pBlockPage = pBlockPages[page];

		uint32 countCells;

		if (page < lastPage) //not last page
		{
			countCells = MAX_SHORT;
		}
		else //last page
		{
			countCells = ((lastBlockOffset - BLOCK_ENGINE_SIZE) & 0xFFFF) + BLOCK_ENGINE_SIZE;
		}

		for (uint32 cell = 0; cell < countCells; cell++)
		{
			BlockCell& blockCell = pBlockPage->pBlock[cell];

			if (MIN_BLOCK_TYPE <= blockCell.Type && blockCell.Type <= MAX_BLOCK_TYPE) //in block
			{
				if (blockCell.Offset >= shrinkLastBlockOffset)
				{
					uint32 newBlockOffset = 0xFFFFFFFF;

					//find free block cell
					while (countReleasedBlockCells)
					{
						countReleasedBlockCells -= BLOCK_ENGINE_SIZE;

						if (tailReleasedBlockOffset < shrinkLastBlockOffset)
						{
							newBlockOffset = tailReleasedBlockOffset;

							tailReleasedBlockOffset = pBlockPages[tailReleasedBlockOffset >> 16]->pBlock[tailReleasedBlockOffset & 0xFFFF].Offset;

							break;
						}
						else
						{
							tailReleasedBlockOffset = pBlockPages[tailReleasedBlockOffset >> 16]->pBlock[tailReleasedBlockOffset & 0xFFFF].Offset;
						}
					}

					if (newBlockOffset == 0xFFFFFFFF)
					{
						printf("\nFAIL STATE (shrinkBlockPages)\n");

						return;
					}

					uint32 oldBlockOffset = blockCell.Offset;
					blockCell.Offset = newBlockOffset;

					//get cells
					BlockCell& srcBlockCell = pBlockPages[oldBlockOffset >> 16]->pBlock[oldBlockOffset & 0xFFFF];
					BlockCell& destBlockCell = pBlockPages[newBlockOffset >> 16]->pBlock[newBlockOffset & 0xFFFF];

					//copy data
					memcpy(&destBlockCell, &srcBlockCell, sizeof(BlockCell) * BLOCK_ENGINE_SIZE);
					memset(&srcBlockCell, 0, sizeof(BlockCell) * BLOCK_ENGINE_SIZE);

					if (!countReleasedBlockCells)
					{
						goto EXIT;
					}

					//after move block, we need check position and rescan range
					uint32 currBlockOffset = (page << 16) | cell;

					if (newBlockOffset < currBlockOffset)
					{
						if (shrinkBlock(newBlockOffset, shrinkLastBlockOffset))
							goto EXIT;
					}
				}
			}
		}
	}

	//check state
	for (uint32 i = 0; i < countReleasedBlockCells; i += BLOCK_ENGINE_SIZE)
	{
		if (tailReleasedBlockOffset < shrinkLastBlockOffset)
		{
			printf("\nFAIL STATE (shrinkBlockPages)\n");

			return;
		}
		else
		{
			tailReleasedBlockOffset = pBlockPages[tailReleasedBlockOffset >> 16]->pBlock[tailReleasedBlockOffset & 0xFFFF].Offset;
		}
	}

	tailReleasedBlockOffset = 0;
	countReleasedBlockCells = 0;

EXIT:

	//delete shrinked pages
	uint32 startPage = (shrinkLastBlockOffset >> 16) + 1;

	uint32 endPage = (lastBlockOffset >> 16);

	for (uint32 page = startPage; page <= endPage; page++)
	{
		if (pBlockPages[page])
		{
			delete pBlockPages[page];

			pBlockPages[page] = 0;
		}
	}

	lastBlockOffset = shrinkLastBlockOffset;

	BlockPagesCount = startPage;

	return;
}

void HArray::shrinkVarPages()
{
	uint32 shrinkLastVarOffset = lastVarOffset - countReleasedVarCells;

	//the are no vars in ha, delete all var pages
	if (!shrinkLastVarOffset)
	{
		tailReleasedVarOffset = 0;
		countReleasedVarCells = 0;

		for (uint32 page = 0; page < VarPagesCount; page++)
		{
			if (pVarPages[page])
			{
				delete pVarPages[page];

				pVarPages[page] = 0;
			}
		}

		lastVarOffset = 0;
		VarPagesCount = 0;

		return;
	}

	//content ====================================================================================
	memset(tailReleasedContentOffsets, 0, sizeof(uint32) * MAX_KEY_SEGMENTS);
	countReleasedContentCells = 0;

	uchar8* pStartReleasedContentCellsType = 0;
	uint32* pStartReleasedContentCellsValue = 0;

	uint32 countReleasedContentCells = 0;
	uint32 startReleasedContentCellsOffset = 0;
	
	int32 lastPage = ContentPagesCount - 1;

	for (uint32 page = 0; page < ContentPagesCount; page++)
	{
		ContentPage* pContentPage = pContentPages[page];

		uint32 countCells;

		if (page < lastPage) //not last page
		{
			countCells = MAX_SHORT;
		}
		else //last page
		{
			countCells = ((lastContentOffset - 1) & 0xFFFF) + 1;
		}

		for (uint32 cell = (page == 0 ? 1 : 0); cell < countCells; cell++)
		{
			uchar8& contentCellType = pContentPage->pType[cell];
			uint32& contentCellValue = pContentPage->pContent[cell];

			if (contentCellType == VAR_TYPE)
			{
				if (contentCellValue >= shrinkLastVarOffset) //should be moved
				{
					uint32 newVarOffset = 0xFFFFFFFF;

					//find free var cell
					while (countReleasedVarCells)
					{
						countReleasedVarCells--;

						if (tailReleasedVarOffset < shrinkLastVarOffset)
						{
							newVarOffset = tailReleasedVarOffset;

							tailReleasedVarOffset = pVarPages[tailReleasedVarOffset >> 16]->pVar[tailReleasedVarOffset & 0xFFFF].ValueContCellValue;

							break;
						}
						else
						{
							tailReleasedVarOffset = pVarPages[tailReleasedVarOffset >> 16]->pVar[tailReleasedVarOffset & 0xFFFF].ValueContCellValue;
						}
					}

					if (newVarOffset == 0xFFFFFFFF)
					{
						printf("\nFAIL STATE (shrinkVarPages)\n");
					}

					//get cells
					VarCell& srcVarCell = pVarPages[contentCellValue >> 16]->pVar[contentCellValue & 0xFFFF];
					VarCell& destVarCell = pVarPages[newVarOffset >> 16]->pVar[newVarOffset & 0xFFFF];

					//copy
					destVarCell = srcVarCell;
					memset(&srcVarCell, 0, sizeof(VarCell));

					//set content cell
					contentCellValue = newVarOffset;
				}
			}

			//scan released content cells
			if (contentCellType == EMPTY_TYPE)
			{
				if (!pStartReleasedContentCellsType)
				{
					pStartReleasedContentCellsType = &contentCellType;
					pStartReleasedContentCellsValue = &contentCellValue;

					startReleasedContentCellsOffset = (page << 16) | cell;
					countReleasedContentCells = 0;
				}
				else
				{
					countReleasedContentCells++;
				}

				if (countReleasedContentCells == MAX_KEY_SEGMENTS - 1 ||
					cell == MAX_SHORT - 1)
				{
					releaseContentCells(pStartReleasedContentCellsValue,
										startReleasedContentCellsOffset,
										countReleasedContentCells);

					pStartReleasedContentCellsType = 0;
					pStartReleasedContentCellsValue = 0;
				}
			}
			else if (pStartReleasedContentCellsType)
			{
				releaseContentCells(pStartReleasedContentCellsValue,
									startReleasedContentCellsOffset,
									countReleasedContentCells);

				pStartReleasedContentCellsType = 0;
				pStartReleasedContentCellsValue = 0;
			}
		}
	}

	//last release
	if (pStartReleasedContentCellsType)
	{
		releaseContentCells(pStartReleasedContentCellsValue,
							startReleasedContentCellsOffset,
							countReleasedContentCells);

		pStartReleasedContentCellsType = 0;
		pStartReleasedContentCellsValue = 0;
	}

	tailReleasedVarOffset = 0;
	countReleasedVarCells = 0;

EXIT:

	//delete shrinked pages
	uint32 startPage = (shrinkLastVarOffset >> 16) + 1;

	uint32 endPage = (lastVarOffset >> 16);

	for (uint32 page = startPage; page <= endPage; page++)
	{
		if (pVarPages[page])
		{
			delete pVarPages[page];

			pVarPages[page] = 0;
		}
	}

	lastVarOffset = shrinkLastVarOffset;

	VarPagesCount = startPage;

	return;
}

void HArray::shrink()
{
	shrinkContentPages();

	shrinkBranchPages();

	shrinkBlockPages();

	shrinkVarPages();
}