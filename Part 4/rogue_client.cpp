#include "../utils.hpp"

using namespace std;

struct thread_args {
    int thread_id;
    int wpp;
};

int num_rogue_reqs;

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
        string offset_str = to_string(offset) + "\n";
        if(send(socketfd, offset_str.data(), offset_str.length(), 0) == -1){
            perror("LOG: couldn't send message");
        }
        if((cnt_bytes = recv(socketfd, buffer, MAX_MESSAGE_LEN - 1, 0)) == -1){
            perror("LOG: client could not recieve data");
            exit(1);
        }
        if(cnt_bytes == 0) break;
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
        ++itr;
        if(!read_completed) offset += words_per_packet;
        else break;
    }
    close(socketfd);
    count_words_and_print_output(word_map, args->thread_id);
    pthread_exit(NULL);
}

void *rouge_client_thread(void* td_args) { 
    json serverConfig = getServerConfig("config.json");
    string portNum = to_string(int(serverConfig["server_port"]));

    thread_args* args = static_cast<thread_args*>(td_args);
    int                 socketfd, cnt_bytes, itr = 0, offset = 0, words_per_packet = args->wpp;;
    char                buffer[MAX_MESSAGE_LEN];;        
    map<string, int>    word_map;

    socketfd = init_client_socket(portNum);

    auto send_function = [&](int offset) {
        string offset_str = to_string(offset) + "\n";
        if(send(socketfd, offset_str.data(), offset_str.length(), 0) == -1){
            perror("LOG: couldn't send message");
        }
    };

    string offset_str = to_string(offset) + "\n";
        if(send(socketfd, offset_str.data(), offset_str.length(), 0) == -1){
            perror("LOG: couldn't send message");
        }

    while(true){
        bool read_completed = false;
        
        for(int rogue_cnt = 0; )
        send_function(offset);

        if((cnt_bytes = recv(socketfd, buffer, MAX_MESSAGE_LEN - 1, 0)) == -1){
            perror("LOG: client could not recieve data");
            exit(1);
        }
        if(cnt_bytes == 0) break;
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
    int num_rogue_clients = 1;
    num_rogue_reqs = 5;

    pthread_t th[num_threads];
    for(int i = 0; i < num_threads; ++i){
        if(i < num_rogue_clients){
            if(pthread_create(&th[i], NULL, &rouge_client_thread, new thread_args{i + 1, words_per_packet}) != 0){
                perror("LOG: Couldn't create thread");
            }
        } else{
            if(pthread_create(&th[i], NULL, &client_thread, new thread_args{i + 1, words_per_packet}) != 0){
                perror("LOG: Couldn't create thread");
            }
        }
    }
    for(int i = 0; i < num_threads; ++i){
        if(pthread_join(th[i], NULL) != 0){
            perror("LOG: Couldn't close thread");
        }
    }

    return 0;
}