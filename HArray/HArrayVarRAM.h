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
		pHeaderBranchPages = 0;
		pContentPages = 0;
		pVarPages = 0;
		pBranchPages = 0;
		pBlockPages = 0;

		HeaderBranchPagesCount = 0;
		ContentPagesCount = 0;
		VarPagesCount = 0;
		BranchPagesCount = 0;
		BlockPagesCount = 0;
	}

	char Name[256];

	uint32 HeaderBranchPagesCount;
	uint32 ContentPagesCount;
	uint32 VarPagesCount;
	uint32 BranchPagesCount;
	uint32 BlockPagesCount;

	uint32 HeaderBranchPagesSize;
	uint32 ContentPagesSize;
	uint32 VarPagesSize;
	uint32 BranchPagesSize;
	uint32 BlockPagesSize;

	HeaderCell* pHeader;

	/*uint32* pActiveContent;
	ContentTypeCell* pActiveContentType;
	BranchCell* pActiveBranch;
	BlockCell* pActiveBlock;*/

	HeaderBranchPage** pHeaderBranchPages;
	ContentPage** pContentPages;
	VarPage** pVarPages;
	BranchPage** pBranchPages;
	BlockPage** pBlockPages;

	uint32 HeaderBase;
	uint32 HeaderBits;
	uint32 HeaderSize;

	uint32* freeBranchCells;
	uint32 countFreeBranchCell;

	uint32 ValueLen;
	uint32 NewParentID;

	uint32 MAX_SAFE_SHORT;

	void init(uchar8 headerBase)
	{
		init(headerBase,
			 INIT_MAX_PAGES,
			 INIT_MAX_PAGES,
			 INIT_MAX_PAGES,
			 INIT_MAX_PAGES,
			 INIT_MAX_PAGES);
	}

	void init(uchar8 headerBase,
			  uint32 headerBranchPagesSize,
			  uint32 contentPagesSize,
			  uint32 varPagesSize,
			  uint32 branchPagesSize,
			  uint32 blockPagesSize)
	{
        //clear pointers
		pHeader = 0;

		NewParentID = 0;

		pHeaderBranchPages = 0;
		pContentPages = 0;
		pVarPages = 0;
		pBranchPages = 0;
		pBlockPages = 0;

		freeBranchCells = 0;

        try
        {
            ValueLen = 1;

            HeaderBase = headerBase;
            HeaderBits = 32-headerBase;
            HeaderSize = (0xFFFFFFFF>>HeaderBits) + 1;

            countFreeBranchCell = 0;

            MAX_SAFE_SHORT = MAX_SHORT - ValueLen - 1;

            pHeader = new HeaderCell[HeaderSize];
            for(uint32 i=0; i < HeaderSize; i++)
            {
                pHeader[i].Type = 0;
            }

			#ifndef _RELEASE

            for(uint32 i=0; i<COUNT_TEMPS; i++)
            {
                tempValues[i] = 0;
                tempCaptions[i] = 0;
            }

            tempCaptions[MOVES_LEVEL1_STAT] = "Moves Level1";
            tempCaptions[MOVES_LEVEL2_STAT] = "Moves Level2";
            tempCaptions[MOVES_LEVEL3_STAT] = "Moves Level3";
            tempCaptions[MOVES_LEVEL4_STAT] = "Moves Level4";
            tempCaptions[MOVES_LEVEL5_STAT] = "Moves Level5";
            tempCaptions[MOVES_LEVEL6_STAT] = "Moves Level6";
            tempCaptions[MOVES_LEVEL7_STAT] = "Moves Level7";
			tempCaptions[MOVES_LEVEL8_STAT] = "Moves Level8";
			tempCaptions[SHORT_WAY_STAT] = "Short way";
			tempCaptions[LONG_WAY_STAT] = "Long way";
			tempCaptions[CONTENT_BRANCH_STAT] = "Content branch";

            #endif

			pHeaderBranchPages = new HeaderBranchPage*[headerBranchPagesSize];
            pContentPages = new ContentPage*[contentPagesSize];
            pVarPages = new VarPage*[varPagesSize];
            pBranchPages = new BranchPage*[branchPagesSize];
            pBlockPages = new BlockPage*[blockPagesSize];

			memset(pHeaderBranchPages, 0, headerBranchPagesSize * sizeof(HeaderBranchPage*));
            memset(pContentPages, 0, contentPagesSize * sizeof(ContentPage*));
			memset(pVarPages, 0, varPagesSize * sizeof(VarPage*));
   			memset(pBranchPages, 0, branchPagesSize * sizeof(BranchPage*));
			memset(pBlockPages, 0, blockPagesSize * sizeof(BlockPage*));

			HeaderBranchPagesCount = 0;
            ContentPagesCount = 0;
            VarPagesCount = 0;
            BranchPagesCount = 0;
            BlockPagesCount = 0;

			HeaderBranchPagesSize = INIT_MAX_PAGES;
            ContentPagesSize = INIT_MAX_PAGES;
            VarPagesSize = INIT_MAX_PAGES;
            BranchPagesSize = INIT_MAX_PAGES;
            BlockPagesSize = INIT_MAX_PAGES;

			lastHeaderBranchOffset = 0;
            lastContentOffset = 1;
            lastVarOffset = 0;
            lastBranchOffset = 0;
            lastBlockOffset = 0;

            freeBranchCells = new uint32[MAX_SHORT];
		}
		catch(...)
		{
            destroy();

            throw;
		}
	}

	uint32 lastHeaderBranchOffset;
	uint32 lastContentOffset;
	uint32 lastVarOffset;
	uint32 lastBranchOffset;
	uint32 lastBlockOffset;

	uint32 getHash()
	{
		return lastHeaderBranchOffset +
			   lastContentOffset +
			   lastVarOffset +
			   lastBranchOffset +
			   lastBlockOffset;
	}

	uint32 getHeaderSize()
	{
		return HeaderSize * sizeof(uint32);
	}

	uint32 getHeaderBranchSize()
	{
		return HeaderBranchPagesSize * sizeof(uint32);
	}

	uint32 getContentSize()
	{
		return ContentPagesCount * sizeof(ContentPage);
	}

	uint32 getVarSize()
	{
		return VarPagesCount * sizeof(VarPage);
	}

	uint32 getBranchSize()
	{
		return BranchPagesCount * sizeof(BranchPage);
	}

	uint32 getBlockSize()
	{
		return BlockPagesCount * sizeof(BlockPage);
	}

	uint32 getUsedMemory()
	{
		return	getHeaderSize() +
				getHeaderBranchSize() +
				getContentSize() +
				getVarSize() +
				getBranchSize() +
				getBlockSize();
	}

	uint32 getTotalMemory()
	{
		return	getHeaderSize() +
				getHeaderBranchSize() +
				getContentSize() +
				getVarSize() +
				getBranchSize() +
				getBlockSize();
				//valueListPool.getTotalMemory();
	}

	void printMemory()
	{
		printf("=================== HArrayVarRAM =========================\n");
		printf("Header size: %d\n", getHeaderSize());
		printf("Content size: %d\n", getContentSize());
		printf("Var size: %d\n", getVarSize());
		printf("Branch size: %d\n", getBranchSize());
		printf("Block size: %d\n", getBlockSize());
		printf("Total size: %d\n", getTotalMemory());
	}

	void printStat()
	{
		#ifndef _RELEASE

		printf("=================== STAT =========================\n");
		for (uint32 i = 0; i<COUNT_TEMPS; i++)
		{
			if (tempCaptions[i])
			{
				printf("%s => %u\n", tempCaptions[i], tempValues[i]);
			}
		}

		#endif
	}

	void clear()
	{
        uint32 headerBase = this->HeaderBase;

		destroy();

		init(headerBase);
	}

	//types: 0-empty, 1..4 branches, 5 value, 6..9 blocks offset, 10 empty branch, 11 value
