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

typedef bool HARRAY_VALUE_VISIT_FUNC(uint32* key, uint32 keyLen, uint32 value, void* pData);

class HArrayUniqueIntValueList : public HArray
{
private:

	struct ScanValuesData
	{
		HARRAY_VALUE_VISIT_FUNC* pVisitor;
		void* pData;
	};

	static bool scanAllValues(uint32* key, uint32 keyLen, uint32 value, std::atomic<uchar8>& blockedByTranID, void* pData)
	{
		ScanValuesData* pScanData = (ScanValuesData*)pData;

		keyLen--;

		return pScanData->pVisitor(key, keyLen, key[keyLen], pScanData->pData);
	}

public:

	bool insert(uint32* key,
		uint32 keyLen,
		uint32 value)
	{
		key[keyLen++] = value;

		return HArray::insert(key, keyLen, 0);
	}

	bool getIntValuesByKey(uint32* key,
		uint32 keyLen,
		HARRAY_VALUE_VISIT_FUNC visitor,
		void* pData)
	{
		ScanValuesData scanData;
		scanData.pVisitor = visitor;
		scanData.pData = pData;

		HArray::scanKeysAndValues(key, keyLen, scanAllValues, &scanData);

		return true;
	}

	bool delValueByKey(uint32* key, uint32 keyLen, uint32 value)
	{
		key[keyLen++] = value;

		return HArray::delValueByKey(key, keyLen);
	}

	bool scanKeysAndValues(uint32* key,
		uint32 keyLen,
		HARRAY_VALUE_VISIT_FUNC visitor,
		void* pData)
	{
		ScanValuesData scanData;
		scanData.pVisitor = visitor;
		scanData.pData = pData;

		HArray::scanKeysAndValues(key, keyLen, scanAllValues, &scanData);

		return true;
	}
};