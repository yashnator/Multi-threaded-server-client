#ifndef __UTILS_H__
#define __UTILS_H__

#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <random>
#include <chrono>
#include <time.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;
extern const std::string invalid_string;

#define MAX_BACKLOGS    	    32
#define MAX_MESSAGE_LEN         1024
#define Taloha                  10       

const auto start_time = std::chrono::system_clock::now();
const auto origin_time = std::chrono::duration_cast<std::chrono::milliseconds>(start_time.time_since_epoch()).count();

int init_server_socket(std::string ipaddr, std::string portNum);
int init_client_socket(std::string portNum);

json getServerConfig(std::string fname);

void count_words_and_print_output(std::map<std::string, int> &recieved_string, int suffix_int);
std::string get_next_words(std::vector<std::string> &data_to_send, int offset, int words_per_packet);

std::string get_ip_address(const struct sockaddr_storage* addr);
uint16_t get_port_num(const struct sockaddr_storage* addr);

inline int get_random(int n){ 
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, n);
    return dist(gen);
};
inline int seconds_since_epoch() { 
    auto curr = std::chrono::system_clock::now();
    auto curr_time = std::chrono::duration_cast<std::chrono::milliseconds>(curr.time_since_epoch()).count();
    curr_time -= origin_time;
    int ans = curr_time;
    return ans;
}

#endif