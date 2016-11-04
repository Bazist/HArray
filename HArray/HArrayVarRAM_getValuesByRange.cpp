#include "stdafx.h"
#include "HArrayVarRAM.h"

void HArrayVarRAM::getValuesByRangeFromBlock(uint32** values,
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
		if(count == size && values)
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

			getValuesByRange(values, count, size, keyOffset + 1,  blockCell.Offset, subMinKey, subMaxKey);
		}
		else if(blockCellType <= MAX_BRANCH_TYPE1) //branch cell
		{
			BranchPage* pBranchPage = pBranchPages[blockCell.Offset >> 16];
			BranchCell& branchCell1 = pBranchPage->pBranch[blockCell.Offset & 0xFFFF];

			//try find value in the list
			for(uint32 i=0; i<blockCellType; i++)
			{
				uint32& keyValue = branchCell1.Values[i];

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

				getValuesByRange(values, count, size, keyOffset + 1, branchCell1.Offsets[i], subMinKey, subMaxKey);
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

				getValuesByRange(values, count, size, keyOffset + 1, branchCell1.Offsets[i], subMinKey, subMaxKey);
			}

			BranchPage* pBranchPage2 = pBranchPages[blockCell.ValueOrOffset >> 16];
			BranchCell branchCell2 = pBranchPage2->pBranch[blockCell.ValueOrOffset & 0xFFFF];

			//try find value in the list
			uint32 countValues = blockCellType - MAX_BRANCH_TYPE1;

			for(uint32 i=0; i<countValues; i++)
			{
				uint32& keyValue = branchCell2.Values[i];

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

void HArrayVarRAM::getValuesByRange(uint32** values,
							uint32& count,
							uint32 size,
							uint32 keyOffset,
							uint32 contentOffset,
							uint32* minKey,
							uint32* maxKey)
{
	//printf("getValuesByRange count=%d size=%d contentOffset=%d keyOffset=%d\n", count, size, contentOffset, keyOffset);

	//!!! change
	uint32 KeyLen = 0;

	for(; keyOffset <= KeyLen; keyOffset++, contentOffset++)
	{
		if(count == size && values)
			return;

		ContentPage* pContentPage = pContentPages[contentOffset>>16];
		ushort16 contentIndex = contentOffset&0xFFFF;

		uint32& contentCellValueOrOffset = pContentPage->pContent[contentIndex].Value;
		uchar8& contentCellType = pContentPage->pContent[contentIndex].Type; //move to type part

		if(contentCellType >= ONLY_CONTENT_TYPE) //ONLY CONTENT =========================================================================================
		{
			if(minKey || maxKey)
			{
				for(; keyOffset < KeyLen; keyOffset++, contentOffset++)
				{
					uint32& keyValue = pContentPages[contentOffset>>16]->pContent[contentOffset&0xFFFF].Value;

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
			for(uint32 i = 0; i<contentCellType; i++) //from 1
			{
				uint32& keyValue = branchCell.Values[i];

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
			uint32& keyValue = contentCellValueOrOffset;

			if (minKey && keyValue < minKey[keyOffset])
				return;

			if (maxKey && keyValue > maxKey[keyOffset])
				return;
		}
	}
}


uint32 HArrayVarRAM::getValuesByRange(uint32** values,
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
			getValuesByRange(values, count, size, 0, contentOffset, minKey, 0);
		}

		//middle range
		for(uint32 currKey = startHeader + 1; currKey < endHeader; currKey++)
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
		uint32 contentOffset = pHeader[startHeader];
		if(contentOffset)
		{
			getValuesByRange(values, count, size, 0, contentOffset, minKey, maxKey);
		}
	}

	return count;
}
