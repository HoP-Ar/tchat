#include "clientserver.hpp"

///////////////////////////////////////////////////////////////////////////
//
//          Functional Description Of Classes 
//
///////////////////////////////////////////////////////////////////////////

//
//Class TCPBase
//

TCPBase::TCPBase(){}

void TCPBase::setup(){
	setPort();
	timeleft.tv_sec = 1;
	timeleft.tv_usec = 0;
	inRun = true;
	setAddr();
}

TCPBase::~TCPBase(){
	stop();
}

void TCPBase::stop(){
	if(!inRun) return;
	close(socFD);
	inRun = false;
}

bool TCPBase::isConnected(){
	return inRun;
}

void TCPBase::setPort(){
	port = 4023;
}

bool TCPBase::send(std::string message){
	messg_len = write(connSocFD, message.c_str(), 1024);
	if(messg_len < 0) return false;
	return true;
}

std::string TCPBase::breceive(){
	clearBuff();
	messg_len = read(connSocFD, messg, 1024);
	if(messg_len < 0){
		stop();
		return "-1";
	}
	return (std::string) messg;
}

std::string TCPBase::receive(){
	fd_set fd4read;
	FD_ZERO(&fd4read);
	FD_SET(connSocFD,&fd4read);
	if(select(connSocFD + 1, &fd4read, NULL, NULL, &timeleft) <= 0) return "";
	return breceive();
}

void TCPBase::clearBuff(){
	for(int i = 0; i < sizeof(messg); i++)
		messg[i] = 0;
}

//
//class TCPClient
//

TCPClient::TCPClient(std::string IPaddress, std::string clientType) : TCPBase(){
	IP = IPaddress.c_str();
	type = clientType;
}

bool TCPClient::start(){
	setup();
	if(!create()) return false;
	return true;
}

bool TCPClient::create(){
	socFD = socket(AF_INET, SOCK_STREAM, 0);
	//test(port);
	if(socFD < 0) return false;
	while(connect(socFD, (sockaddr*) &addr, sizeof(addr)) != 0){
		sleep(1);
	}
	connSocFD = socFD;
	return true;
}

void TCPClient::setAddr(){
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_aton(IP, &addr.sin_addr);
}

//
//class TCPServer
//

TCPServer::TCPServer(std::string serverType) : TCPBase(){
	type = serverType;
	set = false;
}

bool TCPServer::start(){
	setup();
	if(!create()) return false;
	takeClient();
	return true;
}

bool TCPServer::create(){
	socFD = socket(AF_INET, SOCK_STREAM, 0);
	if(socFD < 0) return false;
	if(bind(socFD, (sockaddr*) &addr, sizeof(addr)) < 0) return false;
	if(listen(socFD, 5) < 0) return false;
	set = true;
	return true;
}

void TCPServer::setAddr(){
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
}

void TCPServer::takeClient(){
	socklen_t some = sizeof(addr);
	connSocFD = accept(socFD, (sockaddr*) &addr, &some);
}

//
//class Client
//

Client::Client(std::string IPaddress, std::string clientType) : TCPClient(IPaddress, clientType){}

bool Client::start(){
	setup();
	if(!create()) return false;
	// connecting
	std::string setupMsg;
	// step 1 <<
	setupMsg = breceive();
	if(setupMsg != "OK"){
		std::cout<<"Server: "<<setupMsg<<std::endl;
		return false;
	}
	// step 2 >>
	send(type);
	// step 3 <<
	setupMsg = breceive();
	test("| "+setupMsg+" |");
	int touchPort = atoi(setupMsg.c_str());
	if(touchPort == -1)
		return false;
	test(touchPort);
	addr.sin_port = htons(touchPort);
	stop();
	if(!create()) return false;
	// connected
	test("haha");
	return true;
}

///////////////////////////////////////////////////////////////////////////////////

//
//class ClientListener
//

ClientListener::ClientListener(int clientID, std::vector<ClientListener>* friendList) : TCPServer(""){
	id = clientID;
	setFriendList(friendList);
}

ClientListener::ClientListener(int clientID) : TCPServer(""){
	id = clientID;
}

