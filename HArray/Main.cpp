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

using namespace std;

uint32 totalHArrayTime = 0;
uint32 totalMapTime = 0;

clock_t msclock()
{
	return (ulong64)clock() * 1000 / CLOCKS_PER_SEC; //in ms
}

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
	printf("=== HArrayInt VS std::map<int,int> testing ===\n");

	uint32* intKeys = new uint32[stopOnAmount];

	fillSeqInts(intKeys, stopOnAmount);

	for (uint32 countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search %u SEQUENCE keys (%u bytes each) ...\n", countKeys, sizeof(uint32));
		testHArrayInt(intKeys, countKeys);
		testStdMapInt(intKeys, countKeys);
		printf("\n");
	}

	fillRandInts(intKeys, stopOnAmount);

	for (uint32 countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search %u RANDOM keys (%u bytes each) ...\n", countKeys, sizeof(uint32));
		testHArrayInt(intKeys, countKeys);
		testStdMapInt(intKeys, countKeys);
		printf("\n");
	}

	fillPeriodInts(intKeys, stopOnAmount);

	for (uint32 countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search %u PERIOD keys (%u bytes each) ...\n", countKeys, sizeof(uint32));
		testHArrayInt(intKeys, countKeys);
		testStdMapInt(intKeys, countKeys);
		printf("\n");
	}

	delete[] intKeys;
}

//==== HArrayVarRAM - Binary Keys ===========================================================================================

const uint32 BIN_KEY_LEN = 4;

struct BinKey
{
public:
	uint32 Data[BIN_KEY_LEN]; //16 bytes max
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

	/*
	if(countKeys == 7000000)
	{
	//6750000 - 6800000
	countKeys = 6778843;
	}
	*/

	for (uint32 i = 0; i < countKeys; i++)
	{
		/*
		if(i==6778842)
		printf("%u\n", i);

		if(i==6778843)
		printf("%u\n", i);
		*/

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

	//ha.print();

	//ha.printMemory();

	ha.destroy();
}

void testStdMapBin(BinKey* keys, uint32 countKeys, bool shuffle)
{
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
	printf("=== HArrayVarRAM VS std::map<BinKey,int> testing ===\n");

	BinKey* binKeys = new BinKey[stopOnAmount];

	fillSeqBins(binKeys, stopOnAmount);

	for (uint32 countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search %u SEQUENCE keys (%u bytes each) ...\n", countKeys, sizeof(BinKey));
		testHArrayBin(binKeys, countKeys, shuffle);
		testStdMapBin(binKeys, countKeys, shuffle);
		printf("\n");
	}

	fillRandBins(binKeys, stopOnAmount);

	for (uint32 countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search %u RANDOM keys (%u bytes each) ...\n", countKeys, sizeof(BinKey));
		testHArrayBin(binKeys, countKeys, shuffle);
		testStdMapBin(binKeys, countKeys, shuffle);
		printf("\n");
	}

	fillPeriodBins(binKeys, stopOnAmount);

	for (uint32 countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search %u PERIOD keys (%u bytes each) ...\n", countKeys, sizeof(BinKey));
		testHArrayBin(binKeys, countKeys, shuffle);
		testStdMapBin(binKeys, countKeys, shuffle);
		printf("\n");
	}

	delete[] binKeys;
}

//==== HArrayVarRAM - String Keys ===========================================================================================

const char STR_KEY_LEN = 64;

struct StrKey
{
public:
	char Data[STR_KEY_LEN]; //64 bytes max
};

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

	//ha.print();

	ha.destroy();
}

void testStdMapStr(StrKey* keys, uint32 countKeys)
{
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
	printf("=== HArrayVarRAM VS std::map<StrKey,int> testing ===\n");

	StrKey* strKeys = new StrKey[stopOnAmount];

	fillSeqStrs(strKeys, stopOnAmount);

	for (uint32 countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search %u SIMILAR keys (%u bytes each) ...\n", countKeys, sizeof(StrKey));
		testHArrayStr(strKeys, countKeys);
		testStdMapStr(strKeys, countKeys);
		printf("\n");
	}

	fillRandStrs(strKeys, stopOnAmount);

	for (uint32 countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search %u RANDOM keys (%u bytes each) ...\n", countKeys, sizeof(StrKey));
		testHArrayStr(strKeys, countKeys);
		testStdMapStr(strKeys, countKeys);
		printf("\n");
	}

	delete[] strKeys;
}

int main()
{
	HArrayInt_VS_StdMap_IntKey(1000000,   //start
							   2000000,   //step
							   10000000); //stop

	HArrayVarRAM_VS_StdMap_BinKey(1000000,   //start
								  2000000,   //step
								  10000000,  //stop
								  true); 	 //shuffle

	HArrayVarRAM_VS_StdMap_StrKey(1000000,   //start
							  	  1000000,   //step
							  	  3000000);  //stop

	printf("COEF: %.2f\n", (double)totalMapTime / (double)totalHArrayTime);

	return 0;
};


