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
		memset(this, 0, sizeof(HArray));

		init();
	}

	int32_t ContentPagesCount;
	int32_t VarPagesCount;
	int32_t BranchPagesCount;
	int32_t BlockPagesCount;

	int32_t ContentPagesSize;
	int32_t VarPagesSize;
	int32_t BranchPagesSize;
	int32_t BlockPagesSize;

	int32_t* pHeader;

	ContentPage** pContentPages;
	VarPage** pVarPages;
	BranchPage** pBranchPages;
	BlockPage** pBlockPages;

	NormalizeFunc normalizeFunc;
	CompareFunc compareFunc;
	CompareSegmentFunc compareSegmentFunc;

	int32_t HeaderBase;
	int32_t HeaderBits;
	int32_t HeaderSize;

	int32_t ValueLen;
	int32_t NewParentID;

	int32_t MAX_SAFE_SHORT;

	int32_t lastHeaderBranchOffset;
	int32_t lastContentOffset;
	int32_t lastVarOffset;
	int32_t lastBranchOffset;
	int32_t lastBlockOffset;

	int32_t autoShrinkOnPercents;
	int32_t notMovedContentCellsAfterLastShrink;
	int32_t amountFreeSlotsBeforeHeaderResize;

	void init()
	{
		init(MIN_HEADER_BASE_BITS,
			INIT_MAX_PAGES,
			INIT_MAX_PAGES,
			INIT_MAX_PAGES,
			INIT_MAX_PAGES);
	}

	void init(uint8_t headerBase)
	{
		init(headerBase,
			 INIT_MAX_PAGES,
			 INIT_MAX_PAGES,
			 INIT_MAX_PAGES,
			 INIT_MAX_PAGES);
	}

	void init(uint8_t headerBase,
			  int32_t contentPagesSize,
			  int32_t varPagesSize,
			  int32_t branchPagesSize,
			  int32_t blockPagesSize)
	{
		destroy();

        //clear pointers
		pHeader = 0;

		setUInt32Comparator();

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

            pHeader = new int32_t[HeaderSize];
            for(int32_t i=0; i < HeaderSize; i++)
            {
                pHeader[i] = 0;
            }

			#ifndef _RELEASE

            for(int32_t i=0; i<COUNT_TEMPS; i++)
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

			pContentPages = new ContentPage*[contentPagesSize];
            pVarPages = new VarPage*[varPagesSize];
            pBranchPages = new BranchPage*[branchPagesSize];
            pBlockPages = new BlockPage*[blockPagesSize];

			memset(pContentPages, 0, contentPagesSize * sizeof(ContentPage*));
			memset(pVarPages, 0, varPagesSize * sizeof(VarPage*));
   			memset(pBranchPages, 0, branchPagesSize * sizeof(BranchPage*));
			memset(pBlockPages, 0, blockPagesSize * sizeof(BlockPage*));

			ContentPagesCount = 0;
            VarPagesCount = 0;
            BranchPagesCount = 0;
            BlockPagesCount = 0;

			ContentPagesSize = INIT_MAX_PAGES;
            VarPagesSize = INIT_MAX_PAGES;
            BranchPagesSize = INIT_MAX_PAGES;
            BlockPagesSize = INIT_MAX_PAGES;

			lastHeaderBranchOffset = 0;
            lastContentOffset = 1;
            lastVarOffset = 0;
            lastBranchOffset = 0;
            lastBlockOffset = 0;

            tailReleasedContentOffsets = new int32_t[MAX_KEY_SEGMENTS];

            for(int32_t i=0; i<MAX_KEY_SEGMENTS; i++)
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
		FILE* pFile = 0;

		#ifdef _WIN32
        errno_t errorCode = fopen_s(&pFile, path, "wb");
        if (!errorCode)
  		#endif // _WIN32

#if defined linux  || defined __APPLE__
		pFile = fopen(path, "wb");
		if(pFile != NULL)
        #endif // linux
  		{
			if (fwrite(this, sizeof(HArray), 1, pFile) != 1)
			{
				goto ERROR_LABEL;
			}

    		if(pHeader)
			{
				if (fwrite(pHeader, sizeof(int32_t), HeaderSize, pFile) != HeaderSize)
				{
					goto ERROR_LABEL;
				}
			}

			if(pContentPages)
			{
				for(int32_t i=0; i<ContentPagesCount; i++)
				{
					if (fwrite(pContentPages[i], sizeof(ContentPage), 1, pFile) != 1)
					{
						goto ERROR_LABEL;
					}
				}
			}

			if(pVarPages)
			{
				for(int32_t i=0; i<VarPagesCount; i++)
				{
					if (fwrite(pVarPages[i], sizeof(VarPage), 1, pFile) != 1)
					{
						goto ERROR_LABEL;
					}
				}
			}

			if(pBranchPages)
			{
				for(int32_t i=0; i<BranchPagesCount; i++)
				{
					if (fwrite(pBranchPages[i], sizeof(BranchPage), 1, pFile) != 1)
					{
						goto ERROR_LABEL;
					}
				}
			}

			if(pBlockPages)
			{
				for(int32_t i=0; i<BlockPagesCount; i++)
				{
					if (fwrite(pBlockPages[i], sizeof(BlockPage), 1, pFile) != 1)
					{
						goto ERROR_LABEL;
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
		else
		{
			printf("File '%s' is not opened.", path);
		}

	ERROR_LABEL:

		return false;
	}

	bool loadFromFile(const char* path)
	{
		FILE* pFile = 0;

		#ifdef _WIN32
        errno_t errorCode = fopen_s(&pFile, path, "rb");
        if (!errorCode)
  		#endif // _WIN32

		#if defined linux || defined __APPLE__
		pFile = fopen(path, "rb");
		if(pFile != NULL)
        #endif // linux
  		{
			if (fread(this, sizeof(HArray), 1, pFile) != 1)
			{
				goto ERROR_LABEL;
			}

    		if(pHeader)
			{
				pHeader = new int32_t[HeaderSize];

				if(fread (pHeader, sizeof(int32_t), HeaderSize, pFile) != HeaderSize)
				{
					goto ERROR_LABEL;
				}
			}

			if(pContentPages)
			{
				pContentPages = new ContentPage*[ContentPagesCount];
				ContentPagesSize = ContentPagesCount;

				for(int32_t i=0; i<ContentPagesCount; i++)
				{
					pContentPages[i] = new ContentPage();

					if (fread(pContentPages[i], sizeof(ContentPage), 1, pFile) != 1)
					{
						goto ERROR_LABEL;
					}
				}
			}

			if(pVarPages)
			{
				pVarPages = new VarPage*[VarPagesCount];
				VarPagesSize = VarPagesCount;

				for(int32_t i=0; i<VarPagesCount; i++)
				{
					pVarPages[i] = new VarPage();

					if (fread(pVarPages[i], sizeof(VarPage), 1, pFile) != 1)
					{
						goto ERROR_LABEL;
					}
				}
			}

			if(pBranchPages)
			{
				pBranchPages = new BranchPage*[BranchPagesCount];
				BranchPagesSize = BranchPagesCount;

				for(int32_t i=0; i<BranchPagesCount; i++)
				{
					pBranchPages[i] = new BranchPage();

					if (fread(pBranchPages[i], sizeof(BranchPage), 1, pFile) != 1)
					{
						goto ERROR_LABEL;
					}
				}
			}

			if(pBlockPages)
			{
				pBlockPages = new BlockPage*[BlockPagesCount];
				BlockPagesSize = BlockPagesCount;

				for(int32_t i=0; i<BlockPagesCount; i++)
				{
					pBlockPages[i] = new BlockPage();

					if (fread(pBlockPages[i], sizeof(BlockPage), 1, pFile) != 1)
					{
						goto ERROR_LABEL;
					}
				}
			}

			fclose (pFile);

			return true;
  		}
		else
		{
			printf("File '%s' is not opened.", path);
		}

	ERROR_LABEL:
		destroy();

		return false;
	}

	uint64_t getHeaderSize()
	{
		return (uint64_t)HeaderSize * sizeof(int32_t);
	}

	uint64_t getContentSize()
	{
		return (uint64_t)ContentPagesCount * sizeof(ContentPage);
	}

	uint64_t getVarSize()
	{
		return (uint64_t)VarPagesCount * sizeof(VarPage);
	}

	uint64_t getBranchSize()
	{
		return (uint64_t)BranchPagesCount * sizeof(BranchPage);
	}

	uint64_t getBlockSize()
	{
		return (uint64_t)BlockPagesCount * sizeof(BlockPage);
	}

	uint64_t getUsedMemory()
	{
		return	getHeaderSize() +
				getContentSize() +
				getVarSize() +
				getBranchSize() +
				getBlockSize();
	}

	uint64_t getTotalMemory()
	{
		return	getHeaderSize() +
				getContentSize() +
				getVarSize() +
				getBranchSize() +
				getBlockSize();
				//valueListPool.getTotalMemory();
	}

	//str comparator =====================================================
	static int32_t NormalizeStr(void* key)
	{
		//swap bytes
		int32_t num = ((int32_t*)key)[0];

		return (num >> 24) |			 // move byte 3 to byte 0
			   ((num << 8) & 0xff0000) | // move byte 1 to byte 2
			   ((num >> 8) & 0xff00) |	 // move byte 2 to byte 1
			   (num << 24);			     // byte 0 to byte 3
	}

	static int CompareSegmentStr(void* keySeg1, void* keySeg2, int32_t index)
	{
		return memcmp(keySeg1, keySeg2, 4);
	}

	static int CompareStr(void* key1, int32_t keyLen1,
						  void* key2, int32_t keyLen2)
	{
		return strcmp((char*)key1, (char*)key2);
	}

	//int comparator =====================================================

	static int32_t NormalizeInt32(void* key)
	{
		int num = ((int*)key)[0];

		if (num < 0)
		{
			return 2147483647 - (num * -1);
		}
		else
		{
			return (int32_t)num + 2147483647;
		}
	}

	static int CompareSegmentInt32(void* keySeg1, void* keySeg2, int32_t index)
	{
		if (((int*)keySeg1)[0] < ((int*)keySeg2)[0])
			return -1;

		if (((int*)keySeg1)[0] > ((int*)keySeg2)[0])
			return 1;

		return 0;
	}

	static int CompareInt32(void* key1, int32_t keyLen1,
						    void* key2, int32_t keyLen2)
	{
		int32_t keyLen = keyLen1 < keyLen2 ? keyLen1 : keyLen2;

		for (int32_t i = 0; i < keyLen; i++)
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

	static int32_t NormalizeFloat(void* key)
	{
		int32_t dw = *(int32_t*)key;

		if (dw >> 31)
		{
			return 0xFFFFFFFF - dw;
		}
		else
		{
			return 0x7FFFFFFF + dw;
		}
	}

	static int CompareSegmentFloat(void* keySeg1, void* keySeg2, int32_t index)
	{
		if (((float*)keySeg1)[0] < ((float*)keySeg2)[0])
			return -1;

		if (((float*)keySeg1)[0] > ((float*)keySeg2)[0])
			return 1;

		return 0;
	}

	static int CompareFloat(void* key1, int32_t keyLen1,
							void* key2, int32_t keyLen2)
	{
		int32_t keyLen = keyLen1 < keyLen2 ? keyLen1 : keyLen2;

		for (int32_t i = 0; i < keyLen; i++)
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

	//int32_t comparator =====================================================
	static int CompareSegmentUInt32(void* keySeg1, void* keySeg2, int32_t index)
	{
		if (((int32_t*)keySeg1)[0] < ((int32_t*)keySeg2)[0])
			return -1;

		if (((int32_t*)keySeg1)[0] > ((int32_t*)keySeg2)[0])
			return 1;

		return 0;
	}

	static int CompareUInt32(void* key1, int32_t keyLen1,
						     void* key2, int32_t keyLen2)
	{
		int32_t keyLen = keyLen1 < keyLen2 ? keyLen1 : keyLen2;

		for (int32_t i = 0; i < keyLen; i++)
		{
			if (((int32_t*)key1)[i] < ((int32_t*)key2)[i])
				return -1;

			if (((int32_t*)key1)[i] > ((int32_t*)key2)[i])
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
		printf("Header size: %llu\n", (long long unsigned int)getHeaderSize());
		printf("Content size: %llu\n", (long long unsigned int)getContentSize());
		printf("Var size: %llu\n", (long long unsigned int)getVarSize());
		printf("Branch size: %llu\n", (long long unsigned int)getBranchSize());
		printf("Block size: %llu\n", (long long unsigned int)getBlockSize());
		printf("Total size: %llu\n", (long long unsigned int)getTotalMemory());
	}

	void printStat()
	{
		#ifndef _RELEASE

		printf("=================== STAT =========================\n");
		for (int32_t i = 0; i<COUNT_TEMPS; i++)
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
        int32_t headerBase = this->HeaderBase;

		destroy();

		init(headerBase);
	}

	//types: 0-empty, 1..4 branches, 5 value, 6..9 blocks offset, 10 empty branch, 11 value
#ifndef _RELEASE

	int32_t tempValues[COUNT_TEMPS];
	char* tempCaptions[COUNT_TEMPS];

#endif

	void reallocateContentPages()
	{
		int32_t newSizeContentPages = ContentPagesSize * 2;
		ContentPage** pTempContentPages = new ContentPage*[newSizeContentPages];

		int32_t j=0;
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
		int32_t newSizeVarPages = VarPagesSize * 2;
		VarPage** pTempVarPages = new VarPage*[newSizeVarPages];

		int32_t j=0;
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
		int32_t newSizeBranchPages = BranchPagesSize * 2;
		BranchPage** pTempBranchPages = new BranchPage*[newSizeBranchPages];

		int32_t j=0;
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
		int32_t newSizeBlockPages = BlockPagesSize * 2;
		BlockPage** pTempBlockPages = new BlockPage*[newSizeBlockPages];

		int32_t j=0;
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

	bool insert(int32_t* key, int32_t keyLen, int32_t value);

	//GET =============================================================================================================

	bool getValueByKey(int32_t* key, int32_t keyLen, int32_t& value);

	//HAS =============================================================================================================

	bool hasPartKey(int32_t* key, int32_t keyLen);

	//DELL =============================================================================================================

	bool delValueByKey(int32_t* key, int32_t keyLen);

	//REBUILD =========================================================================================================

	static bool rebuildVisitor(int32_t* key, int32_t keyLen, int32_t value, void* pData);

	int32_t rebuild(int32_t headerBase = 0, bool removeEmptyKeys = false);

	//RANGE keys and values =============================================================================================================
	void sortLastItem(HArrayPair* pairs,
					  int32_t count);

	void getKeysAndValuesByRangeFromBlock(HArrayPair* pairs,
										  int32_t& count,
										  int32_t size,
										  int32_t contentOffset,
										  int32_t keyOffset,
										  int32_t blockOffset,
										  int32_t* minKey,
										  int32_t minKeyLen,
										  int32_t* maxKey,
										  int32_t maxKeyLen);

	void getKeysAndValuesByRange(HArrayPair* pairs,
								 int32_t& count,
								 int32_t size,
								 int32_t keyOffset,
								 int32_t contentOffset,
								 int32_t* minKey,
								 int32_t minKeyLen,
								 int32_t* maxKey,
								 int32_t maxKeyLen);

	int32_t getKeysAndValuesByRange(HArrayPair* pairs,
								int32_t size,
								int32_t* minKey,
								int32_t minKeyLen,
								int32_t* maxKey,
								int32_t maxKeyLen);

	//TEMPLATE ====================================================================================================
	//SCAN BY VISITOR
	void scanKeysAndValuesFromBlock(int32_t* key,
									int32_t contentOffset,
									int32_t keyOffset,
									int32_t blockOffset,
									HARRAY_ITEM_VISIT_FUNC visitor,
									void* pData);

	void scanKeysAndValues(int32_t* key,
						   int32_t keyOffset,
						   int32_t contentOffset,
						   HARRAY_ITEM_VISIT_FUNC visitor,
						   void* pData);

	void scanKeysAndValues(int32_t* key,
						 int32_t keyLen,
						 HARRAY_ITEM_VISIT_FUNC visitor,
						 void* pData);

	void scanKeysAndValues(HARRAY_ITEM_VISIT_FUNC visitor,
							 void* pData);

	//DISMANTLING ====================================================================================================

	void autoShrinkIfCouldBeReleasedAtLeast(int32_t percents)
	{
		autoShrinkOnPercents = percents;
	}

	void releaseContentCells(int32_t* pContentCell, int32_t contentOffset, int32_t len);
	void releaseBranchCell(BranchCell* pBranchCell, int32_t branchOffset);
	void releaseVarCell(VarCell* pVarCell, int32_t varOffset);
	void releaseBlockCells(BlockCell* pBlockCell, int32_t startBlockOffset);

	void shrinkContentPages();
	void shrinkBranchPages();
	void shrinkBlockPages();
	void shrinkVarPages();

	void shrink();

	int32_t* tailReleasedContentOffsets;
	int32_t tailReleasedBranchOffset;
	int32_t tailReleasedBlockOffset;
	int32_t tailReleasedVarOffset;

	int32_t countReleasedContentCells;
	int32_t countReleasedBranchCells;
	int32_t countReleasedBlockCells;
	int32_t countReleasedVarCells;

	bool tryReleaseBlock(SegmentPath* path, int32_t pathLen, int32_t& currPathLen);
	bool dismantling(SegmentPath* path, int32_t pathLen);
	bool dismantlingContentCells(SegmentPath* path, int32_t& currPathLen);
	int32_t moveContentCells(int32_t& startContentOffset,
							ContentPage** newContentPages,
							int32_t& countNewContentPages,
							int32_t shrinkLastContentOffset,
							int32_t* lastContentOffsetOnNewPages);

	//for testing
	bool testContentConsistency();
	bool testBlockConsistency();
	bool testBranchConsistency();
	bool testVarConsistency();

	bool testFillContentPages();
	bool testFillBlockPages();
	bool testFillBranchPages();
	bool testFillVarPages();

	int32_t getFullContentLen(int32_t contentOffset);
	bool shrinkBlock(int32_t startBlockOffset,
					 int32_t shrinkLastBlockOffset);

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
			for(int32_t i=0; i<ContentPagesCount; i++)
			{
				delete pContentPages[i];
			}

			delete[] pContentPages;
			pContentPages = 0;
		}

		if(pVarPages)
		{
			for(int32_t i=0; i<VarPagesCount; i++)
			{
				delete pVarPages[i];
			}

			delete[] pVarPages;
			pVarPages = 0;
		}

		if(pBranchPages)
		{
			for(int32_t i=0; i<BranchPagesCount; i++)
			{
				delete pBranchPages[i];
			}

			delete[] pBranchPages;
			pBranchPages = 0;
		}

		if(pBlockPages)
		{
			for(int32_t i=0; i<BlockPagesCount; i++)
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
	}
};
