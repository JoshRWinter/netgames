#include <functional>

#include <string.h>
#include <time.h>

#include "PongServer.h"

PongServer::PongServer()
	:udp(PONG_PORT)
	,tcp(PONG_PORT)
	,running(true)
	,service(PongServer::loop, this)
{
	if(!tcp || !udp)
	{
		running = false;
		service.join();
		if(!tcp)
			throw PongServerException("could not bind to port tcp:" + std::to_string(PONG_PORT));
		if(!udp)
			throw PongServerException("could not bind to port udp:" + std::to_string(PONG_PORT));
	}
}

PongServer::~PongServer()
{
	running = false;
	service.join();
}

void PongServer::send_data(net::tcp &stream, const void *buf, int len)
{
	int bytes = 0;

	while(bytes != len)
	{
		const int result = stream.send_nonblock((char*)buf + bytes, len - bytes);
		if(result == -1)
			return;

		bytes += result;
	}
}

void PongServer::recv_data(net::tcp &stream, void *buf, int len)
{
	memset(buf, 0, len);
	int bytes = 0;
	const int start = time(NULL);

	while(bytes != len && time(NULL) - start < 5)
	{
		const int result = stream.recv_nonblock((char*)buf + bytes, len - bytes);
		if(result == -1)
			return;

		bytes += result;
	}
}

void PongServer::loop(PongServer *p)
{
	PongServer &server = *p;

	// init some game entities
	server.reset(true);
	server.ball.y = (TABLE_HEIGHT / 2) - (BALL_SIZE / 2);
	server.left.paddle.x = PADDLE_LEFT_X;
	server.right.paddle.x = PADDLE_RIGHT_X;

	server.last = std::chrono::high_resolution_clock::now();
	server.left.pause_request = server.right.pause_request = false;

	if(!server.accept()) // accept 2 clients
		return;

	// main loop
	while(server.running)
	{
		server.recv(); // receive data

		if(server.check_timeout()) // see if anyone has timed out
			return;

		if(!server.left.pause_request && !server.right.pause_request)
			server.step(); // process game

		server.send(); // send data

		server.wait(); // wait until time to do next loop
	}
}

float PongServer::calculate_angle(int at)
{
	return (at - (PADDLE_HEIGHT / 2)) / 100.0;
}

// accept 2 clients over tcp
bool PongServer::accept()
{
	for(int i = 0; i < 2; ++i)
	{
		Client &client = i == 0 ? left : right;

		int sock = -1;
		while(sock == -1 && running)
			sock = tcp.accept();
		if(!running)
			return false;
		net::tcp stream = sock;

		// send id
		client.cid = rand() % 100000;
		send_data(stream, &client.cid, sizeof(client.cid));
		const uint8_t side = i + 1;
		send_data(stream, &side, sizeof(side));
	}

	return true;
}

bool PongServer::check_timeout() const
{
	if(left.last_recv == 0 || right.last_recv == 0)
		return false;

	const int current = time(NULL);

	if(current - left.last_recv > PONG_TIMEOUT)
		return true;
	else if(current - right.last_recv > PONG_TIMEOUT)
		return true;

	return false;
}

// bool is which side is "serving" the ball
void PongServer::reset(bool hostserve)
{
	if(hostserve)
	{
		ball.x = left.paddle.x + PADDLE_WIDTH;
		ball.y = left.paddle.y + (PADDLE_HEIGHT / 2) - (BALL_SIZE / 2);
		ball.speed = BALL_START_SPEED;
		ball.set_angle(0);
	}
	else
	{
		ball.x = right.paddle.x - BALL_SIZE;
		ball.y = right.paddle.y + (PADDLE_HEIGHT / 2) - (BALL_SIZE / 2);
		ball.speed = BALL_START_SPEED;
		ball.set_angle(M_PI);
	}

	ball.yv = 0;
}

