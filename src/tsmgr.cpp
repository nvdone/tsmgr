//NVD Terminal Session Manager
//Copyright © 2022, Nikolay Dudkin

//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
//GNU General Public License for more details.
//You should have received a copy of the GNU General Public License
//along with this program.If not, see<https://www.gnu.org/licenses/>.

#include <windows.h>
#include <wtsapi32.h>
#include <stdio.h>
#include "..\\lib\\cmdline.hpp"

LPCTSTR getStateName(DWORD state)
{
	switch(state)
	{
		case WTSActive:
			return L"Active";
		case WTSConnected:
			return L"Connected";
		case WTSConnectQuery:
			return L"ConnectQuery";
		case WTSShadow:
			return L"Shadow";
		case WTSDisconnected:
			return L"Disconnected";
		case WTSIdle:
			return L"Idle";
		case WTSListen:
			return L"Listen";
		case WTSReset:
			return L"Reset";
		case WTSDown:
			return L"Down";
		case WTSInit:
			return L"Init";
		default:
		break;
	}

	return L"Unknown";
}

int main()
{
	LPTSTR cmdLine = GetCommandLine();
	CmdLine *cl = new CmdLine(cmdLine);

	if(cl->CountCommands() < 3)
	{
		wprintf(L"NVD Terminal Session Manager\r\n(C) 2022, Nikolay Dudkin\r\n\r\nUsage: %s <servername> <list | kill sessionId>\r\n", cl->GetCommand(0)->GetName(1));
		delete cl;
		return 1;
	}

	LPTSTR serverName = cl->GetCommand(1)->GetName(1);
	LPTSTR command = cl->GetCommand(2)->GetName(1);

	if(!wcscmp(command, L"list"))
	{
		HANDLE hServer = WTSOpenServer(serverName);

		PWTS_SESSION_INFO_1 sessionInfo;
		DWORD level = 1;
		DWORD count = 0;

		if(!WTSEnumerateSessionsEx(hServer, &level, 0, &sessionInfo, &count))
		{
			wprintf(L"WTSEnumerateSessionsEx failed!");
			delete cl;
			return 2;
		}

		wprintf(L"%-5s\t%-5s\t%-15.15s\t%-15.15s\t%-15.15s\t%-15.15s\t%-15.15s\t%-15.15s\r\n",
				L"Id",
				L"EnvId",
				L"User",
				L"Domain",
				L"Host",
				L"Farm",
				L"Session",
				L"State"
				);

		for(unsigned int i = 0; i < count; i++)
		{
			wprintf(L"%-5d\t%-5d\t%-15.15s\t%-15.15s\t%-15.15s\t%-15.15s\t%-15.15s\t%-15.15s\r\n",
				sessionInfo[i].SessionId,
				sessionInfo[i].ExecEnvId,
				sessionInfo[i].pUserName ? sessionInfo[i].pUserName : L"",
				sessionInfo[i].pDomainName ? sessionInfo[i].pDomainName : L"",
				sessionInfo[i].pHostName ? sessionInfo[i].pHostName : L"",
				sessionInfo[i].pFarmName ? sessionInfo[i].pFarmName : L"",
				sessionInfo[i].pSessionName ? sessionInfo[i].pSessionName : L"",
				getStateName(sessionInfo[i].State)
				);
		}

		WTSFreeMemory(sessionInfo);
		WTSCloseServer(hServer);
	}

	if(!wcscmp(command, L"kill"))
	{
		if(cl->CountCommands() != 4)
		{
			wprintf(L"NVD Terminal Session Manager\r\n(C) 2022, Nikolay Dudkin\r\n\r\nUsage: %s <servername> kill sessionId\r\n", cl->GetCommand(0)->GetName(1));
			delete cl;
			return 3;
		}

		DWORD sessionId = _wtoi(cl->GetCommand(3)->GetName(1));
		HANDLE hServer = WTSOpenServer(serverName);
		if(!WTSLogoffSession(hServer, sessionId, TRUE))
		{
			wprintf(L"WTSLogoffSession failed!");
			delete cl;
			return 4;
		}

		WTSCloseServer(hServer);
	}

	delete cl;
	return 0;
}
