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

#include "StdAfx.h"
#include "HArrayVarRAM.h"

/*
void HArrayVarRAM::getKeysAndValuesByRange(HArrayFixPair* pairs,
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
		if(count == size && pairs)
			return;

		ContentPage* pContentPage = pContentPages[contentOffset>>16];
		ushort contentIndex = contentOffset&0xFFFF;
		
		uint& contentCellValueOrOffset = pContentPage->pContent[contentIndex].Value;
		uchar& contentCellType = pContentPage->pContent[contentIndex].Type; //move to type part

		if(contentCellType >= ONLY_CONTENT_TYPE) //ONLY CONTENT =========================================================================================
		{
			for(; keyOffset < KeyLen; keyOffset++, contentOffset++)
			{
				uint& keyValue = pContentPages[contentOffset>>16]->pContent[contentOffset&0xFFFF].Value;
					
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

			contentOffset += (KeyLen - keyOffset);

			if(pairs)
			{
				//printf("===> ADD VALUE\n");
				pairs[count++].Value = pContentPages[contentOffset>>16]->pContent[contentOffset&0xFFFF].Value;
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

			if(keyOffset < keyLen)
			{
				contentCellType = varCell.ContentCell.Type; //read from var cell
				contentCellValueOrOffset = varCell.ContentCell.Value;

				if(contentCellType == CONTINUE_VAR_TYPE) //CONTINUE VAR =====================================================================
				{
					contentOffset = contentCellValueOrOffset;

					goto NEXT_KEY_PART;
				}
			}
			else
			{
				return varCell.Value;
			}
		}
		else if(keyOffset == keyLen)
		{
			if(contentCellType == VALUE_TYPE)
			{
				return contentCellValueOrOffset;
			}
			else
			{
				return 0;
			}
		}

		if(contentCellType <= MAX_BRANCH_TYPE1) //BRANCH =====================================================================
		{
			BranchPage* pBranchPage = pBranchPages[contentCellValueOrOffset >> 16];
			BranchCell& branchCell = pBranchPage->pBranch[contentCellValueOrOffset & 0xFFFF];

			//check other
			for(uint i = 0; i<contentCellType; i++) //from 1
			{
				uint& keyValue = branchCell.Values[i];

				pairs[count].Key[keyOffset] = keyValue;

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

				getKeysAndValuesByRange(pairs, count, size, keyOffset + 1, branchCell.Offsets[i], subMinKey, subMaxKey);
			}

			return;
		}
		else if(contentCellType == VALUE_TYPE)
		{
			if(pairs)
			{
				//printf("===> ADD VALUE\n");
				pairs[count++].Value = contentCellValueOrOffset;
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
			uint& keyValue = contentCellValueOrOffset;
			
			pairs[count].Key[keyOffset] = keyValue;

			if (minKey && keyValue < minKey[keyOffset])
				return;

			if (maxKey && keyValue > maxKey[keyOffset])
				return;
		}
	}
}
*/

uint HArrayVarRAM::getTailKeyByHeadKeyAndValue(uint* headKey,
												uint headKeyLen,
												uint* tailKey,
												uint& tailKeyLen,
												uint value)
{
	uint count = 0;
	uint startHeader = headKey[0] >> HeaderBits;

	uint contentOffset = pHeader[startHeader];
	if(contentOffset)
	{
		//getKeysAndValuesByRange(pairs, count, size, 0, contentOffset, minKey, maxKey);
	}
	else
	{
		tailKeyLen = 0;
	}

	return 0;
}

