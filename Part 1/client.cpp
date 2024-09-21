#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <nlohmann/json.hpp>

#define MAX_MESSAGE_LEN     100

using namespace std;
using json = nlohmann::json;

json getServerConfig(string fname){
    std::ifstream jsonConfig("config.json");
    json serverConfig = json::parse(jsonConfig);
    return serverConfig;
}

void count_words_and_print_output(map<string, int> &recieved_string) {
    ofstream file("output.txt");
    if (!file.is_open()) {
        std::cerr << "Error: Could not open the file for writing" << std::endl;
        exit(1);
    }
    for(auto &[str, freq]: recieved_string) {
        file << str << "," << freq << endl;
    }
    file.close();
}

void main_client_process(int socketfd, int words_per_packet) { 
    int                 cnt_bytes = 0, itr = 0, offset = 1;
    char                buffer[100];
    map<string, int>    word_map;

    while(true){
        bool read_completed = false;
        string offset_str = to_string(offset).data();
        if(send(socketfd, offset_str.data(), offset_str.length(), 0) == -1){
            perror("LOG: couldn't send message");
        }
        if((cnt_bytes = recv(socketfd, buffer, MAX_MESSAGE_LEN - 1, 0)) == -1){
            perror("LOG: client cannot recieve data");
            exit(1);
        }
        buffer[cnt_bytes - 1] = '\0';

        char* curr_word;
        curr_word = strtok(buffer, ",");
        while(curr_word != NULL) {
            string curr_str = string(curr_word);
            if(curr_str != "EOF" && curr_str != "$$") {
                ++word_map[string(curr_word)];
            } else{
                read_completed = true;
                break;
            }
            curr_word = strtok(NULL, ",");
        }

        printf("LOG | Iteration %d: client recieved:\"%s\"\n", itr, buffer);
        if(!read_completed) offset += words_per_packet;
        else break;
    }
    count_words_and_print_output(word_map);
}

int main() {
    // Getting server config
    json serverConfig = getServerConfig("config.json");
    string portNum = to_string(int(serverConfig["server_port"]));
    int words_per_packet = int(serverConfig["p"]);
    
    int                 socketfd, cnt_bytes, recieved;
    char                serveraddr[INET_ADDRSTRLEN];
    struct addrinfo     hints, *clientinfo, *list;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if((recieved = getaddrinfo("localhost", portNum.data(), &hints, &clientinfo)) != 0){
        fprintf(stderr, "LOG: couldn't get address info | %s\n", gai_strerror(recieved));
		return 2;
    }
    list = clientinfo;
    if(list == NULL) {
		perror("LOG: no responses\n");
		exit(1);
	}
    if((socketfd = socket(list->ai_family, list->ai_socktype, list->ai_protocol)) == -1){
        perror("LOG: client couldn't create a socket");
        return 1;
    }
    if(connect(socketfd, list->ai_addr, list->ai_addrlen) == -1) {
        close(socketfd);
        perror("LOG: client couldn't connect");
        return 1;
    }

    inet_ntop(list->ai_family, &(((struct sockaddr_in *)list->ai_addr)->sin_addr), serveraddr, sizeof(serveraddr));
    freeaddrinfo(clientinfo);
    printf("LOG: successfully connected to server with ip: %s\n", serveraddr);

    main_client_process(socketfd, words_per_packet);

    close(socketfd);

    return 0;
}