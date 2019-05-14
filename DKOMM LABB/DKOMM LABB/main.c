#include <WinSock2.h>
#include <WS2tcpip.h>
#include "strlib.h"
#include "simpio.h"
#include "genlib.h"

#define buffsize 512

/*FUNCTION PROTOTYPES*/
bool socket_read(SOCKET s);
string charlistToString(char charlist[], int messageLen);
string convert_command(bool nick);
string translateFromServer(string reply);

void main(void) {
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);
	int err = WSAStartup(wVersionRequested, &wsaData);
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	int messageLen;
	char space = 32;
	char buffer[buffsize];
	char colon = 58;
	int colonIndex;

	struct addrinfo *info;
	int ok = getaddrinfo("localhost", "12346", NULL, &info);

	if (ok != 0) {
		WCHAR * error = gai_strerror(ok);
		printf("%S\n", error);
	}
	else while (info->ai_family != AF_INET && info->ai_next != NULL)
		info = info->ai_next;

	ok = connect(s, info->ai_addr, info->ai_addrlen);

	messageLen = recv(s, buffer, buffsize, 0);

	
	/*Welcome Message*/
	string welcomeMessage = charlistToString(buffer, messageLen);
	printf(welcomeMessage);

	/*Nickname loop*/
	bool welcome = FALSE;
	bool quit_client = FALSE;
	string data = "";
	string reply, welcomeCheck, messageFromServer, msg = "";
	int FirstSpaceIndex, startIndex;
	int endLineIndex = 0;
	string endLine = "\n";
	string quitCheck;
	while (!welcome) {
		CHAR * message = convert_command(TRUE);
		ok = send(s, message, strlen(message), 0);

		colonIndex = FindChar(colon, message, 0);
		quitCheck = SubString(message, 0, colonIndex - 2);
		if (StringEqual(quitCheck, "QUIT")) {
			welcome = TRUE;
			quit_client = TRUE;
		}

		while (TRUE) {
			messageLen = recv(s, buffer, buffsize, 0);
			messageFromServer = charlistToString(buffer, messageLen);
			
			startIndex = 0;
			while (TRUE) {
				endLineIndex = FindString(endLine, messageFromServer, startIndex);
				if (endLineIndex == -1) {
					break;
				}
				msg = translateFromServer(SubString(messageFromServer, startIndex, endLineIndex - 1));

				if (!StringEqual(msg, "PING")) {
					printf(msg);
					printf("\n");
				}
				startIndex = endLineIndex + 1;
			}
			break;
		}
		reply = msg;
		FirstSpaceIndex = FindChar(space, reply, 0);
		welcomeCheck = SubString(reply, 0, FirstSpaceIndex - 1);

			if (StringEqual(welcomeCheck, "Welcome")) {
				welcome = TRUE;
			}
	}

	/*Client loop*/

	while (!quit_client) {
		CHAR * message = convert_command(FALSE);
		ok = send(s, message, strlen(message), 0);

		colonIndex = FindChar(colon, message, 0);
		quitCheck = SubString(message, 0, colonIndex - 2);
		if (StringEqual(quitCheck, "QUIT")) {
			quit_client = TRUE;
		}


		while (TRUE) {
			messageLen = recv(s, buffer, buffsize, 0);
			messageFromServer = charlistToString(buffer, messageLen);

			startIndex = 0;
			while (TRUE) {
				endLineIndex = FindString(endLine, messageFromServer, startIndex);
				if (endLineIndex == -1) {
					break;
				}
				msg = translateFromServer(SubString(messageFromServer, startIndex, endLineIndex - 1));

				if (!StringEqual(msg, "PING")) {
					printf(msg);
					printf("\n");
				}
				startIndex = endLineIndex + 1;
			}
			break;
		}
	}


	WSACleanup();
}

