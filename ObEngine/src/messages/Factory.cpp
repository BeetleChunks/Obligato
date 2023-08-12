#include "pch.h"
#include "Factory.h"


ULONGLONG MessageFk::CalcMailslotNotifyId(
	_In_  const std::wstring& SlotName,
	_In_  const std::wstring& NetBios
) {
	ULONGLONG notifyId;
	notifyId = Helpers::SimpleHashNoCase(std::format(L"mailslot|{}|{}", SlotName, NetBios));
	return notifyId;
}

ULONGLONG MessageFk::CalcUdpSocketNotifyId(
	_In_  const std::wstring& Addr,
	_In_  const USHORT& Port
) {
	ULONGLONG notifyId;
	notifyId = Helpers::SimpleHashNoCase(std::format(L"udpsocket|{}|{}", Addr, Port));
	return notifyId;
}


bool MessageFk::AddMailslotNotifyMessage(
	_In_  const std::wstring& SlotName,
	_In_  const std::wstring& NetBios,
	_Out_ ObEngineRef::AddMailslotNotify** lpMessage,
	_Out_ DWORD& dwMessageSize,
	_Out_ ULONGLONG& ullNotifyId
) {
	*lpMessage = nullptr;
	dwMessageSize = NULL;
	ullNotifyId = NULL;

	size_t SlotNameTotalBytes = (SlotName.size() * sizeof(wchar_t)) + sizeof(wchar_t);
	size_t NetBiosTotalBytes = (NetBios.size() * sizeof(wchar_t)) + sizeof(wchar_t);

	size_t MsgSize = SlotNameTotalBytes + NetBiosTotalBytes + sizeof(ObEngineRef::AddMailslotNotify);

	// Generate unique ID
	ullNotifyId = MessageFk::CalcMailslotNotifyId(SlotName, NetBios);

	if (::SIZETToDWord(MsgSize, &dwMessageSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][AddMailslotNotifyMessage] Converting size_t msg size to DWORD");
		return false;
	}

	USHORT usMsgSize;
	if (::SIZETToUShort(MsgSize, &usMsgSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][AddMailslotNotifyMessage] Converting size_t msg size to USHORT");
		return false;
	}

	USHORT usMsgHdrSize;
	if (::SIZETToUShort(sizeof(ObEngineRef::MessageHeader), &usMsgHdrSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][AddMailslotNotifyMessage] Converting size_t ObEngineRef::MessageHeader to USHORT");
		return false;
	}

	USHORT usSlotNameTotalSize;
	if (::SIZETToUShort(SlotNameTotalBytes, &usSlotNameTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][AddMailslotNotifyMessage] Converting size_t slot name to USHORT");
		return false;
	}

	USHORT usNetBiosTotalSize;
	if (::SIZETToUShort(NetBiosTotalBytes, &usNetBiosTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][AddMailslotNotifyMessage] Converting size_t netbios to USHORT");
		return false;
	}

	ObEngineRef::AddMailslotNotify* lpTask;
	lpTask = (ObEngineRef::AddMailslotNotify*)malloc(MsgSize);

	if (lpTask == NULL) {
		Helpers::DbgPrint(L"[Error][AddMailslotNotifyMessage] Failed to allocate message buffer.");
		return false;
	}

	lpTask->Type = ObEngineRef::MessageType::AddMailslotNotify;
	lpTask->Size = usMsgSize - usMsgHdrSize;

	lpTask->SlotNameSize = usSlotNameTotalSize;
	lpTask->NetBiosSize  = usNetBiosTotalSize;

	lpTask->SlotNameOffset = sizeof(ObEngineRef::AddMailslotNotify);
	lpTask->NetBiosOffset  = lpTask->SlotNameOffset + usSlotNameTotalSize;

	lpTask->NotifyId = ullNotifyId;

	::memcpy((UCHAR*)lpTask + lpTask->SlotNameOffset, SlotName.c_str(), SlotNameTotalBytes);
	::memcpy((UCHAR*)lpTask + lpTask->NetBiosOffset,  NetBios.c_str(),  NetBiosTotalBytes);

	*lpMessage = lpTask;

	return true;
}

bool MessageFk::RemoveMailslotNotifyMessage(
	_In_  const ULONGLONG& ullNotifyId,
	_Out_ ObEngineRef::RemoveMailslotNotify** lpMessage,
	_Out_ DWORD& dwMessageSize
) {
	*lpMessage = nullptr;
	dwMessageSize = NULL;

	size_t MsgSize = sizeof(ObEngineRef::RemoveMailslotNotify);

	if (::SIZETToDWord(MsgSize, &dwMessageSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][RemoveMailslotNotifyMessage] Converting size_t msg size to DWORD");
		return false;
	}

	USHORT usMsgSize;
	if (::SIZETToUShort(MsgSize, &usMsgSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][RemoveMailslotNotifyMessage] Converting size_t msg size to USHORT");
		return false;
	}

	USHORT usMsgHdrSize;
	if (::SIZETToUShort(sizeof(ObEngineRef::MessageHeader), &usMsgHdrSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][RemoveMailslotNotifyMessage] Converting size_t ObEngineRef::MessageHeader to USHORT");
		return false;
	}

	ObEngineRef::RemoveMailslotNotify* lpTask;
	lpTask = (ObEngineRef::RemoveMailslotNotify*)malloc(MsgSize);

	if (lpTask == NULL) {
		Helpers::DbgPrint(L"[Error][RemoveMailslotNotifyMessage] Failed to allocate message buffer.");
		return false;
	}

	lpTask->Type = ObEngineRef::MessageType::RemoveMailslotNotify;
	lpTask->Size = usMsgSize - usMsgHdrSize;

	lpTask->NotifyId = ullNotifyId;

	*lpMessage = lpTask;

	return true;
}

