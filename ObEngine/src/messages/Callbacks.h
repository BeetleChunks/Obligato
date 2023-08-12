#pragma once

#include "ObEngine.h"
#include "Helpers.h"
#include "Mailslot.h"
#include "UdpSocket.h"
#include "Factory.h"
#include "Router.h"
#include "WinSecurity.h"


namespace MessageCb {	
	
	// Processes received transport messages
	void CALLBACK OnMsgRecv(PTP_CALLBACK_INSTANCE instance, PVOID context);
}

namespace NotifyCb {
	struct AddMailslotContext {
		std::wstring slotname;
		std::wstring netbios;
		ULONGLONG id;
	};

	struct RemoveMailslotContext {
		ULONGLONG id;
	};


	struct AddUdpSocketContext {
		std::wstring addr;
		USHORT port;
		ULONGLONG id;
	};

	struct RemoveUdpSocketContext {
		ULONGLONG id;
	};


	struct NotificationContext {
		ULONGLONG id;
		bool inbound;
		std::wstring notification;
	};

	// Add mailslot notification mapping
	void CALLBACK OnAddMailslot(PTP_CALLBACK_INSTANCE instance, PVOID context);

	// Remove mailslot notification mapping
	void CALLBACK OnRemoveMailslot(PTP_CALLBACK_INSTANCE instance, PVOID context);


	// Add UDP socket notification mapping
	void CALLBACK OnAddUdpSocket(PTP_CALLBACK_INSTANCE instance, PVOID context);

	// Remove UDP socket notification mapping
	void CALLBACK OnRemoveUdpSocket(PTP_CALLBACK_INSTANCE instance, PVOID context);


	// Process INBOUND and OUTBOUND notifications
	void CALLBACK OnNotification(PTP_CALLBACK_INSTANCE instance, PVOID context);
}

namespace TaskCb {
	struct CreateUserContext {
		ULONGLONG    nid;
		std::wstring domain;
		std::wstring username;
		std::wstring password;
	};

	struct DeleteUserContext {
		ULONGLONG    nid;
		std::wstring domain;
		std::wstring username;
	};

	struct CreateGroupContext {
		ULONGLONG    nid;
		std::wstring domain;
		std::wstring group;
		std::wstring comment;
	};

	struct DeleteGroupContext {
		ULONGLONG    nid;
		std::wstring domain;
		std::wstring group;
	};

	struct AddGroupMemberContext {
		ULONGLONG    nid;
		std::wstring domain;
		std::wstring group;
		std::wstring member;
	};

	struct RemoveGroupMemberContext {
		ULONGLONG    nid;
		std::wstring domain;
		std::wstring group;
		std::wstring member;
	};

	struct AddPrivilegeContext {
		ULONGLONG    nid;
		std::wstring entity;
		std::wstring privilege;
	};

	struct RemovePrivilegeContext {
		ULONGLONG    nid;
		std::wstring entity;
		std::wstring privilege;
	};

	struct SpawnProcessContext {
		ULONGLONG    nid;
		std::wstring app;
		std::wstring args;
		std::wstring cwd;
		std::wstring domain;
		std::wstring username;
	};

	// Processes CreateUser tasks
	void CALLBACK OnCreateUser(PTP_CALLBACK_INSTANCE instance, PVOID context);

	// Processes DeleteUser tasks
	void CALLBACK OnDeleteUser(PTP_CALLBACK_INSTANCE instance, PVOID context);

	// Processes CreateGroup tasks
	void CALLBACK OnCreateGroup(PTP_CALLBACK_INSTANCE instance, PVOID context);

	// Processes DeleteGroup tasks
	void CALLBACK OnDeleteGroup(PTP_CALLBACK_INSTANCE instance, PVOID context);

	// Processes AddGroupMember tasks
	void CALLBACK OnAddGroupMember(PTP_CALLBACK_INSTANCE instance, PVOID context);

	// Processes RemoveGroupMember tasks
	void CALLBACK OnRemoveGroupMember(PTP_CALLBACK_INSTANCE instance, PVOID context);

	// Processes AddPrivilege tasks
	void CALLBACK OnAddPrivilege(PTP_CALLBACK_INSTANCE instance, PVOID context);

	// Processes RemovePrivilege tasks
	void CALLBACK OnRemovePrivilege(PTP_CALLBACK_INSTANCE instance, PVOID context);

	// Processes SpawnProcess tasks
	void CALLBACK OnSpawnProcess(PTP_CALLBACK_INSTANCE instance, PVOID context);
}