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
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <map>
#include <unordered_map>
#include "HArrayInt.h"
#include "HArray.h"
#include "HArrayGeneric.h"

//======== NATURE OF KEYS =============
#define SEQUENCE_TESTS
#define RANDOM_TESTS
#define PERIOD_TESTS

//======== LENGTH OF KEYS =============
#define KEY_INT_TESTS
#define KEY_BIN_TESTS
#define KEY_STR_TESTS

//======== CONTAINERS =================
#define HARRAY_INT_TESTS
#define HARRAY_TESTS
//#define DENSE_HASH_MAP_TESTS //uncomment if you install google::dense_hash_map
#define STD_MAP_TESTS
#define STD_UNORDERED_MAP_TESTS

//======== INFO =======================
//#define PRINT_MEM
//#define PRINT_STAT
//#define CONSISTENCY_TESTS

#ifdef DENSE_HASH_MAP_TESTS
#include <google/dense_hash_map>
#endif

using namespace std;

uint32 totalHArrayTime = 0;
uint32 totalDenseTime = 0;
uint32 totalMapTime = 0;
uint32 totalUnorderedMapTime = 0;

clock_t msclock()
{
	return (uint64_t)clock() * 1000 / CLOCKS_PER_SEC; //in ms
}


//==== HArrayInt - Int Keys ===========================================================================================

void testHArrayInt(uint32* keys, uint32 countKeys)
{
	#ifdef HARRAY_INT_TESTS

	printf("HArrayInt => ");

	HArrayInt ha;
	ha.init(26);

	clock_t start, finish;

	//INSERT ===========================================

	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		ha.insert(keys[i], keys[i]);
	}

	finish = msclock();

	printf("Insert: %d msec, ", (int)(finish - start));

	totalHArrayTime += (finish - start);

	//SEARCH ===========================================
	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		if (ha.getValueByKey(keys[i]) != keys[i])
		{
			printf("Error\n");
			break;
		}
	}

	finish = msclock();

	//uint32 memoryInMb = ha.getTotalMemory() / 1024 / 1024;

	printf("Search: %d msec.\n", (int)(finish - start));

	//printf("Memory: %d mb.\n", memoryInMb);

	totalHArrayTime += (finish - start);

	//ha.print();

	ha.destroy();

	#endif
}

void testStdMapInt(uint32* keys, uint32 countKeys)
{
	#ifdef STD_MAP_TESTS

	printf("std::map => ");

	std::map<uint32, uint32> mymap;

	clock_t start, finish;

	//INSERT ===========================================

	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		mymap[keys[i]] = keys[i];
	}

	finish = msclock();

	printf("Insert: %d msec, ", (int)(finish - start));

	totalMapTime += (finish - start);

	//SEARCH ===========================================
	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		if (mymap[keys[i]] != keys[i])
		{
			printf("Error\n");
			break;
		}
	}

	finish = msclock();

	printf("Search: %d msec.\n", (int)(finish - start));

	totalMapTime += (finish - start);

	//ha.print();

	#endif
}

void testDenseHashMapInt(uint32* keys, uint32 countKeys)
{
	#ifdef DENSE_HASH_MAP_TESTS

	printf("google::dense_hash_map => ");

	google::dense_hash_map<uint32, uint32> mymap;
	mymap.set_empty_key(-1);

	clock_t start, finish;

	//INSERT ===========================================

	start = msclock();

	for (int i = 0; i < countKeys; i++)
	{
		mymap[keys[i]] = keys[i];
	}

	finish = msclock();

	printf("Insert: %d msec, ", (finish - start));

	totalDenseTime += (finish - start);

	//SEARCH ===========================================
	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		if (mymap[keys[i]] != keys[i])
		{
			printf("Error\n");
			break;
		}
	}

	finish = msclock();

	printf("Search: %d msec.\n", (finish - start));

	totalDenseTime += (finish - start);

	//ha.print();

	#endif
}

void testStdUnorderedMapInt(uint32* keys, uint32 countKeys)
{
	#ifdef STD_UNORDERED_MAP_TESTS

	printf("std::unordered_map => ");

	std::unordered_map<uint32, uint32> mymap;

	clock_t start, finish;

	//INSERT ===========================================

	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		mymap[keys[i]] = keys[i];
	}

	finish = msclock();

	printf("Insert: %d msec, ", (int)(finish - start));

	totalUnorderedMapTime += (finish - start);

	//SEARCH ===========================================
	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		if (mymap[keys[i]] != keys[i])
		{
			printf("Error\n");
			break;
		}
	}

	finish = msclock();

	printf("Search: %d msec.\n", (int)(finish - start));

	totalUnorderedMapTime += (finish - start);

	//ha.print();

	#endif
}

