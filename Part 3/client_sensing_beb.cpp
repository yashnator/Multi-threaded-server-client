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
    int                 itr = 0, offset = 0, start_time = seconds_since_epoch();
    int                 last_time = start_time - 1;
    char                buffer[MAX_MESSAGE_LEN];;        
    map<string, int>    word_map, curr_map;

    socketfd = init_client_socket(portNum);
    int backoff_counter = 0;

    while(true){
        bool read_completed = false, successful_slot = true;
        int curr_time = seconds_since_epoch();
        if((last_time / Taloha) == (curr_time / Taloha)) {
            wait_for_next_slot(curr_time, last_time);
        }
        // Sensing code - starts
        if(send(socketfd, busy_ask.data(), busy_ask.length(), 0) == -1){
            perror("LOG: couldn't send message");
        }
        if((cnt_bytes = recv(socketfd, buffer, MAX_MESSAGE_LEN - 1, 0)) == -1){
            perror("LOG: client could not recieve data");
            exit(1);
        }
        if(cnt_bytes <= 0) break;
        buffer[cnt_bytes - 1] = '\0';
        if(string(buffer) == busy_reply) { 
            wait_for_millisec(Taloha);
            continue;
        } 
        // Sensing code - ends
        int rndnum = get_random(num_threads);
        while(rndnum != curr_id){
            curr_time = seconds_since_epoch();
            if(backoff_counter > 0) {
                int bkrnd = get_random(min((1 << backoff_counter) - 1, 30));
                wait_for_millisec(bkrnd * Taloha);
            }
            wait_for_next_slot(curr_time, curr_time);
            rndnum = get_random(num_threads);
        }
        last_time = curr_time;
        string offset_str = (to_string(offset) + "\n").data();
        if(send(socketfd, offset_str.data(), offset_str.length(), 0) == -1){
            perror("LOG: couldn't send message");
        }
        if((cnt_bytes = recv(socketfd, buffer, MAX_MESSAGE_LEN - 1, 0)) == -1){
            perror("LOG: client could not recieve data");
            exit(1);
        }
        if(cnt_bytes <= 0) break;
        buffer[cnt_bytes - 1] = '\0';
        char* curr_word;
        curr_word = strtok(buffer, ",");
        while(curr_word != NULL) {
            string curr_str = string(curr_word);
            if(curr_str == "HUH!"){
                successful_slot = false;
                ++backoff_counter;
                break;
            }
            if(curr_str != "EOF" && curr_str != "$$") {
                string p = string(curr_word);
                ++curr_map[string(curr_word)];
            } else{
                read_completed = true;
                break;
            }
            curr_word = strtok(NULL, ",");
        }
        ++itr;
        if(read_completed) {
            cout << "Client " << curr_id << " completed" << endl;
            break;
        }
        else if(successful_slot) {
            for(auto [s, cnt]: curr_map) word_map[s] += cnt;
            offset += words_per_packet;
            backoff_counter = 0;
        }
        curr_map.clear();
    }
    close(socketfd);
    free(td_args);
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