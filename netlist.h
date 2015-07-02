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


typedef enum pinType{ INPUT, OUTPUT }pinType; // define pin type
typedef std::map<std::string, int> evl_wires_table;

evl_wires_table make_wires_table(const evl_wires &wires) ;
void display_wires_table(std::ostream &out, const evl_wires_table &wires_table) ;
std::string make_net_name(std::string wire_name, int i);


extern std::string file;

class net {

	friend class netlist;

	net(std::string name, int v, bool f){
		net_name = name;
		value = v;
		flag = f;
	}
	~net(){}
	net(const net &);
	net &operator = (const net &);
	std::list<pin *> connections_;
	std::string net_name;
	int value; //true is 1 and false is 0
	bool flag;
public:

	void append_pin(pin *p);
	void set_logic_value(int logic_value){
		value = logic_value;
		flag = true;
	}
	void set_logic_value(){
		flag = true;
	}
	int get_logic_value();
	int compute(); // compute logic value

}; // class net

class pin {

	friend class net;
	friend class gate;
	friend class netlist;

	gate *gate_;
	net *net_;
	size_t pin_index_;
	pinType type;

	bool create(gate *g, size_t pin_index, const evl_pin &p,
	const std::map<std::string, net *> &nets_table);

public:
	std::vector<net *> nets_;
	void set_as_output(){ //set pin as output
		type = OUTPUT;
	}
	void set_as_input(){ //set pin as input
		type = INPUT;
	}
}; // class pin

class gate {

	friend class net;
	friend class netlist;

	gate(const gate &);
	gate &operator = (const gate &);

	bool create(const evl_component &c, const std::map<std::string, net *> &nets_table, const evl_wires_table &wires_table);
	bool create_pin(const evl_pin &ep, size_t pin_index,
		const std::map<std::string, net *> &nets_table,
		const evl_wires_table &wires_table);

	virtual bool validate_structural_semantics() = 0;
	virtual bool compute_output() {return true;};

protected:
	std::vector<pin *> pins_;
	std::string type;
	std::string name;
	gate( std::string type, std::string name):type(type), name(name){}
	virtual ~gate(){};
}; // class gate

class netlist {

	std::list<gate *> gates_;
	std::list<net *> nets_;
	std::map<std::string, net *> nets_table_;
	std::vector<gate *> outputs_;

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
	bool simulate(int cycles);

}; // class netlist


class flip_flop: public gate{

      int state_;
      int next_state_;

      bool validate_structural_semantics();
      bool compute_output();

      public:
             flip_flop(std::string name):gate("evl_dff",name),state_(false),next_state_(false){}
             ~flip_flop(){};
};// class flip_flop


class not_gate: public gate{

      bool validate_structural_semantics();
      bool compute_output();
      public:
             not_gate(std::string name):gate("not",name){}
             ~not_gate(){};
};// class not gate


class output_gate: public gate{

      bool validate_structural_semantics();
      bool compute_output();
      public:
             output_gate(std::string name):gate("evl_output",name){}
             ~output_gate(){};
};// class output_gate


class one_gate: public gate{

      bool validate_structural_semantics();
      bool compute_output();
      public:
             one_gate( std::string name ):gate( "evl_one",name ){}
             ~one_gate(){};
};// class one_gate


class zero_gate: public gate{

      bool validate_structural_semantics();
      bool compute_output();
      public:
             zero_gate( std::string name ):gate( "evl_zero",name ){}
             ~zero_gate(){};
};// class zero_gate

class and_gate: public gate{

	bool validate_structural_semantics();
	bool compute_output();
    public:
			and_gate( std::string name ):gate( "and", name){}
			~and_gate(){};

};

class or_gate: public gate{
	bool validate_structural_semantics();
	bool compute_output();
    public:
			or_gate( std::string name ):gate( "or", name){}
			~or_gate(){};
};

class xor_gate: public gate{
	bool validate_structural_semantics();
	bool compute_output();
    public:
			xor_gate( std::string name ):gate( "xor", name){}
			~xor_gate(){};
};

class buf_gate: public gate{
	bool validate_structural_semantics();
	bool compute_output();
    public:
			buf_gate( std::string name ):gate( "buf", name){}
			~buf_gate(){};
};

class input_gate: public gate{
	bool validate_structural_semantics();
	bool compute_output();

	int number_of_transitions;
	std::ifstream read;
    public:
			input_gate( std::string name_ ):gate( "evl_input", name_){
				number_of_transitions = -1;
				std::string filename = std::string(file) + "." + name + ".evl_input";
				std::cout << "read inputs file from "<< filename << std::endl;
			if(!read)
				std::cerr << "error:input file doesn't exist" << std::endl;
			else
				read.open(filename.c_str(),std::ios::in);
			}
			~input_gate(){};
};

class lut_gate: public gate{
	bool validate_structural_semantics();
	bool compute_output();

	int first_line_flag;
	std::ifstream read;
	std::vector<std::string> lut_values_;
    public:
			lut_gate( std::string name_ ):gate( "evl_lut", name_){
				first_line_flag = 1;
				//first_line_flag = 1;
				std::string filename = std::string(file) + "." + name + ".evl_lut";
				std::cout << "read lut file from "<< filename << std::endl;
			if(!read)
				std::cerr << "error:lut file doesn't exist" << std::endl;
			else
				read.open(filename.c_str(),std::ios::in);
			}
			~lut_gate(){};
};

class tris_gate: public gate{
	bool validate_structural_semantics();
	bool compute_output();

    public:
			tris_gate( std::string name ):gate( "tris", name){
			}
			~tris_gate(){};
};

class clock_gate: public gate{
	bool validate_structural_semantics();
	bool compute_output();

    public:
			clock_gate( std::string name ):gate( "evl_tris", name){
			}
			~clock_gate(){};
};
#endif
