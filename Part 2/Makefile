run: server client
	./server &
	./client

server: server.cpp ../utils.cpp
	g++ -o server server.cpp ../utils.cpp -I/opt/homebrew/Cellar/nlohmann-json/3.11.3/include/

client: client.cpp ../utils.hpp
	g++ -o client client.cpp ../utils.cpp -I/opt/homebrew/Cellar/nlohmann-json/3.11.3/include/

plot:
	python3 plot.py
	explorer.exe average_completion_time.png
	
.PHONY: clean re

clean:
	rm -rf output*.txt client server

re: clean all