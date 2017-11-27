
#pragma once
#include "serviceapi.h"

DWORD WINAPI ServerWorkerClient(LPVOID lpParam)
{
	addLogMessage("Start ServerWorkerClient funcn");
	unsigned __int64 * socket_client = (unsigned __int64*)lpParam;
	//for debug client socket
	char socketstr[8];
	itoa(*socket_client, socketstr, 10);
	addLogMessage("socket  client in ServerWorkerClient = "); addLogMessage(socketstr);
	//
	char message[255] = ""; // ����� ����������� �������� ������ ������ �� ������� ����������
	int bytes_read = 0;

	bytes_read = recv(*socket_client, message, sizeof(message), 0); // message ����� json

	if (bytes_read == 0) // ����� 1 ����� ����� ���� ��� ��������� ��� ������� ������������ ������  
	{
		addLogMessage("ERROR. Bytes reader is field or error of network");
		return -1;
	}
	addLogMessage("Massage from client is accepted:");

	itoa(bytes_read, socketstr, 10);
	
	addLogMessage(message); // ��� ������!
	addLogMessage("Long message = "); addLogMessage(socketstr);
	bytes_read = 0;
	// With JSON
	
	//
	int command;
	char *login = nullptr;
	char *pass = nullptr;
	rapidjson::Document doc;
	doc.Parse(message);
	
	if (!doc.IsObject())
	{
		addLogMessage("JSON doc is not an object");
		return 0;
	}
	if (doc.Size() == 1)
	{
		command = doc["command"].GetInt();
	}
	else 
		if (doc.Size() == 3)
		{
			static const char* members[] = { "command","login", "password" };
			for (size_t i = 0; i < sizeof(members) / sizeof(members[0]); i++)
				if (!doc.HasMember(members[i]))
				{
					addLogMessage("JSON doc hasn't member");
					return 0;
				}

			command = doc["command"].GetInt();
			login = (char*)doc["login"].GetString();
			pass = (char*)doc["password"].GetString();

			addLogMessage("JSON packet is accept");
			addLogMessage(login); addLogMessage(pass);
		}
	if (command == 0)
	{
		bool bSpeak = SpeakWithPipe(login, pass);
		if (!bSpeak)
		{
			addLogMessage("Speak with Pipe return error");
		}
	}
	else 
		if (command == 1)
		{
			LPTSTR userDomenName;
			DWORD sessionid;
			if (GetCurrentUser(userDomenName, sessionid))
			{
				addLogMessageW(userDomenName);
				addLogMessage("function GetWhoSessionIs return true!");
			}
			// ����� ������� � ������� � ������
		}
	
	shutdown(*socket_client, 2); //����� ����� ������� �� �������
	bytes_read = 0;    // ������� �����
	if (socket_client)
		delete socket_client;	// ������� ��������� ���������� � ���� ����� - ������� ������ (����� �� ������� �� �������� � �������)
	socket_client = NULL;	// �� ������ ������ ����������� ������ ������� ���������
	addLogMessage("END ServerWorkerClient func");
	return SEC_E_OK;
}
