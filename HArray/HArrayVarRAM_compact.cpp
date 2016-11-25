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
#include "HArrayVarRAM.h"

void HArrayVarRAM::insertBlockValue(BlockPage** pNewBlockPages,
	 							  	uint32& newBlockPagesCount,
					  				uint32& newLastBlockOffset,
					  				uchar8& contentCellType,
					  				uint32& contentCellValue,
					  				uint32 value,
					  				uint32 offset)
{
	uchar8 idxKeyValue = (contentCellType - MIN_BLOCK_TYPE) * BLOCK_ENGINE_STEP;

	uint32 startOffset = contentCellValue;

NEXT_BLOCK:

	uint32 subOffset = ((value << idxKeyValue) >> BLOCK_ENGINE_SHIFT);
	uint32 blockOffset = startOffset + subOffset;

	BlockPage* pBlockPage = pNewBlockPages[blockOffset >> 16];
	if (!pBlockPage)
	{
		pBlockPage = new BlockPage();
		pNewBlockPages[BlockPagesCount++] = pBlockPage;
	}

	BlockCell& blockCell = pBlockPage->pBlock[blockOffset & 0xFFFF];

	uchar8& blockCellType = blockCell.Type;

	if (blockCellType == EMPTY_TYPE) //free cell, fill
	{
		blockCellType = CURRENT_VALUE_TYPE;
		blockCell.ValueOrOffset = value;
		blockCell.Offset = offset;

		return;
	}
	else //block cell
	{
		if (blockCellType == CURRENT_VALUE_TYPE) //current value
		{
			if (blockCell.ValueOrOffset == value) //value is exists
			{
				return;
			}
			else //create branch with two values
			{
				blockCellType = MIN_BRANCH_TYPE1 + 1;

				//get free branch cell
				BranchCell* pBranchCell;
				if (countFreeBranchCell)
				{
					uint32 branchOffset = freeBranchCells[--countFreeBranchCell];
					pBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];

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

				pBranchCell->Values[1] = value;
				pBranchCell->Offsets[1] = offset;

				return;
			}
		}
		else if (blockCellType <= MAX_BRANCH_TYPE1) //branch cell 1
		{
			BranchPage* pBranchPage1 = pBranchPages[blockCell.Offset >> 16];
			BranchCell& branchCell1 = pBranchPage1->pBranch[blockCell.Offset & 0xFFFF];

			//first branch
			//try find value in the list
			for (uint32 i = 0; i < blockCellType; i++)
			{
				if (branchCell1.Values[i] == value) //key exists, return
				{
					return;
				}
			}

			if (blockCellType < MAX_BRANCH_TYPE1)
			{
				branchCell1.Values[blockCellType] = value;
				branchCell1.Offsets[blockCellType] = offset;

				blockCellType++;

				return;
			}

			//create second branch
			blockCellType = MIN_BRANCH_TYPE2;

			//get free branch cell
			BranchCell* pBranchCell;
			if (countFreeBranchCell)
			{
				uint32 branchOffset = freeBranchCells[--countFreeBranchCell];
				pBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];

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

			pBranchCell->Values[0] = value;
			pBranchCell->Offsets[0] = offset;

			return;
		}
		else if (blockCellType <= MAX_BRANCH_TYPE2) //branch cell 2
		{
			BranchPage* pBranchPage1 = pBranchPages[blockCell.Offset >> 16];
			BranchCell& branchCell1 = pBranchPage1->pBranch[blockCell.Offset & 0xFFFF];

			//first branch
			//try find value in the list
			for (uint32 i = 0; i < BRANCH_ENGINE_SIZE; i++)
			{
				if (branchCell1.Values[i] == value) //update exists
				{
					return;
				}
			}

			//second branch
			BranchPage* pBranchPage2 = pBranchPages[blockCell.ValueOrOffset >> 16];
			BranchCell& branchCell2 = pBranchPage2->pBranch[blockCell.ValueOrOffset & 0xFFFF];

			//try find value in the list
			uint32 countValues = blockCellType - MAX_BRANCH_TYPE1;

			for (uchar8 i = 0; i < countValues; i++)
			{
				if (branchCell2.Values[i] == value) //update, exists
				{
					return;
				}
			}

			//create value in branch
			if (blockCellType < MAX_BRANCH_TYPE2) //add to branch value
			{
				branchCell2.Values[countValues] = value;
				branchCell2.Offsets[countValues] = offset;

				blockCellType++;

				return;
			}
			else
			{
				//CREATE NEXT BLOCK ==========================================================

				const ushort16 branchesSize = BRANCH_ENGINE_SIZE * 2;
				const ushort16 countCell = branchesSize + 1;

				BlockCell blockCells[countCell];
				uchar8 indexes[countCell];

				if (countFreeBranchCell < MAX_SHORT)
				{
					freeBranchCells[countFreeBranchCell++] = blockCell.Offset;
				}

				if (countFreeBranchCell < MAX_SHORT)
				{
					freeBranchCells[countFreeBranchCell++] = blockCell.ValueOrOffset;
				}

				EXTRACT_BRANCH2:

				idxKeyValue += BLOCK_ENGINE_STEP;
				if (idxKeyValue > BLOCK_ENGINE_SHIFT)
					idxKeyValue = 0;

				//idxKeyValue &= BLOCK_ENGINE_MASK;

				uchar8 j = 0;

				//first branch
				for (uint32 i = 0; i < BRANCH_ENGINE_SIZE; i++, j++)
				{
					BlockCell& currBlockCell = blockCells[j];
					currBlockCell.Type = CURRENT_VALUE_TYPE;
					currBlockCell.Offset = branchCell1.Offsets[i];

					uint32& value = branchCell1.Values[i];
					currBlockCell.ValueOrOffset = value;

					indexes[j] = ((value << idxKeyValue) >> BLOCK_ENGINE_SHIFT);
				}

				//second branch
				for (uint32 i = 0; i < BRANCH_ENGINE_SIZE; i++, j++)
				{
					BlockCell& currBlockCell = blockCells[j];
					currBlockCell.Type = CURRENT_VALUE_TYPE;
					currBlockCell.Offset = branchCell2.Offsets[i];

					uint32& value = branchCell2.Values[i];
					currBlockCell.ValueOrOffset = value;

					indexes[j] = ((value << idxKeyValue) >> BLOCK_ENGINE_SHIFT);
				}

				//add current value
				BlockCell& currBlockCell8 = blockCells[branchesSize];
				currBlockCell8.Type = CURRENT_VALUE_TYPE;
				currBlockCell8.Offset = offset;
				currBlockCell8.ValueOrOffset = value;

				indexes[branchesSize] = ((value << idxKeyValue) >> BLOCK_ENGINE_SHIFT);

				//clear map
				uchar8 mapIndexes[BLOCK_ENGINE_SIZE];
				for (uchar8 i = 0; i < countCell; i++)
				{
					mapIndexes[indexes[i]] = 0;
				}

				//fill map
				for (uint32 i = 0; i < countCell; i++)
				{
					uchar8& countIndexes = mapIndexes[indexes[i]];
					countIndexes++;

					if (countIndexes > branchesSize)
					{
						goto EXTRACT_BRANCH2; //use another byte
					}
				}

				//allocate page
				uint32 maxLastBlockOffset = newLastBlockOffset + BLOCK_ENGINE_SIZE * 2;
				if (!pNewBlockPages[maxLastBlockOffset >> 16])
				{
					pNewBlockPages[newBlockPagesCount++] = new BlockPage();

					if (BlockPagesCount == BlockPagesSize)
					{
						reallocateBlockPages();
					}
				}

				//fill block
				blockCellType = MIN_BLOCK_TYPE + (idxKeyValue / BLOCK_ENGINE_STEP);
				blockCell.Offset = newLastBlockOffset;

				for (uint32 i = 0; i < countCell; i++)
				{
					uchar8 idx = indexes[i];
					uint32 offset = newLastBlockOffset + idx;

					BlockPage* pBlockPage = pNewBlockPages[offset >> 16];
					BlockCell& currBlockCell = pBlockPage->pBlock[offset & 0xFFFF];

					uchar8 count = mapIndexes[idx];
					if (count == 1) //one value in block cell
					{
						currBlockCell = blockCells[i];
					}
					else
					{
						if (currBlockCell.Type == 0) //create branch
						{
							//get free branch cell
							BranchCell* pCurrBranchCell;
							if (countFreeBranchCell)
							{
								uint32 branchOffset = freeBranchCells[--countFreeBranchCell];
								pCurrBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];

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
							pCurrBranchCell->Offsets[0] = blockCells[i].Offset;
						}
						else if (currBlockCell.Type < MAX_BRANCH_TYPE1)
						{
							BranchPage* pBranchPage = pBranchPages[currBlockCell.Offset >> 16];
							BranchCell& currBranchCell = pBranchPage->pBranch[currBlockCell.Offset & 0xFFFF];

							currBranchCell.Values[currBlockCell.Type] = blockCells[i].ValueOrOffset;
							currBranchCell.Offsets[currBlockCell.Type] = blockCells[i].Offset;

							currBlockCell.Type++;
						}
						else if (currBlockCell.Type == MAX_BRANCH_TYPE1)
						{
							//get free branch cell
							BranchCell* pCurrBranchCell;
							if (countFreeBranchCell)
							{
								uint32 branchOffset = freeBranchCells[--countFreeBranchCell];
								pCurrBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];

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
							pCurrBranchCell->Offsets[0] = blockCells[i].Offset;
						}
						else
						{
							uint32 countValues = currBlockCell.Type - MAX_BRANCH_TYPE1;

							BranchPage* pBranchPage = pBranchPages[currBlockCell.ValueOrOffset >> 16];
							BranchCell& currBranchCell = pBranchPage->pBranch[currBlockCell.ValueOrOffset & 0xFFFF];

							currBranchCell.Values[countValues] = blockCells[i].ValueOrOffset;
							currBranchCell.Offsets[countValues] = blockCells[i].Offset;
							currBlockCell.Type++;
						}
					}
				}

				newLastBlockOffset += BLOCK_ENGINE_SIZE;

				return;
			}
		}
		else if (blockCell.Type <= MAX_BLOCK_TYPE)
		{
			//go to block
			idxKeyValue = (blockCell.Type - MIN_BLOCK_TYPE) * BLOCK_ENGINE_STEP;
			startOffset = blockCell.Offset;

			//level++;

			goto NEXT_BLOCK;
		}
	}
}

