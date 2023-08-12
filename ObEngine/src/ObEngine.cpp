#include "pch.h"
#include "ObEngine.h"

#include "Helpers.h"
#include "Factory.h"
#include "Mailslot.h"
#include "UdpSocket.h"
#include "Callbacks.h"

namespace ObEngine {
	/*
		UNIQUE IDENTIFIERS
	*/
	ULONGLONG CalcMailslotNotifyId(
		_In_  const std::wstring& slotname,
		_In_  const std::wstring& netbios
	) {
		return MessageFk::CalcMailslotNotifyId(slotname, netbios);
	}

	ULONGLONG CalcUdpSocketNotifyId(
		_In_  const std::wstring& address,
		_In_  const USHORT& port
	) {
		return MessageFk::CalcUdpSocketNotifyId(address, port);
	}


	/*
		START SERVERS
	*/
	void StartMailslotServer(
		const std::wstring& mailslot
	) {
		Helpers::DbgPrint(std::format(L"[Info][AgentLib] Starting mailslot server '{}'.",
			mailslot));

		Mailslot::Server server(mailslot);
		server.Start(MessageCb::OnMsgRecv);
	}

	void StartUdpSocketServer(
		const std::wstring& address, const USHORT& port
	) {
		Helpers::DbgPrint(std::format(L"[Info][AgentLib] Starting UDP Socket server '{}:{}'.",
			address, port));

		UdpSocket::Server server(address, port);
		server.Start(MessageCb::OnMsgRecv);
	}


	/*
		ADD|REMOVE|SEND NOTIFICATION DESTINATIONS/MESSAGES
	*/
	void AddMailslotNotify(
		TransType trans, TransConf& dest,
		const std::wstring& mailslot, const std::wstring& netbios
	) {
		std::unique_ptr<Transport::Client, std::default_delete<Transport::Client>> client;

		if (trans == TransType::Mailslot) {
			std::wstring dstMailslot = std::any_cast<std::wstring>(dest[L"Slotname"]);
			std::wstring dstNetbios = std::any_cast<std::wstring>(dest[L"NetBIOS"]);

			client = std::make_unique<Mailslot::Client>(dstMailslot, dstNetbios);
		}
		else if (trans == TransType::UdpSocket) {
			std::wstring dstUdpAddr = std::any_cast<std::wstring>(dest[L"UdpAddr"]);
			int dstUdpPort = std::any_cast<USHORT>(dest[L"UdpPort"]);

			client = std::make_unique<UdpSocket::Client>(dstUdpAddr, dstUdpPort);
		}

		if (!client->Connect()) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client failed to connect.");
			return;
		}

		/*
			Add Mailslot Notification Destination
		*/
		ObEngineRef::AddMailslotNotify* lpTask = nullptr;
		DWORD dwMsgSize;
		ULONGLONG ullNotifyId;

