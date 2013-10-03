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
#include "rf_techniques.h"
#include <iostream>

using namespace rf_technique;
using namespace rain;

unsigned long long gcounter;

//Verifica se é um branch
bool NET::isBranch(unsigned char len, unsigned long long addr, unsigned long long next)
{
  int long long result = next - addr;
  if(result > len || result < 0)
    return true;
  else
    return false;
}

//Verifica se a instrução mudou de estado user/system.
bool NET::switchState(unsigned long long addr)
{
  bool old_state = rain.system;
  if(addr < System)
    rain.system = false;
  else
    rain.system = true;
  
  if(old_state != rain.system)
    return true;
  else
    return false;
}

/*
 * No caso de um branch tomado invoca o método de gravação NET.
 */
bool NET::branch(unsigned long long addr, unsigned long long target)
{
  if(!rr.isRecording()){
    manager.net(target,&rr);
    verified = true;
    return false;
  }
  else{
    bool back_edge = target < addr;
    //Caso o branch seja uma back edge é o fim do trace.
    if (back_edge){
      buffering(addr);
      record();
      just_quit = true;
    }
    return rr.isRecording();
  }
}

/*
 * Grava o nó no buffer da região.
 */
void NET::buffering(unsigned long long addr)
{
  Region::Node* node;
  
  if(NULL != rain.cur_node){
    if(addr == rain.cur_node->getAddress())
      node = rain.cur_node;
    else
      node = new Region::Node(addr);
  }
  else
    node = new Region::Node(addr);
  
  buffer[number_of_instructions] = node;
  number_of_instructions++;
  
  if (number_of_instructions >= MAX_INST){
    std::cerr << "Fechou pelo limite. \n";
    //Fecha e grava a região no RAIn.
    record();
  }
}

/*
 * Realiza a gravação da região.
 */
void NET::record()
{
  unsigned i = 0;
  Region::Node* node;
  
  node = buffer[i];
  rr.record(node, rain);
  i++;
  for(;i < number_of_instructions; i++){
    node = buffer[i];
    if(rain.insertNode(rr.cur_reg,node)){
      rain.createEdge(rr.cur_reg,rr.cur_node,node);
      rr.cur_node = node;
    }
  }
  
  rain.insertRegion(rr.end());
  number_of_instructions = 0;
  
  region_id++;
}

/*
 * Processa a instrução.
 */
void NET::process(unsigned long long cur_addr, char cur_opcode[16], char unsigned cur_length, 
		  unsigned long long nxt_addr, char nxt_opcode[16], char unsigned nxt_length)
{
  //Armazena o endereço da instrução para o monitoramento.
  manager.cur_ip = cur_addr;
  rr.cur_ip = cur_addr;
  gcounter++;
  rain.next_node = NULL;
  verified = false;
  bool recording;
  //if(135382607==cur_addr) cout << "olha ai " << cur_addr << " regiao " << region_id << "\n";
  
  if(switchState(cur_addr)){
    if(paused)
      paused = false;
    else if(rr.isRecording() && 0 != number_of_instructions)
      paused = true;
  }
  
  if(paused){
    rain.nte->access();
    next(nxt_addr);
    return;
  }
  
  if(rr.isRecording())
    rain.verify(nxt_addr);
  
  rain.update(cur_addr);
  
  recording = rr.isRecording();
  
  if(isBranch(cur_length, cur_addr, nxt_addr))
    recording = branch(cur_addr,nxt_addr);
  
  //Se o recorder estiver ativo: cria um nó e o coloca no buffer da região.
  if(recording)		
    buffering(cur_addr);
  
  if(stop_rf && rr.isRecording())
    record();
  stop_rf = false;
  
  rain.last_node = rain.cur_node;
  
  if(just_quit){
    if(!verified)
      manager.netCount(nxt_addr,&rr);
    if(rr.isRecording()){
      if(NULL != rain.next_node){
	if(NULL!=rain.cur_node)
	  rain.createEdge(rain.cur_node,rain.next_node);
	else if(rain.in_nte){
	  rain.createOrUpdateEdge(rain.nte,rain.next_node);
	}
      }
    }	
    just_quit = false;
    
  }
  
  rain.last_node = rain.cur_node;
  
  next(nxt_addr);
}

void NET::next(unsigned long long target)
{

  if(NULL != rain.next_node){
    //if(gcounter == 99999997) cout << "rain.next_node " << rain.next_node->getAddress() << "\n";
    /*if(NULL != rain.cur_node){
      unsigned long long address = rain.cur_node->getAddress();
      
      if(target == 1)
      cout << "next " << address << "->" << rain.next_node->getAddress() << " " << gcounter << "\n";}*/
    //if(3222682958==target)cout << "direto\n";
    rain.cur_node = rain.next_node;
    //cout << "proximo! " << rain.next_node->getAddress() << "\n";
    
    /*if(NULL != rain.cur_node){
      
      unsigned long long address = rain.cur_node->getAddress();
      if(target == 1)
      cout << "anterior: " << addr << " curnode: " << address << " o " << gcounter << " target " << target << "\n";}*/
    
  }
  else
  {
    //if(gcounter == 99999997) cout << addr << " " << target << " in nte " << rain.in_nte << " " << rr.isRecording() << " not rain.next_node\n";
    /*if(NULL != rain.cur_node){
      unsigned long long address = rain.cur_node->getAddress();
      if(target == 1)
      
      cout << "rain " << address << " " << rain.cur_node->region << " " << gcounter << " next " << target << "\n";}*/
    //if(3222682958==target)cout << "vai no next\n";
    rain.cur_node = rain.next(target);


    /*if(NULL != rain.cur_node){
      
      unsigned long long address = rain.cur_node->getAddress();
      if(target == 1)
      cout << address << "->" << gcounter << " target " << target << "\n";}*/
  }

  //if(gcounter >= 99999997) cout << gcounter << "\n";
  //if(gcounter == 99999997) cout << "addr " << addr << "->" << target << " " << rain.count << "\n";  
}

/*
 * Encerra a execução.
 */
void NET::finish()
{
  //Caso o recorder esteja ativo o trace ainda não foi encerrado.
  if(rr.isRecording()){
    cout << "Fechou forçado. \n";
    //Fecha(forçado) e grava a região no RAIn.
    record();
  }
}
