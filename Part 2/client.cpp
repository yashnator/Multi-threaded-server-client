#include "../utils.hpp"

using namespace std;

struct thread_args {
    int thread_id;
    int wpp;
};

void *client_thread(void* td_args) { 
    json serverConfig = getServerConfig("config.json");
    string portNum = to_string(int(serverConfig["server_port"]));

    thread_args* args = static_cast<thread_args*>(td_args);
    int                 socketfd, cnt_bytes, itr = 0, offset = 0, words_per_packet = args->wpp;;
    char                buffer[MAX_MESSAGE_LEN];;        
    map<string, int>    word_map;

    socketfd = init_client_socket(portNum);

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

        if(cnt_bytes == 0) break;

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
    close(socketfd);
    count_words_and_print_output(word_map, args->thread_id);
    pthread_exit(NULL);
}

int main() {
    // Getting server config
    json serverConfig = getServerConfig("config.json");
    int words_per_packet = int(serverConfig["p"]);
    int num_threads = int(serverConfig["num_clients"]);

    pthread_t th[num_threads];
    for(int i = 0; i < num_threads; ++i){
        if(pthread_create(&th[i], NULL, &client_thread, new thread_args{i + 1, words_per_packet}) != 0){
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