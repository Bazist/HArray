/*
# Copyright(C) 2010-2016 Vyacheslav Makoveichuk
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

void HArrayVarRAM::getValuesByRangeFromBlock(uint** values, 
										uint& count,
										uint size,
										uint contentOffset,
										uint keyOffset,
										uint blockOffset,
										uint* minKey,
										uint* maxKey)
{
	//printf("getValuesByRangeFromBlock count=%d size=%d contentOffset=%d keyOffset=%d blockOffset=%d\n", count, size, contentOffset, keyOffset, blockOffset);

	uint maxOffset = blockOffset + BLOCK_ENGINE_SIZE;
	for(uint offset = blockOffset; offset < maxOffset; offset++)
	{
		if(count == size && values)
			return;

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

			uint* subMinKey = 0;
			uint* subMaxKey = 0;
			
			if (minKey)
			{
				if(keyValue < minKey[keyOffset])
					continue;
				else if(keyValue == minKey[keyOffset])
					subMinKey = minKey;
			}

			if (maxKey)
			{
				if(keyValue > maxKey[keyOffset])
					continue;
				else if(keyValue == maxKey[keyOffset])
					subMaxKey = maxKey;
			}

			getValuesByRange(values, count, size, keyOffset + 1,  blockCell.Offset, subMinKey, subMaxKey);
		}
		else if(blockCellType <= MAX_BRANCH_TYPE1) //branch cell
		{
			BranchPage* pBranchPage = pBranchPages[blockCell.Offset >> 16];
			BranchCell& branchCell1 = pBranchPage->pBranch[blockCell.Offset & 0xFFFF];

			//try find value in the list
			for(uint i=0; i<blockCellType; i++)
			{
				uint& keyValue = branchCell1.Values[i];

				uint* subMinKey = 0;
				uint* subMaxKey = 0;
			
				if (minKey)
				{
					if(keyValue < minKey[keyOffset])
						continue;
					else if(keyValue == minKey[keyOffset])
						subMinKey = minKey;
				}

				if (maxKey)
				{
					if(keyValue > maxKey[keyOffset])
						continue;
					else if(keyValue == maxKey[keyOffset])
						subMaxKey = maxKey;
				}

				getValuesByRange(values, count, size, keyOffset + 1, branchCell1.Offsets[i], subMinKey, subMaxKey);
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

				uint* subMinKey = 0;
				uint* subMaxKey = 0;
			
				if (minKey)
				{
					if(keyValue < minKey[keyOffset])
						continue;
					else if(keyValue == minKey[keyOffset])
						subMinKey = minKey;
				}

				if (maxKey)
				{
					if(keyValue > maxKey[keyOffset])
						continue;
					else if(keyValue == maxKey[keyOffset])
						subMaxKey = maxKey;
				}

				getValuesByRange(values, count, size, keyOffset + 1, branchCell1.Offsets[i], subMinKey, subMaxKey);
			}

			BranchPage* pBranchPage2 = pBranchPages[blockCell.ValueOrOffset >> 16];
			BranchCell branchCell2 = pBranchPage2->pBranch[blockCell.ValueOrOffset & 0xFFFF];

			//try find value in the list
			uint countValues = blockCellType - MAX_BRANCH_TYPE1;

			for(uint i=0; i<countValues; i++)
			{
				uint& keyValue = branchCell2.Values[i];

				uint* subMinKey = 0;
				uint* subMaxKey = 0;
			
				if (minKey)
				{
					if(keyValue < minKey[keyOffset])
						continue;
					else if(keyValue == minKey[keyOffset])
						subMinKey = minKey;
				}

				if (maxKey)
				{
					if(keyValue > maxKey[keyOffset])
						continue;
					else if(keyValue == maxKey[keyOffset])
						subMaxKey = maxKey;
				}

				getValuesByRange(values, count, size, keyOffset + 1, branchCell2.Offsets[i], subMinKey, subMaxKey);
			}
		}
		else if(blockCell.Type <= MAX_BLOCK_TYPE)
		{
			//go to block
			getValuesByRangeFromBlock(values, 
									  count,
									  size,
									  contentOffset,
									  keyOffset,
									  blockCell.Offset,
									  minKey, 
									  maxKey);
		}
	}
}

void HArrayVarRAM::getValuesByRange(uint** values, 
							uint& count,
							uint size,
							uint keyOffset, 
							uint contentOffset,
							uint* minKey,
							uint* maxKey)
{
	//printf("getValuesByRange count=%d size=%d contentOffset=%d keyOffset=%d\n", count, size, contentOffset, keyOffset);
	
	//!!! change
	uint KeyLen = 0;

	for(; keyOffset <= KeyLen; keyOffset++, contentOffset++)
	{
		if(count == size && values)
			return;

		ContentPage* pContentPage = pContentPages[contentOffset>>16];
		ushort contentIndex = contentOffset&0xFFFF;
		
		uint& contentCellValueOrOffset = pContentPage->pContent[contentIndex].Value;
		uchar& contentCellType = pContentPage->pContent[contentIndex].Type; //move to type part

		if(contentCellType >= ONLY_CONTENT_TYPE) //ONLY CONTENT =========================================================================================
		{
			if(minKey || maxKey)
			{
				for(; keyOffset < KeyLen; keyOffset++, contentOffset++)
				{
					uint& keyValue = pContentPages[contentOffset>>16]->pContent[contentOffset&0xFFFF].Value;
				
					if (minKey)
					{
						if(keyValue > minKey[keyOffset])
							minKey = 0;
						else if(keyValue < minKey[keyOffset])
							return;
					}

					if (maxKey)
					{
						if(keyValue < maxKey[keyOffset])
							maxKey = 0;
						else if(keyValue > maxKey[keyOffset])
							return;
					}
				}
			}

			contentOffset += (KeyLen - keyOffset);

			if(values)
			{
				//printf("===> ADD VALUE\n");
				values[count++] = &pContentPages[contentOffset>>16]->pContent[contentOffset&0xFFFF].Value;
			}
			else
			{
				count++;
			}

			return;
		}
		else if(contentCellType <= MAX_BRANCH_TYPE1) //BRANCH =====================================================================
		{
			BranchPage* pBranchPage = pBranchPages[contentCellValueOrOffset >> 16];
			BranchCell& branchCell = pBranchPage->pBranch[contentCellValueOrOffset & 0xFFFF];

			//check other
			for(uint i = 0; i<contentCellType; i++) //from 1
			{
				uint& keyValue = branchCell.Values[i];

				uint* subMinKey = 0;
				uint* subMaxKey = 0;
				
				if (minKey)
				{
					if(keyValue < minKey[keyOffset])
						continue;
					else if(keyValue == minKey[keyOffset])
						subMinKey = minKey;
				}

				if (maxKey)
				{
					if(keyValue > maxKey[keyOffset])
						continue;
					else if(keyValue == maxKey[keyOffset])
						subMaxKey = maxKey;
				}

				getValuesByRange(values, count, size, keyOffset + 1, branchCell.Offsets[i], subMinKey, subMaxKey);
			}

			return;
		}
		else if(contentCellType == VALUE_TYPE)
		{
			if(values)
			{
				//printf("===> ADD VALUE\n");
				values[count++] = &contentCellValueOrOffset;
			}
			else
			{
				count++;
			}

			return;
		}
		else if(contentCellType <= MAX_BLOCK_TYPE) //VALUE IN BLOCK ===================================================================
		{
			getValuesByRangeFromBlock(values, 
									  count,
									  size,
									  contentOffset,
									  keyOffset,
									  contentCellValueOrOffset,
									  minKey,
									  maxKey);

			return;
		}
		else if(contentCellType == CURRENT_VALUE_TYPE)
		{
			uint& keyValue = contentCellValueOrOffset;

			if (minKey && keyValue < minKey[keyOffset])
				return;

			if (maxKey && keyValue > maxKey[keyOffset])
				return;
		}
	}
}


uint HArrayVarRAM::getValuesByRange(uint** values, 
								uint size, 
								uint* minKey, 
								uint* maxKey)
{
	uint count = 0;
	uint startHeader = minKey[0] >> HeaderBits;
	uint endHeader = maxKey[0] >> HeaderBits;

	//start range
	if(startHeader < endHeader)
	{
		uint contentOffset = pHeader[startHeader];
		if(contentOffset)
		{
			getValuesByRange(values, count, size, 0, contentOffset, minKey, 0);
		}

		//middle range
		for(uint currKey = startHeader + 1; currKey < endHeader; currKey++)
		{
			if(count == size)
				return count;

			contentOffset = pHeader[currKey];
		
			if(contentOffset)
			{
				getValuesByRange(values, count, size, 0, contentOffset, 0, 0);
			}
		}

		//end range
		contentOffset = pHeader[endHeader];
		if(contentOffset)
		{
			getValuesByRange(values, count, size, 0, contentOffset, 0, maxKey);
		}
	}
	else
	{
		uint contentOffset = pHeader[startHeader];
		if(contentOffset)
		{
			getValuesByRange(values, count, size, 0, contentOffset, minKey, maxKey);
		}
	}
	
	return count;
}