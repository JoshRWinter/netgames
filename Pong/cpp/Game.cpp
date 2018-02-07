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
	unsigned char scores[2];
	pong.get(left, right, ball, scores);

	const QColor paddle_color(50, 50, 50);

	// draw paddles
	painter.fillRect(left.x, left.y, PADDLE_WIDTH, PADDLE_HEIGHT, paddle_color);
	painter.fillRect(right.x, right.y, PADDLE_WIDTH, PADDLE_HEIGHT, paddle_color);

	// ball
	painter.setBrush(QBrush(QColor(0, 0, 0)));
	painter.drawEllipse(ball.x, ball.y, BALL_SIZE, BALL_SIZE);

	// dotted line
	painter.setPen(Qt::DashLine);
	painter.drawLine(TABLE_WIDTH / 2, 0, TABLE_WIDTH / 2, TABLE_HEIGHT);

	// scores
	const int textheight = 90;
	const QFont font("sans serif", 40, 10);
	painter.setFont(font);
	painter.drawText(0, 10, (TABLE_WIDTH / 2) - 20, textheight, Qt::AlignRight, std::to_string(int(scores[0])).c_str());
	painter.drawText((TABLE_WIDTH / 2) + 20, 10, TABLE_WIDTH / 2, textheight, Qt::AlignLeft, std::to_string(int(scores[1])).c_str());

	// pause screen
	if(pong.paused())
	{
		painter.setBrush(QBrush(QColor(10, 10, 10, 100)));
		painter.drawRect(0, 0, TABLE_WIDTH, TABLE_HEIGHT);
		painter.drawText(0, 0, TABLE_WIDTH, TABLE_HEIGHT, Qt::AlignCenter, "PAUSED");
	}
}

void Game::mouseMoveEvent(QMouseEvent *event)
{
	y = event->y();
}

void Game::keyPressEvent(QKeyEvent *event)
{
	const int key = event->key();
	if(key == Qt::Key_Escape)
		pong.request_pause();
}
