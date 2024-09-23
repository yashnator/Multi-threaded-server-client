run: server client
	./server &
	./client

server: server.cpp ../utils.cpp
	g++-14 -o server server.cpp ../utils.cpp -I/opt/homebrew/Cellar/nlohmann-json/3.11.3/include/

client: client.cpp ../utils.hpp
	g++-14 -o client client.cpp ../utils.cpp -I/opt/homebrew/Cellar/nlohmann-json/3.11.3/include/

plot:
	python3 plot.py
	open average_completion_time.png
	make clean-execs
	
build:
	make server
	make client

.PHONY: clean re

clean-execs:
	rm -rf output*.txt client server

clean-all:
	make clean-execs
	rm -rf stats.txt average_completion_time.png

re: clean all