bool HArrayVarRAM::scanBlocks(uint32 blockOffset, CompactPage* pCompactPage)
{
	bool isOneBlock = true;

	uint32 maxOffset = blockOffset + BLOCK_ENGINE_SIZE;

	for(uint32 offset = blockOffset; offset < maxOffset; offset++)
	{
		BlockPage* pBlockPage = pBlockPages[offset >> 16];
		BlockCell& blockCell = pBlockPage->pBlock[offset & 0xFFFF];

		uchar8& blockCellType = blockCell.Type;

		if(blockCellType == EMPTY_TYPE)
		{
			continue;
		}
		else if(blockCellType == CURRENT_VALUE_TYPE) //current value
		{
			pCompactPage->Offsets[pCompactPage->Count] = blockCell.ValueOrOffset;
			pCompactPage->Offsets[pCompactPage->Count] = blockCell.Offset;

			pCompactPage->Count++;

			if(pCompactPage->Count == MAX_CHAR)
			{
				pCompactPage = pCompactPage->pNextPage = new CompactPage();
			}
		}
		else if(blockCellType <= MAX_BRANCH_TYPE1) //branch cell
		{
			BranchPage* pBranchPage = pBranchPages[blockCell.Offset >> 16];
			BranchCell& branchCell1 = pBranchPage->pBranch[blockCell.Offset & 0xFFFF];

			//try find value in the list
			for(uint32 i=0; i<blockCellType; i++)
			{
				pCompactPage->Offsets[pCompactPage->Count] = branchCell1.Values[i];
				pCompactPage->Offsets[pCompactPage->Count] = branchCell1.Offsets[i];

				pCompactPage->Count++;

				if(pCompactPage->Count == MAX_CHAR)
				{
					pCompactPage = pCompactPage->pNextPage = new CompactPage();
				}
			}
		}
		else if(blockCellType <= MAX_BRANCH_TYPE2) //branch cell
		{
			BranchPage* pBranchPage1 = pBranchPages[blockCell.Offset >> 16];
			BranchCell branchCell1 = pBranchPage1->pBranch[blockCell.Offset & 0xFFFF];

			//try find value in the list
			for(uint32 i=0; i < BRANCH_ENGINE_SIZE; i++)
			{
				pCompactPage->Offsets[pCompactPage->Count] = branchCell1.Values[i];
				pCompactPage->Offsets[pCompactPage->Count] = branchCell1.Offsets[i];

				pCompactPage->Count++;

				if(pCompactPage->Count == MAX_CHAR)
				{
					pCompactPage = pCompactPage->pNextPage = new CompactPage();
				}
			}

			BranchPage* pBranchPage2 = pBranchPages[blockCell.ValueOrOffset >> 16];
			BranchCell branchCell2 = pBranchPage2->pBranch[blockCell.ValueOrOffset & 0xFFFF];

			//try find value in the list
			uint32 countValues = blockCellType - MAX_BRANCH_TYPE1;

			for(uint32 i=0; i<countValues; i++)
			{
				pCompactPage->Offsets[pCompactPage->Count] = branchCell2.Values[i];
				pCompactPage->Offsets[pCompactPage->Count] = branchCell2.Offsets[i];

				pCompactPage->Count++;

				if(pCompactPage->Count == MAX_CHAR)
				{
					pCompactPage = pCompactPage->pNextPage = new CompactPage();
				}
			}
		}
		else if(blockCellType <= MAX_BLOCK_TYPE)
		{
			isOneBlock = false;

			//go to block
			scanBlocks(blockCell.Offset, pCompactPage);
		}
	}

	return isOneBlock;
}

