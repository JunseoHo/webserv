TARGET = webserv
CXX = c++
CXXFLAGS = -std=c++98

SRC = sources/core/main.cpp\
	sources/core/Core.cpp\
	sources/core/SocketManager.cpp\
	sources/core/BufferManager.cpp\
	sources/http/HttpRequest.cpp\
	sources/config/Config.cpp\
	sources/http/HttpResponse.cpp\
	sources/utils/utils.cpp\

OBJ=$(SRC:.cpp=.o)

INC = sources/core/

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) -I$(INC)

clean:
	rm -rf $(OBJ)

fclean:
	$(MAKE) clean
	rm -rf $(TARGET)

re: 
	$(MAKE) clean 
	$(MAKE) all

.PHONY: all clean fclean re