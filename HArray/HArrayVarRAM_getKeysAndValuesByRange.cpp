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

void HArrayVarRAM::sortLastItem(HArrayFixPair* pairs,
								uint32 count)
{
	/*printf("========================\n");
	for(uint32 j=0; j < count; j++)
	{
		printf("%u %u\n", pairs[j].Key[0], pairs[j].Key[1]);
	}*/

	if(count > 1)
	{
		//get last item
		HArrayFixPair lastItem = pairs[count - 1];

		//find element
		uint32 idx = count - 2;
		HArrayFixPair& prevItem = pairs[idx];

		if(lastItem.compareTo(prevItem) == 1)
		{
			//if last item greater then previous item, thats ok
			return;
		}

		//find item great than last
		while(idx)
		{
			idx--;

			if(lastItem.compareTo(pairs[idx]) == 1)
			{
				idx++;

				//move elements
				for(uint32 i = count - 1; i > idx; i--)
				{
					pairs[i] = pairs[i-1];
				}

				pairs[idx] = lastItem;

				return;
			}
		}
	}
}

void HArrayVarRAM::getKeysAndValuesByRangeFromBlock(HArrayFixPair* pairs,
													uint32& count,
													uint32 size,
													uint32 contentOffset,
													uint32 keyOffset,
													uint32 blockOffset,
													uint32* minKey,
													uint32* maxKey)
{
	//printf("getValuesByRangeFromBlock count=%d size=%d contentOffset=%d keyOffset=%d blockOffset=%d\n", count, size, contentOffset, keyOffset, blockOffset);

	uint32 maxOffset = blockOffset + BLOCK_ENGINE_SIZE;

	for(uint32 offset = blockOffset; offset < maxOffset; offset++)
	{
		if(count == size && pairs)
			return;

		BlockPage* pBlockPage = pBlockPages[offset >> 16];
		BlockCell& blockCell = pBlockPage->pBlock[offset & 0xFFFF];

		uchar8& blockCellType = blockCell.Type;

		if(blockCellType == EMPTY_TYPE)
		{
			continue;
		}
		else if(blockCellType == CURRENT_VALUE_TYPE) //current value
		{
			uint32& keyValue = blockCell.ValueOrOffset;

			pairs[count].Key[keyOffset] = keyValue;

			uint32* subMinKey = 0;
			uint32* subMaxKey = 0;

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

			getKeysAndValuesByRange(pairs, count, size, keyOffset + 1,  blockCell.Offset, subMinKey, subMaxKey);
		}
		else if(blockCellType <= MAX_BRANCH_TYPE1) //branch cell
		{
			BranchPage* pBranchPage = pBranchPages[blockCell.Offset >> 16];
			BranchCell& branchCell1 = pBranchPage->pBranch[blockCell.Offset & 0xFFFF];

			//try find value in the list
			for(uint32 i=0; i<blockCellType; i++)
			{
				uint32& keyValue = branchCell1.Values[i];

				pairs[count].Key[keyOffset] = keyValue;

				uint32* subMinKey = 0;
				uint32* subMaxKey = 0;

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

				getKeysAndValuesByRange(pairs, count, size, keyOffset + 1, branchCell1.Offsets[i], subMinKey, subMaxKey);
			}
		}
		else if(blockCellType <= MAX_BRANCH_TYPE2) //branch cell
		{
			BranchPage* pBranchPage1 = pBranchPages[blockCell.Offset >> 16];
			BranchCell branchCell1 = pBranchPage1->pBranch[blockCell.Offset & 0xFFFF];

			//try find value in the list
			for(uint32 i=0; i < BRANCH_ENGINE_SIZE; i++)
			{
				uint32& keyValue = branchCell1.Values[i];

				pairs[count].Key[keyOffset] = keyValue;

				uint32* subMinKey = 0;
				uint32* subMaxKey = 0;

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

				getKeysAndValuesByRange(pairs, count, size, keyOffset + 1, branchCell1.Offsets[i], subMinKey, subMaxKey);
			}

			BranchPage* pBranchPage2 = pBranchPages[blockCell.ValueOrOffset >> 16];
			BranchCell branchCell2 = pBranchPage2->pBranch[blockCell.ValueOrOffset & 0xFFFF];

			//try find value in the list
			uint32 countValues = blockCellType - MAX_BRANCH_TYPE1;

			for(uint32 i=0; i<countValues; i++)
			{
				uint32& keyValue = branchCell2.Values[i];

				pairs[count].Key[keyOffset] = keyValue;

				uint32* subMinKey = 0;
				uint32* subMaxKey = 0;

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

				getKeysAndValuesByRange(pairs, count, size, keyOffset + 1, branchCell2.Offsets[i], subMinKey, subMaxKey);
			}
		}
		else if(blockCell.Type <= MAX_BLOCK_TYPE)
		{
			//go to block
			getKeysAndValuesByRangeFromBlock(pairs,
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

void HArrayVarRAM::getKeysAndValuesByRange(HArrayFixPair* pairs,
										   uint32& count,
										   uint32 size,
										   uint32 keyOffset,
										   uint32 contentOffset,
										   uint32* minKey,
										   uint32* maxKey)
{
	//printf("getValuesByRange count=%d size=%d contentOffset=%d keyOffset=%d\n", count, size, contentOffset, keyOffset);

	for(;; keyOffset++, contentOffset++)
	{
		if(count == size && pairs)
			return;

NEXT_KEY_PART:

		ContentPage* pContentPage = pContentPages[contentOffset>>16];
		ushort16 contentIndex = contentOffset&0xFFFF;

		uint32 contentCellValueOrOffset = pContentPage->pContent[contentIndex].Value;
		uchar8 contentCellType = pContentPage->pContent[contentIndex].Type; //move to type part

		if(contentCellType >= ONLY_CONTENT_TYPE) //ONLY CONTENT =========================================================================================
		{
			uint32 keyLen =  contentCellType - ONLY_CONTENT_TYPE;

			for(uint32 i = 0; i < keyLen; i++, keyOffset++, contentOffset++)
			{
				uint32& keyValue = pContentPages[contentOffset>>16]->pContent[contentOffset&0xFFFF].Value;

				pairs[count].Key[keyOffset] = keyValue;

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

			//contentOffset += (keyLen - keyOffset);

			if(pairs)
			{
				//printf("===> ADD VALUE\n");
				pairs[count].Value = pContentPages[contentOffset>>16]->pContent[contentOffset&0xFFFF].Value;
				pairs[count].KeyLen = keyOffset;

				count++;
				pairs[count] = pairs[count-1]; //copy prev key

				sortLastItem(pairs, count);
			}
			else
			{
				count++;
			}

			return;
		}

		if(contentCellType == VAR_TYPE) //VAR =====================================================================
		{
			VarPage* pVarPage = pVarPages[contentCellValueOrOffset >> 16];
			VarCell& varCell = pVarPage->pVar[contentCellValueOrOffset & 0xFFFF];

			//save value
			pairs[count].Value = varCell.ValueContentCell.Value;
			pairs[count].KeyLen = keyOffset;

			count++;
			pairs[count] = pairs[count-1]; //copy prev key

			sortLastItem(pairs, count);

			contentCellType = varCell.ContCell.Type; //read from var cell
			contentCellValueOrOffset = varCell.ContCell.Value;

			if(contentCellType == CONTINUE_VAR_TYPE) //CONTINUE VAR =====================================================================
			{
				contentOffset = contentCellValueOrOffset;

				//goto
				getKeysAndValuesByRange(pairs, count, size, keyOffset, contentOffset, minKey, maxKey);

				return;
			}
		}

		if(contentCellType <= MAX_BRANCH_TYPE1) //BRANCH =====================================================================
		{
			BranchPage* pBranchPage = pBranchPages[contentCellValueOrOffset >> 16];
			BranchCell& branchCell = pBranchPage->pBranch[contentCellValueOrOffset & 0xFFFF];

			//check other
			for(uint32 i = 0; i<contentCellType; i++) //from 1
			{
				uint32& keyValue = branchCell.Values[i];

				pairs[count].Key[keyOffset] = keyValue;

				uint32* subMinKey = 0;
				uint32* subMaxKey = 0;

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

				getKeysAndValuesByRange(pairs, count, size, keyOffset + 1, branchCell.Offsets[i], subMinKey, subMaxKey);
			}

			return;
		}
		else if(contentCellType == VALUE_TYPE)
		{
			if(pairs)
			{
				//printf("===> ADD VALUE\n");
				pairs[count].Value = contentCellValueOrOffset;
				pairs[count].KeyLen = keyOffset;

				count++;
				pairs[count] = pairs[count-1]; //copy prev key

				sortLastItem(pairs, count);
			}
			else
			{
				count++;
			}

			return;
		}
		else if(contentCellType <= MAX_BLOCK_TYPE) //VALUE IN BLOCK ===================================================================
		{
			getKeysAndValuesByRangeFromBlock(pairs,
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
			uint32& keyValue = contentCellValueOrOffset;

			pairs[count].Key[keyOffset] = keyValue;

			/*
			if (minKey && keyValue < minKey[keyOffset])
				return;

			if (maxKey && keyValue > maxKey[keyOffset])
				return;
			*/

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
}


uint32 HArrayVarRAM::getKeysAndValuesByRange(	HArrayFixPair* pairs,
											uint32 size,
											uint32* minKey,
											uint32* maxKey)
{
	uint32 count = 0;
	uint32 startHeader = minKey[0] >> HeaderBits;
	uint32 endHeader = maxKey[0] >> HeaderBits;

	//start range
	if(startHeader < endHeader)
	{
		uint32 contentOffset = pHeader[startHeader];
		if(contentOffset)
		{
			getKeysAndValuesByRange(pairs, count, size, 0, contentOffset, minKey, 0);
		}

		//middle range
		for(uint32 currKey = startHeader + 1; currKey < endHeader; currKey++)
		{
			if(count == size)
				return count;

			contentOffset = pHeader[currKey];

			if(contentOffset)
			{
				getKeysAndValuesByRange(pairs, count, size, 0, contentOffset, 0, 0);
			}
		}

		//end range
		contentOffset = pHeader[endHeader];
		if(contentOffset)
		{
			getKeysAndValuesByRange(pairs, count, size, 0, contentOffset, 0, maxKey);
		}
	}
	else
	{
		uint32 contentOffset = pHeader[startHeader];
		if(contentOffset)
		{
			getKeysAndValuesByRange(pairs, count, size, 0, contentOffset, minKey, maxKey);
		}
	}

	return count;
}
