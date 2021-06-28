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
		uint32 KeyLen;
		char* Value;
		uint32 ValueLen;
		void* pData;
	};

	static bool scanGetValue(uint32* key, uint32 keyLen, uint32 value, void* pData)
	{
		ScanGetValueData* pScanData = (ScanGetValueData*)pData;

		uint32 valueLenInSegments;

		keyLen--;

		uint32 valueLen = key[keyLen];

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
			for (uint32 i = 0; i < valueLenInSegments; i++)
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
		uint32 keyLen,
		const char* value,
		uint32 valueLen)
	{
		char newKey[MAX_KEY_SEGMENTS];
		uint32 j = 0;

		//add key
		for (uint32 i = 0; i < keyLen; i++, j++)
		{
			newKey[j] = key[i];
		}

		//align
		while (j & 0x3)
		{
			newKey[j++] = 0;
		}

		//add value
		for (uint32 i = 0; i < valueLen; i++, j++)
		{
			newKey[j] = value[i];
		}

		//align
		while (j & 0x3)
		{
			newKey[j++] = 0;
		}

		//add value len
		*(uint32*)(newKey + j) = valueLen;

		j += 4;

		return HArray::insert((uint32*)newKey, j >> 2, 0);
	}

	bool getValueByKey(const char* key,
		uint32 keyLen,
		char* value,
		uint32& valueLen)
	{
		char newKey[MAX_KEY_SEGMENTS];
		uint32 j = 0;

		//add key
		for (uint32 i = 0; i < keyLen; i++, j++)
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

		HArray::scanKeysAndValues((uint32*)newKey, keyLen, scanGetValue, &scanData);

		valueLen = scanData.ValueLen;

		return (valueLen > 0);
	}

	bool hasPartKey(const char* key, uint32 keyLen)
	{
		char newKey[MAX_KEY_SEGMENTS];
		uint32 j = 0;

		//add key
		for (uint32 i = 0; i < keyLen; i++, j++)
		{
			newKey[j] = key[i];
		}

		//align
		while (j & 0x3)
		{
			newKey[j++] = 0;
		}

		return HArray::hasPartKey((uint32*)newKey, j >> 2);
	}

	bool delValueByKey(const char* key, uint32 keyLen)
	{
		
	}
};