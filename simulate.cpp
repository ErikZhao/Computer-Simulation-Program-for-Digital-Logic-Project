#include<math.h>
#include<cmath>
#include "netlist.h"

extern std::string file;

int power_of_2 (int value){
	if(value == 0)
		return 1;
	else if(value == 1)
		return 2;
	else
		return power_of_2(value - 1)*2;
}

std::string hex_to_2(char hex){
	if(hex == '0') return "0000";
	else if(hex == '1') return "0001";
	else if(hex == '2') return "0010";
	else if(hex == '3') return "0011";
	else if(hex == '4') return "0100";
	else if(hex == '5') return "0101";
	else if(hex == '6') return "0110";
	else if(hex == '7') return "0111";
	else if(hex == '8') return "1000";
	else if(hex == '9') return "1001";
	else if((hex == 'A')||(hex == 'a')) return "1010";
	else if((hex == 'B')||(hex == 'b')) return "1011";
	else if((hex == 'C')||(hex == 'c')) return "1100";
	else if((hex == 'D')||(hex == 'd')) return "1101";
	else if((hex == 'E')||(hex == 'e')) return "1110";
	else if((hex == 'F')||(hex == 'f')) return "1111";
	else return "XXXX";
}

bool netlist::simulate(int cycles){

    int i,no;
    int n_outputs;
    int output_pinSize;
	std::map<std::string, net *>::const_iterator it_nets;
	std::vector<net *>::const_iterator it_pin_nets;

    n_outputs = outputs_.size();
    for(no = 0; no < n_outputs; no++){
    	std::string filename = std::string(file) + "." + outputs_[no]->name + ".evl_output";
    	std::ofstream outputfile(filename.c_str(), std::ios::out);

    	output_pinSize = outputs_[no]->pins_.size();
    	outputfile << output_pinSize << std::endl;

    	for(i = 0; i < output_pinSize; i++){
    		outputfile << outputs_[no]->pins_[i]->nets_.size();
    		outputfile << std::endl;
    	}
    	for(i = 0; i < cycles; i++){

    		for(it_nets = nets_table_.begin(); it_nets != nets_table_.end(); ++it_nets){
    			(*it_nets).second->flag = false;
    		}
			for (std::list<net *>::const_iterator n = nets_.begin(); n != nets_.end(); ++n){
				(*n)->get_logic_value();
			}
			outputs_[no]->compute_output();
    	}
    	std::cout << "results saved to " << filename << std::endl;
    }
    return true;
}



int net::get_logic_value(){
	if(flag == true)
		return value;
	else
		return compute();
}


int net::compute(){
	int target = 0;
	std::list<pin *>::iterator it_pin;
	for(it_pin = connections_.begin(); it_pin != connections_.end(); it_pin++){
		if((*it_pin)->type == OUTPUT){

			if((*it_pin)->gate_->compute_output() == false){
				continue;
			}
			else{
				target++;
			}
		}
	}
	if(target == 0){
		set_logic_value(-1);
	}
	return value;
}

//compute flip-flop
bool flip_flop::compute_output(){
	state_ = next_state_;
	pins_[0]->nets_[0]->set_logic_value(state_);
	next_state_ = pins_[1]->nets_[0]->get_logic_value();
	return true;
}

//compute one gate
bool one_gate::compute_output(){
     for (size_t i = 0; i < pins_.size(); i++ ){
         for (size_t j = 0; j < pins_[i]->nets_.size(); j++ ){
             pins_[i]->nets_[j]->set_logic_value( 1 );
         }
     }
     return true;

}


// compute zero gate
bool zero_gate::compute_output() {

    for (size_t i = 0; i < pins_.size(); i++ ){
        for (size_t j = 0; j < pins_[i]->nets_.size(); j++ ){
            pins_[i]->nets_[j]->set_logic_value( 0 );
        }
    }
    return true;
}


// compute not gate
bool not_gate::compute_output() {

      if ( pins_[1]->nets_[0]->get_logic_value() == 1 ){
          pins_[0]->nets_[0]->set_logic_value(0);
      }
      else{
          pins_[0]->nets_[0]->set_logic_value(1);
      }
      return true;
}

// compute and gate
bool and_gate::compute_output(){

	int logic_value = 1;
	for( size_t i= 1; i < pins_.size(); i++){
		if(pins_[i]->nets_[0]->get_logic_value() == 0){
			logic_value = 0;
			break;
		}
	}
	pins_[0]->nets_[0]->set_logic_value(logic_value);
	return true;
}


// compute or gate
bool or_gate::compute_output(){

	int logic_value = 0;
	for( size_t i= 1; i < pins_.size(); i++){
		if(pins_[i]->nets_[0]->get_logic_value() == 1){
			logic_value = 1;
			break;
		}
	}
	pins_[0]->nets_[0]->set_logic_value(logic_value);
	return true;
}


// compute xor gate
bool xor_gate::compute_output() {

      int true_number = 0;
      for (size_t i = 1; i < pins_.size(); i++ ) {
    	  if(pins_[i]->nets_[0]->get_logic_value() == 1)
    	  true_number += 1;
      }
      if ( true_number % 2 == 0 ){
          pins_[0]->nets_[0]->set_logic_value( 0 );
      }
      else{
          pins_[0]->nets_[0]->set_logic_value( 1 );
      }
      return true;
}


