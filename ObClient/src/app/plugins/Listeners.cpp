#include "pch.h"
#include "Listeners.h"
#include "AutoCriticalSection.h"

namespace Obligato {
	/*
		CONSTRUCTOR
	*/
	Listeners::Listeners(std::shared_ptr<Obligato::Logger> log) {
		_logger = log;
		_logger->Debug(std::string("Listeners plugin constructed"));
	}


	/*
		DESTRUCTOR
	*/
	Listeners::~Listeners() {
		_logger->Debug(std::string("Listeners destructed"));
	}


	/*
		PUBLIC
	*/
}