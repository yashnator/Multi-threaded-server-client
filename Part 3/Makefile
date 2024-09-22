: server client client-beb

server: server.cpp ../utils.cpp
	g++-14 -o server server.cpp ../utils.cpp -I/opt/homebrew/Cellar/nlohmann-json/3.11.3/include/

client: client.cpp ../utils.hpp
	g++-14 -o client client.cpp ../utils.cpp -I/opt/homebrew/Cellar/nlohmann-json/3.11.3/include/

client-beb: client_BEB.cpp ../utils.hpp
	g++-14 -o client_beb client_BEB.cpp ../utils.cpp -I/opt/homebrew/Cellar/nlohmann-json/3.11.3/include/
	
.PHONY: clean re

clean:
	rm -rf output*.txt client client_beb client_sensing_beb server

re: clean all