void PongServer::step()
{
	// update ball pos
	ball.x += ball.xv;
	ball.y += ball.yv;

	// check for paddle collision
	if(collide(left.paddle, ball))
	{
		ball.x = left.paddle.x + PADDLE_WIDTH;
		ball.set_angle(calculate_angle((ball.y + (BALL_SIZE / 2)) - left.paddle.y));
	}
	if(collide(right.paddle, ball))
	{
		ball.x = right.paddle.x - BALL_SIZE;
		ball.set_angle(calculate_angle((ball.y + (BALL_SIZE / 2)) - right.paddle.y));
		ball.xv = -ball.xv;
	}

	// bounce off the top and bottom of the screen
	if(ball.y < 0)
	{
		ball.y = 0;
		ball.yv = -ball.yv;
	}
	else if(ball.y + BALL_SIZE > TABLE_HEIGHT)
	{
		ball.y = TABLE_HEIGHT - BALL_SIZE;
		ball.yv = -ball.yv;
	}

	// check for win condition
	if(ball.x + BALL_SIZE > TABLE_WIDTH + 250)
	{
		++left.score;
		reset(true);
	}
	else if(ball.x < -250)
	{
		++right.score;
		reset(false);
	}
}

void PongServer::recv()
{
	unsigned char dgram[IN_DATAGRAM_SIZE];

	while(udp.peek() >= IN_DATAGRAM_SIZE)
	{
		Paddle pdl;
		net::udp_id uid;

		udp.recv(dgram, IN_DATAGRAM_SIZE, uid);

		// decompose the datagram
		std::uint32_t id;
		std::int16_t paddle_y;
		std::uint8_t request_pause;

		memcpy(&id, dgram, sizeof(id));
		memcpy(&paddle_y, dgram + 4, sizeof(paddle_y));
		memcpy(&request_pause, dgram + 6, sizeof(request_pause));

		pdl.y = paddle_y;

		if(id == left.cid)
		{
			if(!left.udpid.initialized)
				left.udpid = uid;

			left.last_recv = time(NULL);

			left.paddle.y = pdl.y;

			left.pause_request = request_pause == 1;
		}
		else if(id == right.cid)
		{
			if(!right.udpid.initialized)
				right.udpid = uid;

			right.last_recv = time(NULL);

			right.paddle.y = pdl.y;

			right.pause_request = request_pause == 1;
		}
		else
		{
			continue;
		}
	}
}

void PongServer::send()
{
	unsigned char dgram[OUT_DATAGRAM_SIZE];

	for(unsigned i = 0; i < 2; ++i)
	{
		const Client &client = i == 0 ? left : right;
		const Paddle &paddle = i == 0 ? right.paddle : left.paddle;

		if(!client.udpid.initialized)
			continue;

		// compose the datagram
		const std::int16_t paddle_y = paddle.y;
		const std::int16_t ball_x = ball.x;
		const std::int16_t ball_y = ball.y;
		const std::uint8_t left_score = left.score;
		const std::uint8_t right_score = right.score;
		const std::uint8_t pause = left.pause_request || right.pause_request ? 1 : 0;

		memcpy(dgram, &paddle_y, sizeof(paddle_y));
		memcpy(dgram + 2, &ball_x, sizeof(ball_x));
		memcpy(dgram + 4, &ball_y, sizeof(ball_y));
		memcpy(dgram + 6, &left_score, sizeof(left_score));
		memcpy(dgram + 7, &right_score, sizeof(right_score));
		memcpy(dgram + 8, &pause, sizeof(pause));

		// send
		udp.send(dgram, sizeof(dgram), client.udpid);
	}
}

void PongServer::wait()
{
	std::chrono::duration<long long, std::nano> diff;
	auto current = std::chrono::high_resolution_clock::now();

	do
	{
		current = std::chrono::high_resolution_clock::now();
		diff = current - last;
	}while(diff.count() < 16666000);

	last = current;
}

bool PongServer::collide(const Paddle &p, const Ball &b)
{
	return p.x + PADDLE_WIDTH > b.x && p.x < b.x + BALL_SIZE && p.y + PADDLE_HEIGHT > b.y && p.y < b.y + BALL_SIZE;
}
