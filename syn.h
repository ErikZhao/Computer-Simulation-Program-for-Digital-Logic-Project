#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <list>

#ifndef GUARD_SYN_H
#define GUARD_SYN_H

struct evl_token {
	enum token_type {NAME, NUMBER, SINGLE};
	token_type type;
	std::string str;
	int line_no;
}; // struct evl_token
typedef std::list<evl_token> evl_tokens;

struct evl_wire{
	std::string name;
	int width;
}; // struct evl_wire
typedef std::vector<evl_wire> evl_wires;

struct evl_pin{
	std::string name;
	int bus_msb;
	int bus_lsb;
}; //struct evl_pin
typedef std::vector<evl_pin> evl_pins;

struct evl_component{
	std::string name;
	std::string type;
	evl_pins pins;
};// struct evl_component
typedef std::vector<evl_component> evl_components;

struct evl_statement{
	enum statement_type {MODULE, WIRE, COMPONENT, ENDMODULE};
	statement_type type;
	evl_tokens tokens;
	evl_wires wires;
	evl_components components;
}; // struct evl_statement
typedef std::list<evl_statement> evl_statements;

	bool extract_tokens_from_line(std::string line, int line_no, evl_tokens &tokens);
	bool extract_tokens_from_file(std::string file_name, evl_tokens &tokens);
	void display_tokens(std::ostream &out, const evl_tokens &tokens);
	bool store_tokens_to_file(std::string file_name, const evl_tokens &tokens);
	bool move_tokens_to_statement(	evl_tokens &statement_tokens,	evl_tokens &tokens);
	bool process_wire_statement(evl_wires &wires, evl_statement &s);
	bool process_component_statement(evl_components &components, evl_statement &s);
	bool group_tokens_into_statements(	evl_statements &statements,	evl_tokens &tokens);
	void display_statements(std::ostream &out,	const evl_statements &statements);
	bool store_statements_to_file(std::string file_name, const evl_statements &statements) ;

	bool parse_evl_file(std::string evl_file, std::string &module_name, evl_wires &wires, evl_components &comps);

#endif



