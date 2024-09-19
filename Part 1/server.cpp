// Server side program that sends
// a 'hi client' message
// to every client concurrently

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
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <sstream>

// PORT number
#define PORT 55555



std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> words;
    std::string word;
    std::istringstream stream(str);

    while (std::getline(stream, word, delimiter)) {
        words.push_back(word);  // Add each word to the vector
    }
    return words;
}

// Function to read words from the memory-mapped file "name.txt"
std::vector<std::string> readWordsFromFile(const std::string& filename) {
    std::ifstream file(filename);  // Open the file for reading
    std::vector<std::string> words;

    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            // Split the line into words by comma
            std::vector<std::string> fileWords = split(line, ',');
            words.insert(words.end(), fileWords.begin(), fileWords.end());
        }
        file.close();
    } else {
        std::cerr << "Error opening file: " << filename << std::endl;
    }

    return words;
}

// Function to send words in packets of size p
void sendWordsInPackets(int clientSocket, const std::vector<std::string>& words, int offset, int k, int p) {
    int wordsSent = 0;  // Keep track of how many words have been sent

    for (int i = offset; i < offset + k && i < words.size(); i += p) {
        std::string packet;

        // Form a packet of size p words
		int cur = 0;
        for (int j = 0; j < p && (i + j) < words.size() && wordsSent < k; ++j) {
			cur++;
            packet += words[i + j] + ",";  // Append each word followed by newline
            wordsSent++;
        }
		if(cur < p && wordsSent < k){
			packet+="EOF";
		}
		packet+='\n';
		std::cout<<packet<<std::endl;

        // Send the packet to the client
        send(clientSocket, packet.c_str(), packet.length(), 0);
    }

    std::cout << "Sent " << wordsSent << " words in packets of " << p << " words." << std::endl;
}





int extractOffset(const std::string& input) {
    // Find the position of the newline character '\n'
    size_t newlinePos = input.find('\n');

    if (newlinePos != std::string::npos) {
        // Extract the substring before '\n'
        std::string offsetStr = input.substr(0, newlinePos);

        // Convert the extracted string to an integer
        int offset = std::stoi(offsetStr);

        return offset;  // Return the integer offset
    } else {
        std::cerr << "Newline character not found!" << std::endl;
        return -1;  // Return an error value
    }
}




int main()
{
	// Server socket id
	int sockfd, ret;

	// Server socket address structures
	struct sockaddr_in serverAddr;

	// Client socket id
	int clientSocket;

	// Client socket address structures
	struct sockaddr_in cliAddr;

	// Stores byte size of server socket address
	socklen_t addr_size;

	// Child process id
	pid_t childpid;

	// Creates a TCP socket id from IPV4 family
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	// Error handling if socket id is not valid
	if (sockfd < 0) {
		printf("Error in connection.\n");
		exit(1);
	}

	printf("Server Socket is created.\n");

	// Initializing address structure with NULL
	memset(&serverAddr, '\0',
		sizeof(serverAddr));

	// Assign port number and IP address
	// to the socket created
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);

	// 127.0.0.1 is a loopback address
	serverAddr.sin_addr.s_addr
		= inet_addr("127.0.0.1");

	// Binding the socket id with
	// the socket structure
	ret = bind(sockfd,
			(struct sockaddr*)&serverAddr,
			sizeof(serverAddr));

	// Error handling
	if (ret < 0) {
		printf("Error in binding.\n");
		exit(1);
	}

	// Listening for connections (upto 10)
	if (listen(sockfd, 10) == 0) {
		printf("Listening...\n\n");
	}

	int cnt = 0;
	while (1) {

		// Accept clients and
		// store their information in cliAddr
		clientSocket = accept(
			sockfd, (struct sockaddr*)&cliAddr,
			&addr_size);

		// Error handling
		if (clientSocket < 0) {
			exit(1);
		}

		// Displaying information of
		// connected client
		printf("Connection accepted from %s:%d\n",
			inet_ntoa(cliAddr.sin_addr),
			ntohs(cliAddr.sin_port));

		// Print number of clients
		// connected till now
		printf("Clients connected: %d\n\n",
			++cnt);

		// Creates a child process
		if ((childpid = fork()) == 0) {

			// Closing the server socket id
			close(sockfd);

			// Send a confirmation message
			// to the client
            while(1){
                // char receiveBuffer[200];
                // int rbyteCount = recv(clientSocket, receiveBuffer, 200, 0);
                // if (rbyteCount < 0) {
                //     std::cout << "Client recv error: " << std::endl;
                //     return 0;
                // } else {
                //     std::cout << "Client: Received data: " << receiveBuffer << std::endl;
                // }

                char receiveBuffer[1024];
                ssize_t rbyteCount = recv(clientSocket, receiveBuffer, sizeof(receiveBuffer) - 1, 0);

                if (rbyteCount < 0) {
                    perror("Client recv error");
                } else if (rbyteCount == 0) {
                    printf("Client closed connection\n");
					cnt--;
					return 0;
                } else {
                    // receiveBuffer[rbyteCount - 2] = '\n'; // Null-terminate the received data
                    printf("Client: Received data: %s\n", receiveBuffer);
					std::cout<<rbyteCount<<std::endl;
					std::string receivedMessage(receiveBuffer, rbyteCount); // Convert to std::string
    				// std::cout << "Client: Received data: " << receivedMessage << std::endl;
					std::cout<<"recd message: "<<receivedMessage<<std::endl;
					int offset = extractOffset(receivedMessage);
					if(offset == -1){
						//error
						// send(clientSocket, ("error").c_str(), ("error").length(), 0);
					}
					else{

						// Assume we receive an offset and k from the client
						// int offset = 10;  // Example: start from the 10th word
						int k = 5;  // Example: send 15 words in total
						int p = 4;  // Example: send 5 words per packet
						std::string filename = "words.txt";
						std::vector<std::string> words = readWordsFromFile(filename);
						std::cout<<"words size:"<<words.size()<<std::endl;
						std::cout<<"offset:"<<offset<<std::endl;
						offset--;

						 if (offset >= words.size()) {
							// If offset is larger than the number of words, respond with a special message
							std::string response = "$$\n";
							send(clientSocket, response.c_str(), response.length(), 0);
						} 
						else {
							// Send k words starting from offset, in packets of size p
							sendWordsInPackets(clientSocket, words, offset, k, p);
						}
					}

                }
                // send(clientSocket, "received your message, thanks!", strlen("received your message, thanks!"), 0);
            }
		}
	}

	// Close the client socket id
	close(clientSocket);
	return 0;
}
