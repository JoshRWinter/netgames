#include <time.h>
#include <string.h>
#include <iostream>

#include "Pong.h"

Pong::Pong()
{
	left.x = PADDLE_LEFT_X;
	right.x = PADDLE_RIGHT_X;
	side = SIDE_LEFT;
	left_score = 0;
	right_score = 0;
	userpause = false;
	serverpause = false;
	playing = false;
	last_recv = 0;
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

void Pong::get(Paddle &l, Paddle &r, Ball &b, unsigned char *scores)
{
	l = left;
	r = right;
	b = ball;
	scores[0] = left_score;
	scores[1] = right_score;
}

void Pong::request_pause()
{
	userpause = !userpause;
}

bool Pong::paused() const
{
	return serverpause;
}

bool Pong::ready() const
{
	return playing;
}

bool Pong::timeout() const
{
	if(last_recv == 0)
		return false;

	return time(NULL) - last_recv > PONG_TIMEOUT;
}

void Pong::recv()
{
	Paddle &p = side == SIDE_LEFT ? right : left;

	unsigned char dgram[OUT_DATAGRAM_SIZE];

	bool got_one = false;

	while(udp.peek() >= sizeof(dgram))
	{
		got_one = true;

		playing = true;
		last_recv = time(NULL);

		std::int16_t paddle_y;
		std::int16_t ball_x;
		std::int16_t ball_y;
		std::int16_t ball_xv;
		std::int16_t ball_yv;
		std::uint8_t l_score;
		std::uint8_t r_score;
		std::uint8_t paused;

		udp.recv(dgram, sizeof(dgram));
		memcpy(&paddle_y, dgram, sizeof(paddle_y));
		memcpy(&ball_x, dgram + 2, sizeof(ball_x));
		memcpy(&ball_y, dgram + 4, sizeof(ball_y));
		memcpy(&ball_xv, dgram + 6,sizeof(ball_xv));
		memcpy(&ball_yv, dgram + 8, sizeof(ball_yv));
		memcpy(&l_score, dgram + 10, sizeof(left_score));
		memcpy(&r_score, dgram + 11, sizeof(right_score));
		memcpy(&paused, dgram + 12, sizeof(paused));

		p.y = paddle_y;
		ball.x = ball_x;
		ball.y = ball_y;
		ball.xv = ball_xv;
		ball.yv = ball_yv;
		left_score = l_score;
		right_score = r_score;
		serverpause = paused == 1;
	}

	if(!got_one)
	{
		ball.x += ball.xv;
		ball.y += ball.yv;
	}
}

void Pong::send()
{
	unsigned char dgram[IN_DATAGRAM_SIZE];

	const std::int16_t paddle_y = (side == SIDE_LEFT ? left : right).y;
	const std::uint8_t request_pause = userpause ? 1 : 0;

	memcpy(dgram, &id, sizeof(id));
	memcpy(dgram + 4, &paddle_y, sizeof(paddle_y));
	memcpy(dgram + 6, &request_pause, sizeof(request_pause));

	udp.send(dgram, IN_DATAGRAM_SIZE);
}