bool MessageFk::MailslotNotifyMessage(
	_In_  const ULONGLONG& ullNotifyId,
	_In_  const std::wstring& Notification,
	_Out_ ObEngineRef::NotifyData** lpMessage,
	_Out_ DWORD& dwMessageSize
) {
	*lpMessage = nullptr;
	dwMessageSize = NULL;

	size_t NotificationTotalBytes = (Notification.size() * sizeof(wchar_t)) + sizeof(wchar_t);

	size_t MsgSize = NotificationTotalBytes + sizeof(ObEngineRef::NotifyData);

	if (::SIZETToDWord(MsgSize, &dwMessageSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][MailslotNotifyMessage] Converting size_t msg size to DWORD");
		return false;
	}

	USHORT usMsgSize;
	if (::SIZETToUShort(MsgSize, &usMsgSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][MailslotNotifyMessage] Converting size_t msg size to USHORT");
		return false;
	}

	USHORT usMsgHdrSize;
	if (::SIZETToUShort(sizeof(ObEngineRef::MessageHeader), &usMsgHdrSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][MailslotNotifyMessage] Converting size_t ObEngineRef::MessageHeader to USHORT");
		return false;
	}

	USHORT usNotificationTotalSize;
	if (::SIZETToUShort(NotificationTotalBytes, &usNotificationTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][MailslotNotifyMessage] Converting size_t notification to USHORT");
		return false;
	}

	ObEngineRef::NotifyData* lpTask;
	lpTask = (ObEngineRef::NotifyData*)malloc(MsgSize);

	if (lpTask == NULL) {
		Helpers::DbgPrint(L"[Error][MailslotNotifyMessage] Failed to allocate message buffer.");
		return false;
	}

	lpTask->Type = ObEngineRef::MessageType::MailslotNotify;
	lpTask->Size = usMsgSize - usMsgHdrSize;

	lpTask->NotificationSize   = usNotificationTotalSize;
	lpTask->NotificationOffset = sizeof(ObEngineRef::NotifyData);

	lpTask->NotifyId = ullNotifyId;
	lpTask->Inbound  = false;

	::memcpy((UCHAR*)lpTask + lpTask->NotificationOffset, Notification.c_str(), NotificationTotalBytes);

	*lpMessage = lpTask;

	return true;
}


bool MessageFk::AddUdpSocketNotifyMessage(
	_In_  const std::wstring& UdpAddr,
	_In_  const USHORT& usUdpPort,
	_Out_ ObEngineRef::AddUdpSocketNotify** lpMessage,
	_Out_ DWORD& dwMessageSize,
	_Out_ ULONGLONG& ullNotifyId
) {
	*lpMessage = nullptr;
	dwMessageSize = NULL;
	ullNotifyId = NULL;

	size_t UdpAddrTotalBytes = (UdpAddr.size() * sizeof(wchar_t)) + sizeof(wchar_t);

	size_t MsgSize = UdpAddrTotalBytes + sizeof(ObEngineRef::AddUdpSocketNotify);

	// Generate unique ID
	ullNotifyId = MessageFk::CalcUdpSocketNotifyId(UdpAddr, usUdpPort);

	if (::SIZETToDWord(MsgSize, &dwMessageSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][AddUdpSocketNotifyMessage] Converting size_t msg size to DWORD");
		return false;
	}

	USHORT usMsgSize;
	if (::SIZETToUShort(MsgSize, &usMsgSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][AddUdpSocketNotifyMessage] Converting size_t msg size to USHORT");
		return false;
	}

	USHORT usMsgHdrSize;
	if (::SIZETToUShort(sizeof(ObEngineRef::MessageHeader), &usMsgHdrSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][AddUdpSocketNotifyMessage] Converting size_t ObEngineRef::MessageHeader to USHORT");
		return false;
	}

	USHORT usUdpAddrTotalSize;
	if (::SIZETToUShort(UdpAddrTotalBytes, &usUdpAddrTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][AddUdpSocketNotifyMessage] Converting size_t UDP addr to USHORT");
		return false;
	}

	ObEngineRef::AddUdpSocketNotify* lpTask;
	lpTask = (ObEngineRef::AddUdpSocketNotify*)malloc(MsgSize);

	if (lpTask == NULL) {
		Helpers::DbgPrint(L"[Error][AddUdpSocketNotifyMessage] Failed to allocate message buffer.");
		return false;
	}

	lpTask->Type = ObEngineRef::MessageType::AddUdpSocketNotify;
	lpTask->Size = usMsgSize - usMsgHdrSize;

	lpTask->UdpAddrSize = usUdpAddrTotalSize;

	lpTask->UdpAddrOffset = sizeof(ObEngineRef::AddUdpSocketNotify);

	lpTask->UdpPort  = usUdpPort;
	lpTask->NotifyId = ullNotifyId;

	::memcpy((UCHAR*)lpTask + lpTask->UdpAddrOffset, UdpAddr.c_str(), UdpAddrTotalBytes);

	*lpMessage = lpTask;

	return true;
}

