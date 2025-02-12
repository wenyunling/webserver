src = $(wildcard code/Buffer/*.cpp) \
	  $(wildcard code/Epoll/*.cpp) \
	  $(wildcard code/Server/*.cpp) \
	  $(wildcard code/ThreadPool/*.cpp) \
	  $(wildcard code/Util/*.cpp) \
	  $(wildcard code/Http/*.cpp) \
      $(wildcard code/Log/*.cpp)  
server:
	g++ -std=c++17 -pthread -g \
	$(src) \
	server.cpp \
	-o server
	
client:
	g++ src/util.cpp src/Buffer.cpp src/Socket.cpp client.cpp -o client

clean:
	rm server