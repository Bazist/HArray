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

#include "stdafx.h"

struct HeaderCellInt
{
	uchar8 Type;
	ushort16 Code;
	uint32 Offset;
	
};

struct MultiplyValueCellInt
{
	uint32 Value;
	ushort16 Code;
};

class MultiplyPageInt
{
public:
	MultiplyPageInt()
	{
		for (uint32 i = 0; i < PAGE_SIZE; i++)
		{
			pValues[i].Code = 0;
		}
	}

	MultiplyValueCellInt pValues[PAGE_SIZE];

	~MultiplyPageInt()
	{
	}
};

class HArrayInt
{
public:
	HArrayInt()
	{
		HEADER_SIZE = 255 * 255 * 255;
		BLOCK_SIZE = 255;

		BLOCK_BITS = 8;

		pMultiplyPages = 0;

		CountDoublePage = 0;
		CountMultiplyPage = 0;

		maxDoubleOffset = 0;
		maxMultiplyOffset = 0;

		lastHeaderCode = 1;

		currReleaseCell = 0;
	}

	uint32 HEADER_BASE;
	uint32 CONTENT_BASE;

	uint32 HEADER_SIZE;
	uint32 BLOCK_SIZE;

	uint32 BLOCK_BITS;

	HeaderCellInt* pHeader;

	MultiplyPageInt** pMultiplyPages;

	uint32 CountDoublePage;
	uint32 CountMultiplyPage;

	uint32 maxDoubleOffset;
	uint32 maxMultiplyOffset;

	ushort16 lastHeaderCode;

	uint32 indexes[MAX_SHORT];
	uint32 values[MAX_SHORT];

	uint32 releaseCells[MAX_SHORT];
	uint32 currReleaseCell;

	void init(uint32 headerBase)
	{
		//clear pointers
		pHeader = 0;
		pMultiplyPages = 0;

		try
		{
			HEADER_BASE = headerBase;
			CONTENT_BASE = 32 - headerBase;

			ulong64 maxKey = 1;
			maxKey <<= (headerBase + CONTENT_BASE);

			BLOCK_BITS = CONTENT_BASE;
			HEADER_SIZE = (maxKey >> BLOCK_BITS);
			BLOCK_SIZE = (maxKey >> headerBase) - 1;

			pHeader = new HeaderCellInt[HEADER_SIZE + 1];

			pMultiplyPages = new MultiplyPageInt * [INIT_MAX_PAGES];

			for (uint32 i = 0; i <= HEADER_SIZE; i++)
			{
				pHeader[i].Type = 0;
			}

			for (uint32 i = 0; i < INIT_MAX_PAGES; i++)
			{
				pMultiplyPages[i] = 0;
			}

			pMultiplyPages[0] = new MultiplyPageInt();

			CountDoublePage++;
			CountMultiplyPage++;
		}
		catch (...)
		{
			destroy();

			throw;
		}
	}

	void stat()
	{
		int count1 = 0;
		int count2 = 0;
		int count3 = 0;

		for (int i = 1; i < HEADER_SIZE - 2; i++)
		{
			if (pHeader[i].Type > 0 && pHeader[i + 1].Type == 0)
			{
				count1++;
			}

			if (pHeader[i].Type > 0 && (pHeader[i - 1].Type == 0 || pHeader[i + 1].Type == 0))
			{
				count2++;
			}

			if (pHeader[i].Type > 0 && pHeader[i + 1].Type == 0 && pHeader[i + 2].Type == 0)
			{
				count3++;
			}
		}

		printf("\nright free %d\n", count1);

		printf("\nright or left free %d\n", count2);

		printf("\nright two free %d\n", count3);
	}

	const int BRANCH_SIZE = 16;

	bool insert(uint32 key, uint32 value)
	{
		try
		{
			uint32 leftPart = (key >> BLOCK_BITS);
			uint32 rightPart = (key & BLOCK_SIZE);

			HeaderCellInt& headerCell = pHeader[leftPart];

			if (headerCell.Type == 0)
			{
				//embedded value
				headerCell.Type = 1;
				headerCell.Code = rightPart;
				headerCell.Offset = value;

				return true;
			}
			else if (headerCell.Type < BRANCH_SIZE)
			{
				//update value
				for (uint32 i = 0; i < headerCell.Type; i++)
				{
					HeaderCellInt& currHeaderCell = pHeader[leftPart + i];
					
					if (currHeaderCell.Code == rightPart)
					{
						currHeaderCell.Offset = value;
						return false;
					}
				}

				HeaderCellInt& rightFreeHeaderCell = pHeader[leftPart + headerCell.Type];

				if (rightFreeHeaderCell.Type == 0)
				{
					//embedded branch
					headerCell.Type++;

					rightFreeHeaderCell.Type = BRANCH_SIZE + 1;
					rightFreeHeaderCell.Code = rightPart;
					rightFreeHeaderCell.Offset = value;

					return true;
				}
				else
				{
					//external branch


				}
			}
			
			//extract branch


			return true;
		}
		catch (...)
		{
			destroy();

			throw;
		}
	}

	uint32 getValueByKey(uint32 key)
	{
		HeaderCellInt& headerCell = pHeader[key >> BLOCK_BITS];
		uint32 rightPart = (key & BLOCK_SIZE);

		switch (headerCell.Type)
		{
			case 1:
			{
				if (headerCell.Code == rightPart)
				{
					return headerCell.Offset;
			}

			break;
		}
		case 2:
		{
	
			break;
		}
		case 3:
		{
			uint32 offset = headerCell.Offset + rightPart;

			MultiplyPageInt* pPage = pMultiplyPages[offset >> 16];
			if (pPage)
			{
				MultiplyValueCellInt& valueCell = pPage->pValues[offset & 0xFFFF];

				if (valueCell.Code == headerCell.Code)
				{
					return valueCell.Value;
				}
			}

			break;
		}
		};

		return 0;
	}

	uint32 getUsedMemory()
	{
		return sizeof(HArrayInt) + HEADER_SIZE * sizeof(HeaderCellInt) + CountMultiplyPage * sizeof(MultiplyPageInt);
	}

	void destroy()
	{
		if (pHeader)
		{
			delete[] pHeader;
			pHeader = 0;
		}
				
		if (pMultiplyPages)
		{
			for (uint32 i = 0; i < CountMultiplyPage; i++)
				delete pMultiplyPages[i];

			delete[] pMultiplyPages;
			pMultiplyPages = 0;
		}
	}

};
