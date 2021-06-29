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

template <class K, class V>
class HArrayGeneric : public HArray
{
private:
	uint32 keyLen;
	uint32 valueLen;

public:
	HArrayGeneric()
	{
		keyLen = sizeof(K);
		valueLen = sizeof(V);
	}

	bool insert(const K key,
				const V value)
	{

		return true;
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