bool MessageFk::RemoveUdpSocketNotifyMessage(
	_In_  const ULONGLONG& ullNotifyId,
	_Out_ ObEngineRef::RemoveUdpSocketNotify** lpMessage,
	_Out_ DWORD& dwMessageSize
) {
	*lpMessage = nullptr;
	dwMessageSize = NULL;

	size_t MsgSize = sizeof(ObEngineRef::RemoveUdpSocketNotify);

	if (::SIZETToDWord(MsgSize, &dwMessageSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][RemoveUdpSocketNotifyMessage] Converting size_t msg size to DWORD");
		return false;
	}

	USHORT usMsgSize;
	if (::SIZETToUShort(MsgSize, &usMsgSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][RemoveUdpSocketNotifyMessage] Converting size_t msg size to USHORT");
		return false;
	}

	USHORT usMsgHdrSize;
	if (::SIZETToUShort(sizeof(ObEngineRef::MessageHeader), &usMsgHdrSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][RemoveUdpSocketNotifyMessage] Converting size_t ObEngineRef::MessageHeader to USHORT");
		return false;
	}

	ObEngineRef::RemoveUdpSocketNotify* lpTask;
	lpTask = (ObEngineRef::RemoveUdpSocketNotify*)malloc(MsgSize);

	if (lpTask == NULL) {
		Helpers::DbgPrint(L"[Error][RemoveUdpSocketNotifyMessage] Failed to allocate message buffer.");
		return false;
	}

	lpTask->Type = ObEngineRef::MessageType::RemoveUdpSocketNotify;
	lpTask->Size = usMsgSize - usMsgHdrSize;

	lpTask->NotifyId = ullNotifyId;

	*lpMessage = lpTask;

	return true;
}

bool MessageFk::UdpSocketNotifyMessage(
	_In_  const ULONGLONG& ullNotifyId,
	_In_  const std::wstring& Notification,
	_Out_ ObEngineRef::NotifyData** lpMessage,
	_Out_ DWORD& dwMessageSize
) {
	*lpMessage = nullptr;
	dwMessageSize = NULL;

	size_t NotificationTotalBytes = (Notification.size() * sizeof(wchar_t)) + sizeof(wchar_t);

	size_t MsgSize = NotificationTotalBytes + sizeof(ObEngineRef::NotifyData);

	if (::SIZETToDWord(MsgSize, &dwMessageSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][UdpSocketNotifyMessage] Converting size_t msg size to DWORD");
		return false;
	}

	USHORT usMsgSize;
	if (::SIZETToUShort(MsgSize, &usMsgSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][UdpSocketNotifyMessage] Converting size_t msg size to USHORT");
		return false;
	}

	USHORT usMsgHdrSize;
	if (::SIZETToUShort(sizeof(ObEngineRef::MessageHeader), &usMsgHdrSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][UdpSocketNotifyMessage] Converting size_t ObEngineRef::MessageHeader to USHORT");
		return false;
	}

	USHORT usNotificationTotalSize;
	if (::SIZETToUShort(NotificationTotalBytes, &usNotificationTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][UdpSocketNotifyMessage] Converting size_t notification to USHORT");
		return false;
	}

	ObEngineRef::NotifyData* lpTask;
	lpTask = (ObEngineRef::NotifyData*)malloc(MsgSize);

	if (lpTask == NULL) {
		Helpers::DbgPrint(L"[Error][UdpSocketNotifyMessage] Failed to allocate message buffer.");
		return false;
	}

	lpTask->Type = ObEngineRef::MessageType::UdpSocketNotify;
	lpTask->Size = usMsgSize - usMsgHdrSize;

	lpTask->NotificationSize = usNotificationTotalSize;
	lpTask->NotificationOffset = sizeof(ObEngineRef::NotifyData);

	lpTask->NotifyId = ullNotifyId;
	lpTask->Inbound = false;

	::memcpy((UCHAR*)lpTask + lpTask->NotificationOffset, Notification.c_str(), NotificationTotalBytes);

	*lpMessage = lpTask;

	return true;
}


bool MessageFk::CreateUserMessage(
	_In_  const std::wstring& Domain,
	_In_  const std::wstring& Username,
	_In_  const std::wstring& Password,
	_Out_ ObEngineRef::CreateUserTask** lpMessage,
	_Out_ DWORD& dwMessageSize
) {
	*lpMessage = nullptr;
	dwMessageSize = NULL;

	size_t DomainTotalBytes = (Domain.size()   * sizeof(wchar_t)) + sizeof(wchar_t);
	size_t UserTotalBytes   = (Username.size() * sizeof(wchar_t)) + sizeof(wchar_t);
	size_t PassTotalBytes   = (Password.size() * sizeof(wchar_t)) + sizeof(wchar_t);

	size_t MsgSize = DomainTotalBytes + UserTotalBytes + PassTotalBytes + sizeof(ObEngineRef::CreateUserTask);

	if (::SIZETToDWord(MsgSize, &dwMessageSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][CreateUserMessage] Converting size_t msg size to DWORD");
		return false;
	}

	USHORT usMsgSize;
	if (::SIZETToUShort(MsgSize, &usMsgSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][CreateUserMessage] Converting size_t msg size to USHORT");
		return false;
	}

	USHORT usMsgHdrSize;
	if (::SIZETToUShort(sizeof(ObEngineRef::MessageHeader), &usMsgHdrSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][CreateUserMessage] Converting size_t ObEngineRef::MessageHeader to USHORT");
		return false;
	}

	USHORT usDomainTotalSize;
	if (::SIZETToUShort(DomainTotalBytes, &usDomainTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][CreateUserMessage] Converting size_t domain to USHORT");
		return false;
	}

	USHORT usUserTotalSize;
	if (::SIZETToUShort(UserTotalBytes, &usUserTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][CreateUserMessage] Converting size_t username to USHORT");
		return false;
	}

	USHORT usPassTotalSize;
	if (::SIZETToUShort(PassTotalBytes, &usPassTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][CreateUserMessage] Converting size_t password to USHORT");
		return false;
	}

	ObEngineRef::CreateUserTask* lpTask;
	lpTask = (ObEngineRef::CreateUserTask*)malloc(MsgSize);

	if (lpTask == NULL) {
		Helpers::DbgPrint(L"[Error][CreateUserMessage] Failed to allocate message buffer.");
		return false;
	}

	lpTask->Type = ObEngineRef::MessageType::CreateUser;
	lpTask->Size = usMsgSize - usMsgHdrSize;

	lpTask->NotifyId   = 0; // Defaults to not send a notification
	lpTask->DomainSize = usDomainTotalSize;
	lpTask->UserSize   = usUserTotalSize;
	lpTask->PassSize   = usPassTotalSize;

	lpTask->DomainOffset = sizeof(ObEngineRef::CreateUserTask);
	lpTask->UserOffset   = lpTask->DomainOffset + usDomainTotalSize;
	lpTask->PassOffset   = lpTask->UserOffset   + usUserTotalSize;

	::memcpy((UCHAR*)lpTask + lpTask->DomainOffset, Domain.c_str(),   DomainTotalBytes);
	::memcpy((UCHAR*)lpTask + lpTask->UserOffset,   Username.c_str(), UserTotalBytes);
	::memcpy((UCHAR*)lpTask + lpTask->PassOffset,   Password.c_str(), PassTotalBytes);

	*lpMessage = lpTask;

	return true;
}

