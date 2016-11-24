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
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <map>
#include "HArrayInt.h"
#include "HArrayVarRAM.h"

//#define SEQUENCE_TESTS
#define RANDOM_TESTS
//#define PERIOD_TESTS
//#define DENSE_HASH_MAP_TESTS //ucomment if you install google::dense_hash_map
//#define STD_MAP_TESTS

#ifdef DENSE_HASH_MAP_TESTS
#include <google/dense_hash_map>
#endif

using namespace std;

uint32 totalHArrayTime = 0;
uint32 totalDenseTime = 0;
uint32 totalMapTime = 0;

clock_t msclock()
{
	return (ulong64)clock() * 1000 / CLOCKS_PER_SEC; //in ms
}


//#define PRINT_MEM
//#define PRINT_STAT

//==== HArrayInt - Int Keys ===========================================================================================

void testHArrayInt(uint32* keys, uint32 countKeys)
{
	printf("HArrayInt => ");

	HArrayInt ha;
	ha.init(26);

	clock_t start, finish;

	//INSERT ===========================================

	start = msclock();

	for (int i = 0; i < countKeys; i++)
	{
		ha.insert(keys[i], keys[i]);
	}

	finish = msclock();

	printf("Insert: %d msec, ", (finish - start));

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

	printf("Search: %d msec.\n", (finish - start));

	totalHArrayTime += (finish - start);

	//ha.print();

	ha.destroy();
}

void testStdMapInt(uint32* keys, uint32 countKeys)
{
	#ifdef STD_MAP_TESTS
	
	printf("std::map => ");

	std::map<uint32, uint32> mymap;

	clock_t start, finish;

	//INSERT ===========================================

	start = msclock();

	for (int i = 0; i < countKeys; i++)
	{
		mymap[keys[i]] = keys[i];
	}

	finish = msclock();

	printf("Insert: %d msec, ", (finish - start));

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

	printf("Search: %d msec.\n", (finish - start));

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
	printf("=== HArrayInt VS google::dense_hash_map<int,int> VS std::map<int,int> testing ===\n");

	uint32* intKeys = new uint32[stopOnAmount];

	#ifdef SEQUENCE_TESTS

	fillSeqInts(intKeys, stopOnAmount);

	for (uint32 countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search %u SEQUENCE keys (%u bytes each) ...\n", countKeys, sizeof(uint32));
		testHArrayInt(intKeys, countKeys);
		testDenseHashMapInt(intKeys, countKeys);
		testStdMapInt(intKeys, countKeys);
		printf("\n");
	}

	#endif

	#ifdef RANDOM_TESTS

	fillRandInts(intKeys, stopOnAmount);

	for (uint32 countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search %u RANDOM keys (%u bytes each) ...\n", countKeys, sizeof(uint32));
		testHArrayInt(intKeys, countKeys);
		testDenseHashMapInt(intKeys, countKeys);
		testStdMapInt(intKeys, countKeys);
		printf("\n");
	}

	#endif

	#ifdef PERIOD_TESTS

	fillPeriodInts(intKeys, stopOnAmount);

	for (uint32 countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search %u PERIOD keys (%u bytes each) ...\n", countKeys, sizeof(uint32));
		testHArrayInt(intKeys, countKeys);
		testDenseHashMapInt(intKeys, countKeys);
		testStdMapInt(intKeys, countKeys);
		printf("\n");
	}

	#endif

	delete[] intKeys;
}

//==== HArrayVarRAM - Binary Keys ===========================================================================================

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
	printf("HArrayVarRAM => ");

	HArrayVarRAM ha;
	ha.init(26);

	clock_t start, finish;

	if (shuffle)
	{
		shuffleBins(keys, countKeys);
	}

	//INSERT ===========================================

	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		ha.insert((uint32*)keys[i].Data, sizeof(BinKey), keys[i].Data[0]);
	}

	finish = msclock();

	printf("Insert: %d msec, ", (finish - start));

	totalHArrayTime += (finish - start);

	if (shuffle)
	{
		shuffleBins(keys, countKeys);
	}

	//SEARCH ===========================================
	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		if (ha.getValueByKey((uint32*)keys[i].Data, sizeof(BinKey)) != keys[i].Data[0])
		{
			printf("Error\n");
			break;
		}
	}

	finish = msclock();

	printf("Search: %d msec.\n", (finish - start));

	totalHArrayTime += (finish - start);

	#ifdef PRINT_MEM
	ha.printMemory();
	#endif

	#ifdef PRINT_STAT
	ha.printStat();
	#endif

	ha.destroy();
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

	printf("Insert: %d msec, ", (finish - start));

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

	printf("Search: %d msec.\n", (finish - start));

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

