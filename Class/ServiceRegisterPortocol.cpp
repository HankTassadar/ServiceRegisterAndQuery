#include"ServiceRegisterPortocol.h"
#include<cstdio>
#include<iostream>
#include<ctime>
#include<openssl/rsa.h>
ServiceRegisterPortocol::ServiceRegisterPortocol(KEYTYPE type, const char* keyfile)
	: _packOver(true)
	, _lastPackLength(0)
	, _packDataPtr(nullptr)
	, _authed(false)
	, _rsa(RSA_new())
	, _type(type)
	, _needDesturctureRsa(true)
{

	RSA* temp = nullptr;
	FILE* keyfilebuf = fopen(keyfile, "r+");
	
	if (keyfilebuf == nullptr) {
		std::cout << "RSA key read error,no file \"" + std::string(keyfile) + "\" exist!" << std::endl;
		exit(0);
	}

	switch (type)
	{
	case ServiceRegisterPortocol::KEYTYPE::PRIVATE:
		temp = PEM_read_RSAPrivateKey(keyfilebuf, &this->_rsa, nullptr, nullptr);
		break;
	case ServiceRegisterPortocol::KEYTYPE::PUBLIC:
		temp = PEM_read_RSAPublicKey(keyfilebuf, &this->_rsa, nullptr, nullptr);
		break;
	default:
		break;
	}
	fclose(keyfilebuf);
	if (temp == nullptr) {
		std::cout << "Read " + std::string(keyfile) + " failed,checked if key file is \"BEGIN RSA P****** KEY\"!" << std::endl;
		exit(0);
	}
	else {
		auto rsaSize = RSA_size(this->_rsa);
		if (rsaSize!=256) {
			std::cout << "rsaSize is not 256 ,checked if rsakey is 2048 bits!" << std::endl;
			exit(0);
		}
	}

}

ServiceRegisterPortocol::ServiceRegisterPortocol(KEYTYPE type, RSA* key)
	: _packOver(true)
	, _lastPackLength(0)
	, _packDataPtr(nullptr)
	, _authed(false)
	, _rsa(key)
	, _type(type)
	, _needDesturctureRsa(false)
{

	auto rsaSize = RSA_size(this->_rsa);
	if (rsaSize != 256) {
		std::cout << "rsaSize is not 256 ,checked if rsakey is 2048 bits!" << std::endl;
		exit(0);
	}
}

ServiceRegisterPortocol::ServiceRegisterPortocol()
	: _packOver(true)
	, _lastPackLength(0)
	, _packDataPtr(nullptr)
	, _authed(false)
	, _rsa(RSA_new())
	, _type(KEYTYPE::PUBLIC)
	, _needDesturctureRsa(false)
{
}

ServiceRegisterPortocol::~ServiceRegisterPortocol()
{
	delete[](this->_packDataPtr);
	if(this->_needDesturctureRsa)
		RSA_free(this->_rsa);
}

std::pair<const char*, size_t> ServiceRegisterPortocol::pack(ServiceRegisterPortocol::Pack pack)
{
	if (this->_packDataPtr != nullptr) {
		delete[](this->_packDataPtr);
		this->_packDataPtr = nullptr;
	}
	std::string re;
	if (pack.msgtype == MSGTYPE::REGISTER)
		re = this->encrypt(pack.data.c_str(), pack.data.size());
	else re = pack.data;


	this->_packDataPtr = new char[256];
	::memset(this->_packDataPtr, 0, 256);

	::memcpy((void*)(this->_packDataPtr), re.c_str(), re.size());
	return { this->_packDataPtr,256 };
}

