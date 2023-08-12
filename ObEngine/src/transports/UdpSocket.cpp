#include "pch.h"
#include "UdpSocket.h"

/*
	SERVER
*/
UdpSocket::Server::Server(const std::wstring& Address, USHORT Port)
	: _Address(Address), _Port(Port) {

	std::string AddrStr = Helpers::ws2s(_Address);

	struct sockaddr_in siServer;
	WSADATA wsa;

	// Initialize winsock
	if (::WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return;
	}
	
	// Create socket
	_SockServer = ::socket(AF_INET, SOCK_DGRAM, 0);
	if (_SockServer == INVALID_SOCKET) {
		return;
	}

	siServer.sin_family      = AF_INET;
	siServer.sin_addr.s_addr = ::inet_addr(AddrStr.c_str());
	siServer.sin_port        = ::htons(_Port);

	// Bind socket
	if (::bind(_SockServer, (struct sockaddr*)&siServer, sizeof(siServer)) == SOCKET_ERROR) {
		::closesocket(_SockServer);
		_SockServer = INVALID_SOCKET;
		return;
	}
}

UdpSocket::Server::~Server() {
	if (_SockServer != INVALID_SOCKET)
		::closesocket(_SockServer);
	
	::WSACleanup();
}

/* PUBLIC */
void UdpSocket::Server::Start(PTP_SIMPLE_CALLBACK OnMsgRecv) {
	if (_SockServer == INVALID_SOCKET) {
		Helpers::DbgPrint(L"[Error][UdpSocket::Server::Start()] Socket is not valid.");
		return;
	}

	struct sockaddr_in siRemote;
	int slen = sizeof(siRemote);
	int BytesRead;
	BYTE msg[ObEngineRef::MAX_UDP_SOCKET_MSG_SIZE];

	while (1) {
		::memset(&msg, 0, ObEngineRef::MAX_UDP_SOCKET_MSG_SIZE);

		BytesRead = ::recvfrom(
			_SockServer,
			(char*)&msg,
			ObEngineRef::MAX_UDP_SOCKET_MSG_SIZE,
			0,
			(struct sockaddr *)&siRemote,
			&slen
		);

		if (BytesRead == SOCKET_ERROR) {
			Helpers::DbgPrint(std::format(L"[Error][UdpSocket::Server::Start()]->recvfrom() - {}",
				WSAGetLastError()));
			break;
		}

		// Verify message size doesn't exceed maximum
		if (BytesRead > ObEngineRef::MAX_UDP_SOCKET_MSG_SIZE) {
			Helpers::DbgPrint(std::format(L"[Error][UdpSocket::Server::Start()] Bytes recv ({}) exceeds max ({})",
				BytesRead, ObEngineRef::MAX_UDP_SOCKET_MSG_SIZE));
			continue;
		}

		// Verify message is at least the size of a message header
		if (BytesRead < sizeof(ObEngineRef::MessageHeader)) {
			Helpers::DbgPrint(std::format(L"[Error][UdpSocket::Server::Start()] Bytes recv ({}) is less than a message header",
				BytesRead));
			continue;
		}

		// Verify number of bytes read matches size reported in message header
		ObEngineRef::MessageHeader* lpMsgHeader = (ObEngineRef::MessageHeader*)msg;
		if (BytesRead != sizeof(ObEngineRef::MessageHeader) + lpMsgHeader->Size) {
			Helpers::DbgPrint(std::format(L"[Error][UdpSocket::Server::Start()] Bytes recv ({}) differs from message header ({})",
				BytesRead, sizeof(ObEngineRef::MessageHeader) + lpMsgHeader->Size));
			continue;
		}

		ObEngineRef::MsgRecvContext* lpContext = new ObEngineRef::MsgRecvContext();
		lpContext->lpMessage = (ObEngineRef::MessageHeader*)malloc(BytesRead);
		
		if (lpContext->lpMessage == NULL) {
			Helpers::DbgPrint(L"[Error][UdpSocket::Server::Start()] Failed to allocate memory for context buffer.");
			continue;
		}
		
		::memcpy(lpContext->lpMessage, lpMsgHeader, BytesRead);
		lpContext->dwMsgSize = BytesRead;
		
		if (!::TrySubmitThreadpoolCallback(OnMsgRecv, lpContext, nullptr)) {
			Helpers::DbgPrint(L"[Error][UdpSocket::Server::Start()] Failed sending message to callback.");
			continue;
		}
	}
}



/*
	CLIENT
*/
UdpSocket::Client::Client(const std::wstring& Address, USHORT Port)
	: _Address(Address), _Port(Port) {

}

UdpSocket::Client::~Client() {
	if (_SockClient != INVALID_SOCKET)
		::closesocket(_SockClient);

	::WSACleanup();
}

/* PUBLIC */
bool UdpSocket::Client::Connect() {
	WSADATA wsa;

	// Initialize winsock
	if (::WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return false;
	}

	// Create socket
	_SockClient = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (_SockClient == INVALID_SOCKET) {
		return false;
	}

	return true;
}

bool UdpSocket::Client::Send(const ObEngineRef::MessageHeader& Message, const DWORD dwMsgSize) {
	int BytesSent, MsgSize;
	struct sockaddr_in siRemote;
	int slen = sizeof(siRemote);

	std::string AddrStr = Helpers::ws2s(_Address);

	if (::DWordToInt(dwMsgSize, &MsgSize)) {
		Helpers::DbgPrint(L"[Error][UdpSocket::Client::Send()] Converting DWORD msg size to int");
		return false;
	}

	siRemote.sin_family = AF_INET;
	siRemote.sin_addr.s_addr = ::inet_addr(AddrStr.c_str());
	siRemote.sin_port = ::htons(_Port);

	// Verify message size doesn't exceed maximum
	if (dwMsgSize > ObEngineRef::MAX_UDP_SOCKET_MSG_SIZE) {
		Helpers::DbgPrint(L"[Error][UdpSocket::Client::Send()] Message size exceeds maximum.");
		return false;
	}

	// Verify message is at least the size of a message header
	if (dwMsgSize < sizeof(ObEngineRef::MessageHeader)) {
		Helpers::DbgPrint(L"[Error][UdpSocket::Client::Send()] Message size less than header size.");
		return false;
	}

	// Verify number of bytes read matches size reported in message header
	if (dwMsgSize != sizeof(ObEngineRef::MessageHeader) + Message.Size) {
		Helpers::DbgPrint(std::format(L"[Error][UdpSocket::Client::Send()] Message size ({}) differs from header ({}).",
			dwMsgSize, Message.Size));
		return false;
	}

	// Send message
	BytesSent = ::sendto(
		_SockClient,
		(char*)&Message,
		MsgSize,
		0,
		(struct sockaddr*)&siRemote,
		slen
	);

	if (BytesSent == SOCKET_ERROR) {
		Helpers::DbgPrint(L"[Error][Mailslot::Client::Send()] Client failed to send message.");
		return false;
	}

	return true;
}