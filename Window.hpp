#pragma once

#include <iostream>
#include <ncurses.h>
#include <unistd.h>
#include <cstring>
#include <string>
#include <pthread.h>
#include <vector>

class Window
{
	public:
		WINDOW *header;
		WINDOW *output;
		WINDOW *online;
		WINDOW *input;
		pthread_mutex_t lock;
	public:
		Window()
		{
			initscr();
			curs_set(0);
			pthread_mutex_init(&lock, NULL);
		}
		void SafeWrefresh(WINDOW *w)
		{
			pthread_mutex_lock(&lock);
			wrefresh(w);
			pthread_mutex_unlock(&lock);
		}
		void DrawHeader()
		{
			int h = LINES*0.2;
			int w = COLS;
			int y = 0;
			int x = 0;
			header = newwin(h, w, y, x);
			box(header,0, 0);
			SafeWrefresh(header);
		}
		void DrawOutput()
		{
			int h = LINES*0.5;
			int w = COLS*0.8;
			int y = LINES*0.2;
			int x = 0;
			output = newwin(h, w, y, x);
			box(output,0, 0);
			SafeWrefresh(output);
		}
		void DrawOnline()
		{
			int h = LINES*0.5;
			int w = COLS*0.2;
			int y = LINES*0.2;
			int x = COLS*0.8;
			online = newwin(h, w, y, x);
			box(online,0, 0);
			SafeWrefresh(online);
		}
		void DrawInput()
		{
			int h = LINES*0.3;
			int w = COLS;
			int y = LINES*0.7;
			int x = 0;
			input = newwin(h, w, y, x);
			box(input,0, 0);
			std::string tips = "Please Enter>";
			PutStringToWin(input, 3, 2, tips);
			SafeWrefresh(input);
		}
		void PutStringToWin(WINDOW *w, int y, int x, const std::string &message)
		{
			mvwaddstr(w, y, x, message.c_str());
			SafeWrefresh(w);
		}
		void GetStringFromInput(std::string &message)
		{
			char arr[1024];
			memset(arr, 0, sizeof(arr));
			wgetnstr(input, arr, sizeof(arr));
			message = arr;
			delwin(input);
			DrawInput();
		}
		void PutMessageToOutput(const std::string &message)
		{
			static int line = 1;
			int y,x;
			getmaxyx(output, y, x);
			if(line+1 >= y)
			{
				delwin(output);
				line = 1;
				DrawOutput();
			}
			PutStringToWin(output, line++, 2, message);
		}
		void PutUserOnline(std::vector<std::string> &onluser)
		{
			int size = onluser.size();
			for(auto i = 0; i < size; ++i)
				PutStringToWin(online, i+1, 2, onluser[i]);
		}
		void Welcome()
		{
			std::string welcome = "hello world";
			int dir = 0;
			int x, y;
			int num = 1;
			for(;;)
			{
				DrawHeader();
				getmaxyx(header,y,x);
				PutStringToWin(header, y/2, num, welcome);
				if(dir == 0)
					num++;
				else
					num--;
				if(num > COLS-welcome.size()-3)
					dir = 1;
				if(num < 2)
					dir = 0;
				usleep(100000);
				delwin(header);
			}
		}
		~Window()
		{
			endwin();
			pthread_mutex_destroy(&lock);
		}
};
