/***************************************************************************
 *   Copyright (C) 2013 by:                                                *
 *   Edson Borin (edson@ic.unicamp.br), and                                *
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

#ifdef DEBUG
#include <assert.h>
#define DBG_ASSERT(cond) assert(cond)
#else
#define DBG_ASSERT(cond)
#endif

Region::Node::Node() : region(NULL), freq_counter(0), 
		       out_edges(NULL), in_edges(NULL) 
{}

Region::Node::Node(unsigned long long a) : region(NULL), freq_counter(0), 
					   out_edges(NULL), in_edges(NULL),
					   addr(a)
{}
	
Region::Node::~Node()
{
  EdgeListItem* it = out_edges;
  while (it) {
    EdgeListItem* next = it->next;
    delete it;
    it = next;
  }

  it = in_edges;
  while (it) {
    EdgeListItem* next = it->next;
    delete it;
    it = next;
  }
}

Region::Edge* Region::Node::findOutEdge(unsigned long long next_ip)
{
  if (!out_edges) return NULL;
  if (out_edges->edge->tgt->getAddress() == next_ip) return out_edges->edge;

  EdgeListItem* prev = NULL;
  for (EdgeListItem* it = out_edges; it; it = it->next) {
    if (it->edge->tgt->getAddress() == next_ip) {

      if (prev != NULL) {
	// Move item to beginning of list...
	prev->next = it->next;
	it->next = out_edges;
	out_edges = it;
      }

      return it->edge;
    }
    prev = it;
  }
  return NULL;
}

Region::Edge* Region::Node::findOutEdge(Region::Node* target) const
{
  for (EdgeListItem* it = out_edges; it; it = it->next) {
    if (it->edge->tgt == target) {
      return it->edge;
    }
  }
  return NULL;
}

Region::Edge* Region::Node::findInEdge(Region::Node* source) const
{
  for (EdgeListItem* it = in_edges; it; it = it->next) {
    if (it->edge->src == source) {
      return it->edge;
    }
  }
  return NULL;
}

void Region::Node::insertOutEdge(Region::Edge* ed, Region::Node* target)
{
#ifdef DEBUG
  assert(ed->src == this);
  assert(ed->tgt == target);
  // Check for edge duplication
  assert(findOutEdge(target) == NULL);
#endif 

  Region::EdgeListItem* it = new Region::EdgeListItem();
  it->edge = ed;
  it->next = out_edges;
  out_edges = it;
}

void Region::Node::insertInEdge(Region::Edge* ed, Region::Node* source)
{
#ifdef DEBUG
  assert(ed->tgt == this);
  assert(ed->src == source);
  // Check for edge duplication
  assert(findInEdge(source) == NULL);
#endif 

  Region::EdgeListItem* it = new Region::EdgeListItem();
  it->edge = ed;
  it->next = in_edges;
  in_edges = it;
}

Region::~Region()
{
  // Remove all edges.
  list<Edge*>::iterator eit = region_inner_edges.begin();
  for (; eit!= region_inner_edges.end(); eit++) {
    Edge* e = *eit;
    delete e;
  }
  region_inner_edges.clear();

  // Remove all nodes.
  for(list<Node*>::iterator nit = nodes.begin(); nit != nodes.end(); nit++) {
    Region::Node* node = (*nit);
    delete node;
  }
  nodes.clear();

  // Remove pointer to entry and exit nodes
  entry_nodes.clear();
  exit_nodes.clear();
}

void Region::insertRegOutEdge(Edge* ed)
{
#ifdef DEBUG
  assert(ed->src->region == this);
  assert(ed->tgt->region != this);
#endif 

  EdgeListItem* it = new EdgeListItem();
  it->edge = ed;
  it->next = reg_out_edges;
  reg_out_edges = it;
}

void Region::insertRegInEdge(Region::Edge* ed)
{
#ifdef DEBUG
  assert(ed->src->region != this);
  assert(ed->tgt->region == this);
#endif 

  EdgeListItem* it = new EdgeListItem();
  it->edge = ed;
  it->next = reg_in_edges;
  reg_in_edges = it;
}

Region::Edge* Region::createInnerRegionEdge(Region::Node* src, Region::Node* tgt)
{
  Edge* ed = new Edge(src,tgt);
  src->insertOutEdge(ed,tgt);
  tgt->insertInEdge(ed,src);
  region_inner_edges.push_back(ed);
  return ed;
}

/** Return the edge that will be followed if the next_ip is executed. */
Region::Edge* RAIn::queryNext(unsigned long long next_ip)
{
  if (cur_node == nte) {
    // NTE node (treated separatedely for efficiency reasons)
    map<unsigned long long, Region::Edge*>::iterator it = 
      nte_out_edges_map.find(next_ip);
    if (it != nte_out_edges_map.end())
      return it->second; // Return existing NTE out edge
    else {
      // Search for region entries, if there is none, return nte_loop_edge
      if (region_entry_nodes.find(next_ip) != region_entry_nodes.end()) {
	// edge representing transition from nte to region missing.
	return NULL;
      }
      else {
	// transition from nte to nte
	return nte_loop_edge;
      }
    }
  }
  else {
    // Region node
    return cur_node->findOutEdge(next_ip);
  }
}

