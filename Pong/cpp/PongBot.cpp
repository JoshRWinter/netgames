#include <iostream>
#include <thread>
#include <chrono>

#include <math.h>

#include "PongBot.h"

PongBot::PongBot()
	:seed(time(NULL))
	,target(0)
	,speed(0)
	,toleft(false)
	,running(true)
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

		bot.ai(left, ball);

		bot.pong.send();

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void PongBot::move(float *origin, int dest, int speed)
{
	if(*origin > dest)
	{
		(*origin) -= speed;
		if(*origin < dest)
			*origin = dest;
	}
	else if(*origin < dest)
	{
		(*origin) += speed;
		if(*origin > dest)
			*origin = dest;
	}
}

void PongBot::ai(Paddle &paddle, const Ball &ball)
{
	const bool before = toleft;
	toleft = ball.xv < 0;

	if(!toleft)
		return;

	if(before != toleft)
	{
		// select a new target and speed
		target = rand_r(&seed) % PADDLE_HEIGHT;
		speed = 4 + (rand_r(&seed) % 10);
	}

	// move the paddle
	move(&paddle.y, (ball.y + (BALL_SIZE / 2)) - target, speed);
	pong.set_y(paddle.y);
}
