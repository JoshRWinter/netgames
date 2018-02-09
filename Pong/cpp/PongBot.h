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

	Pong pong;
	std::atomic<bool> running;
	std::thread service;
};

#endif // PONG_BOT_H
