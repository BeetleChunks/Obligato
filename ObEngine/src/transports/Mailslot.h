#pragma once

#include "ObEngine.h"
#include "Helpers.h"
#include "Transport.h"

namespace Mailslot {
	class Server : public Transport::Server {
	private:
		std::wstring _SlotName;
		std::wstring _FullPath;
		
		HANDLE _hMailslot = INVALID_HANDLE_VALUE;

		bool SetMailslotAccess();
	
	public:
		Server(const std::wstring& SlotName);
		~Server();

		void Start(PTP_SIMPLE_CALLBACK OnMsgRecv) override;
	};


	class Client : public Transport::Client {
	private:
		std::wstring _SlotName;
		std::wstring _FullPath;
		std::wstring _NetBios;

		HANDLE _hMailslot = INVALID_HANDLE_VALUE;

	public:
		Client(const std::wstring& SlotName, const std::wstring& Netbios);
		~Client();

		bool Connect() override;
		bool Send(const ObEngineRef::MessageHeader& Message, const DWORD dwMsgSize) override;
	};
}