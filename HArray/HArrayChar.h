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

class HArrayChar : public HArrayLongValue
{
public:

	bool insert(const char* key,
		uint32 keyLen,
		uint32 value)
	{
		uint32 lastSegmentKeyLen = keyLen & 0x3;

		if (!lastSegmentKeyLen) //key is aligned by 4 bytes, just pass as is
		{
			return HArray::insert((uint32*)key, keyLen / 4, value);
		}
		else
		{
			uint32* keyInSegments = (uint32*)key;
			uint32 keyLenInSegments = keyLen >> 2;

			uint32 newKey[MAX_KEY_SEGMENTS];

			uint32 i = 0;

			for (; i < keyLenInSegments; i++)
			{
				newKey[i] = keyInSegments[i];
			}

			newKey[i] = 0;

			char* lastSegmentNewKey = (char*)&newKey[i];
			char* lastSegmentKey = (char*)&keyInSegments[i];

			for (uint32 j = 0; j < lastSegmentKeyLen; j++)
			{
				lastSegmentNewKey[j] = lastSegmentKey[j];
			}

			return HArray::insert((uint32*)newKey, keyLen / 4 + 1, value);
		}
	}

	bool getValueByKey(const char* key,
		uint32 keyLen,
		uint32& value)
	{
		uint32 lastSegmentKeyLen = keyLen & 0x3;

		if (!lastSegmentKeyLen) //key is aligned by 4 bytes, just pass as is
		{
			return HArray::getValueByKey((uint32*)key, keyLen / 4, value);
		}
		else
		{
			uint32* keyInSegments = (uint32*)key;
			uint32 keyLenInSegments = keyLen >> 2;

			uint32 newKey[MAX_KEY_SEGMENTS];

			uint32 i = 0;

			for (; i < keyLenInSegments; i++)
			{
				newKey[i] = keyInSegments[i];
			}

			newKey[i] = 0;

			char* lastSegmentNewKey = (char*)&newKey[i];
			char* lastSegmentKey = (char*)&keyInSegments[i];

			for (uint32 j = 0; j < lastSegmentKeyLen; j++)
			{
				lastSegmentNewKey[j] = lastSegmentKey[j];
			}

			return HArray::getValueByKey((uint32*)newKey, keyLen / 4 + 1, value);
		}
	}

	bool hasPartKey(const char* key, uint32 keyLen)
	{
		uint32 lastSegmentKeyLen = keyLen & 0x3;

		if (!lastSegmentKeyLen) //key is aligned by 4 bytes, just pass as is
		{
			return HArray::hasPartKey((uint32*)key, keyLen / 4);
		}
		else
		{
			uint32* keyInSegments = (uint32*)key;
			uint32 keyLenInSegments = keyLen >> 2;

			uint32 newKey[MAX_KEY_SEGMENTS];

			uint32 i = 0;

			for (; i < keyLenInSegments; i++)
			{
				newKey[i] = keyInSegments[i];
			}

			newKey[i] = 0;

			char* lastSegmentNewKey = (char*)&newKey[i];
			char* lastSegmentKey = (char*)&keyInSegments[i];

			for (uint32 j = 0; j < lastSegmentKeyLen; j++)
			{
				lastSegmentNewKey[j] = lastSegmentKey[j];
			}

			return HArray::hasPartKey((uint32*)newKey, keyLen / 4 + 1);
		}
	}

	bool delValueByKey(const char* key, uint32 keyLen)
	{
		uint32 lastSegmentKeyLen = keyLen & 0x3;

		if (!lastSegmentKeyLen) //key is aligned by 4 bytes, just pass as is
		{
			return HArray::delValueByKey((uint32*)key, keyLen / 4);
		}
		else
		{
			uint32* keyInSegments = (uint32*)key;
			uint32 keyLenInSegments = keyLen >> 2;

			uint32 newKey[MAX_KEY_SEGMENTS];

			uint32 i = 0;

			for (; i < keyLenInSegments; i++)
			{
				newKey[i] = keyInSegments[i];
			}

			newKey[i] = 0;

			char* lastSegmentNewKey = (char*)&newKey[i];
			char* lastSegmentKey = (char*)&keyInSegments[i];

			for (uint32 j = 0; j < lastSegmentKeyLen; j++)
			{
				lastSegmentNewKey[j] = lastSegmentKey[j];
			}

			return HArray::delValueByKey((uint32*)newKey, keyLen / 4 + 1);
		}
	}

};