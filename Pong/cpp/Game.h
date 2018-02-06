#ifndef GAME_H
#define GAME_H

#include <QWidget>
#include <QPaintEvent>
#include <QTimer>

#include "Pong.h"

class Game : public QWidget
{
public:
	Game(Pong&);

private:
	void step();
	void paintEvent(QPaintEvent*);
	void mouseMoveEvent(QMouseEvent*);

	QTimer *timer;
	Pong &pong;
	int y;
};

#endif // GAME_H
