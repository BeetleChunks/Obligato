#pragma once

#include "ObEngine.h"

/*
	TRANSPORT INTERFACES
*/
namespace Transport {
	// message receiver
	class Server {
	public:
		virtual void Start(PTP_SIMPLE_CALLBACK OnMsgRecv) = 0;
	};

	// message sender
	class Client {
	public:
		virtual bool Connect() = 0;
		virtual bool Send(const ObEngineRef::MessageHeader& Message, const DWORD dwMsgSize) = 0;
	};
}