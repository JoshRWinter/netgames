#include <iostream>
#include <thread>
#include <chrono>

#include "PongBot.h"

PongBot::PongBot()
	:running(true)
	,service(PongBot::loop, this)
{}

PongBot::~PongBot()
{
	running = false;
	service.join();
}

void PongBot::loop(PongBot *p)
{
	PongBot &bot = *p;

	if(!bot.pong.begin_connect("127.0.0.1"))
	{
		std::cout << "bot: could not initialize connection" << std::endl;
		return;
	}

	const int start = time(NULL);
	bool connected = false;
	while(!connected && bot.running && time(NULL) - start < 5)
		connected = bot.pong.connect();

	if(!connected || !bot.running)
	{
		std::cout << "bot: could not connect to localhost" << std::endl;
		return;
	}

	while(bot.running)
	{
		bot.pong.recv();

		if(bot.pong.timeout())
			return;

		Paddle left, right;
		Ball ball;
		unsigned char score[2];
		bot.pong.get(left, right, ball, score);

		bot.pong.set_y(ball.y + (BALL_SIZE / 2));

		bot.pong.send();

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}
