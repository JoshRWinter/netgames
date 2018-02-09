#include <QFormLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QLabel>

#include "Dialog.h"

dlg::Greeter::Greeter()
{
	splayer = false;
	setWindowTitle("Pong Qt by Josh Winter");

	// layouts
	auto hbox = new QHBoxLayout;
	auto vbox = new QVBoxLayout;
	setLayout(vbox);

	// line edits
	connectto = new QLineEdit;

	// buttons
	auto connect = new QPushButton("Go");
	connect->setMaximumWidth(40);
	auto host = new QPushButton("Host Match");
	auto bot = new QPushButton("Single Player");

	// tool tip text
	connectto->setToolTip("Address of a remote host");
	connect->setToolTip("Connect to a host");
	host->setToolTip("Listen for connections");
	bot->setToolTip("Play against a bot");

	// listeners
	QObject::connect(connect, &QPushButton::clicked, [this]
	{
		if(connectto->text().length() == 0)
			QMessageBox::critical(this, "No Address", "You cannot leave the Address field blank.");
		else
			accept();
	});

	QObject::connect(host, &QPushButton::clicked, [this]
	{
		connectto->clear();
		accept();
	});

	QObject::connect(bot, &QPushButton::clicked, [this]
	{
		connectto->clear();
		splayer = true;
		accept();
	});

	hbox->addWidget(new QLabel("Connect to:"));
	hbox->addWidget(connectto);
	hbox->addWidget(connect);

	vbox->addLayout(hbox);
	vbox->addWidget(host);
	vbox->addWidget(bot);
};

std::string dlg::Greeter::get() const
{
	return connectto->text().toStdString();
}

bool dlg::Greeter::single_player() const
{
	return splayer;
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
