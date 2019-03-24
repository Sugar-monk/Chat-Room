#include "ChatServer.hpp"
#include <iostream>

static void Usage(std::string proc)
{
	std::cout << "Usage:" << proc << "tcp_port  udp_port" << std::endl;
}

void *RunProduct(void *arg)
{
	pthread_detach(pthread_self());
	ChatServer *sp = (ChatServer*)arg;
	for(;;)
	{
		sp->Product();

	}
}

void *RunConsume(void *arg)
{
	pthread_detach(pthread_self());
	ChatServer *sp = (chatServer*)arg;
	for(;;)
	{
		sp->Consume();
	}
}
int main(int argc, char *argv[])
{
	if(argc != 3)
	{
		Usage(argv[0]);		//输出一个使用手册
		exit(1);
	}

	int tcp_port = atoi(argv[1]);
	int udp_port = atoi(argv[2]);

	ChartServer *sp = new ChatServer(tcp_port, cdp_port);
	sp->IntiServer();
	
	pthread_t c,p(&c, NULL, RunProduct, (void*)sp);
	pthread_create(&c, NULL, RunConsume, (void*)sp);

	sp->Start();

	return 0;
}