bool socket_read(SOCKET s) {
	bool welcome = FALSE;
	int iResult, j, k;
	string temp = "", protocol, payload;
	char buffer[buffsize];
	do {
		iResult = recv(s, buffer, buffsize, 0);
		if (iResult > 0) {
			do {
				for (int i = 0; i < iResult; i++) {
					temp = Concat(temp, CharToString(buffer[i]));
				}
				j = FindChar(':', temp, 0);
				protocol = SubString(temp, 0, j - 1);
				k = FindChar('\n', temp, j);
				payload = SubString(temp, j + 1, k);
				temp = SubString(temp, k + 1, StringLength(temp));
				if (StringCompare(protocol, "PING") != 0 && StringCompare(protocol, "EOM") != 0) {
					printf("%s", payload);
					if (StringCompare(protocol, "OK") == 0)
						welcome = TRUE;
				}
			} while (protocol != "EOM" && StringLength(temp) != 0);
		}
	} while (iResult > 0);
	return welcome;
}

string charlistToString(char charlist[], int messageLen) {
	string message = "";
	for (int i = 0; i < messageLen; i++) {
		message = Concat(message, CharToString(charlist[i]));
	}
	return message;
}


string convert_command(bool nick) {
	string usrInput, command = "", command_check;
	char * commandList[4];
	char space = 32;
	int space_index, new_space_index;
	bool isCommand = FALSE;
	char * commandClient[9] = { "nick", "j", "p", "msg", "l", "k", "q", "" };
	char * commandServer[9] = { "NICK", "JOIN", "PART", "SEND", "LIST", "KICK", "QUIT", "NOOP :NOOP" };

	while (!isCommand) {
		if (nick) {
			printf("\nChoose your nick name: ");
		}
		else {
			printf("\nInsert command: ");
		}

		usrInput = GetLine();
		space_index = FindChar(space, usrInput, 0);

		command_check = SubString(usrInput, 0, space_index-1);
		for (int i = 0; i < 8; i++) {
			if (StringEqual(command_check, commandClient[i])) {
				command_check = commandServer[i];
				commandList[0] = command_check;
				isCommand = TRUE;
				break;
			}
		}
		if (!isCommand) {
			printf("Not a valid command...\n");
		}
	}

	int commandlist_len;
	if (StringEqual(command_check, "KICK")) {
		commandlist_len = 4;
		for (int i = 0; i < 3; i++)
		{
			new_space_index = FindChar(space, usrInput, space_index + 1);
			if (i == 2) {
				command_check = SubString(usrInput, space_index+1, StringLength(usrInput) - 1);
			}
			else {
				command_check = SubString(usrInput, space_index, new_space_index - 1);
			}
			commandList[i + 1] = command_check;
			space_index = new_space_index;
		}
	}
	else if (StringEqual(command_check, "SEND")) {
		commandlist_len = 3;
		for (int i = 0; i < 2; i++)
		{	
			new_space_index = FindChar(space, usrInput, space_index + 1);
			if (i == 1) {
				command_check = SubString(usrInput, space_index+1, StringLength(usrInput) - 1);
			}
			else {
				command_check = SubString(usrInput, space_index, new_space_index - 1);
			}
			commandList[i + 1] = command_check;
			space_index = new_space_index;
		}
	}
	else {
		commandlist_len = 2;
		command_check = SubString(usrInput, space_index+1, StringLength(usrInput)-1);
		commandList[1] = command_check;
	}
	
	for (int i = 0; i < commandlist_len; i++)
	{	
		if (i == commandlist_len - 1) {
			command = Concat(command, " :");
		}
		command = Concat(command, commandList[i]);
	}

	return command;
}

string translateFromServer(string reply) {
	int colonIndex;
	char colon = 58;
	string translatedMessage, errorCheck;

	if (StringEqual(reply, "msg box:")) {
		return reply;
	}

	colonIndex = FindChar(colon, reply, 0);
	errorCheck = SubString(reply, 0, colonIndex - 1);

	if (StringEqual(errorCheck, "ERROR")) {
		// Om tid finns, lägg till mellanslag efter :, ERROR: msg
		return reply;
	}

	translatedMessage = SubString(reply, colonIndex + 1, StringLength(reply) - 1);
	return translatedMessage;
}


