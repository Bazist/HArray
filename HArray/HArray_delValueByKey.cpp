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
2. List of release branches
3. List of release blocks
4. List of varbranches

Stats

*/

void HArray::releaseContentCells(ContentCell* pContentCell, uint32 contentOffset, uint32 keyLen)
{
	printf("releaseContentCells => %u\n", keyLen);

	memset(pContentCell, 0, sizeof(ContentCell));

	pContentCell->Value = tailReleasedContentOffsets[keyLen];

	tailReleasedContentOffsets[keyLen] = contentOffset;

	countReleasedContentCells += (keyLen + ValueLen);
}

void HArray::releaseBranchCell(BranchCell* pBranchCell, uint32 branchOffset)
{
	printf("releaseBranchCell => 1\n");

	memset(pBranchCell, 0, sizeof(BranchCell));

	pBranchCell->Values[0] = tailReleasedBranchOffset;

	tailReleasedBranchOffset = branchOffset;

	countReleasedBranchCells++;
}

void HArray::releaseBlockCell(BlockCell* pBlockCell, uint32 startBlockOffset)
{
	printf("releaseBlockCell => 16\n");

	memset(pBlockCell, 0, sizeof(BlockCell));

	pBlockCell->Offset = tailReleasedBlockOffset;

	tailReleasedBlockOffset = startBlockOffset;

	countReleasedBlockCells += BLOCK_ENGINE_SIZE;
}

void HArray::releaseVarCell(VarCell* pVarCell, uint32 varOffset)
{
	printf("releaseVarCell => 1\n");

	memset(pVarCell, 0, sizeof(VarCell));

	pVarCell->ValueContCell.Value = tailReleasedVarOffset;

	tailReleasedVarOffset = varOffset;

	countReleasedVarCells++;
}

uint32 HArray::moveContentCells(uint32& startContentOffset,
	ContentPage** pNewContentPage,
	uint32 shrinkLastContentOffset,
	uint32& lastContentOffsetOnNewPage)
{
	ContentCell* pSourceStartContentCell = &pContentPages[startContentOffset >> 16]->pContent[startContentOffset & 0xFFFF];

	//identify key len
	uint32 keyLen;

	if (pSourceStartContentCell->Type >= ONLY_CONTENT_TYPE)
	{
		keyLen = pSourceStartContentCell->Type - ONLY_CONTENT_TYPE;
	}
	else
	{
		//detect key len
		ContentCell* pEndContentCell = pSourceStartContentCell;

		while (true)
		{
			if (pEndContentCell->Type == VALUE_TYPE)
			{
				break;
			}
			else if (pEndContentCell->Type == VAR_TYPE) //check shunt
			{
				VarCell& varCell = pVarPages[pEndContentCell->Value >> 16]->pVar[pEndContentCell->Value & 0xFFFF];

				if (varCell.ContCell.Type == CONTINUE_VAR_TYPE)
				{
					break; //end of key
				}
			}
		}

		keyLen = pEndContentCell - pSourceStartContentCell - 1;
	}

	//get key from pool
	ContentCell* pDestStartContentCell = 0;

	while (tailReleasedContentOffsets[keyLen]) //to existing page
	{
		//release content cells
		startContentOffset = tailReleasedContentOffsets[keyLen];

		ContentPage* pContentPage = pContentPages[startContentOffset >> 16];
		ContentCell& contentCell = pContentPage->pContent[startContentOffset & 0xFFFF];

		tailReleasedContentOffsets[keyLen] = contentCell.Value;

		countReleasedContentCells -= (keyLen + ValueLen);

		if (startContentOffset < shrinkLastContentOffset) //skip spaces on shrink page
		{
			pDestStartContentCell = &contentCell;

			break;
		}
	}

	if (!pDestStartContentCell)
	{
		//to new page
		if (!(*pNewContentPage)) //create new page
		{
			*pNewContentPage = new ContentPage();

			lastContentOffsetOnNewPage = 0;
		}

		startContentOffset = shrinkLastContentOffset + lastContentOffsetOnNewPage;

		pDestStartContentCell = &(*pNewContentPage)->pContent[startContentOffset & 0xFFFF];

		lastContentOffsetOnNewPage += (keyLen + ValueLen);

		//copy data
		for (uint32 i = 0; i <= keyLen; i++, pSourceStartContentCell++, pDestStartContentCell++)
		{
			*pDestStartContentCell = *pSourceStartContentCell;
		}
	}

	return (keyLen + ValueLen);
}