void fillSeqInts(uint32* keys, uint32 countKeys)
{
	for (uint32 i = 0; i < countKeys; i++)
	{
		keys[i] = i;
	}
}

void fillRandInts(uint32* keys, uint32 countKeys)
{
	for (uint32 i = 0; i < countKeys; i++)
	{
		keys[i] = rand();
	}
}

void fillPeriodInts(uint32* keys, uint32 countKeys)
{
	for (uint32 i = 0; i < countKeys; i++)
	{
		keys[i] = i * 17;
	}
}

void HArrayInt_VS_StdMap_IntKey(uint32 startOnAmount, uint32 stepOfAmount, uint32 stopOnAmount)
{
	#ifdef KEY_INT_TESTS

	printf("=== HArrayInt VS google::dense_hash_map<int,int> VS std::map<int,int> VS std::ordinary_map<int,int> testing===\n");

	uint32* intKeys = new uint32[stopOnAmount];

	#ifdef SEQUENCE_TESTS

	fillSeqInts(intKeys, stopOnAmount);

	for (uint32 countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search %u SEQUENCE keys (%llu bytes each) ...\n", countKeys, (long long unsigned int)sizeof(uint32));
		testHArrayInt(intKeys, countKeys);
		testDenseHashMapInt(intKeys, countKeys);
		testStdMapInt(intKeys, countKeys);
		testStdUnorderedMapInt(intKeys, countKeys);
		printf("\n");
	}

	#endif

	#ifdef RANDOM_TESTS

	fillRandInts(intKeys, stopOnAmount);

	for (uint32 countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search %u RANDOM keys (%llu bytes each) ...\n", countKeys, (long long unsigned int)sizeof(uint32));
		testHArrayInt(intKeys, countKeys);
		testDenseHashMapInt(intKeys, countKeys);
		testStdMapInt(intKeys, countKeys);
		testStdUnorderedMapInt(intKeys, countKeys);
		printf("\n");
	}

	#endif

	#ifdef PERIOD_TESTS

	fillPeriodInts(intKeys, stopOnAmount);

	for (uint32 countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search %u PERIOD keys (%llu bytes each) ...\n", countKeys, (long long unsigned int)sizeof(uint32));
		testHArrayInt(intKeys, countKeys);
		testDenseHashMapInt(intKeys, countKeys);
		testStdMapInt(intKeys, countKeys);
		testStdUnorderedMapInt(intKeys, countKeys);
		printf("\n");
	}

	#endif

	delete[] intKeys;

	#endif
}

//==== HArray - Binary Keys ===========================================================================================

const uint32 BIN_KEY_LEN = 4;

struct BinKey
{
public:
	uint32 Data[BIN_KEY_LEN]; //16 bytes max

	bool operator==(const BinKey &other) const
  	{
  		for(uint32 i=0; i<BIN_KEY_LEN; i++)
  		{
  			if(Data[i] != other.Data[i])
  				return false;
  		}

  		return true;
  	}
};

struct BinKeyHasher
{
  std::size_t operator()(const BinKey& k) const
  {
    return k.Data[BIN_KEY_LEN - 1];
  }
};

bool operator<(const BinKey& a, const BinKey& b)
{
	for (uint32 i = 0; i < BIN_KEY_LEN; i++)
	{
		if (a.Data[i] == b.Data[i])
			continue;
		else
			return a.Data[i] < b.Data[i];
	}

	return false;
}

void shuffleBins(BinKey* keys, uint32 countKeys)
{
	for (uint32 i = 0; i < countKeys; i++)
	{
		uint32 idx1 = rand() % countKeys;
		uint32 idx2 = rand() % countKeys;

		BinKey tempKey = keys[idx1];
		keys[idx1] = keys[idx2];
		keys[idx2] = tempKey;
	}
}

