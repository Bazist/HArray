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

CompactPage* HArray::scanBlocks(uint32& count, uint32 blockOffset, CompactPage* pCompactPage)
{
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
			pCompactPage->Values[pCompactPage->Count] = blockCell.ValueOrOffset;
			pCompactPage->Offsets[pCompactPage->Count] = blockCell.Offset;

			pCompactPage->Count++;
			count++;

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
				pCompactPage->Values[pCompactPage->Count] = branchCell1.Values[i];
				pCompactPage->Offsets[pCompactPage->Count] = branchCell1.Offsets[i];

				pCompactPage->Count++;
				count++;

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
				pCompactPage->Values[pCompactPage->Count] = branchCell1.Values[i];
				pCompactPage->Offsets[pCompactPage->Count] = branchCell1.Offsets[i];

				pCompactPage->Count++;
				count++;

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
				pCompactPage->Values[pCompactPage->Count] = branchCell2.Values[i];
				pCompactPage->Offsets[pCompactPage->Count] = branchCell2.Offsets[i];

				pCompactPage->Count++;
				count++;

				if(pCompactPage->Count == MAX_CHAR)
				{
					pCompactPage = pCompactPage->pNextPage = new CompactPage();
				}
			}
		}
		else if(blockCellType <= MAX_BLOCK_TYPE)
		{
			//go to block
			pCompactPage = scanBlocks(count, blockCell.Offset, pCompactPage);
		}
	}

	return pCompactPage;
}

bool HArray::finHeaderBlockPlace(CompactPage* pRootCompactPage,
									   uint32 count,
									   uchar8 parentID,
									   uint32& headerBlockType,
									   uint32& baseHeaderOffset,
									   uint32& leftShift,
									   uint32& rightShift)
{
	
	uchar8 headerCellType = HEADER_CURRENT_VALUE_TYPE << 6 | parentID;

	uint32 minBits;
	for(minBits = 8; minBits <= 20; minBits+=4)
	{
		uint32 headerBlockSize = 1 << minBits;

		if(count < headerBlockSize)
			break;
	}

	//1. Try found without any fails
	for(baseHeaderOffset = 0; baseHeaderOffset < HeaderSize - (1 << 20); baseHeaderOffset++)
	{
		headerBlockType = MIN_HEADER_BLOCK_TYPE;

		for(uint32 bits = minBits; bits <= 20; bits+=4)
		{
			rightShift = 32 - bits;

			for(leftShift = 0; leftShift <= 32 - bits; leftShift++, headerBlockType++) //0..71
			{
				CompactPage* pCompactPage = pRootCompactPage;

				do
				{
					//insert values
					for (uint32 i = 0; i < pCompactPage->Count; i++)
					{
						uint32 headerOffset = pCompactPage->Values[i] << leftShift >> rightShift;

						HeaderCell& headerCell = pHeader[baseHeaderOffset + headerOffset];

						if(headerCell.Type == EMPTY_TYPE) //fill
						{
							headerCell.Type = headerCellType;
							headerCell.Offset = pCompactPage->Offsets[i];
						}
						else //clear all previous filled
						{
							pCompactPage = pRootCompactPage;

							do
							{
								//insert values
								for (uint32 i = 0; i < pCompactPage->Count; i++)
								{
									uint32 headerOffset = pCompactPage->Values[i] << leftShift >> rightShift;

									HeaderCell& headerCell = pHeader[baseHeaderOffset + headerOffset];

									if(headerCell.Type == headerCellType) //clear
									{
										headerCell.Type = EMPTY_TYPE;
										headerCell.Offset = 0;
									}
									else //stop
									{
										goto NOT_FOUND;
									}
								}

								pCompactPage = pCompactPage->pNextPage;
							}
							while (pCompactPage);
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

bool HArray::allocateHeaderBlock(uint32 keyValue,
									   uint32 keyOffset,
									   ContentCell* pContentCell)
{
	return false;

	//1. Scan blocks
	CompactPage* pCompactPage = new CompactPage();

	pCompactPage->Values[0] = keyValue;
	pCompactPage->Offsets[0] = keyOffset;
	pCompactPage->Count = 1;

	uint32 count = 1;

	scanBlocks(count, pContentCell->Value, pCompactPage);

	//return false;

	uint32 headerBlockType;
	uint32 baseHeaderOffset;
	uint32 rightShift;
	uint32 leftShift;

	uchar8 parentID = ++NewParentID;

	if(NewParentID == 64)
	{
		NewParentID = 1;
	}

	//2. Find good place for header block
	if(!finHeaderBlockPlace(pCompactPage,
							count,
							parentID,
							headerBlockType,
							baseHeaderOffset,
							leftShift,
							rightShift))
	{
		return false;
	}

	//3. Delete compact pages
	while (pCompactPage)
	{
		//delete compact page
		CompactPage* pNextCompactPage = pCompactPage->pNextPage;

		delete pCompactPage;

		pCompactPage = pNextCompactPage;
	}

	/*
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

					break;
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

					break;
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

					break;
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

					break;
				}
			}
		}

		//delete compact page
		CompactPage* pNextCompactPage = pCompactPage->pNextPage;

		delete pCompactPage;

		pCompactPage = pNextCompactPage;
	}
	*/

	//4. Set content cell
	pContentCell->Type = headerBlockType;
	pContentCell->Value = baseHeaderOffset;

	//5. Remove old header blocks

	return true;
}