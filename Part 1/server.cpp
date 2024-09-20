#include <fstream>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <nlohmann/json.hpp>

#define MAX_BACKLOGS    	20
#define MAX_MESSAGE_LEN     100

using namespace std;
using json = nlohmann::json;

string invalid_string = "$$";

void sigchild_handler(int child_id){
	int curr_error = errno;
	while(waitpid(-1, NULL, WNOHANG) > 0);
	errno = curr_error;
}

json getServerConfig(string fname){
    std::ifstream jsonConfig("config.json");
    json serverConfig = json::parse(jsonConfig);
    return serverConfig;
}

void *get_address(struct sockaddr *addr){
	return &(((struct sockaddr_in *) addr)->sin_addr);
}

string get_next_words(vector<string> &data_to_send, int offset, int words_per_packet) {
	string msg = "";
	int max_offset = min(offset + words_per_packet - 1, (int)data_to_send.size());
	for(int i = offset - 1; i < max_offset; ++i){
		msg = msg + data_to_send[i];
		if(i + 1 != max_offset) msg += ",";
	}
	msg += "\n";
	return msg;
}

void main_server_process(string &fname, sockaddr_storage &client_addr, char* client_address, int &socketfd, int &clientfd, int words_per_packet) {
	std::ifstream file(fname);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open the file " << fname << std::endl;
        exit(1);
    }
	string file_word;
	vector<string> data_to_send;
	while(getline(file, file_word, ',')) {
		data_to_send.push_back(file_word);
	}
	file.close();

	// Client socket len
	socklen_t 			sin_size;
	int 				cnt_bytes;
	char 				buffer[MAX_MESSAGE_LEN];
	sin_size = sizeof(client_addr);
	clientfd = accept(socketfd, (struct sockaddr *)&client_addr, &sin_size);
	if(clientfd == -1) {
		perror("LOG: couldn't accept connection");
		exit(1);
	}
	inet_ntop(client_addr.ss_family, get_address((struct sockaddr *)&client_addr), client_address, sizeof(client_address));
	printf("LOG: New client connected with ip: %s\n", client_address);
	while(true){
		bool send_completed = false;

		if((cnt_bytes = recv(clientfd, buffer, MAX_MESSAGE_LEN - 1, 0)) == -1){
			perror("LOG: Server did not recieve data");
			exit(1);
		}
		buffer[cnt_bytes] = '\0';
		printf("LOG: server recieved an offset %s\n", buffer);

		int offset = atoi(buffer);
		cout << offset << endl;

		if(offset > data_to_send.size()) {
			if(send(clientfd, invalid_string.data(), invalid_string.length(), 0) == -1){
				perror("LOG: couldn't send message");
				send_completed = true;
			}
		} else{
			string packet_payload = get_next_words(data_to_send, offset, words_per_packet);
			if(send(clientfd, packet_payload.data(), packet_payload.length(), 0) == -1){
				perror("LOG: couldn't send message");
			}
			if((int)data_to_send.size() < offset + words_per_packet){
				send_completed = true;
			}
		}
		if(send_completed){
			close(socketfd);
			close(clientfd);
			break;
		}
	}
}

int main() {
    // Getting server config
    json serverConfig = getServerConfig("config.json");
    string ipaddr = string(serverConfig["server_ip"]);
    string portNum = to_string(int(serverConfig["server_port"]));
	string input_fname = string(serverConfig["input_file"]);
	int words_per_packet = int(serverConfig["p"]);

    // Server starts here

	// Integer vals
	int socketfd, clientfd, everything_OK = 1, recieved;

	// Structs
	struct addrinfo 			hints, *serverinfo, *list;
	struct sockaddr_storage 	client_addr;
	struct sigaction 			sa;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	// Client IPv4 address
	char client_address[INET_ADDRSTRLEN]; // IPv4
	
	if((recieved = getaddrinfo(ipaddr.data(), portNum.data(), &hints, &serverinfo)) != 0) {
		fprintf(stderr, "LOG: couldn't get address info | %s\n", gai_strerror(recieved));
		return 2;
	}
	if(serverinfo == NULL){
		perror("LOG: ssso responses\n");
		exit(1);
	}
	list = serverinfo;
	if(list == NULL) {
		perror("LOG: no responses\n");
		exit(1);
	}
	if((socketfd = socket(list->ai_family, list->ai_socktype, list->ai_protocol)) == -1) {
		fprintf(stderr, "LOG: couldn't create socket\n");
		exit(1);
	}
	if(bind(socketfd, list->ai_addr, list->ai_addrlen) == -1){
		close(socketfd);
		perror("LOG: couldn't bind\n");
		exit(1);
	}
	freeaddrinfo(serverinfo);
	if(listen(socketfd, MAX_BACKLOGS) == -1) {
		perror("LOG: couldn't listen");
		exit(1);
	}

	sa.sa_handler = sigchild_handler;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

	if(sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("LOG: Can't signal");
		exit(1);
	}

	main_server_process(input_fname, client_addr, client_address, socketfd, clientfd, words_per_packet);

    return 0;
}