/** Add edge supposing next_ip is the next instruction to be executed. */
Region::Edge* RAIn::addNext(unsigned long long next_ip)
{
  // Sanity checking
  DBG_ASSERT(queryNext(next_ip) == NULL);
  Region::Edge* edg = NULL;
  Region::Node* next_node = NULL;

  // Search for region entries.
  map<unsigned long long, Region::Node*>::iterator it = 
    region_entry_nodes.find(next_ip);
  if(it != region_entry_nodes.end())
    next_node = it->second;
  else
    next_node = NULL;

  if (cur_node == nte) {
    if (next_node == NULL) {
      cerr << "Error, addNext should be not called when "
	   << "queryNext returns a valid edge." << endl;
      exit (1);
    }
    else {
      // Add edge from nte to region entry.
      edg = createInterRegionEdge(nte, next_node);
    }
  }
  else {
    // current node belongs to region.
    if (next_node == NULL) {
      // add edge from cur_node to nte (back to emulation manager) 
      edg = createInterRegionEdge(cur_node, nte);
    }
    else {
      // add edge from cur_node to region_entry (link regions)
      edg = createInterRegionEdge(cur_node, next_node);
    }
  }

  DBG_ASSERT(edg != NULL);
  return edg;
}

/** Execute the edge (update the current node and related statistics). */
void RAIn::executeEdge(Region::Edge* edge)
{
  DBG_ASSERT(edge->src == cur_node);
  cur_node = edge->tgt;
  edge->freq_counter++;
  cur_node->freq_counter++;
}

Region* RAIn::createRegion()
{
  Region* region; 
  region = new Region();
  region->id = region_id_generator++;
  regions[region->id] = region;
  return region;
}

void RAIn::setEntry(Region::Node* node)
{ 
  region_entry_nodes[node->getAddress()] = node;
  node->region->setEntryNode(node);
}

void RAIn::setExit(Region::Node* node)
{
  node->region->setExitNode(node);
}

/** Create an edge to connect two nodes from different regions. */
Region::Edge* RAIn::createInterRegionEdge(Region::Node* src, Region::Node* tgt)
{
  DBG_ASSERT(src->region != tgt->region);
  Region::Edge* ed = new Region::Edge(src,tgt);
  src->insertOutEdge(ed,tgt);
  tgt->insertInEdge(ed,src);

  if (src->region)
    src->region->insertRegOutEdge(ed);
  if (tgt->region)
    tgt->region->insertRegInEdge(ed);

  inter_region_edges.push_back(ed);
  if (src == nte)
    nte_out_edges_map[tgt->getAddress()] = ed;
  return ed;
}

