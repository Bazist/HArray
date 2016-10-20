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
#include <iostream>
#include <map>
#include <unordered_map>
#include <windows.h>
#include "HArrayInt.h"
#include "HArrayVarRAM.h"

using namespace std;

//==== HArrayInt - Int Keys ===========================================================================================

void testHArrayInt(uint* keys, uint countKeys)
{
	printf("HArrayInt => ");

	HArrayInt ha;
	ha.init(26);

	clock_t start, finish;

	//INSERT ===========================================

	start = clock();

	for (int i = 0; i < countKeys; i++)
	{
		ha.insert(keys[i], keys[i]);
	}

	finish = clock();

	printf("Insert: %d msec, ", (finish - start));

	//SEARCH ===========================================
	start = clock();

	for (uint i = 0; i < countKeys; i++)
	{
		if (ha.getValueByKey(keys[i]) != keys[i])
		{
			printf("Error\n");
			break;
		}
	}

	finish = clock();

	printf("Search: %d msec.\n", (finish - start));

	//ha.print();

	ha.destroy();
}

void testStdMapInt(uint* keys, uint countKeys)
{
	printf("std::map => ");

	std::map<uint, uint> mymap;

	clock_t start, finish;

	//INSERT ===========================================

	start = clock();

	for (int i = 0; i < countKeys; i++)
	{
		mymap[keys[i]] = keys[i];
	}

	finish = clock();

	printf("Insert: %d msec, ", (finish - start));

	//SEARCH ===========================================
	start = clock();

	for (uint i = 0; i < countKeys; i++)
	{
		if (mymap[keys[i]] != keys[i])
		{
			printf("Error\n");
			break;
		}
	}

	finish = clock();

	printf("Search: %d msec.\n", (finish - start));

	//ha.print();
}

void fillSeqInts(uint* keys, uint countKeys)
{
	for (uint i = 0; i < countKeys; i++)
	{
		keys[i] = i;
	}
}

void fillRandInts(uint* keys, uint countKeys)
{
	for (uint i = 0; i < countKeys; i++)
	{
		keys[i] = rand();
	}
}

void fillPeriodInts(uint* keys, uint countKeys)
{
	for (uint i = 0; i < countKeys; i++)
	{
		keys[i] = i * 17;
	}
}

void HArrayInt_VS_StdMap_IntKey(uint startOnAmount, uint stepOfAmount, uint stopOnAmount)
{
	printf("=== HArrayInt VS std::map<int,int> testing ===\n");

	uint* intKeys = new uint[stopOnAmount];

	fillSeqInts(intKeys, stopOnAmount);

	for (uint countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search %u SEQUENCE keys (%u bytes each) ...\n", countKeys, sizeof(uint));
		testHArrayInt(intKeys, countKeys);
		testStdMapInt(intKeys, countKeys);
		printf("\n");
	}

	fillRandInts(intKeys, stopOnAmount);

	for (uint countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search %u RANDOM keys (%u bytes each) ...\n", countKeys, sizeof(uint));
		testHArrayInt(intKeys, countKeys);
		testStdMapInt(intKeys, countKeys);
		printf("\n");
	}

	fillPeriodInts(intKeys, stopOnAmount);

	for (uint countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search %u PERIOD keys (%u bytes each) ...\n", countKeys, sizeof(uint));
		testHArrayInt(intKeys, countKeys);
		testStdMapInt(intKeys, countKeys);
		printf("\n");
	}

	delete[] intKeys;
}

//==== HArrayVarRAM - Binary Keys ===========================================================================================

const uint BIN_KEY_LEN = 4;

struct BinKey
{
public:
	uint Data[BIN_KEY_LEN]; //16 bytes max
};

bool operator<(const BinKey& a, const BinKey& b)
{
	for (uint i = 0; i < BIN_KEY_LEN; i++)
	{
		if (a.Data[i] == b.Data[i])
			continue;
		else
			return a.Data[i] < b.Data[i];
	}

	return false;
}

void testHArrayBin(BinKey* keys, uint countKeys)
{
	printf("HArrayVarRAM => ");

	HArrayVarRAM ha;
	ha.init(26);

	clock_t start, finish;

	//INSERT ===========================================

	start = clock();

	for (uint i = 0; i < countKeys; i++)
	{
		ha.insert(keys[i].Data, sizeof(BinKey), keys[i].Data[0]);
	}

	finish = clock();

	printf("Insert: %d msec, ", (finish - start));

	//SEARCH ===========================================
	start = clock();

	for (uint i = 0; i < countKeys; i++)
	{
		if (ha.getValueByKey(keys[i].Data, sizeof(BinKey)) != keys[i].Data[0])
		{
			printf("Error\n");
			break;
		}
	}

	finish = clock();

	printf("Search: %d msec.\n", (finish - start));

	//ha.print();

	ha.destroy();
}

void testStdMapBin(BinKey* keys, uint countKeys)
{
	printf("std::map => ");

	std::map<BinKey, uint> mymap;

	clock_t start, finish;

	//INSERT ===========================================

	start = clock();

	for (uint i = 0; i < countKeys; i++)
	{
		mymap[keys[i]] = keys[i].Data[0];
	}

	finish = clock();

	printf("Insert: %d msec, ", (finish - start));

	//SEARCH ===========================================
	start = clock();

	for (uint i = 0; i < countKeys; i++)
	{
		if (mymap[keys[i]] != keys[i].Data[0])
		{
			printf("Error\n");
			break;
		}
	}

	finish = clock();

	printf("Search: %d msec.\n", (finish - start));

	//ha.print();
}

void fillSeqBins(BinKey* keys, uint countKeys)
{
	for (uint i = 0; i < countKeys; i++)
	{
		for (uint j = 0; j < BIN_KEY_LEN - 1; j++)
		{
			keys[i].Data[j] = 0;
		}

		keys[i].Data[3] = i;
	}
}

