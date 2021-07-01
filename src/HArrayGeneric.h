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

#include "HArrayLongValue.h"

class ValuePool
{
public:

	const static uint32 MIN_COUNT_PAGES = 1;
	const static uint32 COUNT_VALUE_ON_ONE_PAGE = 1024;

	ValuePool(uint32 valueLen)
	{
		PositionOnPage = 0; //skip first byte
		CurrentPage = 0;
		SizePages = MIN_COUNT_PAGES;

		Pages = new char* [MIN_COUNT_PAGES];
		Pages[CurrentPage] = new char[valueLen * COUNT_VALUE_ON_ONE_PAGE];

		ValueLen = valueLen;
	}

	uint32 ValueLen;
	uint32 PositionOnPage;
	uint32 CurrentPage;
	uint32 SizePages;

	char** Pages;

	void resizePages()
	{
		char** pages = new char* [SizePages * 2];

		for (uint32 i = 0; i < SizePages; i++)
		{
			pages[i] = Pages[i];
		}

		delete[] Pages;

		Pages = pages;

		SizePages += 2;
	}

	void* insValue(uint32 valueLen, uint32& position)
	{
		//allocate position
		if (PositionOnPage + valueLen > MAX_SHORT)
		{
			CurrentPage++;

			if (CurrentPage == SizePages)
			{
				resizePages();
			}

			Pages[CurrentPage] = new char[MAX_SHORT];
		}

		position = PositionOnPage;

		PositionOnPage += ValueLen;

		return &Pages[position >> 16][position & 0xFFFF];
	}

	void delValue(uint32 position)
	{
		char* value = &Pages[position >> 16][position & 0xFFFF];

		for (uint32 i = 0; i < ValueLen; i++)
		{
			value[i] = 0;
		}
	}

	void* getValue(uint32 position)
	{
		return &Pages[position >> 16][position & 0xFFFF];
	}

	uint32 getUsedMemory()
	{
		uint32 usedMemory = sizeof(ValuePool) + ((CurrentPage + 1) * MAX_SHORT);

		return usedMemory;
	}

	uint32 getTotalMemory()
	{
		uint32 usedMemory = sizeof(ValuePool) + (SizePages * sizeof(char*)) + ((CurrentPage + 1) * MAX_SHORT);

		return usedMemory;
	}

	void destroy()
	{
		for (uint32 i = 0; i <= CurrentPage; i++)
		{
			delete[] Pages[i];
		}
	}
};

template <typename K, typename V>
class HArrayGeneric : public HArray
{
private:
	uint32 valueLen;

	template<class T>
	inline uint32* getKeySegments(const T& obj, uint32* keyBuff, uint32& keyLen)
	{
		return obj.getKeySegments(keyBuff, keyLen);
	}

	template<class S>
	inline uint32* getKeySegments_STL(const S& obj, uint32 sizeOfElement, uint32* keyBuff, uint32& keyLen)
	{
		keyLen = sizeOfElement * sizeof(S);

		char* keyBuffBytes = (char*)keyBuff;

		std::copy(obj.begin(), obj.end(), keyBuffBytes);

		while (keyLen & 0x3)
		{
			keyBuffBytes[keyLen++] = 0;
		}

		keyLen >>= 2;

		return keyBuff;
	}

	template<typename C>
	inline uint32* getKeySegments(const std::vector<C>& obj, uint32* keyBuff, uint32& keyLen)
	{
		return getKeySegments_STL(obj, keyBuff, keyLen);
	}

	inline uint32* getKeySegments(const int& obj, uint32* keyBuff, uint32& keyLen)
	{
		keyLen = 1;

		return (uint32*)&obj;
	}

	inline uint32* getKeySegments(const unsigned int& obj, uint32* keyBuff, uint32& keyLen)
	{
		keyLen = 1;

		return (uint32*)&obj;
	}

	inline uint32* getKeySegments(const std::string& obj, uint32* keyBuff, uint32& keyLen)
	{
		keyLen = obj.length();
		char* keyBuffBytes = (char*)keyBuff;

		memcpy(keyBuff, obj.c_str(), keyLen);

		while (keyLen & 0x3)
		{
			keyBuffBytes[keyLen++] = 0;
		}

		keyLen >>= 2;

		return keyBuff;
	}

	inline uint32* getKeySegments(const char*& obj, uint32* keyBuff, uint32& keyLen)
	{
		uint32 keyLen = strlen(obj);
		char* keyBuffBytes = (char*)keyBuff;

		for (uint32 i = 0; i < keyLen; i++)
		{
			keyBuffBytes[i] = obj[i];
		}

		while (keyLen & 0x3)
		{
			keyBuffBytes[keyLen++] = 0;
		}

		keyLen >>= 2;

		return keyBuff;
	}

public:

	ValuePool* pValuePool;

	HArrayGeneric()
	{
		valueLen = sizeof(V);

		pValuePool = new ValuePool(valueLen);
	}

	V& operator[](const K& key)
	{
		uint32 keyBuff[MAX_KEY_SEGMENTS];
		uint32 keyLen = 0;

		uint32* keySegments = getKeySegments(key, keyBuff, keyLen);

		uint32* pValue;

		void* pValueData;

		if (HArray::insertOrGet(keySegments, keyLen, &pValue))
		{
			pValueData = pValuePool->insValue(valueLen, *pValue);

			//copy empty value here
			V v;
			memcpy(pValueData, &v, valueLen);
		}
		else
		{
			pValueData = pValuePool->getValue(*pValue);
		}

		return *(V*)pValueData;
	}

	bool erase(const K key)
	{
		uint32 keyBuff[MAX_KEY_SEGMENTS];
		uint32 keyLen = 0;

		uint32* keySegments = getKeySegments(key, keyBuff, keyLen);

		uint32 value;

		if (HArray::getValueByKey(keySegments, key, value))
		{
			pValuePool->delValue(value);

			HArray::delValueByKey(keySegments, keyLen);

			return true;
		}
		else
		{
			return false;
		}
	}

	~HArrayGeneric()
	{
		pValuePool->destroy();
		delete pValuePool;

		HArray::destroy();
	}
};