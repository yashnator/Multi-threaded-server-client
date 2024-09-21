#include "../utils.hpp"

using namespace std;

string get_ip_address(const struct sockaddr_storage* addr) {
    char ip_str[INET6_ADDRSTRLEN]; 
    if (addr->ss_family == AF_INET) {
        struct sockaddr_in* ipv4 = (struct sockaddr_in*)addr;
		inet_ntop(AF_INET, &(ipv4->sin_addr), ip_str, sizeof(ip_str));
    } else if (addr->ss_family == AF_INET6) {
        struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)addr;
        inet_ntop(AF_INET6, &(ipv6->sin6_addr), ip_str, sizeof(ip_str));
    } else {
        return "Unknown family";
    }
	string ip_adr = string(ip_str);
    return ip_adr; 
}

uint16_t get_port_num(const struct sockaddr_storage* addr) {
    uint16_t port;
    if (addr->ss_family == AF_INET) {
		struct sockaddr_in* ipv4 = (struct sockaddr_in*)addr;
        port = htons(ipv4->sin_port);
    } else if (addr->ss_family == AF_INET6) {
		struct sockaddr_in6* ipv6 = (struct sockaddr_in6*)addr;
        port = htons(ipv6->sin6_port);
    } else {
        return -1;
    }
	return port;
}

string get_next_words(vector<string> &data_to_send, int offset, int words_per_packet) {
	string msg = "";
	int max_offset = min(offset + words_per_packet, (int)data_to_send.size());
	for(int i = offset; i < max_offset; ++i){
		msg = msg + data_to_send[i];
		if(i + 1 != max_offset) msg += ",";
	}
	if(max_offset == (int)data_to_send.size()){
		msg += ",EOF";
	}
	msg += "\n";
	return msg;
}

void main_server_process(int &socketfd) {
	json serverConfig = getServerConfig("config.json");
	string fname = string(serverConfig["input_file"]);
	int words_per_packet = int(serverConfig["p"]);

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
	struct sockaddr_storage 	client_addr;
	socklen_t 					sin_size;
	int 						cnt_bytes, clientfd;
	char 						buffer[MAX_MESSAGE_LEN];
	char 						client_address[INET_ADDRSTRLEN]; 

	sin_size = sizeof(client_addr);
	clientfd = accept(socketfd, (struct sockaddr *)&client_addr, &sin_size);
	if(clientfd == -1) {
		perror("LOG: couldn't accept connection");
		exit(1);
	}
	cout << "LOG: New client connected with ip: " + get_ip_address(&client_addr) + " at port " + to_string(get_port_num(&client_addr)) << endl;
	while(true){
		bool send_completed = false;

		if((cnt_bytes = recv(clientfd, buffer, MAX_MESSAGE_LEN - 1, 0)) == -1){
			perror("LOG: Server did not recieve data");
			exit(1);
		}
		buffer[cnt_bytes] = '\0';
		printf("LOG: server recieved an offset %s\n", buffer);

		int offset = atoi(buffer);

		if(offset > data_to_send.size() || offset < 0) {
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
	int num_threads = int(serverConfig["num_clients"]);

	// Integer vals
	int 						socketfd, everything_OK = 1, recieved;
	struct addrinfo 			hints, *serverinfo, *list;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	
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

	main_server_process(socketfd);

    return 0;
}