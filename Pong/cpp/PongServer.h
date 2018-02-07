#ifndef PONG_SERVER_H
#define PONG_SERVER_H

#include <thread>
#include <atomic>
#include <exception>
#include <vector>
#include <chrono>

#include "network.h"

/* OUTGOING DATAGRAM
0. int16 paddle y position
1. ..
2. int16 ball x position
3. ..
4. int16 ball y position
5. ..
*/

/* INCOMING DATAGRAM
0. uint32 udp id
1. ..
2. ..
3. ..
4. int16 paddle x pos
*/

#define PONG_PORT 28850
#define OUT_DATAGRAM_SIZE 6
#define IN_DATAGRAM_SIZE 6

#define TABLE_WIDTH 800
#define TABLE_HEIGHT 600

#define SIDE_LEFT 1
#define SIDE_RIGHT 2

#define PADDLE_LEFT_X PADDLE_WIDTH
#define PADDLE_RIGHT_X (TABLE_WIDTH - PADDLE_WIDTH - PADDLE_WIDTH)

#define BALL_SIZE 30
struct Ball
{
	Ball()
		:x(0), y(0)
		,xv(0), yv(0) {}

	float x, y;
	float xv, yv;
};

#define PADDLE_HEIGHT 200
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
	bool accept();
	void recv();
	void send();
	void step();
	void wait();
	void compose(std::uint8_t*, const Paddle&, const Ball&);
	void decompose(const std::uint8_t*, Paddle&, std::uint32_t&);
	bool collide(const Paddle&, const Ball&);

	net::udp_server udp;
	net::tcp_server tcp;

	Paddle left, right;
	Ball ball;
	std::chrono::time_point<std::chrono::high_resolution_clock> last;

	// client info
	std::uint32_t client_id[2];
	net::udp_id udpid[2];

	std::atomic<bool> running;
	std::thread service;

};

#endif // PONG_SERVER_H
