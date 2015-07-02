#include "syn.h"

bool extract_tokens_from_line(std::string line, int line_no, evl_tokens &tokens) { // use reference to modify it
	for (size_t i = 0; i < line.size();) {
		if (line[i] == '/') {
			++i;
			if ((i == line.size()) || (line[i] != '/'))
			{
				std::cerr << "LINE " << line_no
					<< ": a single / is not allowed" << std::endl;
				return -1;
			}
			break; // skip the rest of the line by exiting the loop
		} // process comments

		else if (isspace(line[i])) {
			++i; // skip this space character
			continue; // skip the rest of the iteration
		} // skip spaces

		else if ((line[i] == '(') || (line[i] == ')')
				|| (line[i] == '[') || (line[i] == ']')
				|| (line[i] == ':') || (line[i] == ';')
				|| (line[i] == ',')) {
			evl_token token;
			token.line_no = line_no;
			token.type = evl_token::SINGLE;
			token.str = std::string(1, line[i]);
			tokens.push_back(token);
			i++;
			continue;// a SINGLE token
		}

		else if (isalpha(line[i]) || (line[i] == '_')) {

			size_t name_begin = i;
			for (++i; i < line.size(); ++i)
			{
				if (!(isalpha(line[i])
					|| ((line[i] >= '0') && (line[i] <= '9'))
					|| (line[i] == '_') || (line[i] == '$')))
				{
					break; // [name_begin, i) is the range for the token
				}
			} // a NAME token
			evl_token token;
			token.line_no = line_no;
			token.type = evl_token::NAME;
			token.str = line.substr(name_begin, i-name_begin);
			tokens.push_back(token);
		}

		else if (isdigit(line[i])) {
			size_t name_begin = i;
			for (++i; i < line.size(); ++i)
			{
				if (!(((line[i] >= '0') && (line[i] <= '9'))
					||(line[i] == '(') || (line[i] == ')')
					|| (line[i] == '[') || (line[i] == ']')
					|| (line[i] == ':') || (line[i] == ';')
					|| (line[i] == ',') || (line[i] == ' ')
					|| (line[i] == '\t')|| (line[i] == '\r')))
				{
					std::cerr << "LINE " << line_no
						<< ": invalid character" << std::endl;
					return -1;
				}

				else if((line[i] == '(') || (line[i] == ')')
						|| (line[i] == '[') || (line[i] == ']')
						|| (line[i] == ':') || (line[i] == ';')
						|| (line[i] == ',') || (line[i] == ' ')
						|| (line[i] == '\t')|| (line[i] == '\r'))
				{
					break;
				}
			}
			evl_token token;
			token.line_no = line_no;
			token.type = evl_token::NUMBER;
			token.str = line.substr(name_begin, i-name_begin);
			tokens.push_back(token); // a NUMBER token
		}
		else {
			std::cerr << "LINE " << line_no
			<< ": invalid character" << std::endl;
			return false;
		}
	}
	return true; // nothing left
	}

bool extract_tokens_from_file(std::string file_name, evl_tokens &tokens) { // use reference to modify it
	std::ifstream input_file(file_name.c_str());

	if (!input_file) {
		std::cerr << "I can't read " << file_name << "." << std::endl;
		return false;
	}

	tokens.clear(); // be defensive, make it empty
	std::string line;
	for (int line_no = 1; std::getline(input_file, line); ++line_no) {
		if (!extract_tokens_from_line(line, line_no, tokens)) {
			return false;
		}
	}
	return true;
}

void display_tokens(std::ostream &out, const evl_tokens &tokens) {
	evl_tokens dis=tokens;
	for (; !dis.empty(); dis.pop_front()){
			evl_token token=dis.front();

			if (token.type == evl_token::SINGLE){
				out << "SINGLE " << token.str << std::endl;
			}
			else if (token.type == evl_token::NAME){
				out << "NAME " << token.str << std::endl;
			}
			else // must be NUMBER
				out << "NUMBER " << token.str << std::endl;
		}
	}


bool store_tokens_to_file(std::string file_name, const evl_tokens &tokens) {
	std::ofstream output_file(file_name.c_str());
	if (!output_file)
    {
        std::cerr << "I can't write " << file_name << ".tokens ." << std::endl;
        return -1;
    } // verify output_file is ready
	display_tokens(output_file, tokens);
	return true;
}

