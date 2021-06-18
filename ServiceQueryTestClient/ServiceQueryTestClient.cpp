#include<ServiceQuery.hpp>
#include <iostream>
#include<Windows.h>
extern "C"
{
#include<openssl/applink.c>
};

int main()
{
    ServiceQuery query({ "127.0.0.1",35996,uv::SocketAddr::Ipv4 }, 10002);
    auto re = query.query("serviceTest");
    std::cout << re.first << "/" << re.second << std::endl;
    return 0;
}