bool HArrayVarRAM::finHeaderBlockPlace(CompactPage* pRootCompactPage,
									   uint32& headerBlockType,
									   uint32& baseHeaderOffset,
									   uint32& leftShift,
									   uint32& rightShift)
{
	headerBlockType = MIN_HEADER_BLOCK_TYPE;

	//1. Try found without any fails
	for(uint32 bits = 8; bits <= 20; bits+=4)
	{
		for(leftShift = 0; leftShift < 32 - bits; leftShift++, headerBlockType++) //0..71
		{
			rightShift = 32 - bits;

			uint32 headerBlockSize = 1 << bits;

			for(baseHeaderOffset = 0; baseHeaderOffset < HeaderSize - headerBlockSize; baseHeaderOffset++)
			{
				CompactPage* pCompactPage = pRootCompactPage;

				do
				{
					//insert values
					for (uint32 i = 0; i < pCompactPage->Count; i++)
					{
						uint32 headerOffset = pCompactPage->Values[i] << leftShift >> rightShift;

						HeaderCell& headerCell = pHeader[baseHeaderOffset + headerOffset];

						if(headerCell.Type != EMPTY_TYPE)
						{
							goto NOT_FOUND;
						}
					}

					pCompactPage = pCompactPage->pNextPage;
				}
				while (pCompactPage);

				return true; //found !

				NOT_FOUND:;			
			}
		}
	}

	return false;
}

