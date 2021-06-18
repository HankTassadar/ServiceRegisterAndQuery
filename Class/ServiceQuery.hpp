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

class ServiceQuery
{
public:
	ServiceQuery(uv::SocketAddr target, unsigned short port)
		:_portocol(new ServiceRegisterPortocol())
		, _sockaddr(new uv::SocketAddr{ "0.0.0.0",port,uv::SocketAddr::Ipv4 })
		, _loop(uv::EventLoop::DefaultLoop())
		, _client(new uv::Udp(this->_loop))
		, _target(target)
		, _port(port)
		, _getRSP(false)
		, _rspport(0)
	{
		this->setCallback();
		auto a = this->_client->bindAndRead(*this->_sockaddr);
		while (a != 0) {
			this->_port++;
			delete(this->_sockaddr);
			this->_sockaddr = new uv::SocketAddr{ "0.0.0.0",this->_port,uv::SocketAddr::Ipv4 };
			a = this->_client->bindAndRead(*this->_sockaddr);
		}
	}
	ServiceQuery() = delete;
	~ServiceQuery() {
		delete(this->_portocol);
		delete(this->_sockaddr);
		delete(this->_client);
		this->_loop->~EventLoop();
	}

public:
	std::pair<std::string,unsigned short> query(std::string serviceName) {
		std::thread loop([=]() {this->run(); });
		loop.detach();
		auto pack = this->_portocol->pack({ ServiceRegisterPortocol::MSGTYPE::QUERY,
			"Query:"+serviceName});
		int count = 0;
		while (count++!=5) {	//超时重查10次
			this->_client->send(this->_target, pack.first, pack.second);
			int waitCount = 0;
			std::chrono::duration<int, std::milli> a(10);
			bool flag = false;
			while (waitCount++!=50)
			{
				std::this_thread::sleep_for(a);
				this->_mutex.lock();
				flag = this->_getRSP;
				this->_mutex.unlock();
				if (flag)
					break;
			}
			if (flag)break;
		}
		this->_loop->stop();
		return{ this->_ip,this->_rspport };
	}

private:

	void run() {
		this->_loop->run();
	}

	void setCallback() {
		this->_portocol->setMsgTypeCallBack(ServiceRegisterPortocol::MSGTYPE::RSP_QUERY
			, [=](const std::string& data)->ServiceRegisterPortocol::Pack {
				if (data.find("NoService") == std::string::npos) {
					auto pos = data.find('\n');
					std::string port = data.substr(0, pos);
					std::string ip = data.substr(pos + 1, data.size());
					this->_ip = ip;
					this->_rspport = std::stoi(port);
				}

				this->_mutex.lock();
				this->_getRSP = true;
				this->_mutex.unlock();
				return { ServiceRegisterPortocol::MSGTYPE::RSP_NULL,"" };
			});

		this->_client->setMessageCallback(std::bind(&ServiceQuery::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	}
private:
	void onMessage(uv::SocketAddr& addr, const char* data, unsigned length) {
		auto packvec = this->_portocol->unPack(data, length);

		for (auto& pack : *packvec) {

			if (pack.msgtype == ServiceRegisterPortocol::MSGTYPE::REGISTER) {
				std::string addrstr;
				addr.toStr(addrstr);
				pack.data += "\n" + addrstr;
			}

			auto rsppack = this->_portocol->solveThePack(pack);
			if (rsppack.msgtype != ServiceRegisterPortocol::MSGTYPE::RSP_NULL) {
				auto res = this->_portocol->pack(rsppack);
				int i = 0;
				this->_client->send(addr, res.first, res.second);

			}
		}

		delete(packvec);
	}
private:
	uv::SocketAddr* _sockaddr;
	uv::EventLoop* _loop;
	uv::Udp* _client;
	uv::SocketAddr _target;
	unsigned short _port;
private:
	ServiceRegisterPortocol* _portocol;
private:
	std::mutex _mutex;
	bool _getRSP;
	std::string _ip;
	unsigned short _rspport;
};


