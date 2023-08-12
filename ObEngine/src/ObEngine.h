#pragma once

#include <unordered_map>

namespace ObEngineRef {
	/*
		Messages
	*/
	const DWORD MAX_MAILSLOT_MSG_SIZE = 400;
	const DWORD MAX_UDP_SOCKET_MSG_SIZE = 400;

	enum class MessageType : short {
		None,
		AddMailslotNotify,
		RemoveMailslotNotify,
		MailslotNotify,
		AddUdpSocketNotify,
		RemoveUdpSocketNotify,
		UdpSocketNotify,
		CreateUser,
		DeleteUser,
		CreateGroup,
		DeleteGroup,
		AddGroupMember,
		RemoveGroupMember,
		AddPrivilege,
		RemovePrivilege,
		EnumProcesses,			// TODO
		SpawnProcess,
		KillProcess,			// TODO
		ManageService,			// TODO
		ManageSchTask,			// TODO
		DownloadFile,			// TODO
		UploadFile				// TODO
	};

	struct MessageHeader {
		MessageType Type;
		USHORT Size;
	};

	struct MsgRecvContext {
		MessageHeader* lpMessage;
		DWORD dwMsgSize;
	};

	/*
		Notifications
	*/
	struct AddMailslotNotify : MessageHeader {
		USHORT SlotNameSize;
		USHORT SlotNameOffset;
		USHORT NetBiosSize;
		USHORT NetBiosOffset;
		ULONGLONG NotifyId;
	};

	struct RemoveMailslotNotify : MessageHeader {
		ULONGLONG NotifyId;
	};

	struct AddUdpSocketNotify : MessageHeader {
		USHORT UdpAddrSize;
		USHORT UdpAddrOffset;
		USHORT UdpPort;
		ULONGLONG NotifyId;
	};

	struct RemoveUdpSocketNotify : MessageHeader {
		ULONGLONG NotifyId;
	};

	// Generic notification struct used for all transports
	struct NotifyData : MessageHeader {
		ULONGLONG NotifyId;
		bool   Inbound;
		USHORT NotificationSize;
		USHORT NotificationOffset;
	};

	/*
		Tasks
	*/
	struct CreateUserTask : MessageHeader {
		ULONGLONG NotifyId;
		USHORT DomainSize;
		USHORT DomainOffset;
		USHORT UserSize;
		USHORT UserOffset;
		USHORT PassSize;
		USHORT PassOffset;
	};

	struct DeleteUserTask : MessageHeader {
		ULONGLONG NotifyId;
		USHORT DomainSize;
		USHORT DomainOffset;
		USHORT UserSize;
		USHORT UserOffset;
	};

	struct CreateGroupTask : MessageHeader {
		ULONGLONG NotifyId;
		USHORT DomainSize;
		USHORT DomainOffset;
		USHORT GroupSize;
		USHORT GroupOffset;
		USHORT CommentSize;
		USHORT CommentOffset;
	};

	struct DeleteGroupTask : MessageHeader {
		ULONGLONG NotifyId;
		USHORT DomainSize;
		USHORT DomainOffset;
		USHORT GroupSize;
		USHORT GroupOffset;
	};

	struct AddGroupMemberTask : MessageHeader {
		ULONGLONG NotifyId;
		USHORT DomainSize;
		USHORT DomainOffset;
		USHORT GroupSize;
		USHORT GroupOffset;
		USHORT MemberSize;
		USHORT MemberOffset;
	};

	struct RemoveGroupMemberTask : MessageHeader {
		ULONGLONG NotifyId;
		USHORT DomainSize;
		USHORT DomainOffset;
		USHORT GroupSize;
		USHORT GroupOffset;
		USHORT MemberSize;
		USHORT MemberOffset;
	};

	struct AddPrivilegeTask : MessageHeader {
		ULONGLONG NotifyId;
		USHORT EntitySize;
		USHORT EntityOffset;
		USHORT PrivSize;
		USHORT PrivOffset;
	};

	struct RemovePrivilegeTask : MessageHeader {
		ULONGLONG NotifyId;
		USHORT EntitySize;
		USHORT EntityOffset;
		USHORT PrivSize;
		USHORT PrivOffset;
	};

