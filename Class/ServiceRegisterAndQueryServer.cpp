#include"ServiceRegisterAndQueryServer.h"
#include<iostream>
ServiceRegisterAndQueryServer::ServiceRegisterAndQueryServer(int Port)
	:_sockaddr({ "0.0.0.0",(unsigned short)Port,uv::SocketAddr::Ipv4 })
	, _loop(uv::EventLoop::DefaultLoop())
	, _server(new uv::Udp(_loop))
{
	this->_server->bindAndRead(this->_sockaddr);
	this->_timer = new uv::Timer(this->_loop, 1000, 1000, std::bind(&ServiceRegisterAndQueryServer::serverTimer, this, std::placeholders::_1));
}

ServiceRegisterAndQueryServer::~ServiceRegisterAndQueryServer()
{
	delete(this->_timer);

}

void ServiceRegisterAndQueryServer::init()
{
	//设置回调
	this->setPortocolCallback();

	this->_server->setMessageCallback(std::bind(&ServiceRegisterAndQueryServer::onMessage, this
		, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
}

void ServiceRegisterAndQueryServer::run()
{
	this->_timer->start();
	this->_loop->run();
}

void ServiceRegisterAndQueryServer::setPortocolCallback()
{
	portocol.setMsgTypeCallBack(ServiceRegisterPortocol::MSGTYPE::REGISTER
		, std::bind(&ServiceRegisterAndQueryServer::registerCallback, this, std::placeholders::_1));
	portocol.setMsgTypeCallBack(ServiceRegisterPortocol::MSGTYPE::QUERY
		, std::bind(&ServiceRegisterAndQueryServer::queryCallback, this, std::placeholders::_1));
}

ServiceRegisterPortocol::Pack ServiceRegisterAndQueryServer::registerCallback(const std::string& data)
{
	auto pos1 = data.find('\n');
	auto pos2 = data.find('\n', pos1 + 1);
	auto pos3 = data.find_last_of('\n', data.size());
	std::string serviceName = data.substr(0, pos1);
	std::string port = data.substr(pos1 + 1, pos2 - pos1 - 1);
	std::string ip = data.substr(pos3 + 1, data.find(":") - pos3 - 1);
	std::cout << "Register: " + serviceName + "/" + port + "/" + ip << std::endl;
	this->_mutex.lock();
	this->_service[serviceName] = { serviceName,port,ip,time(nullptr) };
	this->_mutex.unlock();
	std::cout << this->_service.size() << std::endl;
	return { ServiceRegisterPortocol::MSGTYPE::RSP_NULL,"" };
}

ServiceRegisterPortocol::Pack ServiceRegisterAndQueryServer::queryCallback(const std::string& data)
{
	std::string re = "QueryRsp:";
	std::cout << "Query: " + data << std::endl;
	auto iter = this->_service.find(data);
	if (iter != this->_service.end())
		re += this->_service[data].IP + "\n" + this->_service[data].Port;
	else
		re += "NoService";
	return { ServiceRegisterPortocol::MSGTYPE::RSP_QUERY,re };
}

void ServiceRegisterAndQueryServer::onMessage(SocketAddr& addr, const char* data, unsigned length)
{

	auto packvec = portocol.unPack(data, length);

	for (auto& pack : *packvec) {

		if (pack.msgtype == ServiceRegisterPortocol::MSGTYPE::REGISTER) {
			std::string addrstr;
			addr.toStr(addrstr);
			pack.data += "\n" + addrstr;
		}

		auto rsppack = portocol.solveThePack(pack);
		if (rsppack.msgtype != ServiceRegisterPortocol::MSGTYPE::RSP_NULL) {
			auto res = portocol.pack(rsppack);
			int i = 0;
			this->_server->send(addr, res.first, res.second);

		}
	}

	delete(packvec);
}

void ServiceRegisterAndQueryServer::serverTimer(Timer* handle)
{
	if (this->_timerCount++ == 3601)this->_timerCount == 0;
	time_t now = time(nullptr);

	if (this->_timerCount % 10 == 0) //每10秒清理一次没有刷新的
	{
		this->_mutex.lock();
		auto now = time(nullptr);
		for (auto iter = this->_service.begin(); iter != this->_service.end();) {
			if (now - iter->second.time > 10) {
				this->_service.erase(iter++);
			}
			else {
				iter++;
			}
		}
		this->_mutex.unlock();
	}
}
