#include <functional>

#include <string.h>
#include <unistd.h>
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
	server.score[0] = 0;
	server.score[1] = 0;
	server.left.x = PADDLE_LEFT_X;
	server.right.x = PADDLE_RIGHT_X;

	server.last = std::chrono::high_resolution_clock::now();
	server.pause_request[0] = server.pause_request[1] = false;

	if(!server.accept()) // accept 2 clients
		return;

	// main loop
	while(server.running)
	{
		server.recv(); // receive data

		if(!server.pause_request[0] && !server.pause_request[1])
			server.step(); // process game

		server.send(); // send data

		server.wait(); // wait until time to do next loop
	}
}

float PongServer::calculate_angle(int at)
{
	return (at - (PADDLE_HEIGHT / 2)) / 10.0f;
}

// accept 2 clients over tcp
bool PongServer::accept()
{
	for(int i = 0; i < 2; ++i)
	{
		int sock = -1;
		while(sock == -1 && running)
			sock = tcp.accept();
		if(!running)
			return false;
		net::tcp stream = sock;

		// send id
		client_id[i] = rand() % 100000;
		send_data(stream, &client_id[i], sizeof(client_id[i]));
		uint8_t side = i + 1;
		send_data(stream, &side, sizeof(side));
	}

	return true;
}

// bool is which side is "serving" the ball
void PongServer::reset(bool hostserve)
{
	if(hostserve)
	{
		ball.x = left.x + PADDLE_WIDTH;
		ball.xv = BALL_START_SPEED;
		ball.y = left.y + (PADDLE_HEIGHT / 2) - (BALL_SIZE / 2);
	}
	else
	{
		ball.x = right.x - BALL_SIZE;
		ball.xv = -BALL_START_SPEED;
		ball.y = right.y + (PADDLE_HEIGHT / 2) - (BALL_SIZE / 2);
	}

	ball.yv = 0;
}

void PongServer::step()
{
	// update ball pos
	ball.x += ball.xv;
	ball.y += ball.yv;

	// check for paddle collision
	if(collide(left, ball))
	{
		ball.xv = -ball.xv;
		ball.x = left.x + PADDLE_WIDTH;
		ball.yv = calculate_angle((ball.y + (BALL_SIZE / 2)) - left.y);
	}
	if(collide(right, ball))
	{
		ball.xv = -ball.xv;
		ball.x = right.x - BALL_SIZE;
		ball.yv = calculate_angle((ball.y + (BALL_SIZE / 2)) - right.y);
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
		++score[0];
		reset(true);
	}
	else if(ball.x < -250)
	{
		++score[1];
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

		if(id == client_id[0])
		{
			left.y = pdl.y;
			if(!udpid[0].initialized)
				udpid[0] = uid;
		}
		else if(id == client_id[1])
		{
			right.y = pdl.y;
			if(!udpid[1].initialized)
				udpid[1] = uid;
		}
		else
		{
			continue;
		}

		// handle pausing
		const int client_index = id == client_id[0] ? 0 : 1;
		pause_request[client_index] = request_pause == 1;
	}
}

void PongServer::send()
{
	unsigned char dgram[OUT_DATAGRAM_SIZE];

	for(unsigned i = 0; i < 2; ++i)
	{
		if(!udpid[i].initialized)
			continue;

		const Paddle &paddle = i == 0 ? right : left;

		// compose the datagram
		const std::int16_t paddle_y = paddle.y;
		const std::int16_t ball_x = ball.x;
		const std::int16_t ball_y = ball.y;
		const std::uint8_t left_score = score[0];
		const std::uint8_t right_score = score[1];
		const std::uint8_t pause = pause_request[0] || pause_request[1] ? 1 : 0;

		memcpy(dgram, &paddle_y, sizeof(paddle_y));
		memcpy(dgram + 2, &ball_x, sizeof(ball_x));
		memcpy(dgram + 4, &ball_y, sizeof(ball_y));
		memcpy(dgram + 6, &left_score, sizeof(left_score));
		memcpy(dgram + 7, &right_score, sizeof(right_score));
		memcpy(dgram + 8, &pause, sizeof(pause));

		// send
		udp.send(dgram, sizeof(dgram), udpid[i]);
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
