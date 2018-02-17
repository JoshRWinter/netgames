#ifndef GAME_H
#define GAME_H

#include <QWidget>
#include <QPaintEvent>
#include <QTimer>
#include <QEvent>

#include "Pong.h"

class WinEvent : public QEvent
{
public:
	WinEvent(Win w)
		:QEvent((QEvent::Type)QEvent::registerEventType())
		,winner(w) {}

	const Win winner;
};

class Game : public QWidget
{
public:
	Game(Pong&);

private:
	void step();
	void customEvent(QEvent*);
	void paintEvent(QPaintEvent*);
	void mouseMoveEvent(QMouseEvent*);
	void keyPressEvent(QKeyEvent*);

	QTimer *timer;
	Pong &pong;
	int y;
};

#endif // GAME_H
