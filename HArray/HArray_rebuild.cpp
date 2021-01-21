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
#include "HArray.h"

struct RebuildData
{
public:
	HArray* pOldHA;
	HArray* pNewHA;

	bool RemoveEmptyKeys;
	uint32 Count;
};

bool HArray::rebuildVisitor(uint32* key, uint32 keyLen, uint32 value, uchar8 valueType, void* pData)
{
	RebuildData* pRD = (RebuildData*)pData;

	if (!pRD->RemoveEmptyKeys || value)
	{
		pRD->pNewHA->insert(key, keyLen << 2, value);

		pRD->Count++;
	}

	return true;
}

uint32 HArray::rebuild(uint32 headerBase, bool removeEmptyKeys)
{
	if(!headerBase)
	{
		headerBase = this->HeaderBase;
	}

	//create new HA
	HArray* pNewHA = new HArray();
	pNewHA->init(headerBase);

	//move elements
	RebuildData rd;
	rd.pOldHA = this;
	rd.pNewHA = pNewHA;
	rd.RemoveEmptyKeys = removeEmptyKeys;
	rd.Count = 0;

	//scan all elements + insert to new HArray
	this->scanKeysAndValues(&rebuildVisitor,
							&rd);

	//destroy old
	this->destroy();

	//copy data from new to old
	this->pHeader = pNewHA->pHeader;
	this->pContentPages = pNewHA->pContentPages;
	this->pVarPages = pNewHA->pVarPages;
	this->pBranchPages = pNewHA->pBranchPages;
	this->pBlockPages = pNewHA->pBlockPages;

	this->ContentPagesCount = pNewHA->ContentPagesCount;
	this->VarPagesCount = pNewHA->VarPagesCount;
	this->BranchPagesCount = pNewHA->BranchPagesCount;
	this->BlockPagesCount = pNewHA->BlockPagesCount;
	
	this->ContentPagesSize = pNewHA->ContentPagesSize;
	this->VarPagesSize = pNewHA->VarPagesSize;
	this->BranchPagesSize = pNewHA->BranchPagesSize;
	this->BlockPagesSize = pNewHA->BlockPagesSize;

	this->normalizeFunc = pNewHA->normalizeFunc;
	this->compareFunc = pNewHA->compareFunc;
	this->compareSegmentFunc = pNewHA->compareSegmentFunc;

	this->HeaderBase = pNewHA->HeaderBase;
	this->HeaderBits = pNewHA->HeaderBits;
	this->HeaderSize = pNewHA->HeaderSize;

	this->tailReleasedBranchOffset = pNewHA->tailReleasedBranchOffset;
	this->countReleasedBranchCells = pNewHA->countReleasedBranchCells;

	this->tailReleasedBlockOffset = pNewHA->tailReleasedBlockOffset;
	this->countReleasedBlockCells = pNewHA->countReleasedBlockCells;

	this->tailReleasedVarOffset = pNewHA->tailReleasedVarOffset;
	this->countReleasedVarCells = pNewHA->countReleasedVarCells;

	this->ValueLen = pNewHA->ValueLen;
	this->NewParentID = pNewHA->NewParentID;
	this->MAX_SAFE_SHORT = pNewHA->MAX_SAFE_SHORT;

	this->lastHeaderBranchOffset = pNewHA->lastHeaderBranchOffset;
	this->lastContentOffset = pNewHA->lastContentOffset;
	this->lastVarOffset = pNewHA->lastVarOffset;
	this->lastBranchOffset = pNewHA->lastBranchOffset;
	this->lastBlockOffset = pNewHA->lastBlockOffset;

	//delete donor
	delete pNewHA;

	return rd.Count;
}
