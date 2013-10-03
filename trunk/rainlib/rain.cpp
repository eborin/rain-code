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
#include "rain.h"
#include <iostream>
#include <fstream>
#include <sstream>  // stringstream
#include <iomanip>  // setbase
#include <stdlib.h> // exit

using namespace std;
using namespace rain;

Region::Node::Node()
{
	region = -1;
	freq_counter = 0;
	edges = new List();
	edges->next = NULL;
	edges->edge = NULL;
	back_edges = new List();
	back_edges->next = NULL;
	back_edges->edge = NULL;
	call = false;
}

Region::Node::Node(unsigned long long addr)
{
	region = -1;
	this->addr = addr;
	freq_counter = 0;
	edges = new List();
	edges->next = NULL;
	edges->edge = NULL;
	back_edges = new List();
	back_edges->next = NULL;
	back_edges->edge = NULL;
	call = false;
}
	
Region::Node::~Node()
{
	Region::Edge* edge;
	List* list = edges;
	List* next;
	while(NULL != list){
		next = list->next;
		edge = list->edge;
		if(NULL != edge)
			delete edge;
		delete list;
		list = next;
	}
	
	list = back_edges;
	while(NULL != list){
		next = list->next;
		delete list;
		list = next;
	}
}

void Region::Node::insertEdge(Region::Edge* ed, Region::Node* target)
{
	if(NULL == edges->edge){
		edges->edge = ed;
	}
	else{
		Region::Node::List* list = edges;
		if(list->edge->tgt->getAddress() == target->getAddress()){
//cout <<"WOA duplicacao de aresta.\n";
			return;
		}
		while(NULL != list->next){
			list = list->next;
			if(list->edge->tgt->getAddress() == target->getAddress()){
//cout <<"WOA duplicacao de aresta.\n";
				return;
			}
		}
		list->next = new Region::Node::List();
		list = list->next;
		list->next = NULL;
		list->edge = ed;
	}
}

void Region::Node::insertBackEdge(Region::Edge* ed, Region::Node* source)
{
	unsigned long long addr = source->getAddress();
	if(NULL == back_edges->edge){
		back_edges->edge = ed;
		back_edges->source = addr;
	}
	else{
		Region::Node::List* list = back_edges;
		if(list->source == addr){
//cout <<"WOA duplicacao de aresta.\n";
			return;
		}
		while(NULL != list->next){
			list = list->next;
			if(list->source == addr){
//cout <<"WOA duplicacao de aresta.\n";
				return;
			}
		}
		list->next = new Region::Node::List();
		list = list->next;
		list->next = NULL;
		list->edge = ed;
		list->source = addr;
	}
}

bool Region::Node::insertUpdateEdge(Region::Edge* ed, Region::Node* target)
{ 
  if(edges->edge == NULL) {
    edges->edge = ed;
    return true;
  }
  else
  {
    Region::Node::List* list = edges;
    if(list->edge->tgt->getAddress() == target->getAddress()){
      //cout <<"WOA duplicacao de aresta.\n";
      list->edge->count++;
      return false;
    }
    while(NULL != list->next){
      list = list->next;
      if(list->edge->tgt->getAddress() == target->getAddress()){
	//cout <<"WOA duplicacao de aresta.\n";
	list->edge->count++;
	return false;
      }
    }
    list->next = new Region::Node::List();
    list = list->next;
    list->next = NULL;
    list->edge = ed;
    
    return true;
    //if(3222488466 == target->getAddress()) cout << "Criado NTE " << gcounter << " contador \n";
  }
}

Region::~Region()
{
  map<unsigned long long,Region::Node* >::iterator entry = nodes.begin();
  Region::Node* prev, *node;
  for(;entry != nodes.end(); entry++){
    node = (*entry).second;
    Region::Node::List* list = node->back_edges;
    if(NULL == list->edge)
      list = NULL;
    while(NULL != list){
      unsigned addr = node->getAddress();
      prev = list->edge->src;
      Region::Node::List* prev_list = prev->edges;
      Region::Node::List* prev_prev = prev_list;
      while(NULL != prev_list->next){
	prev_list = prev_list->next;
	if(prev_list->edge->tgt->getAddress() == addr){
	  prev_prev->next = prev_list->next;
	  Region::Edge* edge = prev_list->edge;
	  delete edge;
	  delete prev_list;
	  break;
	}
	prev_prev = prev_list;
      }
      
      list = list->next;
	  }
  }
  map<unsigned long long,Region::Node* >::iterator it, end;
  end = nodes.end();
  for(it = nodes.begin(); it != end; it++) {
    node = (*it).second;
    delete node;
  }
  
  nodes.clear();
  entrys.clear();
  exits.clear();
}

