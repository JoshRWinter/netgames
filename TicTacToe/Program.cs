using System;
using System.IO;

namespace TicTacToe
{
    class Program
    {
        static void Main(string[] args)
        {
            try
            {
                Run();
            }
            catch (Exception e)
            {
                Console.WriteLine("Error: " + e.Message);
            }
        }

        private static void Run()
        {
            Console.Write("Enter your name: ");
            string myName = Console.ReadLine();

            Game game = new Game(myName);

            Console.Write("Enter an IP Address, or leave blank to listen: ");
            string ip = Console.ReadLine();
            if (ip.Length == 0)
            {
                Console.WriteLine("Listening...");
                game.Listen();
            }
            else
            {
                if (!game.Connect(ip))
                {
                    Console.WriteLine("Could not connect to " + ip + " on port " + Game.PORT);
                    return;
                }
            }

            while (Play(game)) ;
        }

        private static bool Play(Game game)
        {
            Draw(game.Table);

            try
            {
                if (game.IsMyTurn)
                {
                    Console.Write("Select a slot: ");
                    int slot = int.Parse(Console.ReadLine());
                    game.MyTurn(slot);
                }
                else
                {
                    game.OpponentTurn();
                }
            }
            catch (IOException)
            {
                Console.WriteLine("\n\nA network error occured");
                return false;
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
                return Play(game);
            }

            return true;
        }

        private static void Draw(byte[] board)
        {
            Console.WriteLine("-------------");
            Console.WriteLine($"| {GetChar(board[6])} | {GetChar(board[7])} | {GetChar(board[8])} |");
            Console.WriteLine("-------------");
            Console.WriteLine($"| {GetChar(board[3])} | {GetChar(board[4])} | {GetChar(board[5])} |");
            Console.WriteLine("-------------");
            Console.WriteLine($"| {GetChar(board[0])} | {GetChar(board[1])} | {GetChar(board[2])} |");
            Console.WriteLine("-------------");
        }

        private static char GetChar(int letter)
        {
            switch (letter)
            {
                case Game.O:
                    return 'O';
                case Game.X:
                    return 'X';
                case Game.NONE:
                    return ' ';
            }

            return '?';
        }
    }
}
