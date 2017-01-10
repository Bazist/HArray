
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
# You should have received a copy of the GNU General Public LicenseЕ
# along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "HArray.h"

uint32 HArray::getFullContentLen(uint32 contentOffset)
{
	ContentCell* pSourceStartContentCell = &pContentPages[contentOffset >> 16]->pContent[contentOffset & 0xFFFF];
	ContentCell* pEndContentCell = pSourceStartContentCell;

	while (true)
	{
		if (pEndContentCell->Type == VALUE_TYPE ||
			(MIN_BRANCH_TYPE1 <= pEndContentCell->Type && pEndContentCell->Type <= MAX_BRANCH_TYPE1) ||
			(MIN_BLOCK_TYPE <= pEndContentCell->Type && pEndContentCell->Type <= MAX_BLOCK_TYPE))
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

		pEndContentCell++;
	}

	return (pEndContentCell - pSourceStartContentCell) + 1;
}

bool HArray::testContentConsistency()
{
	uint32 count = 0;

	//1. scan header ==============================================================================================
	for (uint32 cell = 0; cell < HeaderSize; cell++)
	{
		HeaderCell& headerCell = pHeader[cell];

		if (headerCell.Type == HEADER_JUMP_TYPE)
		{
			count += getFullContentLen(headerCell.Offset);
		}
	}

	//2. scan branches =============================================================================================
	uint32 lastPage = BranchPagesCount - 1;

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
					count += getFullContentLen(branchCell.Offsets[i]);
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
			countCells = ((lastBlockOffset - 1) & 0xFFFF) + 1;
		}

		for (uint32 cell = 0; cell < countCells; cell++)
		{
			BlockCell& blockCell = pBlockPage->pBlock[cell];

			if (blockCell.Type == CURRENT_VALUE_TYPE)
			{
				count += getFullContentLen(blockCell.Offset);
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

			if (varCell.ContCell.Type == CONTINUE_VAR_TYPE)
			{
				count += getFullContentLen(varCell.ContCell.Value);
			}
		}
	}

	return (countReleasedContentCells + count) == (lastContentOffset - 1);
}

bool HArray::testBranchConsistency()
{
	uint32 count = 0;

	//content ===================================================================================================================
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

		for (uint32 cell = 0; cell < countCells; cell++)
		{
			ContentCell& contentCell = pContentPage->pContent[cell];

			if (MIN_BRANCH_TYPE1 <= contentCell.Type && contentCell.Type <= MAX_BRANCH_TYPE1) //in content
			{
				count++;
			}
			else if (contentCell.Type == VAR_TYPE) //shunted in var cell
			{
				VarCell& varCell = pVarPages[contentCell.Value >> 16]->pVar[contentCell.Value & 0xFFFF];

				if (MIN_BRANCH_TYPE1 <= varCell.ContCell.Type && varCell.ContCell.Type <= MAX_BRANCH_TYPE1) //in content
				{
					count++;
				}
			}
		}
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
			countCells = ((lastBlockOffset - 1) & 0xFFFF) + 1;
		}

		for (uint32 cell = 0; cell < countCells; cell++)
		{
			BlockCell& blockCell = pBlockPage->pBlock[cell];

			if (blockCell.Type >= MIN_BRANCH_TYPE1)
			{
				if (blockCell.Type <= MAX_BRANCH_TYPE1) //one branch found
				{
					count++;
				}
				else if (blockCell.Type <= MAX_BRANCH_TYPE2) //one branch found
				{
					count+=2;
				}
			}
		}
	}

	return (count + countReleasedBranchCells) == lastBranchOffset;
}

bool HArray::testBlockConsistency()
{
	uint32 count = 0;

	//content ===================================================================================================================
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

		for (uint32 cell = 0; cell < countCells; cell++)
		{
			ContentCell& contentCell = pContentPage->pContent[cell];

			if (MIN_BLOCK_TYPE <= contentCell.Type && contentCell.Type <= MAX_BLOCK_TYPE) //in content
			{
				count++;
			}
		}
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
			countCells = ((lastBlockOffset - 1) & 0xFFFF) + 1;
		}

		for (uint32 cell = 0; cell < countCells; cell++)
		{
			BlockCell& blockCell = pBlockPage->pBlock[cell];

			if (MIN_BLOCK_TYPE <= blockCell.Type && blockCell.Type <= MAX_BLOCK_TYPE) //in block
			{
				count++;
			}
		}
	}

	return ((count * BLOCK_ENGINE_SIZE + countReleasedBlockCells) == lastBlockOffset);
}

bool HArray::testVarConsistency()
{
	uint32 count = 0;

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

		for (uint32 cell = 0; cell < countCells; cell++)
		{
			ContentCell& contentCell = pContentPage->pContent[cell];

			if (contentCell.Type == VAR_TYPE)
			{
				count++;
			}
		}
	}

	return (count + countReleasedVarCells) == lastVarOffset;
}

