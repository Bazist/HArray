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

				//delete compact pages
				while(pCompactPage)
				{
					CompactPage* pNextCompactPage = pCompactPage->pNextPage;

					delete pCompactPage;

					pCompactPage = pNextCompactPage;
				}
			}
		}

		//delete old block pages

		//copy pages
	}

	return;
}