bool MessageFk::DeleteUserMessage(
	_In_  const std::wstring& Domain,
	_In_  const std::wstring& Username,
	_Out_ ObEngineRef::DeleteUserTask** lpMessage,
	_Out_ DWORD& dwMessageSize
) {
	*lpMessage = nullptr;
	dwMessageSize = NULL;

	size_t DomainTotalBytes = (Domain.size() * sizeof(wchar_t)) + sizeof(wchar_t);
	size_t UserTotalBytes = (Username.size() * sizeof(wchar_t)) + sizeof(wchar_t);

	size_t MsgSize = DomainTotalBytes + UserTotalBytes + sizeof(ObEngineRef::DeleteUserTask);

	if (::SIZETToDWord(MsgSize, &dwMessageSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][DeleteUserMessage] Converting size_t msg size to DWORD");
		return false;
	}

	USHORT usMsgSize;
	if (::SIZETToUShort(MsgSize, &usMsgSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][DeleteUserMessage] Converting size_t msg size to USHORT");
		return false;
	}

	USHORT usMsgHdrSize;
	if (::SIZETToUShort(sizeof(ObEngineRef::MessageHeader), &usMsgHdrSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][DeleteUserMessage] Converting size_t ObEngineRef::MessageHeader to USHORT");
		return false;
	}

	USHORT usDomainTotalSize;
	if (::SIZETToUShort(DomainTotalBytes, &usDomainTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][DeleteUserMessage] Converting size_t domain to USHORT");
		return false;
	}

	USHORT usUserTotalSize;
	if (::SIZETToUShort(UserTotalBytes, &usUserTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][DeleteUserMessage] Converting size_t username to USHORT");
		return false;
	}

	ObEngineRef::DeleteUserTask* lpTask;
	lpTask = (ObEngineRef::DeleteUserTask*)malloc(MsgSize);

	if (lpTask == NULL) {
		Helpers::DbgPrint(L"[Error][DeleteUserMessage] Failed to allocate message buffer.");
		return false;
	}

	lpTask->Type = ObEngineRef::MessageType::DeleteUser;
	lpTask->Size = usMsgSize - usMsgHdrSize;

	lpTask->NotifyId   = 0; // Defaults to not send a notification
	lpTask->DomainSize = usDomainTotalSize;
	lpTask->UserSize   = usUserTotalSize;

	lpTask->DomainOffset = sizeof(ObEngineRef::DeleteUserTask);
	lpTask->UserOffset   = lpTask->DomainOffset + usDomainTotalSize;

	::memcpy((UCHAR*)lpTask + lpTask->DomainOffset, Domain.c_str(), DomainTotalBytes);
	::memcpy((UCHAR*)lpTask + lpTask->UserOffset, Username.c_str(), UserTotalBytes);

	*lpMessage = lpTask;

	return true;
}

