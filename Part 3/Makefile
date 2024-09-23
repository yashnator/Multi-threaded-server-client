build: server server-cscd client client-beb client-cscd 

plot:
	python plot.py
	open average_completion_time.png
	make clean

run-aloha: server client
	./server &
	./client

run-beb: client_beb server
	./server &
	./client_beb

run-cscd: client_sensing_beb server_sensing_beb
	./server_sensing_beb &
	./client_sensing_beb

server: server.cpp ../utils.cpp
	g++-14 -o server server.cpp ../utils.cpp -I/opt/homebrew/Cellar/nlohmann-json/3.11.3/include/

client: client.cpp ../utils.hpp
	g++-14 -o client client.cpp ../utils.cpp -I/opt/homebrew/Cellar/nlohmann-json/3.11.3/include/

client-beb: client_BEB.cpp ../utils.hpp
	g++-14 -o client_beb client_BEB.cpp ../utils.cpp -I/opt/homebrew/Cellar/nlohmann-json/3.11.3/include/

client-cscd: client_sensing_beb.cpp ../utils.hpp
	g++-14 -o client_sensing_beb client_sensing_beb.cpp ../utils.cpp -I/opt/homebrew/Cellar/nlohmann-json/3.11.3/include/

server-cscd: server_sensing_beb.cpp ../utils.cpp
	g++-14 -o server_sensing_beb server_sensing_beb.cpp ../utils.cpp -I/opt/homebrew/Cellar/nlohmann-json/3.11.3/include/
	
.PHONY: clean re

clean:
	rm -rf output*.txt client client_beb client_sensing_beb server server_sensing_beb

re: clean build