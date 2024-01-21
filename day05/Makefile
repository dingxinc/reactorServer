all:client server

client:client.cpp
	g++ -std=c++11 -g -o client client.cpp
server:server.cpp InetAddress.cpp Socket.cpp Epoll.cpp
	g++ -std=c++11 -g -o server server.cpp InetAddress.cpp Socket.cpp Epoll.cpp

clean:
	rm -f client server