/*
 * --- Atualiza a região. ---
 */
bool Region::update(unsigned long long cur_ip)
{
  map<unsigned long long,Region::Node*>::iterator it = nodes.find(cur_ip);
  if(nodes.end() != it){
    counter++;
    Region::Node* node = (*it).second;
    node->access();
    return true; //o nó estava na região.
  }
  else
    return false;
}

void Region::createEdge(Region::Node* src, Region::Node* tgt)
{
	Region::Edge* ed = new Region::Edge(src,tgt);
	src->insertEdge(ed,tgt);
	tgt->insertBackEdge(ed,tgt);
}

map<unsigned long long,Region::Node* >::iterator Region::getIterator(bool which)
{
	if(which) return nodes.begin();
	else return nodes.end();
}

map<unsigned long long,Region::Node* >::iterator Region::find(unsigned long long addr)
{
	return nodes.find(addr);
}

void Region::copyRegion(Region* region, map<unsigned long long,Region::Node* >::iterator it)
{
	//list<Region::Node *>::iterator entry, end;
	Region::Node* node = (*it).second;
	for(; it != nodes.end(); it++){
		region->includeNode((*it).second,(*it).first);
		nodes.erase(it);
	}
	/*
	entry = entrys.begin();
	while((*entry).second->getAddress() < node->getAddress()){
		entry++;
	}
	if((*entry).second->getAddress() == node->getAddress())
		region->entrys.splice(region->entrys.begin(),entrys,entry,entrys.end());
	else{
		region->setEntry(node);
		region->entrys.splice(++(region->entrys.begin()),entrys,entry,entrys.end());
	}*/
}

//#define Linux 3000000000
//#define Windows 18000000000000000000

bool atualizado = false;
bool bfs = false;

/*
 * Verifica se o proximo nó é o início de uma região.
 */
void RAIn::verify(unsigned long long next_ip)
{
  map<int, Region*>::iterator it,end;
  int how_many = 0;

  map<unsigned long long, int>::iterator index;
  index = entrys.find(next_ip);
  if(index != entrys.end()){
    cur_reg = (*index).second;
    it = regions.find(cur_reg);
    next_node = (*it).second->getEntry();
    how_many++;
    cur_reg = (*it).first;
  }
  
  if(0 == how_many){
    in_region = false;
    in_nte = true;
  }
  else{
    //Caso tenha encontrado o início de uma região, termina a região atual e
    //continua a atualização do rain.
    stop = true;
  }

}

/*
 * --- Atualiza o RAIn. ---
 */
void RAIn::update(unsigned long long addr)
{
  map<int, Region*>::iterator it,end;
  unsigned found = false;
  unsigned last_reg = cur_reg; //last region visited.
  
  map<unsigned long long,Region::Edge*>::iterator edgs;

  if(cur_node != NULL){

    if(cur_node->region != -1) {
      // Currently inside a region
      cur_reg = cur_node->region;
      it = regions.find(cur_reg);
      bool is_there = (*it).second->update(addr);
      if(is_there) {
	found = true;
	//if(7 == cur_reg){ cout << "addr " << addr << " counter " << gcounter << "\n";
	//if(3076853334 == addr) cout << "Update! " << cur_reg << "\n";
      } 
      else {
	cout << "Erro2!\n";
      }
    }
  }
  
  /** Se não pertencia a nenhuma região, então a execução estava em NTE. **/
  if(!found){
    nte->access();	
  }

  updateEdges(found,last_reg);
  
  cur_ip = addr;
}

/*
 * Devolve o próximo nó.
 */
