#include <QWidget>
#include <QApplication>
#include <QPainter>
#include <QMessageBox>

#include "Game.h"

Game::Game(Pong &p)
	:pong(p)
	,y(0)
{
	setWindowTitle("Pong Qt");
	resize(TABLE_WIDTH, TABLE_HEIGHT);
	setFixedSize(size());

	// track the mouse
	setMouseTracking(true);

	// 60 fps
	timer = new QTimer(this);
	QObject::connect(timer, &QTimer::timeout, this, &Game::step);
	timer->start(17);

	// background color
	QPalette palette;
	palette.setColor(backgroundRole(), QColor(70, 130, 180));
	setPalette(palette);
	setAutoFillBackground(true);
}

void Game::step()
{
	pong.recv();

	// check for timeout
	if(pong.timeout())
	{
		QMessageBox::critical(this, "Alert", "Match has ended");
		QApplication::quit();
	}

	// check for win
	const Win winner = pong.check_win();
	if(winner != Win::NONE)
	{
		QApplication::postEvent(this, new WinEvent(winner));
		timer->stop();
	}

	pong.set_y(y - (PADDLE_HEIGHT / 2));

	pong.send();

	repaint();
}

void Game::customEvent(QEvent *e)
{
	WinEvent *event = dynamic_cast<WinEvent*>(e);
	if(event == NULL)
		return;

	switch(event->winner)
	{
		case Win::ME:
			QMessageBox::information(this, "You Win", "Congratulations! You Win!");
			break;
		case Win::OPPONENT:
			QMessageBox::information(this, "You Lose", "Congratulations! You Lose!");
			break;
		default: break;
	}

	QApplication::quit();
}

void Game::paintEvent(QPaintEvent*)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);

	Paddle left, right;
	Ball ball;
	unsigned char scores[2];
	pong.get(left, right, ball, scores);

	const QColor paddle_color(220, 220, 220);

	// draw paddles
	const float fraction = 5.0f;
	QRect lpaddle = QRect(left.x + (PADDLE_WIDTH * ((fraction - 1) / fraction)), left.y, PADDLE_WIDTH / fraction, PADDLE_HEIGHT);
	QRect rpaddle = QRect(right.x, right.y, PADDLE_WIDTH / fraction, PADDLE_HEIGHT);
	painter.fillRect(lpaddle.x(), lpaddle.y(), lpaddle.width(), lpaddle.height(), paddle_color);
	painter.fillRect(rpaddle.x(), rpaddle.y(), rpaddle.width(), rpaddle.height(), paddle_color);
	painter.setBrush(QColor(paddle_color));
	painter.setPen(Qt::NoPen);
	painter.drawEllipse(lpaddle.x() - lpaddle.width(), lpaddle.y(), lpaddle.width() * 2, lpaddle.height());
	painter.drawEllipse(rpaddle.x(), rpaddle.y(), rpaddle.width() * 2, rpaddle.height());

	// ball
	painter.setBrush(QBrush(QColor(220, 220, 220)));
	painter.drawEllipse(ball.x, ball.y, BALL_SIZE, BALL_SIZE);

	// dotted line
	QPen pen(Qt::DashLine);
	pen.setColor(QColor(220, 220, 220));
	painter.setPen(pen);
	painter.drawLine(TABLE_WIDTH / 2, 0, TABLE_WIDTH / 2, TABLE_HEIGHT);

	// scores
	const int textheight = 90;
	const QFont font("sans serif", 40, 10);
	painter.setFont(font);
	painter.drawText(0, 10, (TABLE_WIDTH / 2) - 20, textheight, Qt::AlignRight, std::to_string(int(scores[0])).c_str());
	painter.drawText((TABLE_WIDTH / 2) + 20, 10, TABLE_WIDTH / 2, textheight, Qt::AlignLeft, std::to_string(int(scores[1])).c_str());

	// pause screen
	if(pong.paused() || !pong.ready())
	{
		const char *const msg = pong.paused() ? "PAUSED" : "WAITING FOR PLAYER 2";

		painter.setBrush(QBrush(QColor(10, 10, 10, 120)));
		painter.drawRect(0, 0, TABLE_WIDTH, TABLE_HEIGHT);
		painter.setPen(QColor(255, 255, 255, 255));
		painter.drawText(0, 0, TABLE_WIDTH, TABLE_HEIGHT, Qt::AlignCenter, msg);
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