bool HArrayVarRAM::allocateHeaderBlock(uint32 blockOffset, ContentCell& contentCell)
{
	//1. Scan blocks
	CompactPage* pCompactPage = new CompactPage();
	scanBlocks(blockOffset, pCompactPage);

	uint32 headerBlockType;
	uint32 baseHeaderOffset;
	uint32 rightShift;
	uint32 leftShift;

	//2. Find good place for header block
	finHeaderBlockPlace(pCompactPage, headerBlockType, baseHeaderOffset, leftShift, rightShift);

	uchar8 parentID = NewParentID++;

	//3. Write block values
	while (pCompactPage)
	{
		//insert values
		for (uint32 i = 0; i < pCompactPage->Count; i++)
		{
			uint32 headerOffset = pCompactPage->Values[i] << leftShift >> rightShift;

			HeaderCell& headerCell = pHeader[baseHeaderOffset + headerOffset];

			switch (headerCell.Type)
			{
				case EMPTY_TYPE: //fill header cell
				{
					headerCell.Type = HEADER_CURRENT_VALUE_TYPE << 6 | parentID;
					headerCell.Offset = pCompactPage->Offsets[i];
				}
				case HEADER_JUMP_TYPE: //create header branch
				{
					HeaderBranchPage* pHeaderBranchPage = pHeaderBranchPages[lastHeaderBranchOffset >> 16];
					if (!pHeaderBranchPage)
					{
						pHeaderBranchPage = new HeaderBranchPage();
						pHeaderBranchPages[HeaderBranchPagesCount++] = pHeaderBranchPage;

						if (HeaderBranchPagesCount == HeaderBranchPagesSize)
						{
							reallocateHeaderBranchPages();
						}
					}

					HeaderBranchCell& headerBranchCell = pHeaderBranchPage->pHeaderBranch[lastHeaderBranchOffset & 0xFFFF];
					headerBranchCell.HeaderOffset = headerCell.Offset;

					headerBranchCell.ParentIDs[0] = parentID;
					headerBranchCell.Offsets[0] = pCompactPage->Offsets[i];

					headerCell.Type = HEADER_BRANCH_TYPE;
					headerCell.Offset = lastBranchOffset++;
				}
				case HEADER_BRANCH_TYPE: //header branch, check
				{
					HeaderBranchPage* pHeaderBranchPage;
					HeaderBranchCell* pHeaderBranchCell = &pHeaderBranchPages[headerCell.Offset >> 16]->pHeaderBranch[headerCell.Offset & 0xFFFF];

					while(true)
					{
						for (uint32 i = 0; i < BRANCH_ENGINE_SIZE; i++)
						{
							if (pHeaderBranchCell->ParentIDs[i] == 0) //new way
							{
								pHeaderBranchCell->ParentIDs[i] = parentID;
								pHeaderBranchCell->Offsets[i] = pCompactPage->Offsets[i];

								goto FOUND_SLOT;
							}
						}

						if (pHeaderBranchCell->pNextHeaderBranhCell)
						{
							pHeaderBranchCell = pHeaderBranchCell->pNextHeaderBranhCell;
						}
						else
						{
							break;
						}
					} 

					//create new branch
					pHeaderBranchPage = pHeaderBranchPages[lastHeaderBranchOffset >> 16];
					if (!pHeaderBranchPage)
					{
						pHeaderBranchPage = new HeaderBranchPage();
						pHeaderBranchPages[HeaderBranchPagesCount++] = pHeaderBranchPage;

						if (HeaderBranchPagesCount == HeaderBranchPagesSize)
						{
							reallocateHeaderBranchPages();
						}
					}

					pHeaderBranchCell->pNextHeaderBranhCell = &pHeaderBranchPage->pHeaderBranch[lastHeaderBranchOffset & 0xFFFF];
					
					pHeaderBranchCell->pNextHeaderBranhCell->ParentIDs[0] = parentID;
					pHeaderBranchCell->pNextHeaderBranhCell->Offsets[0] = pCompactPage->Offsets[i];

					FOUND_SLOT:;
				}
				default: //HEADER_CURRENT_VALUE_TYPE
				{
					uchar8 currParentID = headerCell.Type << 2 >> 2;

					HeaderBranchPage* pHeaderBranchPage = pHeaderBranchPages[lastHeaderBranchOffset >> 16];
					if (!pHeaderBranchPage)
					{
						pHeaderBranchPage = new HeaderBranchPage();
						pHeaderBranchPages[HeaderBranchPagesCount++] = pHeaderBranchPage;

						if (HeaderBranchPagesCount == HeaderBranchPagesSize)
						{
							reallocateHeaderBranchPages();
						}
					}

					HeaderBranchCell& headerBranchCell = pHeaderBranchPage->pHeaderBranch[lastHeaderBranchOffset & 0xFFFF];
						
					//existing
					headerBranchCell.ParentIDs[0] = currParentID;
					headerBranchCell.Offsets[0] = headerCell.Offset;

					//our new
					headerBranchCell.ParentIDs[1] = parentID;
					headerBranchCell.Offsets[1] = lastContentOffset;

					headerCell.Type = HEADER_BRANCH_TYPE;
					headerCell.Offset = lastBranchOffset++;
				}
			}
		}

		//delete compact page
		CompactPage* pNextCompactPage = pCompactPage->pNextPage;

		delete pCompactPage;

		pCompactPage = pNextCompactPage;
	}

	//4. Set content cell
	contentCell.Type = headerBlockType;
	contentCell.Value = baseHeaderOffset;

	//5. Remove old header blocks

}

