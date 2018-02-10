#include <memory>
#include <exception>
#include <thread>
#include <chrono>

#include <time.h>
#include <stdlib.h>

#include <QApplication>
#include <QMessageBox>

#include "PongBot.h"
#include "Dialog.h"
#include "Game.h"

int run(int, char**);
void wait(int);

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, INT)
{
	int argc = 0;
	char **argv = NULL;
#else
int main(int argc, char **argv)
{
#endif // _WIN32
	try
	{
		return run(argc, argv);
	}catch(const std::exception &e)
	{
		QMessageBox::critical(NULL, "Fatal Error", e.what());
		return 1;
	}
}

int run(int argc, char **argv)
{
	srand(time(NULL));
	QApplication app(argc, argv);

	std::unique_ptr<PongServer> server;
	std::unique_ptr<PongBot> bot;

	dlg::Greeter greeter;
	if(!greeter.exec())
		return 1;

	const std::string connectto = greeter.get();
	if(connectto.length() == 0) // maybe start the server if user wants to host a match
	{
		server.reset(new PongServer);
		wait(100); // give a lil time for the server to warm up

		Difficulty diff;
		if((diff = greeter.single_player()) != Difficulty::NONE) // start the bot if necessary
		{
			bot.reset(new PongBot(diff));
			wait(300); // give the bot some time to warm up
		}
	}

	Pong pong; // game backend
	const std::string addr = connectto.length() > 0 ? connectto : "127.0.0.1";
	dlg::Connecting connecting(pong, addr, connectto.length() == 0);
	if(!connecting.exec())
		return 1;

	Game game(pong);
	game.show();

	return app.exec();
}

void wait(int mills)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(mills));
}
