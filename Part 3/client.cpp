#include "../utils.hpp"

using namespace std;

struct thread_args {
    int thread_id;
};

void *client_thread(void* td_args) { 
    json serverConfig = getServerConfig("config.json");
    string portNum = to_string(int(serverConfig["server_port"]));
    int num_threads = int(serverConfig["num_clients"]);
    int words_per_packet = int(serverConfig["p"]);

    thread_args* args = static_cast<thread_args*>(td_args);
    int                 socketfd, cnt_bytes, curr_id = args->thread_id;
    int                 itr = 0, offset = 0, start_time = seconds_since_epoch() / Taloha;
    int                 last_time = start_time - 1;
    char                buffer[MAX_MESSAGE_LEN];;        
    map<string, int>    word_map;

    socketfd = init_client_socket(portNum);
    cout << "id " << curr_id << endl;

    while(true){
        bool read_completed = false, successful_slot = true;
        int curr_time = seconds_since_epoch() / Taloha;
        while(last_time == curr_time) curr_time = seconds_since_epoch() / Taloha;
        int rndnum = get_random(num_threads);
        while(rndnum != curr_id){
            last_time = curr_time;
            cout << "LOG | Client " << curr_id << " Didn't send in slot " << curr_time - start_time << " " << rndnum << endl;
            while(last_time == curr_time) curr_time = seconds_since_epoch() / Taloha;
            rndnum = get_random(num_threads);
        }
        last_time = curr_time;
        string offset_str = to_string(offset).data();
        if(send(socketfd, offset_str.data(), offset_str.length(), 0) == -1){
            perror("LOG: couldn't send message");
        }
        if((cnt_bytes = recv(socketfd, buffer, MAX_MESSAGE_LEN - 1, 0)) == -1){
            perror("LOG: client could not recieve data");
            exit(1);
        }
        buffer[cnt_bytes - 1] = '\0';

        cout << "LOG | Client " << args->thread_id << " buffer: " << buffer << endl;

        char* curr_word;
        curr_word = strtok(buffer, ",");
        while(curr_word != NULL) {
            string curr_str = string(curr_word);
            if(curr_str == "HUH"){
                successful_slot = false;
                break;
            }
            if(curr_str != "EOF" && curr_str != "$$") {
                ++word_map[string(curr_word)];
            } else{
                read_completed = true;
                break;
            }
            curr_word = strtok(NULL, ",");
        }

        printf("LOG | Client %d | Iteration %d: client recieved new message\n", args->thread_id, itr);
        ++itr;
        if(read_completed) break;
        else if(successful_slot) offset += words_per_packet;
        else continue;
    }
    close(socketfd);
    free(td_args);
    free(args);
    count_words_and_print_output(word_map, curr_id);
    pthread_exit(NULL);
}

int main() {
    // Getting server config
    json serverConfig = getServerConfig("config.json");
    int num_threads = int(serverConfig["num_clients"]);

    pthread_t th[num_threads];
    for(int i = 0; i < num_threads; ++i){
        if(pthread_create(&th[i], NULL, &client_thread, new thread_args{i + 1}) != 0){
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