#include <QFormLayout>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QLabel>

#include "Dialog.h"
#include "PongServer.h"

dlg::Greeter::Greeter()
{
	diff = Difficulty::NONE;

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
		AIDifficulty aidiff(this);
		if(aidiff.exec())
		{
			diff = aidiff.get();
			connectto->clear();
			accept();
		}
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

Difficulty dlg::Greeter::single_player() const
{
	return diff;
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

dlg::AIDifficulty::AIDifficulty(QWidget *parent)
	:QDialog(parent)
	,diff(Difficulty::NONE)
{
	setWindowTitle("Choose Difficulty");
	resize(200, 0);

	auto vbox = new QVBoxLayout;
	setLayout(vbox);

	// buttons
	auto easy = new QPushButton("Easy");
	auto hard = new QPushButton("Hard");
	auto impossible = new QPushButton("Impossible");
	auto cancel = new QPushButton("Cancel");

	// button listeners
	QObject::connect(easy, &QPushButton::clicked, [this]
	{
		diff = Difficulty::EASY;
		accept();
	});

	QObject::connect(hard, &QPushButton::clicked, [this]
	{
		diff = Difficulty::HARD;
		accept();
	});

	QObject::connect(impossible, &QPushButton::clicked, [this]
	{
		diff = Difficulty::IMPOSSIBLE;
		accept();
	});

	QPushButton::connect(cancel, &QPushButton::clicked, this, &QDialog::reject);

	vbox->addWidget(easy);
	vbox->addWidget(hard);
	vbox->addWidget(impossible);
	vbox->addWidget(cancel);
}

Difficulty dlg::AIDifficulty::get() const
{
	return diff;
}
