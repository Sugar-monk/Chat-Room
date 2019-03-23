#pragma once

#include <iostream>
#include <pthread.h>
#include "ProtocolUtil.hpp"
#include "UserManager.hpp"
#include "json/json.h"
#include "DataPool.hpp"
#include "Message.hpp"

class CharSever;

//为了方便实用，定义一个Param结构体
class Param
{
	public:
		ChatServer *sp;
		int sock;

		std::string ip;
		int port;
	public:
		Param(ChatServer *sp_, int &sock_, const std:;string &ip_, const int port_)
			:sp(sp_),sock(sock_),ip(ip_),port(port_)
		{}
		~Param()
		{}
};


class ChatServer
{
	private:
		int tcp_listen_sock;
		int tcp_port;

		int udp_work_sock;
		int udp_port;

		UserManager um;		//用户管理对象
		DataPool pool;

	public:
		ChatServer(int tcp_port_ = 8888, int udp_port_ = 6666):
			tcp_port(tcp_port_),
			tcp_listen_sock(-1),
			udp_port(udp_port_),
			udp_work_sock(-1)
		{}
		void InitServer()	//初始化服务器：创建两个套接字、绑定套接字
		{
			tcp_listen_sock = SocketApi::Socket(SOCK_STREAM);
			udp_work_sock = SocketApi::Socket(SOCK_DGRAM);
			SocketApi::Bind(udp_work_sock, udp_port);
			SocketApi::Bind(tcp_listen_sock, tcp_port);
			
			SocketApi::Listen(tcp_listen_sock);		//tcp还要做监听的工作

		}

		unsigned int RegisterUser(const std::string &name,\			//	//注册一个新的ID
									const std::string &school,\
										const std:;string &passwd);	
		{
			return um.Insert(name, school, passwd);
		}

		unsigned int LoginUser(const std::string &id,\			//登录验证
								const std::string &passed\
								const std::string &id\
								const int &port)	
		{
			unsigned int result = um.Check(id, passwd);
			if(result >= 10000)
			{
				//um.MoveToOnline(id, ip, port);					//将客户移到用户在线列表
			}
			return result;
		}
		void Product()
		{
			std::string message;
			struct sockaddr_in peer;
			Util::RecvMessage(udp_work_sock, message);
			std::cout << "debug: recv message:" << message << std::endl;
			if(!message.emtey())
			{
				pool.PutMessage(message);
				Message m;
				m.ToRecvValue(message);		//反序列化
				um.AddOlineUser(m.Id(), peer);
			}
		}
		void Consume()
		{
			std::string message;
			pool.GetMessage(message);
			std::cout << "debug: send message:" << message << std::endl;
			auto online = um.OnlineUser();
			if(auto it = online.begin(); it != online.end(); it++)
			{
				Util::sendMessage(udp_work_sock, message, it->second);
			}
		}
		static void *HandlerRequest(void *arg)
		{
			Param *p = (Param*)arg;

			int sock = p->sock;
			ChatServer *sp = p->sp;
			std::string ip = p->ip;
			int port = p->port;
			delete p;
			pthread_detach(pthread_self());			//将此线程进行分离

			Request rq;
			Util::RecvRequest(sock, rq);
			Json::Valve root;
			
			LOG(rq.text, NORMAL);

			Until::UnSeralizer(rq.text, root);		//对正文反序列化
			if(rq.method == "REGISTER")					//注册
			{

				std::string name = root["name"].asString();
				std::string school = root["school"].asString();
				std::string passwd = root["passwd"].asString();
				unsigned int id = sp->RegisterUser(name, school, passwd);		//注册一个新的ID
				send(sock, &id, sizeof(id), 0);
			}
			else if(rq.method == "LOGIN")				//登录
			{
				unsigned int id = root["id"].asInt();
				std::string passwd = root["passwd"].asString();

				unsigned int result = sp->LoginUser(id, passed, ip, port);		//登录验证
				send(sock, &result, sizeof(result), 0);
			}
			else
			{}
			close(sock);
		}

		void Start()
		{

			std::string ip;
			int port;
			//运行主线程
			for(;;)
			{
				std::string ip;
				int port;
				int sock = SockerApi::Accept(tcp_listen_sock, ip, port);
				if(sock > 0)			//sock 登录成功
				{
					std::cout << "get a new client" << ip << ":" << port << std::endl;	

					Prama *p = new Param(this, sock, ip, port);
					pthread_t tid;
					pthread_create(&tid, NULL, HandlerRequest, p);		//让这个线程处理请求
				}
			}

		}
		~ChatServer()
		{}
};
