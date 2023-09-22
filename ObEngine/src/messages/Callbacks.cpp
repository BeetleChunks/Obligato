#include "pch.h"
#include "Callbacks.h"


auto g_router = std::make_shared<Router::Manager>();


void MessageCb::OnMsgRecv(PTP_CALLBACK_INSTANCE instance, PVOID context) {
	auto data = static_cast<ObEngineRef::MsgRecvContext*>(context);

	switch (data->lpMessage->Type) {
	case ObEngineRef::MessageType::AddMailslotNotify: {
		// Verify message size is at least that of AddMailslotNotify
		if (data->dwMsgSize < sizeof(ObEngineRef::AddMailslotNotify)) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) is smaller than definition",
				data->dwMsgSize));
			break;
		}

		ObEngineRef::AddMailslotNotify* lpTask = (ObEngineRef::AddMailslotNotify*)data->lpMessage;

		// Verify message size equals size reported from header
		size_t SizeReported = lpTask->SlotNameSize;
		SizeReported += lpTask->NetBiosSize;
		SizeReported += sizeof(ObEngineRef::AddMailslotNotify);

		if (SizeReported != data->dwMsgSize) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) != reported size ({})",
				data->dwMsgSize, SizeReported));
			break;
		}

		// Verify parameter offsets based on size
		auto ExpectedOffset = sizeof(ObEngineRef::AddMailslotNotify);
		if (lpTask->SlotNameOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Slot name offset is {}, expected {}",
				lpTask->SlotNameOffset, ExpectedOffset));
			break;
		}

		ExpectedOffset += lpTask->SlotNameSize;
		if (lpTask->NetBiosOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] NetBios offset is {}, expected {}",
				lpTask->NetBiosOffset, ExpectedOffset));
			break;
		}

		UCHAR* lpSlotNameBuf = new UCHAR[lpTask->SlotNameSize];
		UCHAR* lpNetBiosBuf = new UCHAR[lpTask->NetBiosSize];

		::memcpy(lpSlotNameBuf, (UCHAR*)lpTask + lpTask->SlotNameOffset, lpTask->SlotNameSize);
		::memcpy(lpNetBiosBuf, (UCHAR*)lpTask + lpTask->NetBiosOffset, lpTask->NetBiosSize);

		NotifyCb::AddMailslotContext* lpContext = new NotifyCb::AddMailslotContext();
		lpContext->slotname = std::wstring((wchar_t*)lpSlotNameBuf);
		lpContext->netbios  = std::wstring((wchar_t*)lpNetBiosBuf);
		lpContext->id = lpTask->NotifyId;

		delete[] lpSlotNameBuf;
		delete[] lpNetBiosBuf;

		if (!::TrySubmitThreadpoolCallback(NotifyCb::OnAddMailslot, lpContext, nullptr)) {
			Helpers::DbgPrint(L"[Error][OnMsgRecv] Failed sending AddMailslot notify.");
			delete lpContext;
			break;
		}

		break;
	}
	case ObEngineRef::MessageType::RemoveMailslotNotify: {
		// Verify message size is at least that of RemoveMailslotNotify
		if (data->dwMsgSize < sizeof(ObEngineRef::RemoveMailslotNotify)) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) is smaller than definition",
				data->dwMsgSize));
			break;
		}

		ObEngineRef::RemoveMailslotNotify* lpTask = (ObEngineRef::RemoveMailslotNotify*)data->lpMessage;

		// Verify message size equals size reported from header
		size_t SizeReported = sizeof(ObEngineRef::RemoveMailslotNotify);

		if (SizeReported != data->dwMsgSize) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) != reported size ({})",
				data->dwMsgSize, SizeReported));
			break;
		}

		NotifyCb::RemoveMailslotContext* lpContext = new NotifyCb::RemoveMailslotContext();
		lpContext->id = lpTask->NotifyId;

		if (!::TrySubmitThreadpoolCallback(NotifyCb::OnRemoveMailslot, lpContext, nullptr)) {
			Helpers::DbgPrint(L"[Error][OnMsgRecv] Failed sending RemoveMailslot notify.");
			delete lpContext;
			break;
		}

		break;
	}
	case ObEngineRef::MessageType::MailslotNotify: {
		// Verify message size is at least that of NotifyData
		if (data->dwMsgSize < sizeof(ObEngineRef::NotifyData)) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) is smaller than definition",
				data->dwMsgSize));
			break;
		}

		ObEngineRef::NotifyData* lpTask = (ObEngineRef::NotifyData*)data->lpMessage;

		// Verify message size equals size reported from header
		size_t SizeReported = lpTask->NotificationSize;
		SizeReported += sizeof(ObEngineRef::NotifyData);

		if (SizeReported != data->dwMsgSize) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) != reported size ({})",
				data->dwMsgSize, SizeReported));
			break;
		}

		// Verify parameter offsets based on size
		auto ExpectedOffset = sizeof(ObEngineRef::NotifyData);
		if (lpTask->NotificationOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Notification offset is {}, expected {}",
				lpTask->NotificationOffset, ExpectedOffset));
			break;
		}

		UCHAR* lpNotificationBuf = new UCHAR[lpTask->NotificationSize];

		::memcpy(lpNotificationBuf, (UCHAR*)lpTask + lpTask->NotificationOffset, lpTask->NotificationSize);

		NotifyCb::NotificationContext* lpContext = new NotifyCb::NotificationContext();
		lpContext->notification = std::wstring((wchar_t*)lpNotificationBuf);
		lpContext->id = lpTask->NotifyId;
		lpContext->inbound = lpTask->Inbound;

		delete[] lpNotificationBuf;

		if (!::TrySubmitThreadpoolCallback(NotifyCb::OnNotification, lpContext, nullptr)) {
			Helpers::DbgPrint(L"[Error][OnMsgRecv] Failed sending Mailslot notification.");
			delete lpContext;
			break;
		}

		break;
	}
	case ObEngineRef::MessageType::AddUdpSocketNotify: {
		// Verify message size is at least that of AddUdpSocketNotify
		if (data->dwMsgSize < sizeof(ObEngineRef::AddUdpSocketNotify)) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) is smaller than definition",
				data->dwMsgSize));
			break;
		}

		ObEngineRef::AddUdpSocketNotify* lpTask = (ObEngineRef::AddUdpSocketNotify*)data->lpMessage;

		// Verify message size equals size reported from header
		size_t SizeReported = lpTask->UdpAddrSize;
		SizeReported += sizeof(ObEngineRef::AddUdpSocketNotify);

		if (SizeReported != data->dwMsgSize) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) != reported size ({})",
				data->dwMsgSize, SizeReported));
			break;
		}

		// Verify parameter offsets based on size
		auto ExpectedOffset = sizeof(ObEngineRef::AddUdpSocketNotify);
		if (lpTask->UdpAddrOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Addr offset is {}, expected {}",
				lpTask->UdpAddrOffset, ExpectedOffset));
			break;
		}

		UCHAR* lpAddrBuf = new UCHAR[lpTask->UdpAddrSize];

		::memcpy(lpAddrBuf, (UCHAR*)lpTask + lpTask->UdpAddrOffset, lpTask->UdpAddrSize);

		NotifyCb::AddUdpSocketContext* lpContext = new NotifyCb::AddUdpSocketContext();
		lpContext->addr = std::wstring((wchar_t*)lpAddrBuf);
		lpContext->port = lpTask->UdpPort;
		lpContext->id   = lpTask->NotifyId;

		delete[] lpAddrBuf;

		if (!::TrySubmitThreadpoolCallback(NotifyCb::OnAddUdpSocket, lpContext, nullptr)) {
			Helpers::DbgPrint(L"[Error][OnMsgRecv] Failed sending AddUdpSocket notify.");
			delete lpContext;
			break;
		}

		break;
	}
	case ObEngineRef::MessageType::RemoveUdpSocketNotify: {
		// Verify message size is at least that of RemoveUdpSocketNotify
		if (data->dwMsgSize < sizeof(ObEngineRef::RemoveUdpSocketNotify)) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) is smaller than definition",
				data->dwMsgSize));
			break;
		}

		ObEngineRef::RemoveUdpSocketNotify* lpTask = (ObEngineRef::RemoveUdpSocketNotify*)data->lpMessage;

		// Verify message size equals size reported from header
		size_t SizeReported = sizeof(ObEngineRef::RemoveUdpSocketNotify);

		if (SizeReported != data->dwMsgSize) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) != reported size ({})",
				data->dwMsgSize, SizeReported));
			break;
		}

		NotifyCb::RemoveUdpSocketContext* lpContext = new NotifyCb::RemoveUdpSocketContext();
		lpContext->id = lpTask->NotifyId;

		if (!::TrySubmitThreadpoolCallback(NotifyCb::OnRemoveUdpSocket, lpContext, nullptr)) {
			Helpers::DbgPrint(L"[Error][OnMsgRecv] Failed sending RemoveUdpSocket notify.");
			delete lpContext;
			break;
		}

		break;
	}
	case ObEngineRef::MessageType::UdpSocketNotify: {
		// Verify message size is at least that of UdpSocketNotify
		if (data->dwMsgSize < sizeof(ObEngineRef::NotifyData)) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) is smaller than definition",
				data->dwMsgSize));
			break;
		}

		ObEngineRef::NotifyData* lpTask = (ObEngineRef::NotifyData*)data->lpMessage;

		// Verify message size equals size reported from header
		size_t SizeReported = lpTask->NotificationSize;
		SizeReported += sizeof(ObEngineRef::NotifyData);

		if (SizeReported != data->dwMsgSize) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) != reported size ({})",
				data->dwMsgSize, SizeReported));
			break;
		}

		// Verify parameter offsets based on size
		auto ExpectedOffset = sizeof(ObEngineRef::NotifyData);
		if (lpTask->NotificationOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Notification offset is {}, expected {}",
				lpTask->NotificationOffset, ExpectedOffset));
			break;
		}

		UCHAR* lpNotificationBuf = new UCHAR[lpTask->NotificationSize];

		::memcpy(lpNotificationBuf, (UCHAR*)lpTask + lpTask->NotificationOffset, lpTask->NotificationSize);

		NotifyCb::NotificationContext* lpContext = new NotifyCb::NotificationContext();
		lpContext->notification = std::wstring((wchar_t*)lpNotificationBuf);
		lpContext->id = lpTask->NotifyId;
		lpContext->inbound = lpTask->Inbound;

		delete[] lpNotificationBuf;

		if (!::TrySubmitThreadpoolCallback(NotifyCb::OnNotification, lpContext, nullptr)) {
			Helpers::DbgPrint(L"[Error][OnMsgRecv] Failed sending UDP socket notification.");
			delete lpContext;
			break;
		}

		break;
	}
	case ObEngineRef::MessageType::CreateUser: {
		// Verify message size is at least that of CreateUserTask
		if (data->dwMsgSize < sizeof(ObEngineRef::CreateUserTask)) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) is smaller than definition",
				data->dwMsgSize));
			break;
		}

		ObEngineRef::CreateUserTask* lpTask = (ObEngineRef::CreateUserTask*)data->lpMessage;

		// Verify message size equals size reported from header
		size_t SizeReported = lpTask->DomainSize;
		SizeReported += lpTask->UserSize;
		SizeReported += lpTask->PassSize;
		SizeReported += sizeof(ObEngineRef::CreateUserTask);

		if (SizeReported != data->dwMsgSize) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) != reported size ({})",
				data->dwMsgSize, SizeReported));
			break;
		}

		// Verify parameter offsets based on size
		auto ExpectedOffset = sizeof(ObEngineRef::CreateUserTask);
		if (lpTask->DomainOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Domain offset is {}, expected {}",
				lpTask->DomainOffset, ExpectedOffset));
			break;
		}

		ExpectedOffset += lpTask->DomainSize;
		if (lpTask->UserOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Username offset is {}, expected {}",
				lpTask->UserOffset, ExpectedOffset));
			break;
		}

		ExpectedOffset += lpTask->UserSize;
		if (lpTask->PassOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Password offset is {}, expected {}",
				lpTask->PassOffset, ExpectedOffset));
			break;
		}

		UCHAR* lpDomainBuf = new UCHAR[lpTask->DomainSize];
		UCHAR* lpUserBuf   = new UCHAR[lpTask->UserSize];
		UCHAR* lpPassBuf   = new UCHAR[lpTask->PassSize];

		::memcpy(lpDomainBuf, (UCHAR*)lpTask + lpTask->DomainOffset, lpTask->DomainSize);
		::memcpy(lpUserBuf,   (UCHAR*)lpTask + lpTask->UserOffset, lpTask->UserSize);
		::memcpy(lpPassBuf,   (UCHAR*)lpTask + lpTask->PassOffset, lpTask->PassSize);

		TaskCb::CreateUserContext* lpContext = new TaskCb::CreateUserContext();
		lpContext->nid      = lpTask->NotifyId;
		lpContext->domain   = std::wstring((wchar_t*)lpDomainBuf);
		lpContext->username = std::wstring((wchar_t*)lpUserBuf);
		lpContext->password = std::wstring((wchar_t*)lpPassBuf);

		delete[] lpDomainBuf;
		delete[] lpUserBuf;
		delete[] lpPassBuf;

		if (!::TrySubmitThreadpoolCallback(TaskCb::OnCreateUser, lpContext, nullptr)) {
			Helpers::DbgPrint(L"[Error][OnMsgRecv] Failed sending CreateUser task.");
			delete lpContext;
			break;
		}

		break;
	}
	case ObEngineRef::MessageType::DeleteUser: {
		// Verify message size is at least that of DeleteUserTask
		if (data->dwMsgSize < sizeof(ObEngineRef::DeleteUserTask)) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) is smaller than definition",
				data->dwMsgSize));
			break;
		}

		ObEngineRef::DeleteUserTask* lpTask = (ObEngineRef::DeleteUserTask*)data->lpMessage;

		// Verify message size equals size reported from header
		size_t SizeReported = lpTask->DomainSize;
		SizeReported += lpTask->UserSize;
		SizeReported += sizeof(ObEngineRef::DeleteUserTask);

		if (SizeReported != data->dwMsgSize) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) != reported size ({})",
				data->dwMsgSize, SizeReported));
			break;
		}

		// Verify parameter offsets based on size
		auto ExpectedOffset = sizeof(ObEngineRef::DeleteUserTask);
		if (lpTask->DomainOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Domain offset is {}, expected {}",
				lpTask->DomainOffset, ExpectedOffset));
			break;
		}

		ExpectedOffset += lpTask->DomainSize;
		if (lpTask->UserOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Username offset is {}, expected {}",
				lpTask->UserOffset, ExpectedOffset));
			break;
		}

		UCHAR* lpDomainBuf = new UCHAR[lpTask->DomainSize];
		UCHAR* lpUserBuf = new UCHAR[lpTask->UserSize];

		::memcpy(lpDomainBuf, (UCHAR*)lpTask + lpTask->DomainOffset, lpTask->DomainSize);
		::memcpy(lpUserBuf, (UCHAR*)lpTask + lpTask->UserOffset, lpTask->UserSize);

		TaskCb::DeleteUserContext* lpContext = new TaskCb::DeleteUserContext();
		lpContext->nid      = lpTask->NotifyId;
		lpContext->domain   = std::wstring((wchar_t*)lpDomainBuf);
		lpContext->username = std::wstring((wchar_t*)lpUserBuf);

		delete[] lpDomainBuf;
		delete[] lpUserBuf;

		if (!::TrySubmitThreadpoolCallback(TaskCb::OnDeleteUser, lpContext, nullptr)) {
			Helpers::DbgPrint(L"[Error][OnMsgRecv] Failed sending DeleteUser task.");
			delete lpContext;
			break;
		}

		break;
	}
	case ObEngineRef::MessageType::CreateGroup: {
		// Verify message size is at least that of CreateGroupTask
		if (data->dwMsgSize < sizeof(ObEngineRef::CreateGroupTask)) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) is smaller than definition",
				data->dwMsgSize));
			break;
		}

		ObEngineRef::CreateGroupTask* lpTask = (ObEngineRef::CreateGroupTask*)data->lpMessage;

		// Verify message size equals size reported from header
		size_t SizeReported = lpTask->DomainSize;
		SizeReported += lpTask->GroupSize;
		SizeReported += lpTask->CommentSize;
		SizeReported += sizeof(ObEngineRef::CreateGroupTask);

		if (SizeReported != data->dwMsgSize) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) != reported size ({})",
				data->dwMsgSize, SizeReported));
			break;
		}

		// Verify parameter offsets based on size
		auto ExpectedOffset = sizeof(ObEngineRef::CreateGroupTask);
		if (lpTask->DomainOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Domain offset is {}, expected {}",
				lpTask->DomainOffset, ExpectedOffset));
			break;
		}

		ExpectedOffset += lpTask->DomainSize;
		if (lpTask->GroupOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Group offset is {}, expected {}",
				lpTask->GroupOffset, ExpectedOffset));
			break;
		}

		ExpectedOffset += lpTask->GroupSize;
		if (lpTask->CommentOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Comment offset is {}, expected {}",
				lpTask->CommentOffset, ExpectedOffset));
			break;
		}

		UCHAR* lpDomainBuf  = new UCHAR[lpTask->DomainSize];
		UCHAR* lpGroupBuf   = new UCHAR[lpTask->GroupSize];
		UCHAR* lpCommentBuf = new UCHAR[lpTask->CommentSize];

		::memcpy(lpDomainBuf,  (UCHAR*)lpTask + lpTask->DomainOffset,  lpTask->DomainSize);
		::memcpy(lpGroupBuf,   (UCHAR*)lpTask + lpTask->GroupOffset,   lpTask->GroupSize);
		::memcpy(lpCommentBuf, (UCHAR*)lpTask + lpTask->CommentOffset, lpTask->CommentSize);

		TaskCb::CreateGroupContext* lpContext = new TaskCb::CreateGroupContext();
		lpContext->nid     = lpTask->NotifyId;
		lpContext->domain  = std::wstring((wchar_t*)lpDomainBuf);
		lpContext->group   = std::wstring((wchar_t*)lpGroupBuf);
		lpContext->comment = std::wstring((wchar_t*)lpCommentBuf);

		delete[] lpDomainBuf;
		delete[] lpGroupBuf;
		delete[] lpCommentBuf;

		if (!::TrySubmitThreadpoolCallback(TaskCb::OnCreateGroup, lpContext, nullptr)) {
			Helpers::DbgPrint(L"[Error][OnMsgRecv] Failed sending CreateGroup task.");
			delete lpContext;
			break;
		}

		break;
	}
	case ObEngineRef::MessageType::DeleteGroup: {
		// Verify message size is at least that of DeleteGroupTask
		if (data->dwMsgSize < sizeof(ObEngineRef::DeleteGroupTask)) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) is smaller than definition",
				data->dwMsgSize));
			break;
		}

		ObEngineRef::DeleteGroupTask* lpTask = (ObEngineRef::DeleteGroupTask*)data->lpMessage;

		// Verify message size equals size reported from header
		size_t SizeReported = lpTask->DomainSize;
		SizeReported += lpTask->GroupSize;
		SizeReported += sizeof(ObEngineRef::DeleteGroupTask);

		if (SizeReported != data->dwMsgSize) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) != reported size ({})",
				data->dwMsgSize, SizeReported));
			break;
		}

		// Verify parameter offsets based on size
		auto ExpectedOffset = sizeof(ObEngineRef::DeleteGroupTask);
		if (lpTask->DomainOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Domain offset is {}, expected {}",
				lpTask->DomainOffset, ExpectedOffset));
			break;
		}

		ExpectedOffset += lpTask->DomainSize;
		if (lpTask->GroupOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Group offset is {}, expected {}",
				lpTask->GroupOffset, ExpectedOffset));
			break;
		}

		UCHAR* lpDomainBuf = new UCHAR[lpTask->DomainSize];
		UCHAR* lpGroupBuf  = new UCHAR[lpTask->GroupSize];

		::memcpy(lpDomainBuf, (UCHAR*)lpTask + lpTask->DomainOffset, lpTask->DomainSize);
		::memcpy(lpGroupBuf,  (UCHAR*)lpTask + lpTask->GroupOffset,  lpTask->GroupSize);

		TaskCb::DeleteGroupContext* lpContext = new TaskCb::DeleteGroupContext();
		lpContext->nid    = lpTask->NotifyId;
		lpContext->domain = std::wstring((wchar_t*)lpDomainBuf);
		lpContext->group  = std::wstring((wchar_t*)lpGroupBuf);

		delete[] lpDomainBuf;
		delete[] lpGroupBuf;

		if (!::TrySubmitThreadpoolCallback(TaskCb::OnDeleteGroup, lpContext, nullptr)) {
			Helpers::DbgPrint(L"[Error][OnMsgRecv] Failed sending DeleteGroup task.");
			delete lpContext;
			break;
		}

		break;
	}
	case ObEngineRef::MessageType::AddGroupMember: {
		// Verify message size is at least that of AddGroupMemberTask
		if (data->dwMsgSize < sizeof(ObEngineRef::AddGroupMemberTask)) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) is smaller than definition",
				data->dwMsgSize));
			break;
		}

		ObEngineRef::AddGroupMemberTask* lpTask = (ObEngineRef::AddGroupMemberTask*)data->lpMessage;

		// Verify message size equals size reported from header
		size_t SizeReported = lpTask->DomainSize;
		SizeReported += lpTask->GroupSize;
		SizeReported += lpTask->MemberSize;
		SizeReported += sizeof(ObEngineRef::AddGroupMemberTask);

		if (SizeReported != data->dwMsgSize) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) != reported size ({})",
				data->dwMsgSize, SizeReported));
			break;
		}

		// Verify parameter offsets based on size
		auto ExpectedOffset = sizeof(ObEngineRef::AddGroupMemberTask);
		if (lpTask->DomainOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Domain offset is {}, expected {}",
				lpTask->DomainOffset, ExpectedOffset));
			break;
		}

		ExpectedOffset += lpTask->DomainSize;
		if (lpTask->GroupOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Group offset is {}, expected {}",
				lpTask->GroupOffset, ExpectedOffset));
			break;
		}

		ExpectedOffset += lpTask->GroupSize;
		if (lpTask->MemberOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Member offset is {}, expected {}",
				lpTask->MemberOffset, ExpectedOffset));
			break;
		}

		UCHAR* lpDomainBuf = new UCHAR[lpTask->DomainSize];
		UCHAR* lpGroupBuf  = new UCHAR[lpTask->GroupSize];
		UCHAR* lpMemberBuf = new UCHAR[lpTask->MemberSize];

		::memcpy(lpDomainBuf, (UCHAR*)lpTask + lpTask->DomainOffset, lpTask->DomainSize);
		::memcpy(lpGroupBuf,  (UCHAR*)lpTask + lpTask->GroupOffset,  lpTask->GroupSize);
		::memcpy(lpMemberBuf, (UCHAR*)lpTask + lpTask->MemberOffset, lpTask->MemberSize);

		TaskCb::AddGroupMemberContext* lpContext = new TaskCb::AddGroupMemberContext();
		lpContext->nid    = lpTask->NotifyId;
		lpContext->domain = std::wstring((wchar_t*)lpDomainBuf);
		lpContext->group  = std::wstring((wchar_t*)lpGroupBuf);
		lpContext->member = std::wstring((wchar_t*)lpMemberBuf);

		delete[] lpDomainBuf;
		delete[] lpGroupBuf;
		delete[] lpMemberBuf;

		if (!::TrySubmitThreadpoolCallback(TaskCb::OnAddGroupMember, lpContext, nullptr)) {
			Helpers::DbgPrint(L"[Error][OnMsgRecv] Failed sending AddGroupMember task.");
			delete lpContext;
			break;
		}

		break;
	}
	case ObEngineRef::MessageType::RemoveGroupMember: {
		// Verify message size is at least that of RemoveGroupMemberTask
		if (data->dwMsgSize < sizeof(ObEngineRef::RemoveGroupMemberTask)) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) is smaller than definition",
				data->dwMsgSize));
			break;
		}

		ObEngineRef::RemoveGroupMemberTask* lpTask = (ObEngineRef::RemoveGroupMemberTask*)data->lpMessage;

		// Verify message size equals size reported from header
		size_t SizeReported = lpTask->DomainSize;
		SizeReported += lpTask->GroupSize;
		SizeReported += lpTask->MemberSize;
		SizeReported += sizeof(ObEngineRef::RemoveGroupMemberTask);

		if (SizeReported != data->dwMsgSize) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) != reported size ({})",
				data->dwMsgSize, SizeReported));
			break;
		}

		// Verify parameter offsets based on size
		auto ExpectedOffset = sizeof(ObEngineRef::RemoveGroupMemberTask);
		if (lpTask->DomainOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Domain offset is {}, expected {}",
				lpTask->DomainOffset, ExpectedOffset));
			break;
		}

		ExpectedOffset += lpTask->DomainSize;
		if (lpTask->GroupOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Group offset is {}, expected {}",
				lpTask->GroupOffset, ExpectedOffset));
			break;
		}

		ExpectedOffset += lpTask->GroupSize;
		if (lpTask->MemberOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Member offset is {}, expected {}",
				lpTask->MemberOffset, ExpectedOffset));
			break;
		}

		UCHAR* lpDomainBuf = new UCHAR[lpTask->DomainSize];
		UCHAR* lpGroupBuf  = new UCHAR[lpTask->GroupSize];
		UCHAR* lpMemberBuf = new UCHAR[lpTask->MemberSize];

		::memcpy(lpDomainBuf, (UCHAR*)lpTask + lpTask->DomainOffset, lpTask->DomainSize);
		::memcpy(lpGroupBuf,  (UCHAR*)lpTask + lpTask->GroupOffset,  lpTask->GroupSize);
		::memcpy(lpMemberBuf, (UCHAR*)lpTask + lpTask->MemberOffset, lpTask->MemberSize);

		TaskCb::RemoveGroupMemberContext* lpContext = new TaskCb::RemoveGroupMemberContext();
		lpContext->nid = lpTask->NotifyId;
		lpContext->domain = std::wstring((wchar_t*)lpDomainBuf);
		lpContext->group = std::wstring((wchar_t*)lpGroupBuf);
		lpContext->member = std::wstring((wchar_t*)lpMemberBuf);

		delete[] lpDomainBuf;
		delete[] lpGroupBuf;
		delete[] lpMemberBuf;

		if (!::TrySubmitThreadpoolCallback(TaskCb::OnRemoveGroupMember, lpContext, nullptr)) {
			Helpers::DbgPrint(L"[Error][OnMsgRecv] Failed sending RemoveGroupMember task.");
			delete lpContext;
			break;
		}

		break;
	}
	case ObEngineRef::MessageType::AddPrivilege: {
		// Verify message size is at least that of AddPrivilegeTask
		if (data->dwMsgSize < sizeof(ObEngineRef::AddPrivilegeTask)) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) is smaller than definition",
				data->dwMsgSize));
			break;
		}

		ObEngineRef::AddPrivilegeTask* lpTask = (ObEngineRef::AddPrivilegeTask*)data->lpMessage;

		// Verify message size equals size reported from header
		size_t SizeReported = lpTask->EntitySize;
		SizeReported += lpTask->PrivSize;
		SizeReported += sizeof(ObEngineRef::AddPrivilegeTask);

		if (SizeReported != data->dwMsgSize) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) != reported size ({})",
				data->dwMsgSize, SizeReported));
			break;
		}

		// Verify parameter offsets based on size
		auto ExpectedOffset = sizeof(ObEngineRef::AddPrivilegeTask);
		if (lpTask->EntityOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Entity offset is {}, expected {}",
				lpTask->EntityOffset, ExpectedOffset));
			break;
		}

		ExpectedOffset += lpTask->EntitySize;
		if (lpTask->PrivOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Privilege offset is {}, expected {}",
				lpTask->PrivOffset, ExpectedOffset));
			break;
		}

		UCHAR* lpEntityBuf = new UCHAR[lpTask->EntitySize];
		UCHAR* lpPrivBuf   = new UCHAR[lpTask->PrivSize];

		::memcpy(lpEntityBuf, (UCHAR*)lpTask + lpTask->EntityOffset, lpTask->EntitySize);
		::memcpy(lpPrivBuf, (UCHAR*)lpTask + lpTask->PrivOffset, lpTask->PrivSize);

		TaskCb::AddPrivilegeContext* lpContext = new TaskCb::AddPrivilegeContext();
		lpContext->nid       = lpTask->NotifyId;
		lpContext->entity    = std::wstring((wchar_t*)lpEntityBuf);
		lpContext->privilege = std::wstring((wchar_t*)lpPrivBuf);

		delete[] lpEntityBuf;
		delete[] lpPrivBuf;

		if (!::TrySubmitThreadpoolCallback(TaskCb::OnAddPrivilege, lpContext, nullptr)) {
			Helpers::DbgPrint(L"[Error][OnMsgRecv] Failed sending AddPrivilege task.");
			delete lpContext;
			break;
		}

		break;
	}
	case ObEngineRef::MessageType::RemovePrivilege: {
		// Verify message size is at least that of RemovePrivilegeTask
		if (data->dwMsgSize < sizeof(ObEngineRef::RemovePrivilegeTask)) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) is smaller than definition",
				data->dwMsgSize));
			break;
		}

		ObEngineRef::RemovePrivilegeTask* lpTask = (ObEngineRef::RemovePrivilegeTask*)data->lpMessage;

		// Verify message size equals size reported from header
		size_t SizeReported = lpTask->EntitySize;
		SizeReported += lpTask->PrivSize;
		SizeReported += sizeof(ObEngineRef::RemovePrivilegeTask);

		if (SizeReported != data->dwMsgSize) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) != reported size ({})",
				data->dwMsgSize, SizeReported));
			break;
		}

		// Verify parameter offsets based on size
		auto ExpectedOffset = sizeof(ObEngineRef::RemovePrivilegeTask);
		if (lpTask->EntityOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Entity offset is {}, expected {}",
				lpTask->EntityOffset, ExpectedOffset));
			break;
		}

		ExpectedOffset += lpTask->EntitySize;
		if (lpTask->PrivOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Privilege offset is {}, expected {}",
				lpTask->PrivOffset, ExpectedOffset));
			break;
		}

		UCHAR* lpEntityBuf = new UCHAR[lpTask->EntitySize];
		UCHAR* lpPrivBuf = new UCHAR[lpTask->PrivSize];

		::memcpy(lpEntityBuf, (UCHAR*)lpTask + lpTask->EntityOffset, lpTask->EntitySize);
		::memcpy(lpPrivBuf, (UCHAR*)lpTask + lpTask->PrivOffset, lpTask->PrivSize);

		TaskCb::RemovePrivilegeContext* lpContext = new TaskCb::RemovePrivilegeContext();
		lpContext->nid = lpTask->NotifyId;
		lpContext->entity = std::wstring((wchar_t*)lpEntityBuf);
		lpContext->privilege = std::wstring((wchar_t*)lpPrivBuf);

		delete[] lpEntityBuf;
		delete[] lpPrivBuf;

		if (!::TrySubmitThreadpoolCallback(TaskCb::OnRemovePrivilege, lpContext, nullptr)) {
			Helpers::DbgPrint(L"[Error][OnMsgRecv] Failed sending RemovePrivilege task.");
			delete lpContext;
			break;
		}

		break;
	}
	case ObEngineRef::MessageType::SpawnProcess: {
		// Verify message size is at least that of SpawnProcessTask
		if (data->dwMsgSize < sizeof(ObEngineRef::SpawnProcessTask)) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) is smaller than definition",
				data->dwMsgSize));
			break;
		}

		ObEngineRef::SpawnProcessTask* lpTask = (ObEngineRef::SpawnProcessTask*)data->lpMessage;

		// Verify message size equals size reported from header
		size_t SizeReported = lpTask->AppSize;
		SizeReported += lpTask->ArgsSize;
		SizeReported += lpTask->CwdSize;
		SizeReported += lpTask->DomainSize;
		SizeReported += lpTask->UserSize;
		SizeReported += sizeof(ObEngineRef::SpawnProcessTask);

		if (SizeReported != data->dwMsgSize) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Message size ({}) != reported size ({})",
				data->dwMsgSize, SizeReported));
			break;
		}

		// Verify parameter offsets based on size
		auto ExpectedOffset = sizeof(ObEngineRef::SpawnProcessTask);
		if (lpTask->AppOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] App offset is {}, expected {}",
				lpTask->AppOffset, ExpectedOffset));
			break;
		}

		ExpectedOffset += lpTask->AppSize;
		if (lpTask->ArgsOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Args offset is {}, expected {}",
				lpTask->ArgsOffset, ExpectedOffset));
			break;
		}

		ExpectedOffset += lpTask->ArgsSize;
		if (lpTask->CwdOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Cwd offset is {}, expected {}",
				lpTask->CwdOffset, ExpectedOffset));
			break;
		}

		ExpectedOffset += lpTask->CwdSize;
		if (lpTask->DomainOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Domain offset is {}, expected {}",
				lpTask->DomainOffset, ExpectedOffset));
			break;
		}

		ExpectedOffset += lpTask->DomainSize;
		if (lpTask->UserOffset != ExpectedOffset) {
			Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] User offset is {}, expected {}",
				lpTask->UserOffset, ExpectedOffset));
			break;
		}

		UCHAR* lpAppBuf    = new UCHAR[lpTask->AppSize];
		UCHAR* lpArgsBuf   = new UCHAR[lpTask->ArgsSize];
		UCHAR* lpCwdBuf    = new UCHAR[lpTask->CwdSize];
		UCHAR* lpDomainBuf = new UCHAR[lpTask->DomainSize];
		UCHAR* lpUserBuf   = new UCHAR[lpTask->UserSize];

		::memcpy(lpAppBuf,    (UCHAR*)lpTask + lpTask->AppOffset,    lpTask->AppSize);
		::memcpy(lpArgsBuf,   (UCHAR*)lpTask + lpTask->ArgsOffset,   lpTask->ArgsSize);
		::memcpy(lpCwdBuf,    (UCHAR*)lpTask + lpTask->CwdOffset,    lpTask->CwdSize);
		::memcpy(lpDomainBuf, (UCHAR*)lpTask + lpTask->DomainOffset, lpTask->DomainSize);
		::memcpy(lpUserBuf,   (UCHAR*)lpTask + lpTask->UserOffset,   lpTask->UserSize);

		TaskCb::SpawnProcessContext* lpContext = new TaskCb::SpawnProcessContext();
		lpContext->nid      = lpTask->NotifyId;
		lpContext->app      = std::wstring((wchar_t*)lpAppBuf);
		lpContext->args     = std::wstring((wchar_t*)lpArgsBuf);
		lpContext->cwd      = std::wstring((wchar_t*)lpCwdBuf);
		lpContext->domain   = std::wstring((wchar_t*)lpDomainBuf);
		lpContext->username = std::wstring((wchar_t*)lpUserBuf);

		delete[] lpAppBuf;
		delete[] lpArgsBuf;
		delete[] lpCwdBuf;
		delete[] lpDomainBuf;
		delete[] lpUserBuf;

		if (!::TrySubmitThreadpoolCallback(TaskCb::OnSpawnProcess, lpContext, nullptr)) {
			Helpers::DbgPrint(L"[Error][OnMsgRecv] Failed sending SpawnProcess task.");
			delete lpContext;
			break;
		}

		break;
	}
	default: {
		Helpers::DbgPrint(std::format(L"[Error][OnMsgRecv] Unknown message type ({})",
			(short)data->lpMessage->Type));
		break;
	}
	}

	// Cleanup
	free(data->lpMessage);
	delete data;
}


