#include "stdafx.h"
#include "HArrayVarRAM.h"

void HArrayVarRAM::delValueByKey(uint32* key,
								 uint32 keyLen,
								 uint32 value,
								 uint32 index)
{
	keyLen >>= 2; //in 4 bytes
	uint32 maxSafeShort = MAX_SAFE_SHORT - keyLen;

	uint32 contentOffset = pHeader[key[0]>>HeaderBits];

	if(contentOffset)
	{
		uint32 keyOffset = 0;

NEXT_KEY_PART:
		ContentPage* pContentPage = pContentPages[contentOffset>>16];
		ushort16 contentIndex = contentOffset&0xFFFF;

		uchar8 contentCellType = pContentPage->pContent[contentIndex].Type; //move to type part

		if(contentCellType >= ONLY_CONTENT_TYPE) //ONLY CONTENT =========================================================================================
		{
			if((keyLen - keyOffset) != (contentCellType - ONLY_CONTENT_TYPE))
			{
				return;
			}

			if(contentIndex < maxSafeShort) //content in one page
			{
				for(; keyOffset < keyLen; contentIndex++, keyOffset++)
				{
					if(pContentPage->pContent[contentIndex].Value != key[keyOffset])
						return;
				}

				//delete value
				if(pContentPage->pContent[contentIndex].Type == VALUE_TYPE)
				{
					pContentPage->pContent[contentIndex].Value = 0;
				}

				return;
			}
			else //content in two pages
			{
				for(; keyOffset < keyLen; contentOffset++, keyOffset++)
				{
					if(pContentPages[contentOffset>>16]->pContent[contentOffset&0xFFFF].Value != key[keyOffset])
						return;
				}

				ContentCell& contentCell = pContentPages[contentOffset>>16]->pContent[contentOffset&0xFFFF];

				//delete value
				if(contentCell.Type == VALUE_TYPE)
				{
					contentCell.Value = 0;
				}

				return;
			}
		}

		uint32& keyValue = key[keyOffset];
		uint32& contentCellValueOrOffset = pContentPage->pContent[contentIndex].Value;

		if(contentCellType == VAR_TYPE) //VAR =====================================================================
		{
			VarPage* pVarPage = pVarPages[contentCellValueOrOffset >> 16];
			VarCell& varCell = pVarPage->pVar[contentCellValueOrOffset & 0xFFFF];

			if(keyOffset < keyLen)
			{
				contentCellType = varCell.ContCell.Type; //read from var cell
				contentCellValueOrOffset = varCell.ContCell.Value;

				if(contentCellType == CONTINUE_VAR_TYPE) //CONTINUE VAR =====================================================================
				{
					contentOffset = contentCellValueOrOffset;

					goto NEXT_KEY_PART;
				}
			}
			else
			{
				if(varCell.ValueContentCell.Type == VALUE_TYPE)
				{
					varCell.ValueContentCell.Value = 0;
				}

				return;
			}
		}
		else if(keyOffset == keyLen)
		{
			if(contentCellType == VALUE_TYPE)
			{
				contentCellValueOrOffset = 0;
			}

			return;
		}

		if(contentCellType <= MAX_BRANCH_TYPE1) //BRANCH =====================================================================
		{
			BranchPage* pBranchPage = pBranchPages[contentCellValueOrOffset >> 16];
			BranchCell& branchCell = pBranchPage->pBranch[contentCellValueOrOffset & 0xFFFF];

			//try find value in the list
			uint32* values = branchCell.Values;

			for(uint32 i=0; i<contentCellType; i++)
			{
				if(values[i] == keyValue)
				{
					contentOffset = branchCell.Offsets[i];
					keyOffset++;

					goto NEXT_KEY_PART;
				}
			}

			return;
		}
		else if(contentCellType == VALUE_TYPE)
		{
			contentCellValueOrOffset = 0;

			return;
		}
		else if(contentCellType <= MAX_BLOCK_TYPE) //VALUE IN BLOCK ===================================================================
		{
			uchar8 idxKeyValue = (contentCellType - MIN_BLOCK_TYPE) * BLOCK_ENGINE_STEP;

			uint32 startOffset = contentCellValueOrOffset;

	NEXT_BLOCK:
			uint32 subOffset = ((keyValue << idxKeyValue) >> BLOCK_ENGINE_SHIFT);
			uint32 blockOffset = startOffset + subOffset;

			BlockPage* pBlockPage = pBlockPages[blockOffset >> 16];
			BlockCell& blockCell = pBlockPage->pBlock[blockOffset & 0xFFFF];

			uchar8& blockCellType = blockCell.Type;

			if(blockCellType == EMPTY_TYPE)
			{
				return;
			}
			else if(blockCellType == CURRENT_VALUE_TYPE) //current value
			{
				if(blockCell.ValueOrOffset == keyValue) //value is exists
				{
					contentOffset = blockCell.Offset;
					keyOffset++;

					goto NEXT_KEY_PART;
				}
				else
				{
					return;
				}
			}
			else if(blockCellType <= MAX_BRANCH_TYPE1) //branch cell
			{
				BranchPage* pBranchPage = pBranchPages[blockCell.Offset >> 16];
				BranchCell& branchCell1 = pBranchPage->pBranch[blockCell.Offset & 0xFFFF];

				//try find value in the list
				for(uint32 i=0; i<blockCellType; i++)
				{
					if(branchCell1.Values[i] == keyValue)
					{
						contentOffset = branchCell1.Offsets[i];
						keyOffset++;

						goto NEXT_KEY_PART;
					}
				}

				return;
			}
			else if(blockCellType <= MAX_BRANCH_TYPE2) //branch cell
			{
				BranchPage* pBranchPage1 = pBranchPages[blockCell.Offset >> 16];
				BranchCell branchCell1 = pBranchPage1->pBranch[blockCell.Offset & 0xFFFF];

				//try find value in the list
				for(uint32 i=0; i < BRANCH_ENGINE_SIZE; i++)
				{
					if(branchCell1.Values[i] == keyValue)
					{
						contentOffset = branchCell1.Offsets[i];
						keyOffset++;

						goto NEXT_KEY_PART;
					}
				}

				BranchPage* pBranchPage2 = pBranchPages[blockCell.ValueOrOffset >> 16];
				BranchCell branchCell2 = pBranchPage2->pBranch[blockCell.ValueOrOffset & 0xFFFF];

				//try find value in the list
				uint32 countValues = blockCellType - MAX_BRANCH_TYPE1;

				for(uint32 i=0; i<countValues; i++)
				{
					if(branchCell2.Values[i] == keyValue)
					{
						contentOffset = branchCell2.Offsets[i];
						keyOffset++;

						goto NEXT_KEY_PART;
					}
				}

				return;
			}
			else if(blockCell.Type <= MAX_BLOCK_TYPE)
			{
				//go to block
				idxKeyValue = (blockCell.Type - MIN_BLOCK_TYPE) * BLOCK_ENGINE_STEP;
				startOffset = blockCell.Offset;

				goto NEXT_BLOCK;
			}
			else
			{
				return;
			}
		}
		else if(contentCellType == CURRENT_VALUE_TYPE) //PART OF KEY =========================================================================
		{
			if(contentCellValueOrOffset == keyValue)
			{
				contentOffset++;
				keyOffset++;

				goto NEXT_KEY_PART;
			}
			else
			{
				return;
			}
		}
	}

	return;
}
