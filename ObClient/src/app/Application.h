#pragma once

#include "ObClient.h"
#include "Logger.h"
#include "Targets.h"
#include "Listeners.h"
#include "Helpers.h"
#include "imgui_stdlib.h"

namespace MainApp {
	// Main UI Render Function - Calls GUI Elements
	void RenderUI();

	/*
		GUI ELEMENTS
	*/
	void LoggerWindow();
	void ListenersWindow();
	void TargetsWindow();
	void TaskAgentsWindow();
}