#ifndef _RELEASE

	uint32 tempValues[COUNT_TEMPS];
	char* tempCaptions[COUNT_TEMPS];

#endif

	void reallocateContentPages()
	{
		uint32 newSizeContentPages = ContentPagesSize * 2;
		ContentPage** pTempContentPages = new ContentPage*[newSizeContentPages];

		uint32 j=0;
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

	void reallocateHeaderBranchPages()
	{
		uint32 newSizeHeaderBranchPages = HeaderBranchPagesSize * 2;
		HeaderBranchPage** pTempHeaderBranchPages = new HeaderBranchPage*[newSizeHeaderBranchPages];

		uint32 j = 0;
		for (; j < HeaderBranchPagesSize; j++)
		{
			pTempHeaderBranchPages[j] = pHeaderBranchPages[j];
		}

		for (; j < newSizeHeaderBranchPages; j++)
		{
			pTempHeaderBranchPages[j] = 0;
		}

		delete[] pHeaderBranchPages;
		pHeaderBranchPages = pTempHeaderBranchPages;

		HeaderBranchPagesSize = newSizeHeaderBranchPages;
	}

	void reallocateVarPages()
	{
		uint32 newSizeVarPages = VarPagesSize * 2;
		VarPage** pTempVarPages = new VarPage*[newSizeVarPages];

		uint32 j=0;
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
		uint32 newSizeBranchPages = BranchPagesSize * 2;
		BranchPage** pTempBranchPages = new BranchPage*[newSizeBranchPages];

		uint32 j=0;
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
		uint32 newSizeBlockPages = BlockPagesSize * 2;
		BlockPage** pTempBlockPages = new BlockPage*[newSizeBlockPages];

		uint32 j=0;
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

	//returns ha1DocIndex
	uint32 insert(uint32* key,
				uint32 keyLen,
				uint32 value);

	//COMPACT =========================================================================================================
	void calcOptimalBlockSize(CompactPage* pCompactPage,
							uint32& bits,
							uint32& offset,
							uchar8 contentCellType);

	void insertBlockValue(BlockPage** pNewBlockPages,
		 				  	uint32& newBlockPagesCount,
			  				uint32& newLastBlockOffset,
			  				uchar8& contentCellType,
			  				uint32& contentCellValue,
			  				uint32 value,
			  				uint32 offset);

	bool scanBlocks(uint32 blockOffset, CompactPage* pCompactPage);

	void compact();

	//GET =============================================================================================================

	uint32 getValueByKey(uint32* key,
					    uint32 keyLen);

	bool hasPartKey(uint32* key, uint32 keyLen);
	void delValueByKey(uint32* key, uint32 keyLen, uint32 value, uint32 index = 0);

	//uint32* getValueByVarKey(uint32* key, uint32 keyLen);

	void getValuesByRangeFromBlock(uint32** values,
									uint32& count,
									uint32 size,
									uint32 contentOffset,
									uint32 keyOffset,
									uint32 blockOffset,
									uint32* minKey,
									uint32* maxKey);

	void getValuesByRange(uint32** values,
								uint32& count,
								uint32 size,
								uint32 keyOffset,
								uint32 contentOffset,
								uint32* minKey,
								uint32* maxKey);

	uint32 getValuesByRange(uint32** values,
						 uint32 size,
						 uint32* minKey,
						 uint32* maxKey);

	//RANGE keys and values =============================================================================================================
	void sortLastItem(HArrayFixPair* pairs,
					  uint32 count);

	void getKeysAndValuesByRangeFromBlock(HArrayFixPair* pairs,
										  uint32& count,
										  uint32 size,
										  uint32 contentOffset,
										  uint32 keyOffset,
										  uint32 blockOffset,
										  uint32* minKey,
										  uint32* maxKey);

	void getKeysAndValuesByRange(HArrayFixPair* pairs,
								 uint32& count,
								 uint32 size,
								 uint32 keyOffset,
								 uint32 contentOffset,
								 uint32* minKey,
								 uint32* maxKey);

	uint32 getKeysAndValuesByRange(HArrayFixPair* pairs,
								 uint32 size,
								 uint32* minKey,
								 uint32* maxKey);

	//TEMPLATE ====================================================================================================
	void scanKeysAndValuesFromBlock(uint32* key,
									uint32 contentOffset,
									uint32 keyOffset,
									uint32 blockOffset,
									HARRAY_ITEM_VISIT_FUNC visitor,
									void* pData);

	void scanKeysAndValues(uint32* key,
						   uint32 keyOffset,
						   uint32 contentOffset,
						   HARRAY_ITEM_VISIT_FUNC visitor,
						   void* pData);

	uint32 scanKeysAndValues(uint32* key,
						 uint32 keyLen,
						 HARRAY_ITEM_VISIT_FUNC visitor,
						 void* pData);

	//=============================================================================================================

	void destroy()
	{
		if(pHeader)
		{
			delete[] pHeader;
			pHeader = 0;
		}

		if (pHeaderBranchPages)
		{
			for (uint32 i = 0; i<HeaderBranchPagesCount; i++)
			{
				delete pHeaderBranchPages[i];
			}

			delete[] pHeaderBranchPages;
			pHeaderBranchPages = 0;
		}

		if(pContentPages)
		{
			for(uint32 i=0; i<ContentPagesCount; i++)
			{
				delete pContentPages[i];
			}

			delete[] pContentPages;
			pContentPages = 0;
		}

		if(pVarPages)
		{
			for(uint32 i=0; i<VarPagesCount; i++)
			{
				delete pVarPages[i];
			}

			delete[] pVarPages;
			pVarPages = 0;
		}

		if(pBranchPages)
		{
			for(uint32 i=0; i<BranchPagesCount; i++)
			{
				delete pBranchPages[i];
			}

			delete[] pBranchPages;
			pBranchPages = 0;
		}

		if(pBlockPages)
		{
			for(uint32 i=0; i<BlockPagesCount; i++)
			{
				delete pBlockPages[i];
			}

			delete[] pBlockPages;
			pBlockPages = 0;
		}

        if(freeBranchCells)
		{
            delete[] freeBranchCells;
            freeBranchCells = 0;
		}

		//valueListPool.destroy();
	}
};
