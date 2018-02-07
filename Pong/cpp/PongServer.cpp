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
	server.left.x = PADDLE_LEFT_X;
	server.left.y = (TABLE_HEIGHT / 2) - (PADDLE_HEIGHT / 2);
	server.right.x = PADDLE_RIGHT_X;
	server.right.y = server.left.y;

	server.ball.x = server.left.x + PADDLE_WIDTH;
	server.ball.y = server.left.y + (PADDLE_HEIGHT / 2) - (BALL_SIZE / 2);
	server.ball.xv = 3;
	server.ball.yv = 0;

	server.last = std::chrono::high_resolution_clock::now();

	if(!server.accept()) // accept 2 clients
		return;

	// main loop
	while(server.running)
	{
		server.recv(); // receive data

		server.step(); // process game

		server.send(); // send data

		server.wait(); // wait until time to do next loop
	}
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

void PongServer::recv()
{
	unsigned char dgram[IN_DATAGRAM_SIZE];

	while(udp.peek() >= IN_DATAGRAM_SIZE)
	{
		Paddle pdl;
		std::uint32_t id;
		net::udp_id uid;

		udp.recv(dgram, IN_DATAGRAM_SIZE, uid);
		decompose(dgram, pdl, id);

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
	}
}

void PongServer::send()
{
	unsigned char dgram[OUT_DATAGRAM_SIZE];

	for(unsigned i = 0; i < 2; ++i)
	{
		if(!udpid[i].initialized)
			continue;

		compose(dgram, i == 0 ? right : left, ball);
		udp.send(dgram, OUT_DATAGRAM_SIZE, udpid[i]);
	}
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
	}
	if(collide(right, ball))
	{
		ball.xv = -ball.xv;
		ball.x = right.x - BALL_SIZE;
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

// encode a datagram
void PongServer::compose(std::uint8_t *raw, const Paddle &paddle, const Ball &ball)
{
	const std::int16_t paddle_y = paddle.y;
	const std::int16_t ball_x = ball.x;
	const std::int16_t ball_y = ball.y;

	memcpy(raw, &paddle_y, sizeof(paddle_y));
	memcpy(raw + 2, &ball_x, sizeof(ball_x));
	memcpy(raw + 4, &ball_y, sizeof(ball_y));
}

// decode a datagram
void PongServer::decompose(const std::uint8_t *raw, Paddle &paddle, std::uint32_t &id)
{
	std::uint32_t cid;
	std::uint16_t paddle_y;

	memcpy(&cid, raw, sizeof(cid));
	memcpy(&paddle_y, raw + 4, sizeof(paddle_y));

	id = cid;
	paddle.y = paddle_y;
}

bool PongServer::collide(const Paddle &p, const Ball &b)
{
	return p.x + PADDLE_WIDTH > b.x && p.x < b.x + BALL_SIZE && p.y + PADDLE_HEIGHT > b.y && p.y < b.y + BALL_SIZE;
}
