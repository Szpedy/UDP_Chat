#include "Client.h"

bool connected = false; // Polaczenie z serwerem
bool invited = false;	// Zaproszenie od drugiego klienta
int accepted = 0;		// zakceptowanie zaproszenia/odrzucenie/brak zaproszenia
std::string ID;

//Zwaraca czas i dodaje 0 gdy mamy wartosci mniejsze od 10
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
	if (ltm->tm_min < 10)
		czas += "0";
	czas += std::to_string(ltm->tm_min);
	return czas;
}
//Czytanie wiadomosci dodawanie danych do struktury
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

//Dziala odwrotnie do read
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

//Zerowanie struktury
void ZeroPS(packetStruct& PS) {
	PS.Operacja = "";
	PS.Identyfikator = "";
	PS.NSekwencyjny = "";
	PS.Operacja = "";
	PS.Status = "";
	PS.Tekst = "";
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
}

//Wysylanie pakietu zeruje strukture po wyslaniu
int SendPacket(SOCKET s, addrinfo *SendAddr, packetStruct &PS, std::string packet) {
	//std::cout << "WYSYLAM:" << std::endl;
	WriteMessage(PS, packet);
	//std::cout << packet<< std::endl;;
	int iResult = sendto(s, packet.c_str(), packet.length(), 0, SendAddr->ai_addr, SendAddr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		throw 0;
	}
	ZeroPS(PS);
	return iResult;
}

//Odbior pakietu zapisuje odebrane dane do struktury
int RecievePacket(SOCKET s, sockaddr_in &RecvAddr, packetStruct &PS, std::string packet, int RecvAddrSize) {
	char buf[1024];
	packet = "";
	int iResult = recvfrom(s, buf, 1024, 0, (SOCKADDR *)& RecvAddr, &RecvAddrSize);
	for (int i = 0; i < iResult; i++) {
		packet.push_back(buf[i]);
	}
	//std::cout << "Odebralem: " << iResult << " bajtow" << std::endl;
	//std::cout << "Otrzymana wiadomosc" << std::endl;
	//std::cout << packet << std::endl;
	ReadMessage(PS, packet);
	if (iResult == SOCKET_ERROR) {
		std::cout << "Zakonczono polaczenie!" << std::endl;
	}
	return iResult;
}

// Funkcja wysylajaca potwierdzenia przyjmuje argumenty przez wartosc aby nie nadpisac danych
int SendAck(SOCKET s, addrinfo *SendAddr, packetStruct PS, std::string packet) {
	//	std::cout << "Wysylam potwierdzenie" << std::endl;
	PS.Operacja = "";
	PS.Tekst = "";
	PS.Status = "ACK";
	WriteMessage(PS, packet);
	int iResult = sendto(s, packet.c_str(), packet.length(), 0, SendAddr->ai_addr, SendAddr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		throw 0;
	}
	return iResult;
}

//Procesudra z ktorej korzysta watek odbierajacy dane
void Recieve(SOCKET s, sockaddr_in RecvAddr, addrinfo *SendAddr, packetStruct PS, std::string packet) {
	// Odbieranie datagramu
	while (true) {
		if (RecievePacket(s, RecvAddr, PS, packet, 16) != -1) {
			if (PS.NSekwencyjny == "0") {
				if (PS.Status != "ACK") {
					SendAck(s, SendAddr, PS, packet);
				}
				//std::cout << "Przetwarzam wiadomosc!" << std::endl;
				if (PS.Status == "ACK")
					ZeroPS(PS);

				if (PS.Operacja == "Polaczenie" && PS.Status == "OK_ID") {
					PS.Identyfikator = PS.Tekst;
					ID = PS.Identyfikator;
					//	std::cout << "Moje ID: " << PS.Identyfikator << std::endl;
					ZeroPS(PS);
					connected = true;
				}
				if (PS.Operacja == "Rozmowa" && PS.Status == "NO_CLIENT") {
					std::cout << "Drugi klient nie polaczyl sie jeszcze z serwerem!" << std::endl;
					accepted = -1;
				}
				if (PS.Operacja == "Rozmowa" && PS.Status == "DEC_INVITE") {
					accepted = -1;
					if (invited == true)
						std::cout << "Uplynal limit czasu na odpowiedz!" << std::endl;
					else
						std::cout << "Drugi klient odrzucil zaproszenie" << std::endl;

					invited = false;

				}
				if (PS.Operacja == "Rozmowa" && PS.Status == "INVITED") {
					std::cout << "Zostales zaproszony do rozmowy!" << std::endl;
					invited = true;
				}
				if (PS.Operacja == "Rozmowa" && PS.Status == "ACC_INVITE") {
					std::cout << "Drugi klient przyjal zaproszenie!" << std::endl;
					std::cout << "ROZMOWA" << std::endl;
					std::cout << "Aby zakonczyc wysli wiadomosc wpisz !END!" << std::endl;
					std::cin.ignore();
					accepted = 1; invited = true;
				}
				if (PS.Operacja == "Wiadomosc" && PS.Status == "OK_MSG") {
					// wyswietlam otrzymana wiadomosc
					std::cout << "Rozmowca: " << PS.Tekst << std::endl;
					std::cout << "ME: ";
				}
				if (PS.Operacja == "Zakonczenie" && PS.Status == "DC_CHAT") {
					accepted = 0;
					invited = false;
					std::cout << "Rozmowca rozlaczyl sie z toba" << std::endl;
				}
			}
			else {
				if (PS.Status != "ACK")
					SendAck(s, SendAddr, PS, packet);
			}

		}
		else
			break;
	}
}

