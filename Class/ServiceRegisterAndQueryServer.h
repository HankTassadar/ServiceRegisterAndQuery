#pragma once
#include<ServiceRegisterPortocol.h>
#include<include/uv11.hpp>
#include<ctime>
#include<list>
using namespace uv;
#ifdef _WIN32
#pragma comment(lib,"uv_cpp.lib")
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib,"Psapi.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "userenv.lib")
#endif // _WIN32

static ServiceRegisterPortocol portocol(ServiceRegisterPortocol::KEYTYPE::PRIVATE, "private.pem");

class ServiceRegisterAndQueryServer
{
public:
	ServiceRegisterAndQueryServer(int Port);
	~ServiceRegisterAndQueryServer();

public:
	void init();
	void run();

private:
	void setPortocolCallback();

	ServiceRegisterPortocol::Pack registerCallback(const std::string&);

	ServiceRegisterPortocol::Pack queryCallback(const std::string&);


private://服务器的回调
	void onMessage(SocketAddr&, const char*, unsigned);

	void serverTimer(Timer* handle);

private:
	struct Service {
		std::string name;
		std::string IP;
		std::string Port;
		time_t time;

	};
	
private:
	uv::SocketAddr _sockaddr;
	uv::EventLoop* _loop;
	uv::Udp* _server;

private:
	std::map < std::string, Service> _service;	//注册的服务
	std::mutex _mutex;
	uv::Timer* _timer;	//定时器
	size_t _timerCount;	//定时计数器

};