void fillRandBins(BinKey* keys, uint countKeys)
{
	for (uint i = 0; i < countKeys; i++)
	{
		for (uint j = 0; j < BIN_KEY_LEN; j++)
		{
			keys[i].Data[j] = (ulong)rand() * (ulong)rand();
		}
	}
}

void fillPeriodBins(BinKey* keys, uint countKeys)
{
	for (uint i = 0; i < countKeys; i++)
	{
		for (uint j = 0; j < BIN_KEY_LEN; j++)
		{
			keys[i].Data[j] = i * 17;
		}
	}
}

void HArrayVarRAM_VS_StdMap_BinKey(uint startOnAmount, uint stepOfAmount, uint stopOnAmount)
{
	printf("=== HArrayVarRAM VS std::map<BinKey,int> testing ===\n");

	BinKey* binKeys = new BinKey[stopOnAmount];

	fillSeqBins(binKeys, stopOnAmount);

	for (uint countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search %u SEQUENCE keys (%u bytes each) ...\n", countKeys, sizeof(BinKey));
		testHArrayBin(binKeys, countKeys);
		testStdMapBin(binKeys, countKeys);
		printf("\n");
	}

	fillRandBins(binKeys, stopOnAmount);

	for (uint countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search %u RANDOM keys (%u bytes each) ...\n", countKeys, sizeof(BinKey));
		testHArrayBin(binKeys, countKeys);
		testStdMapBin(binKeys, countKeys);
		printf("\n");
	}

	fillPeriodBins(binKeys, stopOnAmount);

	for (uint countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search %u PERIOD keys (%u bytes each) ...\n", countKeys, sizeof(BinKey));
		testHArrayBin(binKeys, countKeys);
		testStdMapBin(binKeys, countKeys);
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
	uint i = 0;

	for (; i < len - 1; i++)
	{
		str[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
	}

	str[i] = 0;
}

void testHArrayStr(StrKey* keys, uint countKeys)
{
	printf("HArrayVarRAM => ");

	HArrayVarRAM ha;
	ha.init(26);

	clock_t start, finish;

	//INSERT ===========================================

	start = clock();

	for (uint i = 0; i < countKeys; i++)
	{
		ha.insert((uint*)keys[i].Data, sizeof(StrKey), keys[i].Data[0]);
	}

	finish = clock();

	printf("Insert: %d msec, ", (finish - start));

	//SEARCH ===========================================
	start = clock();

	for (uint i = 0; i < countKeys; i++)
	{
		if (ha.getValueByKey((uint*)keys[i].Data, sizeof(StrKey)) != keys[i].Data[0])
		{
			printf("Error\n");
			break;
		}
	}

	finish = clock();

	printf("Search: %d msec.\n", (finish - start));

	//ha.print();

	ha.destroy();
}

void testStdMapStr(StrKey* keys, uint countKeys)
{
	printf("std::map => ");

	std::map<StrKey, uint> mymap;

	clock_t start, finish;

	//INSERT ===========================================

	start = clock();

	for (uint i = 0; i < countKeys; i++)
	{
		mymap[keys[i]] = keys[i].Data[0];
	}

	finish = clock();

	printf("Insert: %d msec, ", (finish - start));

	//SEARCH ===========================================
	start = clock();

	for (uint i = 0; i < countKeys; i++)
	{
		if (mymap[keys[i]] != keys[i].Data[0])
		{
			printf("Error\n");
			break;
		}
	}

	finish = clock();

	printf("Search: %d msec.\n", (finish - start));

	//ha.print();
}

void fillSeqStrs(StrKey* keys, uint countKeys)
{
	for (uint i = 0; i < countKeys; i++)
	{
		for (uint j = 0; j < STR_KEY_LEN - 8; j++)
		{
			keys[i].Data[j] = '0';
		}

		fillRand(keys[i].Data + (STR_KEY_LEN - 8), 8);
	}
}

void fillRandStrs(StrKey* keys, uint countKeys)
{
	for (uint i = 0; i < countKeys; i++)
	{
		fillRand(keys[i].Data, STR_KEY_LEN);
	}
}

void HArrayVarRAM_VS_StdMap_StrKey(uint startOnAmount, uint stepOfAmount, uint stopOnAmount)
{
	printf("=== HArrayVarRAM VS std::map<StrKey,int> testing ===\n");

	StrKey* strKeys = new StrKey[stopOnAmount];

	fillSeqStrs(strKeys, stopOnAmount);

	for (uint countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search %u SIMILAR keys (%u bytes each) ...\n", countKeys, sizeof(StrKey));
		testHArrayStr(strKeys, countKeys);
		testStdMapStr(strKeys, countKeys);
		printf("\n");
	}

	fillRandStrs(strKeys, stopOnAmount);

	for (uint countKeys = startOnAmount; countKeys <= stopOnAmount; countKeys += stepOfAmount)
	{
		printf("Insert/Search %u RANDOM keys (%u bytes each) ...\n", countKeys, sizeof(StrKey));
		testHArrayStr(strKeys, countKeys);
		testStdMapStr(strKeys, countKeys);
		printf("\n");
	}

	delete[] strKeys;
}

int _tmain(int argc, _TCHAR* argv[])
{
	HArrayInt_VS_StdMap_IntKey(1000000,   //start
							   2000000,   //step
							   10000000); //stop

	HArrayVarRAM_VS_StdMap_BinKey(1000000,   //start
								  2000000,   //step
								  10000000); //stop

	HArrayVarRAM_VS_StdMap_StrKey(1000000,   //start
									1000000,   //step
									3000000); //stop

	system("pause");

	return 0;
};


