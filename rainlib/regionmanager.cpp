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
#include "regionmanager.h"

RegionManager manager;

#define THRESHOLD		50 //Limite para come�ar a gravar.

/*
 * --- Implementa o m�todo NET de cria��o de traces. ---
 */
void RegionManager::net(unsigned long long new_ip, RegionRecorder* rr)
{
	unsigned long long cur_ip_lin = cur_ip;
	unsigned long long new_ip_lin = new_ip;
	
	bool back_edge = new_ip_lin < cur_ip_lin;

	//if (back_edge || just_quit) {
	if (back_edge) {
		if (counter.find(new_ip_lin) != counter.end()) 
			counter[new_ip_lin]++;
		else {
			counter[new_ip_lin] = 0;
			already_reg[new_ip_lin] = false;
		}
	}

	//Caso o contador tenha passado o limite estipulado e ainda n�o foi criado uma regi�o come�ando por este endere�o.
	if (counter.find(new_ip_lin) != counter.end() && !already_reg[new_ip_lin] && counter[new_ip_lin] >= THRESHOLD){
		//come�a a gravar
		rr->start(new_ip);
		already_reg[new_ip_lin] = true;
	}
}

/*
 * Marca��o do NET para instru��es logo ap�s sa�da de regi�es j� formadas. Essas instru��es tamb�m s�o candidatas a virarem regi�es.
 */
void RegionManager::netCount(unsigned long long new_ip, RegionRecorder* rr)
{

	if (counter.find(new_ip) != counter.end()) 
		counter[new_ip]++;
	else {
		counter[new_ip] = 0;
		already_reg[new_ip] = false;
	}
	
	//Caso o contador tenha passado o limite estipulado e ainda n�o foi criado uma regi�o come�ando por este endere�o.
	if (counter.find(new_ip) != counter.end() && !already_reg[new_ip] && counter[new_ip] >= THRESHOLD){
		//come�a a gravar
		rr->start(new_ip);
		already_reg[new_ip] = true;
	}
}

/*
 * --- Implementa o m�todo Trace Tree de cria��o de traces. ---
 */
void RegionManager::tt(unsigned long long new_ip, RegionRecorder* rr)
{
	unsigned long long cur_ip_lin = cur_ip;
	unsigned long long new_ip_lin = new_ip;
	
	bool back_edge = new_ip_lin < cur_ip_lin;

	//if (back_edge || just_quit) {
	if (back_edge) {
		if (counter.find(new_ip_lin) != counter.end()) 
			counter[new_ip_lin]++;
		else {
			counter[new_ip_lin] = 0;
			already_reg[new_ip_lin] = false;
		}
	}

	//Caso o contador tenha passado o limite estipulado e ainda n�o foi criado uma regi�o come�ando por este endere�o.
	if (counter.find(new_ip_lin) != counter.end() && !already_reg[new_ip_lin] && counter[new_ip_lin] >= THRESHOLD){
		//come�a a gravar
		rr->start(new_ip);
		already_reg[new_ip_lin] = true;
	}
}