void NotifyCb::OnAddMailslot(PTP_CALLBACK_INSTANCE instance, PVOID context) {
	auto data = static_cast<AddMailslotContext*>(context);

	if (!g_router->AddMailslotRoute(data->id, data->slotname, data->netbios)) {
		Helpers::DbgPrint(std::format(L"[Error][OnAddMailslot] Failed to add mailslot route: {}",
			data->id));
	}
	else {
		Helpers::DbgPrint(std::format(L"[Info][OnAddMailslot] Added mailslot route (ID:{}) -> \\\\{}\\mailslot\\{}",
			data->id, data->netbios, data->slotname));
	}
	
	delete data;
}

void NotifyCb::OnRemoveMailslot(PTP_CALLBACK_INSTANCE instance, PVOID context) {
	auto data = static_cast<RemoveMailslotContext*>(context);

	if (!g_router->RemoveRoute(data->id)) {
		Helpers::DbgPrint(std::format(L"[Error][OnRemoveMailslot] Failed to remove mailslot route: {}",
			data->id));
	}
	else {
		Helpers::DbgPrint(std::format(L"[Info][OnRemoveMailslot] Removed mailslot route: {}",
			data->id));
	}

	delete data;
}


void NotifyCb::OnAddUdpSocket(PTP_CALLBACK_INSTANCE instance, PVOID context) {
	auto data = static_cast<AddUdpSocketContext*>(context);

	if (!g_router->AddUdpSocketRoute(data->id, data->addr, data->port)) {
		Helpers::DbgPrint(std::format(L"[Error][OnAddUdpSocket] Failed to add UDP socket route: {}",
			data->id));
	}
	else {
		Helpers::DbgPrint(std::format(L"[Info][OnAddUdpSocket] Added UDP socket route (ID:{}) -> udp:socket://{}:{}",
			data->id, data->addr, data->port));
	}

	delete data;
}

