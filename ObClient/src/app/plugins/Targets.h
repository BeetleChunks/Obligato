#pragma once

#include "CriticalSection.h"
#include "ObClient.h"
#include "Logger.h"
#include "Helpers.h"

namespace Obligato {
	class Targets {
	private:
		mutable CriticalSection _lock;
		std::shared_ptr<Obligato::Logger> _logger;

		ObClient::TargetsDb _targetsdb;
		size_t _index = 0;
	
	public:
		Targets(std::shared_ptr<Obligato::Logger> log);
		~Targets();

		
		// Add target to the database
		bool AddMailslotTarget(
			_In_ const std::wstring& mailslot,
			_In_ const std::wstring& netbios
		);

		bool AddUdpSocketTarget(
			_In_ const std::wstring& address,
			_In_ const USHORT& port
		);

		// Remove target from the database
		bool RemoveTarget(
			_In_ const size_t& index
		);

		// Create a deep copy of current targets db, _targetsdb
		void GetTargetsDbSnapshot(
			_Out_ ObClient::TargetsDb& TargetsDbSnapshot
		);

		// Clears the targets db, _targetsdb
		void ClearTargetsDb();
	};
}