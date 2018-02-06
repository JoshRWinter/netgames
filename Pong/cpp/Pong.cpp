#include <time.h>
#include <string.h>

#include "Pong.h"

Pong::Pong()
{
	left.x = PADDLE_LEFT_X;
	right.x = PADDLE_RIGHT_X;
}

bool Pong::begin_connect(const std::string &ip)
{
	udp = std::move(net::udp(ip, PONG_PORT));
	tcp = std::move(net::tcp(ip, PONG_PORT));
	return !tcp.error() && !udp.error();
}

bool Pong::connect()
{
	const bool success = tcp.connect();

	if(success == false)
		return false;

	// connect handshake
	PongServer::recv_data(tcp, &id, sizeof(id));
	PongServer::recv_data(tcp, &side, sizeof(side));

	return !tcp.error();
}

void Pong::set_y(int v)
{
	(side == SIDE_LEFT ? left : right).y = v - (PADDLE_HEIGHT / 2);
}

void Pong::get(Paddle &l, Paddle &r, Ball &b)
{
	l = left;
	r = right;
	b = ball;
}

void Pong::recv()
{
	Paddle &p = side == SIDE_LEFT ? right : left;

	unsigned char dgram[OUT_DATAGRAM_SIZE];

	while(udp.peek() >= sizeof(dgram))
	{
		std::uint16_t paddle_y;
		std::uint16_t ball_x;
		std::uint16_t ball_y;

		udp.recv(dgram, sizeof(dgram));
			memcpy(&paddle_y, dgram, sizeof(paddle_y));
		memcpy(&ball_x, dgram + 2, sizeof(ball_x));
		memcpy(&ball_y, dgram + 4, sizeof(ball_y));

		p.y = paddle_y;
		ball.x = ball_x;
		ball.y = ball_y;
	}
}

void Pong::send()
{
	unsigned char dgram[IN_DATAGRAM_SIZE];

	std::uint16_t paddle_y = (side == SIDE_LEFT ? left : right).y;
	memcpy(dgram, &id, sizeof(id));
	memcpy(dgram + 4, &paddle_y, sizeof(paddle_y));

	udp.send(dgram, IN_DATAGRAM_SIZE);
}