		if (!MessageFk::AddMailslotNotifyMessage(mailslot, netbios, &lpTask, dwMsgSize, ullNotifyId)) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client AddMailslotNotifyMessage() failed.");
			return;
		}

		if (!client->Send(*lpTask, dwMsgSize)) {
			Helpers::DbgPrint(std::format(L"[Error][AgentLib] Failed to send add mailslot notify message - {}",
				Helpers::ErrorMessage()));
		}
		else {
			Helpers::DbgPrint(std::format(L"[Info][AgentLib] Add mailslot notify message sent - ID:{}",
				ullNotifyId));
		}

		free(lpTask);
	}

	void RemoveMailslotNotify(
		TransType trans, TransConf& dest,
		const std::wstring& mailslot, const std::wstring& netbios
	) {
		std::unique_ptr<Transport::Client, std::default_delete<Transport::Client>> client;

		if (trans == TransType::Mailslot) {
			std::wstring dstMailslot = std::any_cast<std::wstring>(dest[L"Slotname"]);
			std::wstring dstNetbios = std::any_cast<std::wstring>(dest[L"NetBIOS"]);

			client = std::make_unique<Mailslot::Client>(dstMailslot, dstNetbios);
		}
		else if (trans == TransType::UdpSocket) {
			std::wstring dstUdpAddr = std::any_cast<std::wstring>(dest[L"UdpAddr"]);
			int dstUdpPort = std::any_cast<USHORT>(dest[L"UdpPort"]);

			client = std::make_unique<UdpSocket::Client>(dstUdpAddr, dstUdpPort);
		}

		if (!client->Connect()) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client failed to connect.");
			return;
		}

		/*
			Remove Mailslot Notification Destination
		*/
		ULONGLONG ullNotifyId = MessageFk::CalcMailslotNotifyId(mailslot, netbios);

		ObEngineRef::RemoveMailslotNotify* lpTask = nullptr;
		DWORD dwMsgSize;

		if (!MessageFk::RemoveMailslotNotifyMessage(ullNotifyId, &lpTask, dwMsgSize)) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client RemoveMailslotNotifyMessage() failed.");
			return;
		}

		if (!client->Send(*lpTask, dwMsgSize)) {
			Helpers::DbgPrint(std::format(L"[Error][AgentLib] Failed to send remove mailslot notify message - {}",
				Helpers::ErrorMessage()));
		}
		else {
			Helpers::DbgPrint(std::format(L"[Info][AgentLib] Remove mailslot notify message sent - ID:{}",
				ullNotifyId));
		}

		free(lpTask);
	}

	void SendMailslotNotify(
		TransType trans, TransConf& dest,
		ULONGLONG ullNotifyId, const std::wstring& notification
	) {
		std::unique_ptr<Transport::Client, std::default_delete<Transport::Client>> client;

		if (trans == TransType::Mailslot) {
			std::wstring dstMailslot = std::any_cast<std::wstring>(dest[L"Slotname"]);
			std::wstring dstNetbios = std::any_cast<std::wstring>(dest[L"NetBIOS"]);

			client = std::make_unique<Mailslot::Client>(dstMailslot, dstNetbios);
		}
		else if (trans == TransType::UdpSocket) {
			std::wstring dstUdpAddr = std::any_cast<std::wstring>(dest[L"UdpAddr"]);
			int dstUdpPort = std::any_cast<USHORT>(dest[L"UdpPort"]);

			client = std::make_unique<UdpSocket::Client>(dstUdpAddr, dstUdpPort);
		}

		if (!client->Connect()) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client failed to connect.");
			return;
		}

		/*
			Send Mailslot Notification

			After notifications are implemented, the notification can be sent to
			NotifyCb::OnNotification() directly or to a local transport
			server, which will then forward them to NotifyCb::OnNotification().
			Messages are marked as inbound or outbound so they will be processed
			accordingly regardless of how they make their way to that callback.

			Based on the destination configuration for the given notify ID, the
			NotifyCb::OnNotification() callback will use an appropriate transport
			client to send the notification.
		*/
		ObEngineRef::NotifyData* lpTask = nullptr;
		DWORD dwMsgSize;

		if (!MessageFk::MailslotNotifyMessage(ullNotifyId, notification, &lpTask, dwMsgSize)) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client MailslotNotifyMessage() failed.");
			return;
		}

		if (!client->Send(*lpTask, dwMsgSize)) {
			Helpers::DbgPrint(std::format(L"[Error][AgentLib] Failed to send notification message - {}",
				Helpers::ErrorMessage()));
		}
		else {
			Helpers::DbgPrint(std::format(L"[Info][AgentLib] Notification message sent - ID:{}",
				ullNotifyId));
		}

		free(lpTask);
	}


	void AddUdpSocketNotify(
		TransType trans, TransConf& dest,
		const std::wstring& addr, const USHORT& port
	) {
		std::unique_ptr<Transport::Client, std::default_delete<Transport::Client>> client;

		if (trans == TransType::Mailslot) {
			std::wstring dstMailslot = std::any_cast<std::wstring>(dest[L"Slotname"]);
			std::wstring dstNetbios = std::any_cast<std::wstring>(dest[L"NetBIOS"]);

			client = std::make_unique<Mailslot::Client>(dstMailslot, dstNetbios);
		}
		else if (trans == TransType::UdpSocket) {
			std::wstring dstUdpAddr = std::any_cast<std::wstring>(dest[L"UdpAddr"]);
			int dstUdpPort = std::any_cast<USHORT>(dest[L"UdpPort"]);

			client = std::make_unique<UdpSocket::Client>(dstUdpAddr, dstUdpPort);
		}

		if (!client->Connect()) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client failed to connect.");
			return;
		}

		/*
			Add UDP Socket Notification Destination
		*/
		ObEngineRef::AddUdpSocketNotify* lpTask = nullptr;
		DWORD dwMsgSize;
		ULONGLONG ullNotifyId;

		if (!MessageFk::AddUdpSocketNotifyMessage(addr, port, &lpTask, dwMsgSize, ullNotifyId)) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client AddUdpSocketNotifyMessage() failed.");
			return;
		}

		if (!client->Send(*lpTask, dwMsgSize)) {
			Helpers::DbgPrint(std::format(L"[Error][AgentLib] Failed to send add udp socket notify message - {}",
				Helpers::ErrorMessage()));
		}
		else {
			Helpers::DbgPrint(std::format(L"[Info][AgentLib] Add udp socket notify message sent - ID:{}",
				ullNotifyId));
		}

		free(lpTask);
	}

	void RemoveUdpSocketNotify(
		TransType trans, TransConf& dest,
		const std::wstring& addr, const USHORT& port
	) {
		std::unique_ptr<Transport::Client, std::default_delete<Transport::Client>> client;

		if (trans == TransType::Mailslot) {
			std::wstring dstMailslot = std::any_cast<std::wstring>(dest[L"Slotname"]);
			std::wstring dstNetbios = std::any_cast<std::wstring>(dest[L"NetBIOS"]);

			client = std::make_unique<Mailslot::Client>(dstMailslot, dstNetbios);
		}
		else if (trans == TransType::UdpSocket) {
			std::wstring dstUdpAddr = std::any_cast<std::wstring>(dest[L"UdpAddr"]);
			int dstUdpPort = std::any_cast<USHORT>(dest[L"UdpPort"]);

			client = std::make_unique<UdpSocket::Client>(dstUdpAddr, dstUdpPort);
		}

		if (!client->Connect()) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client failed to connect.");
			return;
		}

		/*
			Remove UDP Socket Notification Destination
		*/
		ULONGLONG ullNotifyId = MessageFk::CalcUdpSocketNotifyId(addr, port);

		ObEngineRef::RemoveUdpSocketNotify* lpTask = nullptr;
		DWORD dwMsgSize;

		if (!MessageFk::RemoveUdpSocketNotifyMessage(ullNotifyId, &lpTask, dwMsgSize)) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client RemoveUdpSocketNotifyMessage() failed.");
			return;
		}

		if (!client->Send(*lpTask, dwMsgSize)) {
			Helpers::DbgPrint(std::format(L"[Error][AgentLib] Failed to send remove udp socket notify message - {}",
				Helpers::ErrorMessage()));
		}
		else {
			Helpers::DbgPrint(std::format(L"[Info][AgentLib] Remove udp socket notify message sent - ID:{}",
				ullNotifyId));
		}

		free(lpTask);
	}

	void SendUdpSocketNotify(
		TransType trans, TransConf& dest,
		ULONGLONG ullNotifyId, const std::wstring& notification
	) {
		std::unique_ptr<Transport::Client, std::default_delete<Transport::Client>> client;

		if (trans == TransType::Mailslot) {
			std::wstring dstMailslot = std::any_cast<std::wstring>(dest[L"Slotname"]);
			std::wstring dstNetbios = std::any_cast<std::wstring>(dest[L"NetBIOS"]);

			client = std::make_unique<Mailslot::Client>(dstMailslot, dstNetbios);
		}
		else if (trans == TransType::UdpSocket) {
			std::wstring dstUdpAddr = std::any_cast<std::wstring>(dest[L"UdpAddr"]);
			int dstUdpPort = std::any_cast<USHORT>(dest[L"UdpPort"]);

			client = std::make_unique<UdpSocket::Client>(dstUdpAddr, dstUdpPort);
		}

		if (!client->Connect()) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client failed to connect.");
			return;
		}

		/*
			Send UDP Socket Notification

			After notifications are implemented, the notification can be sent to
			NotifyCb::OnNotification() directly or to a local transport
			server, which will then forward them to NotifyCb::OnNotification().
			Messages are marked as inbound or outbound so they will be processed
			accordingly regardless of how they make their way to that callback.

			Based on the destination configuration for the given notify ID, the
			NotifyCb::OnNotification() callback will use an appropriate transport
			client to send the notification.
		*/
		ObEngineRef::NotifyData* lpTask = nullptr;
		DWORD dwMsgSize;

		if (!MessageFk::UdpSocketNotifyMessage(ullNotifyId, notification, &lpTask, dwMsgSize)) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client UdpSocketNotifyMessage() failed.");
			return;
		}

		if (!client->Send(*lpTask, dwMsgSize)) {
			Helpers::DbgPrint(std::format(L"[Error][AgentLib] Failed to send notification message - {}",
				Helpers::ErrorMessage()));
		}
		else {
			Helpers::DbgPrint(std::format(L"[Info][AgentLib] Notification message sent - ID:{}",
				ullNotifyId));
		}

		free(lpTask);
	}


	/*
		SEND TASKS
	*/
	void CreateUserTask(
		TransType trans, TransConf& dest,
		ULONGLONG ullNotifyId,
		const std::wstring& domain, const std::wstring& username,
		const std::wstring& password
	) {
		std::unique_ptr<Transport::Client, std::default_delete<Transport::Client>> client;

		if (trans == TransType::Mailslot) {
			std::wstring dstMailslot = std::any_cast<std::wstring>(dest[L"Slotname"]);
			std::wstring dstNetbios = std::any_cast<std::wstring>(dest[L"NetBIOS"]);

			client = std::make_unique<Mailslot::Client>(dstMailslot, dstNetbios);
		}
		else if (trans == TransType::UdpSocket) {
			std::wstring dstUdpAddr = std::any_cast<std::wstring>(dest[L"UdpAddr"]);
			int dstUdpPort = std::any_cast<USHORT>(dest[L"UdpPort"]);

			client = std::make_unique<UdpSocket::Client>(dstUdpAddr, dstUdpPort);
		}

		if (!client->Connect()) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client failed to connect.");
			return;
		}

		ObEngineRef::CreateUserTask* lpTask = nullptr;
		DWORD dwMsgSize;

		if (!MessageFk::CreateUserMessage(domain, username, password, &lpTask, dwMsgSize)) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client CreateUserMessage() failed.");
			return;
		}

		// Configure notification endpoint (must have been added already)
		lpTask->NotifyId = ullNotifyId;

		if (!client->Send(*lpTask, dwMsgSize)) {
			Helpers::DbgPrint(std::format(L"[Error][AgentLib] Failed to send create user message - {}",
				Helpers::ErrorMessage()));
		}
		else {
			Helpers::DbgPrint(L"[Info][AgentLib] Create user message sent");
		}

		free(lpTask);
	}

	void DeleteUserTask(
		TransType trans, TransConf& dest,
		ULONGLONG ullNotifyId,
		const std::wstring& domain, const std::wstring& username
	) {
		std::unique_ptr<Transport::Client, std::default_delete<Transport::Client>> client;

		if (trans == TransType::Mailslot) {
			std::wstring dstMailslot = std::any_cast<std::wstring>(dest[L"Slotname"]);
			std::wstring dstNetbios = std::any_cast<std::wstring>(dest[L"NetBIOS"]);

			client = std::make_unique<Mailslot::Client>(dstMailslot, dstNetbios);
		}
		else if (trans == TransType::UdpSocket) {
			std::wstring dstUdpAddr = std::any_cast<std::wstring>(dest[L"UdpAddr"]);
			int dstUdpPort = std::any_cast<USHORT>(dest[L"UdpPort"]);

			client = std::make_unique<UdpSocket::Client>(dstUdpAddr, dstUdpPort);
		}

		if (!client->Connect()) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client failed to connect.");
			return;
		}

		ObEngineRef::DeleteUserTask* lpTask = nullptr;
		DWORD dwMsgSize;

		if (!MessageFk::DeleteUserMessage(domain, username, &lpTask, dwMsgSize)) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client DeleteUserMessage() failed.");
			return;
		}

		// Configure notification endpoint (must have been added already)
		lpTask->NotifyId = ullNotifyId;

		if (!client->Send(*lpTask, dwMsgSize)) {
			Helpers::DbgPrint(std::format(L"[Error][AgentLib] Failed to send delete user message - {}",
				Helpers::ErrorMessage()));
		}
		else {
			Helpers::DbgPrint(L"[Info][AgentLib] Delete user message sent");
		}

		free(lpTask);
	}

	void CreateGroupTask(
		TransType trans, TransConf& dest,
		ULONGLONG ullNotifyId,
		const std::wstring& domain, const std::wstring& group,
		const std::wstring& comment
	) {
		std::unique_ptr<Transport::Client, std::default_delete<Transport::Client>> client;

		if (trans == TransType::Mailslot) {
			std::wstring dstMailslot = std::any_cast<std::wstring>(dest[L"Slotname"]);
			std::wstring dstNetbios = std::any_cast<std::wstring>(dest[L"NetBIOS"]);

			client = std::make_unique<Mailslot::Client>(dstMailslot, dstNetbios);
		}
		else if (trans == TransType::UdpSocket) {
			std::wstring dstUdpAddr = std::any_cast<std::wstring>(dest[L"UdpAddr"]);
			int dstUdpPort = std::any_cast<USHORT>(dest[L"UdpPort"]);

			client = std::make_unique<UdpSocket::Client>(dstUdpAddr, dstUdpPort);
		}

		if (!client->Connect()) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client failed to connect.");
			return;
		}

		ObEngineRef::CreateGroupTask* lpTask = nullptr;
		DWORD dwMsgSize;

		if (!MessageFk::CreateGroupMessage(domain, group, comment, &lpTask, dwMsgSize)) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client CreateGroupMessage() failed.");
			return;
		}

		// Configure notification endpoint (must have been added already)
		lpTask->NotifyId = ullNotifyId;

		if (!client->Send(*lpTask, dwMsgSize)) {
			Helpers::DbgPrint(std::format(L"[Error][AgentLib] Failed to send create group message - {}",
				Helpers::ErrorMessage()));
		}
		else {
			Helpers::DbgPrint(L"[Info][AgentLib] Create group message sent");
		}

		free(lpTask);
	}

	void DeleteGroupTask(
		TransType trans, TransConf& dest,
		ULONGLONG ullNotifyId,
		const std::wstring& domain, const std::wstring& group
	) {
		std::unique_ptr<Transport::Client, std::default_delete<Transport::Client>> client;

		if (trans == TransType::Mailslot) {
			std::wstring dstMailslot = std::any_cast<std::wstring>(dest[L"Slotname"]);
			std::wstring dstNetbios = std::any_cast<std::wstring>(dest[L"NetBIOS"]);

			client = std::make_unique<Mailslot::Client>(dstMailslot, dstNetbios);
		}
		else if (trans == TransType::UdpSocket) {
			std::wstring dstUdpAddr = std::any_cast<std::wstring>(dest[L"UdpAddr"]);
			int dstUdpPort = std::any_cast<USHORT>(dest[L"UdpPort"]);

			client = std::make_unique<UdpSocket::Client>(dstUdpAddr, dstUdpPort);
		}

		if (!client->Connect()) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client failed to connect.");
			return;
		}

		ObEngineRef::DeleteGroupTask* lpTask = nullptr;
		DWORD dwMsgSize;

		if (!MessageFk::DeleteGroupMessage(domain, group, &lpTask, dwMsgSize)) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client DeleteGroupMessage() failed.");
			return;
		}

		// Configure notification endpoint (must have been added already)
		lpTask->NotifyId = ullNotifyId;

		if (!client->Send(*lpTask, dwMsgSize)) {
			Helpers::DbgPrint(std::format(L"[Error][AgentLib] Failed to send delete group message - {}",
				Helpers::ErrorMessage()));
		}
		else {
			Helpers::DbgPrint(L"[Info][AgentLib] Delete group message sent");
		}

		free(lpTask);
	}

	void AddGroupMemberTask(
		TransType trans, TransConf& dest,
		ULONGLONG ullNotifyId,
		const std::wstring& domain, const std::wstring& group,
		const std::wstring& member
	) {
		std::unique_ptr<Transport::Client, std::default_delete<Transport::Client>> client;

		if (trans == TransType::Mailslot) {
			std::wstring dstMailslot = std::any_cast<std::wstring>(dest[L"Slotname"]);
			std::wstring dstNetbios = std::any_cast<std::wstring>(dest[L"NetBIOS"]);

			client = std::make_unique<Mailslot::Client>(dstMailslot, dstNetbios);
		}
		else if (trans == TransType::UdpSocket) {
			std::wstring dstUdpAddr = std::any_cast<std::wstring>(dest[L"UdpAddr"]);
			int dstUdpPort = std::any_cast<USHORT>(dest[L"UdpPort"]);

			client = std::make_unique<UdpSocket::Client>(dstUdpAddr, dstUdpPort);
		}

		if (!client->Connect()) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client failed to connect.");
			return;
		}

		ObEngineRef::AddGroupMemberTask* lpTask = nullptr;
		DWORD dwMsgSize;

		if (!MessageFk::AddGroupMemberMessage(domain, group, member, &lpTask, dwMsgSize)) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client AddGroupMemberMessage() failed.");
			return;
		}

		// Configure notification endpoint (must have been added already)
		lpTask->NotifyId = ullNotifyId;

		if (!client->Send(*lpTask, dwMsgSize)) {
			Helpers::DbgPrint(std::format(L"[Error][AgentLib] Failed to send add group member message - {}",
				Helpers::ErrorMessage()));
		}
		else {
			Helpers::DbgPrint(L"[Info][AgentLib] Add group member message sent");
		}

		free(lpTask);
	}

	void RemoveGroupMemberTask(
		TransType trans, TransConf& dest,
		ULONGLONG ullNotifyId,
		const std::wstring& domain, const std::wstring& group,
		const std::wstring& member
	) {
		std::unique_ptr<Transport::Client, std::default_delete<Transport::Client>> client;

		if (trans == TransType::Mailslot) {
			std::wstring dstMailslot = std::any_cast<std::wstring>(dest[L"Slotname"]);
			std::wstring dstNetbios = std::any_cast<std::wstring>(dest[L"NetBIOS"]);

			client = std::make_unique<Mailslot::Client>(dstMailslot, dstNetbios);
		}
		else if (trans == TransType::UdpSocket) {
			std::wstring dstUdpAddr = std::any_cast<std::wstring>(dest[L"UdpAddr"]);
			int dstUdpPort = std::any_cast<USHORT>(dest[L"UdpPort"]);

			client = std::make_unique<UdpSocket::Client>(dstUdpAddr, dstUdpPort);
		}

		if (!client->Connect()) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client failed to connect.");
			return;
		}

		ObEngineRef::RemoveGroupMemberTask* lpTask = nullptr;
		DWORD dwMsgSize;

		if (!MessageFk::RemoveGroupMemberMessage(domain, group, member, &lpTask, dwMsgSize)) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client RemoveGroupMemberMessage() failed.");
			return;
		}

		// Configure notification endpoint (must have been added already)
		lpTask->NotifyId = ullNotifyId;

		if (!client->Send(*lpTask, dwMsgSize)) {
			Helpers::DbgPrint(std::format(L"[Error][AgentLib] Failed to send remove group member message - {}",
				Helpers::ErrorMessage()));
		}
		else {
			Helpers::DbgPrint(L"[Info][AgentLib] Remove group member message sent");
		}

		free(lpTask);
	}

	void AddPrivilegeTask(
		TransType trans, TransConf& dest,
		ULONGLONG ullNotifyId,
		const std::wstring& entity, const std::wstring& privilege
	) {
		std::unique_ptr<Transport::Client, std::default_delete<Transport::Client>> client;

		if (trans == TransType::Mailslot) {
			std::wstring dstMailslot = std::any_cast<std::wstring>(dest[L"Slotname"]);
			std::wstring dstNetbios = std::any_cast<std::wstring>(dest[L"NetBIOS"]);

			client = std::make_unique<Mailslot::Client>(dstMailslot, dstNetbios);
		}
		else if (trans == TransType::UdpSocket) {
			std::wstring dstUdpAddr = std::any_cast<std::wstring>(dest[L"UdpAddr"]);
			int dstUdpPort = std::any_cast<USHORT>(dest[L"UdpPort"]);

			client = std::make_unique<UdpSocket::Client>(dstUdpAddr, dstUdpPort);
		}

		if (!client->Connect()) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client failed to connect.");
			return;
		}

		ObEngineRef::AddPrivilegeTask* lpTask = nullptr;
		DWORD dwMsgSize;

		if (!MessageFk::AddPrivilegeMessage(entity, privilege, &lpTask, dwMsgSize)) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client AddPrivilegeMessage() failed.");
			return;
		}

		// Configure notification endpoint (must have been added already)
		lpTask->NotifyId = ullNotifyId;

		if (!client->Send(*lpTask, dwMsgSize)) {
			Helpers::DbgPrint(std::format(L"[Error][AgentLib] Failed to send add privilege message - {}",
				Helpers::ErrorMessage()));
		}
		else {
			Helpers::DbgPrint(L"[Info][AgentLib] Add privilege message sent");
		}

		free(lpTask);
	}

	void RemovePrivilegeTask(
		TransType trans, TransConf& dest,
		ULONGLONG ullNotifyId,
		const std::wstring& entity, const std::wstring& privilege
	) {
		std::unique_ptr<Transport::Client, std::default_delete<Transport::Client>> client;

		if (trans == TransType::Mailslot) {
			std::wstring dstMailslot = std::any_cast<std::wstring>(dest[L"Slotname"]);
			std::wstring dstNetbios = std::any_cast<std::wstring>(dest[L"NetBIOS"]);

			client = std::make_unique<Mailslot::Client>(dstMailslot, dstNetbios);
		}
		else if (trans == TransType::UdpSocket) {
			std::wstring dstUdpAddr = std::any_cast<std::wstring>(dest[L"UdpAddr"]);
			int dstUdpPort = std::any_cast<USHORT>(dest[L"UdpPort"]);

			client = std::make_unique<UdpSocket::Client>(dstUdpAddr, dstUdpPort);
		}

		if (!client->Connect()) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client failed to connect.");
			return;
		}

		ObEngineRef::RemovePrivilegeTask* lpTask = nullptr;
		DWORD dwMsgSize;

		if (!MessageFk::RemovePrivilegeMessage(entity, privilege, &lpTask, dwMsgSize)) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client RemovePrivilegeTask() failed.");
			return;
		}

		// Configure notification endpoint (must have been added already)
		lpTask->NotifyId = ullNotifyId;

		if (!client->Send(*lpTask, dwMsgSize)) {
			Helpers::DbgPrint(std::format(L"[Error][AgentLib] Failed to send remove privilege message - {}",
				Helpers::ErrorMessage()));
		}
		else {
			Helpers::DbgPrint(L"[Info][AgentLib] Remove privilege message sent");
		}

		free(lpTask);
	}

	void SpawnProcessTask(
		TransType trans, TransConf& dest,
		ULONGLONG ullNotifyId,
		const std::wstring& app, const std::wstring& args,
		const std::wstring& cwd, const std::wstring& domain,
		const std::wstring& username
	) {
		std::unique_ptr<Transport::Client, std::default_delete<Transport::Client>> client;

		if (trans == TransType::Mailslot) {
			std::wstring dstMailslot = std::any_cast<std::wstring>(dest[L"Slotname"]);
			std::wstring dstNetbios = std::any_cast<std::wstring>(dest[L"NetBIOS"]);

			client = std::make_unique<Mailslot::Client>(dstMailslot, dstNetbios);
		}
		else if (trans == TransType::UdpSocket) {
			std::wstring dstUdpAddr = std::any_cast<std::wstring>(dest[L"UdpAddr"]);
			int dstUdpPort = std::any_cast<USHORT>(dest[L"UdpPort"]);

			client = std::make_unique<UdpSocket::Client>(dstUdpAddr, dstUdpPort);
		}

		if (!client->Connect()) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client failed to connect.");
			return;
		}

		ObEngineRef::SpawnProcessTask* lpTask = nullptr;
		DWORD dwMsgSize;

		if (!MessageFk::SpawnProcessMessage(app, args, cwd, domain, username, &lpTask, dwMsgSize)) {
			Helpers::DbgPrint(L"[Error][AgentLib] Client SpawnProcessMessage() failed.");
			return;
		}

		// Configure notification endpoint (must have been added already)
		lpTask->NotifyId = ullNotifyId;

		if (!client->Send(*lpTask, dwMsgSize)) {
			Helpers::DbgPrint(std::format(L"[Error][AgentLib] Failed to send spawn process message - {}",
				Helpers::ErrorMessage()));
		}
		else {
			Helpers::DbgPrint(L"[Info][AgentLib] Spawn process message sent");
		}

		free(lpTask);
	}
}