Region::Node* RAIn::next(unsigned long long next_ip)
{
	//Se o nó estava em NTE, procura-o entre as entradas de região.
	if(in_nte){
//if(3222682958 == next_ip) cout << "tava em in nte.\n";
		map<unsigned long long, int>::iterator index;
		index = entrys.find(next_ip);
		if(index != entrys.end()){
			int reg = (*index).second;
			map<int, Region*>::iterator regs = regions.find(reg);
			map<unsigned long long,Region::Node* >::iterator it = (*regs).second->find(next_ip);
			return (*it).second;
		}
		else
			return NULL;
	}
	
	//Procura o próximo nó entre as arestas do último nó.
	if(NULL != cur_node){
//if(3222682958 == next_ip) cout << "Vixe!\n";
		Region::Node::List* list = cur_node->edges;
		if(NULL != list->edge){
			while(NULL != list->next){
				if(next_ip == list->edge->tgt->getAddress()){
					list->edge->count++;
					atualizado = true;
					return list->edge->tgt;
				}
				list = list->next;
			}
			
			//Verifica a última aresta.
			if(next_ip == list->edge->tgt->getAddress()){
				list->edge->count++;
				atualizado = true;
				return list->edge->tgt;
			}
		}
		//Caso o nó não esteja entre as arestas procura-o entre as entradas de regiões e se o encontrar adiciona uma nova aresta entre o último nó e este.
		map<unsigned long long, int>::iterator index;
		index = entrys.find(next_ip);
		if(index != entrys.end()){
			int reg = (*index).second;
			map<int, Region*>::iterator regs = regions.find(reg);
			map<unsigned long long,Region::Node* >::iterator it = (*regs).second->find(next_ip);
			next_node = (*it).second;
			createEdge(cur_node,next_node);
			setExit(regions[cur_node->region],cur_node);
			return next_node;
		}
		else
			return NULL;
	}

	return NULL;

}

// Atualiza o índice da região atualmente a ser gravada. (-1 para o default)
void RAIn::currentRegion(int index)
{
	if(-1 == index)
		this->cur_index = this->count;
	else
		this->cur_index = index;
}

void RAIn::updateEdges(bool found,int last_reg)
{
	if(!found){
		//Caso estava atualizando uma região e saiu, o último nó era uma saída.
		if(in_region && !bfs)
		{
if(-1==last_node->region) cout << "ERRRO AQUI!\n";
			setExit(regions[last_node->region],last_node);
			just_quit = true;
		}
		in_region = false;
	
		//Cria arestas para NTE caso venha de alguma região.
		if(!in_nte){
			if(NULL != last_node){
				createEdge(last_node,nte);
			}
		}	
		in_nte = true;
	}
	//Caso pertencia a alguma região.
	else{
		in_region = true;
					
		//Se trocou de região cria uma aresta entre elas ou atualiza.
		if(last_reg != cur_reg){
			if(NULL != cur_node){
				if(NULL != last_node){
// cout << "last node region " << last_node->region << " " << last_node->getAddress() << " " << cur_reg << " last reg " << last_reg << "\n";
					setExit(regions[last_node->region],last_node);
					if(!atualizado)
						createEdge(last_node,cur_node);
				}
			}
		}
	
		//Cria arestas de NTE caso venha de lá.
		if(in_nte){
			if(NULL == last_node){ 
				createOrUpdateEdge(nte,cur_node);
			}
		}	
		in_nte = false;
	}
}

Region* RAIn::createRegion()
{
	Region* region; 
	region = new Region();
	
	return region;
}

Region::Node* RAIn::createNode(Region* region, unsigned long long addr)
{
	if(region->find(addr) == region->getIterator(false)){
		Region::Node* node = new Region::Node(addr);
		node->region = this->cur_index;
		region->includeNode(node,addr);
		return node;
	}

	return NULL;
}

bool RAIn::insertNode(Region* region, Region::Node* node)
{
	unsigned long long addr = node->getAddress();
	map<unsigned long long,Region::Node*>::iterator it;
	it = region->find(addr);
	if(it == region->getIterator(false)){
		node->region = this->cur_index;
		region->includeNode(node,addr);
		return true;
	}
	
	return false;
}

void RAIn::insertRegion(Region* region)
{
	regions[this->count] = region;
	unsigned long long  entry = region->getEntry()->getAddress();
	entrys[entry] = this->count;
	this->count++;
	this->cur_index = this->count;
}

void RAIn::setEntry(Region* region, Region::Node* node)
{
	region->setEntryNode(node);
}

void RAIn::setExit(Region* region, Region::Node* node)
{
	region->setExitNode(node);
}

