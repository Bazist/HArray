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


#pragma once

#ifndef _HARRAY_VAR_RAM		 // Allow use of features specific to Windows XP or later.                   
#define _HARRAY_VAR_RAM 0x778 // Change this to the appropriate value to target other versions of Windows.

#endif


#include "HArrayFixBase.h"

class HArrayVarRAM
{
public:
	HArrayVarRAM()
	{
		pContentPages = 0;
		pVarPages = 0;
		pBranchPages = 0;
		pBlockPages = 0;

		ContentPagesCount = 0;
		VarPagesCount = 0;
		BranchPagesCount = 0;
		BlockPagesCount = 0;
	}

	uint ContentPagesCount;
	uint VarPagesCount;
	uint BranchPagesCount;
	uint BlockPagesCount;

	uint ContentPagesSize;
	uint VarPagesSize;
	uint BranchPagesSize;
	uint BlockPagesSize;

	uint* pHeader;
	
	/*uint* pActiveContent;
	ContentTypeCell* pActiveContentType;
	BranchCell* pActiveBranch;
	BlockCell* pActiveBlock;*/
	
	ContentPage** pContentPages;
	VarPage** pVarPages;
	BranchPage** pBranchPages;
	BlockPage** pBlockPages;

	uint HeaderBase;
	uint HeaderBits;
	uint HeaderSize;

	uint* freeBranchCells;
	uint countFreeBranchCell;

	uint ValueLen;
	
	uint MAX_SAFE_SHORT;

	void init(uint valueLen, 
			  uchar headerBase)
	{
		init(valueLen,
			 headerBase,
			 INIT_MAX_PAGES,
			 INIT_MAX_PAGES,
			 INIT_MAX_PAGES,
			 INIT_MAX_PAGES);
	}

	void init(uint valueLen, 
			  uchar headerBase,
			  uint contentPagesSize,
			  uint varPagesSize,
			  uint branchPagesSize,
			  uint blockPagesSize)
	{
		ValueLen = valueLen / 4;
		
		HeaderBase = headerBase;
		HeaderBits = 32-headerBase;
		HeaderSize = (0xFFFFFFFF>>HeaderBits) + 1;

		countFreeBranchCell = 0;

		MAX_SAFE_SHORT = MAX_SHORT - valueLen - 1;

		pHeader = new uint[HeaderSize];
		for(uint i=0; i<HeaderSize; i++)
		{
			pHeader[i] = 0;
		}

		#ifndef _RELEASE

		for(uint i=0; i<COUNT_TEMPS; i++)
		{
			tempValues[i] = 0;
			tempCaptions[i] = 0;
		}

		tempCaptions[0] = "Moves Level1";
		tempCaptions[1] = "Moves Level2";
		tempCaptions[2] = "Moves Level3";
		tempCaptions[3] = "Moves Level4";
		tempCaptions[4] = "Amount Blocks";
		tempCaptions[5] = "Fill Blocks 1..64";
		tempCaptions[6] = "Fill Blocks 64..128";
		tempCaptions[7] = "Fill Blocks 128..192";
		tempCaptions[8] = "Fill Blocks 192..256";
		tempCaptions[9] = "Next blocks";
		tempCaptions[10] = "Short way";
		tempCaptions[11] = "Long way";
		tempCaptions[12] = "CURRENT_VALUE_TYPE";
		tempCaptions[13] = "ONLY_CONTENT_TYPE ";
		tempCaptions[14] = "MAX_BRANCH_TYPE1  ";
		tempCaptions[15] = "MAX_BLOCK_TYPE    ";
		tempCaptions[16] = "Fill Blocks 1..16";
		tempCaptions[17] = "Fill Blocks 16..32";
		tempCaptions[18] = "Fill Blocks 32..48";
		tempCaptions[19] = "Fill Blocks 48..64";
		tempCaptions[20] = "Fill Blocks 1..8";
		tempCaptions[21] = "Next Block Level 3";

		#endif

		pContentPages = new ContentPage*[contentPagesSize];
		pVarPages = new VarPage*[varPagesSize];
		pBranchPages = new BranchPage*[branchPagesSize];
		pBlockPages = new BlockPage*[blockPagesSize];

		memset(pContentPages, 0, contentPagesSize * sizeof(uint));
		memset(pVarPages, 0, varPagesSize * sizeof(uint));
		memset(pBranchPages, 0, branchPagesSize * sizeof(uint));
		memset(pBlockPages, 0, blockPagesSize * sizeof(uint));

		ContentPagesCount = 0;
		VarPagesCount = 0;
		BranchPagesCount = 0;
		BlockPagesCount = 0;

		ContentPagesSize = INIT_MAX_PAGES;
		VarPagesSize = INIT_MAX_PAGES;
		BranchPagesSize = INIT_MAX_PAGES;
		BlockPagesSize = INIT_MAX_PAGES;
		
		lastContentOffset = 1;
		lastVarOffset = 0;
		lastBranchOffset = 0;
		lastBlockOffset = 0;

		freeBranchCells = new uint[MAX_SHORT];
	}