bool move_tokens_to_statement(	evl_tokens &statement_tokens,	evl_tokens &tokens) {

	for (; !tokens.empty();) {
		statement_tokens.push_back(tokens.front());
		tokens.erase(tokens.begin()); // consume one token per iteration
		//std::cout<<"erase "<<tokens.front().str<<std::endl;
		if (statement_tokens.back().str == ";")
			break; // exit if the ending ";" is found
	}
	if (statement_tokens.back().str != ";") {
		std::cerr << "Look for ';' but reach the end of file" << std::endl;
		return false;
	}
	return true;
}



bool process_wire_statement(evl_wires &wires, evl_statement &s) {
	enum state_type {INIT, WIRE, DONE, WIRES, WIRE_NAME, BUS, BUS_MSB, BUS_COLON, BUS_LSB, BUS_DONE};
	state_type state = INIT;
	int bus_width = 1;
	// use branches here to compute state transitions
	for (; !s.tokens.empty() && (state != DONE); s.tokens.pop_front()) {
		evl_token t = s.tokens.front();
			if (state == INIT) {
				if (t.str == "wire") {
					state = WIRE;
				}
				else {
					std::cerr << "Need 'wire' but found '" << t.str
					<< "' on line " << t.line_no << std::endl;
					return false;
				}
			}
			else if (state == WIRE) {
				if (t.type == evl_token::NAME) {
					evl_wire wire;
					wire.name = t.str; wire.width = bus_width;
					wires.push_back(wire);
					state = WIRE_NAME;
				}
				else if(t.str == "["){
					state = BUS;
				}
				else {
					std::cerr << "Need NAME or '[' but found '" << t.str
					<< "' on line " << t.line_no << std::endl;
					return false;
				}
			}
			else if (state == WIRES) {
				if (t.type == evl_token::NAME) {
					evl_wire wire;
					wire.name = t.str; wire.width = bus_width;
					wires.push_back(wire);
					state = WIRE_NAME;
				}
				else {
					std::cerr << "Need NAME but found '" << t.str
					<< "' on line " << t.line_no << std::endl;
					return false;
				}
			}
			else if (state == WIRE_NAME) {
				if (t.str == ",") {
					state = WIRES;
				}
				else if (t.str == ";") {
					state = DONE;
				}
				else {
					std::cerr << "Need ',' or ';' but found '" << t.str
					<< "' on line " << t.line_no << std::endl;
					return false;
				}
			}
			else if (state == BUS){
				if(t.type == evl_token::NUMBER){

					bus_width = atoi(t.str.c_str())+1;
					state = BUS_MSB;
				}
				else {
					std::cerr << "Need 'wire' but found '" << t.str
					<< "' on line " << t.line_no << std::endl;
					return false;
				}
			}
			else if (state == BUS_MSB){
				if(t.str == ":"){
					state = BUS_COLON;
				}
				else {
					std::cerr << "Need 'wire' but found '" << t.str
					<< "' on line " << t.line_no << std::endl;
					return false;
				}
			}
			else if (state == BUS_COLON){
				if(t.str == "0"){
					state = BUS_LSB;
				}
				else {
					std::cerr << "Need 'wire' but found '" << t.str
					<< "' on line " << t.line_no << std::endl;
					return false;
				}
			}
			else if (state == BUS_LSB){
				if(t.str == "]"){
					state = BUS_DONE;
				}
				else {
					std::cerr << "Need 'wire' but found '" << t.str
					<< "' on line " << t.line_no << std::endl;
					return false;
				}
			}
			else if (state == BUS_DONE){
				if (t.type == evl_token::NAME) {
					evl_wire wire;
					wire.name = t.str; wire.width = bus_width;
					wires.push_back(wire);
					state = WIRE_NAME;
				}
				else {
					std::cerr << "Need NAME but found '" << t.str
					<< "' on line " << t.line_no << std::endl;
					return false;
				}
			}

	}
	if (!s.tokens.empty() || (state != DONE)) {
		std::cerr << "something wrong with the statement" << std::endl;
		return false;
	}
	return true;
}

