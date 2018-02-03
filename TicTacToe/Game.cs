using System;
using System.Net;
using System.Net.Sockets;
using System.IO;
using System.Text;

namespace TicTacToe
{
    internal class Game
    {
        internal const int PORT = 3005;
        internal const int NONE = 0;
        internal const int X = 1;
        internal const int O = 2;

        TcpClient tcp;
        BinaryWriter tcpOut;
        BinaryReader tcpIn;
        bool hosting;

        internal string OpponentName { get; private set; }
        internal string MyName { get; private set; }
        internal bool IsMyTurn { get; private set; }
        internal byte[] Table { get; private set; }

        internal Game(string name)
        {
            MyName = name;
            tcp = null;
            Table = new byte[9];
            for (int i = 0; i < Table.Length; ++i)
            {
                Table[i] = NONE;
            }
        }

        internal void Disconnect()
        {
            tcp.Close();
        }

        internal bool Connect(string ipaddr)
        {
            tcp = new TcpClient(ipaddr, PORT);
            bool connected = tcp.Connected;
            if (connected)
            {
                tcpOut = new BinaryWriter(tcp.GetStream());
                tcpIn = new BinaryReader(tcp.GetStream());

                // tell my name
                SendString(MyName);
                // get their name
                OpponentName = GetString();
                // opponent is hosting
                hosting = false;
                IsMyTurn = true;
            }

            return connected;
        }

        internal void Listen()
        {
            TcpListener listener = new TcpListener(IPAddress.Any, PORT);
            listener.Start();

            while (true)
            {
                if (listener.Pending())
                {
                    tcp = listener.AcceptTcpClient();
                    listener.Stop();

                    tcpOut = new BinaryWriter(tcp.GetStream());
                    tcpIn = new BinaryReader(tcp.GetStream());

                    // get their name
                    OpponentName = GetString();
                    // tell my name
                    SendString(MyName);
                    // i am hosting
                    hosting = true;
                    IsMyTurn = false;
                    return;
                }
            }
        }

        // return X or O
        internal int GetLetter()
        {
            return hosting ? X : O;
        }

        internal void MyTurn(int index)
        {
            index = index - 1;

            if (Table[index] != NONE)
            {
                throw new TicTacToeException("That slot has previously been selected!");
            }

            // tell the opponent my desired index
            if (index < 0 || index > 8)
            {
                throw new TicTacToeException("<index> must be [0, 8]");
            }

            Table[index] = (byte)GetLetter();

            tcpOut.Write((byte)index);

            IsMyTurn = !IsMyTurn;
        }

        internal void OpponentTurn()
        {
            // read the selected index from the opponent
            byte b = tcpIn.ReadByte();
            if (b < 0 || b > 8)
                throw new TicTacToeException("got byte " + b + " from opponent. They are trying to hack you!!!!!");

            Table[b] = (byte)(GetLetter() == X ? O : X);

            IsMyTurn = !IsMyTurn;
        }

        internal bool CheckMeWin()
        {
            return CheckWin(GetLetter());
        }

        internal bool CheckOpponentWin()
        {
            return CheckWin(GetLetter() == X ? O : X);
        }

        private bool CheckWin(int letter)
        {
            // rows
            if (Table[6] == letter && Table[7] == letter && Table[8] == letter)
                return true;
            if (Table[3] == letter && Table[4] == letter && Table[5] == letter)
                return true;
            if (Table[0] == letter && Table[1] == letter && Table[2] == letter)
                return true;

            // columns
            if (Table[6] == letter && Table[3] == letter && Table[0] == letter)
                return true;
            if (Table[7] == letter && Table[4] == letter && Table[1] == letter)
                return true;
            if (Table[8] == letter && Table[5] == letter && Table[2] == letter)
                return true;

            // diagnals
            if (Table[6] == letter && Table[4] == letter && Table[2] == letter)
                return true;
            if (Table[8] == letter && Table[4] == letter && Table[0] == letter)
                return true;

            return false;
        }

        private string GetString()
        {
            Int32 len = tcpIn.ReadInt32();
            byte[] data = tcpIn.ReadBytes(len);

            return Encoding.ASCII.GetString(data);
        }

        private void SendString(string str)
        {
            tcpOut.Write((Int32)str.Length);
            byte[] data = Encoding.ASCII.GetBytes(str);
            tcpOut.Write(data);
        }
    }

    internal class TicTacToeException : Exception
    {
        public TicTacToeException(string msg)
        {
            Message = msg;
        }

        public override string Message { get; }
    }
}
