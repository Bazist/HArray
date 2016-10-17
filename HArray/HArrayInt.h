/*
# Copyright(C) 2010-2016 Vyacheslav Makoveychuk (email: slv709@gmail.com, skype: vyacheslavm81)
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
# You should have received a copy of the GNU General Public License
# along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

struct HeaderCellInt
{
	uint Offset;
	ushort Code;
	uchar Type;
};

struct DoubleValueCellInt
{
	uint Value1;
	uint Value2;
	ushort Code2;
};

struct MultiplyValueCellInt
{
	uint Value;
	ushort Code;
};

class DoublePageInt
{
public:
	DoublePageInt()
	{
		for(uint i=0; i<PAGE_SIZE; i++)
		{
			pValues[i].Code2 = 0;
		}
	}

	DoubleValueCellInt pValues[PAGE_SIZE];

	~DoublePageInt()
	{
	}
};

class MultiplyPageInt
{
public:
	MultiplyPageInt()
	{
		for(uint i=0; i<PAGE_SIZE; i++)
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
		HEADER_SIZE = 255*255*255;
		BLOCK_SIZE = 255;

		BLOCK_BITS = 8;

		pDoublePages = 0;
		pMultiplyPages = 0;

		CountDoublePage = 0;
		CountMultiplyPage = 0;

		maxDoubleOffset = 0;
		maxMultiplyOffset = 0;

		lastHeaderCode = 1;

		currFreeCell = 0;
	}

	uint HEADER_BASE;
	uint CONTENT_BASE;

	uint HEADER_SIZE;
	uint BLOCK_SIZE;

	uint BLOCK_BITS;

	HeaderCellInt* pHeader;

	DoublePageInt** pDoublePages;
	MultiplyPageInt** pMultiplyPages;

	uint CountDoublePage;
	uint CountMultiplyPage;

	uint maxDoubleOffset;
	uint maxMultiplyOffset;

	ushort lastHeaderCode;

	uint indexes[MAX_SHORT];
	uint values[MAX_SHORT];

	uint freeCells[MAX_SHORT];
	uint currFreeCell;

	void init(uint headerBase, uint contentBase)
	{
		if(!contentBase)
			contentBase = 32-headerBase;	
	
		HEADER_BASE = headerBase;
		CONTENT_BASE = contentBase;
		
		ulong maxKey = 1;
	
		for(uint i=0; i<headerBase+contentBase; i++)
			maxKey *= 2;
	
		BLOCK_BITS = contentBase;
		HEADER_SIZE = (maxKey>>BLOCK_BITS);
		BLOCK_SIZE = (maxKey>>headerBase) - 1;

		pHeader = new HeaderCellInt[HEADER_SIZE+1];
	
		pDoublePages = new DoublePageInt*[INIT_MAX_PAGES];
		pMultiplyPages = new MultiplyPageInt*[INIT_MAX_PAGES];
	
		for(uint i=0; i<=HEADER_SIZE; i++)
		{
			pHeader[i].Type = 0;
		}

		for(uint i=0; i<INIT_MAX_PAGES; i++)
		{
			pDoublePages[i] = 0;
			pMultiplyPages[i] = 0;
		}

		pDoublePages[0] = new DoublePageInt();
		pMultiplyPages[0] = new MultiplyPageInt();

		CountDoublePage++;
		CountMultiplyPage++;
	}
		
	bool insert(uint key, uint value)
	{
		uint leftPart = (key>>BLOCK_BITS);
		uint rightPart = (key&BLOCK_SIZE);
		
		HeaderCellInt& headerCell = pHeader[leftPart];
		
		if(headerCell.Type)
		{
			//Existing block
			uint countValues;

			if(headerCell.Type == 1) //with one element in block
			{
				if(headerCell.Code == rightPart) //update
				{
					headerCell.Offset = value;
					return false;
				}

				//Allocate block
				if(currFreeCell > 0)
				{
					uint blockOffset = freeCells[--currFreeCell];
					DoublePageInt* pPage = pDoublePages[blockOffset>>16];
					DoubleValueCellInt& valueCell = pPage->pValues[blockOffset&0xFFFF];
				
					valueCell.Value1 = headerCell.Offset;

					valueCell.Code2 = rightPart;
					valueCell.Value2 = value;

					headerCell.Offset = blockOffset;
					headerCell.Type = 2;

					return true;
				}
			
				uint currPage = (maxDoubleOffset>>16);
				DoublePageInt* pPage = pDoublePages[currPage];

				if(!pPage)
				{
					pPage = new DoublePageInt();
					pDoublePages[currPage] = pPage;
					CountDoublePage++;
				}

				DoubleValueCellInt& valueCell = pPage->pValues[maxDoubleOffset&0xFFFF];

				valueCell.Value1 = headerCell.Offset;

				valueCell.Code2 = rightPart;
				valueCell.Value2 = value;

				headerCell.Offset = maxDoubleOffset++;
				headerCell.Type = 2;

				return true;
			}
			else if(headerCell.Type == 2) //with two elements in block
			{
				uint currPage = (headerCell.Offset>>16);
				DoublePageInt* pPage = pDoublePages[currPage];
		
				DoubleValueCellInt& valueCell = pPage->pValues[headerCell.Offset&0xFFFF];
			
				if(headerCell.Code == rightPart)
				{
					valueCell.Value1 = value;
					return false;
				}

				if(valueCell.Code2 == rightPart)
				{
					valueCell.Value2 = value;
					return false;
				}

				indexes[0] = headerCell.Code;
				values[0] = valueCell.Value1;

				indexes[1] = valueCell.Code2;
				values[1] = valueCell.Value2;

				indexes[2] = rightPart;
				values[2] = value;

				countValues = 3;

				if(currFreeCell < MAX_SHORT)
				{
					freeCells[currFreeCell++] = headerCell.Offset;
				}
			}
			else if(headerCell.Type == 3) //with multiply elements in block
			{
				uint offset = headerCell.Offset+rightPart;

				uint currPage = (offset>>16);
				MultiplyPageInt* pPage = pMultiplyPages[currPage];
		
				if(!pPage)
				{
					pPage = new MultiplyPageInt();
					pMultiplyPages[currPage] = pPage;
					CountMultiplyPage++;

					MultiplyValueCellInt& valueCell = pPage->pValues[offset&0xFFFF];
					valueCell.Value = value; //insert
					valueCell.Code = headerCell.Code;
					return true;
				}

				MultiplyValueCellInt& valueCell = pPage->pValues[offset&0xFFFF];
		
				if(valueCell.Code==0)
				{
					valueCell.Value = value; //insert
					valueCell.Code = headerCell.Code;
					return true;
				}
				else if(valueCell.Code==headerCell.Code)
				{
					valueCell.Value = value; //update
					return false;
				}
			
				//Extract block
				indexes[0] = rightPart;
				values[0] = value;

				countValues = 1;
			
				//with calculating MultiplyPageInt
				for(uint i=0; i<=BLOCK_SIZE; i++)
				{
					uint offset = headerCell.Offset + i;

					uint currPage = (offset>>16);
					MultiplyPageInt* pPage = pMultiplyPages[currPage];
		
					if(pPage)
					{
						MultiplyValueCellInt& valueCell = pPage->pValues[offset&0xFFFF];

						if(valueCell.Code == headerCell.Code)
						{
							indexes[countValues] = i;
							values[countValues] = valueCell.Value;
				
							countValues++;

							valueCell.Value = 0;
							valueCell.Code = 0;
						}
					}
				}
			}

			lastHeaderCode++;
			if(lastHeaderCode == 0)
				lastHeaderCode = 1;

			//Find offset for block
			uint blockOffset = maxMultiplyOffset;
			/*
			if(blockOffset > BLOCK_SIZE)
			{
				blockOffset -= BLOCK_SIZE;
			}
			*/
		
			while(true)
			{
				bool isFound = true;

				for(uint i=0; i<countValues; i++)
				{
					uint offset = blockOffset + indexes[i];

					uint currPage = (offset>>16);
					MultiplyPageInt* pPage = pMultiplyPages[currPage];

					if(!pPage)
					{
						pPage = new MultiplyPageInt();
						pMultiplyPages[currPage] = pPage;
						CountMultiplyPage++;
					}

					MultiplyValueCellInt& valueCell = pPage->pValues[offset&0xFFFF];

					if(valueCell.Code > 0)
					{
						isFound = false;
						break;
					}
				}
			
				if(isFound)
				{
					//Allocate block
					for(uint i=0; i<countValues; i++)
					{
						uint offset = blockOffset + indexes[i];

						uint currPage = (offset>>16);
						MultiplyPageInt* pPage = pMultiplyPages[currPage];
						MultiplyValueCellInt& valueCell = pPage->pValues[offset&0xFFFF];

						valueCell.Value = values[i]; //insert
						valueCell.Code = lastHeaderCode;
					}

					headerCell.Offset = blockOffset;
					headerCell.Code = lastHeaderCode;
					headerCell.Type = 3;

					if(blockOffset > maxMultiplyOffset)
					{
						maxMultiplyOffset = blockOffset;
					}

					break;
				}
				else
				{
					blockOffset++;
				}
			}
		}
		else
		{
			headerCell.Offset = value;
			headerCell.Code = rightPart;
			headerCell.Type = 1;
		}

		return true;
	}

	uint getValueByKey(uint key)
	{
		HeaderCellInt& headerCell = pHeader[key>>BLOCK_BITS];
		uint rightPart = (key&BLOCK_SIZE);
		
		switch (headerCell.Type)
		{
			case 1:
			{
				if(headerCell.Code == rightPart)
				{
					return headerCell.Offset;
				}

				break;
			}
			case 2:
			{
				DoublePageInt* pPage = pDoublePages[headerCell.Offset>>16];
				if(pPage)
				{
					DoubleValueCellInt& valueCell = pPage->pValues[headerCell.Offset&0xFFFF];

					if(headerCell.Code == rightPart)
					{
						return valueCell.Value1;
					}
					else if(valueCell.Code2 == rightPart)
					{
						return valueCell.Value2;
					}
				}

				break;
			}
			case 3:
			{
				uint offset = headerCell.Offset + rightPart;

				MultiplyPageInt* pPage = pMultiplyPages[offset>>16];
				if(pPage)
				{
					MultiplyValueCellInt& valueCell = pPage->pValues[offset&0xFFFF];

					if(valueCell.Code==headerCell.Code)
					{
						return valueCell.Value;
					}
				}

				break;
			}
		};

		return 0;
	}


	uint getValuesByRange(uint* buffer, 
						 uint size, 
						 uint startKey, 
						 uint endKey)
	{
		uint count = 0;

		uint startLeftPart = (startKey>>BLOCK_BITS);
		uint endLeftPart = (endKey>>BLOCK_BITS);

		//start
		for(uint currHeader = startLeftPart; currHeader <= endLeftPart; currHeader++)
		{
			HeaderCellInt& headerCell = pHeader[currHeader];
			if(headerCell.Type == 1)
			{
				buffer[count++] = headerCell.Offset;
			}
			else if(headerCell.Type == 2)
			{
				DoublePageInt* pPage = pDoublePages[headerCell.Offset>>16];
			
				if(pPage)
				{
					DoubleValueCellInt& valueCell = pPage->pValues[headerCell.Offset&0xFFFF];
					if(headerCell.Code < valueCell.Code2)
					{
						buffer[count++] = valueCell.Value1;
						buffer[count++] = valueCell.Value2;
					}
					else
					{
						buffer[count++] = valueCell.Value2;
						buffer[count++] = valueCell.Value1;
					}
				}
			}
			else if(headerCell.Type == 3)
			{
				uint startBlockOffset = headerCell.Offset;

				if(currHeader == startLeftPart)
				{
					startBlockOffset += (startKey&BLOCK_SIZE);
				}

				uint endBlockOffset = headerCell.Offset;

				if(currHeader == endLeftPart)
				{
					endBlockOffset += (endKey&BLOCK_SIZE);
				}
				else
				{
					endBlockOffset += BLOCK_SIZE;
				}
				
				for(uint offset = startBlockOffset; offset <= endBlockOffset; offset++)
				{
					uint currPage = (offset>>16);
					MultiplyPageInt* pPage = pMultiplyPages[currPage];
			
					if(pPage)
					{
						MultiplyValueCellInt& valueCell = pPage->pValues[offset&0xFFFF];

						if(valueCell.Code==headerCell.Code)
						{
							buffer[count++] = valueCell.Value;
						}
						
						if(count >= size)
						{
							break;
						}
					}
				}
			}
		}

		return count;
	}

	void rebuild()
	{
		//create new
		HArrayInt* pNewArray = new HArrayInt();
		pNewArray->init(HEADER_BASE, CONTENT_BASE);

		//copy
		uint count = 0;

		uint startKey = 0;
		uint endKey = 0xFFFFFFFF;

		uint startLeftPart = (startKey>>BLOCK_BITS);
		uint endLeftPart = (endKey>>BLOCK_BITS);

		//start
		for(uint currHeader = startLeftPart; currHeader <= endLeftPart; currHeader++)
		{
			/*if(currHeader == 941647)
			{
				currHeader = 941647;
			}*/

			HeaderCellInt& headerCell = pHeader[currHeader];
			if(headerCell.Type == 1)
			{
				uint key = (currHeader<<BLOCK_BITS) + headerCell.Code;
				uint value = headerCell.Offset;

				pNewArray->insert(key, value);
			}
			else if(headerCell.Type == 2)
			{
				DoublePageInt* pPage = pDoublePages[headerCell.Offset>>16];
			
				if(pPage)
				{
					DoubleValueCellInt& valueCell = pPage->pValues[headerCell.Offset&0xFFFF];

					uint key1 = (currHeader<<BLOCK_BITS) + headerCell.Code;
					uint value1 = valueCell.Value1;

					uint key2 = (currHeader<<BLOCK_BITS) + valueCell.Code2;
					uint value2 = valueCell.Value2;

					if(headerCell.Code < valueCell.Code2)
					{
						pNewArray->insert(key1, value1);
						pNewArray->insert(key2, value2);
					}
					else
					{
						pNewArray->insert(key2, value2);
						pNewArray->insert(key1, value1);
					}
				}
			}
			else if(headerCell.Type == 3)
			{
				uint startBlockOffset = headerCell.Offset;

				if(currHeader == startLeftPart)
				{
					startBlockOffset += (startKey&BLOCK_SIZE);
				}

				uint endBlockOffset = headerCell.Offset;

				if(currHeader == endLeftPart)
				{
					endBlockOffset += (endKey&BLOCK_SIZE);
				}
				else
				{
					endBlockOffset += BLOCK_SIZE;
				}
				
				for(uint offset = startBlockOffset, code = 0; offset < endBlockOffset; offset++, code++)
				{
					uint currPage = (offset>>16);
					MultiplyPageInt* pPage = pMultiplyPages[currPage];
			
					if(pPage)
					{
						MultiplyValueCellInt& valueCell = pPage->pValues[offset&0xFFFF];

						if(valueCell.Code==headerCell.Code)
						{
							uint key = (currHeader<<BLOCK_BITS) + code;
							uint value = valueCell.Value;

							pNewArray->insert(key, value);
						}
					}
				}
			}
		}

		//destroy
		this->destroy();

		this->pHeader = pNewArray->pHeader;
		this->pDoublePages = pNewArray->pDoublePages;
		this->pMultiplyPages = pNewArray->pMultiplyPages;

		this->CountDoublePage = pNewArray->CountDoublePage;
		this->CountMultiplyPage = pNewArray->CountMultiplyPage;

		this->maxDoubleOffset = pNewArray->maxDoubleOffset;
		this->maxMultiplyOffset = pNewArray->maxMultiplyOffset;

		this->lastHeaderCode = pNewArray->lastHeaderCode;
		
		delete pNewArray;
	}

	uint getKeysAndValuesByRange(uint* buffer, 
								 uint size, 
								 uint startKey, 
								 uint endKey)
	{
		uint count = 0;

		uint startLeftPart = (startKey>>BLOCK_BITS);
		uint endLeftPart = (endKey>>BLOCK_BITS);

		//start
		for(uint currHeader = startLeftPart; currHeader <= endLeftPart; currHeader++)
		{
			HeaderCellInt& headerCell = pHeader[currHeader];
			if(headerCell.Type == 1)
			{
				uint key = (currHeader<<BLOCK_BITS) + headerCell.Code;
				uint value = headerCell.Offset;

				buffer[count++] = key;
				buffer[count++] = value;
			}
			else if(headerCell.Type == 2)
			{
				DoublePageInt* pPage = pDoublePages[headerCell.Offset>>16];
			
				if(pPage)
				{
					DoubleValueCellInt& valueCell = pPage->pValues[headerCell.Offset&0xFFFF];

					uint key1 = (currHeader<<BLOCK_BITS) + headerCell.Code;
					uint value1 = valueCell.Value1;

					uint key2 = (currHeader<<BLOCK_BITS) + valueCell.Code2;
					uint value2 = valueCell.Value2;

					if(headerCell.Code < valueCell.Code2)
					{
						buffer[count++] = key1;
						buffer[count++] = value1;
						buffer[count++] = key2;
						buffer[count++] = value2;
					}
					else
					{
						buffer[count++] = key2;
						buffer[count++] = value2;
						buffer[count++] = key1;
						buffer[count++] = value1;
					}
				}
			}
			else if(headerCell.Type == 3)
			{
				uint startBlockOffset = headerCell.Offset;

				if(currHeader == startLeftPart)
				{
					startBlockOffset += (startKey&BLOCK_SIZE);
				}

				uint endBlockOffset = headerCell.Offset;

				if(currHeader == endLeftPart)
				{
					endBlockOffset += (endKey&BLOCK_SIZE);
				}
				else
				{
					endBlockOffset += BLOCK_SIZE;
				}
				
				for(uint offset = startBlockOffset, code = 0; offset <= endBlockOffset; offset++, code++)
				{
					uint currPage = (offset>>16);
					MultiplyPageInt* pPage = pMultiplyPages[currPage];
			
					if(pPage)
					{
						MultiplyValueCellInt& valueCell = pPage->pValues[offset&0xFFFF];

						if(valueCell.Code==headerCell.Code)
						{
							uint key = (currHeader<<BLOCK_BITS) + code;
							uint value = valueCell.Value;

							buffer[count++] = key;
							buffer[count++] = value;
						}
					}
				}
			}
		}

		return count;
	}
	
	uint getUsedMemory()
	{
		return sizeof(HArrayInt) + HEADER_SIZE * sizeof(HeaderCellInt) + CountDoublePage * sizeof(DoublePageInt) + CountMultiplyPage * sizeof(MultiplyPageInt);
	}

	void destroy()
	{
		if(pHeader)
		{
			delete[] pHeader;
			pHeader = 0;
		}
	
		if(pDoublePages)
		{
			for(uint i=0; i<CountDoublePage; i++)
				delete pDoublePages[i];

			delete[] pDoublePages;
			pDoublePages = 0;
		}
		
		if(pMultiplyPages)
		{
			for(uint i=0; i<CountMultiplyPage; i++)
				delete pMultiplyPages[i];
	
			delete[] pMultiplyPages;
			pMultiplyPages = 0;
		}
	}

};	