//delay zwraca 1 gdy przerwanie nastapilo poprzez akcje 0 jezeli przekroczono czas oczekiwania
int delay(int &value) {
	for (int i = 0;; i++) {
		if (i == 1000) {
			return 0;
		}
		if (value == 0)
			Sleep(10);

		else
			return 1;
	}
}

int main() {
	int iResult;
	WSADATA wsaData;

	SOCKET SendSocket = INVALID_SOCKET;
	sockaddr_in RecvAddr;	// struktura do odbierania
	int RecvAddrSize = sizeof(RecvAddr);
	struct addrinfo *SendAddr = NULL, hints; // struktura do wysylania

	packetStruct PS;
	std::string IP;
	std::string message;
	std::string packet;

	//-----------------------------------------------
	// Inicjalizacja

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		std::cout << "WSAStartup failed with error: %d\n", iResult;
		return 1;
	}

	// Tworzenie Socketa
	SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (SendSocket == INVALID_SOCKET) {
		//wprintf(L"socket failed with error: %ld\n", WSAGetLastError());
		std::cout << "socket failed with error: %ld\n", WSAGetLastError();
		WSACleanup();
		return 1;
	}

	// Tworzenie struktury z adresami ip
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	// Pobranie adresu IP serwera
	//std::cout << "Podaj adres IP serwera:\t";
	IP = "127.0.0.1";
	// Sprawdzanie poprawnoci adresu IP
	iResult = getaddrinfo(IP.c_str(), "64000", &hints, &SendAddr);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}
	//---------------------------------------------
	// Klient gotowy do dzialania :)

	//Wyslanie ¿¹danie ID
	PS.NSekwencyjny = "1";
	PS.Operacja = "Polaczenie";
	SendPacket(SendSocket, SendAddr, PS, packet);
	PS.NSekwencyjny = "0";
	PS.Status = "REQ_ID";
	SendPacket(SendSocket, SendAddr, PS, packet);
	std::thread reciever(Recieve, SendSocket, RecvAddr, SendAddr, PS, packet);
	reciever.detach();
	char op = '0';

	//Czekam na polaczenie gdy nie polaczy w zadanym czasie koncze program
	for (int i = 0;; i++) {
		if (i == 0) {
			std::cout << "Czekam na polaczenie z serwerem." << std::endl;
		}

		if (i == 100)
			break;

		if (!connected)
			Sleep(10);
		else
			break;
	}
	// dopoki jestesmy polaczeni do serwera
	while (connected) {
		//jezeli nie zostalismy zakceptowani przeez innego klienta mamy do wyboru az 4 opcje
		if (accepted <= 0) {
			std::cout << "MENU" << std::endl;
			std::cout << "Polacz z klientem (1)" << std::endl;
			std::cout << "Zakoncz polaczenie z serwerem (2)" << std::endl;
			std::cout << "Zakceptuj polaczenie od drugiego klienta(3)" << std::endl;
			std::cout << "Odrzuc polaczenie(4)" << std::endl;
			std::cin >> op;
			if (op == '1') {
				if (invited == true)
					std::cout << "Najpierw odpowiedz na zaproszenie ze strony drugiego klienta!" << std::endl;
				else {
					//proba polaczenie z innym klientem
					std::cout << "Proba polaczenia z drugim klientem: " << std::endl;
					accepted = 0;
					PS.Operacja = "Rozmowa";
					PS.NSekwencyjny = "1";
					PS.Identyfikator = ID;
					SendPacket(SendSocket, SendAddr, PS, packet);

					PS.Status = "INVITE";
					PS.NSekwencyjny = "0";
					PS.Identyfikator = ID;
					SendPacket(SendSocket, SendAddr, PS, packet);

					//czekam na odpowiedz
					if (delay(accepted) == 0) {
						std::cout << "Klient nie odpowiedzial na zaproszenie" << std::endl;
						//Klient nie odpowiedzial na zaproszenie w zadanym czasie wysylam do niego wiadomosc o tym
						PS.Operacja = "Rozmowa";
						PS.NSekwencyjny = "1";
						PS.Identyfikator = ID;
						SendPacket(SendSocket, SendAddr, PS, packet);

						PS.Status = "DEC_INVITE";
						PS.NSekwencyjny = "0";
						PS.Identyfikator = ID;
						SendPacket(SendSocket, SendAddr, PS, packet);
					}
				}
			}
			if (op == '2') {
				//Rozlaczyc z serwrem zwolic miejsce dla innego clienta
				PS.Operacja = "Zakonczenie";
				PS.NSekwencyjny = "1";
				PS.Identyfikator = ID;
				SendPacket(SendSocket, SendAddr, PS, packet);

				PS.Status = "DC_SERVER";
				PS.NSekwencyjny = "0";
				PS.Identyfikator = ID;
				SendPacket(SendSocket, SendAddr, PS, packet);
				connected = false;
			}
			if (op == '3') {
				if (invited == true) {
					//juz nie jestemy zaproszeni
					invited = false;
					PS.Operacja = "Rozmowa";
					PS.NSekwencyjny = "1";
					PS.Identyfikator = ID;
					SendPacket(SendSocket, SendAddr, PS, packet);

					PS.Status = "ACC_INVITE";
					PS.NSekwencyjny = "0";
					PS.Identyfikator = ID;
					SendPacket(SendSocket, SendAddr, PS, packet);
					//zakceptowalismy zaproszenie wiec zaczynamy chat
					accepted = 1;
					std::cout << "ROZMOWA" << std::endl;
					std::cout << "Aby zakonczyc wysli wiadomosc wpisz !END!" << std::endl;
					std::cin.ignore();
				}
				else {
					std::cout << "Brak zaproszen" << std::endl;
				}
			}
			if (op == '4') {
				if (invited == true) {
					// nie jestesmy juz zaproszeni
					invited = false;
					PS.Operacja = "Rozmowa";
					PS.NSekwencyjny = "1";
					PS.Identyfikator = ID;
					SendPacket(SendSocket, SendAddr, PS, packet);

					PS.Status = "DEC_INVITE";
					PS.NSekwencyjny = "0";
					PS.Identyfikator = ID;
					SendPacket(SendSocket, SendAddr, PS, packet);
				}
				else {
					std::cout << "Brak zaproszen" << std::endl;
				}
			}

		}
		else {
			//Polaczylismy sie z drugim klientem zaczynamy chat
			std::cout << "ME: ";
			//wiadomosc nie moze zawierac znakow !| oraz nie moze byc zbyt dluga
			std::getline(std::cin, message);
			if (message.find("!|") != std::string::npos) {
				std::cout << "Wiadomosc nie zostala wyslana zawiera znaki specjalne !|" << std::endl;
				continue;
			}
			if (message.length() > 900) {
				std::cout << "Wiadomosc jest zbyt dluga!" << std::endl;
				continue;
			}
			if (message == "!END!") {
				PS.Operacja = "Zakonczenie";
				PS.NSekwencyjny = "1";
				PS.Identyfikator = ID;
				SendPacket(SendSocket, SendAddr, PS, packet);

				PS.Status = "DC_CHAT";
				PS.NSekwencyjny = "0";
				PS.Identyfikator = ID;
				SendPacket(SendSocket, SendAddr, PS, packet);
				accepted = 0;
			}
			//sprawdzenie czy drugi klient nie rozlaczyl sie z nami badz my nie zakonczylismy polaczenia
			if (accepted == 0)
				continue;

			PS.Operacja = "Wiadomosc";
			PS.NSekwencyjny = "2";
			PS.Identyfikator = ID;
			SendPacket(SendSocket, SendAddr, PS, packet);

			PS.Status = "OK_MSG";
			PS.NSekwencyjny = "1";
			PS.Identyfikator = ID;
			SendPacket(SendSocket, SendAddr, PS, packet);

			PS.Tekst = message;
			PS.NSekwencyjny = "0";
			PS.Identyfikator = ID;
			SendPacket(SendSocket, SendAddr, PS, packet);
		}
	}

	// Zamykanie gniazda 

	system("PAUSE");
	std::cout << "Finished working. Closing socket.\n";
	iResult = closesocket(SendSocket);
	if (iResult == SOCKET_ERROR) {
		std::cout << "closesocket failed with error: %d\n", WSAGetLastError();
		WSACleanup();
		return 1;
	}
	std::cout << "Exiting.\n";
	WSACleanup();
	return 1;
}