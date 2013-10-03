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
#ifndef REGIONRECORDER_H
#define REGIONRECORDER_H

#include "rain.h"

using namespace std;

/*
 * -- Classe que grava as regiões do RAIn. --
 */
class RegionRecorder
{
 public:
  rain::Region*       cur_reg; 	// current Region.
  rain::Region::Node* cur_node; 	// current node.
  unsigned long long cur_ip; //current instruction pointer.

 RegionRecorder() : recording(false) 
    {}
  
  bool isRecording() {return recording;}
  unsigned long long getHead() {return head;}
  void start(unsigned long long);
  void record(rain::Region::Node*, rain::RAIn&);
  void abort();
  rain::Region* end();

 private:
  unsigned long long head;
  bool recording;
};

#endif // REGIONRECORDER_H
