
CC ?= g++
OPT ?= -Wall -g
sources ?= sources/
results ?= results/
LIBBOOST ?= -lboost_thread-mt -lboost_system-mt
CFLAG ?= -lpthread
install_dir ?= ~/execs/

default: tchat

compile:
	@echo -en "Compiling...\r"
clientserver.o: $(sources)clientserver.cpp $(sources)clientserver.hpp
	@$(CC) $(OPT) -c $(sources)clientserver.cpp
chat.o: $(sources)chat.cpp $(sources)chat.hpp $(sources)clientserver.hpp
	@$(CC) $(OPT) -c $(sources)chat.cpp
main.o: $(sources)main.cpp $(sources)chat.hpp
	@$(CC) $(OPT) -c $(sources)main.cpp
tchat: compile clientserver.o chat.o main.o
	@echo -en "Compiling tchat\r"
	@$(CC) $(OPT) clientserver.o chat.o main.o -o tchat $(CFLAG)
	@echo "Success.                                               "
	@mkdir -p $(results)
	@cp tchat $(results)
	@echo "tchat -> $(results)"
install: tchat
	@echo -en "Installing...\r"
	@cp $(results)tchat $(install_dir)
	@echo 'tchat -> $(install_dir)'
tar:
	@mkdir -p $(results)
	@tar -czvf $(results)tchat.tar.gz ../tchat/sources/* ../tchat/Makefile
clean:
	rm -rf $(results)
	rm -rf *.o 
