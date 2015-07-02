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
	net *n = new net(net_name, 0, false);

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
		if(c.type == "not"){
			gates_.push_back(new not_gate(c.name));
		}
		else if(c.type == "and"){
			gates_.push_back(new and_gate(c.name));
		}
		else if(c.type == "or"){
			gates_.push_back(new or_gate(c.name));
		}
		else if(c.type == "xor"){
			gates_.push_back(new xor_gate(c.name));
		}
		else if(c.type == "buf"){
			gates_.push_back(new buf_gate(c.name));
		}
		else if(c.type == "evl_one"){
			gates_.push_back(new one_gate(c.name));
		}
		else if(c.type == "evl_zero"){
			gates_.push_back(new zero_gate(c.name));
		}
		else if(c.type == "evl_dff"){
			gates_.push_back(new flip_flop(c.name));
		}
	    else if( c.type == "evl_output"){
	         gates_.push_back(new output_gate(c.name));
	         outputs_.push_back(gates_.back());
	         //std::cout << outputs_.back()->name<< std::endl;
	    }
	    else if(c.type == "evl_input"){
	    	gates_.push_back(new input_gate(c.name));
	    }
	    else if(c.type == "evl_lut"){
	    	gates_.push_back(new lut_gate(c.name));
	    }
	    else if(c.type == "tris"){
	    	gates_.push_back(new tris_gate(c.name));
	    }
	    else if(c.type == "evl_clock"){
	    	gates_.push_back(new clock_gate(c.name));
	    }
		else{
			return false;
		}

		if((gates_.back()->create(c, nets_table_, wires_table)) == false){
			std::cerr << "error while creating gate!"<<std::endl;
			return false;
		}
		return true;
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
	return validate_structural_semantics();
	//return true;
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

bool gate::validate_structural_semantics() {
    return false;
}


// not gate validate
bool not_gate::validate_structural_semantics() {
    if ( pins_.size() != 2 ){
        std::cerr <<"not gate has 2 pins"<< std::endl;
        return false;
    }
    else if ( pins_[0]->nets_.size() != 1 || pins_[1]->nets_.size() != 1 ){
        std::cerr <<"pin width (net size) of not gate is 1"<< std::endl;
        return false;
    }
    else{
         pins_[0]->set_as_output();
         pins_[1]->set_as_input();
         return true;
    }

}

//validate gate and
bool and_gate::validate_structural_semantics(){
	if(pins_.size() < 3){
		std::cerr << "error: and should have more than or equal to 3 pins" << std::endl;
		return false;
	}
	if(pins_[0]->nets_.size() != 1){
		std::cerr << "error: pin width of and gate should be 1"<<std::endl;
	}
	else pins_[0]->set_as_output();
	for(size_t i = 1; i < pins_.size(); i++){
		if(pins_[i]->nets_.size() != 1){
			std::cerr << "error: pin width of and gate should be 1 " << std::endl;
			return false;
		}
		else pins_[i]->set_as_input();
	}
	return true;
}

//validate gate or
bool or_gate::validate_structural_semantics(){
	if(pins_.size() < 3){
		std::cerr << "error: or should have more than or equal to 3 pins" << std::endl;
		return false;
	}
	if(pins_[0]->nets_.size() != 1){
		std::cerr << "error: pin width of or gate should be 1"<<std::endl;
	}
	else pins_[0]->set_as_output();
	for(size_t i = 1; i < pins_.size(); i++){
		if(pins_[i]->nets_.size() != 1){
			std::cerr << "error: pin width of or gate should be 1 " << std::endl;
			return false;
		}
		else pins_[i]->set_as_input();
	}
	return true;
}

//validate xor gate
bool xor_gate::validate_structural_semantics(){
	if(pins_.size() < 3){
		std::cerr << "error: xor should have more than or equal to 3 pins" << std::endl;
		return false;
	}
	if(pins_[0]->nets_.size() != 1){
		std::cerr << "error: pin width of xor gate should be 1"<<std::endl;
	}
	else pins_[0]->set_as_output();
	for(size_t i = 1; i < pins_.size(); i++){
		if(pins_[i]->nets_.size() != 1){
			std::cerr << "error: pin width of xor gate should be 1 " << std::endl;
			return false;
		}
		else pins_[i]->set_as_input();
	}
	return true;
}

// validate buf
bool buf_gate::validate_structural_semantics() {
    if ( pins_.size() != 2 ){
        std::cerr <<"buf gate has 2 pins"<< std::endl;
        return false;
    }
    else if ( pins_[0]->nets_.size() != 1 || pins_[1]->nets_.size() != 1 ){
        std::cerr <<"width of buf pin is 1"<< std::endl;
        return false;
    }
    else{
         pins_[0]->set_as_output();
         pins_[1]->set_as_input();
         return true;
    }

}

// one gate validate
bool one_gate::validate_structural_semantics() {

    if ( pins_.size() < 1 ){
        return false;
    }
    else{
         for(size_t i=0; i<pins_.size(); i++ ){
              pins_[i]->set_as_output();
         }
         return true;
    }
}

// zero gate validate
bool zero_gate::validate_structural_semantics() {
    if ( pins_.size() < 1 ){
        return false;
    }
    else{
         for(size_t i=0; i<pins_.size(); i++ ){
              pins_[i]->set_as_output();
         }
         return true;
    }
}

//output gate validate
bool output_gate::validate_structural_semantics() {
    if ( pins_.size() < 1 ){
    	 return false;
    }
    else{
         for(size_t i=0; i<pins_.size(); i++ ){
              pins_[i]->set_as_input();
         }
         return true;
    }

}

//input gate validate
bool input_gate::validate_structural_semantics() {
    if ( pins_.size() < 1 ){
    	std::cerr << "input gate should have at least 2 pins" << std::endl;
        return false;
    }
    else{
         for(size_t i=0; i<pins_.size(); i++ ){
              pins_[i]->set_as_output();
         }
         return true;
    }

}

// validate dff
bool flip_flop::validate_structural_semantics() {
    if ( pins_.size() != 3 ){
        std::cerr <<"dff gate has 3 pins"<< std::endl;
        return false;
    }
    else if ( pins_[0]->nets_.size() != 1 || pins_[1]->nets_.size() != 1 ){
        std::cerr <<"width of dff pin is 1"<< std::endl;
        return false;
    }
    else{
         pins_[0]->set_as_output();
         pins_[1]->set_as_input();
         return true;
    }

}

bool lut_gate::validate_structural_semantics() {
    if ( pins_.size() != 2 ){
    	std::cerr << "lut gate should have 2 pins" << std::endl;
        return false;
    }
    else{
         pins_[0]->set_as_output();
         pins_[1]->set_as_input();
         return true;
    }

}

bool tris_gate::validate_structural_semantics() {
    if ( pins_.size() != 3 ){
        std::cerr <<"tris gate has 2 pins"<< std::endl;
        return false;
    }
    else if ( pins_[0]->nets_.size() != 1 || pins_[1]->nets_.size() != 1 || pins_[2]->nets_.size() != 1){
        std::cerr <<"width of tris pin is 1"<< std::endl;
        return false;
    }
    else{
         pins_[0]->set_as_output();
         pins_[1]->set_as_input();
         pins_[2]->set_as_input();
         return true;
    }

}

bool clock_gate::validate_structural_semantics(){
	if (pins_.size() < 1){
		std::cerr << "Clock could one have one net" << std::endl;
		return false;
	}
	for (size_t i = 0; i < pins_.size(); ++i){
		pins_[i]->set_as_output();

	}
	return true;
}

