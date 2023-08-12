#include "pch.h"
#include "Targets.h"
#include "AutoCriticalSection.h"

/*
	CONSTRUCTOR
*/
Obligato::Targets::Targets(std::shared_ptr<Obligato::Logger> log) {
	_logger = log;

	// TODO: Load targets from disk

	_logger->Debug(std::string("Targets plugin constructed"));
}


/*
	DESTRUCTOR
*/
Obligato::Targets::~Targets() {
	// TODO: Save database to disk
	_logger->Debug(std::string("Targets destructed"));
}


/*
	PUBLIC
*/
bool Obligato::Targets::AddMailslotTarget(
	_In_ const std::wstring& mailslot,
	_In_ const std::wstring& netbios
) {
	ObClient::TargetObj target;

	target[L"transport"] = ObEngine::TransType::Mailslot;
	target[L"uid"]       = ObEngine::CalcMailslotNotifyId(mailslot, netbios);
	target[L"uri"]       = std::format(L"MAILSLOT://{}/{}", netbios, mailslot);
	target[L"mailslot"]  = mailslot;
	target[L"netbios"]   = netbios;

	AutoCriticalSection locker(_lock);
	_index++;
	target[L"index"] = _index;
	_targetsdb.push_back(target);

	_logger->Info(std::format("Added target MAILSLOT://{}/{}",
		Obligato::ws2s(netbios), Obligato::ws2s(mailslot)));

	return true;
}

bool Obligato::Targets::AddUdpSocketTarget(
	_In_ const std::wstring& address,
	_In_ const USHORT& port
) {
	ObClient::TargetObj target;
	
	target[L"transport"] = ObEngine::TransType::UdpSocket;
	target[L"uid"]       = ObEngine::CalcUdpSocketNotifyId(address, port);
	target[L"uri"]       = std::format(L"UDP-SOCKET://{}:{}", address, port);
	target[L"address"]   = address;
	target[L"port"]      = port;

	AutoCriticalSection locker(_lock);
	_index++;
	target[L"index"] = _index;
	_targetsdb.push_back(target);

	_logger->Info(std::format("Added target UDP-SOCKET://{}:{}",
		Obligato::ws2s(address), port));

	return true;
}

bool Obligato::Targets::RemoveTarget(
	_In_ const size_t& index
) {
	AutoCriticalSection locker(_lock);
	for (size_t i = 0; i < _targetsdb.size(); i++) {
		auto cur_index = std::any_cast<USHORT>(_targetsdb[i][L"index"]);

		if (cur_index == index)
			_targetsdb.erase(_targetsdb.begin() + i);
	}

	return true;
}

void Obligato::Targets::GetTargetsDbSnapshot(
	_Out_ ObClient::TargetsDb& TargetsDbSnapshot
) {
	AutoCriticalSection locker(_lock);
	TargetsDbSnapshot = _targetsdb;
}

void Obligato::Targets::ClearTargetsDb() {
	AutoCriticalSection locker(_lock);
	_targetsdb.clear();
}