void NotifyCb::OnRemoveUdpSocket(PTP_CALLBACK_INSTANCE instance, PVOID context) {
	auto data = static_cast<RemoveUdpSocketContext*>(context);

	if (!g_router->RemoveRoute(data->id)) {
		Helpers::DbgPrint(std::format(L"[Error][OnRemoveUdpSocket] Failed to remove UDP socket route: {}",
			data->id));
	}
	else {
		Helpers::DbgPrint(std::format(L"[Info][OnRemoveUdpSocket] Removed UDP socket route: {}",
			data->id));
	}

	delete data;
}


void NotifyCb::OnNotification(PTP_CALLBACK_INSTANCE instance, PVOID context) {
	auto data = static_cast<NotificationContext*>(context);

	if (data->inbound == true) {
		Helpers::DbgPrint(std::format(L"[Info][OnNotification] INBOUND notification: {}",
			data->notification));
	}
	else {
		/*
			If the notification is OUTBOUND then we look up the notify
			configuration for the given ID (data->id), change inbound
			to true, and then send the notification with an appropriate
			transport client.
		*/
		Router::Route route;
		if (g_router->GetRoute(data->id, route)) {
			std::unique_ptr<Transport::Client, std::default_delete<Transport::Client>> client;
			ObEngineRef::NotifyData* lpTask = nullptr;
			DWORD dwMsgSize;
			std::wstring RouteInfo;

			// Configure client based on route type
			auto routetype = std::any_cast<Router::RouteType>(route["type"]);
			
			if (routetype == Router::RouteType::Mailslot) {
				auto netbios  = std::any_cast<std::wstring>(route["netbios"]);
				auto slotname = std::any_cast<std::wstring>(route["slot"]);

				RouteInfo = std::format(L"\\\\{}\\mailslot\\{}", netbios, slotname);
				
				client = std::make_unique<Mailslot::Client>(slotname, netbios);

				if (!MessageFk::MailslotNotifyMessage(data->id, data->notification, &lpTask, dwMsgSize)) {
					Helpers::DbgPrint(L"[Error][OnNotification] MailslotNotifyMessage() failed.");
					delete data;
					return;
				}
			}
			else if (routetype == Router::RouteType::UdpSocket) {
				auto addr = std::any_cast<std::wstring>(route["addr"]);
				auto port = std::any_cast<USHORT>(route["port"]);

				RouteInfo = std::format(L"udp:socket://{}:{}", addr, port);

				client = std::make_unique<UdpSocket::Client>(addr, port);

				if (!MessageFk::UdpSocketNotifyMessage(data->id, data->notification, &lpTask, dwMsgSize)) {
					Helpers::DbgPrint(L"[Error][OnNotification] UdpSocketNotifyMessage() failed.");
					delete data;
					return;
				}
			}
			else {
				Helpers::DbgPrint(L"[Error][OnNotification] Unknown route type defined.");
				delete data;
				return;
			}

			// Connect and send notification
			if (client->Connect()) {
				lpTask->Inbound = true;

				if (!client->Send(*lpTask, dwMsgSize)) {
					Helpers::DbgPrint(std::format(
						L"[Error][OnNotification] OUTBOUND Notification send failure to '{}'", RouteInfo));
				}
				else {
					Helpers::DbgPrint(std::format(
						L"[Info][OnNotification] OUTBOUND Notification sent to '{}'", RouteInfo));
				}
			}
			else {
				Helpers::DbgPrint(std::format(
					L"[Error][OnNotification] Client failed to connect to '{}'", RouteInfo));
			}

			free(lpTask);
		}
	}

	delete data;
}


