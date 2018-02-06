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

	private:
		QLineEdit *connectto;
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

}

#endif // DIALOG_H