std::vector<ServiceRegisterPortocol::Pack>* ServiceRegisterPortocol::unPack(const char* data, size_t size)
{
	auto packvec = new std::vector<ServiceRegisterPortocol::Pack>();
	if (size % 256 != 0)
		return packvec;
	auto packnum = size / 256;
	for (auto i = 0; i < packnum; i++) {
		char* ptr = (char*)data;
		std::string datastr(ptr + (i * 256), 256);
		if (datastr.find("Query:") != std::string::npos) {
			while (datastr.back() == '\0') {
				datastr.pop_back();
			}
			datastr = datastr.substr(datastr.find(':') + 1
				, datastr.size() - datastr.find(':'));
			packvec->push_back({ MSGTYPE::QUERY,datastr });
		}
		else if (datastr.find("QueryRsp:") != std::string::npos) {
			while (datastr.back() == '\0') {
				datastr.pop_back();
			}
			datastr = datastr.substr(datastr.find(':') + 1
				, datastr.size() - datastr.find(':'));
			packvec->push_back({ MSGTYPE::RSP_QUERY,datastr });
		}
		else {
			auto decryptodata = this->decrypt(datastr.c_str());
			packvec->push_back({ MSGTYPE::REGISTER,decryptodata });
		}
	}
	return packvec;
}

ServiceRegisterPortocol::Pack ServiceRegisterPortocol::solveThePack(ServiceRegisterPortocol::Pack& pack)
{
	return this->_callback[pack.msgtype](pack.data);
}

void ServiceRegisterPortocol::setMsgTypeCallBack(ServiceRegisterPortocol::MSGTYPE msgtype, std::function<ServiceRegisterPortocol::Pack(const std::string&)> func)
{
	this->_callback[msgtype] = func;
}

std::string ServiceRegisterPortocol::encrypt(const char* data, int datalength)
{
	int encryptTimes = datalength / 245 + 1;
	std::string re;
	auto temp = new char[256];
	int flen = 0;
	for (int i = 0; i < encryptTimes; i++) {

		::memset(temp, 0, 256);

		if (i == encryptTimes - 1) {

			flen = datalength - i * 245;

		}
		else {

			flen = 245;

		}

		switch (this->_type)
		{

		case ServiceRegisterPortocol::KEYTYPE::PRIVATE:
			RSA_private_encrypt(flen, (const unsigned char*)(data + i * 245), (unsigned char*)temp, this->_rsa, RSA_PKCS1_PADDING);
			break;

		case ServiceRegisterPortocol::KEYTYPE::PUBLIC:
			RSA_public_encrypt(flen, (const unsigned char*)(data + i * 245), (unsigned char*)temp, this->_rsa, RSA_PKCS1_PADDING);
			break;

		default:
			break;

		}

		re.append(temp, 256);
	}
	
	delete[](temp);
	return re;
}

std::string ServiceRegisterPortocol::decrypt(const char* data)
{
	auto temp = new char[256];
	::memset(temp, 0, 256);

	switch (this->_type)
	{

	case ServiceRegisterPortocol::KEYTYPE::PRIVATE:
		RSA_private_decrypt(256, (const unsigned char*)data, (unsigned char*)temp, this->_rsa, RSA_PKCS1_PADDING);
		break;

	case ServiceRegisterPortocol::KEYTYPE::PUBLIC:
		RSA_public_decrypt(256, (const unsigned char*)data, (unsigned char*)temp, this->_rsa, RSA_PKCS1_PADDING);
		break;

	default:
		break;

	}

	std::string re(temp, 245);
	delete[](temp);

	return re;
}

size_t ServiceRegisterPortocol::byte64ToUInt(const char* buffer)
{
	return static_cast<size_t>(
		static_cast<unsigned char>(buffer[0]) |
		static_cast<unsigned char>(buffer[1]) << 8 |
		static_cast<unsigned char>(buffer[2]) << 16 |
		static_cast<unsigned char>(buffer[3]) << 24 |
		static_cast<unsigned char>(buffer[4]) << 32 |
		static_cast<unsigned char>(buffer[5]) << 40 |
		static_cast<unsigned char>(buffer[6]) << 48 |
		static_cast<unsigned char>(buffer[7]) << 56);
}

ServiceRegisterPortocol::MSGTYPE ServiceRegisterPortocol::byte32ToMsgType(const char* buffer)
{
	return static_cast<ServiceRegisterPortocol::MSGTYPE>(
		static_cast<unsigned char>(buffer[0]) |
		static_cast<unsigned char>(buffer[1]) << 8 |
		static_cast<unsigned char>(buffer[2]) << 16 |
		static_cast<unsigned char>(buffer[3]) << 24);
}

