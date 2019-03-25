#pragma once

#include<iostream>
#include<string>

using namespace std;

#define NORMAL 0
#define WARNING 1
#define ERROR 2


//为了方便识别错误级别，将系统返回的0,1,2转换为方便识别的英文
const char *log_level[]={

	"Normal",
	"Warning",
	"Mrror",
	NULL,
};

void Log(string msg, int level, string file, int line )
{
	cout << '[' << msg << ']' << '[' << log_level[level] << ']' << " : " << \
		file << " : " << line << " : " << line << " : " <<endl;

}

#define LOG(msg,level) Log(msg, level, __FILE__, __LINE__)		
//定义一个宏，方便使用，就可以在使用时直接调用LOG，不用传文件地址和第几行，
//直接用c语言中已经定义了的两个宏
