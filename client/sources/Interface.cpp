#include "../headers/Interface.h"
// #include "../headers/Load.h"
// #include "../headers/Dump.h"
#include <cstring>
#include <string>


//default constructor just creates new objects
Interface::Interface(){
	std::cout << "Please enter my address:";
	std::string p_address;
	std::cin >> p_address;
	const char * p_address_char = p_address.c_str();

	std::cout << "Please enter the port of the server:";
	int p_port;
	std::cin >> p_port;

	mySocket = new TCPSocket(p_address_char, (int)p_port);

	// std::cout << "Server addr: " << mySocket->getRemoteAddress() << '\n';  // Selector returning remote IP address
 //    std::cout << "Cleint addr: " << mySocket->getMyAddress() << '\n';      // Selector returning local IP address
}



void Interface::print_query(std::vector<std::vector<std::string>> p_set){
	for(int i = 1; i < p_set[0].size(); ++i){
		for (int j = 0; j < p_set.size(); ++j){
			if(j == p_set.size()-1){
			std::cout << p_set[j][0] << ":" << p_set[j][i];
			}else{
			std::cout << p_set[j][0] << ":" << p_set[j][i] << ", ";
			}
		}
		std::cout << '\n';
	}
}
int Interface::argSize(char * p_arg, int split){
	int i;
	int a;
	int targets;
	if (split == 0){
		for(i = 0; i < 4; i++){
			a = p_arg[i] - '0';
			if (i==0){
				targets = a;
			}
			else {
				targets = (targets * 10) + a;
			}
		}
		return targets;
	}
	else{
		for(i = 4; i < 8; i++){
			a = p_arg[i] - '0';
			if (i == 4) {
				targets = a;
			}
			else {
				targets = (targets * 10) + a;
			}
			
		}
		return targets;
	}
}

std::vector<std::vector<std::string>> Interface::parseSeg(string target, int targetNum, int argNum) {
        std::vector<std::vector<std::string>> result;
        int c;
        for (c = 0; c < targetNum; c++){
            std::vector<std::string> segment;
            auto rName = target.substr(0, target.find('|'));
            int count = 0;
            segment.push_back(rName);
            target.erase(0, target.find('|') + 1);
            auto rParams = target.substr(0, target.find('$'));
            size_t pos = 0;
            std::string token;
            while ((pos = rParams.find('|')) != std::string::npos) {
                    token = rParams.substr(0,pos);
                    segment.push_back(token);
                    rParams.erase(0, pos + 1);
                    count++;
                    if (count == argNum){
                        result.push_back(segment);
                    }
            }
        
            target.erase(0, target.find('$'));
            if(!rParams.empty()) segment.push_back(rParams);
        }
        return result;
}

string Interface::convertToStr(char * c_arg) {
	string result;
	int i = 8;
	while(c_arg[i]!=NULL) {
		result += c_arg[i];
		i++;
	}
	return result;
}

void Interface::unformat(char * p_arg){
	//get num of targets and elements
	int targets = argSize(p_arg,0);
	int elements = argSize(p_arg,1);
	//convert char * to string starting after numbers
	string arg = convertToStr(p_arg);
	std::vector<std::vector<std::string>> unformated = parseSeg(arg,targets,elements);
	print_query(unformated);
	
}


void Interface::commandLine(){


	while(1){

		std::string statement;
		std::getline(std::cin, statement);
		if(statement.length() > 0){
			if(statement.substr(0,4) == "LOAD"){
				std::string pathName = statement.substr(5,std::string::npos);

				// Grabs the path type of the file
				std::string pathCheck = pathName.substr(pathName.rfind("."));
				
				// Errors if incorrect path type
				if(pathCheck != ".sri") {
					std::cout << "Invalid file. Please use a .sri file.\n";
					return;
				}

				std::ifstream file(pathName);
				if (file.is_open()){
					std::string line;
					while(std::getline(file, line)){
						// Send each line to be parsed by server
						const char * p_line = line.c_str();
						mySocket->writeToSocket(p_line, 50);

						char * temp_buffer;
						temp_buffer = (char*)calloc(1000, sizeof(char));

						if(mySocket->readFromSocket(temp_buffer, 1000) == -1){
							std::cout << "Reading from clientSocket Failed\n";
							mySocket->setPeerDisconnected(true);
							return;
						}
						if(strcmp((const char *)temp_buffer,"null") != 0){
							break;
						}
					}
				} else {
					std::cout << "Error opening file" << std::endl;
				}
			} else if(statement.substr(0,4) == "DUMP") {
				const char * p_statement = statement.c_str();
				mySocket->writeToSocket(p_statement, 50);

				

				std::string pathName = statement.substr(6,std::string::npos);

				// Grabs the path type of the file
				std::string pathCheck = pathName.substr(pathName.rfind("."));
				
				// Errors if incorrect path type
				if(pathCheck != ".sri") {
					std::cout << "Invalid file. Please use a .sri file.\n";
					return;
				}				

				std::ofstream outFile(pathName);
				if(outFile.is_open()){
					// if buffer recieves null termainated that means DUMP is done
					while(temp_buffer[0] == '\0') {
						char * temp_buffer;
						temp_buffer = (char*)calloc(1000, sizeof(char));

						if(mySocket->readFromSocket(temp_buffer, 1000) == -1){
							std::cout << "Reading from clientSocket Failed\n";
							mySocket->setPeerDisconnected(true);
							return;
						}
						// Unformat buffer? Then recieve each line
						std::string bufferString;
						int i = 0; 
						while(temp_buffer[i] != '\0') {
							bufferString += temp_buffer[i];
							i++;
						}

						outFile << bufferString << std::endl;
						std::cout << "This was read from the server: " << temp_buffer << std::endl;


						char * buffer =(char *) calloc(4, sizeof(char));
						buffer[0] = 'n'; buffer[1] = 'u'; buffer[2] = 'l'; buffer[3] = 'l';
						mySocket->writeToSocket(buffer, 50);
						// Send response here asking for more
					}
					// Buffer finished
					outFile.close();
				} else {
					std::cout << "Error opening file" << std::endl;
				}
			} else {
				const char * p_statement = statement.c_str();
				mySocket->writeToSocket(p_statement, 50);
				if (statement == "x") {
					delete(mySocket);
					break;
				}

				char * temp_buffer;
				temp_buffer = (char*)calloc(1000, sizeof(char));

				if(mySocket->readFromSocket(temp_buffer, 1000) == -1){
					std::cout << "Reading from clientSocket Failed\n";
					mySocket->setPeerDisconnected(true);
					return;
				}
				if(strcmp((const char *)temp_buffer,"null") != 0){
					unformat(temp_buffer);
				}
			}
		}
		std::cout << "SRI Session:";
	}
}

Interface::~Interface(){
	delete(mySocket);
}

