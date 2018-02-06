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
	void get(Paddle&, Paddle&, Ball&);
	void recv();
	void send();

private:
	Paddle left,right;
	Ball ball;

	std::uint32_t id;
	std::uint8_t side;
	net::tcp tcp;
	net::udp udp;
};

#endif // PONG_H
