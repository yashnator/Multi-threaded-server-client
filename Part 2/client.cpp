#include "../utils.hpp"

using namespace std;

struct thread_args {
    int thread_id;
    int sockfd;
    int wpp;
};

void *client_thread(void* td_args) { 
    thread_args* args = (thread_args*)td_args;
    int                 cnt_bytes = 0, itr = 0, offset = 0, socketfd = args->sockfd, words_per_packet = args->wpp;
    char                buffer[100];
    map<string, int>    word_map;

    while(true){
        bool read_completed = false;
        string offset_str = to_string(offset).data();
        if(send(socketfd, offset_str.data(), offset_str.length(), 0) == -1){
            perror("LOG: couldn't send message");
        }
        if((cnt_bytes = recv(socketfd, buffer, MAX_MESSAGE_LEN - 1, 0)) == -1){
            perror("LOG: client could not recieve data");
            exit(1);
        }
        buffer[cnt_bytes - 1] = '\0';

        cout << "LOG | Client buffer: " << buffer << endl;

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

        printf("LOG | Iteration %d: client recieved new message\n", itr);
        ++itr;
        if(!read_completed) offset += words_per_packet;
        else break;
    }
    count_words_and_print_output(word_map, args->thread_id);
    pthread_exit(NULL);
}

int main() {
    // Getting server config
    json serverConfig = getServerConfig("config.json");
    string portNum = to_string(int(serverConfig["server_port"]));
    int words_per_packet = int(serverConfig["p"]);
    int num_threads = int(serverConfig["num_clients"]);
    
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

    pthread_t th[num_threads];
    for(int i = 0; i < num_threads; ++i){
        thread_args td_args;
        td_args.thread_id = i + 1;
        td_args.sockfd = socketfd;
        td_args.wpp = words_per_packet;
        if(pthread_create(&th[i], NULL, &client_thread, (void *)&td_args) != 0){
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