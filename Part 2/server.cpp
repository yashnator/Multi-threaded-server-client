#include "../utils.hpp"

using namespace std;

struct thread_args {
    int thread_id;
	int sockfd;
};

void *server_thread(void* td_args) {
	json serverConfig = getServerConfig("config.json");
	string fname = string(serverConfig["input_file"]);
	int words_per_packet = int(serverConfig["p"]);

	thread_args* args = (thread_args*)td_args;
	int socketfd = args->sockfd;

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
			close(clientfd);
			break;
		}
	}
	close(socketfd);
	pthread_exit(NULL);
}

int main() {
    // Getting server config
    json serverConfig = getServerConfig("config.json");
    string ipaddr = string(serverConfig["server_ip"]);
    string portNum = to_string(int(serverConfig["server_port"]));
	int num_threads = int(serverConfig["num_clients"]);

	int socketfd = init_server_socket(ipaddr, portNum);

	pthread_t th[num_threads];
	for(int i = 0; i < num_threads; ++i){
        thread_args td_args{i, socketfd};
        if(pthread_create(&th[i], NULL, &server_thread, (void *)&td_args) != 0){
            perror("LOG: Couldn't create thread");
        }
    }
    for(int i = 0; i < num_threads; ++i){
        if(pthread_join(th[i], NULL) != 0){
            perror("LOG: Couldn't close thread");
        }
    }

    return 0;
}