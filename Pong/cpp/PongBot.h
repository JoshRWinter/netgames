#ifndef PONG_BOT_H
#define PONG_BOT_H

#include <atomic>
#include <thread>

#include "Pong.h"

class PongBot
{
public:
	PongBot();
	~PongBot();

private:
	static void loop(PongBot*);
	static void move(float*, int, int);
	void ai(Paddle&, const Ball&);

	unsigned seed;

	// ai parameters
	int target; // bot will try to hit this part of the paddle
	int speed; // bot will move this speed
	bool toleft; // direction

	Pong pong;
	std::atomic<bool> running;
	std::thread service;
};

#endif // PONG_BOT_H
