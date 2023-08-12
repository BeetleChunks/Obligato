#include "pch.h"
#include "Router.h"
#include "AutoCriticalSection.h"


Router::Manager::Manager() {}

Router::Manager::~Manager() {}


bool Router::Manager::GetRoute(
	_In_  const ULONGLONG& id,
	_Out_ Route& route
) {
	AutoCriticalSection locker(_lock);

	// Check if route exists
	if (_route_table.find(id) == _route_table.end()) {
		route.clear();
		return false;
	}

	route = _route_table[id];

	return true;
}

bool Router::Manager::RemoveRoute(
	_In_ const ULONGLONG& id
) {
	AutoCriticalSection locker(_lock);

	// Check if route exists
	if (_route_table.find(id) == _route_table.end()) {
		return false;
	}

	_route_table.erase(id);

	return true;
}

bool Router::Manager::AddMailslotRoute(
	_In_ const ULONGLONG& id,
	_In_ const std::wstring& SlotName,
	_In_ const std::wstring& NetBios
) {
	AutoCriticalSection locker(_lock);

	Route route;
	route["type"]    = RouteType::Mailslot;
	route["slot"]    = SlotName;
	route["netbios"] = NetBios;

	_route_table[id] = route;

	return true;
}

bool Router::Manager::AddUdpSocketRoute(
	_In_ const ULONGLONG& id,
	_In_ const std::wstring& Addr,
	_In_ const USHORT& Port
) {
	AutoCriticalSection locker(_lock);

	Route route;
	route["type"] = RouteType::UdpSocket;
	route["addr"] = Addr;
	route["port"] = Port;

	_route_table[id] = route;

	return true;
}