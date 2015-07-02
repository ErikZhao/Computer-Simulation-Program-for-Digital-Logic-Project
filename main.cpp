#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <list>
#include "syn.h"


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "You should provide a file name." << std::endl;
        return -1;
    }

    std::string evl_file = argv[1];
    evl_tokens tokens;
    evl_statements statements;


    if (!extract_tokens_from_file(evl_file, tokens)) {
    	return -1;
    }
    /*
    display_tokens(std::cout, tokens);

    if (!store_tokens_to_file(evl_file+".tokens", tokens)) {
    	return -1;
    }
    */


    if(!group_tokens_into_statements(statements, tokens)){
    	return -1;
    }

    display_statements(std::cout, statements);

    if(!store_statements_to_file(evl_file+".syntax", statements)){
    	return -1;
    }

    return 0;
}


