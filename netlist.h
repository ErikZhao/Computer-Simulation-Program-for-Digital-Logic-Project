//netlist.h

#ifndef GUARD_NETLIST_H
#define GUARD_NETLIST_H

#include<map>
#include<stdexcept>
#include<sstream>
#include <assert.h>

#include "syn.h"


class netlist;
class gate;
class net;
class pin;



typedef std::map<std::string, int> evl_wires_table;

evl_wires_table make_wires_table(const evl_wires &wires) ;
void display_wires_table(std::ostream &out, const evl_wires_table &wires_table) ;
std::string make_net_name(std::string wire_name, int i);

class net {

public:
	std::list<pin *> connections_;
	std::string net_name;
	void append_pin(pin *p);

}; // class net

class pin {

public:
	bool create(gate *g, size_t pin_index, const evl_pin &p,
	const std::map<std::string, net *> &nets_table);
	gate *gate_;
	net *net_;
	size_t pin_index_;
	std::vector<net *> nets_;
}; // class pin

class gate {

public:
	std::vector<pin *> pins_;
	std::string type;
	std::string name;
	bool create(const evl_component &c, const std::map<std::string, net *> &nets_table, const evl_wires_table &wires_table);
	bool create_pin(const evl_pin &ep, size_t pin_index,
		const std::map<std::string, net *> &nets_table,
		const evl_wires_table &wires_table);
}; // class gate

class netlist {
	std::list<gate *> gates_;
	std::list<net *> nets_;
	std::map<std::string, net *> nets_table_;

public:
	bool create(const evl_wires &wires, const evl_components &comps, const evl_wires_table &wires_table);
	bool create_nets(const evl_wires &wires);
	void create_net(std::string net_name);
	bool create_gates(const evl_components &comps,
		const evl_wires_table &wires_table);
	bool create_gate(const evl_component &c,
		const evl_wires_table &wires_table);
	void display(std::ostream &out, std::string module_name);
	bool save(std::string nl_file, std::string module_name);
	void simulate(int cycles);
}; // class netlist





#endif
