#pragma once
#include<string>
#include<functional>
#include<map>
#include<vector>
#include<openssl/ssl.h>
#include<openssl/bio.h>
#include<openssl/rsa.h>

#ifdef _WIN32
#pragma comment(lib,"libssl_static.lib")
#pragma comment(lib,"libcrypto_static.lib")
#endif // _WIN32


class ServiceRegisterPortocol
{
public:
	enum class KEYTYPE
	{
		PRIVATE,
		PUBLIC
	};
	enum class MSGTYPE
	{
		REGISTER=6685,
		QUERY,
		RSP_QUERY,
		RSP_NULL
	};
	struct Pack {
		ServiceRegisterPortocol::MSGTYPE msgtype;
		std::string data;
	};
public:
	ServiceRegisterPortocol(KEYTYPE type,const char* keyfile);
	ServiceRegisterPortocol(KEYTYPE type, RSA* key);
	ServiceRegisterPortocol();
	~ServiceRegisterPortocol();

public:
	/**
	* 是否已经处理了所有的包
	*/
	bool isPackOver() {
		return _packOver;
	}

	/**
	* 是否已经进行过身份验证
	*/
	bool isAuth() {
		return _authed;
	}
public:
	/**
	* pack.data.size() must < 245;
	*/
	std::pair<const char*, size_t> pack(ServiceRegisterPortocol::Pack pack);

	std::vector<ServiceRegisterPortocol::Pack>* unPack(const char* data, size_t size);

	ServiceRegisterPortocol::Pack solveThePack(ServiceRegisterPortocol::Pack& pack);

	void setMsgTypeCallBack(ServiceRegisterPortocol::MSGTYPE msgtype, std::function<ServiceRegisterPortocol::Pack(const std::string&)> func);

private:
	std::string encrypt(const char* data, int datalength);

	std::string decrypt(const char* data);

	size_t byte64ToUInt(const char* buffer);

	ServiceRegisterPortocol::MSGTYPE byte32ToMsgType(const char* buffer);

private:
	bool _packOver;
	int _lastPackLength;
	std::string _lastPackData;
	char* _packDataPtr;
	std::map<ServiceRegisterPortocol::MSGTYPE, std::function<ServiceRegisterPortocol::Pack(const std::string&)>> _callback;

private:
	bool _authed;
	RSA* _rsa;	//use 2048 bit RSA
	KEYTYPE _type;

private:
	bool _needDesturctureRsa;
};

