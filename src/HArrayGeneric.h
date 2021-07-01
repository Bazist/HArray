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
	inline uint32* getKeySegments_STL(const S& obj, uint32* keyBuff, uint32& keyLen)
	{
		keyLen = obj.size() * sizeof(S);

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
		uint32 keyLen = obj.length();
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
	HArrayGeneric()
	{
		valueLen = sizeof(V);
	}

	bool insert(const K& key,
				const V& value)
	{
		uint32 keyBuff[MAX_KEY_SEGMENTS];
		uint32 keyLen = 0;

		uint32* keySegments = getKeySegments(key, keyBuff, keyLen);

		return HArray::insert(keySegments, keyLen, value);
	}

	int& operator[](const K& key)
	{

	}

	bool getValueByKey(const K key,
					   V& outValue)
	{
		return true;
	}

	bool delValueByKey(const K key)
	{
		return true;
	}
};