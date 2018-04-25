GDB = -g
THREAD = -pthread
PQXX = -lpqxx -lpq
GPB = `pkg-config --cflags --libs protobuf`
PROTO = amazon.pb.cc
All: server.o client.o database.o engine
engine: main.cpp server.o client.o amazon.pb.cc Common.hpp server.hpp server.hpp database.hpp
	g++ -std=c++11 main.cpp server.o client.o database.o $(PROTO) $(PQXX) $(GDB) $(THREAD) $(GPB) -o engine
server.o: server.cpp server.hpp
	g++ -std=c++11 server.cpp -c $(GDB) 
client.o: client.cpp client.hpp
	g++ -std=c++11 client.cpp -c $(GDB)	
database.o: database.cpp database.hpp
	g++ -std=c++11 database.cpp -c $(GDB) $(PROTO) $(PQXX) $(GPB)
clean:
	rm -f *~ client.o server.o database.o engine
