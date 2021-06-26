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

typedef bool HARRAY_VALUE_VISIT_FUNC(uint32 value, uchar8 valueType, void* pData);

class HArrayUniqueIntValueList : public HArray
{
private:

	struct ScanValuesData
	{
		uint32 KeyLenWithValueLen;
		HARRAY_VALUE_VISIT_FUNC* pVisitor;
		void* pData;
	};

	static bool scanValues(uint32* key, uint32 keyLen, uint32 value, uchar8 valueType, void* pData)
	{
		ScanValuesData* pScanData = (ScanValuesData*)pData;

		if (pScanData->KeyLenWithValueLen == keyLen && !value)
		{
			return pScanData->pVisitor(key[keyLen - 1], valueType, pScanData->pData);
		}
		else
		{
			return true;
		}
	}

public:

	bool insert(uint32* key,
				uint32 keyLen,
				uint32 value)
	{
		uint32 existingValue;

		if (HArray::getValueByKey(key, keyLen, existingValue)) //list with one value
		{
			if (existingValue != value) //value already exists
			{
				//delete existing value
				HArray::delValueByKey(key, keyLen);

				//insert existing value as part of key
				key[keyLen++] = existingValue;

				HArray::insert(key, keyLen, 0);

				//insert new value as part of key
				key[keyLen - 1] = value;

				HArray::insert(key, keyLen, 0);

				return true;
			}
			else
			{
				return false;
			}
		}
		else if (HArray::hasPartKey(key, keyLen)) //insert new value in list
		{
			key[keyLen++] = value;

			return HArray::insert(key, keyLen, 0);
		}
		else //insert new value as one value
		{
			return HArray::insert(key, keyLen, value);
		}
	}

	bool getIntValuesByKey(uint32* key,
						   uint32 keyLen,
						   HARRAY_VALUE_VISIT_FUNC visitor,
						   void* pData)
	{
		uint32 value;
		uchar8 valueType;

		if (HArray::getValueByKey(key, keyLen, value, valueType)) //list with one value
		{
			return visitor(value, valueType, pData);
		}
		else
		{
			ScanValuesData scanData;
			scanData.KeyLenWithValueLen = keyLen + 1;
			scanData.pVisitor = visitor;
			scanData.pData = pData;

			HArray::scanKeysAndValues(key, keyLen, scanValues, &scanData);

			return true;
		}
	}

	bool delValueByKey(uint32* key, uint32 keyLen, uint32 value)
	{
		uint32 existingValue;

		if (HArray::getValueByKey(key, keyLen, existingValue)) //list with one value
		{
			return HArray::delValueByKey(key, keyLen);
		}
		else //list with many values
		{
			key[keyLen++] = value;

			return HArray::delValueByKey(key, keyLen);
		}
	}
};