#pragma once
#include"ServiceRegisterPortocol.h"
#include<include/uv11.hpp>

#ifdef _WIN32
#pragma comment(lib,"uv_cpp.lib")
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib,"Psapi.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "userenv.lib")
#endif // _WIN32

class ServiceRegister
{
public:
	ServiceRegister(uv::SocketAddr target,unsigned short port)
		:_portocol(new ServiceRegisterPortocol(ServiceRegisterPortocol::KEYTYPE::PUBLIC,"public.pem"))
		, _sockaddr(new uv::SocketAddr{"0.0.0.0",port,uv::SocketAddr::Ipv4})
		,_loop(uv::EventLoop::DefaultLoop())
		,_client(new uv::Udp(this->_loop))
		,_target(target)
		,_port(port)
	{
		auto a = this->_client->bindAndRead(*this->_sockaddr);
		while (a != 0) {
			this->_port++;
			delete(this->_sockaddr);
			this->_sockaddr = new uv::SocketAddr{ "0.0.0.0",this->_port,uv::SocketAddr::Ipv4 };
			a = this->_client->bindAndRead(*this->_sockaddr);
		}
	}
	~ServiceRegister() {
		delete(this->_portocol);
	}

public:
	void regist(std::string serviceName, int port) {
		auto pack = this->_portocol->pack({ ServiceRegisterPortocol::MSGTYPE::REGISTER,
			serviceName + "\n" + std::to_string(port) + "\n" });
		auto re = this->_client->send(this->_target, pack.first, pack.second);
	}
	void run() {
		this->_loop->run();
	}
private:
private:
	uv::SocketAddr* _sockaddr;
	uv::EventLoop* _loop;
	uv::Udp* _client;
	uv::SocketAddr _target;
	unsigned short _port;
private:
	ServiceRegisterPortocol* _portocol;
};

