#include "utils.hpp"

const std::string invalid_string = "$$\n";
const std::string grumpy_string = "HUH!\n";
const std::string busy_ask = "BUSY?\n";
const std::string idle_reply = "IDLE\n";
const std::string busy_reply = "BUSY\n";

void wait_for_millisec(int num_millisec){
    using namespace std::this_thread;
    using std::chrono::system_clock;
    sleep_for(std::chrono::milliseconds(num_millisec));
}

void wait_for_next_slot(int curr_time, int last_time) {
    int nextslot = last_time / Taloha;
    ++nextslot;
    nextslot *= Taloha;
    if(nextslot - curr_time > 0) {
        wait_for_millisec(nextslot - curr_time);
    }
}

int init_server_socket(std::string ipaddr, std::string portNum) {
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
    int yes = 1;
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
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

	return socketfd;
}

int init_client_socket(std::string portNum) {
    struct addrinfo     hints, *clientinfo, *list; 
    int                 socketfd, recieved; 
    char                serveraddr[INET_ADDRSTRLEN];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if((recieved = getaddrinfo("localhost", portNum.data(), &hints, &clientinfo)) != 0){
        fprintf(stderr, "LOG: couldn't get address info | %s\n", gai_strerror(recieved));
		exit(2);
    }
    list = clientinfo;
    if(list == NULL) {
		perror("LOG: no responses\n");
		exit(1);
	}
    if((socketfd = socket(list->ai_family, list->ai_socktype, list->ai_protocol)) == -1){
        perror("LOG: client couldn't create a socket");
        exit(1);
    }
    if(connect(socketfd, list->ai_addr, list->ai_addrlen) == -1) {
        close(socketfd);
        perror("LOG: client couldn't connect");
        exit(1);
    }

    inet_ntop(list->ai_family, &(((struct sockaddr_in *)list->ai_addr)->sin_addr), serveraddr, sizeof(serveraddr));
    freeaddrinfo(clientinfo);
    printf("LOG: successfully connected to server with ip: %s\n", serveraddr);

    return socketfd;
}

json getServerConfig(std::string fname){
    std::ifstream jsonConfig("config.json");
    json serverConfig = json::parse(jsonConfig);
    return serverConfig;
}

void count_words_and_print_output(std::map<std::string, int> &recieved_string, int suffix_int) {
    std::ofstream file("output_" + std::to_string(suffix_int) + ".txt");
    for(auto [str, freq]: recieved_string) {
        file << str << "," << freq << std::endl;
    }
    file.close();
}

std::string get_ip_address(const struct sockaddr_storage* addr) {
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
	std::string ip_adr = std::string(ip_str);
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

std::string get_next_words(std::vector<std::string> &data_to_send, int offset, int words_per_packet) {
	std::string msg = "";
	int max_offset = std::min(offset + words_per_packet, (int)data_to_send.size());
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