void ClientListener::setFriendList(std::vector<ClientListener>* friendList){	
	friends = friendList;
}

bool ClientListener::start(){
	if(!create()) return false;
	takeClient();
	std::cout<<"One more client has been connected on "<<port<<" port."<<std::endl;
	while(1){
		receive();
	}
	return true;
}

void ClientListener::setPort(){
//	test("begin");
	sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = INADDR_ANY;
	int socFDs = socket(AF_INET, SOCK_STREAM, 0);
	if(socFDs >= 0)
		for(int listenPort = 42310; listenPort < 60000; listenPort++){
//			test(listenPort);
			saddr.sin_port = htons(listenPort);
			if(bind(socFDs, (sockaddr*) &saddr, sizeof(saddr)) < 0){
//				test();
				continue;
			}
			else{
				close(socFDs);
				port = listenPort;
//				test("end");
				return;
			}
		}
//	test(socFDs);
	port = -1;
//	test("end 1");
}

int ClientListener::getPort(){
	return port;
}

int ClientListener::getID(){
	return id;
}

void ClientListener::receive(){
	std::string msg = breceive();
	for(int i = 0; i < friends->size(); i++)
		if(id != (*friends)[i].getID())
			(*friends)[i].send(msg);
}

//
//class ClientGroup
//

ClientGroup::ClientGroup(std::string groupType){
	type = groupType;
}

int ClientGroup::getCount(){
	return clients.size();
}

std::string ClientGroup::getType(){
	return type;
}

std::string ClientGroup::creatSlot(){
	boost::thread newThread;
	int newID;
	if(clients.size() != 0){
		newID = clients.back().getID() + 1;
	}
	else{
		newID = 0;
	}
	ClientListener newOne(newID);
	clients.push_back(newOne);
	clients.back().setFriendList(&clients);
	clients.back().setup();	 							//there is some problem.
	newThread = boost::thread(boost::bind(&ClientListener::start, clients.back()));
	std::ostringstream newPort;
	newPort<<clients.back().getPort();
	return newPort.str();
}

///////////////////////////////////////////////////////////////////////////////////

//
//class Server
//

Server::Server(std::string serverType, int maxClients) : TCPServer(serverType) {
	slots = maxClients;
}

Server::Server(std::string serverType) : TCPServer(serverType) {
	slots = 10;
}

Server::Server(int maxClients) : TCPServer(NULL) {
	slots = maxClients;
}

bool Server::start(){
	setup();
	if(!create()) return false;
	threadForClientReceive = boost::thread(boost::bind(&Server::takeClient, this));
	return true;
}

void Server::wait_to_end(){
	threadForClientReceive.join();
}

void Server::takeClient(){
	socklen_t some = sizeof(addr);
	bool notAssigned;
	std::string msg;
	std::string clientType;
	ClientGroup* clientGroup;
	while(1){
		connSocFD = accept(socFD, (sockaddr*) &addr, &some);
		// receiving
		int mn = 0;
		for(int i = 0; i < groups.size(); i++){
			if(groups[i].getCount() != 0)
				mn += groups[i].getCount();
			else{
				groups.erase(groups.begin() + i);
				i--;
			}
		}
		// step 1 >>
		if(mn >= slots){
			send("Sorry. Server is full, please try later.");
////////////// maybe need
			close(connSocFD);				// maybe need
			continue;
		}
		send("OK");
		// step 2 <<
		clientType = breceive();
		// step 3 >>
		if((type.c_str() != NULL)&&(clientType != type)){
			send("-1");
////////////// maybe need
			close(connSocFD);				// maybe need
			continue;
		}
		notAssigned = true;
		for(int i = 0; i < groups.size(); i++){
			if(clientType == groups[i].getType()){
				clientGroup = &groups[i];
				notAssigned = false;
				break;
			}
		}
		if(notAssigned){
			ClientGroup newOne(clientType);
			groups.push_back(newOne);
			clientGroup = &groups[groups.size() - 1];
		}
		msg = clientGroup->creatSlot();
		test("rM = " + msg);
		send(msg);
////////// maybe need
		close(connSocFD);					// maybe need
		// received
	}
}
//...
/////////////////////////////////////// END OF FILE ////////////////////////////////////////
