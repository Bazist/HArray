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

#include <vector>

#include "HArrayLongValue.h"

class ValuePool
{
public:
	ValuePool(uint32_t valueLen);

	void* insValue(uint32_t valueLen, uint32_t& position);
	void delValue(uint32_t position);
	void* getValue(uint32_t position);
	uint32_t getUsedMemory();
	uint32_t getTotalMemory();

private: // helpers
	void resizePages();

private: // data
	uint32_t ValueLen;
	uint32_t PositionOnPage;
	uint32_t CurrentPage;
	uint32_t SizePages;

	std::vector<std::vector<char>> Pages;
};

template <typename K, typename V>
class HArrayGeneric : public HArray
{
private:
	uint32_t valueLen;

	template<class T>
	inline uint32_t* getKeySegments(const T& obj, uint32_t* keyBuff, uint32_t& keyLen)
	{
		return obj.getKeySegments(keyBuff, keyLen);
	}

	template<class S>
	inline uint32_t* getKeySegments_STL(const S& obj, uint32_t sizeOfElement, uint32_t* keyBuff, uint32_t& keyLen)
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
	uint32_t* getKeySegments(const std::vector<C>& obj, uint32_t* keyBuff, uint32_t& keyLen)
	{
		return getKeySegments_STL(obj, keyBuff, keyLen);
	}

	uint32_t* getKeySegments(const int& obj, uint32_t* keyBuff, uint32_t& keyLen);
	uint32_t* getKeySegments(const unsigned int& obj, uint32_t* keyBuff, uint32_t& keyLen);
	uint32_t* getKeySegments(const std::string& obj, uint32_t* keyBuff, uint32_t& keyLen);
	uint32_t* getKeySegments(const char*& obj, uint32_t* keyBuff, uint32_t& keyLen);

public:
	std::unique_ptr<ValuePool> valuePool;

	HArrayGeneric()
	{
		valueLen = sizeof(V);

		valuePool = std::make_unique<ValuePool>(valueLen);
	}

	V& operator[](const K& key)
	{
		uint32_t keyBuff[MAX_KEY_SEGMENTS];
		uint32_t keyLen = 0;

		uint32_t* keySegments = getKeySegments(key, keyBuff, keyLen);

		uint32_t* pValue;

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

	bool erase(const K& key)
	{
		uint32_t keyBuff[MAX_KEY_SEGMENTS];
		uint32_t keyLen = 0;

		uint32_t* keySegments = getKeySegments(key, keyBuff, keyLen);

		uint32_t value;

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
		HArray::destroy();
	}
};
