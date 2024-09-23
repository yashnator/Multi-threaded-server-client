#include "../utils.hpp"
#include <queue>

using namespace std;

priority_queue<int, vector<int>, greater<int>> pq;
pthread_mutex_t queue_lock, request_lock;

struct thread_args {
    int thread_id;
	int sockfd;
};

void add_to_fifo(int tsp) {
	pthread_mutex_lock(&queue_lock);
	pq.push(tsp);
	pthread_mutex_unlock(&queue_lock);
}

void pop_from_fifo() {
	pthread_mutex_lock(&queue_lock);
	pq.pop();
	pthread_mutex_unlock(&queue_lock);
}

void send_handler(vector<string> &data_to_send, int clientfd, int offset, bool &send_completed, int words_per_packet) {
	if(offset >= data_to_send.size() || offset < 0) {
		if(send(clientfd, invalid_string.data(), invalid_string.length(), 0) == -1){
			perror("LOG: couldn't send message");
			send_completed = true;
		}
	} else{
		string packet_payload = get_next_words(data_to_send, offset, words_per_packet);
		if(send(clientfd, packet_payload.data(), packet_payload.length(), 0) == -1){
			perror("LOG: couldn't send message");
		}
		if((int)data_to_send.size() <= offset + words_per_packet){
			send_completed = true;
		}
	}
}

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
		if(cnt_bytes <= 0) break;
		buffer[cnt_bytes - 1] = '\0';

		int offset = atoi(buffer);

		// Add to fifo and wait for your turn!
		int recv_tsp = seconds_since_epoch();
		add_to_fifo(recv_tsp);
		while(recv_tsp != pq.top());
		pop_from_fifo();

		// Lock for send
		pthread_mutex_lock(&request_lock);
		send_handler(data_to_send, clientfd, offset, send_completed, words_per_packet);
		pthread_mutex_unlock(&request_lock);

		if(send_completed){
			close(clientfd);
			break;
		}
	}
	pthread_exit(NULL);
}

int main() {
    // Getting server config
    json serverConfig = getServerConfig("config.json");
    string ipaddr = string(serverConfig["server_ip"]);
    string portNum = to_string(int(serverConfig["server_port"]));
	int num_threads = int(serverConfig["num_clients"]);

	int socketfd = init_server_socket(ipaddr, portNum);

	if((pthread_mutex_init(&queue_lock, NULL) != 0) || (pthread_mutex_init(&request_lock, NULL) != 0)){
		perror("Mutex create failed");
		return 1;
	}

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

	if((pthread_mutex_destroy(&queue_lock) != 0 )|| (pthread_mutex_destroy(&request_lock) != 0)){
		perror("Mutex destroy failed");
		return 1;
	}

	close(socketfd);

    return 0;
}