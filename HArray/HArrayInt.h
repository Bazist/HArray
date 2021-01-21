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
	uchar8 Type2;
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

		printf("\nExtBranch1 %d\n", ExtBranch1);

		printf("\nExtBranch2 %d\n", ExtBranch2);

		printf("\nExtBranch3 %d\n", ExtBranch3);

		printf("\nExtBlock %d\n", ExtBlock);

		printf("\nCollision1 %d\n", Collision1);

		printf("\nCollisionN %d\n", CollisionN);
	}

	const static int BRANCH_SIZE = 16;
	const static int MAX_JUMP_ITEMS = 16;

	const static int EMPTY_TYPE = 0;
	const static int SINGLE_TYPE = 1;
	const static int BRANCH_TYPE = 2;
	const static int BRANCH_ITEM_TYPE = BRANCH_TYPE + MAX_JUMP_ITEMS + 1;
	//BRANCH_ITEM_TYPE + 16 jumps
	const static int BRANCH_ITEM_TYPE_AFTER_JUMP = BRANCH_ITEM_TYPE + MAX_JUMP_ITEMS + 1;

	const static int BRANCH_ITEM_TYPE_WITH_COLLISION = BRANCH_ITEM_TYPE_AFTER_JUMP + 1;
	//BRANCH_ITEM_TYPE + 16 jumps

	const static int BRANCH_ITEM_TYPE_AFTER_JUMP_WITH_COLLISION = BRANCH_ITEM_TYPE_AFTER_JUMP + MAX_JUMP_ITEMS + 1;
	//BRANCH_ITEM_TYPE + 16 jumps

	const static int BLOCK_TYPE = BRANCH_ITEM_TYPE_AFTER_JUMP_WITH_COLLISION + MAX_JUMP_ITEMS + 1;


	int ExtBranch1 = 0;
	int ExtBranch2 = 0;
	int ExtBranch3 = 0;
	int ExtBlock = 0;
	int Collision1 = 0;
	int CollisionN = 0;

	bool insert(uint32 key, uint32 value)
	{
		try
		{
			uint32 leftPart = (key >> BLOCK_BITS);
			uint32 rightPart = (key & BLOCK_SIZE);

			HeaderCellInt& headerCell = pHeader[leftPart];

			switch (headerCell.Type)
			{
				case EMPTY_TYPE: //WAVE 1
				{
					//embedded value
					headerCell.Type = SINGLE_TYPE;
					headerCell.Code = rightPart;
					headerCell.Offset = value;

					return true;
				}
				case SINGLE_TYPE: //WAVE 2
				{
					if (headerCell.Code == rightPart)
					{
						headerCell.Offset = value;

						return false;
					}

					HeaderCellInt& rightFreeHeaderCell = pHeader[leftPart + 1];

					if (rightFreeHeaderCell.Type == EMPTY_TYPE)
					{
						//embedded branch
						headerCell.Type = BRANCH_TYPE;

						rightFreeHeaderCell.Type = BRANCH_ITEM_TYPE;
						rightFreeHeaderCell.Code = rightPart;
						rightFreeHeaderCell.Offset = value;

						return true;
					}
					else
					{
						for (uint32 offset = leftPart + 1, jump = 1; jump < MAX_JUMP_ITEMS; offset++, jump++)
						{
							if (pHeader[offset].Type == EMPTY_TYPE)
							{
								headerCell.Type = BRANCH_TYPE + jump;

								pHeader[offset].Type = BRANCH_ITEM_TYPE_AFTER_JUMP;
								pHeader[offset].Code = rightPart;
								pHeader[offset].Offset = value;

								return true;
							}
						}

						ExtBlock++;

						return false;
					}
				}
				case BRANCH_TYPE: //WAVE3
				{
					uint32 i = leftPart;

					while (true)
					{
						if (pHeader[i].Type == BRANCH_ITEM_TYPE)
						{
							if (pHeader[i].Code == rightPart)
							{
								pHeader[i].Offset = value;

								return false;
							}
							else
							{
								i++;
							}
						}
						else if (BRANCH_ITEM_TYPE < pHeader[i].Type && pHeader[i].Type < BRANCH_ITEM_TYPE_AFTER_JUMP)
						{
							if (pHeader[i].Code == rightPart)
							{
								pHeader[i].Offset = value;

								return false;
							}
							else
							{
								i += BRANCH_ITEM_TYPE_AFTER_JUMP - pHeader[i].Type;
							}
						}
						else
						{
							break;
						}
					}

					//insert value
					if (pHeader[i].Type == EMPTY_TYPE)
					{
						pHeader[i].Type = BRANCH_ITEM_TYPE;
						pHeader[i].Code = rightPart;
						pHeader[i].Offset = value;

						return true;
					}

					for (uint32 j = i + 1, jump = 1; jump <= MAX_JUMP_ITEMS; j++, jump++)
					{
						if (pHeader[j].Type == EMPTY_TYPE)
						{
							pHeader[j].Type = BRANCH_ITEM_TYPE_AFTER_JUMP;
							pHeader[j].Code = rightPart;
							pHeader[j].Offset = value;

							pHeader[i].Type = BRANCH_ITEM_TYPE + jump;

							return true;
						}
					}
					
					//jump
					ExtBranch1++;

				}
				case BRANCH_ITEM_TYPE:
				{
					//collision
					for (uint32 offset = leftPart + 1, jump = 1; jump < MAX_JUMP_ITEMS; offset++, jump++)
					{
						if (pHeader[offset].Type == EMPTY_TYPE)
						{
							headerCell.Type = BRANCH_ITEM_TYPE_WITH_COLLISION + jump;

							pHeader[offset].Type = BRANCH_ITEM_TYPE_AFTER_JUMP;
							pHeader[offset].Code = rightPart;
							pHeader[offset].Offset = value;

							return true;
						}
					}

					Collision1++;

					return true;
				}
				case BRANCH_ITEM_TYPE_AFTER_JUMP:
				{
					//collision
					for (uint32 offset = leftPart + 1, jump = 1; jump < MAX_JUMP_ITEMS; offset++, jump++)
					{
						if (pHeader[offset].Type == EMPTY_TYPE)
						{
							headerCell.Type = BRANCH_ITEM_TYPE_AFTER_JUMP_WITH_COLLISION + jump;

							pHeader[offset].Type = BRANCH_ITEM_TYPE_AFTER_JUMP;
							pHeader[offset].Code = rightPart;
							pHeader[offset].Offset = value;

							return true;
						}
					}

					Collision1++;

					return true;
				}
			}

			CollisionN++;

			//const static int BRANCH_ITEM_TYPE = BRANCH_TYPE + MAX_JUMP_ITEMS + 1;
			//BRANCH_ITEM_TYPE + 16 jumps
			//const static int BRANCH_ITEM_TYPE_AFTER_JUMP = BRANCH_ITEM_TYPE + MAX_JUMP_ITEMS + 1;

			//const static int BRANCH_ITEM_TYPE_WITH_COLLISION = BRANCH_ITEM_TYPE_AFTER_JUMP + 1;
			//BRANCH_ITEM_TYPE + 16 jumps

			//const static int BRANCH_ITEM_TYPE_AFTER_JUMP_WITH_COLLISION = BRANCH_ITEM_TYPE_AFTER_JUMP + MAX_JUMP_ITEMS + 1;
			//BRANCH_ITEM_TYPE + 16 jumps

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