bool MessageFk::CreateGroupMessage(
	_In_  const std::wstring& Domain,
	_In_  const std::wstring& Group,
	_In_  const std::wstring& Comment,
	_Out_ ObEngineRef::CreateGroupTask** lpMessage,
	_Out_ DWORD& dwMessageSize
) {
	*lpMessage = nullptr;
	dwMessageSize = NULL;

	size_t DomainTotalBytes  = (Domain.size() * sizeof(wchar_t)) + sizeof(wchar_t);
	size_t GroupTotalBytes   = (Group.size() * sizeof(wchar_t)) + sizeof(wchar_t);
	size_t CommentTotalBytes = (Comment.size() * sizeof(wchar_t)) + sizeof(wchar_t);

	size_t MsgSize = DomainTotalBytes + GroupTotalBytes + CommentTotalBytes + sizeof(ObEngineRef::CreateGroupTask);

	if (::SIZETToDWord(MsgSize, &dwMessageSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][CreateGroupMessage] Converting size_t msg size to DWORD");
		return false;
	}

	USHORT usMsgSize;
	if (::SIZETToUShort(MsgSize, &usMsgSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][CreateGroupMessage] Converting size_t msg size to USHORT");
		return false;
	}

	USHORT usMsgHdrSize;
	if (::SIZETToUShort(sizeof(ObEngineRef::MessageHeader), &usMsgHdrSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][CreateGroupMessage] Converting size_t ObEngineRef::MessageHeader to USHORT");
		return false;
	}

	USHORT usDomainTotalSize;
	if (::SIZETToUShort(DomainTotalBytes, &usDomainTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][CreateGroupMessage] Converting size_t domain to USHORT");
		return false;
	}

	USHORT usGroupTotalSize;
	if (::SIZETToUShort(GroupTotalBytes, &usGroupTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][CreateGroupMessage] Converting size_t group to USHORT");
		return false;
	}

	USHORT usCommentTotalSize;
	if (::SIZETToUShort(CommentTotalBytes, &usCommentTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][CreateGroupMessage] Converting size_t comment to USHORT");
		return false;
	}

	ObEngineRef::CreateGroupTask* lpTask;
	lpTask = (ObEngineRef::CreateGroupTask*)malloc(MsgSize);

	if (lpTask == NULL) {
		Helpers::DbgPrint(L"[Error][CreateGroupMessage] Failed to allocate message buffer.");
		return false;
	}

	lpTask->Type = ObEngineRef::MessageType::CreateGroup;
	lpTask->Size = usMsgSize - usMsgHdrSize;

	lpTask->NotifyId    = 0; // Defaults to not send a notification
	lpTask->DomainSize  = usDomainTotalSize;
	lpTask->GroupSize   = usGroupTotalSize;
	lpTask->CommentSize = usCommentTotalSize;

	lpTask->DomainOffset  = sizeof(ObEngineRef::CreateGroupTask);
	lpTask->GroupOffset   = lpTask->DomainOffset + usDomainTotalSize;
	lpTask->CommentOffset = lpTask->GroupOffset + usGroupTotalSize;

	::memcpy((UCHAR*)lpTask + lpTask->DomainOffset,  Domain.c_str(),  DomainTotalBytes);
	::memcpy((UCHAR*)lpTask + lpTask->GroupOffset,   Group.c_str(),   GroupTotalBytes);
	::memcpy((UCHAR*)lpTask + lpTask->CommentOffset, Comment.c_str(), CommentTotalBytes);

	*lpMessage = lpTask;

	return true;
}

bool MessageFk::DeleteGroupMessage(
	_In_  const std::wstring& Domain,
	_In_  const std::wstring& Group,
	_Out_ ObEngineRef::DeleteGroupTask** lpMessage,
	_Out_ DWORD& dwMessageSize
) {
	*lpMessage = nullptr;
	dwMessageSize = NULL;

	size_t DomainTotalBytes = (Domain.size() * sizeof(wchar_t)) + sizeof(wchar_t);
	size_t GroupTotalBytes  = (Group.size()  * sizeof(wchar_t)) + sizeof(wchar_t);

	size_t MsgSize = DomainTotalBytes + GroupTotalBytes + sizeof(ObEngineRef::DeleteGroupTask);

	if (::SIZETToDWord(MsgSize, &dwMessageSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][DeleteGroupMessage] Converting size_t msg size to DWORD");
		return false;
	}

	USHORT usMsgSize;
	if (::SIZETToUShort(MsgSize, &usMsgSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][DeleteGroupMessage] Converting size_t msg size to USHORT");
		return false;
	}

	USHORT usMsgHdrSize;
	if (::SIZETToUShort(sizeof(ObEngineRef::MessageHeader), &usMsgHdrSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][DeleteGroupMessage] Converting size_t ObEngineRef::MessageHeader to USHORT");
		return false;
	}

	USHORT usDomainTotalSize;
	if (::SIZETToUShort(DomainTotalBytes, &usDomainTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][DeleteGroupMessage] Converting size_t domain to USHORT");
		return false;
	}

	USHORT usGroupTotalSize;
	if (::SIZETToUShort(GroupTotalBytes, &usGroupTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][DeleteGroupMessage] Converting size_t group to USHORT");
		return false;
	}

	ObEngineRef::DeleteGroupTask* lpTask;
	lpTask = (ObEngineRef::DeleteGroupTask*)malloc(MsgSize);

	if (lpTask == NULL) {
		Helpers::DbgPrint(L"[Error][DeleteGroupMessage] Failed to allocate message buffer.");
		return false;
	}

	lpTask->Type = ObEngineRef::MessageType::DeleteGroup;
	lpTask->Size = usMsgSize - usMsgHdrSize;

	lpTask->NotifyId = 0; // Defaults to not send a notification
	lpTask->DomainSize = usDomainTotalSize;
	lpTask->GroupSize = usGroupTotalSize;

	lpTask->DomainOffset = sizeof(ObEngineRef::DeleteGroupTask);
	lpTask->GroupOffset = lpTask->DomainOffset + usDomainTotalSize;

	::memcpy((UCHAR*)lpTask + lpTask->DomainOffset, Domain.c_str(), DomainTotalBytes);
	::memcpy((UCHAR*)lpTask + lpTask->GroupOffset,  Group.c_str(),  GroupTotalBytes);

	*lpMessage = lpTask;

	return true;
}

