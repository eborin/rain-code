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
#ifndef REGIONMANAGER_H
#define REGIONMANAGER_H

#include "regionrecorder.h"

using namespace std;

class RegionManager
{
  map<unsigned long long,unsigned> counter;
  map<unsigned long long,bool> already_reg; // already a TRegion.

public:
  unsigned long long cur_ip; //current instruction pointer.

  RegionManager() : cur_ip(0) {}
  
  void abort(unsigned head)
  {
    already_reg[head] = false;
  }
  
  void net(unsigned long long,RegionRecorder*); 	//Next-Executing Tail.
  void netCount(unsigned long long,RegionRecorder*); //Parte do algoritmo NET.
  void mret2();			//Most Recently Executed Tail 2 (NET em duas fases).
  void lei();				//Last-Executed Iteration.
  void tt(unsigned long long, RegionRecorder*); 	//NET para formação de Trace Trees.
  void treegion();		//Método para formação de Treegions.
};

#endif // REGIONMANAGER_H
