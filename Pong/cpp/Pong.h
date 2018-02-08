#ifndef PONG_H
#define PONG_H

#include "PongServer.h"

class Pong
{
public:
	Pong();
	bool begin_connect(const std::string&);
	bool connect();
	void set_y(int);
	void get(Paddle&, Paddle&, Ball&, unsigned char*);
	void request_pause();
	bool paused()const;
	bool ready()const;
	bool timeout()const;
	void recv();
	void send();

private:
	Paddle left,right;
	Ball ball;
	bool userpause, serverpause;
	bool playing;
	int last_recv;

	std::uint8_t left_score, right_score;
	std::uint32_t id;
	std::uint8_t side;
	net::tcp tcp;
	net::udp udp;
};

#endif // PONG_H
