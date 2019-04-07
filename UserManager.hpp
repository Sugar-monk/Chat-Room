#pragma once

#include <unordered_map>
#include <iostream>
#include <string>
//#include <pthread>

class User
{
	private:
		std::string nick_name;
		std::string school;
		std::string passwd;
		unsigned int id;
	public:
		User()
		{}
	
		User(const std::string &nick_name_, const std::string &school_, const std::string &passwd_):
			nick_name(nick_name_),
			school(school_),
			passwd(passwd_)
		{}
		bool IsPasswdOk(const std::string &passwd_)
		{
			return passwd == passwd_ ? true : false;
		}
		std::string &GetNickName()
		{
			return nick_name;
		}
		std::string &GetSchool()
		{
			return school;
		}
		~User()
		{}

};


class UserManager
{
	private:
		unsigned int user_id;		//用户ID
		//需求：将用户存储起来，当需要的时候可以将用户调用起来（登录成功要收发消息）
		std::unordered_map<unsigned int, User> users;
		std::unordered_map<unsigned int, struct sockaddr_in> online_users;  //用户在线信息
		pthread_mutex_t lock;

		void Lock()
		{	
			pthread_mutex_lock(&lock);
		}
		void unLock()
		{
			pthread_mutex_unlock(&lock);
		}
	public:
		UserManager():user_id(10000)
		{
			pthread_mutex_init(&lock, NULL);
		}
		
		unsigned int Insert(const std::string &n_,
								const std::string &s_,
									const std::string &p_)
		{
			Lock();
			/*待升级项目，随机ID
			 * srand(unsigned time(NULL));
			*/
			unsigned int id = user_id++;
			User u(n_, s_, p_);
			//在用户表中查找当前ID，如果没有找到，就进行插入返回ID,如果有就返回1
			if(users.find(id) == users.end())
			{
				users.insert(make_pair(id, u));
				//users.inster({id, u});
				unLock();
				return id;
			}
			unLock();
			return 1;
		}
		unsigned int Check(const unsigned int &id, const std::string &passwd)
		{
			Lock();
			//在用户列表中查找用户是否存在，如果存在就校验密码
			auto it	= users.find(id);
			if(it != users.end())		
			{
				User &u = it->second;
				if(u.IsPasswdOk(passwd))
				{
					unLock();
					return id;
				}
			}
			unLock();
			return 2;
		}
		void AddOnlineUser(unsigned int id, struct sockaddr_in &peer)
		{
			Lock();
			auto it = online_users.find(id);
			if(it == online_users.end())
			{
				online_users.insert({id, peer});
			}
			unLock();
		}
		std::unordered_map<unsigned int, struct sockaddr_in> OnlineUser()
		{
			Lock();
			auto online = online_users;
			unLock();
			return online;
		}
		void GetUserInfo(const unsigned int &id, std::string &name_, 
				std::string &school_)
		{
			Lock();
			name_ = users[id].GetNickName();
			school_ = users[id].GetSchool();
			unLock();
		}

		~UserManager()
		{
			pthread_mutex_destroy(&lock);
		}


};
