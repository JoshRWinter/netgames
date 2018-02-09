#ifndef PONG_SERVER_H
#define PONG_SERVER_H

#include <thread>
#include <atomic>
#include <exception>
#include <vector>
#include <chrono>

#ifdef _WIN32
#define _USE_MATH_DEFINES
#endif // _WIN32
#include <math.h>

#include "network.h"

/* OUTGOING DATAGRAM
0 : int16 paddle y position
1 : ..
2 : int16 ball x position
3 : ..
4 : int16 ball y position
5 : ..
6 : int16 ball x velocity
7 : ..
8 : int16 ball y velocity
9 : ..
10: uint8 left player score
11: uint8 right player score
12: uint8 paused boolean
*/

/* INCOMING DATAGRAM
0 : uint32 udp id
1 : ..
2 : ..
3 : ..
4 : int16 paddle x pos
5 : ..
6 : uint8 request paused
*/

#define PONG_PORT 28850
#define OUT_DATAGRAM_SIZE 13
#define IN_DATAGRAM_SIZE 7
#define PONG_TIMEOUT 2 // seconds

#define TABLE_WIDTH 1100
#define TABLE_HEIGHT 700

#define SIDE_LEFT 1
#define SIDE_RIGHT 2

#define PADDLE_LEFT_X PADDLE_WIDTH
#define PADDLE_RIGHT_X (TABLE_WIDTH - PADDLE_WIDTH - PADDLE_WIDTH)

#define BALL_SIZE 20
#define BALL_START_SPEED 18.0f
#define BALL_MAX_SPEED 35.0f
#define BALL_SPEEDUP 0.4f
struct Ball
{
	Ball()
		:x(0), y(0)
		,xv(0), yv(0)
		,angle(0), speed(BALL_START_SPEED) {}

	void set_angle(float ang)
	{
		angle = ang;
		xv = cos(angle) * speed;
		yv = sin(angle) * speed;
	}

	void set_speed(float spd)
	{
		speed = spd;
		set_angle(angle);
	}

	float x, y;
	float xv, yv;
	float angle;
	float speed;
};

#define PADDLE_HEIGHT 180
#define PADDLE_WIDTH 30
struct Paddle
{
	Paddle()
		:x(0), y(0) {}

	float x, y;
};

class PongServerException : public std::exception
{
public:
	PongServerException(const std::string &m)
		:message(m)
	{}

	virtual const char *what()const noexcept
	{
		return message.c_str();
	}

private:
	const std::string message;
};

struct Client
{
public:
	Client():
	cid(0), score(0), pause_request(false), last_recv(0) {}

	Paddle paddle;
	std::uint32_t cid;
	net::udp_id udpid;
	unsigned char score;
	bool pause_request;
	int last_recv;
};

#define NORMAL_GAME_TIMER 70
#define START_GAME_TIMER 150
class PongServer
{
public:
	PongServer();
	PongServer(const PongServer&) = delete;
	~PongServer();

	static void recv_data(net::tcp&, void*, int);
	static void send_data(net::tcp&, const void*, int);

private:
	static void loop(PongServer*);
	static float calculate_angle(int);
	bool accept();
	bool check_timeout()const;
	void reset(bool);
	void step();
	void recv();
	void send();
	void wait();
	bool collide(const Paddle&, const Ball&);
	int game_timer;

	net::udp_server udp;
	net::tcp_server tcp;

	std::chrono::time_point<std::chrono::high_resolution_clock> last;

	// client info
	Client left, right;
	Ball ball;

	std::atomic<bool> running;
	std::thread service;
};

#endif // PONG_SERVER_H
