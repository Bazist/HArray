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
# You should have received a copy of the GNU General Public LicenseЕ
# along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "HArray.h"

void HArray::releaseContentCells(uint32* pContentCellValue, uint32 contentOffset, uint32 keyLen)
{
	#ifndef _RELEASE

	printf("releaseContentCells => %u\n", keyLen);

	#endif

	uint32 dataLen = keyLen + ValueLen;

	memset(pContentCellValue, 0, dataLen * sizeof(uint32));

	*pContentCellValue = tailReleasedContentOffsets[keyLen];

	tailReleasedContentOffsets[keyLen] = contentOffset;

	countReleasedContentCells += dataLen;
}

void HArray::releaseBranchCell(BranchCell* pBranchCell, uint32 branchOffset)
{
	#ifndef _RELEASE

	printf("releaseBranchCell => 1\n");

	#endif

	memset(pBranchCell, 0, sizeof(BranchCell));

	pBranchCell->Values[0] = tailReleasedBranchOffset;

	tailReleasedBranchOffset = branchOffset;

	countReleasedBranchCells++;
}

void HArray::releaseBlockCells(BlockCell* pStartBlockCell, uint32 startBlockOffset)
{
	#ifndef _RELEASE

	printf("releaseBlockCells => 16\n");

	#endif

	memset(pStartBlockCell, 0, sizeof(BlockCell) * BLOCK_ENGINE_SIZE);

	pStartBlockCell->Offset = tailReleasedBlockOffset;

	tailReleasedBlockOffset = startBlockOffset;

	countReleasedBlockCells += BLOCK_ENGINE_SIZE;
}

void HArray::releaseVarCell(VarCell* pVarCell, uint32 varOffset)
{
	#ifndef _RELEASE

	printf("releaseVarCell => 1\n");

	#endif

	memset(pVarCell, 0, sizeof(VarCell));

	pVarCell->ValueContCellValue = tailReleasedVarOffset;

	tailReleasedVarOffset = varOffset;

	countReleasedVarCells++;
}

