#include "../utils.hpp"
#include <atomic>

using namespace std;

struct thread_args {
    int thread_id;
	int sockfd;
};

struct server_stats {
	atomic<bool> is_busy;
	atomic<int> currently_serving;
	atomic<int> last_time_served;

	server_stats(): is_busy(false), currently_serving(-1), last_time_served(-1) { }
};

struct server_stats* shared_stats; // Static initialization fiasco?

bool check_collision(int sockid, int sock_start_time) {
	int prev_max;
	do {
		prev_max = shared_stats->last_time_served;
	} while(prev_max < !(shared_stats->last_time_served.compare_exchange_strong(prev_max, sock_start_time)));
	// cout << "Sending huh 1: Busy " << shared_stats->is_busy << " | Serving " << shared_stats->currently_serving << " | " << shared_stats->last_time_served << endl;
	if(shared_stats->is_busy){
		if(shared_stats->currently_serving == sockid){
			if(shared_stats->last_time_served > sock_start_time) {
				// cout << "Collision hogaya guru time se" << endl;
				shared_stats->is_busy = false;
				return false;
			} else{
				// cout << "Crow crow" << endl;
				return true;
			}
		} else{
			// cout << "Collision hogaya guru kis aur se" << endl;
			return false;
		}
	} else{
		bool expecting = false, desired = true;
		if(shared_stats->is_busy.compare_exchange_strong(expecting, desired)){
			// I am using this slot!
			// cout << "milagaya" << endl;
			shared_stats->currently_serving = sockid;
			return true;
		} else{
			return false;
		}
	}
}

bool send_aloha(int sockfd, int tsp) {
	if(check_collision(sockfd, tsp)) return true;
	send(sockfd, grumpy_string.data(), grumpy_string.length(), 0);
	return false;
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

		int tsp = seconds_since_epoch();

		// if(!send_aloha(clientfd, tsp)) continue;
		if((cnt_bytes = recv(clientfd, buffer, MAX_MESSAGE_LEN - 1, 0)) == -1){
			perror("LOG: Server did not recieve data");
			exit(1);
		}
		buffer[cnt_bytes] = '\0';
		// if(!send_aloha(clientfd, tsp)) continue;
		printf("LOG: server recieved an offset %s from the client at PORT: %d\n", buffer, to_string(get_port_num(&client_addr)));

		int offset = atoi(buffer);

		if(offset > data_to_send.size() || offset < 0) {
			if(!send_aloha(clientfd, tsp)) continue;
			if(send(clientfd, invalid_string.data(), invalid_string.length(), 0) == -1){
				perror("LOG: couldn't send message");
				send_completed = true;
			}
			shared_stats->is_busy = false;
		} else{
			string packet_payload = get_next_words(data_to_send, offset, words_per_packet);
			if(!send_aloha(clientfd, tsp)) continue;
			if(send(clientfd, packet_payload.data(), packet_payload.length(), 0) == -1){
				perror("LOG: couldn't send message");
			}
			shared_stats->is_busy = false;
			if((int)data_to_send.size() < offset + words_per_packet){
				send_completed = true;
			}
		}
		if(send_completed){
			shared_stats->is_busy = false;
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

	shared_stats = new server_stats();

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

	close(socketfd);

    return 0;
}