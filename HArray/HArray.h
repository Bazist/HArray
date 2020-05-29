/*
# Copyright(C) 2010-2017 Viacheslav Makoveichuk (email: slv709@gmail.com, skype: vyacheslavm81)
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

#include "HArrayBase.h"

class HArray
{
public:
	HArray()
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

	NormalizeFunc normalizeFunc;
	CompareFunc compareFunc;
	CompareSegmentFunc compareSegmentFunc;

	uint32 HeaderBase;
	uint32 HeaderBits;
	uint32 HeaderSize;

	uint32 ValueLen;
	uint32 NewParentID;

	uint32 MAX_SAFE_SHORT;

	uint32 lastHeaderBranchOffset;
	uint32 lastContentOffset;
	uint32 lastVarOffset;
	uint32 lastBranchOffset;
	uint32 lastBlockOffset;

	uint32 autoShrinkOnPercents;
	uint32 notMovedContentCellsAfterLastShrink;
	uint32 amountFreeSlotsBeforeHeaderResize;

	void init()
	{
		init(MIN_HEADER_BASE_BITS,
			INIT_MAX_PAGES,
			INIT_MAX_PAGES,
			INIT_MAX_PAGES,
			INIT_MAX_PAGES,
			INIT_MAX_PAGES);
	}

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
		
		setUInt32Comparator();

		NewParentID = 0;

		pHeaderBranchPages = 0;
		pContentPages = 0;
		pVarPages = 0;
		pBranchPages = 0;
		pBlockPages = 0;

		autoShrinkOnPercents = 1; //1 percent by default
		notMovedContentCellsAfterLastShrink = 0;

		try
        {
            ValueLen = 1;

            HeaderBase = headerBase;
            HeaderBits = 32-headerBase;
            HeaderSize = (0xFFFFFFFF>>HeaderBits) + 1;

			if (headerBase == MIN_HEADER_BASE_BITS) //header resizable
				amountFreeSlotsBeforeHeaderResize = HeaderSize >> MAX_HEADER_FILL_FACTOR_BITS;
			else
				amountFreeSlotsBeforeHeaderResize = 0xFFFFFFFF;

			tailReleasedContentOffsets = 0;
			tailReleasedBranchOffset = 0;
			tailReleasedBlockOffset = 0;
			tailReleasedVarOffset = 0;

            countReleasedContentCells = 0;
            countReleasedBranchCells = 0;
			countReleasedBlockCells = 0;
			countReleasedVarCells = 0;
			
            MAX_SAFE_SHORT = MAX_SHORT - ValueLen;

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

            tailReleasedContentOffsets = new uint32[MAX_KEY_SEGMENTS];

            for(uint32 i=0; i<MAX_KEY_SEGMENTS; i++)
            	tailReleasedContentOffsets[i] = 0;
		}
		catch(...)
		{
            destroy();

            throw;
		}
	}

	bool saveToFile(const char* path)
	{
		FILE * pFile = fopen (path, "wb");
  		if (pFile!=NULL)
  		{
			if (fwrite(this, sizeof(HArray), 1, pFile) != 1)
			{
				goto ERROR;
			}

    		if(pHeader)
			{
				if (fwrite(pHeader, sizeof(HeaderCell), HeaderSize, pFile) != HeaderSize)
				{
					goto ERROR;
				}
			}

			if (pHeaderBranchPages)
			{
				for (uint32 i = 0; i<HeaderBranchPagesCount; i++)
				{
					if (fwrite(pHeaderBranchPages[i], sizeof(HeaderBranchPage), 1, pFile) != 1)
					{
						goto ERROR;
					}
				}
			}

			if(pContentPages)
			{
				for(uint32 i=0; i<ContentPagesCount; i++)
				{
					if (fwrite(pContentPages[i], sizeof(ContentPage), 1, pFile) != 1)
					{
						goto ERROR;
					}
				}
			}

			if(pVarPages)
			{
				for(uint32 i=0; i<VarPagesCount; i++)
				{
					if (fwrite(pVarPages[i], sizeof(VarPage), 1, pFile) != 1)
					{
						goto ERROR;
					}
				}
			}

			if(pBranchPages)
			{
				for(uint32 i=0; i<BranchPagesCount; i++)
				{
					if (fwrite(pBranchPages[i], sizeof(BranchPage), 1, pFile) != 1)
					{
						goto ERROR;
					}
				}
			}

			if(pBlockPages)
			{
				for(uint32 i=0; i<BlockPagesCount; i++)
				{
					if (fwrite(pBlockPages[i], sizeof(BlockPage), 1, pFile) != 1)
					{
						goto ERROR;
					}
				}
			}

			/*
	        if(releaseBranchCells)
			{
	            delete[] releaseBranchCells;
	            releaseBranchCells = 0;
			}
			*/

    		fclose (pFile);

			return true;
  		}
		
	ERROR:

		return false;
	}

	bool loadFromFile(const char* path)
	{
		FILE * pFile = fopen (path, "rb");
  		if (pFile!=NULL)
  		{
			if (fread(this, sizeof(HArray), 1, pFile) != 1)
			{
				goto ERROR;
			}

    		if(pHeader)
			{
				pHeader = new HeaderCell[HeaderSize];

				if(fread (pHeader, sizeof(HeaderCell), HeaderSize, pFile) != HeaderSize)
				{
					goto ERROR;
				}
			}

			if (pHeaderBranchPages)
			{
				pHeaderBranchPages = new HeaderBranchPage*[HeaderBranchPagesCount];
				HeaderBranchPagesSize = HeaderBranchPagesCount;

				for (uint32 i = 0; i<HeaderBranchPagesCount; i++)
				{
					pHeaderBranchPages[i] = new HeaderBranchPage();

					if (fread(pHeaderBranchPages[i], sizeof(HeaderBranchPage), 1, pFile) != 1)
					{
						goto ERROR;
					}
				}
			}

			if(pContentPages)
			{
				pContentPages = new ContentPage*[ContentPagesCount];
				ContentPagesSize = ContentPagesCount;

				for(uint32 i=0; i<ContentPagesCount; i++)
				{
					pContentPages[i] = new ContentPage();

					if (fread(pContentPages[i], sizeof(ContentPage), 1, pFile) != 1)
					{
						goto ERROR;
					}
				}
			}

			if(pVarPages)
			{
				pVarPages = new VarPage*[VarPagesCount];
				VarPagesSize = VarPagesCount;

				for(uint32 i=0; i<VarPagesCount; i++)
				{
					pVarPages[i] = new VarPage();

					if (fread(pVarPages[i], sizeof(VarPage), 1, pFile) != 1)
					{
						goto ERROR;
					}
				}
			}

			if(pBranchPages)
			{
				pBranchPages = new BranchPage*[BranchPagesCount];
				BranchPagesSize = BranchPagesCount;

				for(uint32 i=0; i<BranchPagesCount; i++)
				{
					pBranchPages[i] = new BranchPage();

					if (fread(pBranchPages[i], sizeof(BranchPage), 1, pFile) != 1)
					{
						goto ERROR;
					}
				}
			}

			if(pBlockPages)
			{
				pBlockPages = new BlockPage*[BlockPagesCount];
				BlockPagesSize = BlockPagesCount;

				for(uint32 i=0; i<BlockPagesCount; i++)
				{
					pBlockPages[i] = new BlockPage();

					if (fread(pBlockPages[i], sizeof(BlockPage), 1, pFile) != 1)
					{
						goto ERROR;
					}
				}
			}

			//releaseBranchCells = 0;

			/*
	        if(releaseBranchCells)
			{
	            delete[] releaseBranchCells;
	            releaseBranchCells = 0;
			}
			*/

    		fclose (pFile);

			return true;
  		}

	ERROR:
		destroy();

		return false;
	}

	uint32 getHash()
	{
		return lastHeaderBranchOffset +
			   lastContentOffset +
			   lastVarOffset +
			   lastBranchOffset +
			   lastBlockOffset;
	}

	ulong64 getHeaderSize()
	{
		return HeaderSize * sizeof(uint32);
	}

	ulong64 getHeaderBranchSize()
	{
		return HeaderBranchPagesSize * sizeof(uint32);
	}

	ulong64 getContentSize()
	{
		return ContentPagesCount * sizeof(ContentPage);
	}

	ulong64 getVarSize()
	{
		return VarPagesCount * sizeof(VarPage);
	}

	ulong64 getBranchSize()
	{
		return BranchPagesCount * sizeof(BranchPage);
	}

	ulong64 getBlockSize()
	{
		return BlockPagesCount * sizeof(BlockPage);
	}

	ulong64 getUsedMemory()
	{
		return	getHeaderSize() +
				getHeaderBranchSize() +
				getContentSize() +
				getVarSize() +
				getBranchSize() +
				getBlockSize();
	}

	ulong64 getTotalMemory()
	{
		return	getHeaderSize() +
				getHeaderBranchSize() +
				getContentSize() +
				getVarSize() +
				getBranchSize() +
				getBlockSize();
				//valueListPool.getTotalMemory();
	}

	//str comparator =====================================================
	static uint32 NormalizeStr(void* key)
	{
		//swap bytes
		uint32 num = ((uint32*)key)[0];

		return (num >> 24) |			 // move byte 3 to byte 0
			   ((num << 8) & 0xff0000) | // move byte 1 to byte 2
			   ((num >> 8) & 0xff00) |	 // move byte 2 to byte 1
			   (num << 24);			     // byte 0 to byte 3
	}

	static int CompareSegmentStr(void* keySeg1, void* keySeg2, uint32 index)
	{
		return memcmp(keySeg1, keySeg2, 4);
	}

	static int CompareStr(void* key1, uint32 keyLen1,
						  void* key2, uint32 keyLen2)
	{
		return strcmp((char*)key1, (char*)key2);
	}

	//int comparator =====================================================

	static uint32 NormalizeInt32(void* key)
	{
		int num = ((int*)key)[0];

		if (num < 0)
		{
			return 2147483647 - (num * -1);
		}
		else
		{
			return (uint32)num + 2147483647;
		}
	}

	static int CompareSegmentInt32(void* keySeg1, void* keySeg2, uint32 index)
	{
		if (((int*)keySeg1)[0] < ((int*)keySeg2)[0])
			return -1;

		if (((int*)keySeg1)[0] > ((int*)keySeg2)[0])
			return 1;

		return 0;
	}

	static int CompareInt32(void* key1, uint32 keyLen1,
						    void* key2, uint32 keyLen2)
	{
		uint32 keyLen = keyLen1 < keyLen2 ? keyLen1 : keyLen2;

		for (uint32 i = 0; i < keyLen; i++)
		{
			if (((int*)key1)[i] < ((int*)key2)[i])
				return -1;

			if (((int*)key1)[i] > ((int*)key2)[i])
				return 1;
		}

		if (keyLen1 < keyLen2)
			return -1;

		if (keyLen1 > keyLen2)
			return 1;

		return 0;
	}

	//float comparator =====================================================

	static uint32 NormalizeFloat(void* key)
	{
		uint32 dw = *(uint32*)key;

		if (dw >> 31)
		{
			return 0xFFFFFFFF - dw;
		}
		else
		{
			return 0x7FFFFFFF + dw;
		}
	}

	static int CompareSegmentFloat(void* keySeg1, void* keySeg2, uint32 index)
	{
		if (((float*)keySeg1)[0] < ((float*)keySeg2)[0])
			return -1;

		if (((float*)keySeg1)[0] > ((float*)keySeg2)[0])
			return 1;

		return 0;
	}

	static int CompareFloat(void* key1, uint32 keyLen1,
							void* key2, uint32 keyLen2)
	{
		uint32 keyLen = keyLen1 < keyLen2 ? keyLen1 : keyLen2;

		for (uint32 i = 0; i < keyLen; i++)
		{
			if (((float*)key1)[i] < ((float*)key2)[i])
				return -1;

			if (((float*)key1)[i] > ((float*)key2)[i])
				return 1;
		}

		if (keyLen1 < keyLen2)
			return -1;

		if (keyLen1 > keyLen2)
			return 1;

		return 0;
	}

	//uint32 comparator =====================================================
	static int CompareSegmentUInt32(void* keySeg1, void* keySeg2, uint32 index)
	{
		if (((uint32*)keySeg1)[0] < ((uint32*)keySeg2)[0])
			return -1;

		if (((uint32*)keySeg1)[0] > ((uint32*)keySeg2)[0])
			return 1;

		return 0;
	}

	static int CompareUInt32(void* key1, uint32 keyLen1,
						     void* key2, uint32 keyLen2)
	{
		uint32 keyLen = keyLen1 < keyLen2 ? keyLen1 : keyLen2;

		for (uint32 i = 0; i < keyLen; i++)
		{
			if (((uint32*)key1)[i] < ((uint32*)key2)[i])
				return -1;

			if (((uint32*)key1)[i] > ((uint32*)key2)[i])
				return 1;
		}

		if (keyLen1 < keyLen2)
			return -1;

		if (keyLen1 > keyLen2)
			return 1;

		return 0;
	}

	void setUInt32Comparator()
	{
		normalizeFunc = 0;
		compareSegmentFunc = CompareSegmentUInt32;
		compareFunc = CompareUInt32;
	}

	void setStrComparator()
	{
		normalizeFunc = NormalizeStr;
		compareSegmentFunc = CompareSegmentStr;
		compareFunc = CompareStr;
	}

	void setInt32Comparator()
	{
		normalizeFunc = NormalizeInt32;
		compareSegmentFunc = CompareSegmentInt32;
		compareFunc = CompareInt32;
	}

	void setFloatComparator()
	{
		normalizeFunc = NormalizeFloat;
		compareSegmentFunc = CompareSegmentFloat;
		compareFunc = CompareFloat;
	}

	void setCustomComparator(NormalizeFunc normFunc,
							 CompareSegmentFunc compSegFunc,
							 CompareFunc compFunc)
	{
		normalizeFunc = normFunc;
		compareSegmentFunc = compSegFunc;
		compareFunc = compFunc;
	}

	void printMemory()
	{
		printf("=================== HArray =========================\n");
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
	bool finHeaderBlockPlace(CompactPage* pRootCompactPage,
							uint32 count,
							uchar8 parentID,
							uint32& headerBlockType,
							uint32& baseHeaderOffset,
						    uint32& leftOffset,
						    uint32& rightOffset);

	CompactPage* scanBlocks(uint32& count, uint32 blockOffset, CompactPage* pCompactPage);

	bool allocateHeaderBlock(uint32 keyValue,
							 uint32 keyOffset,
							 uchar8* pContentCellType,
							 uint32* pContentCellValue);

	//REBUILD =========================================================================================================

	void resizeHeader();

	static bool rebuildVisitor(uint32* key, uint32 keyLen, uint32 value, uchar8 valueType, void* pData);

	uint32 rebuild(uint32 headerBase = 0, bool removeEmptyKeys = false);
	
	//GET =============================================================================================================

	uint32* getValueByKey(uint32* key,
					      uint32 keyLen);

	bool hasPartKey(uint32* key, uint32 keyLen);
	bool delValueByKey(uint32* key, uint32 keyLen);

	//RANGE keys and values =============================================================================================================
	void sortLastItem(HArrayPair* pairs,
					  uint32 count);

	void getKeysAndValuesByRangeFromBlock(HArrayPair* pairs,
										  uint32& count,
										  uint32 size,
										  uint32 contentOffset,
										  uint32 keyOffset,
										  uint32 blockOffset,
										  uint32* minKey,
										  uint32 minKeyLen,
										  uint32* maxKey,
										  uint32 maxKeyLen);

	void getKeysAndValuesByRange(HArrayPair* pairs,
								 uint32& count,
								 uint32 size,
								 uint32 keyOffset,
								 uint32 contentOffset,
								 uint32* minKey,
								 uint32 minKeyLen,
								 uint32* maxKey,
								 uint32 maxKeyLen);

	uint32 getKeysAndValuesByRange(HArrayPair* pairs,
								uint32 size,
								uint32* minKey,
								uint32 minKeyLen,
								uint32* maxKey,
								uint32 maxKeyLen);

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

	uint32 scanKeysAndValues(HARRAY_ITEM_VISIT_FUNC visitor,
							 void* pData);

	//DISMANTLING ====================================================================================================

	void autoShrinkIfCouldBeReleasedAtLeast(uint32 percents)
	{
		autoShrinkOnPercents = percents;
	}

	void releaseContentCells(uint32* pContentCell, uint32 contentOffset, uint32 len);
	void releaseBranchCell(BranchCell* pBranchCell, uint32 branchOffset);
	void releaseVarCell(VarCell* pVarCell, uint32 varOffset);
	void releaseBlockCells(BlockCell* pBlockCell, uint32 startBlockOffset);

	void shrinkContentPages();
	void shrinkBranchPages();
	void shrinkBlockPages();
	void shrinkVarPages();

	void shrink();

	uint32* tailReleasedContentOffsets;
	uint32 tailReleasedBranchOffset;
	uint32 tailReleasedBlockOffset;
	uint32 tailReleasedVarOffset;

	uint32 countReleasedContentCells;
	uint32 countReleasedBranchCells;
	uint32 countReleasedBlockCells;
	uint32 countReleasedVarCells;

	bool tryReleaseBlock(SegmentPath* path, uint32 pathLen, int32& currPathLen);
	bool dismantling(SegmentPath* path, uint32 pathLen);
	bool dismantlingContentCells(SegmentPath* path, int32& currPathLen);
	uint32 moveContentCells(uint32& startContentOffset,
							ContentPage** newContentPages,
							uint32& countNewContentPages,
							uint32 shrinkLastContentOffset,
							uint32* lastContentOffsetOnNewPages);

	//for testing
	bool testContentConsistency();
	bool testBlockConsistency();
	bool testBranchConsistency();
	bool testVarConsistency();
	
	bool testFillContentPages();
	bool testFillBlockPages();
	bool testFillBranchPages();
	bool testFillVarPages();

	uint32 getFullContentLen(uint32 contentOffset);
	bool shrinkBlock(uint32 startBlockOffset,
					 uint32 shrinkLastBlockOffset);
	
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

        if(tailReleasedContentOffsets)
		{
            delete[] tailReleasedContentOffsets;
            tailReleasedContentOffsets = 0;
		}

		//valueListPool.destroy();
	}
};
