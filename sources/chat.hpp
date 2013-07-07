/////////////////////////////////////////////////////////////
//chat
/////////////////////////////////////////////////////////////

#ifndef CHAT_HPP
#define CHAT_HPP

#include <string>
#include <map>
#include <vector>
#include "clientserver.hpp"

class Chat{
public:
	Chat(TCPBase* connection);
	void start();
private:
	TCPBase* conn;
	void listen();
};

class Opts{
public:
	Opts(int argc_a, char** argv_a);
	void run();
protected:
	int argc;
	char** argv;
	std::map<std::string, int> opts;
	std::map<int, std::vector<std::string> > opt_groups;
	int opt_nm;
	std::vector<std::string> opt_val;
	std::string optDescription[10];
	void init();
	void grouping();
	void run0_cgroup();
	void run1_c1x1();
	void run2_server();
	void run3_s1x1();
};
#endif