bool MessageFk::AddGroupMemberMessage(
	_In_  const std::wstring& Domain,
	_In_  const std::wstring& Group,
	_In_  const std::wstring& Member,
	_Out_ ObEngineRef::AddGroupMemberTask** lpMessage,
	_Out_ DWORD& dwMessageSize
) {
	*lpMessage = nullptr;
	dwMessageSize = NULL;

	size_t DomainTotalBytes = (Domain.size() * sizeof(wchar_t)) + sizeof(wchar_t);
	size_t GroupTotalBytes  = (Group.size() * sizeof(wchar_t)) + sizeof(wchar_t);
	size_t MemberTotalBytes = (Member.size() * sizeof(wchar_t)) + sizeof(wchar_t);

	size_t MsgSize = DomainTotalBytes + GroupTotalBytes + MemberTotalBytes + sizeof(ObEngineRef::AddGroupMemberTask);

	if (::SIZETToDWord(MsgSize, &dwMessageSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][AddGroupMemberMessage] Converting size_t msg size to DWORD");
		return false;
	}

	USHORT usMsgSize;
	if (::SIZETToUShort(MsgSize, &usMsgSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][AddGroupMemberMessage] Converting size_t msg size to USHORT");
		return false;
	}

	USHORT usMsgHdrSize;
	if (::SIZETToUShort(sizeof(ObEngineRef::MessageHeader), &usMsgHdrSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][AddGroupMemberMessage] Converting size_t ObEngineRef::MessageHeader to USHORT");
		return false;
	}

	USHORT usDomainTotalSize;
	if (::SIZETToUShort(DomainTotalBytes, &usDomainTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][AddGroupMemberMessage] Converting size_t domain to USHORT");
		return false;
	}

	USHORT usGroupTotalSize;
	if (::SIZETToUShort(GroupTotalBytes, &usGroupTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][AddGroupMemberMessage] Converting size_t group to USHORT");
		return false;
	}

	USHORT usMemberTotalSize;
	if (::SIZETToUShort(MemberTotalBytes, &usMemberTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][AddGroupMemberMessage] Converting size_t member to USHORT");
		return false;
	}

	ObEngineRef::AddGroupMemberTask* lpTask;
	lpTask = (ObEngineRef::AddGroupMemberTask*)malloc(MsgSize);

	if (lpTask == NULL) {
		Helpers::DbgPrint(L"[Error][AddGroupMemberMessage] Failed to allocate message buffer.");
		return false;
	}

	lpTask->Type = ObEngineRef::MessageType::AddGroupMember;
	lpTask->Size = usMsgSize - usMsgHdrSize;

	lpTask->NotifyId   = 0; // Defaults to not send a notification
	lpTask->DomainSize = usDomainTotalSize;
	lpTask->GroupSize  = usGroupTotalSize;
	lpTask->MemberSize = usMemberTotalSize;

	lpTask->DomainOffset = sizeof(ObEngineRef::AddGroupMemberTask);
	lpTask->GroupOffset  = lpTask->DomainOffset + usDomainTotalSize;
	lpTask->MemberOffset = lpTask->GroupOffset + usGroupTotalSize;

	::memcpy((UCHAR*)lpTask + lpTask->DomainOffset, Domain.c_str(), DomainTotalBytes);
	::memcpy((UCHAR*)lpTask + lpTask->GroupOffset,  Group.c_str(),  GroupTotalBytes);
	::memcpy((UCHAR*)lpTask + lpTask->MemberOffset, Member.c_str(), MemberTotalBytes);

	*lpMessage = lpTask;

	return true;
}

bool MessageFk::RemoveGroupMemberMessage(
	_In_  const std::wstring& Domain,
	_In_  const std::wstring& Group,
	_In_  const std::wstring& Member,
	_Out_ ObEngineRef::RemoveGroupMemberTask** lpMessage,
	_Out_ DWORD& dwMessageSize
) {
	*lpMessage = nullptr;
	dwMessageSize = NULL;

	size_t DomainTotalBytes = (Domain.size() * sizeof(wchar_t)) + sizeof(wchar_t);
	size_t GroupTotalBytes = (Group.size() * sizeof(wchar_t)) + sizeof(wchar_t);
	size_t MemberTotalBytes = (Member.size() * sizeof(wchar_t)) + sizeof(wchar_t);

	size_t MsgSize = DomainTotalBytes + GroupTotalBytes + MemberTotalBytes + sizeof(ObEngineRef::RemoveGroupMemberTask);

	if (::SIZETToDWord(MsgSize, &dwMessageSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][RemoveGroupMemberMessage] Converting size_t msg size to DWORD");
		return false;
	}

	USHORT usMsgSize;
	if (::SIZETToUShort(MsgSize, &usMsgSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][RemoveGroupMemberMessage] Converting size_t msg size to USHORT");
		return false;
	}

	USHORT usMsgHdrSize;
	if (::SIZETToUShort(sizeof(ObEngineRef::MessageHeader), &usMsgHdrSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][RemoveGroupMemberMessage] Converting size_t ObEngineRef::MessageHeader to USHORT");
		return false;
	}

	USHORT usDomainTotalSize;
	if (::SIZETToUShort(DomainTotalBytes, &usDomainTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][RemoveGroupMemberMessage] Converting size_t domain to USHORT");
		return false;
	}

	USHORT usGroupTotalSize;
	if (::SIZETToUShort(GroupTotalBytes, &usGroupTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][RemoveGroupMemberMessage] Converting size_t group to USHORT");
		return false;
	}

	USHORT usMemberTotalSize;
	if (::SIZETToUShort(MemberTotalBytes, &usMemberTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][RemoveGroupMemberMessage] Converting size_t member to USHORT");
		return false;
	}

	ObEngineRef::RemoveGroupMemberTask* lpTask;
	lpTask = (ObEngineRef::RemoveGroupMemberTask*)malloc(MsgSize);

	if (lpTask == NULL) {
		Helpers::DbgPrint(L"[Error][RemoveGroupMemberMessage] Failed to allocate message buffer.");
		return false;
	}

	lpTask->Type = ObEngineRef::MessageType::RemoveGroupMember;
	lpTask->Size = usMsgSize - usMsgHdrSize;

	lpTask->NotifyId = 0; // Defaults to not send a notification
	lpTask->DomainSize = usDomainTotalSize;
	lpTask->GroupSize = usGroupTotalSize;
	lpTask->MemberSize = usMemberTotalSize;

	lpTask->DomainOffset = sizeof(ObEngineRef::RemoveGroupMemberTask);
	lpTask->GroupOffset = lpTask->DomainOffset + usDomainTotalSize;
	lpTask->MemberOffset = lpTask->GroupOffset + usGroupTotalSize;

	::memcpy((UCHAR*)lpTask + lpTask->DomainOffset, Domain.c_str(), DomainTotalBytes);
	::memcpy((UCHAR*)lpTask + lpTask->GroupOffset,  Group.c_str(),  GroupTotalBytes);
	::memcpy((UCHAR*)lpTask + lpTask->MemberOffset, Member.c_str(), MemberTotalBytes);

	*lpMessage = lpTask;

	return true;
}