bool HArray::tryReleaseBlock(SegmentPath* path, uint32 pathLen, int32& currPathLen)
{
	//check block ==============================================================
	//scan values in block

	SegmentPath& sp = path[currPathLen];

	uint32 values[BRANCH_ENGINE_SIZE * 2];
	uint32 offsets[BRANCH_ENGINE_SIZE * 2];
	uint32 count = 0;

	BranchCell* releaseBranchCells[BRANCH_ENGINE_SIZE * 2];
	uint32 releaseBranchOffsets[BRANCH_ENGINE_SIZE * 2];
	uint32 countReleaseBranchCells = 0;

	uint32 subOffset = 0;
	BlockCell* pCurrBlockCell = sp.pBlockCell - sp.BlockSubOffset;

	for (; subOffset < BLOCK_ENGINE_SIZE; subOffset++, pCurrBlockCell++)
	{
		if (pCurrBlockCell->Type)
		{
			if (pCurrBlockCell->Type == CURRENT_VALUE_TYPE)
			{
				if (count >= BRANCH_ENGINE_SIZE * 2) //leave block, do nothing, exit
				{
					return false;
				}

				values[count] = pCurrBlockCell->ValueOrOffset;
				offsets[count] = pCurrBlockCell->Offset;

				count++;
			}
			else
			{
				if (count + pCurrBlockCell->Type > BRANCH_ENGINE_SIZE * 2) //leave block, do nothing, exit
				{
					return false;
				}

				if (pCurrBlockCell->Type <= MAX_BRANCH_TYPE1)
				{
					BranchCell& branchCell1 = pBranchPages[pCurrBlockCell->Offset >> 16]->pBranch[pCurrBlockCell->Offset & 0xFFFF];

					for (uint32 i = 0; i < pCurrBlockCell->Type; i++, count++)
					{
						values[count] = branchCell1.Values[i];
						offsets[count] = branchCell1.Offsets[i];
					}

					releaseBranchCells[countReleaseBranchCells] = &branchCell1;
					releaseBranchOffsets[countReleaseBranchCells] = pCurrBlockCell->Offset;
					countReleaseBranchCells++;
				}
				else if (pCurrBlockCell->Type <= MAX_BRANCH_TYPE2)
				{
					BranchCell& branchCell1 = pBranchPages[pCurrBlockCell->Offset >> 16]->pBranch[pCurrBlockCell->Offset & 0xFFFF];

					for (uint32 i = 0; i < BRANCH_ENGINE_SIZE; i++, count++)
					{
						values[count] = branchCell1.Values[i];
						offsets[count] = branchCell1.Offsets[i];
					}

					releaseBranchCells[countReleaseBranchCells] = &branchCell1;
					releaseBranchOffsets[countReleaseBranchCells] = pCurrBlockCell->Offset;
					countReleaseBranchCells++;

					BranchCell& branchCell2 = pBranchPages[pCurrBlockCell->ValueOrOffset >> 16]->pBranch[pCurrBlockCell->ValueOrOffset & 0xFFFF];

					uint32 countValues = pCurrBlockCell->Type - MAX_BRANCH_TYPE1;

					for (uint32 i = 0; i < countValues; i++, count++)
					{
						values[count] = branchCell2.Values[i];
						offsets[count] = branchCell2.Offsets[i];
					}

					releaseBranchCells[countReleaseBranchCells] = &branchCell2;
					releaseBranchOffsets[countReleaseBranchCells] = pCurrBlockCell->ValueOrOffset;
					countReleaseBranchCells++;
				}
			}
		}
	}

	//if less than 8 values ===================================================================================
	if (currPathLen <= 0 || 
		path[currPathLen - 1].Type != BLOCK_OFFSET_SEGMENT_TYPE) //top block
	{
		if (count == 0)
		{
			//release branches on block
			for(uint32 i=0; i<countReleaseBranchCells; i++)
			{
				releaseBranchCell(releaseBranchCells[i],
								  releaseBranchOffsets[i]);
			}

			releaseBlockCells(sp.pBlockCell - sp.BlockSubOffset, sp.StartBlockOffset);

			dismantlingContentCells(path, currPathLen);

			return true;
		}
		else if (count == 1) //remove block, create CURRENT_VALUE_TYPE
		{
			//if last element offset+1, inject to content;
			if (offsets[0] == sp.ContentOffset + 1)
			{
				*sp.pContentCellType = CURRENT_VALUE_TYPE;
				*sp.pContentCellValue = values[0];

				//release branches on block
				for(uint32 i=0; i<countReleaseBranchCells; i++)
				{
					releaseBranchCell(releaseBranchCells[i],
									  releaseBranchOffsets[i]);
				}

				releaseBlockCells(sp.pBlockCell - sp.BlockSubOffset, sp.StartBlockOffset);
			}
		}
		else if (count <= BRANCH_ENGINE_SIZE) //remove block, create BRANCH
		{
			//release branches on block
			for(uint32 i=0; i<countReleaseBranchCells; i++)
			{
				releaseBranchCell(releaseBranchCells[i],
								  releaseBranchOffsets[i]);
			}

			//create branch ==================================================================================
			uint32 branchOffset;
			BranchCell* pBranchCell;

			if (countReleasedBranchCells)
			{
				branchOffset = tailReleasedBranchOffset;

				pBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];

				tailReleasedBranchOffset = pBranchCell->Values[0];

				countReleasedBranchCells--;
			}
			else
			{
				BranchPage* pBranchPage = pBranchPages[lastBranchOffset >> 16];
				if (!pBranchPage)
				{
					pBranchPage = new BranchPage();
					pBranchPages[BranchPagesCount++] = pBranchPage;

					if (BranchPagesCount == BranchPagesSize)
					{
						reallocateBranchPages();
					}
				}

				branchOffset = lastBranchOffset++;
				pBranchCell = &pBranchPage->pBranch[branchOffset & 0xFFFF];
			}

			//fill branch
			for (uint32 i = 0; i < count; i++)
			{
				pBranchCell->Offsets[i] = offsets[i];
				pBranchCell->Values[i] = values[i];
			}

			//set content cell
			*sp.pContentCellType = MIN_BRANCH_TYPE1 + count - 1;
			*sp.pContentCellValue = branchOffset;

			//release block cell
			releaseBlockCells(sp.pBlockCell - sp.BlockSubOffset, sp.StartBlockOffset);
		}
	}
	else //sub block
	{
		SegmentPath& prevSP = path[currPathLen - 1];

		if (count == 0)
		{
			prevSP.pBlockCell->Type = 0;
			prevSP.pBlockCell->ValueOrOffset = 0;
			prevSP.pBlockCell->Offset = 0;

			//release branches on block
			for(uint32 i=0; i<countReleaseBranchCells; i++)
			{
				releaseBranchCell(releaseBranchCells[i],
								  releaseBranchOffsets[i]);
			}

			releaseBlockCells(sp.pBlockCell - sp.BlockSubOffset, sp.StartBlockOffset);

			return true;
		}
		else if (count == 1) //remove block, create CURRENT_VALUE_TYPE
		{
			prevSP.pBlockCell->Type = CURRENT_VALUE_TYPE;
			prevSP.pBlockCell->ValueOrOffset = values[0];
			prevSP.pBlockCell->Offset = offsets[0];

			//release branches on block
			for(uint32 i=0; i<countReleaseBranchCells; i++)
			{
				releaseBranchCell(releaseBranchCells[i],
								  releaseBranchOffsets[i]);
			}

			releaseBlockCells(sp.pBlockCell - sp.BlockSubOffset, sp.StartBlockOffset);
		}
		else if (count <= BRANCH_ENGINE_SIZE) //remove block, create BRANCH
		{
			//release branches on block
			for(uint32 i=0; i<countReleaseBranchCells; i++)
			{
				releaseBranchCell(releaseBranchCells[i],
								  releaseBranchOffsets[i]);
			}

			//create branch ==================================================================================
			//get free branch cell
			uint32 branchOffset;
			BranchCell* pBranchCell;

			if (countReleasedBranchCells)
			{
				branchOffset = tailReleasedBranchOffset;

				pBranchCell = &pBranchPages[branchOffset >> 16]->pBranch[branchOffset & 0xFFFF];

				tailReleasedBranchOffset = pBranchCell->Values[0];

				countReleasedBranchCells--;
			}
			else
			{
				BranchPage* pBranchPage = pBranchPages[lastBranchOffset >> 16];
				if (!pBranchPage)
				{
					pBranchPage = new BranchPage();
					pBranchPages[BranchPagesCount++] = pBranchPage;

					if (BranchPagesCount == BranchPagesSize)
					{
						reallocateBranchPages();
					}
				}

				branchOffset = lastBranchOffset++;
				pBranchCell = &pBranchPage->pBranch[branchOffset & 0xFFFF];
			}

			//fill branch
			for (uint32 i = 0; i < count; i++)
			{
				pBranchCell->Offsets[i] = offsets[i];
				pBranchCell->Values[i] = values[i];
			}

			//set content cell
			prevSP.pBlockCell->Type = MIN_BRANCH_TYPE1 + count - 1;
			prevSP.pBlockCell->Offset = branchOffset;
			prevSP.pBlockCell->ValueOrOffset = 0;

			//release block
			releaseBlockCells(sp.pBlockCell - sp.BlockSubOffset, sp.StartBlockOffset);
		}
		else if (count <= BRANCH_ENGINE_SIZE * 2) //remove block, create two BRANCHES
		{
			//release branches on block
			for(uint32 i=0; i<countReleaseBranchCells; i++)
			{
				releaseBranchCell(releaseBranchCells[i],
								  releaseBranchOffsets[i]);
			}

			//create branch 1 ==================================================================================
			//get free branch cell
			uint32 branchOffset1;
			BranchCell* pBranchCell1;

			if (countReleasedBranchCells)
			{
				branchOffset1 = tailReleasedBranchOffset;

				pBranchCell1 = &pBranchPages[branchOffset1 >> 16]->pBranch[branchOffset1 & 0xFFFF];

				tailReleasedBranchOffset = pBranchCell1->Values[0];

				countReleasedBranchCells--;
			}
			else
			{
				BranchPage* pBranchPage = pBranchPages[lastBranchOffset >> 16];
				if (!pBranchPage)
				{
					pBranchPage = new BranchPage();
					pBranchPages[BranchPagesCount++] = pBranchPage;

					if (BranchPagesCount == BranchPagesSize)
					{
						reallocateBranchPages();
					}
				}

				branchOffset1 = lastBranchOffset++;
				pBranchCell1 = &pBranchPage->pBranch[branchOffset1 & 0xFFFF];
			}

			//fill branch
			uint32 i = 0;
			for (; i < BRANCH_ENGINE_SIZE; i++)
			{
				pBranchCell1->Offsets[i] = offsets[i];
				pBranchCell1->Values[i] = values[i];
			}

			//create branch 2 ================================================================================
			//get free branch cell
			uint32 branchOffset2;
			BranchCell* pBranchCell2;

			if (countReleasedBranchCells)
			{
				branchOffset2 = tailReleasedBranchOffset;

				pBranchCell2 = &pBranchPages[branchOffset2 >> 16]->pBranch[branchOffset2 & 0xFFFF];

				tailReleasedBranchOffset = pBranchCell2->Values[0];

				countReleasedBranchCells--;
			}
			else
			{
				BranchPage* pBranchPage = pBranchPages[lastBranchOffset >> 16];
				if (!pBranchPage)
				{
					pBranchPage = new BranchPage();
					pBranchPages[BranchPagesCount++] = pBranchPage;

					if (BranchPagesCount == BranchPagesSize)
					{
						reallocateBranchPages();
					}
				}

				branchOffset2 = lastBranchOffset++;
				pBranchCell2 = &pBranchPage->pBranch[branchOffset2 & 0xFFFF];
			}

			//fill branch
			for (int j = 0; i < count; i++, j++)
			{
				pBranchCell2->Offsets[j] = offsets[i];
				pBranchCell2->Values[j] = values[i];
			}

			//set content cell
			prevSP.pBlockCell->Type = MIN_BRANCH_TYPE1 + count - 1;
			prevSP.pBlockCell->Offset = branchOffset1;
			prevSP.pBlockCell->ValueOrOffset = branchOffset2;

			//release block
			releaseBlockCells(sp.pBlockCell - sp.BlockSubOffset, sp.StartBlockOffset);
		}
	}

	return false;
}

