#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <list>
#include "syn.h"
#include "netlist.h"


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "You should provide a file name." << std::endl;
        return -1;
    }

    std::string module_name;
    evl_wires wires;
    evl_components comps;
    if(!parse_evl_file(argv[1], module_name, wires, comps))
    	return -1;

    evl_wires_table wires_table = make_wires_table(wires);

    netlist nl;
    if(!nl.create(wires, comps, wires_table))
    	return -1;
    //netlist nl(wires, comps, wires_table);

    std::string nl_file = std::string(argv[1])+".sim_out.evl_output";
    nl.save(nl_file, module_name); //save the netlist for Project 3

    //nl.simulate(1000); // simulate 1000 cycles for Porject 4


    //project 2 main function
    /*
    std::string evl_file = argv[1];
    evl_tokens tokens;
    evl_statements statements;


    if (!extract_tokens_from_file(evl_file, tokens)) {
    	return -1;
    }

    display_tokens(std::cout, tokens);

    if (!store_tokens_to_file(evl_file+".tokens", tokens)) {
    	return -1;
    }



    if(!group_tokens_into_statements(statements, tokens)){
    	return -1;
    }

    display_statements(std::cout, statements);

    if(!store_statements_to_file(evl_file+".syntax", statements)){
    	return -1;
    }
    */

    return 0;
}


