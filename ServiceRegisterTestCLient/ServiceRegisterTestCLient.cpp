#include<ServiceRegister.hpp>
#include<Windows.h>
extern "C"
{
#include<openssl/applink.c>
};

int main() {

	ServiceRegister regist1({ "127.0.0.1",35996,uv::SocketAddr::Ipv4 },10002);
	//ServiceRegister regist2({ "49.232.218.70",35996,uv::SocketAddr::Ipv4 },10002);
	for(int i=0;i<10;i++)
		regist1.regist("ArkHelperService", 15996);
	//regist1.run();
	return 0;
}