void HArray::shrinkContentPages()
{
	uint32 shrinkLastPage = ((lastContentOffset >> 16) - 1);
	uint32 shrinkLastContentOffset = shrinkLastPage << 16; //begin of previous page

	uint32 currMovedLen = 0;
	uint32 totalMovedLen = lastContentOffset - shrinkLastContentOffset;

	ContentPage* pNewContentPage = 0;
	uint32 lastContentOffsetOnNewPage = 0;

	uint32 lastPage;

	//1. scan header ==============================================================================================
	for (uint32 cell = 0; cell < HeaderSize; cell++)
	{
		HeaderCell& headerCell = pHeader[cell];

		if (headerCell.Offset >= shrinkLastContentOffset &&
			headerCell.Type == HEADER_JUMP_TYPE)
		{
			currMovedLen += moveContentCells(headerCell.Offset, //changed
				&pNewContentPage,
				shrinkLastContentOffset,
				lastContentOffsetOnNewPage);

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
		BranchPage* pBranchPage = pBranchPages[page >> 16];

		uint32 countCells;

		if (page < lastPage) //not last page
		{
			countCells = MAX_SHORT;
		}
		else //last page
		{
			countCells = (lastBranchOffset & 0xFFFF) + 1;
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
							&pNewContentPage,
							shrinkLastContentOffset,
							lastContentOffsetOnNewPage);

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
		BlockPage* pBlockPage = pBlockPages[page >> 16];

		uint32 countCells;

		if (page < lastPage) //not last page
		{
			countCells = MAX_SHORT;
		}
		else //last page
		{
			countCells = (lastBlockOffset & 0xFFFF) + 1;
		}

		for (uint32 cell = 0; cell < countCells; cell++)
		{
			BlockCell& blockCell = pBlockPage->pBlock[cell];

			if (blockCell.Type == CURRENT_VALUE_TYPE &&
				blockCell.Offset >= shrinkLastContentOffset)
			{
				currMovedLen += moveContentCells(blockCell.Offset, //changed
												 &pNewContentPage,
												 shrinkLastContentOffset,
												 lastContentOffsetOnNewPage);

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
		VarPage* pVarPage = pVarPages[page >> 16];

		uint32 countCells;

		if (page < lastPage) //not last page
		{
			countCells = MAX_SHORT;
		}
		else //last page
		{
			countCells = (lastBranchOffset & 0xFFFF) + 1;
		}

		for (uint32 cell = 0; cell < countCells; cell++)
		{
			VarCell& varCell = pVarPage->pVar[cell];

			if (varCell.ContCell.Type == CONTINUE_VAR_TYPE &&
				varCell.ContCell.Value >= shrinkLastContentOffset)
			{
				currMovedLen += moveContentCells(varCell.ContCell.Value, //changed
												 &pNewContentPage,
												 shrinkLastContentOffset,
												 lastContentOffsetOnNewPage);

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
				contentOffset = pContentPages[contentOffset >> 16]->pContent[contentOffset & 0xFFFF].Value;
			}
			else
			{
				if (contentOffset == tailReleasedContentOffsets[i]) //remove tail
				{
					contentOffset = tailReleasedContentOffsets[i] = pContentPages[contentOffset >> 16]->pContent[contentOffset & 0xFFFF].Value;
				}
				else //remove in middle
				{
					pContentPages[prevContentOffset >> 16]->pContent[prevContentOffset & 0xFFFF].Value = pContentPages[contentOffset >> 16]->pContent[contentOffset & 0xFFFF].Value;
				}

				countReleasedContentCells -= (i + 1);
			}

			prevContentOffset = contentOffset;
		}
	}
	
EXIT:
	//6. remove pages ==============================================================================================
	if (pContentPages[shrinkLastPage])
	{
		delete pContentPages[shrinkLastPage];
	}

	if (pContentPages[shrinkLastPage + 1])
	{
		delete pContentPages[shrinkLastPage + 1];
	}

	if (pNewContentPage) //not all content were moved to existing pages
	{
		pContentPages[shrinkLastPage] = pNewContentPage;

		lastContentOffset = (shrinkLastPage << 16) + lastContentOffsetOnNewPage;
	}
	else //all content were moved to existing pages
	{
		lastContentOffset = (shrinkLastPage << 16);
	}
}

void HArray::shrinkBranchPages()
{
	uint32 shrinkLastBranchOffset = lastBranchOffset - countReleasedBranchCells;

	//content ===================================================================================================================
	int32 lastPage = ContentPagesCount - 1;

	for (uint32 page = 0; page < ContentPagesCount; page++)
	{
		ContentPage* pContentPage = pContentPages[page >> 16];

		uint32 countCells;

		if (page < lastPage) //not last page
		{
			countCells = MAX_SHORT;
		}
		else //last page
		{
			countCells = (lastContentOffset & 0xFFFF) + 1;
		}

		for (uint32 cell = 0; cell < countCells; cell++)
		{
			ContentCell& contentCell = pContentPage->pContent[cell];

			if (MIN_BRANCH_TYPE1 <= contentCell.Type && contentCell.Type <= MAX_BRANCH_TYPE1) //in content
			{
				if (contentCell.Value >= shrinkLastBranchOffset)
				{
					uint32 newBranchOffset;

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

					//get cells
					BranchCell& srcBranchCell = pBranchPages[contentCell.Value >> 16]->pBranch[contentCell.Value & 0xFFFF];
					BranchCell& destBranchCell = pBranchPages[newBranchOffset >> 16]->pBranch[newBranchOffset & 0xFFFF];

					//copy
					destBranchCell = srcBranchCell;

					//set content cell
					contentCell.Value = newBranchOffset;

					if (!countReleasedBranchCells)
					{
						goto EXIT;
					}
				}
			}
			else if (contentCell.Type == VAR_TYPE) //shunted in var cell
			{
				VarCell& varCell = pVarPages[contentCell.Value >> 16]->pVar[contentCell.Value & 0xFFFF];

				if (MIN_BRANCH_TYPE1 <= varCell.ContCell.Type && varCell.ContCell.Type <= MAX_BRANCH_TYPE1) //in content
				{
					if (varCell.ContCell.Value >= shrinkLastBranchOffset)
					{
						uint32 newBranchOffset;

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

						//get cells
						BranchCell& srcBranchCell = pBranchPages[varCell.ContCell.Value >> 16]->pBranch[varCell.ContCell.Value & 0xFFFF];
						BranchCell& destBranchCell = pBranchPages[newBranchOffset >> 16]->pBranch[newBranchOffset & 0xFFFF];

						//copy
						destBranchCell = srcBranchCell;

						//set content cell
						varCell.ContCell.Value = newBranchOffset;

						if (!countReleasedBranchCells)
						{
							goto EXIT;
						}
					}
				}
			}
		}
	}

	//blocks ===================================================================================================================
	lastPage = BlockPagesCount - 1;

	for (uint32 page = 0; page < BlockPagesCount; page++)
	{
		BlockPage* pBlockPage = pBlockPages[page >> 16];

		uint32 countCells;

		if (page < lastPage) //not last page
		{
			countCells = MAX_SHORT;
		}
		else //last page
		{
			countCells = (lastBlockOffset & 0xFFFF) + 1;
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
						uint32 newBranchOffset;

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

						//get cells
						BranchCell& srcBranchCell = pBranchPages[blockCell.Offset >> 16]->pBranch[blockCell.Offset & 0xFFFF];
						BranchCell& destBranchCell = pBranchPages[newBranchOffset >> 16]->pBranch[newBranchOffset & 0xFFFF];

						//copy
						destBranchCell = srcBranchCell;

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
						uint32 newBranchOffset;

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

						//get cells
						BranchCell& srcBranchCell = pBranchPages[blockCell.Offset >> 16]->pBranch[blockCell.Offset & 0xFFFF];
						BranchCell& destBranchCell = pBranchPages[newBranchOffset >> 16]->pBranch[newBranchOffset & 0xFFFF];

						//copy
						destBranchCell = srcBranchCell;

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
						uint32 newBranchOffset;

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

						//get cells
						BranchCell& srcBranchCell = pBranchPages[blockCell.ValueOrOffset >> 16]->pBranch[blockCell.ValueOrOffset & 0xFFFF];
						BranchCell& destBranchCell = pBranchPages[newBranchOffset >> 16]->pBranch[newBranchOffset & 0xFFFF];

						//copy
						destBranchCell = srcBranchCell;

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

EXIT:
	//max one page we can delete
	BranchPagesCount = (shrinkLastBranchOffset >> 16) + 1;

	if (pBranchPages[BranchPagesCount])
	{
		delete pBranchPages[BranchPagesCount];

		pBranchPages[BranchPagesCount] = 0;
	}

	lastBranchOffset = shrinkLastBranchOffset;
}

void HArray::shrinkBlockPages()
{
	uint32 shrinkLastBlockOffset = lastVarOffset - countReleasedBlockCells;

	//content ===================================================================================================================
	int32 lastPage = ContentPagesCount - 1;

	for (uint32 page = 0; page < ContentPagesCount; page++)
	{
		ContentPage* pContentPage = pContentPages[page >> 16];

		uint32 countCells;

		if (page < lastPage) //not last page
		{
			countCells = MAX_SHORT;
		}
		else //last page
		{
			countCells = (lastContentOffset & 0xFFFF) + 1;
		}

		for (uint32 cell = 0; cell < countCells; cell++)
		{
			ContentCell& contentCell = pContentPage->pContent[cell];

			if (MIN_BLOCK_TYPE <= contentCell.Type && contentCell.Type <= MAX_BLOCK_TYPE) //in content
			{
				if (contentCell.Value >= shrinkLastBlockOffset)
				{
					uint32 newBlockOffset;

					//find free block cell
					while (countReleasedBlockCells)
					{
						countReleasedBlockCells--;

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

					uint32 oldBlockOffset = contentCell.Value;
					contentCell.Value = newBlockOffset;

					for (uint32 i = 0; i < BLOCK_ENGINE_SIZE; i++, oldBlockOffset++, newBlockOffset++)
					{
						//get cells
						BlockCell& srcBlockCell = pBlockPages[oldBlockOffset >> 16]->pBlock[oldBlockOffset & 0xFFFF];

						BlockCell& destBlockCell = pBlockPages[newBlockOffset >> 16]->pBlock[newBlockOffset & 0xFFFF];

						//copy
						destBlockCell = srcBlockCell;
					}

					if (!countReleasedBranchCells)
					{
						goto EXIT;
					}
				}
			}
		}
	}

	//sub blocks ===================================================================================================================
	lastPage = BlockPagesCount - 1;

	for (uint32 page = 0; page < BlockPagesCount; page++)
	{
		BlockPage* pBlockPage = pBlockPages[page >> 16];

		uint32 countCells;

		if (page < lastPage) //not last page
		{
			countCells = MAX_SHORT;
		}
		else //last page
		{
			countCells = (lastBlockOffset & 0xFFFF) + 1;
		}

		for (uint32 startCell = 0; startCell < countCells; startCell += BLOCK_ENGINE_SIZE)
		{
			bool isEmptyBlock = true;

			for (uint32 cell = startCell; cell < BLOCK_ENGINE_SIZE; cell++)
			{
				if (!pBlockPage->pBlock[cell].Type)
				{
					isEmptyBlock = false;

					break;
				}
			}

			if (!isEmptyBlock)
			{
				for (uint32 cell = startCell; cell < BLOCK_ENGINE_SIZE; cell++)
				{
					BlockCell& blockCell = pBlockPage->pBlock[cell];

					if (MIN_BLOCK_TYPE <= blockCell.Type && blockCell.Type <= MAX_BLOCK_TYPE) //in block
					{
						if (blockCell.Offset >= shrinkLastBlockOffset)
						{
							uint32 newBlockOffset;

							//find free block cell
							while (countReleasedBlockCells)
							{
								countReleasedBlockCells--;

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

							uint32 oldBlockOffset = blockCell.Offset;
							blockCell.Offset = newBlockOffset;

							for (uint32 i = 0; i < BLOCK_ENGINE_SIZE; i++, oldBlockOffset++, newBlockOffset++)
							{
								//get cells
								BlockCell& srcBlockCell = pBlockPages[oldBlockOffset >> 16]->pBlock[oldBlockOffset & 0xFFFF];

								BlockCell& destBlockCell = pBlockPages[newBlockOffset >> 16]->pBlock[newBlockOffset & 0xFFFF];

								//copy
								destBlockCell = srcBlockCell;
							}

							if (!countReleasedBranchCells)
							{
								goto EXIT;
							}
						}
					}
				}
			}
		}
	}

EXIT:
	//max one page we can delete
	BlockPagesCount = (shrinkLastBlockOffset >> 16) + 1;

	if (pBlockPages[BlockPagesCount])
	{
		delete pBlockPages[BlockPagesCount];

		pBlockPages[BlockPagesCount] = 0;
	}

	lastBlockOffset = shrinkLastBlockOffset;
}

void HArray::shrinkVarPages()
{
	uint32 shrinkLastVarOffset = lastVarOffset - countReleasedVarCells;

	int32 lastPage = ContentPagesCount - 1;

	for (uint32 page = 0; page < ContentPagesCount; page++)
	{
		ContentPage* pContentPage = pContentPages[page >> 16];

		uint32 countCells;

		if (page < lastPage) //not last page
		{
			countCells = MAX_SHORT;
		}
		else //last page
		{
			countCells = (lastContentOffset & 0xFFFF) + 1;
		}

		for (uint32 cell = 0; cell < countCells; cell++)
		{
			ContentCell& contentCell = pContentPage->pContent[cell];

			if (contentCell.Type == VAR_TYPE)
			{
				if (contentCell.Value >= shrinkLastVarOffset) //should be moved
				{
					uint32 newVarOffset;

					//find free var cell
					while (countReleasedVarCells)
					{
						countReleasedVarCells--;

						if (tailReleasedVarOffset < shrinkLastVarOffset)
						{
							newVarOffset = tailReleasedVarOffset;

							tailReleasedVarOffset = pVarPages[tailReleasedVarOffset >> 16]->pVar[tailReleasedVarOffset & 0xFFFF].ValueContCell.Value;

							break;
						}
						else
						{
							tailReleasedVarOffset = pVarPages[tailReleasedVarOffset >> 16]->pVar[tailReleasedVarOffset & 0xFFFF].ValueContCell.Value;
						}
					}

					//get cells
					VarCell& srcVarCell = pVarPages[contentCell.Value >> 16]->pVar[contentCell.Value & 0xFFFF];
					VarCell& destVarCell = pVarPages[newVarOffset >> 16]->pVar[newVarOffset & 0xFFFF];

					//copy
					destVarCell = srcVarCell;

					//set content cell
					contentCell.Value = newVarOffset;

					if (!countReleasedVarCells)
					{
						goto EXIT;
					}
				}
			}
		}
	}

EXIT:
	//max one page we can delete
	VarPagesCount = (shrinkLastVarOffset >> 16) + 1;

	if (pVarPages[VarPagesCount])
	{
		delete pVarPages[VarPagesCount];

		pVarPages[VarPagesCount] = 0;
	}

	lastVarOffset = shrinkLastVarOffset;
}

bool HArray::tryReleaseBlock(SegmentPath* path, uint32 pathLen, int32& currPathLen)
{
	//check block ==============================================================
	//scan values in block

	SegmentPath& sp = path[currPathLen];

	uint32 count = 0;

	uint32 values[BRANCH_ENGINE_SIZE * 2];
	uint32 offsets[BRANCH_ENGINE_SIZE * 2];

	uint32 subOffset = 0;
	BlockCell* pCurrBlockCell = sp.pBlockCell - sp.BlockSubOffset;

	for (; subOffset < BLOCK_ENGINE_SIZE; subOffset++, pCurrBlockCell++)
	{
		if (pCurrBlockCell->Type)
		{
			if (pCurrBlockCell->Type == CURRENT_VALUE_TYPE)
			{
				values[count] = pCurrBlockCell->ValueOrOffset;
				offsets[count] = pCurrBlockCell->Offset;

				count++;

				if (count > BRANCH_ENGINE_SIZE * 2) //leave block, do nothing, exit
				{
					return false;
				}
			}
			else
			{
				if (count + pCurrBlockCell->Type > BRANCH_ENGINE_SIZE * 2) //leave block, do nothing, exit
				{
					return false;
				}

				if (pCurrBlockCell->Type <= MAX_BRANCH_TYPE1)
				{
					BranchPage* pBranchPage1 = pBranchPages[pCurrBlockCell->Offset >> 16];
					BranchCell branchCell1 = pBranchPage1->pBranch[pCurrBlockCell->Offset & 0xFFFF];

					for (uint32 i = 0; i < pCurrBlockCell->Type; i++, count++)
					{
						values[count] = branchCell1.Values[i];
						offsets[count] = branchCell1.Offsets[i];
					}
				}
				else if (pCurrBlockCell->Type <= MAX_BRANCH_TYPE2)
				{
					BranchPage* pBranchPage1 = pBranchPages[pCurrBlockCell->Offset >> 16];
					BranchCell branchCell1 = pBranchPage1->pBranch[pCurrBlockCell->Offset & 0xFFFF];

					for (uint32 i = 0; i < BRANCH_ENGINE_SIZE; i++, count++)
					{
						values[count] = branchCell1.Values[i];
						offsets[count] = branchCell1.Offsets[i];
					}

					BranchPage* pBranchPage2 = pBranchPages[pCurrBlockCell->ValueOrOffset >> 16];
					BranchCell branchCell2 = pBranchPage2->pBranch[pCurrBlockCell->ValueOrOffset & 0xFFFF];

					uint32 countValues = pCurrBlockCell->Type - MAX_BRANCH_TYPE1;

					for (uint32 i = 0; i < countValues; i++, count++)
					{
						values[count] = branchCell2.Values[i];
						offsets[count] = branchCell2.Offsets[i];
					}
				}

				count += pCurrBlockCell->Type;
			}
		}
	}

	//if less than 8 values ===================================================================================
	SegmentPath& prevSP = path[currPathLen - 1];

	if (prevSP.Type != BLOCK_OFFSET_SEGMENT_TYPE) //top block
	{
		if (count == 0)
		{
			releaseBlockCell(sp.pBlockCell, sp.StartBlockOffset);

			return dismantlingContentCells(path, currPathLen);
		}
		else if (count == 1) //remove block, create CURRENT_VALUE_TYPE
		{
			//if last element offset+1, inject to content;
			if (offsets[0] == sp.ContentOffset + 1)
			{
				sp.pContentCell->Type = CURRENT_VALUE_TYPE;
				sp.pContentCell->Value = values[0];

				releaseBlockCell(sp.pBlockCell, sp.StartBlockOffset);
			}
		}
		else if (count <= BRANCH_ENGINE_SIZE) //remove block, create BRANCH
		{
			//create branch ==================================================================================
			uint32 branchOffset;
			BranchCell* pBranchCell;

			if (countReleasedBranchCells)
			{
				branchOffset = tailReleasedBranchOffset;

				pBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];

				tailReleasedBranchOffset = pBranchCell->Values[0];

				countReleasedBranchCells--;
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

				branchOffset = lastBranchOffset;
				pBranchCell = &pBranchPage->pBranch[lastBranchOffset & 0xFFFF];
			}

			//fill branch
			for (uint32 i = 0; i < count; i++)
			{
				pBranchCell->Offsets[i] = offsets[i];
				pBranchCell->Values[i] = values[i];
			}

			//set content cell
			sp.pContentCell->Type = MIN_BRANCH_TYPE1 + count - 1;
			sp.pContentCell->Value = branchOffset;

			//release block cell
			releaseBlockCell(sp.pBlockCell, sp.StartBlockOffset);
		}
	}
	else //sub block
	{
		if (count == 0)
		{
			prevSP.pBlockCell->Type = 0;
			prevSP.pBlockCell->ValueOrOffset = 0;
			prevSP.pBlockCell->Offset = 0;

			releaseBlockCell(sp.pBlockCell, sp.StartBlockOffset);

			return true;
		}
		else if (count == 1) //remove block, create CURRENT_VALUE_TYPE
		{
			prevSP.pBlockCell->Type = CURRENT_VALUE_TYPE;
			prevSP.pBlockCell->ValueOrOffset = values[0];
			prevSP.pBlockCell->Offset = offsets[0];

			releaseBlockCell(sp.pBlockCell, sp.StartBlockOffset);
		}
		else if (count <= BRANCH_ENGINE_SIZE) //remove block, create BRANCH
		{
			//create branch ==================================================================================
			//get free branch cell
			uint32 branchOffset;
			BranchCell* pBranchCell;

			if (countReleasedBranchCells)
			{
				branchOffset = tailReleasedBranchOffset;

				pBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];

				tailReleasedBranchOffset = pBranchCell->Values[0];

				countReleasedBranchCells--;
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

				branchOffset = lastBranchOffset;
				pBranchCell = &pBranchPage->pBranch[lastBranchOffset & 0xFFFF];
			}

			//fill branch
			for (uint32 i = 0; i < count; i++)
			{
				pBranchCell->Offsets[i] = offsets[i];
				pBranchCell->Values[i] = values[i];
			}

			//set content cell
			prevSP.pBlockCell->Type = MIN_BRANCH_TYPE1 + count - 1;
			prevSP.pBlockCell->ValueOrOffset = branchOffset;

			//release block
			releaseBlockCell(sp.pBlockCell, sp.StartBlockOffset);
		}
		else if (count <= BRANCH_ENGINE_SIZE * 2) //remove block, create two BRANCHES
		{
			//create branch 1 ==================================================================================
			//get free branch cell
			uint32 branchOffset1;
			BranchCell* pBranchCell1;

			if (countReleasedBranchCells)
			{
				branchOffset1 = tailReleasedBranchOffset;

				pBranchCell1 = &pBranchPages[branchOffset1 >> 16]->pBranch[branchOffset1 & 0xFFFF];

				tailReleasedBranchOffset = pBranchCell1->Values[0];

				countReleasedBranchCells--;
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

				branchOffset1 = lastBranchOffset;
				pBranchCell1 = &pBranchPage->pBranch[lastBranchOffset & 0xFFFF];
			}

			//fill branch
			uint32 i = 0;
			for (; i < BLOCK_ENGINE_SIZE; i++)
			{
				pBranchCell1->Offsets[i] = offsets[i];
				pBranchCell1->Values[i] = values[i];
			}

			//create branch 2 ================================================================================
			//get free branch cell
			uint32 branchOffset2;
			BranchCell* pBranchCell2;

			if (countReleasedBranchCells)
			{
				branchOffset2 = tailReleasedBranchOffset;

				pBranchCell2 = &pBranchPages[branchOffset2 >> 16]->pBranch[branchOffset2 & 0xFFFF];

				tailReleasedBranchOffset = pBranchCell2->Values[0];

				countReleasedBranchCells--;
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

				branchOffset2 = lastBranchOffset;
				pBranchCell2 = &pBranchPage->pBranch[lastBranchOffset & 0xFFFF];
			}

			//fill branch
			for (int j = 0; i < count; i++, j++)
			{
				pBranchCell2->Offsets[j] = offsets[i];
				pBranchCell2->Values[j] = values[i];
			}

			//set content cell
			prevSP.pBlockCell->Type = MIN_BRANCH_TYPE1 + count - 1;
			prevSP.pBlockCell->Offset = branchOffset1;
			prevSP.pBlockCell->ValueOrOffset = branchOffset2;

			//release block
			releaseBlockCell(sp.pBlockCell, sp.StartBlockOffset);
		}
	}

	return false;
}

bool HArray::dismantlingContentCells(SegmentPath* path, int32& currPathLen)
{
	//release contents
	uint32 contentOffsetLen = 0;

	while (true)
	{
		//just remove
		path[currPathLen].pContentCell->Type = 0;
		path[currPathLen].pContentCell->Value = 0;

		if (currPathLen > 0)
		{
			if (path[currPathLen - 1].Type == CURRENT_VALUE_SEGMENT_TYPE)
			{
				currPathLen--;
				contentOffsetLen++;
			}
			else
			{
				releaseContentCells(path[currPathLen].pContentCell,
									path[currPathLen].ContentOffset,
									contentOffsetLen);

				return false;
			}
		}
		else
		{
			releaseContentCells(path[currPathLen].pContentCell,
								path[currPathLen].ContentOffset,
								contentOffsetLen);

			return true;
		}
	}

	return true;
}

bool HArray::dismantling(SegmentPath* path, uint32 pathLen)
{
	//DISMANTLING =============================================================================================
	for (int32 currPathLen = pathLen - 1; currPathLen >= 0; currPathLen--)
	{
		SegmentPath& sp = path[currPathLen];

		switch (sp.Type)
		{
		case CURRENT_VALUE_SEGMENT_TYPE:
		{
			if(dismantlingContentCells(path, currPathLen))
			{
				return true;
			}
			else
			{
				break;
			}
		}

		case BRANCH_SEGMENT_TYPE:
		{
			if (sp.pContentCell->Type > MIN_BRANCH_TYPE1) //not last way
			{
				//remove item
				uint32 lastIndex = sp.pContentCell->Type - 1;

				sp.pBranchCell1->Values[sp.BranchIndex] = sp.pBranchCell1->Values[lastIndex];
				sp.pBranchCell1->Offsets[sp.BranchIndex] = sp.pBranchCell1->Offsets[lastIndex];

				sp.pContentCell->Type--;

				if (sp.pContentCell->Type == MIN_BRANCH_TYPE1) //last way is original ?
				{
					if (sp.pBranchCell1->Offsets[0] == sp.ContentOffset + 1)
					{
						sp.pContentCell->Type = CURRENT_VALUE_TYPE;
						sp.pContentCell->Value = sp.pBranchCell1->Values[0];

						releaseBranchCell(sp.pBranchCell1, sp.BranchOffset1);
					}
				}

				return false;
			}
			else //last way
			{
				//release branch
				releaseBranchCell(sp.pBranchCell1, sp.BranchOffset1);

				if(dismantlingContentCells(path, currPathLen))
				{
					return true;
				}
				else
				{
					break;
				}
			}
		}

		case BLOCK_VALUE_SEGMENT_TYPE:
		{
			sp.pBlockCell->Type = 0;
			sp.pBlockCell->Offset = 0;
			sp.pBlockCell->ValueOrOffset = 0;

			if (tryReleaseBlock(path, pathLen, currPathLen))
			{
				break;
			}
			else
			{
				return false;
			}
		}

		case BLOCK_BRANCH1_SEGMENT_TYPE:
		{
			//remove item
			if (sp.pBlockCell->Type <= MAX_BRANCH_TYPE1) //in one branch
			{
				uint32 lastIndex = sp.pBlockCell->Type - 1;

				sp.pBranchCell1->Values[sp.BranchIndex] = sp.pBranchCell1->Values[lastIndex];
				sp.pBranchCell1->Offsets[sp.BranchIndex] = sp.pBranchCell1->Offsets[lastIndex];

				sp.pBlockCell->Type--;

				if (sp.pBlockCell->Type == MIN_BRANCH_TYPE1) //only one item, inject to block
				{
					sp.pBlockCell->Type = CURRENT_VALUE_TYPE;
					sp.pBlockCell->ValueOrOffset = sp.pBranchCell1->Values[0];

					releaseBranchCell(sp.pBranchCell1, sp.BranchOffset1);
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
				if (sp.pBlockCell->Type < MIN_BRANCH_TYPE2) //not last item in branch2
				{
					releaseBranchCell(sp.pBranchCell2, sp.BranchOffset2);
				}
			}

			return false;
		}

		case BLOCK_BRANCH2_SEGMENT_TYPE:
		{
			if (sp.pBlockCell->Type > MIN_BRANCH_TYPE2) //not last item
			{
				uint32 lastIndex = sp.pBlockCell->Type - MAX_BRANCH_TYPE1 - 1;

				sp.pBranchCell2->Values[sp.BranchIndex] = sp.pBranchCell2->Values[lastIndex];
				sp.pBranchCell2->Offsets[sp.BranchIndex] = sp.pBranchCell2->Offsets[lastIndex];
			}
			else //last item
			{
				releaseBranchCell(sp.pBranchCell2, sp.BranchOffset2);
			}

			sp.pBlockCell->Type--;

			return false;
		}

		case BLOCK_OFFSET_SEGMENT_TYPE:
		{
			break; //skip
		}

		case VAR_SHUNT_SEGMENT_TYPE:
		{
			if (sp.pVarCell->ValueContCell.Type)
			{
				//delete continue way, inject value
				*sp.pContentCell = sp.pVarCell->ValueContCell;

				releaseVarCell(sp.pVarCell, sp.VarOffset);

				return false;
			}
			else
			{
				//delete continue way, value was deleted earlier

				releaseVarCell(sp.pVarCell, sp.VarOffset);

				break;
			}
		}

		case VAR_VALUE_SEGMENT_TYPE:
		{
			if (sp.pVarCell->ContCell.Type == CONTINUE_VAR_TYPE)
			{
				if (sp.pVarCell->ValueContCell.Type)
				{
					sp.pVarCell->ValueContCell.Type = 0;

					return false;
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

				releaseVarCell(sp.pVarCell, sp.VarOffset);

				return false;
			}
		}

		default: //fail state
			break;
		}
	}

	return true;
}

bool HArray::delValueByKey(uint32* key,
						   uint32 keyLen)
{
	//EXTRACT PATH =============================================================================================

	SegmentPath path[128];
	int32 pathLen = 0;

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
			uint32 restKeyLen = keyLen - keyOffset;

			if (restKeyLen != (contentCellType - ONLY_CONTENT_TYPE))
			{
				return false;
			}

			uint32 origKeyOffset = keyOffset;

			uint32 origContentIndex = contentIndex;

			for (; keyOffset < keyLen; contentIndex++, keyOffset++)
			{
				if (pContentPage->pContent[contentIndex].Value != key[keyOffset])
					return false;
			}

			//remove
			releaseContentCells(&contentCell, contentOffset, restKeyLen);

			for (; origKeyOffset <= keyLen; origContentIndex++, origKeyOffset++)
			{
				pContentPage->pContent[origContentIndex].Type = 0;
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
				//save path
				SegmentPath& sp = path[pathLen++];
				sp.Type = CURRENT_VALUE_SEGMENT_TYPE;
				sp.pContentCell = &contentCell;
				sp.ContentOffset = contentOffset;

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

			for (uint32 i = 0; i < contentCellType; i++)
			{
				if (values[i] == keyValue)
				{
					//save path
					SegmentPath& sp = path[pathLen++];
					sp.Type = BRANCH_SEGMENT_TYPE;
					sp.pContentCell = &contentCell;
					sp.pBranchCell1 = &branchCell;
					sp.BranchOffset1 = *pContentCellValueOrOffset;
					sp.BranchIndex = i;
					sp.ContentOffset = contentOffset;

					contentOffset = branchCell.Offsets[i];
					keyOffset++;

					goto NEXT_KEY_PART;
				}
			}

			return 0;
		}
		else if (contentCellType == VALUE_TYPE)
		{
			if (keyOffset == keyLen)
			{
				//save path
				SegmentPath& sp = path[pathLen++];
				sp.Type = CURRENT_VALUE_SEGMENT_TYPE;
				sp.pContentCell = &contentCell;
				sp.ContentOffset = contentOffset;

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
					//save path
					SegmentPath& sp = path[pathLen++];
					sp.Type = BLOCK_VALUE_SEGMENT_TYPE;
					sp.pContentCell = &contentCell;
					sp.pBlockCell = &blockCell;
					sp.StartBlockOffset = startOffset;
					sp.ContentOffset = contentOffset;
					sp.BlockSubOffset = subOffset;
					//sp.pBranchCell = 0;
					//sp.Index = i;

					contentOffset = blockCell.Offset;
					keyOffset++;

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
				for (uint32 i = 0; i < blockCellType; i++)
				{
					if (branchCell1.Values[i] == keyValue)
					{
						//save path
						SegmentPath& sp = path[pathLen++];
						sp.Type = BLOCK_BRANCH1_SEGMENT_TYPE;
						sp.pContentCell = &contentCell;
						sp.pBlockCell = &blockCell;
						sp.StartBlockOffset = startOffset;
						sp.pBranchCell1 = &branchCell1;
						sp.BranchOffset1 = blockCell.Offset;
						sp.BranchIndex = i;
						sp.BlockSubOffset = subOffset;

						contentOffset = branchCell1.Offsets[i];
						keyOffset++;

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
						sp.BlockSubOffset = subOffset;

						contentOffset = branchCell1.Offsets[i];
						keyOffset++;

						goto NEXT_KEY_PART;
					}
				}

				//try find value in the list
				uint32 countValues = blockCellType - MAX_BRANCH_TYPE1;

				for (uint32 i = 0; i < countValues; i++)
				{
					if (branchCell2.Values[i] == keyValue)
					{
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
						sp.BlockSubOffset = subOffset;

						contentOffset = branchCell2.Offsets[i];
						keyOffset++;

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
				//save path
				SegmentPath& sp = path[pathLen++];
				sp.Type = CURRENT_VALUE_SEGMENT_TYPE;
				sp.pContentCell = &contentCell;
				sp.ContentOffset = contentOffset;

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

	//DISMANTLING ====================================================================================
DISMANTLING:

	if (dismantling(path, pathLen))
	{
		headerCell.Type = 0;
		headerCell.Offset = 0;
	}

	//SHRINK =============================================================================================
	if (countReleasedContentCells > MAX_COUNT_RELEASED_CONTENT_CELLS)
	{
		shrinkContentPages();
	}

	if (countReleasedBranchCells > MAX_COUNT_RELEASED_BRANCH_CELLS)
	{
		shrinkBranchPages();
	}

	if (countReleasedBlockCells > MAX_COUNT_RELEASED_BLOCK_CELLS)
	{
		shrinkBlockPages();
	}

	if (countReleasedVarCells > MAX_COUNT_RELEASED_VAR_CELLS)
	{
		shrinkVarPages();
	}

	return true;
}