	uint lastContentOffset;
	uint lastVarOffset;
	uint lastBranchOffset;
	uint lastBlockOffset;

	uint getHeaderSize()
	{
		return HeaderSize * sizeof(uint);
	}

	uint getContentSize()
	{
		return ContentPagesCount * sizeof(ContentPage);
	}

	uint getVarSize()
	{
		return VarPagesCount * sizeof(VarPage);
	}

	uint getBranchSize()
	{
		return BranchPagesCount * sizeof(BranchPage);
	}

	uint getBlockSize()
	{
		return BlockPagesCount * sizeof(BlockPage);
	}
	
	uint getUsedMemory()
	{
		return	getHeaderSize() + 
				getContentSize() + 
				getVarSize() + 
				getBranchSize() +
				getBlockSize();
	}

	uint getTotalMemory()
	{
		return	getHeaderSize() + 
				getContentSize() + 
				getVarSize() + 
				getBranchSize() +
				getBlockSize();
	}

	void clear()
	{
		destroy();
		init(4, 24);
	}
	
	//types: 0-empty, 1..4 branches, 5 value, 6..9 blocks offset, 10 empty branch, 11 value
#ifndef _RELEASE

	uint tempValues[COUNT_TEMPS];
	char* tempCaptions[COUNT_TEMPS];

#endif

	void reallocateContentPages()
	{
		uint newSizeContentPages = ContentPagesSize * 2;
		ContentPage** pTempContentPages = new ContentPage*[newSizeContentPages];
			
		uint j=0;
		for(; j < ContentPagesSize ; j++)
		{
			pTempContentPages[j] = pContentPages[j];
		}

		for(; j < newSizeContentPages ; j++)
		{
			pTempContentPages[j] = 0;
		}

		delete[] pContentPages;
		pContentPages = pTempContentPages;

		ContentPagesSize = newSizeContentPages;
	}

	void reallocateVarPages()
	{
		uint newSizeVarPages = VarPagesSize * 2;
		VarPage** pTempVarPages = new VarPage*[newSizeVarPages];
			
		uint j=0;
		for(; j < VarPagesSize ; j++)
		{
			pTempVarPages[j] = pVarPages[j];
		}

		for(; j < newSizeVarPages ; j++)
		{
			pTempVarPages[j] = 0;
		}

		delete[] pVarPages;
		pVarPages = pTempVarPages;

		VarPagesSize = newSizeVarPages;
	}

	void reallocateBranchPages()
	{
		uint newSizeBranchPages = BranchPagesSize * 2;
		BranchPage** pTempBranchPages = new BranchPage*[newSizeBranchPages];
			
		uint j=0;
		for(; j < BranchPagesSize ; j++)
		{
			pTempBranchPages[j] = pBranchPages[j];
		}

		for(; j < newSizeBranchPages ; j++)
		{
			pTempBranchPages[j] = 0;
		}

		delete[] pBranchPages;
		pBranchPages = pTempBranchPages;

		BranchPagesSize = newSizeBranchPages;
	}

