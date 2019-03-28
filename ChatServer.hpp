#include "ProtocolUtil.hpp"
#include "UserManager.hpp"
#include "DataPool.hpp"
#include "Message.hpp"
#include <pthread.h>
#include <iostream>
#include "Log.hpp"
#pragma  once


class ChatServer;

//为了方便实用，定义一个Param结构体
class Param
{
	public:
		ChatServer *sp;
		int sock;

		std::string ip;
		int port;
	public:
		Param(ChatServer *sp_, int &sock_, const std::string &ip_, const int &port_)
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

		unsigned int RegisterUser(const std::string &name,			//	//注册一个新的ID
									const std::string &school,
										const std::string &passwd)	
		{
			return um.Insert(name, school, passwd);
		}

		unsigned int LoginUser(const unsigned int &id,			//登录验证
								const std::string &passwd,
									const std::string &ip,
										const int port)	
		{
			return um.Check(id, passwd);
		}
		//UDP		生产着消费者模型中生产者
		void Product()
		{
			std::string message;
			struct sockaddr_in peer;
			Util::RecvMessage(udp_work_sock, message, peer);
			std::cout << "debug: recv message:" << message << std::endl;
			if(!message.empty())
			{
				pool.PutMessage(message);
				Message m;
				m.ToRecvValue(message);		//反序列化
				if(m.Type() == LOGIN_TYPE)
				{
					um.AddOnlineUser(m.Id(), peer);
					std::string name_, school_;
					um.GetUserInfo(m.Id(), name_, school_);
					Message new_mesg(name_, school_, m.Text(), m.Id(), m.Type());
					new_mesg.ToSendString(message);
				}
			}
		}
		void Consume()				//生产者消费者模型中的消费者
		{
			std::string message;
			pool.GetMessage(message);
			std::cout << "debug: send message:" << message << std::endl;
			auto online = um.OnlineUser();
			for(auto it = online.begin(); it != online.end(); ++it)
			{
				Util::SendMessage(udp_work_sock, message, it->second);
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
			Json::Value root;
			
			LOG(rq.text, NORMAL);

			Util::UnSeralizer(rq.text, root);		//对正文反序列化
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

				unsigned int result = sp->LoginUser(id, passwd, ip, port);		//登录验证
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
				int sock = SocketApi::Accept(tcp_listen_sock, ip, port);
				if(sock > 0)			//sock 登录成功
				{
					std::cout << "get a new client" << ip << ":" << port << std::endl;	

					Param *p = new Param(this, sock, ip, port);
					pthread_t tid;
					pthread_create(&tid, NULL, HandlerRequest, p);		//让这个线程处理请求
				}
			}

		}
		~ChatServer()
		{}
};