void testHArrayBin(BinKey* keys, uint32 countKeys, bool shuffle)
{
	#ifdef HARRAY_TESTS

	printf("HArray => ");

	HArray ha;
	ha.init(14);

	clock_t start, finish;

	if (shuffle)
	{
		shuffleBins(keys, countKeys);
	}

	//INSERT ===========================================

	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		ha.insert((uint32*)keys[i].Data, BIN_KEY_LEN, keys[i].Data[0]);
	}

	finish = msclock();

	printf("Insert: %d msec, ", (int)(finish - start));

	totalHArrayTime += (finish - start);

	if (shuffle)
	{
		shuffleBins(keys, countKeys);
	}

	//SEARCH ===========================================
	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		uint32 value;

		ha.getValueByKey((uint32*)keys[i].Data, BIN_KEY_LEN, value);

		if (value != keys[i].Data[0])
		{
			printf("Error\n");
			break;
		}
	}

	finish = msclock();

	printf("Search: %d msec, ", (int)(finish - start));

	totalHArrayTime += (finish - start);

	if(!ha.testContentConsistency())
	{
		printf("\n==========> testContentConsistency failed !!!\n");

		return;
	}

	uint32 memoryInMb = (uint32)ha.getTotalMemory() / 1024 / 1024;

	//DELETE ===========================================

	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		ha.delValueByKey((uint32*)keys[i].Data, BIN_KEY_LEN);

		#ifdef CONSISTENCY_TESTS

		//if(i >= 213573)
		if (i % 10000 == 0)
		{
			printf("%u\n", i);

			if (!ha.testFillContentPages())
			{
				printf("\n!!! 111111 testFillContentPages failed !!!\n");

				return;
			}

			if (!ha.testFillBlockPages())
			{
				printf("\n!!! 111111 testBlockPages failed !!!\n");

				return;
			}

			if (!ha.testFillBranchPages())
			{
				printf("\n!!! 111111 testFillBranchPages failed !!!\n");

				return;
			}

			if (!ha.testContentConsistency())
			{
				printf("\n!!! testContentConsistency failed !!!\n");

				return;
			}

			if (!ha.testBranchConsistency())
			{
				printf("\n!!! testBranchConsistency failed !!!\n");

				return;
			}

			if (!ha.testBlockConsistency())
			{
				printf("\n!!! testBlockConsistency failed !!!\n");

				return;
			}
		}

		#endif
	}

	finish = msclock();

	printf("Delete: %d msec., ", (int)(finish - start));

	printf("Memory: %d mb.\n", memoryInMb);

	//totalHArrayTime += (finish - start);

	#ifdef PRINT_MEM
	ha.printMemory();
	#endif

	#ifdef PRINT_STAT
	ha.printStat();
	#endif

	ha.destroy();

	#endif
}

void testStdMapBin(BinKey* keys, uint32 countKeys, bool shuffle)
{
	#ifdef STD_MAP_TESTS

	printf("std::map => ");

	std::map<BinKey, uint32> mymap;

	clock_t start, finish;

	if (shuffle)
	{
		shuffleBins(keys, countKeys);
	}

	//INSERT ===========================================

	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		mymap[keys[i]] = keys[i].Data[0];
	}

	finish = msclock();

	printf("Insert: %d msec, ", (int)(finish - start));

	totalMapTime += (finish - start);

	if (shuffle)
	{
		shuffleBins(keys, countKeys);
	}

	//SEARCH ===========================================
	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		if (mymap[keys[i]] != keys[i].Data[0])
		{
			printf("Error\n");
			break;
		}
	}

	finish = msclock();

	printf("Search: %d msec.\n", (int)(finish - start));

	totalMapTime += (finish - start);

	//ha.print();

	#endif
}

void testDenseHashMapBin(BinKey* keys, uint32 countKeys, bool shuffle)
{
	#ifdef DENSE_HASH_MAP_TESTS

	printf("google::dense_hash_map => ");

	google::dense_hash_map<BinKey, uint32, BinKeyHasher> mymap;

	BinKey emptyKey;
	memset(&emptyKey, 0, sizeof(BinKey));
	emptyKey.Data[0] = 1;

	mymap.set_empty_key(emptyKey);

	clock_t start, finish;

	if (shuffle)
	{
		shuffleBins(keys, countKeys);
	}

	//INSERT ===========================================

	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		mymap[keys[i]] = keys[i].Data[0];
	}

	finish = msclock();

	printf("Insert: %d msec, ", (finish - start));

	totalDenseTime += (finish - start);

	if (shuffle)
	{
		shuffleBins(keys, countKeys);
	}

	//SEARCH ===========================================
	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		if (mymap[keys[i]] != keys[i].Data[0])
		{
			printf("Error\n");
			break;
		}
	}

	finish = msclock();

	printf("Search: %d msec.\n", (finish - start));

	totalDenseTime += (finish - start);

	#endif
	//ha.print();
}

