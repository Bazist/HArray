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
public:

	bool insert(uint32* key,
				uint32 keyLen,
				uint32* value,
				uint32 valueLen)
	{
		//uint32 existingValue;

		//if (HArray::getValueByKey(key, keyLen, existingValue)) //list with one value
		//{
		//	if (existingValue == value) //it is update
		//	{
		//		HArray::insert(key, keyLen, value);
		//	}
		//	else
		//	{
		//		//delete existing value
		//		HArray::delValueByKey(key, keyLen);

		//		//insert existing value as part of key
		//		key[keyLen++] = existingValue;

		//		HArray::insert(key, keyLen, 0);

		//		//insert new value as part of key
		//		key[keyLen - 1] = value;

		//		HArray::insert(key, keyLen, 0);
		//	}
		//}
		//else //insert new value
		//{
		//	HArray::insert(key, keyLen, value);
		//}
	}

	bool getIntValuesByKey(uint32* key,
		uint32 keyLen,
		uint32* values,
		uint32& countValues)
	{
		//if (HArray::getValueByKey(key, keyLen, values[0])) //list with one value
		//{
		//	countValues = 1;

		//	return true;
		//}
	}

	bool delValueByKey(uint32* key, uint32 keyLen, uint32 value)
	{
		////uint32 existingValue;

		////if (HArray::getValueByKey(key, keyLen, existingValue)) //list with one value
		////{
		////	HArray::delValueByKey(key, keyLen);
		////}
		////else //list with many values
		////{
		////	key[keyLen++] = value;

		////	HArray::delValueByKey(key, keyLen);
		////}
	}
};