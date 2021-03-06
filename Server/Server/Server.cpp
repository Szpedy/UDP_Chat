#include "Server.h"
std::vector<clientStruct> Clients;
bool open_chat = false; // czy chat pomiedzy klientami jest otwarty

std::string getTime() {
	time_t now = time(0);
	tm *ltm = localtime(&now);
	std::string czas;
	if (ltm->tm_mday < 10)
		czas += "0";
	czas += std::to_string(ltm->tm_mday) + "-";
	if (ltm->tm_mon + 1 < 10)
		czas += "0";
	czas += std::to_string(ltm->tm_mon + 1) + "-";
	czas += std::to_string(ltm->tm_year + 1900) + " ";

	if (ltm->tm_hour < 10)
		czas += "0";
	czas += std::to_string(ltm->tm_hour) + ":";
	if (ltm->tm_min + 1 < 10)
		czas += "0";
	czas += std::to_string(ltm->tm_min);
	return czas;
}

std::string getID() {
	std::random_device rd;
	std::default_random_engine generator(rd());
	std::string ID;
	for (int i = 0; i < 10; i++) {
		std::uniform_int_distribution<int> distribution(0, 10);
		if (distribution(generator) > 3) {
			//losujemy literke
			std::uniform_int_distribution<int> distribution(97, 122);
			ID += distribution(generator);
		}
		else
		{
			//losujemy cyfre
			std::uniform_int_distribution<int> distribution(48, 57);
			ID += distribution(generator);
		}
	}
	return ID;
}

std::string Clientinfo(sockaddr_in Client) {
	char ipAddress[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &Client.sin_addr, ipAddress, INET_ADDRSTRLEN);
	return ipAddress;
}

void ReadMessage(packetStruct& PS, std::string packet) {
	std::string help;
	while (packet.find('!|') != std::string::npos) {
		help = packet.substr(0, packet.find('!|') + 1);
		if (help.find("Czas") != std::string::npos) {
			PS.Czas = help.substr(6, help.size() - 8);
		}
		if (help.find("Operacja") != std::string::npos) {
			PS.Operacja = help.substr(10, help.size() - 12);
		}
		if (help.find("Status") != std::string::npos) {
			PS.Status = help.substr(8, help.size() - 10);
		}
		if (help.find("Identyfikator") != std::string::npos) {
			PS.Identyfikator = help.substr(15, help.size() - 17);
		}
		if (help.find("Tekst") != std::string::npos) {
			PS.Tekst = help.substr(7, help.size() - 9);
		}

		if (help.find("NSekwencyjny") != std::string::npos) {
			PS.NSekwencyjny = help.substr(14, help.size() - 16);
		}
		packet.erase(0, packet.find('!|') + 1);
	}
}

void ZeroPS(packetStruct& PS) {
	PS.Operacja = "";
	PS.Identyfikator = "";
	PS.NSekwencyjny = "";
	PS.Operacja = "";
	PS.Status = "";
	PS.Tekst = "";
}

void WriteMessage(packetStruct& PS, std::string& packet) {
	PS.Czas = getTime();
	packet = "Czas+!" + PS.Czas;
	if (PS.Operacja.size()) {
		packet += "!|";
		packet += "Operacja+!";
		packet += PS.Operacja;
	}
	if (PS.Status.size()) {
		packet += "!|";
		packet += "Status+!";
		packet += PS.Status;
	}
	if (PS.Identyfikator.size()) {
		packet += "!|";
		packet += "Identyfikator+!";
		packet += PS.Identyfikator;
	}
	if (PS.Tekst.size()) {
		packet += "!|";
		packet += "Tekst+!";
		packet += PS.Tekst;
	}
	if (PS.NSekwencyjny.size()) {
		packet += "!|";
		packet += "NSekwencyjny+!";
		packet += PS.NSekwencyjny;
	}
	//switch (atoi(PS.NSekwencyjny.c_str()))
	packet += "!|";
}

//wyswietlanie pakietu
void printMSG(const packetStruct &PS) {
	if (PS.Operacja.size()) {
		std::cout << "Operacja: ";
		std::cout << PS.Operacja << std::endl;
	}
	if (PS.Status.size()) {
		std::cout << "Status: ";
		std::cout << PS.Status << std::endl;
	}
	if (PS.Identyfikator.size()) {
		std::cout << "Identyfikator: ";
		std::cout << PS.Identyfikator << std::endl;
	}
	if (PS.Tekst.size()) {
		std::cout << "Tekst: ";
		std::cout << PS.Tekst << std::endl;
	}
	if (PS.Czas.size()) {
		std::cout << "Czas: ";
		std::cout << PS.Czas << std::endl;
	}
	if (PS.NSekwencyjny.size()) {
		std::cout << "NSekwencyjny: ";
		std::cout << PS.NSekwencyjny << std::endl;
	}
	std::cout << std::endl;
}