void HArrayVarRAM_VS_StdMap_BinKey(uint32 startOnAmount, uint32 stepOfAmount, uint32 stopOnAmount, bool shuffle = false)
{
	printf("=== HArrayVarRAM VS google::dense_hash_map<BinKey, int> VS std::map<BinKey,int> testing ===\n");

	BinKey* binKeys = new BinKey[stopOnAmount];

	#ifdef SEQUENCE_TESTS

	fillSeqBins(binKeys, stopOnAmount);

	for (uint32 countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search %u SEQUENCE keys (%u bytes each) ...\n", countKeys, sizeof(BinKey));
		testHArrayBin(binKeys, countKeys, shuffle);
		testDenseHashMapBin(binKeys, countKeys, shuffle);
		testStdMapBin(binKeys, countKeys, shuffle);
		printf("\n");
	}

	#endif

	#ifdef RANDOM_TESTS

	fillRandBins(binKeys, stopOnAmount);

	for (uint32 countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search %u RANDOM keys (%u bytes each) ...\n", countKeys, sizeof(BinKey));
		testHArrayBin(binKeys, countKeys, shuffle);
		testDenseHashMapBin(binKeys, countKeys, shuffle);
		testStdMapBin(binKeys, countKeys, shuffle);
		printf("\n");
	}

	#endif

	#ifdef PERIOD_TESTS

	fillPeriodBins(binKeys, stopOnAmount);

	for (uint32 countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search %u PERIOD keys (%u bytes each) ...\n", countKeys, sizeof(BinKey));
		testHArrayBin(binKeys, countKeys, shuffle);
		testDenseHashMapBin(binKeys, countKeys, shuffle);
		testStdMapBin(binKeys, countKeys, shuffle);
		printf("\n");
	}

	#endif

	delete[] binKeys;
}

//==== HArrayVarRAM - String Keys ===========================================================================================

const char STR_KEY_LEN = 64;

struct StrKey
{
public:
	char Data[STR_KEY_LEN]; //64 bytes max

	bool operator==(const StrKey &other) const
  	{ 
  		return strcmp(Data, other.Data) == 0;
  	}
};

#ifdef DENSE_HASH_MAP_TESTS

struct StrKeyHasher
{
  std::size_t operator()(const StrKey& k) const
  {
    return std::tr1::hash<const char*>()(k.Data);
  }
};

#endif

bool operator<(const StrKey& a, const StrKey& b)
{
	return (strcmp(a.Data, b.Data) < 0);
}

static const char alphanum[] =
"0123456789"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz";

void fillRand(char *str, const int len)
{
	uint32 i = 0;

	for (; i < len - 1; i++)
	{
		str[i] = alphanum[rand() % (sizeof(alphanum)-1)];
	}

	str[i] = 0;
}

void testHArrayStr(StrKey* keys, uint32 countKeys)
{
	printf("HArrayVarRAM => ");

	HArrayVarRAM ha;
	ha.init(26);

	clock_t start, finish;

	//INSERT ===========================================

	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		ha.insert((uint32*)keys[i].Data, sizeof(StrKey), keys[i].Data[0]);
	}

	finish = msclock();

	printf("Insert: %d msec, ", (finish - start));

	totalHArrayTime += (finish - start);

	//SEARCH ===========================================
	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		if (ha.getValueByKey((uint32*)keys[i].Data, sizeof(StrKey)) != keys[i].Data[0])
		{
			printf("Error\n");
			break;
		}
	}

	finish = msclock();

	printf("Search: %d msec.\n", (finish - start));

	totalHArrayTime += (finish - start);

	#ifdef PRINT_MEM
	ha.printMemory();
	#endif

	#ifdef PRINT_STAT
	ha.printStat();
	#endif

	ha.destroy();
}

