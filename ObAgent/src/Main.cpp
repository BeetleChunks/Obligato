#include "pch.h"
#include "ObEngine.h"

void AgentDbg(_In_ const std::wstring& msg) {
	std::wcout << msg << std::endl;
	::OutputDebugStringW(msg.c_str());
}

int wmain(int argc, wchar_t* argv[]) {
	if (argc == 1) {
		//ObEngine::StartMailslotServer(std::wstring(L"obligato"));
		//ObEngine::StartUdpSocketServer(std::wstring(L"0.0.0.0"), 11337);
		return 0;
	}
	else if (argc > 2) {
		std::wstring task = argv[1];

		std::vector<std::wstring> vwsArgs;

		int j = 2;
		while (j < argc) {
			vwsArgs.push_back(argv[j]);
			j++;
		}

		// Mailslot Destination
		std::wstring dstMailslot  = L"";
		std::wstring dstNetbios   = L"";

		// UDP Socket Destination
		std::wstring dstUdpAddr   = L"";
		USHORT dstUdpPort         = 0;

		// Mailslot Notification
		std::wstring mailslot     = L"";
		std::wstring netbios      = L"";

		// UDP Socket Notification
		std::wstring udpAddr      = L"";
		USHORT udpPort            = 0;

		// Notification Message
		std::wstring notification = L"";
		
		// Task Parameters
		std::wstring domain       = L"";
		std::wstring username     = L"";
		std::wstring password     = L"";
		std::wstring group        = L"";
		std::wstring comment      = L"";
		std::wstring member       = L"";
		std::wstring entity       = L"";
		std::wstring privilege    = L"";
		std::wstring appName      = L"";
		std::wstring appArgs      = L"";
		std::wstring appCwd       = L"";

		size_t i = 0;
		while (i < vwsArgs.size()) {
			if (vwsArgs[i] == L"-dMs") {
				dstMailslot = vwsArgs[i + 1];
				i += 2;
			}
			else if (vwsArgs[i] == L"-dNb") {
				dstNetbios = vwsArgs[i + 1];
				i += 2;
			}
			else if (vwsArgs[i] == L"-dUA") {
				dstUdpAddr = vwsArgs[i + 1];
				i += 2;
			}
			else if (vwsArgs[i] == L"-dUP") {
				int tmpPort = std::stoi(vwsArgs[i + 1]);
				if (::IntToUShort(tmpPort, &dstUdpPort) != S_OK) {
					AgentDbg(L"[Error][-dUP] Port is not valid.");
					return 0;
				}
				i += 2;
			}
			else if (vwsArgs[i] == L"-Ms") {
				mailslot = vwsArgs[i + 1];
				i += 2;
			}
			else if (vwsArgs[i] == L"-Nb") {
				netbios = vwsArgs[i + 1];
				i += 2;
			}
			else if (vwsArgs[i] == L"-UA") {
				udpAddr = vwsArgs[i + 1];
				i += 2;
			}
			else if (vwsArgs[i] == L"-UP") {
				int tmpPort = std::stoi(vwsArgs[i + 1]);
				if (::IntToUShort(tmpPort, &udpPort) != S_OK) {
					AgentDbg(L"[Error][-UP] Port is not valid.");
					return 0;
				}
				i += 2;
			}
			else if (vwsArgs[i] == L"-n") {
				notification = vwsArgs[i + 1];
				i += 2;
			}
			else if (vwsArgs[i] == L"-d") {
				domain = vwsArgs[i + 1];
				i += 2;
			}
			else if (vwsArgs[i] == L"-u") {
				username = vwsArgs[i + 1];
				i += 2;
			}
			else if (vwsArgs[i] == L"-p") {
				password = vwsArgs[i + 1];
				i += 2;
			}
			else if (vwsArgs[i] == L"-g") {
				group = vwsArgs[i + 1];
				i += 2;
			}
			else if (vwsArgs[i] == L"-c") {
				comment = vwsArgs[i + 1];
				i += 2;
			}
			else if (vwsArgs[i] == L"-m") {
				member = vwsArgs[i + 1];
				i += 2;
			}
			else if (vwsArgs[i] == L"-e") {
				entity = vwsArgs[i + 1];
				i += 2;
			}
			else if (vwsArgs[i] == L"-Pr") {
				privilege = vwsArgs[i + 1];
				i += 2;
			}
			else if (vwsArgs[i] == L"-An") {
				appName = vwsArgs[i + 1];
				i += 2;
			}
			else if (vwsArgs[i] == L"-Aa") {
				appArgs = vwsArgs[i + 1];
				i += 2;
			}
			else if (vwsArgs[i] == L"-Ac") {
				appCwd = vwsArgs[i + 1];
				i += 2;
			}
			else if (vwsArgs[i] == L"-h") {
				return 0;
			}
		}

		// Determine and Configure Transport
		ObEngine::TransType transType;
		ObEngine::TransConf transConf;
		if (!dstMailslot.empty() && !dstNetbios.empty()) {
			transType = ObEngine::TransType::Mailslot;
			transConf[L"Slotname"] = dstMailslot;
			transConf[L"NetBIOS"]  = dstNetbios;
		}
		else if (!dstUdpAddr.empty() && dstUdpPort != -1) {
			transType = ObEngine::TransType::UdpSocket;
			transConf[L"UdpAddr"] = dstUdpAddr;
			transConf[L"UdpPort"] = dstUdpPort;
		}
		else {
			transType = ObEngine::TransType::None;
		}

		// Calculate Notification Destination ID
		ULONGLONG notifyId = 0;
		if (!mailslot.empty() && !netbios.empty()) {
			notifyId = ObEngine::CalcMailslotNotifyId(mailslot, netbios);
		}
		else if (!udpAddr.empty() && udpPort != -1) {
			notifyId = ObEngine::CalcUdpSocketNotifyId(udpAddr, udpPort);
		}


		// Start Servers
		if (task == L"StartMsServer")
			ObEngine::StartMailslotServer(mailslot);
		else if (task == L"StartUdpServer")
			ObEngine::StartUdpSocketServer(udpAddr, udpPort);
		
		// Add|Remove|Send Notification Destinations/Messages
		else if (task == L"AddMsNotify")
			ObEngine::AddMailslotNotify(transType, transConf, mailslot, netbios);
		else if (task == L"DelMsNotify")
			ObEngine::RemoveMailslotNotify(transType, transConf, mailslot, netbios);
		else if (task == L"SendMsNotify")
			ObEngine::SendMailslotNotify(transType, transConf, notifyId, notification);

		else if (task == L"AddUdpSocNotify")
			ObEngine::AddUdpSocketNotify(transType, transConf, udpAddr, udpPort);
		else if (task == L"DelUdpSocNotify")
			ObEngine::RemoveUdpSocketNotify(transType, transConf, udpAddr, udpPort);
		else if (task == L"SendUdpSocNotify")
			ObEngine::SendUdpSocketNotify(transType, transConf, notifyId, notification);
		
		// Send Tasks
		else if (task == L"AddUser")
			ObEngine::CreateUserTask(transType, transConf, notifyId, domain, username, password);
		else if (task == L"DelUser")
			ObEngine::DeleteUserTask(transType, transConf, notifyId, domain, username);
		else if (task == L"AddGroup")
			ObEngine::CreateGroupTask(transType, transConf, notifyId, domain, group, comment);
		else if (task == L"DelGroup")
			ObEngine::DeleteGroupTask(transType, transConf, notifyId, domain, group);
		else if (task == L"AddGroupMem")
			ObEngine::AddGroupMemberTask(transType, transConf, notifyId, domain, group, member);
		else if (task == L"RmGroupMem")
			ObEngine::RemoveGroupMemberTask(transType, transConf, notifyId, domain, group, member);
		else if (task == L"AddPriv")
			ObEngine::AddPrivilegeTask(transType, transConf, notifyId, entity, privilege);
		else if (task == L"RmPriv")
			ObEngine::RemovePrivilegeTask(transType, transConf, notifyId, entity, privilege);
		else if (task == L"SpwnProc")
			ObEngine::SpawnProcessTask(transType, transConf, notifyId, appName, appArgs, appCwd, domain, username);
	}
	
	return 1;
}