CPP = g++
CPPFLAGS = -Wall -Wextra -std=c++17 

SERVER_TRG = $(basename server.cpp)
CLIENT_TRG = $(basename client.cpp)

all: $(SERVER_TRG) $(CLIENT_TRG)

$(SERVER_TRG) : server.cpp
	$(CPP) $(CPPFLAGS) server.cpp -o $(SERVER_TRG)

$(CLIENT_TRG) : client.cpp
	$(CPP) $(CPPFLAGS) client.cpp -o $(CLIENT_TRG)

clean:
	rm -f $(SERVER_TRG) $(CLIENT_TRG)
