#pragma once

#include "CriticalSection.h"


namespace Router {
	using Route = std::unordered_map<std::string, std::any>;
	using Table = std::unordered_map<ULONGLONG, Route>;

	enum class RouteType : short {
		None,
		Mailslot,
		UdpSocket
	};
	
	class Manager {
	private:
		mutable CriticalSection _lock;
		Table _route_table;

	public:
		Manager();
		~Manager();

		// Returns route entry for the given ID
		bool GetRoute(
			_In_  const ULONGLONG& id,
			_Out_ Route& route
		);

		// Removes route entry for the given ID
		bool RemoveRoute(
			_In_ const ULONGLONG& id
		);
		
		// Add a mailslot route entry, overwrites existing
		bool AddMailslotRoute(
			_In_ const ULONGLONG& id,
			_In_ const std::wstring& SlotName,
			_In_ const std::wstring& NetBios
		);

		// Add a mailslot route entry, overwrites existing
		bool AddUdpSocketRoute(
			_In_ const ULONGLONG& id,
			_In_ const std::wstring& Addr,
			_In_ const USHORT& Port
		);
	};
}