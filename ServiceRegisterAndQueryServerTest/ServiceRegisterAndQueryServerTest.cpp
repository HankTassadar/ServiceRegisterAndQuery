#include"ServiceRegisterAndQueryServer.h"


extern "C"
{
#include<openssl/applink.c>
};

int main()
{
	ServiceRegisterAndQueryServer server(35996);
	server.init();
	server.run();
	return 0;
}