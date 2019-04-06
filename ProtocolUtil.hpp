#pragma once

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <json/json.h>
#include <sstream>
#include "Log.hpp"
#include <vector>

		
#define BACKLOG 5
#define MESSAGE_SIZE 1024

//listen的第二个参数：底层最大连接数+1
class Request
{
	public:
		std::string method;		//请求方法
		std::string content_length; 
		std::string blank;
		std::string text;

	public:
		Request():blank("\n")
		{}
		~Request()
		{}
};
class Util		//最基本的工具类
{
	public:
	static bool RegisterEnter(std::string &n_, std::string &s_, std::string &passwd)
	{
		std::cout << "Please Enter Nick Name:";
		std::cin  >> n_;
		std::cout << "Please Enter School:";
		std::cin  >> s_;
		std::cout << "Please Enter Passwd: ";
		std::cin  >> passwd;
		std::string again;
		std::cout << "Please Enter Passwd Again : ";
		std::cin  >> again;
	//	while(1)		//扩展成如果两次密码不同，重新输入密码
		if(passwd == again)
		{
			return true;
		}
		return false;
	}

	static void Seralizer(Json::Value &root,std::string &outString)
	{
		Json::FastWriter w;
		outString = w.write(root);
	}
	static void UnSeralizer(std::string &inString, Json::Value &root)
	{
		Json::Reader r;
		r.parse(inString, root, false);
	}
	static bool	LoginEnter(unsigned int &id, std::string &passwd)
	{
		std::cout << "Please Enter Your ID:";
		std::cin  >> id;
		std::cout << "Please Enter Your Passwd:";
		std::cin  >> passwd;
		//扩展密码确认
		return true;
	}
	static std::string InToString(int n)
	{
		std::stringstream ss;
		ss << n;
		return ss.str();
	}	
	static int StringToInt(std::string &str)
	{
		std::stringstream ss(str);
		int n;
		ss >> n;
		return n;
	}
	static void RecvOneLine(int sock, std::string &outString)
	{
		char c = 'x';
		while(c != '\n')
		{
			ssize_t s = recv(sock, &c, 1, 0);
			if(s > 0)
			{
				if(c == '\n')
				{
					break;
				}
				outString.push_back(c);
			}
			else
				break;
		}
	}

	//TCP
	static void RecvRequest(int sock,Request &rq)
	{
		RecvOneLine(sock, rq.method);			//方法
		RecvOneLine(sock, rq.content_length);	//长度
		RecvOneLine(sock, rq.blank);			//空行

		std::string &cl = rq.content_length;
		std::size_t pos = cl.find(": ");
		if(std::string::npos == pos)		//没找到正文
		{
			return;
		}
		std::string sub = cl.substr(pos+2);
		int size = StringToInt(sub);
		char c;
		for(auto i = 0; i < size; ++i)
		{
			recv(sock, &c, 1, 0);
			(rq.text).push_back(c);
		}
	}
	static void SendRequest(int sock,Request &rq)
	{
		std::string &m_ = rq.method;
		std::string &cl_ = rq.content_length;
		std::string &te_ = rq.text;
		std::string &b_ = rq.blank;

		send(sock, m_.c_str(), m_.size(), 0);
		send(sock, cl_.c_str(), cl_.size(), 0);
		send(sock, b_.c_str(), b_.size(), 0);
		send(sock, te_.c_str(), te_.size(), 0);
	}
	//UDP使用
	static void RecvMessage(int sock,std::string &message, struct sockaddr_in &peer)
	{
		char msg[MESSAGE_SIZE];
		socklen_t len = sizeof(peer);
		ssize_t s = recvfrom(sock, msg, sizeof(msg)-1,0, (struct sockaddr*)&peer, &len);
		if(s <= 0)
		{
			LOG("recvfrom message error",WARNING);
		}
		else
		{
			message = msg;
		}
	}

	static void SendMessage(int sock,const std::string &message, const sockaddr_in &peer)
	{
		//报错flag-1
		socklen_t len = sizeof(peer);
		sendto(sock, message.c_str(), message.size(),
				0, (struct sockaddr*)&peer, len);
	}
	static void addUser(std::vector<std::string> &online, std::string &user)
	{
		for(auto it = online.begin(); it != online.end(); ++it)
		{
			if(*it == user)
				return;
		}
		online.push_back(user);
	}

};


class SocketApi
{
	public:
		static int Socket(int type)						//创建套接字
		{
			int sock = socket(AF_INET, type ,0);
			if(sock < 0)									//说明创建套接字失败,输出失败的级别和位置
			{
				LOG("socket error", ERROR);
				exit(2);
			}
		}

		static void Bind(int sock, int port)				//绑定套接字和端口号
		{
			struct sockaddr_in local;	
			local.sin_family = AF_INET;						//将sockaddr,填充
			local.sin_addr.s_addr = htonl(INADDR_ANY);		//将点分十进制转换成对应的四字节IP地址
			local.sin_port = htons(port);

			if(bind(sock, (struct sockaddr*)&local, sizeof(local)) < 0)		
			{
				LOG("socket error", ERROR);
				exit(3);
			}
		}

		static void Listen(int sock)						//监听套接字
		{
			if(listen(sock,BACKLOG) < 0)
			{
				LOG("socket error", ERROR);
				exit(4);
			}
		}

		static int Accept(int listen_sock, std::string &out_ip, int &out_port)  //获取新链接
		{
			struct sockaddr_in peer;							//远端信息
			socklen_t len = sizeof(peer);
			int sock = accept(listen_sock, (struct sockaddr*)&peer, &len);
			if(sock < 0)
			{
				LOG("accept error", WARNING);
				return -1;
			}
			out_ip = inet_ntoa(peer.sin_addr);					//将收到的网络IP地址由四字节IP转换为容易接受的点分十进制IP
			out_port = htons(peer.sin_port);
			return sock;
		}
		static bool Connect(const int &sock, std::string peer_ip, const int &port)
		{
			struct sockaddr_in peer;							 //要连接的地址
			peer.sin_family = AF_INET;
			peer.sin_addr.s_addr = inet_addr(peer_ip.c_str());  //将字符串风格的IP地址转换为四字节IP地址
			peer.sin_port = htons(port);						//将主机IP地址转换为网络IP地址

			if(connect(sock,(struct sockaddr*)&peer, sizeof(peer)) < 0)
			{
				LOG("connect error", WARNING);
				return false;
			}

			return true;
		}

};
