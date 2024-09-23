#include "../utils.hpp"

using namespace std;

struct thread_args {
    int thread_id;
    int wpp;
};

void *client_thread(void* td_args) { 
    json serverConfig = getServerConfig("config.json");
    string portNum = to_string(int(serverConfig["server_port"]));
    auto start = std::chrono::high_resolution_clock::now();
    int k = int(serverConfig["k"]);
    int p = int(serverConfig["p"]);

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
        // cout << "LOG | Client buffer: " << buffer << endl;

        int num_pkts = (k + p - 1) / p;
        while(num_pkts){
            if((cnt_bytes = recv(socketfd, buffer, MAX_MESSAGE_LEN - 1, 0)) == -1){
                perror("LOG: client could not recieve data");
                exit(1);
            }
            if(cnt_bytes <= 0) break;
            buffer[cnt_bytes] = '\0';
            string pkt_str = string(buffer);
            string curr_word = "";
            for(int i = pkt_str.length() - 1; i >= 0; --i){
                if(pkt_str[i] == '\n'){
                    --num_pkts;
                } else if(pkt_str[i] == ','){
                    reverse(curr_word.begin(), curr_word.end());
                    if(curr_word == "EOF" || curr_word == "$$"){
                        read_completed = true;
                    }
                    ++word_map[curr_word];
                } else{
                    curr_word.push_back(pkt_str[i]);
                }
                pkt_str.pop_back();
            }
            if(curr_word.length() > 0) ++word_map[curr_word];
            if(read_completed) break;
        }

        printf("LOG | Iteration %d: client recieved new message\n", itr);
        ++itr;
        if(!read_completed) offset += k;
        else break;
    }

    auto end = std::chrono::high_resolution_clock::now();
    chrono::duration<double> diff = end - start;
    double completion_time = diff.count();
    cout << "Completion time: " << completion_time << " seconds" << endl;

    string str_to_write = to_string(completion_time) + " ";

    std::ofstream outfile;
    outfile.open("stats.txt", ios_base::app);
    cout<<"comp time: "<<completion_time<<endl;
    if (outfile.is_open()) {
        outfile << str_to_write;
        outfile.close();
    } else {
        std::cerr << "Error: Could not open stats.txt file." << endl;
    }
    count_words_and_print_output(word_map, args->thread_id);

    close(socketfd);

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