bool HArray::dismantlingContentCells(SegmentPath* path, int32& currPathLen)
{
	//release contents
	uint32 contentOffsetLen = 0;

	while (true)
	{
		//just remove
		*path[currPathLen].pContentCellType = 0;
		*path[currPathLen].pContentCellValue = 0;

		if (currPathLen > 0)
		{
			if (path[currPathLen - 1].Type == CURRENT_VALUE_SEGMENT_TYPE)
			{
				currPathLen--;
				contentOffsetLen++;
			}
			else
			{
				releaseContentCells(path[currPathLen].pContentCellValue,
									path[currPathLen].ContentOffset,
									contentOffsetLen);

				return false;
			}
		}
		else
		{
			releaseContentCells(path[currPathLen].pContentCellValue,
								path[currPathLen].ContentOffset,
								contentOffsetLen);

			return true;
		}
	}

	return true;
}

bool HArray::dismantling(SegmentPath* path, uint32 pathLen)
{
	//DISMANTLING =============================================================================================
	for (int32 currPathLen = pathLen - 1; currPathLen >= 0; currPathLen--)
	{
		SegmentPath& sp = path[currPathLen];

		switch (sp.Type)
		{
		case CURRENT_VALUE_SEGMENT_TYPE:
		{
			if(dismantlingContentCells(path, currPathLen))
			{
				return true;
			}
			else
			{
				break;
			}
		}

		case BRANCH_SEGMENT_TYPE:
		{
			if (*sp.pContentCellType > MIN_BRANCH_TYPE1) //not last way
			{
				//remove item
				uint32 lastIndex = *sp.pContentCellType - 1;

				sp.pBranchCell1->Values[sp.BranchIndex] = sp.pBranchCell1->Values[lastIndex];
				sp.pBranchCell1->Offsets[sp.BranchIndex] = sp.pBranchCell1->Offsets[lastIndex];

				sp.pBranchCell1->Values[lastIndex] = 0;
				sp.pBranchCell1->Offsets[lastIndex] = 0;

				(*sp.pContentCellType)--;

				if (*sp.pContentCellType == MIN_BRANCH_TYPE1) //last way is original ?
				{
					if (sp.pBranchCell1->Offsets[0] == sp.ContentOffset + 1)
					{
						*sp.pContentCellType = CURRENT_VALUE_TYPE;
						*sp.pContentCellValue = sp.pBranchCell1->Values[0];

						releaseBranchCell(sp.pBranchCell1, sp.BranchOffset1);
					}
				}

				return false;
			}
			else //last way
			{
				//release branch
				releaseBranchCell(sp.pBranchCell1, sp.BranchOffset1);

				if(dismantlingContentCells(path, currPathLen))
				{
					return true;
				}
				else
				{
					break;
				}
			}
		}

		case BLOCK_VALUE_SEGMENT_TYPE:
		{
			sp.pBlockCell->Type = 0;
			sp.pBlockCell->Offset = 0;
			sp.pBlockCell->ValueOrOffset = 0;

			if (tryReleaseBlock(path, pathLen, currPathLen))
			{
				break;
			}
			else
			{
				return false;
			}
		}

		case BLOCK_BRANCH1_SEGMENT_TYPE:
		{
			//remove item
			if (sp.pBlockCell->Type <= MAX_BRANCH_TYPE1) //in one branch
			{
				uint32 lastIndex = sp.pBlockCell->Type - 1;

				sp.pBranchCell1->Values[sp.BranchIndex] = sp.pBranchCell1->Values[lastIndex];
				sp.pBranchCell1->Offsets[sp.BranchIndex] = sp.pBranchCell1->Offsets[lastIndex];

				sp.pBranchCell1->Values[lastIndex] = 0;
				sp.pBranchCell1->Offsets[lastIndex] = 0;

				sp.pBlockCell->Type--;

				if (sp.pBlockCell->Type == MIN_BRANCH_TYPE1) //only one item, inject to block
				{
					sp.pBlockCell->Type = CURRENT_VALUE_TYPE;
					sp.pBlockCell->ValueOrOffset = sp.pBranchCell1->Values[0];
					sp.pBlockCell->Offset = sp.pBranchCell1->Offsets[0];

					releaseBranchCell(sp.pBranchCell1, sp.BranchOffset1);
				}

				//check block ?
			}
			else //in two branches
			{
				uint32 lastIndex = sp.pBlockCell->Type - MAX_BRANCH_TYPE1 - 1;

				sp.pBranchCell1->Values[sp.BranchIndex] = sp.pBranchCell2->Values[lastIndex];
				sp.pBranchCell1->Offsets[sp.BranchIndex] = sp.pBranchCell2->Offsets[lastIndex];

				sp.pBranchCell2->Values[lastIndex] = 0;
				sp.pBranchCell2->Offsets[lastIndex] = 0;

				sp.pBlockCell->Type--;

				//if it was last item in branch2 then release branch2
				if (sp.pBlockCell->Type < MIN_BRANCH_TYPE2) //not last item in branch2
				{
					releaseBranchCell(sp.pBranchCell2, sp.BranchOffset2);
				}
			}

			return false;
		}

		case BLOCK_BRANCH2_SEGMENT_TYPE:
		{
			if (sp.pBlockCell->Type > MIN_BRANCH_TYPE2) //not last item
			{
				uint32 lastIndex = sp.pBlockCell->Type - MAX_BRANCH_TYPE1 - 1;

				sp.pBranchCell2->Values[sp.BranchIndex] = sp.pBranchCell2->Values[lastIndex];
				sp.pBranchCell2->Offsets[sp.BranchIndex] = sp.pBranchCell2->Offsets[lastIndex];

				sp.pBranchCell2->Values[lastIndex] = 0;
				sp.pBranchCell2->Offsets[lastIndex] = 0;
			}
			else //last item
			{
				releaseBranchCell(sp.pBranchCell2, sp.BranchOffset2);
			}

			sp.pBlockCell->Type--;

			return false;
		}

		case BLOCK_OFFSET_SEGMENT_TYPE:
		{
			if (tryReleaseBlock(path, pathLen, currPathLen))
			{
				break;
			}
			else
			{
				return false;
			}
		}

		case VAR_SHUNT_SEGMENT_TYPE:
		{
			if (sp.pVarCell->ValueContCellType)
			{
				//delete continue way, inject value
				*sp.pContentCellType = sp.pVarCell->ValueContCellType;
				*sp.pContentCellValue = sp.pVarCell->ValueContCellValue;

				releaseVarCell(sp.pVarCell, sp.VarOffset);

				return false;
			}
			else
			{
				//delete continue way, value was deleted earlier
				releaseContentCells(sp.pContentCellValue, sp.ContentOffset, 0);

				releaseVarCell(sp.pVarCell, sp.VarOffset);

				break;
			}
		}

		case VAR_VALUE_SEGMENT_TYPE:
		{
			if (sp.pVarCell->ContCellType == CONTINUE_VAR_TYPE)
			{
				if (sp.pVarCell->ValueContCellType)
				{
					sp.pVarCell->ValueContCellType = 0;
				}

				return false;
			}
			else
			{
				//was only shunted value, remove var cell
				*sp.pContentCellType = sp.pVarCell->ContCellType;
				*sp.pContentCellValue = sp.pVarCell->ContCellValue;

				releaseVarCell(sp.pVarCell, sp.VarOffset);

				return false;
			}
		}

		default: //fail state
			break;
		}
	}

	return true;
}