void testStdUnorderedMapBin(BinKey* keys, uint32 countKeys, bool shuffle)
{
	#ifdef STD_UNORDERED_MAP_TESTS

	printf("std::unordered_map => ");

	std::unordered_map<BinKey, uint32, BinKeyHasher> mymap;

	clock_t start, finish;

	if (shuffle)
	{
		shuffleBins(keys, countKeys);
	}

	//INSERT ===========================================

	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		mymap[keys[i]] = keys[i].Data[0];
	}

	finish = msclock();

	printf("Insert: %d msec, ", (int)(finish - start));

	totalUnorderedMapTime += (finish - start);

	if (shuffle)
	{
		shuffleBins(keys, countKeys);
	}

	//SEARCH ===========================================
	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		if (mymap[keys[i]] != keys[i].Data[0])
		{
			printf("Error\n");
			break;
		}
	}

	finish = msclock();

	printf("Search: %d msec.\n", (int)(finish - start));

	totalUnorderedMapTime += (finish - start);

	#endif
	//ha.print();
}

void fillSeqBins(BinKey* keys, uint32 countKeys)
{
	for (uint32 i = 0; i < countKeys; i++)
	{
		for (uint32 j = 0; j < BIN_KEY_LEN - 1; j++)
		{
			keys[i].Data[j] = 0;
		}

		keys[i].Data[3] = i;
	}
}

void fillRandBins(BinKey* keys, uint32 countKeys)
{
	for (uint32 i = 0; i < countKeys; i++)
	{
		for (uint32 j = 0; j < BIN_KEY_LEN; j++)
		{
			keys[i].Data[j] = (uint32)rand() * (uint32)rand();
		}
	}
}

void fillPeriodBins(BinKey* keys, uint32 countKeys)
{
	for (uint32 i = 0; i < countKeys; i++)
	{
		for (uint32 j = 0; j < BIN_KEY_LEN; j++)
		{
			keys[i].Data[j] = i * 17;
		}
	}
}

void HArray_VS_StdMap_BinKey(uint32 startOnAmount, uint32 stepOfAmount, uint32 stopOnAmount, bool shuffle = false)
{
	#ifdef KEY_BIN_TESTS

	printf("=== HArray VS google::dense_hash_map<BinKey, int> VS std::map<BinKey,int> VS std::ordinary_map<BinKey,int> testing ===\n");

	BinKey* binKeys = new BinKey[stopOnAmount];

	#ifdef SEQUENCE_TESTS

	fillSeqBins(binKeys, stopOnAmount);

	for (uint32 countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search/Delete %u SEQUENCE keys (%llu bytes each) ...\n", countKeys, (long long unsigned int)sizeof(BinKey));
		testHArrayBin(binKeys, countKeys, shuffle);
		testDenseHashMapBin(binKeys, countKeys, shuffle);
		testStdMapBin(binKeys, countKeys, shuffle);
		testStdUnorderedMapBin(binKeys, countKeys, shuffle);
		printf("\n");
	}

	#endif

	#ifdef RANDOM_TESTS

	fillRandBins(binKeys, stopOnAmount);

	for (uint32 countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search/Delete %u RANDOM keys (%llu bytes each) ...\n", countKeys, (long long unsigned int)sizeof(BinKey));
		testHArrayBin(binKeys, countKeys, shuffle);
		testDenseHashMapBin(binKeys, countKeys, shuffle);
		testStdMapBin(binKeys, countKeys, shuffle);
		testStdUnorderedMapBin(binKeys, countKeys, shuffle);
		printf("\n");
	}

	#endif

	#ifdef PERIOD_TESTS

	fillPeriodBins(binKeys, stopOnAmount);

	for (uint32 countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search/Delete %u PERIOD keys (%llu bytes each) ...\n", countKeys, (long long unsigned int)sizeof(BinKey));
		testHArrayBin(binKeys, countKeys, shuffle);
		testDenseHashMapBin(binKeys, countKeys, shuffle);
		testStdMapBin(binKeys, countKeys, shuffle);
		testStdUnorderedMapBin(binKeys, countKeys, shuffle);
		printf("\n");
	}

	#endif

	delete[] binKeys;

	#endif
}