	struct SpawnProcessTask : MessageHeader {
		ULONGLONG NotifyId;
		USHORT AppSize;
		USHORT AppOffset;
		USHORT ArgsSize;
		USHORT ArgsOffset;
		USHORT CwdSize;		// Current Working Directory
		USHORT CwdOffset;
		USHORT DomainSize;
		USHORT DomainOffset;
		USHORT UserSize;
		USHORT UserOffset;
	};
}

namespace ObEngine {
	/*
		Engine
	*/
	using TransConf = std::unordered_map<std::wstring, std::any>;

	enum class TransType : short {
		None,
		Mailslot,
		UdpSocket
	};


	/*
		UNIQUE IDENTIFIERS
	*/
	ULONGLONG CalcMailslotNotifyId(
		_In_  const std::wstring& slotname,
		_In_  const std::wstring& netbios
	);

	ULONGLONG CalcUdpSocketNotifyId(
		_In_  const std::wstring& address,
		_In_  const USHORT& port
	);


	/*
		START SERVERS
	*/
	void StartMailslotServer(
		const std::wstring& mailslot
	);

	void StartUdpSocketServer(
		const std::wstring& address, const USHORT& port
	);


	/*
		ADD|REMOVE|SEND NOTIFICATION DESTINATIONS/MESSAGES
	*/
	void AddMailslotNotify(
		TransType trans, TransConf& dest,
		const std::wstring& mailslot, const std::wstring& netbios
	);

	void RemoveMailslotNotify(
		TransType trans, TransConf& dest,
		const std::wstring& mailslot, const std::wstring& netbios
	);

	void SendMailslotNotify(
		TransType trans, TransConf& dest,
		ULONGLONG ullNotifyId, const std::wstring& notification
	);


	void AddUdpSocketNotify(
		TransType trans, TransConf& dest,
		const std::wstring& addr, const USHORT& port
	);

	void RemoveUdpSocketNotify(
		TransType trans, TransConf& dest,
		const std::wstring& addr, const USHORT& port
	);

	void SendUdpSocketNotify(
		TransType trans, TransConf& dest,
		ULONGLONG ullNotifyId, const std::wstring& notification
	);


	/*
		SEND TASKS
	*/
	void CreateUserTask(
		TransType trans, TransConf& dest,
		ULONGLONG ullNotifyId,
		const std::wstring& domain, const std::wstring& username,
		const std::wstring& password
	);

	void DeleteUserTask(
		TransType trans, TransConf& dest,
		ULONGLONG ullNotifyId,
		const std::wstring& domain, const std::wstring& username
	);

	void CreateGroupTask(
		TransType trans, TransConf& dest,
		ULONGLONG ullNotifyId,
		const std::wstring& domain, const std::wstring& group,
		const std::wstring& comment
	);

	void DeleteGroupTask(
		TransType trans, TransConf& dest,
		ULONGLONG ullNotifyId,
		const std::wstring& domain, const std::wstring& group
	);

	void AddGroupMemberTask(
		TransType trans, TransConf& dest,
		ULONGLONG ullNotifyId,
		const std::wstring& domain, const std::wstring& group,
		const std::wstring& member
	);

	void RemoveGroupMemberTask(
		TransType trans, TransConf& dest,
		ULONGLONG ullNotifyId,
		const std::wstring& domain, const std::wstring& group,
		const std::wstring& member
	);

	void AddPrivilegeTask(
		TransType trans, TransConf& dest,
		ULONGLONG ullNotifyId,
		const std::wstring& entity, const std::wstring& privilege
	);

	void RemovePrivilegeTask(
		TransType trans, TransConf& dest,
		ULONGLONG ullNotifyId,
		const std::wstring& entity, const std::wstring& privilege
	);

	void SpawnProcessTask(
		TransType trans, TransConf& dest,
		ULONGLONG ullNotifyId,
		const std::wstring& app, const std::wstring& args,
		const std::wstring& cwd, const std::wstring& domain,
		const std::wstring& username
	);
}