bool process_component_statement(evl_components &components, evl_statement &s) {
	enum state_type {INIT, TYPE, NAME, PINS, PIN_NAME, BUS, BUS_MSB, BUS_COLON, BUS_LSB, BUS_DONE, PINS_DONE, DONE};
	state_type state = INIT;
	evl_component comp;
	evl_pin pin;
	// use branches here to compute state transitions
	for (; !s.tokens.empty() && (state != DONE); s.tokens.erase(s.tokens.begin())) {
		evl_token t = s.tokens.front();
			if (state == INIT) {
				if (t.type == evl_token::NAME) {
					comp.type = t.str;
					comp.name="";
					state = TYPE;
				}
				else {
					std::cerr << "Need NAME but found '" << t.str
					<< "' on line " << t.line_no << std::endl;
					return false;
				}
			}
			else if (state == TYPE) {
				if (t.type == evl_token::NAME) {
					comp.name = t.str;

					state = NAME;
				}
				else if(t.str == "("){
					state = PINS;

				}
				else {
					std::cerr << "Need NAME or '(' but found '" << t.str
					<< "' on line " << t.line_no << std::endl;
					return false;
				}
			}
			else if (state == NAME){
				if(t.str == "("){
					state = PINS;
				}
				else {
					std::cerr << "Need '(' but found '" << t.str
					<< "' on line " << t.line_no << std::endl;
					return false;
				}
			}
			else if (state == PINS) {
				if (t.type == evl_token::NAME) {

					pin.name= t.str;
					pin.bus_msb = -1;
					pin.bus_lsb = -1;
					state = PIN_NAME;

				}
				else {
					std::cerr << "Need PIN NAME but found '" << t.str
					<< "' on line " << t.line_no << std::endl;
					return false;
				}
			}
			else if (state == PIN_NAME) {
				if (t.str == ",") {
					comp.pins.push_back(pin);
					state = PINS;
				}
				else if (t.str == ")") {
					comp.pins.push_back(pin);
					state = PINS_DONE;
				}
				else if (t.str == "["){
					state = BUS;
				}
				else {
					std::cerr << "Need ',' or ')' or '[' but found '" << t.str
					<< "' on line " << t.line_no << std::endl;
					return false;
				}
			}
			else if (state == BUS){
				if(t.type == evl_token::NUMBER){

					pin.bus_msb = atoi(t.str.c_str());
					//bus_width = atoi(t.str.c_str())+1;
					state = BUS_MSB;
				}
				else {
					std::cerr << "Need NUMBER but found '" << t.str
					<< "' on line " << t.line_no << std::endl;
					return false;
				}
			}
			else if (state == BUS_MSB){
				if(t.str == ":"){
					state = BUS_COLON;
				}
				else if(t.str == "]"){
					state = BUS_DONE;
				}
				else {
					std::cerr << "Need ':' or ']' but found '" << t.str
					<< "' on line " << t.line_no << std::endl;
					return false;
				}
			}
			else if (state == BUS_COLON){
				if(t.type == evl_token::NUMBER){

					pin.bus_lsb = atoi(t.str.c_str());
					state = BUS_LSB;
				}
				else {
					std::cerr << "Need NUMBER but found '" << t.str
					<< "' on line " << t.line_no << std::endl;
					return false;
				}
			}
			else if (state == BUS_LSB){
				if(t.str == "]"){
					state = BUS_DONE;
				}
				else {
					std::cerr << "Need 'wire' but found '" << t.str
					<< "' on line " << t.line_no << std::endl;
					return false;
				}
			}
			else if (state == BUS_DONE){
				if (t.str == ",") {
					comp.pins.push_back(pin);
					state = PINS;
				}
				else if (t.str == ")") {
					comp.pins.push_back(pin);
					state = PINS_DONE;
				}
				else {
					std::cerr << "Need ',' or ')' but found '" << t.str
					<< "' on line " << t.line_no << std::endl;
					return false;
				}
			}
			else if (state == PINS_DONE){
				if(t.str == ";"){
				components.push_back(comp);
				state = DONE;
				}
				else {
					std::cerr << "Need ';' but found '" << t.str
					<< "' on line " << t.line_no << std::endl;
					return false;
				}
			}

	}
	if (!s.tokens.empty() || (state != DONE)) {
		std::cerr << "something wrong with the component statement" << std::endl;
		return false;
	}
	return true;
}

