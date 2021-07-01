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

typedef bool HARRAY_VALUE_VISIT_FUNC(int32_t value, void* pData);

class HArrayUniqueIntValueList : public HArray
{
private:

	struct ScanValuesData
	{
		int32_t KeyLenWithValueLen;
		HARRAY_VALUE_VISIT_FUNC* pVisitor;
		void* pData;
	};

	static bool scanValues(int32_t* key, int32_t keyLen, int32_t value, void* pData)
	{
		ScanValuesData* pScanData = (ScanValuesData*)pData;

		if (pScanData->KeyLenWithValueLen == keyLen && !value)
		{
			return pScanData->pVisitor(key[keyLen - 1], pScanData->pData);
		}
		else
		{
			return true;
		}
	}

public:

	bool insert(int32_t* key,
		int32_t keyLen,
		int32_t value)
	{
		int32_t existingValue;

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

	bool getIntValuesByKey(int32_t* key,
		int32_t keyLen,
		HARRAY_VALUE_VISIT_FUNC visitor,
		void* pData)
	{
		int32_t value;

		if (HArray::getValueByKey(key, keyLen, value)) //list with one value
		{
			return visitor(value, pData);
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

	bool delValueByKey(int32_t* key, int32_t keyLen, int32_t value)
	{
		int32_t existingValue;

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