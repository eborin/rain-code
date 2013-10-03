/***************************************************************************
 *   Copyright (C) 2013 by:                                                *
 *   - Edson Borin (edson@ic.unicamp.br), and                              *
 *   - Raphael Zinsly (raphael.zinsly@gmail.com)                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "regionrecorder.h"

using namespace std;
using namespace rain;

/*
 * --- Inicia a gravação de uma região. ---
 */
void RegionRecorder::start(unsigned long long addr)
{
  head = addr;
  recording = true;
}

void RegionRecorder::record(Region::Node* node, RAIn& rain)
{
  this->cur_node = node;
  cur_reg = rain.createRegion();
  rain.insertNode(cur_reg,this->cur_node);
  cur_reg->setEntryNode(this->cur_node);	
}

/*
 * --- Encerra a gravação de uma região. ---
 */
Region* RegionRecorder::end()
{
  cur_reg->setExitNode(cur_node);
  Region* region = cur_reg;
  
  //delete cur_reg;
  cur_ip = 0;
  recording = false;
	
  return region;
}

void RegionRecorder::abort()
{
  cur_ip = 0;
  recording = false;
}