// Odbiór pakietu
int RecievePacket(SOCKET s, sockaddr_in &RecvAddr, packetStruct &PS, std::string& packet, int RecvAddrSize) {
	std::cout << "Odbieram dane.........\n";
	char buf[1024];
	packet = "";
	int iResult = recvfrom(s, buf, 1024, 0, (SOCKADDR *)& RecvAddr, &RecvAddrSize);
	if (iResult == SOCKET_ERROR) {
		return -1;
	}

	for (int i = 0; i < iResult; i++) {
		packet.push_back(buf[i]);
	}
	std::cout << "Odebralem: " << iResult << " bajtow" << std::endl;
	std::cout << packet << std::endl;

	return 1;
}

//Wysylanie
int SendPacket(SOCKET s, sockaddr_in SendAddr, packetStruct &PS, std::string& packet) {
	std::cout << "WYSLALEM WIADOMOSC" << std::endl;
	WriteMessage(PS, packet);
	int iResult = sendto(s, packet.c_str(), packet.length(), 0, (SOCKADDR *)& SendAddr, 16);
	if (iResult == SOCKET_ERROR) {
		throw 0;
	}
	ZeroPS(PS);
	return iResult;
}

int SendAck(SOCKET s, sockaddr_in SendAddr, packetStruct PS, std::string packet) {
	std::cout << "Wysylam potwierdzenie" << std::endl;
	PS.Operacja = "";
	PS.Tekst = "";
	PS.Status = "ACK";
	WriteMessage(PS, packet);
	int iResult = sendto(s, packet.c_str(), packet.length(), 0, (SOCKADDR *)& SendAddr, 16);
	if (iResult == SOCKET_ERROR) {
		throw 0;
	}
	return iResult;
}

int recipient(int who) {
	if (who == 0)
		return 1;
	else
		return 0;
}

void handlePacket(SOCKET s, sockaddr_in Client, std::string& packet, packetStruct &PS, int who) {
	ReadMessage(PS, packet);
	//moge przetworzyc zadanie
	if (PS.NSekwencyjny == "0") {
		if (PS.Status != "ACK")
			SendAck(s, Client, PS, packet);

		if (PS.Operacja == "Polaczenie" && PS.Status == "REQ_ID") {
			std::cout << "WYSLE ID KLIENTOWI" << std::endl;
			ZeroPS(PS);
			PS.Operacja = "Polaczenie";
			PS.NSekwencyjny = "2";
			SendPacket(s, Client, PS, packet);
			PS.NSekwencyjny = "1";
			PS.Status = "OK_ID";
			SendPacket(s, Client, PS, packet);
			PS.NSekwencyjny = "0";
			PS.Tekst = Clients[who].ID;
			SendPacket(s, Client, PS, packet);
		}

		if (PS.Operacja == "Rozmowa" && PS.Status == "INVITE") {
			// nie ma drugiego klienta
			if (Clients.size() < 2) {
				ZeroPS(PS);
				PS.Identyfikator = Clients[who].ID;
				PS.Operacja = "Rozmowa";
				PS.NSekwencyjny = "1";
				SendPacket(s, Client, PS, packet);
				PS.Identyfikator = Clients[who].ID;
				PS.Status = "NO_CLIENT";
				PS.NSekwencyjny = "0";
				SendPacket(s, Client, PS, packet);
			}
			else {
				int rec = recipient(who);
				ZeroPS(PS);
				PS.Identyfikator = Clients[rec].ID;
				PS.Operacja = "Rozmowa";
				PS.NSekwencyjny = "1";
				SendPacket(s, Clients[rec].addr, PS, packet);
				PS.Identyfikator = Clients[rec].ID;
				PS.Status = "INVITED";
				PS.NSekwencyjny = "0";
				SendPacket(s, Clients[rec].addr, PS, packet);
			}
		}

		if (PS.Operacja == "Rozmowa" && PS.Status == "ACC_INVITE") {
			open_chat = true;
			int rec = recipient(who);
			ZeroPS(PS);
			PS.Identyfikator = Clients[rec].ID;
			PS.Operacja = "Rozmowa";
			PS.NSekwencyjny = "1";
			SendPacket(s, Clients[rec].addr, PS, packet);
			PS.Identyfikator = Clients[rec].ID;
			PS.Status = "ACC_INVITE";
			PS.NSekwencyjny = "0";
			SendPacket(s, Clients[rec].addr, PS, packet);
		}

		if (PS.Operacja == "Rozmowa" && PS.Status == "DEC_INVITE") {
			int rec = recipient(who);
			ZeroPS(PS);
			PS.Identyfikator = Clients[rec].ID;
			PS.Operacja = "Rozmowa";
			PS.NSekwencyjny = "1";
			SendPacket(s, Clients[rec].addr, PS, packet);

			PS.Identyfikator = Clients[rec].ID;
			PS.Status = "DEC_INVITE";
			PS.NSekwencyjny = "0";
			SendPacket(s, Clients[rec].addr, PS, packet);
		}

		if (PS.Operacja == "Wiadomosc" && PS.Status == "OK_MSG" && open_chat == true) {
			// Przesylam wiadomosc drugiemu klientowi
			std::string msg = PS.Tekst;
			int rec = recipient(who);
			ZeroPS(PS);

			PS.Identyfikator = Clients[rec].ID;
			PS.Operacja = "Wiadomosc";
			PS.NSekwencyjny = "2";
			SendPacket(s, Clients[rec].addr, PS, packet);

			PS.Identyfikator = Clients[rec].ID;
			PS.Status = "OK_MSG";
			PS.NSekwencyjny = "1";
			SendPacket(s, Clients[rec].addr, PS, packet);

			PS.Identyfikator = Clients[rec].ID;
			PS.Tekst = msg;
			PS.NSekwencyjny = "0";
			SendPacket(s, Clients[rec].addr, PS, packet);

		}

		if (PS.Operacja == "Zakonczenie" && PS.Status == "DC_SERVER") {
			ZeroPS(PS);
			PS.Operacja = "Zakonczenie";
			PS.NSekwencyjny = "1";
			SendPacket(s, Client, PS, packet);

			PS.NSekwencyjny = "0";
			PS.Status = "OK_DC";
			SendPacket(s, Client, PS, packet);
			std::cout << "Klient o ID: " << Clients[who].ID << " rozlaczyl sie z serwerem!" << std::endl;
			Clients.erase(Clients.begin() + who);
			open_chat = false;
		}

		//Przeslanie klientowi informacji ze drugi klient zakonczyl z nim polaczenie
		if (PS.Operacja == "Zakonczenie" && PS.Status == "DC_CHAT") {
			open_chat = false;
			ZeroPS(PS);
			int rec = recipient(who);
			PS.Operacja = "Zakonczenie";
			PS.NSekwencyjny = "1";
			SendPacket(s, Clients[rec].addr, PS, packet);
			PS.NSekwencyjny = "0";
			PS.Status = "DC_CHAT";
			SendPacket(s, Clients[rec].addr, PS, packet);
		}

		if (PS.Status == "ACK")
			ZeroPS(PS);
	}
	else {
		std::cout << "Czekam na sekwencje 0!!" << std::endl;
		if (PS.Status != "ACK")
			SendAck(s, Client, PS, packet);
	}
}

