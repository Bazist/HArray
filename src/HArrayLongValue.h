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

#include "HArray.h"

class HArrayLongValue : public HArray
{
private:

	struct ScanLongValuesData
	{
		int32_t KeyLen;
		int32_t* Value;
		int32_t ValueLen;
		void* pData;
	};

	static bool scanValues(int32_t* key, int32_t keyLen, int32_t value, void* pData)
	{
		ScanLongValuesData* pScanData = (ScanLongValuesData*)pData;

		keyLen--;

		pScanData->ValueLen = key[keyLen]; //valueLen was saved as last segment in key

		if (pScanData->KeyLen + pScanData->ValueLen == keyLen) //our key is composite key: key + value
		{
			for (int32_t i = 0; i < pScanData->ValueLen; i++)
			{
				pScanData->Value[i] = key[pScanData->KeyLen++];
			}

			return false;
		}
		else
		{
			return true;
		}
	}

public:

	bool insert(int32_t* key,
		int32_t keyLen,
		int32_t* value,
		int32_t valueLen)
	{
		if (HArray::hasPartKey(key, keyLen)) //it is update
		{
			delValueByKey(key, keyLen);
		}

		for (int32_t i = 0; i < valueLen; i++, keyLen++)
		{
			key[keyLen] = value[i];
		}

		key[keyLen++] = valueLen;

		return HArray::insert(key, keyLen, 0);
	}

	bool getValueByKey(int32_t* key,
		int32_t keyLen,
		int32_t* value,
		int32_t& valueLen)
	{
		ScanLongValuesData scanData;
		scanData.KeyLen = keyLen;
		scanData.Value = value;
		scanData.ValueLen = 0;

		HArray::scanKeysAndValues(key, keyLen, scanValues, &scanData);

		valueLen = scanData.ValueLen;

		return (valueLen > 0);
	}

	bool delValueByKey(int32_t* key, int32_t keyLen)
	{
		int32_t value[MAX_CHAR - ONLY_CONTENT_TYPE];
		int32_t valueLen = 0;

		if (getValueByKey(key, keyLen, value, valueLen))
		{
			for (int32_t i = 0; i < valueLen; i++, keyLen++)
			{
				key[keyLen] = value[i];
			}

			key[keyLen++] = valueLen;

			return HArray::delValueByKey(key, keyLen);
		}
		else
		{
			return false;
		}
	}
};