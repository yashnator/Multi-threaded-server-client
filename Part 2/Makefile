all: server client

server: server.cpp ../utils.hpp
	g++-14 -o server server.cpp ../utils.hpp -I/opt/homebrew/Cellar/nlohmann-json/3.11.3/include/
	g++-14 -o client client.cpp ../utils.hpp -I/opt/homebrew/Cellar/nlohmann-json/3.11.3/include/

client: client.cpp ../utils.hpp
	g++-14 -o server server.cpp ../utils.hpp -I/opt/homebrew/Cellar/nlohmann-json/3.11.3/include/
	g++-14 -o client client.cpp ../utils.hpp -I/opt/homebrew/Cellar/nlohmann-json/3.11.3/include/
	
.PHONY: clean re

clean:
	rm -rf output.txt client server

re: clean all