//==== HArray - String Keys ===========================================================================================

const char STR_KEY_LEN = 64 / 4;

static const char alphanum[] =
"0123456789"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz";

void fillRand(char* str, const uint32 len)
{
	uint32 i = 0;

	for (; i < len - 1; i++)
	{
		str[i] = alphanum[rand() % (sizeof(alphanum)-1)];
	}

	str[i] = 0;
}

void testHArrayStr(std::string* keys, uint32 countKeys)
{
	#ifdef HARRAY_TESTS

	printf("HArray => ");

	HArray ha;
	ha.init(26);

	clock_t start, finish;

	//INSERT ===========================================

	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		const char* str = keys[i].c_str();

		ha.insert((uint32*)str, STR_KEY_LEN, str[0]);
	}

	finish = msclock();

	printf("Insert: %d msec, ", (int)(finish - start));

	totalHArrayTime += (finish - start);

	//SEARCH ===========================================
	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		const char* str = keys[i].c_str();

		uint32 value;

		ha.getValueByKey((uint32*)str, STR_KEY_LEN, value);

		if (value != (uint32)str[0])
		{
			printf("Error\n");
			break;
		}
	}

	finish = msclock();

	printf("Search: %d msec, ", (int)(finish - start));

	totalHArrayTime += (finish - start);

	uint32 memoryInMb = (uint32)ha.getTotalMemory() / 1024 / 1024;

	//DELETE ===========================================

	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		const char* str = keys[i].c_str();

		ha.delValueByKey((uint32*)str, STR_KEY_LEN);

		#ifdef CONSISTENCY_TESTS

		if (i % 10000 == 0)
		{
			printf("%u\n", i);

			if (!ha.testFillContentPages())
			{
				printf("\n!!! 111111 testFillContentPages failed !!!\n");

				return;
			}

			if (!ha.testFillBlockPages())
			{
				printf("\n!!! 111111 testBlockPages failed !!!\n");

				return;
			}

			if (!ha.testFillBranchPages())
			{
				printf("\n!!! 111111 testFillBranchPages failed !!!\n");

				return;
			}

			if(!ha.testContentConsistency())
			{
				printf("\n!!! testContentConsistency failed !!!\n");

				return;
			}

			if(!ha.testBranchConsistency())
			{
				printf("\n!!! testBranchConsistency failed !!!\n");

				return;
			}

			if(!ha.testBlockConsistency())
			{
				printf("\n!!! testBlockConsistency failed !!!\n");

				return;
			}
		}

		#endif
	}

	finish = msclock();

	printf("Delete: %d msec, ", (int)(finish - start));

	printf("Memory: %d mb.\n", memoryInMb);

	//totalHArrayTime += (finish - start);

	#ifdef PRINT_MEM
	ha.printMemory();
	#endif

	#ifdef PRINT_STAT
	ha.printStat();
	#endif


	ha.destroy();

	#endif
}

void testHArrayStrVar(std::string* keys, uint32 countKeys)
{
#ifdef HARRAY_TESTS

	printf("HArray => ");

	HArray ha;
	ha.init(26);

	clock_t start, finish;

	//INSERT ===========================================

	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		const char* str = keys[i / 15].c_str();

		ha.insert((uint32*)str, (i % 15) + 1, str[0]);
	}

	finish = msclock();

	printf("Insert: %d msec, ", (int)(finish - start));

	//totalHArrayTime += (finish - start);

	//SEARCH ===========================================
	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		const char* str = keys[i / 15].c_str();

		uint32 value;

		ha.getValueByKey((uint32*)str, (i % 15) + 1, value);

		if (value != (uint32)str[0])
		{
			printf("Error\n");
			break;
		}
	}

	finish = msclock();

	printf("Search: %d msec, ", (int)(finish - start));

	//totalHArrayTime += (finish - start);

	uint32 memoryInMb = (uint32)ha.getTotalMemory() / 1024 / 1024;