bool HArray::delValueByKey(uint32* key,
						   uint32 keyLen)
{
	//EXTRACT PATH =============================================================================================

	SegmentPath path[MAX_KEY_SEGMENTS];
	int32 pathLen = 0;

	keyLen >>= 2; //in 4 bytes

	uint32 headerOffset;

	if (!normalizeFunc)
	{
		headerOffset = key[0] >> HeaderBits;
	}
	else
	{
		headerOffset = (*normalizeFunc)(key);
	}

	HeaderCell& headerCell = pHeader[headerOffset];

	if (headerCell.Type)
	{
		uint32 contentOffset = headerCell.Offset;

		uint32 keyOffset = 0;

		bool isVarContCell;

	NEXT_KEY_PART:

		if (pathLen == 28)
		{
			pathLen = 28;
		}

		ContentPage* pContentPage = pContentPages[contentOffset >> 16];
		ushort16 contentIndex = contentOffset & 0xFFFF;

		uint32& contentCellValue = pContentPage->pContent[contentIndex];
		uchar8& contentCellType = pContentPage->pType[contentIndex]; //move to type part

		if (contentCellType >= ONLY_CONTENT_TYPE) //ONLY CONTENT =========================================================================================
		{
			uint32 restKeyLen = keyLen - keyOffset;

			if (restKeyLen != (contentCellType - ONLY_CONTENT_TYPE))
			{
				return false;
			}

			uint32 origKeyOffset = keyOffset;

			uint32 origContentIndex = contentIndex;

			for (; keyOffset < keyLen; contentIndex++, keyOffset++)
			{
				if (pContentPage->pContent[contentIndex] != key[keyOffset])
					return false;
			}

			//remove
			releaseContentCells(&contentCellValue, contentOffset, restKeyLen);

			for (; origKeyOffset <= keyLen; origContentIndex++, origKeyOffset++)
			{
				pContentPage->pType[origContentIndex] = 0;
			}

			goto DISMANTLING;
		}

		uint32& keyValue = key[keyOffset];
		uint32* pContentCellValueOrOffset = &contentCellValue;

		isVarContCell = false;

		if (contentCellType == VAR_TYPE) //VAR =====================================================================
		{
			VarPage* pVarPage = pVarPages[(*pContentCellValueOrOffset) >> 16];
			VarCell& varCell = pVarPage->pVar[(*pContentCellValueOrOffset) & 0xFFFF];

			if (keyOffset < keyLen)
			{
				//save path
				SegmentPath& sp = path[pathLen++];
				sp.Type = VAR_SHUNT_SEGMENT_TYPE;
				sp.pContentCellType = &contentCellType;
				sp.pContentCellValue = &contentCellValue;
				sp.ContentOffset = contentOffset;
				sp.pVarCell = &varCell;
				sp.VarOffset = *pContentCellValueOrOffset;

				contentCellType = varCell.ContCellType; //read from var cell

				isVarContCell = true;

				if (contentCellType == CONTINUE_VAR_TYPE) //CONTINUE VAR =====================================================================
				{
					contentOffset = varCell.ContCellValue;

					goto NEXT_KEY_PART;
				}
				else
				{
					pContentCellValueOrOffset = &varCell.ContCellValue;
				}
			}
			else
			{
				if(varCell.ValueContCellType)
				{
					//save path
					SegmentPath& sp = path[pathLen++];
					sp.Type = VAR_VALUE_SEGMENT_TYPE;
					sp.pContentCellType = &contentCellType;
					sp.pContentCellValue = &contentCellValue;
					sp.ContentOffset = contentOffset;
					sp.pVarCell = &varCell;
					sp.VarOffset = *pContentCellValueOrOffset;

					//return &varCell.ValueContentCell.Value;
					goto DISMANTLING;
				}
				else
				{
					return false;
				}
			}
		}
		else if (keyOffset == keyLen)
		{
			if (contentCellType == VALUE_TYPE)
			{
				//save path
				SegmentPath& sp = path[pathLen++];
				sp.Type = CURRENT_VALUE_SEGMENT_TYPE;
				sp.pContentCellType = &contentCellType;
				sp.pContentCellValue = &contentCellValue;
				sp.ContentOffset = contentOffset;

				goto DISMANTLING;
			}
			else
			{
				return false;
			}
		}

		if (contentCellType <= MAX_BRANCH_TYPE1) //BRANCH =====================================================================
		{
			BranchPage* pBranchPage = pBranchPages[(*pContentCellValueOrOffset) >> 16];
			BranchCell& branchCell = pBranchPage->pBranch[(*pContentCellValueOrOffset) & 0xFFFF];

			//try find value in the list
			uint32* values = branchCell.Values;

			for (uint32 i = 0; i < contentCellType; i++)
			{
				if (values[i] == keyValue)
				{
					//save path
					SegmentPath& sp = path[pathLen++];
					sp.Type = BRANCH_SEGMENT_TYPE;
					sp.pContentCellType = &contentCellType;
					sp.pContentCellValue = &contentCellValue;
					sp.pBranchCell1 = &branchCell;
					sp.BranchOffset1 = *pContentCellValueOrOffset;
					sp.BranchIndex = i;
					sp.ContentOffset = contentOffset;

					contentOffset = branchCell.Offsets[i];
					keyOffset++;

					goto NEXT_KEY_PART;
				}
			}

			return 0;
		}
		else if (contentCellType == VALUE_TYPE)
		{
			if (keyOffset == keyLen)
			{
				//save path
				SegmentPath& sp = path[pathLen++];
				sp.Type = CURRENT_VALUE_SEGMENT_TYPE;
				sp.pContentCellType = &contentCellType;
				sp.pContentCellValue = &contentCellValue;
				sp.ContentOffset = contentOffset;

				goto DISMANTLING;
			}
			else
			{
				return false;
			}
		}
		else if (contentCellType <= MAX_BLOCK_TYPE) //VALUE IN BLOCK ===================================================================
		{
			uchar8 idxKeyValue = (contentCellType - MIN_BLOCK_TYPE) * BLOCK_ENGINE_STEP;

			uint32 startOffset = *pContentCellValueOrOffset;

		NEXT_BLOCK:
			uint32 subOffset = ((keyValue << idxKeyValue) >> BLOCK_ENGINE_SHIFT);
			uint32 blockOffset = startOffset + subOffset;

			BlockPage* pBlockPage = pBlockPages[blockOffset >> 16];
			BlockCell& blockCell = pBlockPage->pBlock[blockOffset & 0xFFFF];

			uchar8& blockCellType = blockCell.Type;

			if (blockCellType == EMPTY_TYPE)
			{
				return false;
			}
			else if (blockCellType == CURRENT_VALUE_TYPE) //current value
			{
				if (blockCell.ValueOrOffset == keyValue) //value is exists
				{
					//save path
					SegmentPath& sp = path[pathLen++];
					sp.Type = BLOCK_VALUE_SEGMENT_TYPE;
					sp.pContentCellType = &contentCellType;
					sp.pContentCellValue = &contentCellValue;
					sp.pBlockCell = &blockCell;
					sp.StartBlockOffset = startOffset;
					sp.ContentOffset = contentOffset;
					sp.BlockSubOffset = subOffset;
					//sp.pBranchCell = 0;
					//sp.Index = i;

					contentOffset = blockCell.Offset;
					keyOffset++;

					goto NEXT_KEY_PART;
				}
				else
				{
					return false;
				}
			}
			else if (blockCellType <= MAX_BRANCH_TYPE1) //branch cell
			{
				BranchCell& branchCell1 = pBranchPages[blockCell.Offset >> 16]->pBranch[blockCell.Offset & 0xFFFF];

				//try find value in the list
				for (uint32 i = 0; i < blockCellType; i++)
				{
					if (branchCell1.Values[i] == keyValue)
					{
						//save path
						SegmentPath& sp = path[pathLen++];
						sp.Type = BLOCK_BRANCH1_SEGMENT_TYPE;
						sp.pContentCellType = &contentCellType;
						sp.pContentCellValue = &contentCellValue;
						sp.pBlockCell = &blockCell;
						sp.StartBlockOffset = startOffset;
						sp.pBranchCell1 = &branchCell1;
						sp.BranchOffset1 = blockCell.Offset;
						sp.BranchIndex = i;
						sp.BlockSubOffset = subOffset;

						contentOffset = branchCell1.Offsets[i];
						keyOffset++;

						goto NEXT_KEY_PART;
					}
				}

				return 0;
			}
			else if (blockCellType <= MAX_BRANCH_TYPE2) //branch cell
			{
				BranchCell& branchCell1 = pBranchPages[blockCell.Offset >> 16]->pBranch[blockCell.Offset & 0xFFFF];
				BranchCell& branchCell2 = pBranchPages[blockCell.ValueOrOffset >> 16]->pBranch[blockCell.ValueOrOffset & 0xFFFF];

				//try find value in the list
				for (uint32 i = 0; i < BRANCH_ENGINE_SIZE; i++)
				{
					if (branchCell1.Values[i] == keyValue)
					{
						//save path
						SegmentPath& sp = path[pathLen++];
						sp.Type = BLOCK_BRANCH1_SEGMENT_TYPE;
						sp.pContentCellType = &contentCellType;
						sp.pContentCellValue = &contentCellValue;
						sp.pBlockCell = &blockCell;
						sp.StartBlockOffset = startOffset;
						sp.pBranchCell1 = &branchCell1;
						sp.BranchOffset1 = blockCell.Offset;
						sp.pBranchCell2 = &branchCell2;
						sp.BranchOffset2 = blockCell.ValueOrOffset;
						sp.BranchIndex = i;
						sp.BlockSubOffset = subOffset;

						contentOffset = branchCell1.Offsets[i];
						keyOffset++;

						goto NEXT_KEY_PART;
					}
				}

				//try find value in the list
				uint32 countValues = blockCellType - MAX_BRANCH_TYPE1;

				for (uint32 i = 0; i < countValues; i++)
				{
					if (branchCell2.Values[i] == keyValue)
					{
						//save path
						SegmentPath& sp = path[pathLen++];
						sp.Type = BLOCK_BRANCH2_SEGMENT_TYPE;
						sp.pContentCellType = &contentCellType;
						sp.pContentCellValue = &contentCellValue;
						sp.pBlockCell = &blockCell;
						sp.StartBlockOffset = startOffset;
						sp.pBranchCell1 = &branchCell1;
						sp.BranchOffset1 = blockCell.Offset;
						sp.pBranchCell2 = &branchCell2;
						sp.BranchOffset2 = blockCell.ValueOrOffset;
						sp.BranchIndex = i;
						sp.BlockSubOffset = subOffset;

						contentOffset = branchCell2.Offsets[i];
						keyOffset++;

						goto NEXT_KEY_PART;
					}
				}

				return 0;
			}
			else if (blockCell.Type <= MAX_BLOCK_TYPE)
			{
				//save path
				SegmentPath& sp = path[pathLen++];
				sp.Type = BLOCK_OFFSET_SEGMENT_TYPE;
				sp.pBlockCell = &blockCell;
				sp.pContentCellType = &contentCellType;
				sp.pContentCellValue = &contentCellValue;
				sp.StartBlockOffset = startOffset;
				sp.ContentOffset = contentOffset;
				sp.BlockSubOffset = subOffset;

				//go to block
				idxKeyValue = (blockCell.Type - MIN_BLOCK_TYPE) * BLOCK_ENGINE_STEP;
				startOffset = blockCell.Offset;
								
				goto NEXT_BLOCK;
			}
			else
			{
				return false;
			}
		}
		else if (contentCellType == CURRENT_VALUE_TYPE) //PART OF KEY =========================================================================
		{
			if (*pContentCellValueOrOffset == keyValue)
			{
				if (!isVarContCell) //do not add content cell from VarCell
				{
					//save path
					SegmentPath& sp = path[pathLen++];
					sp.Type = CURRENT_VALUE_SEGMENT_TYPE;
					sp.pContentCellType = &contentCellType;
					sp.pContentCellValue = &contentCellValue;
					sp.ContentOffset = contentOffset;
				}

				contentOffset++;
				keyOffset++;

				goto NEXT_KEY_PART;
			}
			else
			{
				return false;
			}
		}
	}

	//DISMANTLING ====================================================================================
DISMANTLING:

	if (dismantling(path, pathLen))
	{
		headerCell.Type = EMPTY_TYPE;
		headerCell.Offset = 0;

		amountFreeSlotsBeforeHeaderResize++;
	}
	
	//SHRINK =============================================================================================
	if (autoShrinkOnPercents)
	{
		ulong64 stepReleaseMemory = getUsedMemory() * autoShrinkOnPercents / 100;

		//MAX_COUNT_RELEASED_CONTENT_CELLS
		if ((countReleasedContentCells - notMovedContentCellsAfterLastShrink) * sizeof(uint32) > stepReleaseMemory &&
			(countReleasedContentCells - notMovedContentCellsAfterLastShrink) * sizeof(uint32) > MIN_COUNT_RELEASED_CONTENT_CELLS * sizeof(uint32))
		{
			shrinkContentPages();
		}
		else if (countReleasedBranchCells * sizeof(BranchCell) > stepReleaseMemory &&
			countReleasedBranchCells * sizeof(BranchCell) > MIN_COUNT_RELEASED_BRANCH_CELLS * sizeof(BranchCell))
		{
			shrinkBranchPages();
		}
		else if (countReleasedBlockCells * sizeof(BlockCell) > stepReleaseMemory &&
			countReleasedBlockCells * sizeof(BlockCell) > MIN_COUNT_RELEASED_BLOCK_CELLS * sizeof(BlockCell))
		{
			shrinkBlockPages();
		}
		else if (countReleasedVarCells * sizeof(VarCell) > stepReleaseMemory &&
				 countReleasedVarCells * sizeof(VarCell) > MIN_COUNT_RELEASED_VAR_CELLS * sizeof(VarCell))
		{
			shrinkVarPages();
		}
	}

	return true;
}