//compute output gate
bool output_gate::compute_output(){

	int pin_size;
	int pin_value;
	int width;
	int nets_size;
	int w;

	std::string filename = std::string(file) + "." + name + ".evl_output";
	std::ofstream outputfile(filename.c_str(), std::ios::app);

	pin_size = pins_.size();

	for(int i = 0; i < pin_size; i++){
		width = pins_[i]->nets_.size();
		// width == 1
		if(width == 1){
			pin_value = pins_[i]->nets_[0]->get_logic_value();
			outputfile << pin_value << " ";
		}
		// width != 1
		else{
			pin_value = 0;
			nets_size = pins_[i]->nets_.size();
			w = width/4;
			//std::cout << w << std::endl ;
			if(width%4 != 0){
				w = w+1;
			}
			//int w_=ceil(w);
			outputfile.width(w);
			outputfile.fill('0');
			for(int j = 0; j < nets_size; j++){
				pin_value = pin_value + (int)pins_[i]->nets_[j]->get_logic_value()*power_of_2(j);
				//std::cout << (int)pins_[i]->nets_[j]->get_logic_value() ;
			}
			//std::cout << std::endl;

			outputfile << std::hex << std::uppercase << pin_value<<" ";
		}
	}
	outputfile << std::endl;
	return true;
}


//comput input gate
bool input_gate::compute_output(){

	std::string line;
	std::string num_string;
	std::string::size_type i;
	std::vector<std::string> input_values_;
	int pin_size = pins_.size();
	//std::string filename = std::string(file) + "." + name +".evl_input";

	int pin_index;
	int k;
	size_t j;

	//read first line
	if(number_of_transitions == -1){
		getline(read, line);
		number_of_transitions++;
		if(!read)
			std::cerr<<"error:input file doesn't exist"<<std::endl;
		//check pin number
		for(i = 0; i < line.size();){
			if(isdigit(line[i])){
				size_t num_start = i;
				for(++i; i < line.size(); ++i){
					if(!isdigit(line[i]))
						break;
				}
				num_string = std::string(line,num_start,i-num_start);
				if(pin_size != atoi(num_string.c_str())){
					std::cerr << "pin number is not the same as evl file" << std::endl;
				}
				break;
			}
			else if(isspace(line[i])){
				++i;
				continue;
			}
			else{
				std::cerr << "error : invalid input file" << std::endl;
				i++;
				continue;
			}
		}

		//check pin width
		int index = 0;
		for(;i < line.size() ;){
			if(isspace(line[i])){
				++i;
				continue;
			}
			else if(isdigit(line[i])){
				size_t num_start = i;
				for(++i; i < line.size(); ++i){
					if(!isdigit(line[i]))
						break;
				}
				num_string = std::string(line, num_start, i-num_start);
				int pin_w = pins_[index]->nets_.size();
				if(pin_w != atoi(num_string.c_str())){
					std::cerr << "pin width is not the same as evl file" << std::endl;
				}
				index++;
				continue;
			}
			else{
				std::cerr << "error: invalid input file" << std::endl;
			}
		}
	}

	//read additional line from file
	if(number_of_transitions == 0 && !read.eof()){
		getline(read, line);
		//read number of transition
		for ( i = 0; i < line.size();){
			if(isdigit(line[i])){
				size_t num_start = i;
				for(++i; i < line.size(); ++i){
					if(!isdigit(line[i]))
						break;
				}

				num_string = std::string(line, num_start, i-num_start);
				number_of_transitions = atoi(num_string.c_str());
				break;
			}
			else if(isspace(line[i])){
				++i;
				continue;
			}
			else{
				std::cerr << "error : invalid input file" << std::endl;
				i++;
				continue;
			}
		}

		//read pin value
		for(; i < line.size();){
			if(isspace(line[i])){
				++i;
				continue;
			}

			else if (isdigit(line[i]) || ((line[i] >= 'a') && (line[i] <= 'f')) || ((line[i] >= 'A') && (line[i] <= 'F'))){
				num_string = hex_to_2(line[i]);
				for (++i; i < line.size(); ++i){
					if(!(isdigit(line[i]) || ((line[i] >= 'a') && (line[i] <= 'f')) || ((line[i] >= 'A') && (line[i] <= 'F')))){
						break;
					}
					else{
						num_string = num_string + hex_to_2(line[i]);
					}
				}
				input_values_.push_back(num_string);
				continue;
			}
			else{
				std::cerr << "error : invalid input file" << std::endl;
			}

		}

		//set value
		for (pin_index = 0; pin_index < pin_size; pin_index++){
			int value_size = input_values_[pin_index].size();
			int w_error = pins_[pin_index]->nets_.size() - value_size;
			if(w_error > 0){
				for(int m = 0; m < w_error; m++){
					input_values_[pin_index] = "0" + input_values_[pin_index];

				}
			}
			//std::cout<<input_values_[pin_index]<<std::endl;
			value_size = input_values_[pin_index].size();

			for(j = 0, k = value_size - 1; j < pins_[pin_index]->nets_.size(); j++, k--){
				if(input_values_[pin_index][k] == '0')
					pins_[pin_index]->nets_[j]->set_logic_value(0);
				else
					pins_[pin_index]->nets_[j]->set_logic_value(1);
			}
		}
		if(number_of_transitions > 0)
			number_of_transitions--;
	}

	else{
		for(k = 0; k < pin_size; k++){
			for(j = 0; j < pins_[k]->nets_.size(); j++){
				pins_[k]->nets_[j]->set_logic_value();
			}
		}

		if(number_of_transitions > 0)
			number_of_transitions--;
	}
	return true;
}