void RAIn::printRAInStats(ostream& stats_f)
{
  // Print statistics for regions
  printRegionsStats(stats_f);
  // Print statistics for NTE
  stats_f << "0" << "," << nte->freq_counter << endl;
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

//TODO: Fix this
// First updated at printTRegion
// Then, updated and used at printTable  
map<struct hash, struct table> results;

unsigned long long Region::allNodesFreq() const
{
  unsigned long long c = 0;
  for (list<Node*>::const_iterator it = nodes.begin(); it!=nodes.end(); it++) {
    c += (*it)->freq_counter;
  }
  return c;
}

unsigned long long Region::entryNodesFreq() const
{
  unsigned long long c = 0;
  for (list<Node*>::const_iterator it = entry_nodes.begin(); it!=entry_nodes.end(); it++) {
    c += (*it)->freq_counter;
  }
  return c;
}

unsigned long long Region::exitNodesFreq() const
{
  unsigned long long c = 0;
  for (list<Node*>::const_iterator it = exit_nodes.begin(); it!=exit_nodes.end(); it++) {
    c += (*it)->freq_counter;
  }
  return c;
}

unsigned long long Region::externalEntries() const
{
  unsigned long long c = 0;
  for (EdgeListItem* it = reg_in_edges; it; it=it->next) {
    Edge* e = it->edge;
    c += e->freq_counter;
  }
  return c;
}

void RAIn::printRegionsStats(ostream& stats_f)
{
  map<unsigned, Region*>::iterator rit;

  stats_f << "Region," 
	  << "# Nodes," 
	  << "All Nodes Freq," 
	  << "Entry Nodes Freq," 
	  << "Exit Nodes Freq," 
	  << "External Entries,"
	  << endl;
  for (rit = regions.begin(); rit != regions.end(); rit++)
  {
    Region* r = rit->second;

    stats_f << r->id << ","
	    << r->nodes.size() << "," 
	    << r->allNodesFreq() << "," 
	    << r->entryNodesFreq() << "," 
	    << r->exitNodesFreq() << "," 
	    << r->externalEntries() << ","
	    << endl;
  }
}

void RAIn::printOverallStats(ostream& stats_f)
{
  unsigned long long total_stat_reg_size = 0;
  unsigned long long total_reg_entries = 0;
  unsigned long long total_reg_freq = 0;
  unsigned long long nte_freq = nte->freq_counter;
  map<unsigned long long,unsigned> unique_instrs;

  map<unsigned, Region*>::iterator rit;
  for (rit = regions.begin(); rit != regions.end(); rit++)
  {
    Region* r = rit->second;
    total_stat_reg_size += r->nodes.size();
    total_reg_entries += r->externalEntries();
    total_reg_freq += r->allNodesFreq();
    list<Region::Node*>::const_iterator it;
    for (it=r->nodes.begin(); it!=r->nodes.end(); it++) {
      unique_instrs[(*it)->getAddress()] = 1;
    }
  }
  unsigned long long total_unique_instrs = unique_instrs.size();

  stats_f << "# of instructions on regions" << "," << total_stat_reg_size  << endl;
  stats_f << "Region entry frequency count" << "," << total_reg_entries << endl;
  stats_f << "Freq. of instructions on regions" << "," << total_reg_freq << endl;
  stats_f << "Freq. of instructions out of regions" << "," << nte_freq << endl;
  stats_f << "# of unique included on regions" << "," << total_unique_instrs << endl;
  stats_f << "Average dyn. size" << "," << 
    (double) total_reg_freq / (double) total_reg_entries << endl;
}

void RAIn::printRegionDOT(Region* region, ostream& reg)
{
#if 0
  Region::Node* node_pointer;
  unsigned long long head;
  unsigned count = 0, enters = 0, nodes = 0;
  list<Region::Node* > exits = region->getExitNodes();
  list<Region::Node* >::iterator ex;
  node_pointer = region->getEntry();
  Region::Node::List* list = node_pointer->out_edges;
  head = node_pointer->getAddress();

  reg << "digraph G{\n";

  do{
    Region::Edge* edge = list->edge;
    
    if(edge == NULL)
      break;

    reg << "_" << edge->source() << "->_" << edge->target() << ";" << endl;
    reg << "count: " << edge->freq_counter << endl;
    node_pointer = list->edge->tgt;
    list = node_pointer->out_edges;
  }
  while(index == node_pointer->region && head != node_pointer->getAddress());

  reg << "}\n";

  //TODO: FIXME: print it to another file???
  if (0 /* print node */ ) {
    map<unsigned long long,Region::Node* >::iterator it, end;
    end = region->nodes.end();
    for(it = region->nodes.begin(); it != end; it++) {
      nodes++;
      //node << (*it2).first << " " << (*it2).second->getCounter() << endl;
      //if(1==index) cout<<"print " << (*it2).first << "\n";
      count += (*it).second->freq_counter;
    }
  }
#endif
}


void RAIn::printRegionsDOT(string& dotf_prefix)
{
  map<unsigned, Region*>::iterator it,end;
  it = regions.begin();
  end = regions.end();
  for(unsigned c=1; it != end; it++, c++) {
    ostringstream fn;
    fn << dotf_prefix << std::setfill ('0') << std::setw (4) << c << ".dot"; 
    ofstream dotf(fn.str().c_str());
    printRegionDOT(it->second, dotf);
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
	  map<unsigned, Region*>::iterator it = regions.find(region_id);
	  printValidate(it->second, it->first, i, graph);
	}
	
	graph << "}";
	
	graph.close();
}

void RAIn::printValidate(Region* region, int index,int i,ostream& graph)
{
  #if 0
	string color[] = {"red", "blue", "cyan", "green",  "yellow", "purple", "orange", "brown", "yellowgreen", "darkorange", "pink", "indigo", "magenta", "grey", "orangered", "red4", "blue4", "green4", "yellow4", "chocolate", "antiquewhite"};

	Region::Node* node_pointer;
	unsigned long long head;
cout << "Region " << index << " i " << i << "\n";
	map<unsigned long long,Region::Node* >::iterator it2, end;
	end = region->nodes.end();
	it2 = region->nodes.begin();
	for(; it2 != end; it2++) {
	  graph << "state_" << setbase(16) << (*it2).first
		<< " [ style = \"filled\" penwidth = 1 fillcolor = \"" << color[i] 
		<< "\" fontname = \"Courier New\" shape = \"Mrecord\" label = \"" << (*it2).first
		<< "\"];\n";
	}

	node_pointer = region->getEntry();
	Region::Node::List* list = node_pointer->out_edges;
	head = node_pointer->getAddress();
	
	do{
		Region::Edge* edge = list->edge;

		unsigned long long from = edge->source();
		unsigned long long to = edge->target();
		graph << "state_" << from << "->" << "state_" << to << ";\n";
	
		node_pointer = list->edge->tgt;
		list = node_pointer->out_edges;
//cout << from << "\n";
	}while(index == node_pointer->region && head != node_pointer->getAddress());
#endif
}

