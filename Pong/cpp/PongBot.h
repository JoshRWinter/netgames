#ifndef PONG_BOT_H
#define PONG_BOT_H

#include <atomic>
#include <thread>

#include "Pong.h"

struct DifficultyParameters
{
	DifficultyParameters(int ms, int sm)
		:min_speed(ms)
		,speed_mod(sm) {}

	const int min_speed;
	const int speed_mod;
};

class PongBot
{
public:
	PongBot(Difficulty);
	~PongBot();

private:
	static DifficultyParameters get_difficulty_params(Difficulty);
	static void loop(PongBot*);
	static void move(float*, int, int);
	void ai(Paddle&, const Ball&);

	unsigned seed;
	const DifficultyParameters params;

	// ai parameters
	int target; // bot will try to hit this part of the paddle
	int speed; // bot will move this speed
	bool toleft; // direction

	Pong pong;
	std::atomic<bool> running;
	std::thread service;
};

#endif // PONG_BOT_H
