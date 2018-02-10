#include <iostream>
#include <thread>
#include <chrono>

#include <math.h>

#include "PongBot.h"

PongBot::PongBot(Difficulty d)
	:seed(time(NULL))
	,params(get_difficulty_params(d))
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

DifficultyParameters PongBot::get_difficulty_params(Difficulty diff)
{
	switch(diff)
	{
	case Difficulty::EASY:
		return DifficultyParameters(2, 9);
	case Difficulty::HARD:
		return DifficultyParameters(4, 11);
	case Difficulty::IMPOSSIBLE:
		return DifficultyParameters(100, 1);
	default:
		throw std::runtime_error("no difficulty");
	}
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
#ifdef _WIN32
		target = rand() % PADDLE_HEIGHT;
		speed = params.min_speed + (rand() % params.speed_mod);
#else
		target = rand_r(&seed) % PADDLE_HEIGHT;
		speed = params.min_speed + (rand_r(&seed) % params.speed_mod);
#endif // _WIN32
	}

	// move the paddle
	move(&paddle.y, (ball.y + (BALL_SIZE / 2)) - target, speed);
	pong.set_y(paddle.y);
}
