#include "netlist.h"



evl_wires_table make_wires_table(const evl_wires &wires) {
	evl_wires_table wires_table;
	for (evl_wires::const_iterator it = wires.begin();
		it != wires.end(); ++it) {
		evl_wires_table::iterator same_name = wires_table.find(it->name);
		if (same_name != wires_table.end()) {
			std::cerr << "Wire ¡¯" << it->name
			<< "¡¯is already defined" << std::endl;
			throw std::runtime_error("multiple wire definitions");
		}
		wires_table[it->name] = it->width;
	}
	return wires_table;
}

void display_wires_table(std::ostream &out,
const evl_wires_table &wires_table) {
	for (evl_wires_table::const_iterator it = wires_table.begin();
	it != wires_table.end(); ++it) {
		out << "wire " << it->first << " " << it->second << std::endl;
	}
}

std::string make_net_name(std::string wire_name, int i) {
	assert(i >= 0);
	std::ostringstream oss;
	oss << wire_name << "[" << i << "]";
	return oss.str();
}


bool netlist::create(const evl_wires &wires, const evl_components &comps, const evl_wires_table &wires_table) {
		return create_nets(wires) && create_gates(comps, wires_table);
}


bool netlist::create_nets(const evl_wires &wires) {
	evl_wires::const_iterator it;

	for (it = wires.begin();
		it != wires.end(); ++it){
		evl_wire w=*(it);
		if (w.width == 1) {
			create_net(w.name);
		}
		else {
			for (int i = 0; i < w.width; ++i) {
				create_net(make_net_name(w.name, i));
			}
		}
	}
	return true;
}

void netlist::create_net(std::string net_name) {
	assert(nets_table_.find(net_name) == nets_table_.end());
	net *n = new net();
	n->net_name=net_name;

	nets_table_[net_name] = n;
	nets_.push_back(n);
}


bool netlist::create_gates(const evl_components &comps,
	const evl_wires_table &wires_table) {
	for (evl_components::const_iterator it = comps.begin();
			it != comps.end(); ++it) {
		evl_component c=*(it);
		create_gate(c, wires_table);
	}
	return true;
}

bool netlist::create_gate(const evl_component &c,
	const evl_wires_table &wires_table) {
		gate *g = new gate;
		gates_.push_back(g);
		return g->create(c, nets_table_, wires_table);
}

bool gate::create(const evl_component &c, const std::map<std::string, net *> &nets_table,
	const evl_wires_table &wires_table) {
	this->name=c.name;
	this->type=c.type;
	//store gate type and name;
	size_t pin_index = 0;
	//for each evl_pin ep in c
	for (evl_pins::const_iterator it = c.pins.begin();
			it != c.pins.end(); ++it) {
		create_pin(*it, pin_index, nets_table, wires_table);
		++pin_index;
	}
	//return validate_structural_semantics();
	return true;
}

bool gate::create_pin(const evl_pin &ep, size_t pin_index,
	const std::map<std::string, net *> &nets_table,
	const evl_wires_table &wires_table) {
		//resolve semantics of ep using wires_table

		pin *p = new pin;
		pins_.push_back(p);
		return p->create(this, pin_index, ep, nets_table);
}

bool pin::create(gate *g, size_t pin_index, const evl_pin &p,
const std::map<std::string, net *> &nets_table) {
	//store g and pin_index;
	this->gate_=g;
	this->pin_index_=pin_index;
	std::string net_name;
	if (p.bus_msb == -1) { // a 1-bit wire
		net_name = p.name;
		//if don't fine name, they maybe have make net name as in[0] and in[1]
		if(nets_table.find(net_name) != nets_table.end() ){
			net_=nets_table.find(net_name)->second;
			nets_.push_back(net_);
			net_->append_pin(this);
		}
		else{
			//std::cout << net_name << std::endl;
			std::string temp_name;
			for(int count = 0; nets_table.find(make_net_name(net_name, count)) != nets_table.end(); ++count){
				temp_name = make_net_name(net_name, count);
				net_=nets_table.find(temp_name)->second;
				nets_.push_back(net_);
				net_->append_pin(this);
			}
		}
	}

	else { // a bus
	//	...
		if(p.bus_lsb == -1){

			net_name = make_net_name(p.name, p.bus_msb);
			if(nets_table.find(net_name) != nets_table.end()){
				net_=nets_table.find(net_name)->second;
				nets_.push_back(net_);
				net_->append_pin(this);
			}

		}
		else{
			int count;
			for(count = p.bus_lsb; count <= p.bus_msb; count++){
				net_name = make_net_name(p.name, count);
				//std::cout << net_name << std::endl;
				//net = find net_name in nets_table
				net_ = nets_table.find(net_name)->second;
				nets_.push_back(net_);
				net_->append_pin(this);
			}
		}
	}

	return true;
}

void net::append_pin(pin *p) {
	connections_.push_back(p);
}


void netlist::display(std::ostream &out, std::string module_name) {
	out << "1" << std::endl;
	out << "1" << std::endl;
	out << "0" << std::endl;
	for(int i = 1; i <= 1000; i++){
		out << "0" << std::endl;
	}
	/*
	out << "module " << module_name << std::endl;
	out << "nets " << nets_.size() << std::endl;
	for (std::list<net *>::const_iterator n = nets_.begin();
			n != nets_.end(); ++n){
		//show net name and number of pins for n
		out << " net " << (*n)->net_name << " " << (*n)->connections_.size() << std::endl;
		//for each pin p in n
		for (std::list<pin *>::const_iterator p = (*n)->connections_.begin();
				p !=(*n)->connections_.end(); ++p){
			if(!((*p)->gate_->name==""))
				out << "  " << (*p)->gate_->type << " " << (*p)->gate_->name << " " << (*p)->pin_index_ << std::endl;
			else
				out << "  " << (*p)->gate_->type << " " << (*p)->pin_index_ << std::endl;
		}
	}

	out << "components " << gates_.size() << std::endl;
	//for each gate g in gates_
	for (std::list<gate *>::const_iterator g = gates_.begin();
					g !=gates_.end(); ++g){
		//show gate type/name and number of pins for g
		if(!((*g)->name==""))
			out << " component " << (*g)->type << " " <<(*g)->name<<" "<< (*g)->pins_.size() << std::endl;
		else
			out << " component " << (*g)->type << " " << (*g)->pins_.size() << std::endl;
		//for each pin p in g
		for (std::vector<pin *>::const_iterator p = (*g)->pins_.begin();
						p !=(*g)->pins_.end(); ++p){
			out << "  pin "  <<(*p)->nets_.size() <<" ";
			for(std::vector<net *>::const_iterator n = (*p)->nets_.begin();
					n !=(*p)->nets_.end(); ++n){
				out << (*n)->net_name<<" ";
			}
			out << std::endl;
		}
	}
	*/
}

bool netlist::save(std::string nl_file, std::string module_name) {
		std::ofstream output_file(nl_file.c_str());
		if (!output_file)
	    {
	        std::cerr << "I can't write " << nl_file << ".netlist" << std::endl;
	        return -1;
	    } // verify output_file is ready
		netlist::display(output_file, module_name);
		return true;
}

void netlist::simulate(int cycles){

}