bool MessageFk::AddPrivilegeMessage(
	_In_  const std::wstring& Entity,
	_In_  const std::wstring& Privilege,
	_Out_ ObEngineRef::AddPrivilegeTask** lpMessage,
	_Out_ DWORD& dwMessageSize
) {
	*lpMessage = nullptr;
	dwMessageSize = NULL;

	size_t EntityTotalBytes = (Entity.size()    * sizeof(wchar_t)) + sizeof(wchar_t);
	size_t PrivTotalBytes   = (Privilege.size() * sizeof(wchar_t)) + sizeof(wchar_t);

	size_t MsgSize = EntityTotalBytes + PrivTotalBytes + sizeof(ObEngineRef::AddPrivilegeTask);

	if (::SIZETToDWord(MsgSize, &dwMessageSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][AddPrivilegeMessage] Converting size_t msg size to DWORD");
		return false;
	}

	USHORT usMsgSize;
	if (::SIZETToUShort(MsgSize, &usMsgSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][AddPrivilegeMessage] Converting size_t msg size to USHORT");
		return false;
	}

	USHORT usMsgHdrSize;
	if (::SIZETToUShort(sizeof(ObEngineRef::MessageHeader), &usMsgHdrSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][AddPrivilegeMessage] Converting size_t ObEngineRef::MessageHeader to USHORT");
		return false;
	}

	USHORT usEntityTotalSize;
	if (::SIZETToUShort(EntityTotalBytes, &usEntityTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][AddPrivilegeMessage] Converting size_t entity to USHORT");
		return false;
	}

	USHORT usPrivTotalSize;
	if (::SIZETToUShort(PrivTotalBytes, &usPrivTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][AddPrivilegeMessage] Converting size_t privilege to USHORT");
		return false;
	}

	ObEngineRef::AddPrivilegeTask* lpTask;
	lpTask = (ObEngineRef::AddPrivilegeTask*)malloc(MsgSize);

	if (lpTask == NULL) {
		Helpers::DbgPrint(L"[Error][AddPrivilegeMessage] Failed to allocate message buffer.");
		return false;
	}

	lpTask->Type = ObEngineRef::MessageType::AddPrivilege;
	lpTask->Size = usMsgSize - usMsgHdrSize;

	lpTask->NotifyId   = 0; // Defaults to not send a notification
	lpTask->EntitySize = usEntityTotalSize;
	lpTask->PrivSize   = usPrivTotalSize;

	lpTask->EntityOffset = sizeof(ObEngineRef::AddPrivilegeTask);
	lpTask->PrivOffset   = lpTask->EntityOffset + usEntityTotalSize;

	::memcpy((UCHAR*)lpTask + lpTask->EntityOffset, Entity.c_str(),    EntityTotalBytes);
	::memcpy((UCHAR*)lpTask + lpTask->PrivOffset,   Privilege.c_str(), PrivTotalBytes);

	*lpMessage = lpTask;

	return true;
}

bool MessageFk::RemovePrivilegeMessage(
	_In_  const std::wstring& Entity,
	_In_  const std::wstring& Privilege,
	_Out_ ObEngineRef::RemovePrivilegeTask** lpMessage,
	_Out_ DWORD& dwMessageSize
) {
	*lpMessage = nullptr;
	dwMessageSize = NULL;

	size_t EntityTotalBytes = (Entity.size()    * sizeof(wchar_t)) + sizeof(wchar_t);
	size_t PrivTotalBytes   = (Privilege.size() * sizeof(wchar_t)) + sizeof(wchar_t);

	size_t MsgSize = EntityTotalBytes + PrivTotalBytes + sizeof(ObEngineRef::RemovePrivilegeTask);

	if (::SIZETToDWord(MsgSize, &dwMessageSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][RemovePrivilegeMessage] Converting size_t msg size to DWORD");
		return false;
	}

	USHORT usMsgSize;
	if (::SIZETToUShort(MsgSize, &usMsgSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][RemovePrivilegeMessage] Converting size_t msg size to USHORT");
		return false;
	}

	USHORT usMsgHdrSize;
	if (::SIZETToUShort(sizeof(ObEngineRef::MessageHeader), &usMsgHdrSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][RemovePrivilegeMessage] Converting size_t ObEngineRef::MessageHeader to USHORT");
		return false;
	}

	USHORT usEntityTotalSize;
	if (::SIZETToUShort(EntityTotalBytes, &usEntityTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][RemovePrivilegeMessage] Converting size_t entity to USHORT");
		return false;
	}

	USHORT usPrivTotalSize;
	if (::SIZETToUShort(PrivTotalBytes, &usPrivTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][RemovePrivilegeMessage] Converting size_t privilege to USHORT");
		return false;
	}

	ObEngineRef::RemovePrivilegeTask* lpTask;
	lpTask = (ObEngineRef::RemovePrivilegeTask*)malloc(MsgSize);

	if (lpTask == NULL) {
		Helpers::DbgPrint(L"[Error][RemovePrivilegeMessage] Failed to allocate message buffer.");
		return false;
	}

	lpTask->Type = ObEngineRef::MessageType::RemovePrivilege;
	lpTask->Size = usMsgSize - usMsgHdrSize;

	lpTask->NotifyId   = 0; // Defaults to not send a notification
	lpTask->EntitySize = usEntityTotalSize;
	lpTask->PrivSize   = usPrivTotalSize;

	lpTask->EntityOffset = sizeof(ObEngineRef::RemovePrivilegeTask);
	lpTask->PrivOffset   = lpTask->EntityOffset + usEntityTotalSize;

	::memcpy((UCHAR*)lpTask + lpTask->EntityOffset, Entity.c_str(),    EntityTotalBytes);
	::memcpy((UCHAR*)lpTask + lpTask->PrivOffset,   Privilege.c_str(), PrivTotalBytes);

	*lpMessage = lpTask;

	return true;
}

