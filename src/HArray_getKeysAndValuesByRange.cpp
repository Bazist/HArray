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

void HArray::sortLastItem(HArrayPair* pairs,
								int32_t count)
{
	if(count > 1)
	{
		//get last item
		HArrayPair lastItem = pairs[count - 1];

		//find element
		int32_t idx = count - 2;
		HArrayPair& prevItem = pairs[idx];

		if((*compareFunc)(lastItem.Key, lastItem.KeyLen,
					      prevItem.Key, prevItem.KeyLen) == 1)
		{
			//if last item greater then previous item, thats ok
			return;
		}

		//find item great than last
		while(idx)
		{
			idx--;

			if ((*compareFunc)(lastItem.Key, lastItem.KeyLen,
							   pairs[idx].Key, pairs[idx].KeyLen) == 1)
			{
				idx++;

				//move elements
				for(int32_t i = count - 1; i > idx; i--)
				{
					pairs[i] = pairs[i-1];
				}

				pairs[idx] = lastItem;

				return;
			}
		}
	}
}

void HArray::getKeysAndValuesByRangeFromBlock(HArrayPair* pairs,
													int32_t& count,
													int32_t size,
													int32_t contentOffset,
													int32_t keyOffset,
													int32_t blockOffset,
													int32_t* minKey,
													int32_t minKeyLen,
													int32_t* maxKey,
													int32_t maxKeyLen)
{
	int32_t maxOffset = blockOffset + BLOCK_ENGINE_SIZE;

	for(int32_t offset = blockOffset; offset < maxOffset; offset++)
	{
		if(count == size && pairs)
			return;

		BlockPage* pBlockPage = pBlockPages[offset >> 16];
		BlockCell& blockCell = pBlockPage->pBlock[offset & 0xFFFF];

		uint8_t& blockCellType = blockCell.Type;

		if(blockCellType == EMPTY_TYPE)
		{
			continue;
		}
		else if(blockCellType == CURRENT_VALUE_TYPE) //current value
		{
			int32_t& keyValue = blockCell.ValueOrOffset;

			pairs[count].Key[keyOffset] = keyValue;

			int32_t* subMinKey = 0;
			int32_t subMinKeyLen = 0;

			int32_t* subMaxKey = 0;
			int32_t subMaxKeyLen = 0;

			if (minKey)
			{
				if(keyOffset < minKeyLen)
				{
					int res = (*compareSegmentFunc)(&keyValue, &minKey[keyOffset], keyOffset);

					if(res == -1)
						continue;
					else if(res == 0)
					{
						subMinKey = minKey;
						subMinKeyLen = minKeyLen;
					}
				}
				else
				{
					subMinKey = 0;
					subMinKeyLen = 0;
				}
			}

			if (maxKey)
			{
				if(keyOffset < maxKeyLen)
				{
					int res = (*compareSegmentFunc)(&keyValue, &maxKey[keyOffset], keyOffset);

					if(res == 1)
						continue;
					else if(res == 0)
					{
						subMaxKey = maxKey;
						subMaxKeyLen = maxKeyLen;
					}
				}
				else
				{
					subMaxKey = 0;
					subMaxKeyLen = 0;
				}
			}

			getKeysAndValuesByRange(pairs, count, size, keyOffset + 1,  blockCell.Offset,
									subMinKey, subMinKeyLen, subMaxKey, subMaxKeyLen);
		}
		else if(blockCellType <= MAX_BRANCH_TYPE1) //branch cell
		{
			BranchPage* pBranchPage = pBranchPages[blockCell.Offset >> 16];
			BranchCell& branchCell1 = pBranchPage->pBranch[blockCell.Offset & 0xFFFF];

			//try find value in the list
			for(int32_t i=0; i<blockCellType; i++)
			{
				int32_t& keyValue = branchCell1.Values[i];

				pairs[count].Key[keyOffset] = keyValue;

				int32_t* subMinKey = 0;
				int32_t subMinKeyLen = 0;

				int32_t* subMaxKey = 0;
				int32_t subMaxKeyLen = 0;

				if (minKey)
				{
					if(keyOffset < minKeyLen)
					{
						int res = (*compareSegmentFunc)(&keyValue, &minKey[keyOffset], keyOffset);

						if(res == -1)
							continue;
						else if(res == 0)
						{
							subMinKey = minKey;
							subMinKeyLen = minKeyLen;
						}
					}
					else
					{
						subMinKey = 0;
						subMinKeyLen = 0;
					}
				}

				if (maxKey)
				{
					if(keyOffset < maxKeyLen)
					{
						int res = (*compareSegmentFunc)(&keyValue, &maxKey[keyOffset], keyOffset);

						if(res == 1)
							continue;
						else if(res == 0)
						{
							subMaxKey = maxKey;
							subMaxKeyLen = maxKeyLen;
						}
					}
					else
					{
						subMaxKey = 0;
						subMaxKeyLen = 0;
					}
				}

				getKeysAndValuesByRange(pairs, count, size, keyOffset + 1, branchCell1.Offsets[i],
										subMinKey, subMinKeyLen, subMaxKey, subMaxKeyLen);
			}
		}
		else if(blockCellType <= MAX_BRANCH_TYPE2) //branch cell
		{
			BranchPage* pBranchPage1 = pBranchPages[blockCell.Offset >> 16];
			BranchCell branchCell1 = pBranchPage1->pBranch[blockCell.Offset & 0xFFFF];

			//try find value in the list
			for(int32_t i=0; i < BRANCH_ENGINE_SIZE; i++)
			{
				int32_t& keyValue = branchCell1.Values[i];

				pairs[count].Key[keyOffset] = keyValue;

				int32_t* subMinKey = 0;
				int32_t subMinKeyLen = 0;

				int32_t* subMaxKey = 0;
				int32_t subMaxKeyLen = 0;

				if (minKey)
				{
					if(keyOffset < minKeyLen)
					{
						int res = (*compareSegmentFunc)(&keyValue, &minKey[keyOffset], keyOffset);

						if(res == -1)
							continue;
						else if(res == 0)
						{
							subMinKey = minKey;
							subMinKeyLen = minKeyLen;
						}
					}
					else
					{
						subMinKey = 0;
						subMinKeyLen = 0;
					}
				}

				if (maxKey)
				{
					if(keyOffset < maxKeyLen)
					{
						int res = (*compareSegmentFunc)(&keyValue, &maxKey[keyOffset], keyOffset);

						if(res == 1)
							continue;
						else if(res == 0)
						{
							subMaxKey = maxKey;
							subMaxKeyLen = maxKeyLen;
						}
					}
					else
					{
						subMaxKey = 0;
						subMaxKeyLen = 0;
					}
				}

				getKeysAndValuesByRange(pairs, count, size, keyOffset + 1, branchCell1.Offsets[i], 
										subMinKey, subMinKeyLen, subMaxKey, subMaxKeyLen);
			}

			BranchPage* pBranchPage2 = pBranchPages[blockCell.ValueOrOffset >> 16];
			BranchCell branchCell2 = pBranchPage2->pBranch[blockCell.ValueOrOffset & 0xFFFF];

			//try find value in the list
			int32_t countValues = blockCellType - MAX_BRANCH_TYPE1;

			for(int32_t i=0; i<countValues; i++)
			{
				int32_t& keyValue = branchCell2.Values[i];

				pairs[count].Key[keyOffset] = keyValue;

				int32_t* subMinKey = 0;
				int32_t subMinKeyLen = 0;

				int32_t* subMaxKey = 0;
				int32_t subMaxKeyLen = 0;

				if (minKey)
				{
					if(keyOffset < minKeyLen)
					{
						int res = (*compareSegmentFunc)(&keyValue, &minKey[keyOffset], keyOffset);

						if(res == -1)
							continue;
						else if(res == 0)
						{
							subMinKey = minKey;
							subMinKeyLen = minKeyLen;
						}
					}
					else
					{
						subMinKey = 0;
						subMinKeyLen = 0;
					}
				}

				if (maxKey)
				{
					if(keyOffset < maxKeyLen)
					{
						int res = (*compareSegmentFunc)(&keyValue, &maxKey[keyOffset], keyOffset);

						if(res == 1)
							continue;
						else if(res == 0)
						{
							subMaxKey = maxKey;
							subMaxKeyLen = maxKeyLen;
						}
					}
					else
					{
						subMaxKey = 0;
						subMaxKeyLen = 0;
					}
				}

				getKeysAndValuesByRange(pairs, count, size, keyOffset + 1, branchCell2.Offsets[i],
										subMinKey, subMinKeyLen, subMaxKey, subMaxKeyLen);
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
											 minKeyLen,
											 maxKey,
											 maxKeyLen);
		}
	}
}

