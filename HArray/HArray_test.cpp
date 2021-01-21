
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

uint32 HArray::getFullContentLen(uint32 contentOffset)
{
	uchar8* pSourceStartContentCellType = &pContentPages[contentOffset >> 16]->pType[contentOffset & 0xFFFF];
	uint32* pSourceStartContentCellValue = &pContentPages[contentOffset >> 16]->pContent[contentOffset & 0xFFFF];

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

	return (pEndContentCellValue - pSourceStartContentCellValue) + 1;
}

bool HArray::testContentConsistency()
{
	uint32 count = 0;

	//1. scan header ==============================================================================================
	for (uint32 cell = 0; cell < HeaderSize; cell++)
	{
		uint32 contentOffset = pHeader[cell];

		if (contentOffset)
		{
			count += getFullContentLen(contentOffset);
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
			countCells = ((lastBlockOffset - BLOCK_ENGINE_SIZE) & 0xFFFF) + BLOCK_ENGINE_SIZE;
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

			if (varCell.ContCellType == CONTINUE_VAR_TYPE)
			{
				count += getFullContentLen(varCell.ContCellValue);
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
			uchar8& contentCellType = pContentPage->pType[cell];
			uint32& contentCellValue = pContentPage->pContent[cell];

			if (MIN_BRANCH_TYPE1 <= contentCellType && contentCellType <= MAX_BRANCH_TYPE1) //in content
			{
				count++;
			}
			else if (contentCellType == VAR_TYPE) //shunted in var cell
			{
				VarCell& varCell = pVarPages[contentCellValue >> 16]->pVar[contentCellValue & 0xFFFF];

				if (MIN_BRANCH_TYPE1 <= varCell.ContCellType && varCell.ContCellType <= MAX_BRANCH_TYPE1) //in content
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
			countCells = ((lastBlockOffset - BLOCK_ENGINE_SIZE) & 0xFFFF) + BLOCK_ENGINE_SIZE;
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
			if (MIN_BLOCK_TYPE <= pContentPage->pType[cell] && pContentPage->pType[cell] <= MAX_BLOCK_TYPE) //in content
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
			countCells = ((lastBlockOffset - BLOCK_ENGINE_SIZE) & 0xFFFF) + BLOCK_ENGINE_SIZE;
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
			if (pContentPage->pType[cell] == VAR_TYPE)
			{
				count++;
			}
		}
	}

	return (count + countReleasedVarCells) == lastVarOffset;
}

bool HArray::testFillContentPages()
{
	char* control = new char[lastContentOffset];
	memset(control, 0, lastContentOffset);

	//1. scan header ==============================================================================================
	for (uint32 cell = 0; cell < HeaderSize; cell++)
	{
		uint32 contentOffset = pHeader[cell];

		if (contentOffset)
		{
			uint32 len = getFullContentLen(contentOffset);

			for (uint32 j = contentOffset; j < contentOffset + len; j++)
			{
				control[j]++;
			}
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
					uint32 len = getFullContentLen(branchCell.Offsets[i]);

					for (uint32 j = branchCell.Offsets[i]; j < branchCell.Offsets[i] + len; j++)
					{
						control[j]++;
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

			if (blockCell.Type == CURRENT_VALUE_TYPE)
			{
				uint32 len = getFullContentLen(blockCell.Offset);

				for (uint32 j = blockCell.Offset; j < blockCell.Offset + len; j++)
				{
					control[j]++;
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

			if (varCell.ContCellType == CONTINUE_VAR_TYPE)
			{
				uint32 len = getFullContentLen(varCell.ContCellValue);

				for (uint32 j = varCell.ContCellValue; j < varCell.ContCellValue + len; j++)
				{
					control[j]++;
				}
			}
		}
	}

	//5. content cells =================================================================================================
	for (uint32 i = 0; i < MAX_KEY_SEGMENTS; i++)
	{
		uint32 contentOffset = tailReleasedContentOffsets[i];

		while (contentOffset)
		{
			uint32 len = i + 1;

			for (uint32 j = contentOffset; j < contentOffset + len; j++)
			{
				control[j]++;
			}

			contentOffset = pContentPages[contentOffset >> 16]->pContent[contentOffset & 0xFFFF];
		}
	}

	//check control values
	for (uint32 i = 1; i < lastContentOffset; i++)
	{
		if (control[i] != 1)
		{
			delete[] control;

			return false;
		}
	}

	delete[] control;

	return true;
}

bool HArray::testFillBranchPages()
{
	if (!countReleasedBranchCells)
	{
		return true;
	}

	char* control = new char[lastBranchOffset];
	memset(control, 0, lastBranchOffset);

	uint32 shrinkLastBranchOffset = lastBranchOffset - countReleasedBranchCells;

	uint32 currTailReleasedBranchOffset = tailReleasedBranchOffset;
	uint32 currCountReleasedBranchCells = countReleasedBranchCells;

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
			uchar8 contentCellType = pContentPage->pType[cell];
			uint32 contentCellValue = pContentPage->pContent[cell];

			if (MIN_BRANCH_TYPE1 <= contentCellType && contentCellType <= MAX_BRANCH_TYPE1) //in content
			{
				control[contentCellValue]++;
			}
			else if (contentCellType == VAR_TYPE) //shunted in var cell
			{
				VarCell& varCell = pVarPages[contentCellValue >> 16]->pVar[contentCellValue & 0xFFFF];

				if (MIN_BRANCH_TYPE1 <= varCell.ContCellType && varCell.ContCellType <= MAX_BRANCH_TYPE1) //in content
				{
					control[varCell.ContCellValue]++;
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
			countCells = ((lastBlockOffset - BLOCK_ENGINE_SIZE) & 0xFFFF) + BLOCK_ENGINE_SIZE;
		}

		for (uint32 cell = 0; cell < countCells; cell++)
		{
			BlockCell& blockCell = pBlockPage->pBlock[cell];

			if (blockCell.Type >= MIN_BRANCH_TYPE1)
			{
				if (blockCell.Type <= MAX_BRANCH_TYPE1) //one branch found
				{
					control[blockCell.Offset]++;
				}
				else if (blockCell.Type <= MAX_BRANCH_TYPE2) //one branch found
				{
					control[blockCell.Offset]++;
					control[blockCell.ValueOrOffset]++;
				}
			}
		}
	}

	//check state
	for (uint32 i = 0; i < currCountReleasedBranchCells; i++)
	{
		control[currTailReleasedBranchOffset]++;

		currTailReleasedBranchOffset = pBranchPages[currTailReleasedBranchOffset >> 16]->pBranch[currTailReleasedBranchOffset & 0xFFFF].Values[0];
	}

	for (uint32 i = 0; i < lastBranchOffset; i++)
	{
		if (control[i] != 1)
		{
			delete[] control;

			return false;
		}
	}

	delete[] control;

	return true;
}

bool HArray::testFillBlockPages()
{
	if (!countReleasedBlockCells)
	{
		return true;
	}

	char* control = new char[lastBlockOffset / BLOCK_ENGINE_SIZE];
	memset(control, 0, lastBlockOffset / BLOCK_ENGINE_SIZE);

	uint32 shrinkLastBlockOffset = lastBlockOffset - countReleasedBlockCells;

	uint32 currTailReleasedBlockOffset = tailReleasedBlockOffset;
	uint32 currCountReleasedBlockCells = countReleasedBlockCells;

	//content ===================================================================================================================
	uint32 lastPage = ContentPagesCount - 1;

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
			if (MIN_BLOCK_TYPE <= pContentPage->pType[cell] && pContentPage->pType[cell] <= MAX_BLOCK_TYPE) //in content
			{
				control[pContentPage->pContent[cell] / BLOCK_ENGINE_SIZE]++;
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
			countCells = ((lastBlockOffset - BLOCK_ENGINE_SIZE) & 0xFFFF) + BLOCK_ENGINE_SIZE;
		}

		for (uint32 cell = 0; cell < countCells; cell++)
		{
			BlockCell& blockCell = pBlockPage->pBlock[cell];

			if (MIN_BLOCK_TYPE <= blockCell.Type && blockCell.Type <= MAX_BLOCK_TYPE) //in block
			{
				control[blockCell.Offset / BLOCK_ENGINE_SIZE]++;
			}
		}
	}

	//check state
	for (uint32 i = 0; i < currCountReleasedBlockCells; i += BLOCK_ENGINE_SIZE)
	{
		control[currTailReleasedBlockOffset / BLOCK_ENGINE_SIZE]++;

		currTailReleasedBlockOffset = pBlockPages[currTailReleasedBlockOffset >> 16]->pBlock[currTailReleasedBlockOffset & 0xFFFF].Offset;
	}

	for (uint32 i = 0; i < lastBlockOffset / BLOCK_ENGINE_SIZE; i++)
	{
		if (control[i] != 1)
		{
			delete[] control;

			return false;
		}
	}

	delete[] control;

	return true;
}

bool HArray::testFillVarPages()
{
	if (!countReleasedVarCells)
	{
		return true;
	}

	char* control = new char[lastVarOffset];
	memset(control, 0, lastVarOffset);

	uint32 currTailReleasedVarOffset = tailReleasedVarOffset;
	uint32 currCountReleasedVarCells = countReleasedVarCells;

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
			if (pContentPage->pType[cell] == VAR_TYPE) //in content
			{
				control[pContentPage->pContent[cell]]++;
			}
		}
	}

	//check state
	for (uint32 i = 0; i < currCountReleasedVarCells; i++)
	{
		control[currTailReleasedVarOffset]++;

		currTailReleasedVarOffset = pVarPages[currTailReleasedVarOffset >> 16]->pVar[currTailReleasedVarOffset & 0xFFFF].ValueContCellValue;
	}

	for (uint32 i = 0; i < lastVarOffset; i++)
	{
		if (control[i] != 1)
		{
			delete[] control;

			return false;
		}
	}

	delete[] control;

	return true;
}