//syntax
bool group_tokens_into_statements(	evl_statements &statements,	evl_tokens &tokens){
	for(; !tokens.empty();){
		//generate one statement per iteration
		evl_token token = tokens.front();

		if (token.str == "module") { // MODULE statement
			evl_statement module;
			module.type = evl_statement::MODULE;

			if (!move_tokens_to_statement(module.tokens, tokens))
				return false;

			statements.push_back(module);
		}

		else if (token.str == "endmodule") { // ENDMODULE statement
			evl_statement endmodule;
			endmodule.type = evl_statement::ENDMODULE;
			endmodule.tokens.push_back(token);
			tokens.erase(tokens.begin());
			statements.push_back(endmodule);
		}

		else if (token.str == "wire") { // WIRE statement
			evl_statement wire;
			wire.type = evl_statement::WIRE;

			if (!move_tokens_to_statement(wire.tokens, tokens))
				return false;
			if(!process_wire_statement(wire.wires, wire))
				return false;

			statements.push_back(wire);
		}

		else { // COMPONENT statement
			evl_statement component;
			component.type = evl_statement::COMPONENT;
			if (!move_tokens_to_statement(component.tokens, tokens))
				return false;
			if(!process_component_statement(component.components, component))
				return false;

			statements.push_back(component);
		}


	}
	return true;
}

void display_statements(std::ostream &out,
	const evl_statements &statements) {
	long comp_count = 0;
	long wire_count = 0;
	evl_statement s;

	for (evl_statements::const_iterator iter = statements.begin();
	iter != statements.end(); ++iter){
		if((*iter).type == evl_statement::MODULE){
			evl_tokens module_tokens=(*iter).tokens;
					for(; !module_tokens.empty();){
						evl_token token = module_tokens.front();
						if(!(token.str==";"))
							out << token.str<<" ";
						module_tokens.pop_front();
					}
					out<<std::endl;

		}
		if((*iter).type == evl_statement::COMPONENT){
			comp_count++;
			for(size_t i=0; i < (*iter).components.size(); ++i){
			s.components.push_back((*iter).components[i]);
			}
		}
		if((*iter).type == evl_statement::WIRE){
			wire_count=wire_count+(*iter).wires.size();
			for(size_t i=0; i < (*iter).wires.size(); ++i){
			s.wires.push_back((*iter).wires[i]);
			}
		}
	}

		out << "wires "<<wire_count<<std::endl;


		for(size_t i=0; i < s.wires.size(); ++i){
				out <<"  wire "<< s.wires[i].name<<" "<<s.wires[i].width<<std::endl;
		}


	out<<"components "<<comp_count<<std::endl;

	for(size_t i=0; i < s.components.size(); ++i){
		if(s.components[i].name==""){
			out <<"  component "<< s.components[i].type
				<<s.components[i].name<<" "<<s.components[i].pins.size()
				<<std::endl;
		}
		else{
			out <<"  component "<< s.components[i].type
				<<" "<<s.components[i].name<<" "<<s.components[i].pins.size()
				<<std::endl;
		}

		for(size_t j=0; j<s.components[i].pins.size(); ++j){

			if(s.components[i].pins[j].bus_lsb!=-1&&s.components[i].pins[j].bus_msb!=-1){
				out<<"    pin "<<s.components[i].pins[j].name<<" "
				   <<s.components[i].pins[j].bus_msb<<" "
				   <<s.components[i].pins[j].bus_lsb<<std::endl;
			}
			else if(s.components[i].pins[j].bus_lsb==-1&&s.components[i].pins[j].bus_msb==-1){
				out<<"    pin "<<s.components[i].pins[j].name<<std::endl;
			}
			else if(s.components[i].pins[j].bus_lsb==-1&&s.components[i].pins[j].bus_msb!=-1){
				out<<"    pin "<<s.components[i].pins[j].name<<" "
				   <<s.components[i].pins[j].bus_msb<<std::endl;
			}
		}
	}
	out << "ENDMODULE" << std::endl;

}

bool store_statements_to_file(std::string file_name, const evl_statements &statements) {
	std::ofstream output_file(file_name.c_str());
	if (!output_file)
    {
        std::cerr << "I can't write " << file_name << ".syntax" << std::endl;
        return -1;
    } // verify output_file is ready
	display_statements(output_file, statements);
	return true;
}

/*
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

    return 0;
}

*/
