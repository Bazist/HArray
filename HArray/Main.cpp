/*
# Copyright(C) 2010-2016 Vyacheslav Makoveichuk
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

const uint COUNT_KEYS = 10000000;
HArrayVarRAM ha;

struct Uint4
{
   Uint4(){}
   
   uint data[4];
};

bool operator<(const Uint4& a, const Uint4& b) 
{ 
   for(uint i=0; i<4; i++)
   {
	   if(b.data[i]==a.data[i])
		   continue;
	   else
		   return b.data[i] < a.data[i];
   }

   return false;
}

Uint4 keys[COUNT_KEYS];

void fill()
{
	FILE* file = fopen("c:\\rand2.avi", "rb");

	fseek(file, 0, 0);

	for(uint i=0; i<COUNT_KEYS; i++)
	{
		fread(&keys[i].data, 4, 4, file);
	}
	
	fclose(file);
}

void testRandomMap()
{
	fill();

	std::map<Uint4, uint> mymap;

	clock_t start, finish;
	start = clock();

	for(uint i=0; i<COUNT_KEYS; i++)
	{
		mymap.insert ( std::make_pair(keys[i], keys[i].data[0]) );
	}

	finish = clock();

	printf( "Insert: %d msec, ", (finish - start) );

	uint control = 0;

	start = clock();

	for(uint i=0; i<COUNT_KEYS; i++)
	{
		if(mymap[keys[i]] != keys[i].data[0])
			printf("Error\n");	
	}

	finish = clock();

	printf( "Search: %d msec.", (finish - start) );
}

void testMapScan()
{
	fill();

	std::map<Uint4, Uint4> mymap;

	clock_t start, finish;
	start = clock();

	for(uint i=0; i<COUNT_KEYS; i++)
	{
		mymap.insert ( std::pair<Uint4,Uint4>(keys[i], keys[i]) );	
	}

	finish = clock();

	printf( "Insert: %d msec\n", (finish - start) );

	uint control = 0;

	start = clock();

	for (auto it = mymap.cbegin(); it != mymap.cend(); ++it)
		control += (*it).first.data[0];

	finish = clock();

	printf( "Scan: %d msec\n", (finish - start) );
	printf( "Control: %d msec\n", control );
}

void testSequence()
{
	clock_t start, finish;

	//ha.init("c:\\fts\\header.pg", "c:\\fts\\content.pg", "c:\\fts\\branch.pg", "c:\\fts\\block.pg", 16, 16, 24);
	ha.init(4, 24);

	uint key[4];
	
	start = clock();
	
	for(key[0] = 0; key[0] < 1; key[0]++)
		for(key[1] = 0; key[1] < 1; key[1]++)
			for(key[2] = 0; key[2] < 1; key[2]++)
				for(key[3] = 0; key[3] < COUNT_KEYS; key[3]++)
					ha.insert(key, 16, *key);
	
	finish = clock();

	printf( "Insert: %d msec\n", (finish - start) );

	uint count = 0;
	
	start = clock();

	for(key[0] = 0; key[0] < 1; key[0]++)
		for(key[1] = 0; key[1] < 1; key[1]++)
			for(key[2] = 0; key[2] < 1; key[2]++)
				for(key[3] = 0; key[3] < COUNT_KEYS; key[3]++)
					count += ha.getValueByKey(key, 16);

	finish = clock();

	printf( "Find: %d msec\n", (finish - start) );

	printf( "%d\n", count);
}

void testSequenceMap()
{
	clock_t start, finish;

	//ha.init("c:\\fts\\header.pg", "c:\\fts\\content.pg", "c:\\fts\\branch.pg", "c:\\fts\\block.pg", 16, 16, 24);
	ha.init(4, 24);

	Uint4 key;

	std::map<Uint4, uint> mymap;
	
	start = clock();

	for(key.data[0] = 0; key.data[0] < 1; key.data[0]++)
		for(key.data[1] = 0; key.data[1] < 1; key.data[1]++)
			for(key.data[2] = 0; key.data[2] < 1; key.data[2]++)
				for(key.data[3] = 0; key.data[3] < COUNT_KEYS; key.data[3]++)
					mymap.insert ( std::pair<Uint4,uint>(key, key.data[3]) );
	
	finish = clock();

	printf( "Insert: %d msec\n", (finish - start) );

	uint count = 0;
	
	start = clock();

	for(key.data[0] = 0; key.data[0] < 1; key.data[0]++)
		for(key.data[1] = 0; key.data[1] < 1; key.data[1]++)
			for(key.data[2] = 0; key.data[2] < 1; key.data[2]++)
				for(key.data[3] = 0; key.data[3] < COUNT_KEYS; key.data[3]++)
					count += mymap[key];

	finish = clock();

	printf( "Find: %d msec\n", (finish - start) );

	printf( "%d\n", count);
}

void testSequence2()
{
	fill();

	clock_t start, finish;

	//ha.init("c:\\fts\\header.pg", "c:\\fts\\content.pg", "c:\\fts\\branch.pg", "c:\\fts\\block.pg", 16, 16, 24);
	ha.init(16, 24);

	uint key[4];
	
	start = clock();

	for(key[0] = 0; key[0] < 1; key[0]++)
		for(key[1] = 0; key[1] < 1; key[1]++)
			for(key[2] = 0; key[2] < 1; key[2]++)
				for(key[3] = COUNT_KEYS; key[3] > 0; key[3]--)
					ha.insert(key, 16, *key);
	
	finish = clock();

	printf( "Insert: %d msec\n", (finish - start) );

	uint count = 0;
	
	start = clock();

	for(key[0] = 0; key[0] < 1; key[0]++)
		for(key[1] = 0; key[1] < 1; key[1]++)
			for(key[2] = 0; key[2] < 1; key[2]++)
				for(key[3] = COUNT_KEYS; key[3] > 0; key[3]--)
					count += ha.getValueByKey(key, 16);

	finish = clock();

	printf( "Find: %d msec\n", (finish - start) );

	printf( "%d\n", count);
}

void testSequence3()
{
	fill();

	clock_t start, finish;

	//ha.init("c:\\fts\\header.pg", "c:\\fts\\content.pg", "c:\\fts\\branch.pg", "c:\\fts\\block.pg", 16, 16, 24);
	ha.init(16, 24);

	uint key[4];
	
	start = clock();

	for(key[0] = 0; key[0] < 100; key[0]++)
		for(key[1] = 0; key[1] < 100; key[1]++)
			for(key[2] = 0; key[2] < 100; key[2]++)
				for(key[3] = 0; key[3] < 10; key[3]++)
					ha.insert(key, 16, *key);
	
	finish = clock();

	printf( "Insert: %d msec\n", (finish - start) );

	uint count = 0;
	
	start = clock();

	for(key[0] = 0; key[0] < 100; key[0]++)
		for(key[1] = 0; key[1] < 100; key[1]++)
			for(key[2] = 0; key[2] < 100; key[2]++)
				for(key[3] = 0; key[3] < 10; key[3]++)
					count += ha.getValueByKey(key, 16);

	finish = clock();

	printf( "Find: %d msec\n", (finish - start) );

	printf( "%d\n", count);
}

void testRandom()
{
	fill();
	
	clock_t start, finish;
		
	ha.init(4, 26);
	
	//INSERT ===========================================

	start = clock();

	for(uint i=0; i<COUNT_KEYS; i++)
	{
		ha.insert(keys[i].data, sizeof(Uint4), keys[i].data[0]);
	}

	finish = clock();

	printf( "Insert: %d msec, ", (finish - start) );

	//SEARCH ===========================================
	start = clock();

	for(uint i=0; i<COUNT_KEYS; i++)
	{
		if(ha.getValueByKey(keys[i].data, sizeof(Uint4)) != keys[i].data[0])
			printf("Error %d\n", i);
	}

	finish = clock();

	printf("Search: %d msec.", (finish - start));
}

void testRange()
{
	uint minKey[4];
	uint maxKey[4];

	for(uint i=0; i<4; i++)
	{
		minKey[i] = 0;
		maxKey[i] = 0;
	}

	//ha.insert(minKey, maxKey);

	minKey[0] = 0;
	maxKey[0] = 0xFFFFFFFF;
	
	/*
	minKey[3] = 0;
	maxKey[3] = 0xFFFFFFFF;
	*/

	uint* values[5000];

	uint count = 0;
	
	clock_t start, finish;

	start = clock();

	//for(uint i=0; i<10000; i++)
	//{
		//get next range
	count += ha.getValuesByRange(0, 0, minKey, maxKey);

	/*	minKey[0]+=20000000;
		maxKey[0]+=20000000;
	}*/

	finish = clock();

	printf( "Range: %d msec\n", (finish - start) );

	printf( "Range ======================================\n");
	for(uint i=0; i<count; i++)
	{
		//printf( "%u %u %u %u\n", values[i][0], values[i][1], values[i][2], values[i][3]);
	}

	printf("Count: %u\n", count);
}

int _tmain(int argc, _TCHAR* argv[])
{
	printf("=== HArrayVarRAM testing ===\n");
	printf("Start benchmarks ....\n");
	printf("Insert/Search %u random keys (%u bytes each) ...\n", COUNT_KEYS, sizeof(Uint4));

	printf("HArrayVarRAM => ");
	testRandom();
	printf("\n");

	printf("std::map => ");
	testRandomMap();
	printf("\n");

	system("pause");

	return 0;
};