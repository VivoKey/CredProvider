
#include "sessionid.h"

BOOL SpeakWithPipe(char * loginbuf, char* password)
{
	addLogMessage("Start SpeakWithPipe func");

	LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\pipeforcp");

	char bufferRequest[255] = ""; // ����� ��� �������� ������
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	int BUFSIZE = 255; //������ ������
	bool fConnected = FALSE; // ��������� � ������� 
	BOOL fSuccess = FALSE; // ���������� ���������� ��� writepipe

	hPipe = CreateNamedPipe(
		lpszPipename,             // pipe name 
		PIPE_ACCESS_DUPLEX,       // read/write access 
		PIPE_TYPE_MESSAGE |       // message type pipe 
		PIPE_READMODE_MESSAGE |   // message-read mode 
		PIPE_WAIT,                // blocking mode 
		10,							// max. instances  == listen_socket
		BUFSIZE,                  // output buffer size 
		BUFSIZE,                  // input buffer size 
		0,                        // client time-out 
		NULL);                    // default security attribute 

	if (hPipe == INVALID_HANDLE_VALUE)
	{
		addLogMessage("CreateNamedPipe failed in ClientTherd");
		CloseHandle(hPipe); // rewrite
		return FALSE;
	}
	addLogMessage("Wait pipe connection");
	fConnected = ConnectNamedPipe(hPipe, NULL) ?
		TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
	// CP ���������� � ������ ��� - ������ ����������� � ���
	if (fConnected)
	{
		DWORD  writtenBytes = 0; // ��������� � ����� ������
		DWORD bufferSizeWrite = 255; //-����� ����������� ������������� ��� ����������
		addLogMessage("CP was connected, start to write in pipe..."); // debugMessage
																	  // Loop until done reading

		if (!WriteFile(
			hPipe,        // handle to pipe 
			loginbuf,     // buffer to write from 
			bufferSizeWrite, // number of bytes to write  - ����� ����������� ������������� ��� ���������� 
			&writtenBytes,   // number of bytes written 
			NULL))      // not overlapped I/O  )
		{
			addLogMessage("SocketWorkerThread WriteFile failed.");
			CloseHandle(hPipe);
			return FALSE;
		}
		if (!WriteFile(
			hPipe,        // handle to pipe 
			password,     // buffer to write from 
			bufferSizeWrite, // number of bytes to write  - ����� ����������� ������������� ��� ���������� 
			&writtenBytes,   // number of bytes written 
			NULL))      // not overlapped I/O  )
		{
			addLogMessage("SocketWorkerThread WriteFile failed.");
			CloseHandle(hPipe);
			return FALSE;
		}
		// ������ ������ � ���������� (����� ����� 1) �� + ������ �������� ������ 2) ��� 
		//������ �� ������ ��������� ��������������� ����� � ������� ����������
	}
	else
	{
		addLogMessage("Connected to pipe is lose");
		return FALSE;
	}

	//� ���������� ��� �������� ����������� ���������� ����� ����������� ��������� �������� ������ 
	//	����� �� ����������� �� ���������� ������� � ����� !
	// ������� ������ � �������� ��� � ��������
	FlushFileBuffers(hPipe);
	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);
	return TRUE;
}

BOOL GetCurrentUser(LPTSTR &szUpn, DWORD & pSessionId)
{
	LPTSTR szUserName;
	LPTSTR szDomainName;
	char tmpbuf[8] = "";
	int dwSessionId = 0;
	PHANDLE hUserToken = 0;
	PHANDLE hTokenDup = 0;

	PWTS_SESSION_INFO pSessionInfo = 0;
	DWORD dwCount = 0;

	// Get the list of all terminal sessions 
	WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1,
		&pSessionInfo, &dwCount);

	int dataSize = sizeof(WTS_SESSION_INFO);

	// look over obtained list in search of the active session
	for (DWORD i = 0; i < dwCount; ++i)
	{
		WTS_SESSION_INFO si = pSessionInfo[i];
		if (WTSActive == si.State)
		{
			// If the current session is active � store its ID
			dwSessionId = si.SessionId;
			pSessionId = dwSessionId; // in global programm
			itoa(dwSessionId, tmpbuf, 10);
			addLogMessage("Active session is ");
			addLogMessage(tmpbuf);
			break;
		}
	}
	WTSFreeMemory(pSessionInfo);

	DWORD dwLen = 0;
	BOOL bStatus = WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE,
		dwSessionId,
		WTSDomainName,
		&szDomainName,
		&dwLen);
	if (bStatus)
	{
		bStatus = WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE,
			dwSessionId,
			WTSUserName,
			&szUserName,
			&dwLen);
		if (bStatus)
		{
			DWORD cbUpn = _tcslen(szUserName) + 1 + _tcslen(szDomainName);
			szUpn = (LPTSTR)LocalAlloc(0, (cbUpn + 1) * sizeof(TCHAR));

			_tcscpy(szUpn, szUserName);
			_tcscat(szUpn, _T("@"));
			_tcscat(szUpn, szDomainName);

			addLogMessage("UPN = "); addLogMessageW(szUpn);
			//LocalFree(szUpn); // in global system
			WTSFreeMemory(szUserName);
			WTSFreeMemory(szDomainName);
		}
		else
		{
			addLogMessage("WTSQuerySessionInformation on WTSUserName failed with error");
			return FALSE;
		}
	}
	else
	{
		addLogMessage("WTSQuerySessionInformation on WTSDomainName failed with erro");
		return FALSE;
	}
	return TRUE;
}