void HArray::getKeysAndValuesByRange(HArrayPair* pairs,
									int32_t& count,
									int32_t size,
									int32_t keyOffset,
									int32_t contentOffset,
									int32_t* minKey,
									int32_t minKeyLen,
									int32_t* maxKey,
									int32_t maxKeyLen)
{
	//printf("getValuesByRange count=%d size=%d contentOffset=%d keyOffset=%d\n", count, size, contentOffset, keyOffset);

	for(;; keyOffset++, contentOffset++)
	{
		if(count == size && pairs)
			return;

		ContentPage* pContentPage = pContentPages[contentOffset>>16];
		uint16_t contentIndex = contentOffset&0xFFFF;

		int32_t contentCellValueOrOffset = pContentPage->pContent[contentIndex];
		uint8_t contentCellType = pContentPage->pType[contentIndex]; //move to type part

		if(contentCellType >= ONLY_CONTENT_TYPE) //ONLY CONTENT =========================================================================================
		{
			int32_t keyLen =  contentCellType - ONLY_CONTENT_TYPE;

			for(int32_t i = 0; i < keyLen; i++, keyOffset++, contentOffset++)
			{
				int32_t& keyValue = pContentPages[contentOffset>>16]->pContent[contentOffset&0xFFFF];

				pairs[count].Key[keyOffset] = keyValue;

				if (minKey)
				{
					if(keyOffset < minKeyLen)
					{
						int res = (*compareSegmentFunc)(&keyValue, &minKey[keyOffset], keyOffset);

						if(res == 1)
						{
							minKey = 0;
							minKeyLen = 0;
						}
						else if(res == -1)
							return;
					}
					else
					{
						minKey = 0;
						minKeyLen = 0;
					}
				}

				if (maxKey)
				{
					if(keyOffset < maxKeyLen)
					{
						int res = (*compareSegmentFunc)(&keyValue, &maxKey[keyOffset], keyOffset);

						if(res == -1)
						{
							maxKey = 0;
							maxKeyLen = 0;
						}
						else if(res == 1)
							return;
					}
					else
					{
						maxKey = 0;
						maxKeyLen = 0;
					}
				}
			}

			//contentOffset += (keyLen - keyOffset);

			if(pairs)
			{
				//printf("===> ADD VALUE\n");
				pairs[count].Value = pContentPages[contentOffset>>16]->pContent[contentOffset&0xFFFF];
				pairs[count].KeyLen = keyOffset;
				//pairs[count].print();

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
			pairs[count].Value = varCell.ValueContCellValue;
			pairs[count].KeyLen = keyOffset;
			//pairs[count].print();

			count++;
			pairs[count] = pairs[count-1]; //copy prev key

			sortLastItem(pairs, count);

			contentCellType = varCell.ContCellType; //read from var cell
			contentCellValueOrOffset = varCell.ContCellValue;

			if(contentCellType == CONTINUE_VAR_TYPE) //CONTINUE VAR =====================================================================
			{
				contentOffset = contentCellValueOrOffset;

				//goto
				getKeysAndValuesByRange(pairs, count, size, keyOffset, contentOffset, minKey, minKeyLen, maxKey, maxKeyLen);

				return;
			}
		}

		if(contentCellType <= MAX_BRANCH_TYPE1) //BRANCH =====================================================================
		{
			BranchPage* pBranchPage = pBranchPages[contentCellValueOrOffset >> 16];
			BranchCell& branchCell = pBranchPage->pBranch[contentCellValueOrOffset & 0xFFFF];

			//check other
			for(int32_t i = 0; i<contentCellType; i++) //from 1
			{
				int32_t& keyValue = branchCell.Values[i];

				pairs[count].Key[keyOffset] = keyValue;

				int32_t* subMinKey = 0;
				int32_t subMinKeyLen = 0;

				int32_t* subMaxKey = 0;
				int32_t subMaxKeyLen = 0;

				if (minKey)
				{
					if(keyOffset < minKeyLen)
					{
						int res = (*compareSegmentFunc)(&keyValue, &minKey[keyOffset], keyOffset);

						if(res == -1)
							continue;
						else if(res == 0)
						{
							subMinKey = minKey;
							subMinKeyLen = minKeyLen;
						}
					}
					else
					{
						subMinKey = 0;
						subMinKeyLen = 0;
					}
				}

				if (maxKey)
				{
					if(keyOffset < maxKeyLen)
					{
						int res = (*compareSegmentFunc)(&keyValue, &maxKey[keyOffset], keyOffset);

						if(res == 1)
							continue;
						else if(res == 0)
						{
							subMaxKey = maxKey;
							subMaxKeyLen = maxKeyLen;
						}
					}
					else
					{
						subMinKey = 0;
						subMinKeyLen = 0;
					}
				}

				getKeysAndValuesByRange(pairs, count, size, keyOffset + 1, branchCell.Offsets[i],
										subMinKey, subMinKeyLen, subMaxKey, subMaxKeyLen);
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
				//pairs[count].print();

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
											 minKeyLen,
											 maxKey,
											 maxKeyLen);

			return;
		}
		else if(contentCellType == CURRENT_VALUE_TYPE)
		{
			int32_t& keyValue = contentCellValueOrOffset;

			pairs[count].Key[keyOffset] = keyValue;

			/*
			if (minKey && keyValue < minKey[keyOffset])
				return;

			if (maxKey && keyValue > maxKey[keyOffset])
				return;
			*/

			if (minKey)
			{
				if(keyOffset < minKeyLen)
				{
					int res = (*compareSegmentFunc)(&keyValue, &minKey[keyOffset], keyOffset);

					if(res == 1)
					{
						minKey = 0;
						minKeyLen = 0;
					}
					else if(res == -1)
						return;
				}
				else
				{
					minKey = 0;
					minKeyLen = 0;
				}
			}

			if (maxKey)
			{
				if(keyOffset < maxKeyLen)
				{
					int res = (*compareSegmentFunc)(&keyValue, &maxKey[keyOffset], keyOffset);

					if(res == -1)
					{
						maxKey = 0;
						maxKeyLen = 0;
					}
					else if(res == 1)
						return;
				}
				else
				{
					maxKey = 0;
					maxKeyLen = 0;
				}
			}
		}
	}
}


int32_t HArray::getKeysAndValuesByRange(HArrayPair* pairs,
									    int32_t pairsSize,
										int32_t* minKey,
										int32_t minKeyLen,
										int32_t* maxKey,
										int32_t maxKeyLen)
{
	minKeyLen >>= 2;
	maxKeyLen >>= 2;

	int32_t count = 0;

	int32_t startHeader;
	int32_t endHeader;

	if (!normalizeFunc)
	{
		if(minKeyLen)
		{
			startHeader = minKey[0] >> HeaderBits;
		}
		else
		{
			startHeader = 0;
		}

		if(maxKeyLen)
		{
			endHeader = maxKey[0] >> HeaderBits;
		}
		else
		{
			endHeader = 0xFFFFFFFF >> HeaderBits;
		}
	}
	else
	{
		if(minKeyLen)
		{
			startHeader = (*normalizeFunc)(minKey) >> HeaderBits;
		}
		else
		{
			startHeader = 0;
		}

		if(maxKeyLen)
		{
			endHeader = (*normalizeFunc)(maxKey) >> HeaderBits;
		}
		else
		{
			endHeader = 0xFFFFFFFF >> HeaderBits;
		}
	}
		
	//start range
	if(startHeader < endHeader)
	{
		int32_t contentOffset = pHeader[startHeader];
		if(contentOffset)
		{
			getKeysAndValuesByRange(pairs, count, pairsSize, 0, contentOffset, minKey, minKeyLen, 0, 0);
		}

		//middle range
		for(int32_t currKey = startHeader + 1; currKey < endHeader; currKey++)
		{
			if(count == pairsSize)
				return count;

			contentOffset = pHeader[currKey];

			if(contentOffset)
			{
				getKeysAndValuesByRange(pairs, count, pairsSize, 0, contentOffset, 0, 0, 0, 0);
			}
		}

		//end range
		contentOffset = pHeader[endHeader];
		if(contentOffset)
		{
			getKeysAndValuesByRange(pairs, count, pairsSize, 0, contentOffset, 0, 0, maxKey, maxKeyLen);
		}
	}
	else
	{
		int32_t contentOffset = pHeader[startHeader];
		if(contentOffset)
		{
			getKeysAndValuesByRange(pairs, count, pairsSize, 0, contentOffset, minKey, minKeyLen, maxKey, maxKeyLen);
		}
	}

	return count;
}