//std::vector<std::string> luts_;
//comput lut gate

bool lut_gate::compute_output(){

	std::string line;
	std::string num_string;
	std::string::size_type i;

		//check pin width
	if(first_line_flag == 1){
		getline(read, line);
		first_line_flag = 0;
		if(!read)
		std::cerr<<"error:input file doesn't exist"<<std::endl;

		int index = 0;
		for(i = 0; i < line.size();){
			if(isspace(line[i])){
				++i;
				continue;
			}
			else if(isdigit(line[i])){
				size_t num_start = i;
				for(++i; i < line.size(); ++i){
					if(!isdigit(line[i]))
						break;
				}
				num_string = std::string(line, num_start, i-num_start);
				int pin_w = pins_[index]->nets_.size();
				if(pin_w != atoi(num_string.c_str())){
					std::cerr << "pin width is not the same as evl file" << std::endl;
				}
				index++;
				continue;
			}
			else{
				std::cerr << "error: invalid input file" << std::endl;
			}
		}

		while(!read.eof()){
			std::string lut_string;
			std::string lut_line;
			getline(read, lut_line);
			for(i = 0; i < lut_line.size();){
				if(isspace(lut_line[i])){
					++i;
					continue;
					}
				else if(isdigit(lut_line[i])|| ((lut_line[i] >= 'a') && (lut_line[i] <= 'f')) || ((lut_line[i] >= 'A') && (lut_line[i] <= 'F'))){
					//size_t num_start = i;

					for (; i < lut_line.size(); ++i){
						if(!(isdigit(lut_line[i]) || ((lut_line[i] >= 'a') && (lut_line[i] <= 'f')) || ((lut_line[i] >= 'A') && (lut_line[i] <= 'F')))){
							break;
						}
						else{
							lut_string = lut_string + hex_to_2(lut_line[i]);
						}

					}
					lut_values_.push_back(lut_string);
					continue;
				}
				else{
					std::cerr << "error: invalid input file" << std::endl;
				}
			}

		}


	}

	//read additional line from file
	if(first_line_flag == 0 ){
		int pin_value = 0;
		int nets_size = pins_[1]->nets_.size();

		for(int t = 0; t < nets_size; t++){
			pin_value = pin_value + (int)pins_[1]->nets_[t]->get_logic_value()*power_of_2(t);
		}
		std::string lut_string;
		for(size_t k = 0; k < lut_values_.size(); ++k){
			int k_t = k;
			if (k_t == pin_value){
				int value_size = lut_values_[k_t].size();
				int err = pins_[0]->nets_.size() - value_size;
				if(err > 0){
					for(int t = 0; t < err ; t++)
					lut_values_[k_t] = "0" + lut_values_[k_t];
				}
				value_size = lut_values_[k_t].size();

				for(size_t j = 0 , m = value_size - 1 ; j < pins_[0]->nets_.size() ; j++, m-- ) {
				     if ( lut_values_[k_t][m] == '0' )
				            pins_[0]->nets_[j]->set_logic_value( 0 );

				      else
				            pins_[0]->nets_[j]->set_logic_value( 1 );

			    }
				//std::cout << lut_string << std::endl;
				break;
			}
			else
				continue;
		}

	}
	return true;

}


// compute tris gate
bool tris_gate::compute_output(){

	if (pins_[2]->nets_[0]->get_logic_value() == 0){
		return false;
	}
	else{
		if (pins_[1]->nets_[0]->get_logic_value()==-1){
			return false;
		}
		else{
			if (pins_[1]->nets_[0]->get_logic_value() == 1){
				pins_[0]->nets_[0]->set_logic_value(1);
			}
			else{
				pins_[0]->nets_[0]->set_logic_value(0);
			}
			return true;
		}
	}

}

// compute buf gate
bool buf_gate::compute_output(){

	if (pins_[1]->nets_[0]->get_logic_value()==-1){
		return false;
	}
	else{
		if (pins_[1]->nets_[0]->get_logic_value() == 1){
			pins_[0]->nets_[0]->set_logic_value(1);
		}
		else{
			pins_[0]->nets_[0]->set_logic_value(0);
		}
		return true;
	}
}

//compute clock gate
bool clock_gate::compute_output(){
	for (size_t i = 0; i < pins_.size(); i++){
		for (size_t j = 0; j < pins_[i]->nets_.size(); j++){
			pins_[i]->nets_[j]->set_logic_value(0);
		}
	}
	return true;
}


