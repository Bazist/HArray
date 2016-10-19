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

const uint COUNT_KEYS = 1000000;
const uint KEY_LEN = 256;

class Str
{
public:
	char Val[KEY_LEN];
};

bool operator<(const Str& a, const Str& b)
{
	return strcmp(a.Val, b.Val) == 1;
}

Str keys[COUNT_KEYS];

HArrayVarRAM ha;

void gen_random(char *s, const int len) {
	static const char alphanum[] =
		"0123456789"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz";

	for (int i = 0; i < len; ++i) {
		s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
	}

	s[len] = 0;
}


void fill()
{
	for (uint i = 0; i<COUNT_KEYS; i++)
	{
		gen_random(keys[i].Val, KEY_LEN - 1);
	}
}

void testRandomMap()
{
	std::map<Str, uint> mymap;

clock_t start, finish;
	start = clock();

	for (uint i = 0; i<COUNT_KEYS; i++)
	{
		mymap[keys[i]] = i;
	}

	finish = clock();

	printf("Insert: %d msec, ", (finish - start));

	start = clock();

	for (uint i = 0; i<COUNT_KEYS; i++)
	{
		if (mymap[keys[i]] != i)
			printf("Error\n");
	}

	finish = clock();

	printf("Search: %d msec.", (finish - start));
}


void testRandom()
{
	clock_t start, finish;

	//INSERT ===========================================

	start = clock();

	ha.init(4, 24);

	for (uint i = 0; i<COUNT_KEYS; i++)
	{
		ha.insert((unsigned int*)&keys[i], KEY_LEN, i);
	}

	finish = clock();

	printf("Insert: %d msec, ", (finish - start));

	//SEARCH ===========================================
	start = clock();

	for (uint i = 0; i<COUNT_KEYS; i++)
	{
		if (ha.getValueByKey((unsigned int*)&keys[i], KEY_LEN) != i)
			printf("Error %d\n", i);
	}

	finish = clock();

	printf("Search: %d msec.", (finish - start));

	//ha.print();

	ha.destroy();
}


int _tmain(int argc, _TCHAR* argv[])
{
	fill();
	printf("=== HArrayVarRAM testing ===\n");
	printf("Start benchmarks ....\n");
	printf("Insert/Search %u random keys (%u bytes each) ...\n", COUNT_KEYS, KEY_LEN);

	printf("HArrayVarRAM => ");
	testRandom();
	printf("\n");

	printf("std::map => ");
	testRandomMap();
	printf("\n");
	
	system("pause");

	return 0;
};


