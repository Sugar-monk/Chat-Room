#pragma once

#include <iostream>
#include <string>
#include "ProtocolUtil.hpp"
#include "Message.hpp"
#include "Window.hpp"
#include <pthread.h>
#include <vector>

#define TCP_PORT 8888
#define UDP_PORT 6666
class ChatClient;

class Pap
{
	public:
		Window *wp;
		ChatClient *cp;
};

class ChatClient
{
	private:
		int tcp_sock;
		int udp_sock;
		std::string peer_ip;		//要连接的ID
		std::string passwd;
		struct sockaddr_in server;
	public:
		unsigned int id;
		std::string nick_name;
		std::string school;
	public:
		ChatClient(std::string ip_)
			:peer_ip(ip_)
		{
			id = 0;
			tcp_sock = -1;
			udp_sock = -1;
			server.sin_family = AF_INET;
			server.sin_port = htons(UDP_PORT);
			server.sin_addr.s_addr = inet_addr(peer_ip.c_str());
		}
		void InitClient()
		{
			udp_sock = SocketApi::Socket(SOCK_DGRAM);
		}

		bool ConnectServer()		//连接服务器
		{
			tcp_sock = SocketApi::Socket(SOCK_STREAM);
			return SocketApi::Connect(tcp_sock, peer_ip, TCP_PORT);
		}
		bool Register()			//注册
		{	
			//链接到服务器
			if(Util::RegisterEnter(nick_name, school, passwd) && ConnectServer())
			{
				Request rq;
				rq.method = "REGISTER\n";
				
				Json::Value root;		//构建root
				root["name"] = nick_name;
				root["school"] = school; 
				root["passwd"] = passwd;

				Util::Seralizer(root, rq.text);		//正文

				rq.content_length = "Content-Lenght: ";			//正文长度
				rq.content_length += Util::InToString((rq.text).size());	
				rq.content_length += "\n";

				Util::SendRequest(tcp_sock, rq);		//往服务器中发送请求
				recv(tcp_sock, &id, sizeof(id), 0);
				bool ret = false;

				if(id >= 10000)
				{
					ret = true;
					std::cout << "Register Success! Your Login ID Is :" << id << std::endl;
				}
				else
				{
					std::cout << "Register Failed! Code is :" << id <<std::endl;
				}
				close(tcp_sock);

				return ret;
			}
		}
		bool Login()
		{
			//使用短链接，链接成功后就释放
			if(Util::LoginEnter(id, passwd) && ConnectServer())  //链接到服务器
			{
				Request rq;
				rq.method = "LOGIN\n";
				
				Json::Value root;		//构建root
				root["id"] = id;
				root["passwd"] = passwd; 

				Util::Seralizer(root, rq.text);		//正文

				rq.content_length = "Content-Lenght: ";			//正文长度
				rq.content_length += Util::InToString((rq.text).size());	
				rq.content_length += "\n";

			
				Util::SendRequest(tcp_sock, rq);		//往服务器中发送请求
				unsigned int result = 0;
				recv(tcp_sock, &result, sizeof(result), 0);
				bool ret = false;

				if(id >= 10000)
				{
					std::string name_ = "None";
					std::string school_ = "None";
					std::string text_ = "I am login! Talk with me...";
					unsigned int id_ = result;
					unsigned int type = LOGIN_TYPE;
					Message m(name_, school_, text_, id_, type);
					std::string sendString;
					m.ToSendString(sendString);	//序列化
					Udp_Send(sendString);
					ret = true;
					std::cout << "Longin  Success!"  << std::endl;
				}
				else
				{
					std::cout << "Login Failed! Code is :" << result  <<std::endl;
				}

				close(tcp_sock);
				return ret;
			}

		}
		static void *Welcome(void *arg)
		{
			pthread_detach(pthread_self());
			Window *wp = (Window*)arg;
			wp->Welcome();
		}
		void Udp_Send(const std::string &message)
		{
			Util::SendMessage(udp_sock, message, server);
		}
		void Udp_Recv(std::string &message)
		{
			struct sockaddr_in peer;
			Util::RecvMessage(udp_sock, message, peer);
		}
		static void *Input(void* arg)
		{
			pthread_detach(pthread_self());
			struct Pap *pp = (struct Pap*)arg;
			Window *wp = pp->wp;
			ChatClient *cp = pp->cp;

			wp->DrawInput();
			std::string text;
			for(;;)
			{
				wp->GetStringFromInput(text);
				Message msg(cp->nick_name, cp->school, text, cp->id);
				std::string sendString;
				msg.ToSendString(sendString);
				cp->Udp_Send(sendString);
			}
		}
		void Chat()
		{
			Window w;
			pthread_t h,m;
			struct Pap pp = {&w, this};

			pthread_create(&h, NULL, Welcome, &w);
			pthread_create(&m, NULL, Input, &pp);

			w.DrawOutput();
			w.DrawOnline();

			std::string showString;
			std::string recvString;
			std::vector<std::string> online;
			for(;;)
			{
				Message message;
				Udp_Recv(recvString);
				message.ToRecvValue(recvString);
				if(message.Id() == id && message.Type() == LOGIN_TYPE)
				{
					nick_name = message.NickName();
					school = message.School();
				}
				showString = message.NickName();
				showString += "-";
				showString += message.School();

				std::string user = showString;
				Util::addUser(online, user);

				showString += "#";
				showString += message.Text();
				w.PutMessageToOutput(showString);

				w.PutUserOnline(online);

			}
		}
		~ChatClient()
		{}
};
		//	tcp_sock = SocketApi::Socket(SOCK_STREAM);