void TaskCb::OnCreateUser(PTP_CALLBACK_INSTANCE instance, PVOID context) {
	auto data = static_cast<CreateUserContext*>(context);

	Helpers::DbgPrint(std::format(L"[Info][OnCreateUser] Creating user {}\\{}:{}",
		data->domain, data->username, data->password));

	bool result = WinSecurity::CreateUser(data->username, data->password);

	if (data->nid != 0) {
		// Send user creation notification
		NotifyCb::NotificationContext* lpContext = new NotifyCb::NotificationContext();
		
		if (result == false)
			lpContext->notification = L"Failed to create user";
		else
			lpContext->notification = L"Successfully created user";
		
		lpContext->id           = data->nid;
		lpContext->inbound      = false;

		if (!::TrySubmitThreadpoolCallback(NotifyCb::OnNotification, lpContext, nullptr)) {
			Helpers::DbgPrint(L"[Error][OnCreateUser] Failed sending Mailslot notification.");
			delete lpContext;
		}
	}

	delete data;
}

void TaskCb::OnDeleteUser(PTP_CALLBACK_INSTANCE instance, PVOID context) {
	auto data = static_cast<DeleteUserContext*>(context);

	Helpers::DbgPrint(std::format(L"[Info][OnDeleteUser] Deleting user {}\\{}",
		data->domain, data->username));

	bool result = WinSecurity::DeleteUser(data->username);

	if (data->nid != 0) {
		// Send user deletion notification
		NotifyCb::NotificationContext* lpContext = new NotifyCb::NotificationContext();
		
		if (result == false)
			lpContext->notification = L"Failed to delete user";
		else
			lpContext->notification = L"Successfully deleted user";

		lpContext->id           = data->nid;
		lpContext->inbound      = false;

		if (!::TrySubmitThreadpoolCallback(NotifyCb::OnNotification, lpContext, nullptr)) {
			Helpers::DbgPrint(L"[Error][OnDeleteUser] Failed sending Mailslot notification.");
			delete lpContext;
		}
	}

	delete data;
}

