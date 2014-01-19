#include "chat.hpp"
//v0.1 #include <boost/thread.hpp>
#include <pthread.h>
#include <iostream>
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////////////////////////
static std::string cgroup_mode = "";
static std::string c1x1_mode = "";
static std::string server_mode = "";
static std::string s1x1_mode = "";
static std::string version = "";
//////////////////////////////////////////////////////////////////////////////////////////////

void* chat_listen_thread(void* null){
	((Chat*) null)->listen();
	return 0;
}

Chat::Chat(TCPBase* connection){
	conn = connection;
}

void Chat::start(){
	//v0.1 boost::thread ls;
	pthread_t ls;
	std::string msg;
	
	if(!conn->start())
		return;
//	std::cout<<"started"<<std::endl; /////
	//v0.1 ls = boost::thread(&Chat::listen, this);
	pthread_create(&ls, NULL, &chat_listen_thread, this);
	while(conn->isConnected() && msg != "/exit"){
		std::getline(std::cin, msg);
		conn->send(msg);
	}
//	std::cout<<conn->isConnected()<<std::endl; /////
}

void Chat::listen(){
	std::string msg = "Connected.";
	while(conn->isConnected() && msg != "/exit"){
		std::cout<<"\e[33m"<<msg<<"\e[0m"<<std::endl;
		msg = conn->breceive();
	}
	std::cout<<"\e[31mConnection closed.\e[0m"<<std::flush;
	conn->stop();
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////

Opts::Opts(int argc_a, char** argv_a){
	init();
	grouping();
	argc = argc_a;
	argv = argv_a;
	std::string theOpt;
	bool correct = false;
	if(argc > 1){
		theOpt = argv[1];
		if(theOpt[0] == 45)
			correct = true;
	}
	if(correct) {
		correct = false;
		for(int i = 0; i < int(opt_groups[argc - 1].size()); i++){
			if (argv[1] == "-" + opt_groups[argc - 1][i]){
				// test(opt_groups[argc - 1][i]);
				opt_nm = (int) opts[opt_groups[argc - 1][i]]/10;
				for(int j = 2; j < argc; j++){
					opt_val.push_back(argv[j]);
					// test(argv[j]);
				}
				correct = true;
				break;
			}
		}
	}
	if(!correct)
		std::cout<<"Please read description for correct use."<<std::endl;
}

void Opts::run(){
	switch(opt_nm){
	case 0:
		std::cout<<"\n\ttchat is started on 'cgroup' mode\n"<<std::endl;
		run0_cgroup();
		break;
	case 1:
		std::cout<<"\n\ttchat is started on 'c1x1' mode\n"<<std::endl;
		run1_c1x1();
		break;
	case 2:
		std::cout<<"\n\ttchat is started on 'server' mode\n"<<std::endl;
		run2_server();
		break;
	case 3:
		std::cout<<"\n\ttchat is started on 's1x1' mode\n"<<std::endl;
		run3_s1x1();
		break;
	case 4:
		std::cout<<"\n\ttchat -<opt> [argument]\n\n <opt> : <description>\n -----   -------------\n";
		for(std::map<std::string, int>::iterator it = opts.begin(); it != opts.end(); it++){
			std::cout<<" "<<it->first<<" : "<<optDescription[(int) it->second / 10]<<".\n";
		}
		std::cout<<std::endl;
		break;
	case 5:	
		std::cout<<"tchat [Version 1.1], 2013.\n";
		std::cout<<"This is free software: you are free to change and redistribute it.\n";
		std::cout<<"There is NO WARRANTY, to the extent permitted by law.\n\n";
		std::cout<<"Written by Hovhannes Palyan.";
		std::cout<<std::endl;
	break;
	}
}

void Opts::init(){
	opt_nm = -1;
	opts["-cgroup"] = 2;
	opts["-c1x1"] = 12;
	opts["-server"] = 22;
	opts["-s1x1"] = 31;
	opts["-help"] = 41;
	opts["h"] = 41;
	opts["?"] = 41;
	opts["V"] = 51;
	opts["S"] = 22;
	opts["s"] = 31;
	opts["C"] = 2;
	opts["c"] = 12;
	opts["-version"] = 51;
	optDescription[0] = "Chat with group.\t/: -<opt> <IPaddress>";
	optDescription[1] = "Chat with 1 person.\t/: -<opt> <IPaddress>";
	optDescription[2] = "Create a chat server.\t/: -<opt> <Client Limit>";
	optDescription[3] = "Create chat connection and wait 1 person.\t/: -<opt>";
	optDescription[4] = "Show help.\t/: -<opt>";
	optDescription[5] = "Show product information.\t/: -<opt>";
}

void Opts::grouping(){
	for(std::map<std::string, int>::iterator it = opts.begin(); it != opts.end(); it++){
		opt_groups[it->second - (int) (it->second / 10) * 10].push_back(it->first);
	}
}

void Opts::run0_cgroup(){
	TCPBase* client = new Client(opt_val[0], "chatG");
	Chat c(client);
	c.start();
	delete(client);
}

void Opts::run1_c1x1(){
	TCPBase* client = new TCPClient(opt_val[0], "chat1x1");
	Chat c(client);
	c.start();
	delete(client);
}

void Opts::run2_server(){
	Server server("chatG", atoi(opt_val[0].c_str()));
	server.start();
	server.wait_to_end();
}

void Opts::run3_s1x1(){
	TCPBase* server = new TCPServer("char1x1");
	Chat c(server);
	c.start();
	delete(server);
}

