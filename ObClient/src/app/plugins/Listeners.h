#pragma once

#include "CriticalSection.h"
#include "ObClient.h"
#include "Logger.h"

namespace Obligato {
	class Listeners {
	private:
		mutable CriticalSection _lock;
		std::shared_ptr<Obligato::Logger> _logger;

		ObClient::TargetsDb _targetsdb;
		size_t _index = 0;

	public:
		Listeners(std::shared_ptr<Obligato::Logger> log);
		~Listeners();

		// Create listener instance

		// Delete listener instance

		// List listener instances
	};
}