//DELETE ===========================================

	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		const char* str = keys[i / 15].c_str();

		ha.delValueByKey((uint32*)str, (i % 15) + 1);

		/*
		if (i >= 2849)
		if (i % 1 == 0)
		{
			printf("%u\n", i);

			uint32 j = 0;
			for (; j <= i; j++)
			{
				const char* str2 = keys[j / 15].c_str();

				if (ha.getValueByKey((uint32*)str2, (j % 15) * 4 + 4) != 0)
				{
					printf("Error\n");
					return;
				}
			}

			for (; j < countKeys; j++)
			{
				if (j == 520170)
				{
					j = 520170;
				}

				const char* str2 = keys[j / 15].c_str();

				if (*ha.getValueByKey((uint32*)str2, (j % 15) * 4 + 4) != str2[0])
				{
					printf("Error\n");
					return;
				}
			}
		}
		*/

#ifdef CONSISTENCY_TESTS

		//if (i >= 2849)
		if (i % 1000 == 0)
		{
			printf("%u\n", i);

			if (!ha.testFillContentPages())
			{
				printf("\n!!! testFillContentPages failed !!!\n");

				return;
			}

			if (!ha.testFillBlockPages())
			{
				printf("\n!!! testBlockPages failed !!!\n");

				return;
			}

			if (!ha.testFillBranchPages())
			{
				printf("\n!!! testFillBranchPages failed !!!\n");

				return;
			}

			if (!ha.testFillVarPages())
			{
				printf("\n!!! testFillVarPages failed !!!\n");

				return;
			}

			if (!ha.testContentConsistency())
			{
				printf("\n!!! testContentConsistency failed !!!\n");

				return;
			}

			if (!ha.testBranchConsistency())
			{
				printf("\n!!! testBranchConsistency failed !!!\n");

				return;
			}

			if (!ha.testBlockConsistency())
			{
				printf("\n!!! testBlockConsistency failed !!!\n");

				return;
			}

			if (!ha.testVarConsistency())
			{
				printf("\n!!! testBlockConsistency failed !!!\n");

				return;
			}
		}

#endif
	}

	finish = msclock();

	printf("Delete: %d msec., ", (int)(finish - start));

	printf("Memory: %d mb.\n", memoryInMb);

	//totalHArrayTime += (finish - start);

#ifdef PRINT_MEM
	ha.printMemory();
#endif

#ifdef PRINT_STAT
	ha.printStat();
#endif

	ha.destroy();

#endif
}

void testStdMapStr(std::string* keys, uint32 countKeys)
{
	#ifdef STD_MAP_TESTS

	printf("std::map => ");

	std::map<std::string, uint32> mymap;

	clock_t start, finish;

	//INSERT ===========================================

	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		const char* str = keys[i].c_str();
		mymap[keys[i]] = str[0];
	}

	finish = msclock();

	printf("Insert: %d msec, ", (int)(finish - start));

	totalMapTime += (finish - start);

	//SEARCH ===========================================
	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		const char* str = keys[i].c_str();
		if (mymap[keys[i]] != (uint32)str[0])
		{
			printf("Error\n");
			break;
		}
	}

	finish = msclock();

	printf("Search: %d msec.\n", (int)(finish - start));

	totalMapTime += (finish - start);

	//ha.print();

	#endif
}

void testDenseHashMapStr(std::string* keys, uint32 countKeys)
{
	#ifdef DENSE_HASH_MAP_TESTS

	printf("google::dense_hash_map => ");

	google::dense_hash_map<std::string, uint32> mymap;

	mymap.set_empty_key("");

	clock_t start, finish;

	//INSERT ===========================================

	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		const char* str = keys[i].c_str();
		mymap[keys[i]] = str[0];
	}

	finish = msclock();

	printf("Insert: %d msec, ", (finish - start));

	totalDenseTime += (finish - start);

	//SEARCH ===========================================
	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		const char* str = keys[i].c_str();

		if (mymap[keys[i]] != str[0])
		{
			printf("Error\n");
			break;
		}
	}

	finish = msclock();

	printf("Search: %d msec.\n", (finish - start));

	totalDenseTime += (finish - start);

	#endif

	//ha.print();
}

void testStdUnorderedMapStr(std::string* keys, uint32 countKeys)
{
	#ifdef STD_UNORDERED_MAP_TESTS

	printf("std::unordered_map => ");

	std::unordered_map<std::string, uint32> mymap;

	clock_t start, finish;

	//INSERT ===========================================

	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		const char* str = keys[i].c_str();

		mymap[keys[i]] = str[0];
	}

	finish = msclock();

	printf("Insert: %d msec, ", (int)(finish - start));

	totalUnorderedMapTime += (finish - start);

	//SEARCH ===========================================
	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		const char* str = keys[i].c_str();

		if (mymap[keys[i]] != (uint32)str[0])
		{
			printf("Error\n");
			break;
		}
	}

	finish = msclock();

	printf("Search: %d msec.\n", (int)(finish - start));

	totalUnorderedMapTime += (finish - start);

	#endif

	//ha.print();
}