void TaskCb::OnCreateGroup(PTP_CALLBACK_INSTANCE instance, PVOID context) {
	auto data = static_cast<CreateGroupContext*>(context);

	Helpers::DbgPrint(std::format(L"[Info][OnCreateGroup] Creating group {}\\{} - {}",
		data->domain, data->group, data->comment));

	bool result = WinSecurity::CreateGroup(data->group, data->comment);

	if (data->nid != 0) {
		// Send group creation notification
		NotifyCb::NotificationContext* lpContext = new NotifyCb::NotificationContext();

		if (result == false)
			lpContext->notification = L"Failed to create group";
		else
			lpContext->notification = L"Successfully created group";

		lpContext->id = data->nid;
		lpContext->inbound = false;

		if (!::TrySubmitThreadpoolCallback(NotifyCb::OnNotification, lpContext, nullptr)) {
			Helpers::DbgPrint(L"[Error][OnCreateGroup] Failed sending Mailslot notification.");
			delete lpContext;
		}
	}

	delete data;
}

void TaskCb::OnDeleteGroup(PTP_CALLBACK_INSTANCE instance, PVOID context) {
	auto data = static_cast<DeleteGroupContext*>(context);

	Helpers::DbgPrint(std::format(L"[Info][OnDeleteGroup] Deleting group {}\\{}",
		data->domain, data->group));

	bool result = WinSecurity::DeleteGroup(data->group);

	if (data->nid != 0) {
		// Send group deletion notification
		NotifyCb::NotificationContext* lpContext = new NotifyCb::NotificationContext();

		if (result == false)
			lpContext->notification = L"Failed to delete group";
		else
			lpContext->notification = L"Successfully deleted group";

		lpContext->id = data->nid;
		lpContext->inbound = false;

		if (!::TrySubmitThreadpoolCallback(NotifyCb::OnNotification, lpContext, nullptr)) {
			Helpers::DbgPrint(L"[Error][OnDeleteGroup] Failed sending Mailslot notification.");
			delete lpContext;
		}
	}

	delete data;
}

