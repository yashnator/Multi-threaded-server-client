all: server client

server: server.cpp
	g++-14 -o server server.cpp -I/opt/homebrew/Cellar/nlohmann-json/3.11.3/include/
	g++-14 -o client client.cpp -I/opt/homebrew/Cellar/nlohmann-json/3.11.3/include/

client: client.cpp
	g++-14 -o server server.cpp -I/opt/homebrew/Cellar/nlohmann-json/3.11.3/include/
	g++-14 -o client client.cpp -I/opt/homebrew/Cellar/nlohmann-json/3.11.3/include/
	
.PHONY: clean re

clean:
	rm -rf output.txt client server

re: clean all