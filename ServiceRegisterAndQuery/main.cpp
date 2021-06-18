#include"ServiceRegisterAndQueryServer.h"
#include<fstream>
#include<sstream>
#include<iostream>
#include<ctime>
int main()
{
	ServiceRegisterAndQueryServer server(35996);
	server.init();
	server.run();
	return 0;
}