	void reallocateBlockPages()
	{
		uint newSizeBlockPages = BlockPagesSize * 2;
		BlockPage** pTempBlockPages = new BlockPage*[newSizeBlockPages];
			
		uint j=0;
		for(; j < BlockPagesSize ; j++)
		{
			pTempBlockPages[j] = pBlockPages[j];
		}

		for(; j < newSizeBlockPages ; j++)
		{
			pTempBlockPages[j] = 0;
		}

		delete[] pBlockPages;
		pBlockPages = pTempBlockPages;

		BlockPagesSize = newSizeBlockPages;
	}
	
	//INSERT =============================================================================================================

	bool insert(uint* key, uint keyLen, uint value);
		
	//GET =============================================================================================================

	uint getValueByKey(uint* key, uint keyLen);

	//uint* getValueByVarKey(uint* key, uint keyLen);
	
	void getValuesByRangeFromBlock(uint** values, 
									uint& count,
									uint size,
									uint contentOffset,
									uint keyOffset,
									uint blockOffset,
									uint* minKey,
									uint* maxKey);
	
	void getValuesByRange(uint** values, 
								uint& count,
								uint size,
								uint keyOffset, 
								uint contentOffset,
								uint* minKey,
								uint* maxKey);
	
	uint getValuesByRange(uint** values, 
						 uint size, 
						 uint* minKey, 
						 uint* maxKey);

	//RANGE keys and values =============================================================================================================
	void sortLastItem(HArrayFixPair* pairs, 
					  uint count);
	
	void getKeysAndValuesByRangeFromBlock(HArrayFixPair* pairs,
										  uint& count,
										  uint size,
										  uint contentOffset,
										  uint keyOffset,
										  uint blockOffset,
										  uint* minKey,
										  uint* maxKey);
	
	void getKeysAndValuesByRange(HArrayFixPair* pairs,
								 uint& count,
								 uint size,
								 uint keyOffset, 
								 uint contentOffset,
								 uint* minKey,
								 uint* maxKey);
	
	uint getKeysAndValuesByRange(HArrayFixPair* pairs,
								 uint size, 
								 uint* minKey, 
								 uint* maxKey);

	uint getTailKeyByHeadKeyAndValue(uint* headKey,
									 uint headKeyLen,
									 uint* tailKey,
									 uint& tailKeyLen,
									 uint value);
	
	//TEMPLATE ====================================================================================================
	void scanKeysAndValuesFromBlock(uint* key,
									uint contentOffset,
									uint keyOffset,
									uint blockOffset,
									HARRAY_ITEM_VISIT_FUNC visitor);
	
	void scanKeysAndValues(uint* key,
						   uint keyOffset, 
						   uint contentOffset,
						   HARRAY_ITEM_VISIT_FUNC visitor);
	
	uint HArrayVarRAM::scanKeysAndValues(uint* key, 
										 uint keyLen,
										 HARRAY_ITEM_VISIT_FUNC visitor);

	//=============================================================================================================

	void destroy()
	{
		if(pHeader)
		{
			delete[] pHeader;
			pHeader = 0;
		}

		if(pContentPages)
		{
			for(uint i=0; i<ContentPagesCount; i++)
			{
				delete pContentPages[i];
			}

			delete[] pContentPages;
			pContentPages = 0;
		}

		if(pVarPages)
		{
			for(uint i=0; i<VarPagesCount; i++)
			{
				delete pVarPages[i];
			}

			delete[] pVarPages;
			pVarPages = 0;
		}

		if(pBranchPages)
		{
			for(uint i=0; i<BranchPagesCount; i++)
			{
				delete pBranchPages[i];
			}
			
			delete[] pBranchPages;
			pBranchPages = 0;
		}

		if(pBlockPages)
		{
			for(uint i=0; i<BlockPagesCount; i++)
			{
				delete pBlockPages[i];
			}

			delete[] pBlockPages;
			pBlockPages = 0;
		}

		delete[] freeBranchCells;
	}
};