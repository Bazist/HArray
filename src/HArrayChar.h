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

class HArrayChar : public HArray
{
private:

	struct ScanGetValueData
	{
		int32_t KeyLen;
		char* Value;
		int32_t ValueLen;
		void* pData;
	};

	static bool scanGetValue(int32_t* key, int32_t keyLen, int32_t value, void* pData)
	{
		ScanGetValueData* pScanData = (ScanGetValueData*)pData;

		int32_t valueLenInSegments;

		keyLen--;

		int32_t valueLen = key[keyLen];

		if (valueLen & 0x3)
		{
			valueLenInSegments = (valueLen >> 2) + 1;
		}
		else
		{
			valueLenInSegments = (valueLen >> 2);
		}

		if (pScanData->KeyLen + valueLenInSegments == keyLen) //our key is composite key: key + value
		{
			for (int32_t i = 0; i < valueLenInSegments; i++)
			{
				pScanData->Value[i] = key[pScanData->KeyLen++];
			}

			pScanData->ValueLen = valueLen; //value len in chars

			return false;
		}
		else
		{
			return true;
		}
	}

public:

	bool insert(const char* key,
		int32_t keyLen,
		const char* value,
		int32_t valueLen)
	{
		char newKey[MAX_KEY_SEGMENTS];
		int32_t j = 0;

		//add key
		for (int32_t i = 0; i < keyLen; i++, j++)
		{
			newKey[j] = key[i];
		}

		//align
		while (j & 0x3)
		{
			newKey[j++] = 0;
		}

		//add value
		for (int32_t i = 0; i < valueLen; i++, j++)
		{
			newKey[j] = value[i];
		}

		//align
		while (j & 0x3)
		{
			newKey[j++] = 0;
		}

		//add value len
		*(int32_t*)(newKey + j) = valueLen;

		j += 4;

		return HArray::insert((int32_t*)newKey, j >> 2, 0);
	}

	bool getValueByKey(const char* key,
		int32_t keyLen,
		char* value,
		int32_t& valueLen)
	{
		char newKey[MAX_KEY_SEGMENTS];
		int32_t j = 0;

		//add key
		for (int32_t i = 0; i < keyLen; i++, j++)
		{
			newKey[j] = key[i];
		}
		
		//align
		while (j & 0x3)
		{
			newKey[j++] = 0;
		}

		keyLen = j >> 2;

		ScanGetValueData scanData;
		scanData.KeyLen = keyLen;
		scanData.Value = value;
		scanData.ValueLen = 0;

		HArray::scanKeysAndValues((int32_t*)newKey, keyLen, scanGetValue, &scanData);

		valueLen = scanData.ValueLen;

		return (valueLen > 0);
	}

	bool hasPartKey(const char* key, int32_t keyLen)
	{
		char newKey[MAX_KEY_SEGMENTS];
		int32_t j = 0;

		//add key
		for (int32_t i = 0; i < keyLen; i++, j++)
		{
			newKey[j] = key[i];
		}

		//align
		while (j & 0x3)
		{
			newKey[j++] = 0;
		}

		return HArray::hasPartKey((int32_t*)newKey, j >> 2);
	}

	bool delValueByKey(const char* key, int32_t keyLen)
	{
		char value[MAX_KEY_SEGMENTS];
		int32_t valueLen;

		if (getValueByKey(key, keyLen, value, valueLen))
		{
			char newKey[MAX_KEY_SEGMENTS];
			int32_t j = 0;

			//add key
			for (int32_t i = 0; i < keyLen; i++, j++)
			{
				newKey[j] = key[i];
			}

			//align
			while (j & 0x3)
			{
				newKey[j++] = 0;
			}

			//add value
			for (int32_t i = 0; i < valueLen; i++, j++)
			{
				newKey[j] = value[i];
			}

			//align
			while (j & 0x3)
			{
				newKey[j++] = 0;
			}

			//add value len
			*(int32_t*)(newKey + j) = valueLen;

			j += 4;

			HArray::delValueByKey((int32_t*)newKey, j >> 2);

		}
		else
		{
			return false;
		}
	}
};