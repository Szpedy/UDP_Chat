#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <ctime>
#include <thread>
#include <limits>
// Link with ws2_32.lib
#pragma warning(disable:4996) 
#pragma comment(lib, "Ws2_32.lib")
#pragma once

struct packetStruct {
	std::string Czas; // czas systemowy
	std::string NSekwencyjny; // licznik, który gdy osi¹gnie 0 znaczy, ¿e operacja zosta³a zakoñczona
	std::string Identyfikator; // ID klienta
	std::string Operacja; // polaczenie | start | rozmowa | wiadomosc | zakonczenie
	std::string Status = ""; // OK | NO
	std::string Tekst; // wiadomosc wysylana
					   //wszystkie górne wysy³ane oraz Operacja lub Status lub Tekst
};

std::string getTime();
void ReadMessage(packetStruct& PS, std::string packet);
void WriteMessage(packetStruct& PS, std::string& packet);
void ZeroPS(packetStruct& PS);
void printMSG(const packetStruct &PS);
int SendPacket(SOCKET s, addrinfo *SendAddr, packetStruct &PS, std::string packet);
int RecievePacket(SOCKET s, sockaddr_in &RecvAddr, packetStruct &PS, std::string packet, int RecvAddrSize);
int SendAck(SOCKET s, addrinfo *SendAddr, packetStruct PS, std::string packet);
void Recieve(SOCKET s, sockaddr_in RecvAddr, addrinfo *SendAddr, packetStruct PS, std::string packet);
int delay(int &value);
