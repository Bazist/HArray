
/*
# Copyright(C) 2010-2017 Vyacheslav Makoveychuk (email: slv709@gmail.com, skype: vyacheslavm81)
# This file is part of VyMa\Trie.
#
# VyMa\Trie is free software : you can redistribute it and / or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Vyma\Trie is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public LicenseЕ
# along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "HArray.h"

void HArray::resizeHeader()
{
	uint32 newHeaderBase = HeaderBase + 2; //in four times bigger
    uint32 newHeaderBits = 32-HeaderBase;
    uint32 newHeaderSize = (0xFFFFFFFF>>HeaderBits) + 1;

	HeaderCell* pNewHeader = new HeaderCell[newHeaderSize];

	//zoom
	for(uint32 i=0, j=0; i<HeaderSize; i++)
	{
		HeaderCell& headerCell = pHeader[i];

		pNewHeader[j++] = headerCell;
		pNewHeader[j++] = headerCell;
		pNewHeader[j++] = headerCell;
		pNewHeader[j++] = headerCell;
	}

	//copy members
	delete[] pHeader;
	pHeader = pNewHeader;

	HeaderBase = newHeaderBase;
	HeaderBits = newHeaderBits;
	HeaderSize = newHeaderSize;
}