void testStdMapStr(StrKey* keys, uint32 countKeys)
{
	#ifdef STD_MAP_TESTS

	printf("std::map => ");

	std::map<StrKey, uint32> mymap;

	clock_t start, finish;

	//INSERT ===========================================

	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		mymap[keys[i]] = keys[i].Data[0];
	}

	finish = msclock();

	printf("Insert: %d msec, ", (finish - start));

	totalMapTime += (finish - start);

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

	totalMapTime += (finish - start);

	//ha.print();

	#endif
}

void testDenseHashMapStr(StrKey* keys, uint32 countKeys)
{
	#ifdef DENSE_HASH_MAP_TESTS

	printf("google::dense_hash_map => ");

	google::dense_hash_map<StrKey, uint32, StrKeyHasher> mymap;

	StrKey emptyKey;
	memset(&emptyKey, 0, sizeof(StrKey));

	mymap.set_empty_key(emptyKey);

	clock_t start, finish;

	//INSERT ===========================================

	start = msclock();

	for (uint32 i = 0; i < countKeys; i++)
	{
		mymap[keys[i]] = keys[i].Data[0];
	}

	finish = msclock();

	printf("Insert: %d msec, ", (finish - start));

	totalDenseTime += (finish - start);

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


void fillSeqStrs(StrKey* keys, uint32 countKeys)
{
	for (uint32 i = 0; i < countKeys; i++)
	{
		for (uint32 j = 0; j < STR_KEY_LEN - 8; j++)
		{
			keys[i].Data[j] = '0';
		}

		fillRand(keys[i].Data + (STR_KEY_LEN - 8), 8);
	}
}

void fillRandStrs(StrKey* keys, uint32 countKeys)
{
	for (uint32 i = 0; i < countKeys; i++)
	{
		fillRand(keys[i].Data, STR_KEY_LEN);
	}
}

void HArrayVarRAM_VS_StdMap_StrKey(uint32 startOnAmount, uint32 stepOfAmount, uint32 stopOnAmount)
{
	printf("=== HArrayVarRAM VS google::dense_hash_map<StrKey, int> VS std::map<StrKey,int> testing ===\n");

	StrKey* strKeys = new StrKey[stopOnAmount];

	#ifdef SEQUENCE_TESTS

	fillSeqStrs(strKeys, stopOnAmount);

	for (uint32 countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search %u SIMILAR keys (%u bytes each) ...\n", countKeys, sizeof(StrKey));
		testHArrayStr(strKeys, countKeys);
		testDenseHashMapStr(strKeys, countKeys);
		testStdMapStr(strKeys, countKeys);
		printf("\n");
	}

	#endif

	#ifdef RANDOM_TESTS

	fillRandStrs(strKeys, stopOnAmount);

	for (uint32 countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search %u RANDOM keys (%u bytes each) ...\n", countKeys, sizeof(StrKey));
		testHArrayStr(strKeys, countKeys);
		testDenseHashMapStr(strKeys, countKeys);
		testStdMapStr(strKeys, countKeys);
		printf("\n");
	}

	#endif

	delete[] strKeys;
}

/*
void dense_fake()
{
	google::dense_hash_map<const char*, uint32, hash<const char*>, eqstr> mymap;

	mymap.set_empty_key(NULL);

	char buff[10];
	strcpy(buff, "hello");

	mymap[buff] = 1;

	buff[1] = 'c';

	int val = mymap[buff];
}
*/

int main()
{
	//HArrayInt_VS_StdMap_IntKey(1000000,   //start
	//						   2000000,   //step
	//						   10000000); //stop


	HArrayVarRAM_VS_StdMap_BinKey(10000000,   //start
								  2000000,   //step
								  10000000,  //stop
								  false); 	 //shuffle
	
	//HArrayVarRAM_VS_StdMap_StrKey(1000000,   //start
	//						  	  1000000,   //step
	//						  	  1000000);  //stop

	printf("COEF Map VS HArray: %.2f\n", (double)totalMapTime / (double)totalHArrayTime);

	printf("COEF Dense VS HArray: %.2f\n", (double)totalDenseTime / (double)totalHArrayTime);

	return 0;
};