void TaskCb::OnAddGroupMember(PTP_CALLBACK_INSTANCE instance, PVOID context) {
	auto data = static_cast<AddGroupMemberContext*>(context);

	Helpers::DbgPrint(std::format(L"[Info][OnAddGroupMember] Adding member {} to group {}\\{}",
		data->member, data->domain, data->group));

	bool result = WinSecurity::AddGroupMember(data->group, data->member);

	if (data->nid != 0) {
		// Send group creation notification
		NotifyCb::NotificationContext* lpContext = new NotifyCb::NotificationContext();

		if (result == false)
			lpContext->notification = L"Failed to add group member";
		else
			lpContext->notification = L"Successfully added group member";

		lpContext->id = data->nid;
		lpContext->inbound = false;

		if (!::TrySubmitThreadpoolCallback(NotifyCb::OnNotification, lpContext, nullptr)) {
			Helpers::DbgPrint(L"[Error][OnAddGroupMember] Failed sending Mailslot notification.");
			delete lpContext;
		}
	}

	delete data;
}

void TaskCb::OnRemoveGroupMember(PTP_CALLBACK_INSTANCE instance, PVOID context) {
	auto data = static_cast<RemoveGroupMemberContext*>(context);

	Helpers::DbgPrint(std::format(L"[Info][OnRemoveGroupMember] Removing member {} from group {}\\{}",
		data->member, data->domain, data->group));

	bool result = WinSecurity::RemoveGroupMember(data->group, data->member);

	if (data->nid != 0) {
		// Send group creation notification
		NotifyCb::NotificationContext* lpContext = new NotifyCb::NotificationContext();

		if (result == false)
			lpContext->notification = L"Failed to remove group member";
		else
			lpContext->notification = L"Successfully removed group member";

		lpContext->id = data->nid;
		lpContext->inbound = false;

		if (!::TrySubmitThreadpoolCallback(NotifyCb::OnNotification, lpContext, nullptr)) {
			Helpers::DbgPrint(L"[Error][OnRemoveGroupMember] Failed sending Mailslot notification.");
			delete lpContext;
		}
	}

	delete data;
}

