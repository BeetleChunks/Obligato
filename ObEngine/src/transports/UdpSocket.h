#pragma once

#include "ObEngine.h"
#include "Helpers.h"
#include "Transport.h"

namespace UdpSocket {
	class Server : public Transport::Server {
	private:
		std::wstring _Address;
		USHORT _Port;

		SOCKET _SockServer = INVALID_SOCKET;

	public:
		Server(const std::wstring& Address, USHORT Port);
		~Server();

		void Start(PTP_SIMPLE_CALLBACK OnMsgRecv) override;
	};


	class Client : public Transport::Client {
	private:
		std::wstring _Address;
		USHORT _Port;

		SOCKET _SockClient = INVALID_SOCKET;

	public:
		Client(const std::wstring& Address, USHORT Port);
		~Client();

		bool Connect() override;
		bool Send(const ObEngineRef::MessageHeader& Message, const DWORD dwMsgSize) override;
	};
}