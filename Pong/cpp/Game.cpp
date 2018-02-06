#include <QWidget>
#include <QApplication>
#include <QPainter>

#include "Game.h"

Game::Game(Pong &p)
	:pong(p)
	,y(0)
{
	setWindowTitle("Pong Qt");
	resize(TABLE_WIDTH, TABLE_HEIGHT);

	// track the mouse
	setMouseTracking(true);

	// 60 fps
	timer = new QTimer(this);
	QObject::connect(timer, &QTimer::timeout, this, &Game::step);
	timer->start(17);

	// background color
	QPalette palette;
	palette.setColor(backgroundRole(), QColor(100, 100, 100));
	setPalette(palette);
	setAutoFillBackground(true);
}

void Game::step()
{
	pong.recv();
	pong.send();

	pong.set_y(y);

	repaint();
}

void Game::paintEvent(QPaintEvent*)
{
	QPainter painter(this);

	Paddle left, right;
	Ball ball;
	pong.get(left, right, ball);

	const QColor paddle_color(50, 50, 50);

	// draw paddles
	painter.fillRect(left.x, left.y, PADDLE_WIDTH, PADDLE_HEIGHT, paddle_color);
	painter.fillRect(right.x, right.y, PADDLE_WIDTH, PADDLE_HEIGHT, paddle_color);

	// ball
	painter.setBrush(QBrush(QColor(0, 0, 0)));
	painter.drawEllipse(ball.x, ball.y, BALL_SIZE, BALL_SIZE);
}

void Game::mouseMoveEvent(QMouseEvent *event)
{
	y = event->y();
}