bool MessageFk::SpawnProcessMessage(
	_In_  const std::wstring& App,
	_In_  const std::wstring& Args,
	_In_  const std::wstring& Cwd,
	_In_  const std::wstring& Domain,
	_In_  const std::wstring& Username,
	_Out_ ObEngineRef::SpawnProcessTask** lpMessage,
	_Out_ DWORD& dwMessageSize
) {
	*lpMessage = nullptr;
	dwMessageSize = NULL;

	size_t AppTotalBytes    = (App.size()      * sizeof(wchar_t)) + sizeof(wchar_t);
	size_t ArgsTotalBytes   = (Args.size()     * sizeof(wchar_t)) + sizeof(wchar_t);
	size_t CwdTotalBytes    = (Cwd.size()      * sizeof(wchar_t)) + sizeof(wchar_t);
	size_t DomainTotalBytes = (Domain.size()   * sizeof(wchar_t)) + sizeof(wchar_t);
	size_t UserTotalBytes   = (Username.size() * sizeof(wchar_t)) + sizeof(wchar_t);

	size_t MsgSize = AppTotalBytes + ArgsTotalBytes + CwdTotalBytes + DomainTotalBytes
		+ UserTotalBytes + sizeof(ObEngineRef::SpawnProcessTask);

	if (::SIZETToDWord(MsgSize, &dwMessageSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][SpawnProcessMessage] Converting size_t msg size to DWORD");
		return false;
	}

	USHORT usMsgSize;
	if (::SIZETToUShort(MsgSize, &usMsgSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][SpawnProcessMessage] Converting size_t msg size to USHORT");
		return false;
	}

	USHORT usMsgHdrSize;
	if (::SIZETToUShort(sizeof(ObEngineRef::MessageHeader), &usMsgHdrSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][SpawnProcessMessage] Converting size_t ObEngineRef::MessageHeader to USHORT");
		return false;
	}

	USHORT usAppTotalSize;
	if (::SIZETToUShort(AppTotalBytes, &usAppTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][SpawnProcessMessage] Converting size_t app to USHORT");
		return false;
	}

	USHORT usArgsTotalSize;
	if (::SIZETToUShort(ArgsTotalBytes, &usArgsTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][SpawnProcessMessage] Converting size_t args to USHORT");
		return false;
	}

	USHORT usCwdTotalSize;
	if (::SIZETToUShort(CwdTotalBytes, &usCwdTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][SpawnProcessMessage] Converting size_t cwd to USHORT");
		return false;
	}

	USHORT usDomainTotalSize;
	if (::SIZETToUShort(DomainTotalBytes, &usDomainTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][SpawnProcessMessage] Converting size_t domain to USHORT");
		return false;
	}

	USHORT usUserTotalSize;
	if (::SIZETToUShort(UserTotalBytes, &usUserTotalSize) != S_OK) {
		Helpers::DbgPrint(L"[Error][SpawnProcessMessage] Converting size_t username to USHORT");
		return false;
	}

	ObEngineRef::SpawnProcessTask* lpTask;
	lpTask = (ObEngineRef::SpawnProcessTask*)malloc(MsgSize);

	if (lpTask == NULL) {
		Helpers::DbgPrint(L"[Error][SpawnProcessMessage] Failed to allocate message buffer.");
		return false;
	}

	lpTask->Type = ObEngineRef::MessageType::SpawnProcess;
	lpTask->Size = usMsgSize - usMsgHdrSize;

	lpTask->NotifyId   = 0; // Defaults to not send a notification
	lpTask->AppSize    = usAppTotalSize;
	lpTask->ArgsSize   = usArgsTotalSize;
	lpTask->CwdSize    = usCwdTotalSize;
	lpTask->DomainSize = usDomainTotalSize;
	lpTask->UserSize   = usUserTotalSize;

	lpTask->AppOffset    = sizeof(ObEngineRef::SpawnProcessTask);
	lpTask->ArgsOffset   = lpTask->AppOffset    + usAppTotalSize;
	lpTask->CwdOffset    = lpTask->ArgsOffset   + usArgsTotalSize;
	lpTask->DomainOffset = lpTask->CwdOffset    + usCwdTotalSize;
	lpTask->UserOffset   = lpTask->DomainOffset + usDomainTotalSize;

	::memcpy((UCHAR*)lpTask + lpTask->AppOffset,    App.c_str(),      AppTotalBytes);
	::memcpy((UCHAR*)lpTask + lpTask->ArgsOffset,   Args.c_str(),     ArgsTotalBytes);
	::memcpy((UCHAR*)lpTask + lpTask->CwdOffset,    Cwd.c_str(),      CwdTotalBytes);
	::memcpy((UCHAR*)lpTask + lpTask->DomainOffset, Domain.c_str(),   DomainTotalBytes);
	::memcpy((UCHAR*)lpTask + lpTask->UserOffset,   Username.c_str(), UserTotalBytes);

	*lpMessage = lpTask;

	return true;
}