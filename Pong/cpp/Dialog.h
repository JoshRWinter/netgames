#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QTimer>
#include <QLineEdit>

#include "Pong.h"

namespace dlg
{
	class Greeter : public QDialog
	{
	public:
		Greeter();
		std::string get()const;
		Difficulty single_player()const;

	private:

		QLineEdit *connectto;
		Difficulty diff;
	};

	class Connecting : public QDialog
	{
	public:
		Connecting(Pong&, const std::string&, bool);

	private:
		void connect();

		QTimer *timer;
		Pong &pong;
		const int begin;
		const std::string &ip;
	};

	class AIDifficulty : public QDialog
	{
	public:
		AIDifficulty(QWidget*);
		Difficulty get()const;

	public:
		Difficulty diff;
	};

}

#endif // DIALOG_H
