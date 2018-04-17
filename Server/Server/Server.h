#ifndef UNICODE
#define UNICODE
#endif

#define WIN32_LEAN_AND_MEAN

#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <bitset>
#include <random>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <ctime>

// Link with ws2_32.lib
#pragma warning(disable:4996) 
#pragma comment(lib, "Ws2_32.lib")
#pragma once

struct packetStruct {
	std::string Czas; // czas systemowy
	std::string NSekwencyjny; // licznik, który gdy osi¹gnie 0 znaczy, ¿e operacja zosta³a zakoñczona
	std::string Identyfikator; // ID klienta
	std::string Operacja; // polaczenie | start | rozmowa | wiadomosc | zakonczenie
	std::string Status; // OK | NO
	std::string Tekst; // wiadomosc wysylana
					   //wszystkie górne wysy³ane oraz Operacja lub Status lub Tekst
};

struct clientStruct
{
	sockaddr_in addr;
	std::string ID;
	packetStruct PS;
};

std::string getTime();
std::string getID();
std::string Clientinfo(sockaddr_in Client);
void ReadMessage(packetStruct& PS, std::string packet);
void ZeroPS(packetStruct& PS);
void WriteMessage(packetStruct& PS, std::string& packet);
void printMSG(const packetStruct &PS);
int RecievePacket(SOCKET s, sockaddr_in &RecvAddr, packetStruct &PS, std::string& packet, int RecvAddrSize);
int SendPacket(SOCKET s, sockaddr_in SendAddr, packetStruct &PS, std::string& packet);
int SendAck(SOCKET s, sockaddr_in SendAddr, packetStruct PS, std::string packet);
int recipient(int who);
void handlePacket(SOCKET s, sockaddr_in Client, std::string& packet, packetStruct &PS, int who);
int whoSentDatagram(std::vector<clientStruct>& Clients, sockaddr_in Client, packetStruct PS, std::string packet);