void fillSeqStrs(std::string* keys, uint32 countKeys)
{
	char key[STR_KEY_LEN + 1];

	for (uint32 i = 0; i < countKeys; i++)
	{
		for (uint32 j = 0; j < STR_KEY_LEN - 8; j++)
		{
			key[j] = '0';
		}

		fillRand(key + (STR_KEY_LEN - 8), 8);

		keys[i] = key;
	}
}

void fillRandStrs(std::string* keys, uint32 countKeys)
{
	char key[STR_KEY_LEN + 1];

	for (uint32 i = 0; i < countKeys; i++)
	{
		fillRand(key, STR_KEY_LEN);

		keys[i] = key;
	}
}

void HArray_VS_StdMap_StrKey(uint32 startOnAmount, uint32 stepOfAmount, uint32 stopOnAmount)
{
	#ifdef KEY_STR_TESTS

	printf("=== HArray VS google::dense_hash_map<StrKey, int> VS std::map<std::string,int> VS std::ordinary_map<std::string,int> testing ===\n");

	std::string* strKeys = new std::string[stopOnAmount];

	#ifdef SEQUENCE_TESTS

	fillSeqStrs(strKeys, stopOnAmount);

	for (uint32 countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search/Delete %u SIMILAR keys (%u bytes each) ...\n", countKeys, STR_KEY_LEN * 4);
		testHArrayStr(strKeys, countKeys);
		testDenseHashMapStr(strKeys, countKeys);
		testStdMapStr(strKeys, countKeys);
		testStdUnorderedMapStr(strKeys, countKeys);
		printf("\n");
	}

	#endif

	#ifdef RANDOM_TESTS

	fillRandStrs(strKeys, stopOnAmount);

	for (uint32 countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search/Delete %u RANDOM keys (%u bytes each) ...\n", countKeys, STR_KEY_LEN * 4);
		testHArrayStr(strKeys, countKeys);
		testDenseHashMapStr(strKeys, countKeys);
		testStdMapStr(strKeys, countKeys);
		testStdUnorderedMapStr(strKeys, countKeys);
		printf("\n");
	}

	#endif

	delete[] strKeys;

	#endif
}

void HArray_VS_StdMap_StrKey_Var(uint32 startOnAmount, uint32 stepOfAmount, uint32 stopOnAmount)
{
	#ifdef KEY_STR_TESTS

	printf("=== HArray VS google::dense_hash_map<StrKey, int> VS std::map<std::string,int> VS std::ordinary_map<std::string,int> testing ===\n");

	std::string* strKeys = new std::string[stopOnAmount];

	#ifdef RANDOM_TESTS

	fillRandStrs(strKeys, stopOnAmount);

	for (uint32 countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search/Delete %u RANDOM VAR keys (%u bytes each) ...\n", countKeys, STR_KEY_LEN);
		testHArrayStrVar(strKeys, countKeys);
		printf("\n");
	}

	#endif

	delete[] strKeys;

	#endif
}


int main()
{
	HArrayGeneric<std::string, std::string> ha;

	ha["dou"] = "hello";

	std::string sayHello = ha["dou"];
	
	return 0;
	HArrayInt_VS_StdMap_IntKey(1000000,   //start
								2000000,   //step
								10000000); //stop

	HArray_VS_StdMap_BinKey(1000000, //start
							2000000, //step
							10000000,//stop
							false);  //shuffle

	HArray_VS_StdMap_StrKey(1000000,  //start
							1000000,  //step
							3000000); //stop

	HArray_VS_StdMap_StrKey_Var(1000000,  //start
								1000000,  //step
								3000000); //stop

	printf("COEF HArray VS Map: In average HArray faster in %.2f times.\n", (double)totalMapTime / (double)totalHArrayTime);
	printf("COEF HArray VS Unordered Map: In average HArray faster in %.2f times.\n", (double)totalUnorderedMapTime / (double)totalHArrayTime);
	printf("COEF HArray VS Dense: In average HArray faster in %.2f times.\n", (double)totalDenseTime / (double)totalHArrayTime);

	return 0;
};
