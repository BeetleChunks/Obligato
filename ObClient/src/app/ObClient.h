#pragma once

#include "ObEngine.h"

namespace ObClient {
	using TargetsDb = std::vector<std::map<std::wstring, std::any>>;
	using TargetObj = std::map<std::wstring, std::any>;

	using ListenersDb = std::vector<std::map<std::wstring, std::any>>;
	using ListenerObj = std::map<std::wstring, std::any>;

	using AgentTaskMap = std::map<ObEngineRef::MessageType, std::string>;
}