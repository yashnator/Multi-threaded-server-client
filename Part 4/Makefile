all: server client rogue-client

server: server.cpp ../utils.cpp
	g++-14 -o server server.cpp ../utils.cpp -I/opt/homebrew/Cellar/nlohmann-json/3.11.3/include/

client: client.cpp ../utils.hpp
	g++-14 -o client client.cpp ../utils.cpp -I/opt/homebrew/Cellar/nlohmann-json/3.11.3/include/

rogue-client: client.cpp ../utils.hpp
	g++-14 -o rogue_client rogue_client.cpp ../utils.cpp -I/opt/homebrew/Cellar/nlohmann-json/3.11.3/include/
	
.PHONY: clean re

clean:
	rm -rf output*.txt client server

re: clean all