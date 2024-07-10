CXX = c++
CXXFLAGS = -std=c++98

TARGET = webserv

SRC = sources/core/main.cpp\
	sources/core/Server.cpp\
	sources/http/HttpRequest.cpp\
	sources/config/Config.cpp

INC = sources/core/

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) -I$(INC)

clean:
	rm -f $(TARGET)
