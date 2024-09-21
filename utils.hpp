#ifndef __UTILS_H__
#define __UTILS_H__

#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::string invalid_string = "$$";

#define MAX_BACKLOGS    	    32
#define MAX_MESSAGE_LEN         1024

json getServerConfig(std::string fname){
    std::ifstream jsonConfig("config.json");
    json serverConfig = json::parse(jsonConfig);
    return serverConfig;
}

void count_words_and_print_output(std::map<std::string, int> &recieved_string, int suffix_int) {
    std::ofstream file("output_" + std::to_string(suffix_int) + ".txt");
    if (!file.is_open()) {
        std::cerr << "Error: Could not open the file for writing" << std::endl;
        exit(1);
    }
    for(auto &[str, freq]: recieved_string) {
        file << str << "," << freq << std::endl;
    }
    file.close();
}

#endif