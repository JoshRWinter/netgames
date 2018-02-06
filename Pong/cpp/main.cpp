#include <memory>

#include <time.h>
#include <stdlib.h>

#include <QApplication>
#include <QMessageBox>

#include "Dialog.h"
#include "Game.h"

int main(int argc, char **argv)
{
	srand(time(NULL));
	QApplication app(argc, argv);
	std::unique_ptr<PongServer> server;

	dlg::Greeter greeter;
	if(!greeter.exec())
		return 1;

	const std::string connectto = greeter.get();
	if(connectto.length() == 0) // maybe start the server if user wants to host a match
	{
		try
		{
			server.reset(new PongServer);
		}catch(const std::exception &e)
		{
			QMessageBox::critical(NULL, "Server error", e.what());
			return 1;
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