void TaskCb::OnAddPrivilege(PTP_CALLBACK_INSTANCE instance, PVOID context) {
	auto data = static_cast<AddPrivilegeContext*>(context);

	Helpers::DbgPrint(std::format(L"[Info][OnAddPrivilege] Adding '{}' privilege to {}",
		data->privilege, data->entity));

	bool result = WinSecurity::AddPrivilege(data->entity, data->privilege);

	if (data->nid != 0) {
		// Send group deletion notification
		NotifyCb::NotificationContext* lpContext = new NotifyCb::NotificationContext();

		if (result == false)
			lpContext->notification = L"Failed to add privilege";
		else
			lpContext->notification = L"Successfully added privilege";

		lpContext->id = data->nid;
		lpContext->inbound = false;

		if (!::TrySubmitThreadpoolCallback(NotifyCb::OnNotification, lpContext, nullptr)) {
			Helpers::DbgPrint(L"[Error][OnAddPrivilege] Failed sending Mailslot notification.");
			delete lpContext;
		}
	}

	delete data;
}

void TaskCb::OnRemovePrivilege(PTP_CALLBACK_INSTANCE instance, PVOID context) {
	auto data = static_cast<RemovePrivilegeContext*>(context);

	Helpers::DbgPrint(std::format(L"[Info][OnRemovePrivilege] Removing '{}' privilege from {}",
		data->privilege, data->entity));

	bool result = WinSecurity::RemovePrivilege(data->entity, data->privilege);

	if (data->nid != 0) {
		// Send group deletion notification
		NotifyCb::NotificationContext* lpContext = new NotifyCb::NotificationContext();

		if (result == false)
			lpContext->notification = L"Failed to remove privilege";
		else
			lpContext->notification = L"Successfully removed privilege";

		lpContext->id = data->nid;
		lpContext->inbound = false;

		if (!::TrySubmitThreadpoolCallback(NotifyCb::OnNotification, lpContext, nullptr)) {
			Helpers::DbgPrint(L"[Error][OnRemovePrivilege] Failed sending Mailslot notification.");
			delete lpContext;
		}
	}

	delete data;
}