void RAIn::createEdge(Region* region, Region::Node* src, Region::Node* tgt)
{
	region->createEdge(src,tgt);
}

//Cria ou atualiza uma aresta.
void RAIn::createEdge(Region::Node* src, Region::Node* tgt)
{
	Region::Edge* ed = new Region::Edge(src,tgt);
	src->insertEdge(ed,tgt);
	tgt->insertBackEdge(ed,tgt);
}

void RAIn::createOrUpdateEdge(Region::Node* src, Region::Node* tgt)
{
	Region::Edge* ed = new Region::Edge(src,tgt);
	bool inserted = src->insertUpdateEdge(ed,tgt);
	if(inserted)
		tgt->insertBackEdge(ed,tgt);
}

struct table
{
	unsigned number_of_nodes;
	unsigned frequency;
	unsigned number_of_entries;
	unsigned extern_entries;
};

struct hash
{
	unsigned long long address;
	int region;
};

bool operator<(const hash &a, const hash &b)
{
	return (a.address*a.region) < (b.address*b.region);
}

map<struct hash, struct table> results;

void RAIn::printRegionDOT(Region* region, int index, ostream& reg)
{
  region->cleanExits();
  Region::Node* node_pointer;
  bool do_break = false;
  unsigned long long head;
  struct hash function;
  map<struct hash, struct table>::iterator tab;
  unsigned count = 0, enters = 0, nodes = 0;
  list<Region::Node* > exits = region->getExitNodes();
  list<Region::Node* >::iterator ex;
  node_pointer = region->getEntry();
  Region::Node::List* list = node_pointer->edges;
  head = node_pointer->getAddress();
  reg << "digraph G{\n";
  do{
    Region::Edge* edge = list->edge;
    if(edge == NULL){ 
      do_break = true; break;}
    unsigned long long from = edge->source();
    unsigned long long to = edge->target();
    reg << "_" << from << "->_" << to << ";\n";
    reg << "count: " << edge->count << "\n";
    
    node_pointer = list->edge->tgt;
    list = node_pointer->edges;
    //cout << from << "\n";
  }while(index == node_pointer->region && head != node_pointer->getAddress());

  reg << "}\n";
  
  for(ex = exits.begin(); exits.end() != ex; ex++){
    list = (*ex)->edges;
    while(NULL != list){
      if(NULL == list->edge)
	break;
      unsigned long long address = list->edge->tgt->getAddress();
      int region = list->edge->tgt->region;
      //if(135382607 == address) cout << "Aqui c: " << list->edge->count << " veio de " << list->edge->source()  << " " << index << " \n";
      //if(80 == index) cout << " alvo " << address << " region " << region << " count " << list->edge->count << " veio de " << list->edge->source()  << " " << index << " \n";
      function.address = address;
      function.region = region;
      tab = results.find(function);
      if(results.end() != tab)
	results[function].number_of_entries += list->edge->count;
      else
	results[function].number_of_entries = list->edge->count;
      
      if(region != index){
	if(results.end() != tab)
	  results[function].extern_entries += list->edge->count;
	else
	  results[function].extern_entries = list->edge->count;
      }
      
      list = list->next;
    }
  }
  
  map<unsigned long long,Region::Node* >::iterator it2, end;
  end = region->getIterator(false);
  it2 = region->getIterator(true);
  for(; it2 != end; it2++) {
    nodes++;
    //TODO: FIXME: print it to another file???
    //node << (*it2).first << " " << (*it2).second->getCounter() << endl;

    //if(1==index) cout<<"print " << (*it2).first << "\n";
    count += (*it2).second->getCounter();
  }
  function.address = head;
  function.region = index;
  results[function].frequency = count;
  results[function].number_of_nodes = nodes;
  
  //table << nodes << " & " << count << " & " << enters << "\n";
}

void RAIn::printRAInStats(ostream& stats_f)
{
  // Print statistics for regions
  printRegionsStats(stats_f);
  // Print statistics for NTE
  stats_f << "0" << "," << nte->getCounter() << endl;
}

