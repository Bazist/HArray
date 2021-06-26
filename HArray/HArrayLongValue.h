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
		uint32 KeyLen;
		uint32* Value;
		uint32 ValueLen;
		void* pData;
	};

	static bool scanValues(uint32* key, uint32 keyLen, uint32 value, uchar8 valueType, void* pData)
	{
		ScanLongValuesData* pScanData = (ScanLongValuesData*)pData;

		pScanData->ValueLen = value; //valueLen was saved in value

		if (pScanData->KeyLen + pScanData->ValueLen == keyLen) //our key is composite key: key + value
		{
			for (uint32 i = 0; i < pScanData->ValueLen; i++)
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

	bool insert(uint32* key,
				uint32 keyLen,
				uint32* value,
				uint32 valueLen)
	{
		if (HArray::hasPartKey(key, keyLen)) //it is update
		{
			HArray::delValueByKey(key, keyLen);
		}

		for (uint32 i = 0; i < valueLen; i++, keyLen++)
		{
			key[keyLen] = value[i];
		}

		return HArray::insert(key, keyLen, valueLen);
	}

	bool getValueByKey(uint32* key,
		uint32 keyLen,
		uint32* value,
		uint32& valueLen)
	{
		ScanLongValuesData scanData;
		scanData.KeyLen = keyLen;
		scanData.Value = value;
		scanData.ValueLen = 0;

		HArray::scanKeysAndValues(key, keyLen, scanValues, &scanData);

		valueLen = scanData.ValueLen;

		return (valueLen > 0);
	}

	bool delValueByKey(uint32* key, uint32 keyLen)
	{
		uint32 value[MAX_CHAR - ONLY_CONTENT_TYPE];
		uint32 valueLen = 0;

		if (getValueByKey(key, keyLen, value, valueLen))
		{
			for (uint32 i = 0; i < valueLen; i++, keyLen++)
			{
				key[keyLen] = value[i];
			}

			return HArray::delValueByKey(key, keyLen);
		}
		else
		{
			return false;
		}
	}
};