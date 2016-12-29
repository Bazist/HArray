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
#include "HArray.h"

/*

Main strategy
1. OnlyContentType => zero content, zero header
2. CurrentValue => ???
3. Branch => Decrease branch => Remove branch + Current Value in content
4. Branch1 in Block => Decrease branch => Remove branch + inject in block
5. Branch2 in Block => Decrease branch => Remove branch + inject in block
6. In Low Block less than 8 values => Remove block => create branches in block above level
7. In Top Block less than 4 values => Create branch
8. VarBranch shunt (remove value) => Remove VarBranch + inject ContentCell
9. VarBranch shunt (remove transit key) => Remove VarBranch + inject ContentCell.Value
10. VarBranch continue (remove value) => Remove VarBranch + inject ContentCell
//11. VarBranch continue (remove value) => Remove VarBranch + inject ContentCell

Pools
1. Table of content holes
2. List of free branches
3. List of free blocks
4. List of varbranches

Stats

*/


bool HArray::delValueByKey(uint32* key,
								 uint32 keyLen)
{
	uint32* pValue = getValueByKey(key, keyLen);

	if (pValue)
	{
		*pValue = 0;

		return true;
	}
	else
	{
		return false;
	}
}
