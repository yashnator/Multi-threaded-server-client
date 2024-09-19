// Client Side program to test
// the TCP server that returns
// a 'hi client' message

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <sstream>

using namespace std;
// PORT number
#define PORT 55555




vector<string> splitMessage(const string &message) {
    vector<string> words;
    string currentWord;
    istringstream stream(message);

    // Iterate through the message and split by ',' or '\n'
    while (getline(stream, currentWord, ',')) {
        // Check if the word contains a newline or 'EOF'
        size_t newlinePos = currentWord.find('\n');
        if (newlinePos != string::npos) {
            // Split at the newline
            string wordBeforeNewline = currentWord.substr(0, newlinePos);
            if (!wordBeforeNewline.empty()) {
                words.push_back(wordBeforeNewline);  // Add the word before '\n'
            }

            // If "EOF" appears before the newline, treat it as a word
            if (wordBeforeNewline == "EOF") {
                break;  // Stop processing as EOF indicates the end of transmission
            }
        } else {
            // Add the word normally if no newline is found
            words.push_back(currentWord);
        }
    }

    return words;
}


int main()
{
	// Socket id
	int clientSocket, ret;

	int k = 5;
	int p = 4;


	// Client socket structure
	struct sockaddr_in cliAddr;

	// char array to store incoming message
	char buffer[1024];

    // Server socket address structures
	struct sockaddr_in serverAddr;

	// Creating socket id
	clientSocket = socket(AF_INET,
						SOCK_STREAM, 0);

	if (clientSocket < 0) {
		printf("Error in connection.\n");
		exit(1);
	}
	printf("Client Socket is created.\n");

	// Initializing socket structure with NULL
	memset(&cliAddr, '\0', sizeof(cliAddr));

	// Initializing buffer array with NULL
	memset(buffer, '\0', sizeof(buffer));

	// Assigning port number and IP address
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);

	// 127.0.0.1 is Loopback IP
	serverAddr.sin_addr.s_addr
		= inet_addr("127.0.0.1");

	// connect() to connect to the server
	ret = connect(clientSocket,
				(struct sockaddr*)&serverAddr,
				sizeof(serverAddr));

	if (ret < 0) {
		printf("Error in connection.\n");
		exit(1);
	}

	printf("Connected to Server.\n");

	map<string, int> wordFreq;

	int cur = 1;
	while(true){
		string message = to_string(cur);
		message+='\n';
		int sbyteCount = send(clientSocket, message.c_str(), message.length(), 0);
		if (sbyteCount < 0) {
			cout << "Client send error: " <<  endl;
			return -1;
		}
		else {
			cout << "Client: Sent " << sbyteCount << " bytes" << endl;
		}

		if (sbyteCount < 0) {
			cout << "Client send error: " << endl;
			return -1;
		}
		char receiveBuffer[1024];

		int rbyteCount = recv(clientSocket, receiveBuffer, 1024, 0);

		receiveBuffer[rbyteCount] = '\0';

		// Convert the buffer to string
		string receivedMessage(receiveBuffer);
		vector<string> newwords = splitMessage(receivedMessage);

		if (rbyteCount < 0) {
			cout << "Client recv error: "  << endl;
			return 0;
		} else {
			cout << "Client: Received data: " << receivedMessage << endl;
		}

		bool done = false;
		for(auto word: newwords){
			if(word == "EOF" || word == "$$"){
				done = true;
				break;
			}
			wordFreq[word]++;
		}
		if(done)break;
		cur+=k;
	}

	for(auto p: wordFreq){
		cout<<p.first<<" "<<p.second<<endl;
	}

	return 0;
}