void HArrayVarRAM::compact()
{
	CompactPage* pCompactPage = 0;
	BlockPage** pNewBlockPages = new BlockPage*[1024];

	uint32 newBlockPagesCount = 0;
	uint32 newLastBlockOffset = 0;

	//find block entry
	for(uint32 contentOffset = 1; contentOffset < lastContentOffset; contentOffset++)
	{
		ContentCell* pContentCell = &pContentPages[contentOffset >> 16]->pContent[contentOffset & 0xFFFF];

		if(pContentCell->Type == VAR_TYPE)
		{
			VarCell& varCell = pVarPages[pContentCell->Value >> 16]->pVar[pContentCell->Value & 0xFFFF];

			pContentCell = &varCell.ContCell;
		}

		if(MIN_BLOCK_TYPE <= pContentCell->Type && pContentCell->Type <= MAX_BLOCK_TYPE)
		{
			//get all block cells
			if(!pCompactPage)
			{
				pCompactPage = new CompactPage();
			}

			bool isOneBlock = scanBlocks(pContentCell->Value, pCompactPage);

			if(isOneBlock)
			{
				//one block, just move
				uint32 blockOffset = pContentCell->Value;

				pContentCell->Value = newLastBlockOffset;

				uint32 maxOffset = blockOffset + BLOCK_ENGINE_SIZE;

				for(; blockOffset < maxOffset; blockOffset++, newLastBlockOffset++)
				{
					//allocate new block page if need
					BlockPage* pNewBlockPage = pNewBlockPages[newLastBlockOffset >> 16];
					if (!pNewBlockPage)
					{
						pNewBlockPage = new BlockPage();
						pNewBlockPages[newBlockPagesCount++] = pNewBlockPage;
					}

					//copy BlockCell
					pNewBlockPage->pBlock[newLastBlockOffset & 0xFFFF] = pBlockPages[blockOffset >> 16]->pBlock[blockOffset & 0xFFFF];
				}
			}
			else
			{
				//calc optimal size of block	
				uint32 bits = 0;
				uint32 offset = 0;

				/*
				calcOptimalBlockSize(pCompactPage,
									 bits,
									 offset,
									 pContentCell->Type);
				*/

				pContentCell->Value = newLastBlockOffset;

				newLastBlockOffset += (1 << bits);

				//insert value
				while(pCompactPage)
				{
					//insert values
					for(uint32 i=0; i<MAX_CHAR; i++)
					{
						insertBlockValue(pNewBlockPages,
										 newBlockPagesCount,
										 newLastBlockOffset,
										 pContentCell->Type,
										 pContentCell->Value,
										 pCompactPage->Values[i],
										 pCompactPage->Offsets[i]);
					}

					//delete compact page
					CompactPage* pNextCompactPage = pCompactPage->pNextPage;

					delete pCompactPage;

					pCompactPage = pCompactPage->pNextPage;
				}
			}
		}

		//delete old block pages
		uint32 page;
		for(page = 0; page < BlockPagesCount; page++)
		{
			delete pBlockPages[page];
		}

		//allocate enough space
		while(newBlockPagesCount > BlockPagesSize)
		{
			reallocateBlockPages();
		}

		//copy new pages
		for(page = 0; page < newBlockPagesCount; page++)
		{
			pBlockPages[page] = pNewBlockPages[page];
		}

		BlockPagesCount = newBlockPagesCount;

		for(; page < BlockPagesSize; page++)
		{
			pBlockPages[page] = 0;
		}

		delete[] pNewBlockPages;
	}

	return;
}