void TaskCb::OnSpawnProcess(PTP_CALLBACK_INSTANCE instance, PVOID context) {
	/*
		TODO: Review logic to determine if the implementation or
		priority can/should be improved.

		We only spawns a single process even if multiple sessions are
		found for a given user. Therefore, we must prioritize certain
		sessions over others. The following describes the priority
		logic and order, with a larger number giving higher priority.

		[Logon Type]
			https://learn.microsoft.com/en-us/windows/win32/api/ntsecapi/ne-ntsecapi-security_logon_type
			
			(90) Interactive
			(80) CachedInteractive
			(70) RemoteInteractive
			(60) CachedRemoteInteractive
			(50) NetworkCleartext
			(40) Network
			(30) Service
			(20) Batch
			(10) Other
			
			If a priority logon type is not found then any enumerated
			session can be used. It will depend on order when processing.
			
			If the priority level is 0 after processing all enumerated
			sessions then a process will not spawn. This should only
			occur when 0 sessions are found.
	*/
	
	auto data = static_cast<SpawnProcessContext*>(context);
	
	Helpers::DbgPrint(std::format(L"[Info][OnSpawnProcess] Spawning process:\n\tApp : {}\n\tArgs: {}\n\tCWD : {}\n\tUser: {}\\{}",
		data->app, data->args, data->cwd, data->domain, data->username));

	WinSecurity::Sessions sessions;
	bool result = WinSecurity::GetUserSessions(data->domain, data->username, sessions);

	// Spawn process if a session was found, using priority for multiple
	if (!sessions.empty()) {
		// Track priority, 0 meaning a priority hasn't been set for a session
		DWORD CurrentPriorityLevel = 0;

		// Session index to use
		size_t SessionIndex = sessions.size();

		for (size_t i = 0; i < sessions.size(); i++) {
			DWORD CurrentLogonType = std::any_cast<DWORD>(sessions[i][L"LogonType"]);

			switch (CurrentLogonType) {
			case SECURITY_LOGON_TYPE::Interactive: {
				if (CurrentPriorityLevel < 90) {
					CurrentPriorityLevel = 90;
					SessionIndex = i;
				}

				break;
			}
			case SECURITY_LOGON_TYPE::CachedInteractive: {
				if (CurrentPriorityLevel < 80) {
					CurrentPriorityLevel = 80;
					SessionIndex = i;
				}

				break;
			}
			case SECURITY_LOGON_TYPE::RemoteInteractive: {
				if (CurrentPriorityLevel < 70) {
					CurrentPriorityLevel = 70;
					SessionIndex = i;
				}

				break;
			}
			case SECURITY_LOGON_TYPE::CachedRemoteInteractive: {
				if (CurrentPriorityLevel < 60) {
					CurrentPriorityLevel = 60;
					SessionIndex = i;
				}

				break;
			}
			case SECURITY_LOGON_TYPE::NetworkCleartext: {
				if (CurrentPriorityLevel < 50) {
					CurrentPriorityLevel = 50;
					SessionIndex = i;
				}

				break;
			}
			case SECURITY_LOGON_TYPE::Network: {
				if (CurrentPriorityLevel < 40) {
					CurrentPriorityLevel = 40;
					SessionIndex = i;
				}

				break;
			}
			case SECURITY_LOGON_TYPE::Service: {
				if (CurrentPriorityLevel < 30) {
					CurrentPriorityLevel = 30;
					SessionIndex = i;
				}

				break;
			}
			case SECURITY_LOGON_TYPE::Batch: {
				if (CurrentPriorityLevel < 20) {
					CurrentPriorityLevel = 20;
					SessionIndex = i;
				}

				break;
			}
			default: {
				if (CurrentPriorityLevel < 10) {
					CurrentPriorityLevel = 10;
					SessionIndex = i;
				}

				break;
			}
			}
		}

		// Spawn process if session found
		if (CurrentPriorityLevel > 0 && SessionIndex < sessions.size()) {
			DWORD dwSessId = std::any_cast<DWORD>(sessions[SessionIndex][L"Session"]);
			result = WinSecurity::SpawnProcessInSession(data->app, data->args, data->cwd, dwSessId, true);
		}
		else {
			result = false;
		}
	}
	else {
		result = false;
	}
	
	// Send status notification if requested
	if (data->nid != 0) {
		// Send spawn process notification
		NotifyCb::NotificationContext* lpContext = new NotifyCb::NotificationContext();

		if (result == false)
			lpContext->notification = L"Failed to spawn process";
		else
			lpContext->notification = L"Successfully spawned process";

		lpContext->id = data->nid;
		lpContext->inbound = false;

		if (!::TrySubmitThreadpoolCallback(NotifyCb::OnNotification, lpContext, nullptr)) {
			Helpers::DbgPrint(L"[Error][OnSpawnProcess] Failed sending Mailslot notification.");
			delete lpContext;
		}
	}

	delete data;
}