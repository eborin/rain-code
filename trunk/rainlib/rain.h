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
#ifndef RAIN_H
#define RAIN_H

//#include <iomanip>
#include <ostream>
#include <map>
#include <list>
#include <set>

using namespace std;

namespace rain {

  /** 
   *  @brief A Region object represents a region of code.
   */
  class Region {

  public:
    class Edge;
    
    /** 
     *  @brief A Region Node object represents one instruction inside the region
     */
    class Node
    {
    public:
      
      Node();
      Node(unsigned long long);
      Node(unsigned long long, string);
      ~Node();
      
      void access() {freq_counter++;}
      unsigned long long getAddress() {return addr;}
      unsigned getCounter() {return freq_counter;}
      string getOpName() {return op_name;}
      
      void insertEdge(Edge*, Node*);
      void insertBackEdge(Edge*, Node*);
      bool insertUpdateEdge(Edge*, Node*);
      
    public:
      struct List
      {
	Edge* edge;
	List* next;
	unsigned long long source;
      };
      
      unsigned freq_counter; //< Frequency counter.
      int      region;
      bool     call;
      
      List* edges;
      List* back_edges;
      
    private:
      unsigned long long addr; //< Instruction address.
      string          op_name; //< Instruction opcode name.
      
    };
    
    /** 
     *  @brief A Region Edge represents the control flow between instructions.
     */
    class Edge
    {
    public:

      Edge() {}
      Edge(Node* x, Node* y) : src(x), tgt(y), count(1) {}
      
      /// Address of the target instruction.
      unsigned long long target() { return tgt->getAddress(); }
      /// Address of the source instruction.
      unsigned long long source() { return src->getAddress(); }
      
    public:
      Node* src; 
      Node* tgt;

      unsigned count; //< Frequency counter
    };
    
  public:
    
    Region() : counter(0)
      {}
    Region(unsigned long long addr) : counter(0)
      {
	Node* node = new Node(addr);
	includeNode(node,addr);
      }
	
    ~Region();
    
    bool update(unsigned long long);
    
    void includeNode(Node * node, unsigned long long addr){nodes.insert(make_pair(addr,node));}
    void setEntryNode(Node* node){entrys.push_back(node);}
    void setExitNode(Node* node){exits.push_back(node);}
    void createEdge(Node*, Node*);
    
    list<Node* > getEntryNodes(){return entrys;}
    Node* getEntry() {return entrys.front();}
    list<Node* > getExitNodes(){return exits;}
    map<unsigned long long,Node* > getNodes(){return nodes;}
    int size(){return (int) nodes.size();}
    map<unsigned long long,Node* >::iterator getIterator(bool);
    
    void cleanExits(){exits.sort(); exits.unique();}
    
    /**
     * Methods required by Treegion
     */
    map<unsigned long long,Node* >::iterator find(unsigned long long);
    map<unsigned long long,Edge* >::iterator getIteratorEdge(bool);
    void copyRegion(Region*, map<unsigned long long,Node* >::iterator);

    unsigned counter;

  private:
    /** A região é composta por nós ligados por arestas, semelhante a um CFG. */
    map<unsigned long long,Node* > nodes;
    list<Node* > entrys;
    list<Node* > exits;
    
  };
  
  /*
   * --- Classe que implementa a estrutura do RAIn. ---
   */
  class RAIn
  {
  private:
    bool in_region; //processamento está em alguma região já formada.
    bool stop; //indica se o processo de criação de uma região deva parar.
    bool just_quit; //indica se acabou de sair de uma região.
    
    //Node* nte; // No Trace being Executed.
    //unsigned count; //counter.
    //map<int, Region*> regions;
    //map<unsigned long long, int> entrys;
    map<unsigned long long, Region::Edge*> edges;
    //bool in_nte;
    int cur_reg; //current region(índice).
  
  public:	
    
    // Moved from global to here.... :-(
    Region::Node* last_node;
    Region::Node* cur_node;
    Region::Node* next_node;
    
    Region::Node* nte;
    map<int, Region*> regions;
    bool in_nte;
    map<unsigned long long, int> entrys;
    unsigned count;
    unsigned cur_index;
    unsigned long long cur_ip; //current instruction pointer.
    string last_name; //usado para tt.
    bool system;
    
    RAIn()
    {
      nte = new Region::Node(0);
      count = 1;
      cur_index = 1;
      nte->region = 0;
      system = false;
      in_region = false;
      stop = false;
      just_quit = false;
    }
  
    ~RAIn()
    {
      delete nte;
      edges.clear();
      regions.clear();
    }
  
    void verify(unsigned long long);
    void update(unsigned long long);
    Region::Node* next(unsigned long long);
    void currentRegion(int);
    
    void updateEdges(bool,int);
    
    Region* createRegion();
    Region::Node* createNode(Region *, unsigned long long);
    bool insertNode(Region *, Region::Node*);
    void insertRegion(Region *);
    void createEdge(Region *, Region::Node*, Region::Node*);
    void createEdge(Region::Node*, Region::Node*);
    void createOrUpdateEdge(Region::Node*, Region::Node*);
    void setEntry(Region *, Region::Node *);
    void setExit(Region *, Region::Node *);
    
    void printRegion(Region *, int, ostream&, ostream&);
    void printTable();
    void printRAIn(bool, ostream&, set<unsigned>&);
    void toValidate(set<unsigned>&);
    void printValidate(Region *,int,int,ostream&);
  };
  
  class RF_Technique
  {
  public:

    virtual void 
      process(unsigned long long cur_addr, char cur_opcode[16], char unsigned cur_length, 
	      unsigned long long nxt_addr, char nxt_opcode[16], char unsigned nxt_length) = 0;

    virtual void finish() = 0;

    RAIn rain;
  };

};

#endif // RAIN_H