int whoSentDatagram(std::vector<clientStruct>& Clients, sockaddr_in Client, packetStruct PS, std::string packet) {
	int i = 0;
	ReadMessage(PS, packet);
	for (i = 0; i < Clients.size(); i++) {
		if (PS.Identyfikator == Clients[i].ID || (Client.sin_port == Clients[i].addr.sin_port && Clientinfo(Client) == Clientinfo(Clients[i].addr))) {
			return i;
		}
	}
	//Nowy klient
	if (Clients.size() < 2) {
		if (PS.Status == "ACK") //ACK od rozlaczonego klienta
			return -1;
		std::cout << "NOWY KLIENT!" << std::endl;
		std::string ID = getID();
		clientStruct cs;
		cs.addr = Client;
		cs.ID = ID;
		cs.PS = packetStruct();
		Clients.push_back(cs);
		return i;
	}

	std::cout << "Serwer jest pelny wiadomosc od trzeciego gracza " << i << std::endl;
	return i;
}

int main() {
	int iResult = 0;
	WSADATA wsaData;

	SOCKET RecvSocket;
	unsigned short Port = 64000;
	std::string packet;

	//ustawienia klientow
	sockaddr_in RecvAddr;
	int ClientAddrSize = sizeof(RecvAddr);
	packetStruct PS;
	//-----------------------------------------------
	// Inicjalizacja
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		printf("WSAStartup failed with error %d\n", iResult);
		return 1;
	}

	// Tworze gniazdo
	RecvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (RecvSocket == INVALID_SOCKET) {
		printf("socket failed with error %d\n", WSAGetLastError());
		return 1;
	}

	//Przypisuje do gniazda adres ip oraz port
	RecvAddr.sin_family = AF_INET;
	RecvAddr.sin_port = htons(Port);
	RecvAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	iResult = ::bind(RecvSocket, (SOCKADDR *)& RecvAddr, sizeof(RecvAddr));
	if (iResult != 0) {
		wprintf(L"bind failed with error %d\n", WSAGetLastError());
		return 1;
	}

	//-----------------------------------------------
	// Serwer gotowy do pracy :)

	while (true) {
		RecievePacket(RecvSocket, RecvAddr, PS, packet, ClientAddrSize);
		int Client = whoSentDatagram(Clients, RecvAddr, PS, packet);

		if (Client > 1) {
			std::cout << "Trzeci klient" << std::endl;
			continue;
		}

		if (Client == -1) //Nieznany|rozlaczony klient
			continue;

		handlePacket(RecvSocket, Clients[Client].addr, packet, Clients[Client].PS, Client);
	}

	//-----------------------------------------------
	// Zamykanie gniazda
	printf("Finished working. Closing socket.\n");
	iResult = closesocket(RecvSocket);
	if (iResult == SOCKET_ERROR) {
		wprintf(L"closesocket failed with error %d\n", WSAGetLastError());
		return 1;
	}

	printf("Exiting.\n");
	WSACleanup();
	system("PAUSE");
	return 0;
}