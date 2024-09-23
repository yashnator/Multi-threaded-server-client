#include "../utils.hpp"

using namespace std;

int num_trd = 1;

vector<double> completion_times;

struct thread_args {
    int thread_id;
};

void *client_thread(void* td_args) { 
    json serverConfig = getServerConfig("config.json");
    string portNum = to_string(int(serverConfig["server_port"]));
    int num_threads = int(serverConfig["num_clients"]);
    int k = int(serverConfig["k"]);
    int p = int(serverConfig["p"]);

    thread_args* args = static_cast<thread_args*>(td_args);
    int                 socketfd, cnt_bytes, curr_id = args->thread_id;
    int                 itr = 0, offset = 0, start_time = seconds_since_epoch();
    int                 last_time = start_time - 1;
    char                buffer[MAX_MESSAGE_LEN];;        
    map<string, int>    word_map, curr_map;
    auto start = std::chrono::high_resolution_clock::now();

    socketfd = init_client_socket(portNum);

    while(true){
        bool read_completed = false, successful_slot = true;
        int curr_time = seconds_since_epoch();
        if((last_time / Taloha) == (curr_time / Taloha)) {
            wait_for_next_slot(curr_time, last_time); 
        }
        int rndnum = get_random(num_threads);
        while(rndnum != curr_id){
            curr_time = seconds_since_epoch();
            wait_for_next_slot(curr_time, curr_time); 
            rndnum = get_random(num_threads);
        }
        last_time = curr_time;

        string offset_str = to_string(offset).data();
        if(send(socketfd, offset_str.data(), offset_str.length(), 0) == -1){
            perror("LOG: couldn't send message");
        }
        
        int num_pkts = (k + p - 1) / p;
        while(num_pkts){
            if((cnt_bytes = recv(socketfd, buffer, MAX_MESSAGE_LEN - 1, 0)) == -1){
                perror("LOG: client could not recieve data");
                exit(1);
            }
            if(cnt_bytes <= 0) {
                read_completed = true;
                break;
            }
            buffer[cnt_bytes] = '\0';
            string pkt_str = string(buffer);
            string curr_word = "";
            for(int i = 0; i < pkt_str.length(); ++i){
                if(pkt_str[i] == ',' || pkt_str[i] == '\n'){
                    if(curr_word == "HUH!"){
                        successful_slot = false;
                        break;
                    }
                    if(curr_word == "$$" || curr_word == "EOF"){
                        break;
                    }
                    ++curr_map[curr_word];
                    curr_word = "";
                } else{
                    curr_word.push_back(pkt_str[i]);
                }
                if(pkt_str[i] == '\n') --num_pkts;
            }
            if(curr_word.length() > 0) ++curr_map[curr_word];
            if(read_completed || !successful_slot) break;
        }

        ++itr;
        if(read_completed) {
            break;
        }
        else if(successful_slot) {
            for(auto [s, cnt]: curr_map) word_map[s] += cnt;
            offset += k;
        }
        curr_map.clear();
    }

    auto end = std::chrono::high_resolution_clock::now();
    chrono::duration<double> diff = end - start;
    double completion_time = diff.count();
    completion_time*=1000;

    string str_to_write = to_string(num_trd) + ":" + to_string(completion_time) + " ";

    std::ofstream outfile;
    outfile.open("stats.txt", ios_base::app);
    if (outfile.is_open()) {
        outfile << str_to_write;
        outfile.close();
    } else {
        std::cerr << "Error: Could not open stats.txt file." << endl;
    }
    completion_times.push_back(completion_time);
    close(socketfd);
    free(td_args);
    count_words_and_print_output(word_map, curr_id);
    pthread_exit(NULL);
}

int main() {
    // Getting server config
    json serverConfig = getServerConfig("config.json");
    int num_threads = int(serverConfig["num_clients"]);
    num_trd = num_threads;
    Taloha = int(serverConfig["T"]);

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

    cout << "Average time: " << (accumulate(completion_times.begin(), completion_times.end(), 0.0) / (num_threads * 1.0)) << endl;

    return 0;
}