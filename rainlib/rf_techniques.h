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
#ifndef RF_TECHNIQUES_H
#define RF_TECHNIQUES_H

#include "rain.h"
#include "regionrecorder.h"
#include "regionmanager.h"
#include "trace_io.h"

namespace rf_technique {

  class NET : public rain::RF_Technique
  {
  public:
    
  NET() : region_id(1), number_of_instructions(0), paused(false), just_quit(true), stop_rf(false) {}
    
    void process(unsigned long long cur_addr, char cur_opcode[16], char unsigned cur_length, 
		 unsigned long long nxt_addr, char nxt_opcode[16], char unsigned nxt_length);
    
    void finish();
    
  protected:
    
    bool isBranch(unsigned char, unsigned long long, unsigned long long);
    bool switchState(unsigned long long);
    bool branch(unsigned long long, unsigned long long);
    void buffering(unsigned long long);
    void record();
    void next(unsigned long long);
    
#define System 0	//Linux/Windows/0

    RegionRecorder rr;
    RegionManager  manager;

    int region_id;      //< Region unique identifier
    
#define MAX_INST	1000 //Número máximo de instruções na região.
    
    rain::Region::Node* buffer[MAX_INST]; //buffer de instruções que serão gravadas na região.

    unsigned number_of_instructions; //número de instruções no buffer de instruções.
    bool verified;
    bool paused;
    bool just_quit;
    bool stop_rf;   //< Stop region formation.
  };


}; // namespace rf_technique


#endif  // RF_TECHNIQUES_H