void RAIn::printRegionsStats(ostream& stats_f)
{
  map<int, Region*>::iterator it,end;
  unsigned long long address;
  int region;
  struct hash function;
  map<struct hash, struct table>::iterator tab;
  
  Region::Node::List* list = nte->edges;
  while(NULL != list){
    address = list->edge->tgt->getAddress();
    region = list->edge->tgt->region;
    function.address = address;
    function.region = region;
    //if(135382607 == address) cout << "Aqui c: " << list->edge->count << " veio de NTE\n";
    tab = results.find(function);
    if(results.end() != tab){
      results[function].number_of_entries += list->edge->count;
      results[function].extern_entries += list->edge->count;
    }
    else{
      results[function].number_of_entries = list->edge->count;
      results[function].extern_entries = list->edge->count;
    }
    list = list->next;
    //if(135382607 == address) cout << "Function: " << results[function].number_of_entries << " veio de NTE " << address*region << "\n";
  }
  
  unsigned entries;
  end = regions.end();
  it = regions.begin();
  stats_f << "Region,Static Size,Frequency,# Entries,External Entries" << endl;
  for(; it != end; it++) {
    region = (*it).first;
    address = (*it).second->getEntry()->getAddress();
    function.address = address;
    function.region = region;
    tab = results.find(function);
    //if(results.end() == tab) cout << "Pau na " << (*it).first << " end " << address << "\n";
    entries = results[function].number_of_entries;
    //if(3222373754 == address) cout << "Function: " << results[function].number_of_entries << " veio de NTE " << address*region << " " << region << " " << address << "\n";
    stats_f << region << ","
	    << results[function].number_of_nodes << "," 
	    << results[function].frequency << "," 
	    << entries << "," 
	    << results[function].extern_entries << endl;
  }
}

void RAIn::printRegionsDOT(string& dotf_prefix)
{
  map<int, Region*>::iterator it,end;
  it = regions.begin();
  end = regions.end();
  for(unsigned c=1; it != end; it++, c++) {
    ostringstream fn;
    fn << dotf_prefix << std::setfill ('0') << std::setw (4) << c << ".dot"; 
    ofstream dotf(fn.str().c_str());
    printRegionDOT((*it).second, (*it).first, dotf);
    //cout << (*it).second->getExitNodes().front()->getAddress() << "\n";
    dotf.close();
  }
}

void RAIn::validateRegions(set<unsigned>& regions_to_validate)
{
	ofstream graph;
	graph.open("results/graph.cfg");
	graph << setbase(16) << "digraph G{\n";

	graph << "state_0 [ style = \"filled\" penwidth = 3 fillcolor = \"plum4\" fontname = \"Courier New\" shape = \"Mrecord\" label = \"NTE\"];\n";

	set<unsigned>::const_iterator rit;
	int i;
	for (i=0, rit = regions_to_validate.begin(); rit != regions_to_validate.end(); rit++, i++) {
	  unsigned region_id = *rit;
	  map<int, Region*>::iterator it = regions.find(region_id);
	  printValidate(it->second, it->first, i, graph);
	}
	
	graph << "}";
	
	graph.close();
}

void RAIn::printValidate(Region* region, int index,int i,ostream& graph)
{
	string color[] = {"red", "blue", "cyan", "green",  "yellow", "purple", "orange", "brown", "yellowgreen", "darkorange", "pink", "indigo", "magenta", "grey", "orangered", "red4", "blue4", "green4", "yellow4", "chocolate", "antiquewhite"};

	Region::Node* node_pointer;
	unsigned long long head;
cout << "Region " << index << " i " << i << "\n";
	map<unsigned long long,Region::Node* >::iterator it2, end;
	end = region->getIterator(false);
	it2 = region->getIterator(true);
	for(; it2 != end; it2++) {
	  graph << "state_" << setbase(16) << (*it2).first
		<< " [ style = \"filled\" penwidth = 1 fillcolor = \"" << color[i] 
		<< "\" fontname = \"Courier New\" shape = \"Mrecord\" label = \"" << (*it2).first
		<< "\"];\n";
	}


	node_pointer = region->getEntry();
	Region::Node::List* list = node_pointer->edges;
	head = node_pointer->getAddress();
	
	do{
		Region::Edge* edge = list->edge;

		unsigned long long from = edge->source();
		unsigned long long to = edge->target();
		graph << "state_" << from << "->" << "state_" << to << ";\n";
	
		node_pointer = list->edge->tgt;
		list = node_pointer->edges;
//cout << from << "\n";
	}while(index == node_pointer->region && head != node_pointer->getAddress());
}

