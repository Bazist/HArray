
/*
# Copyright(C) 2010-2017 Viacheslav Makoveichuk (email: slv709@gmail.com, skype: vyacheslavm81)
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
	/*
	uint32 newHeaderBase = HeaderBase + 2; //in four times bigger
    uint32 newHeaderBits = 32-newHeaderBase;
    uint32 newHeaderSize = (0xFFFFFFFF>>newHeaderBits) + 1;
	uint32 newAmountFreeSlotsBeforeHeaderResize = newHeaderSize >> MAX_HEADER_FILL_FACTOR_BITS;

	HeaderCell* pNewHeader = new HeaderCell[newHeaderSize];

	//zoom
	for(uint32 i=0, j=0; i<HeaderSize; i++)
	{
		HeaderCell& headerCell = pHeader[i];

		if (headerCell.Type)
		{
			ContentCell& contentCell = pContentPages[headerCell.Offset >> 16]->pContent[headerCell.Offset & 0xFFFF];

			if(contentCell.Type <= MAX_BRANCH_TYPE1)
			{
				BranchCell& branchCell = pBranchPages[contentCell.Value >> 16]->pBranch[contentCell.Value & 0xFFFF];

				for (uint32 k = 0; k < contentCell.Type; k++)
				{
					uint32 headerOffset;

					if (!normalizeFunc)
					{
						headerOffset = contentCell.Value >> newHeaderBits;
					}
					else
					{
						headerOffset = (*normalizeFunc)(&contentCell.Value) >> newHeaderBits;
					}

					pNewHeader[headerOffset] = headerCell;
				}

				pNewHeader[j++] = headerCell;
				pNewHeader[j++] = headerCell;
				pNewHeader[j++] = headerCell;
				pNewHeader[j++] = headerCell;
			}
			else if (contentCell.Type <= MAX_BLOCK_TYPE)
			{
				pNewHeader[j++] = headerCell;
				pNewHeader[j++] = headerCell;
				pNewHeader[j++] = headerCell;
				pNewHeader[j++] = headerCell;
			}
			else //only content
			{
				pNewHeader[j++].Type = 0;
				pNewHeader[j++].Type = 0;
				pNewHeader[j++].Type = 0;
				pNewHeader[j++].Type = 0;

				uint32 headerOffset;

				if (!normalizeFunc)
				{
					headerOffset = contentCell.Value >> newHeaderBits;
				}
				else
				{
					headerOffset = (*normalizeFunc)(&contentCell.Value) >> newHeaderBits;
				}

				pNewHeader[headerOffset] = headerCell;
			}
		}
		else
		{
			pNewHeader[j++].Type = 0;
			pNewHeader[j++].Type = 0;
			pNewHeader[j++].Type = 0;
			pNewHeader[j++].Type = 0;
		}
	}

	//copy members
	delete[] pHeader;
	pHeader = pNewHeader;

	HeaderBase = newHeaderBase;
	HeaderBits = newHeaderBits;
	HeaderSize = newHeaderSize;

	amountFreeSlotsBeforeHeaderResize = newAmountFreeSlotsBeforeHeaderResize;

	printf("Resize: %u\n", newHeaderBase);

	*/
}