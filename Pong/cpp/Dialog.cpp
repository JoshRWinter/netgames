#include <QFormLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QLabel>

#include "Dialog.h"

dlg::Greeter::Greeter()
{
	// layouts
	auto form = new QFormLayout;
	auto hbox = new QHBoxLayout;
	auto vbox = new QVBoxLayout;
	setLayout(vbox);

	// line edits
	connectto = new QLineEdit;

	// buttons
	auto connect = new QPushButton("Connect");
	auto cancel = new QPushButton("Cancel");

	// listeners
	QObject::connect(connect, &QPushButton::clicked, this, &QDialog::accept);
	QObject::connect(cancel, &QPushButton::clicked, this, &QDialog::reject);

	form->addRow("Connect To", connectto);
	vbox->addLayout(form);

	hbox->addWidget(connect);
	hbox->addWidget(cancel);
	vbox->addLayout(hbox);
};

std::string dlg::Greeter::get() const
{
	return connectto->text().toStdString();
}

dlg::Connecting::Connecting(Pong &p, const std::string &addr, bool listening)
	:pong(p)
	,begin(time(NULL))
	,ip(addr)
{
	if(!pong.begin_connect(ip))
	{
		QMessageBox::critical(this, "Error", ("Could not connect to \"" + ip + "\"").c_str());
		reject();
		throw std::runtime_error("invalid ip address");
	}

	const char *const message = listening ? "Listening..." : "Connecting...";
	resize(400, 250);
	setWindowTitle(message);
	auto layout = new QVBoxLayout;
	setLayout(layout);

	layout->addWidget(new QLabel(message));
	timer = new QTimer(this);
	QObject::connect(timer, &QTimer::timeout, this, &dlg::Connecting::connect);
	timer->start(10);
}

void dlg::Connecting::connect()
{
	if(pong.connect())
	{
		timer->stop();
		accept();
	}
	else if(time(NULL) - begin > 10)
	{
		timer->stop();
		QMessageBox::critical(this, "Error", ("Could not connect to \"" + ip + "\"").c_str());
		reject();
	}
}
