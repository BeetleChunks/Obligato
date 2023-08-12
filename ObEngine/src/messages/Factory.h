#pragma once

#include "ObEngine.h"
#include "Helpers.h"

namespace MessageFk {
	/*
		Generates the UID for a Mailslot notification route
	*/
	ULONGLONG CalcMailslotNotifyId(
		_In_  const std::wstring& SlotName,
		_In_  const std::wstring& NetBios
	);

	/*
		Generates the UID for a UDP Socket notification route
	*/
	ULONGLONG CalcUdpSocketNotifyId(
		_In_  const std::wstring& Addr,
		_In_  const USHORT& Port
	);

	
	/*
		Allocates and builds an AddMailslotNotify message. The caller
		is responsible for freeing the message buffer.
	*/
	bool AddMailslotNotifyMessage(
		_In_  const std::wstring& SlotName,
		_In_  const std::wstring& NetBios,
		_Out_ ObEngineRef::AddMailslotNotify** lpMessage,
		_Out_ DWORD& dwMessageSize,
		_Out_ ULONGLONG& ullNotifyId
	);

	/*
		Allocates and builds a RemoveMailslotNotify message. The caller
		is responsible for freeing the message buffer.
	*/
	bool RemoveMailslotNotifyMessage(
		_In_  const ULONGLONG& ullNotifyId,
		_Out_ ObEngineRef::RemoveMailslotNotify** lpMessage,
		_Out_ DWORD& dwMessageSize
	);

	/*
		Allocates and builds an OUTBOUND MailslotNotify message. The
		caller is responsible for freeing the message buffer.
	*/
	bool MailslotNotifyMessage(
		_In_  const ULONGLONG& ullNotifyId,
		_In_  const std::wstring& Notification,
		_Out_ ObEngineRef::NotifyData** lpMessage,
		_Out_ DWORD& dwMessageSize
	);


	/*
		Allocates and builds an AddUdpSocketNotify message. The caller
		is responsible for freeing the message buffer.
	*/
	bool AddUdpSocketNotifyMessage(
		_In_  const std::wstring& UdpAddr,
		_In_  const USHORT& usUdpPort,
		_Out_ ObEngineRef::AddUdpSocketNotify** lpMessage,
		_Out_ DWORD& dwMessageSize,
		_Out_ ULONGLONG& ullNotifyId
	);

	/*
		Allocates and builds a RemoveUdpSocketNotify message. The caller
		is responsible for freeing the message buffer.
	*/
	bool RemoveUdpSocketNotifyMessage(
		_In_  const ULONGLONG& ullNotifyId,
		_Out_ ObEngineRef::RemoveUdpSocketNotify** lpMessage,
		_Out_ DWORD& dwMessageSize
	);

	/*
		Allocates and builds an OUTBOUND UdpSocketNotify message. The
		caller is responsible for freeing the message buffer.
	*/
	bool UdpSocketNotifyMessage(
		_In_  const ULONGLONG& ullNotifyId,
		_In_  const std::wstring& Notification,
		_Out_ ObEngineRef::NotifyData** lpMessage,
		_Out_ DWORD& dwMessageSize
	);


	/*
		Allocates and builds a CreateUserTask message. The caller
		is responsible for freeing the message buffer.
	*/
	bool CreateUserMessage(
		_In_  const std::wstring& Domain,
		_In_  const std::wstring& Username,
		_In_  const std::wstring& Password,
		_Out_ ObEngineRef::CreateUserTask** lpMessage,
		_Out_ DWORD& dwMessageSize
	);

	/*
		Allocates and builds a DeleteUserTask message. The caller
		is responsible for freeing the message buffer.
	*/
	bool DeleteUserMessage(
		_In_  const std::wstring& Domain,
		_In_  const std::wstring& Username,
		_Out_ ObEngineRef::DeleteUserTask** lpMessage,
		_Out_ DWORD& dwMessageSize
	);

	/*
		Allocates and builds a CreateGroupTask message. The caller
		is responsible for freeing the message buffer.
	*/
	bool CreateGroupMessage(
		_In_  const std::wstring& Domain,
		_In_  const std::wstring& Group,
		_In_  const std::wstring& Comment,
		_Out_ ObEngineRef::CreateGroupTask** lpMessage,
		_Out_ DWORD& dwMessageSize
	);

	/*
		Allocates and builds a DeleteGroupTask message. The caller
		is responsible for freeing the message buffer.
	*/
	bool DeleteGroupMessage(
		_In_  const std::wstring& Domain,
		_In_  const std::wstring& Group,
		_Out_ ObEngineRef::DeleteGroupTask** lpMessage,
		_Out_ DWORD& dwMessageSize
	);

	/*
		Allocates and builds a AddGroupMemberTask message. The caller
		is responsible for freeing the message buffer.
	*/
	bool AddGroupMemberMessage(
		_In_  const std::wstring& Domain,
		_In_  const std::wstring& Group,
		_In_  const std::wstring& Member,
		_Out_ ObEngineRef::AddGroupMemberTask** lpMessage,
		_Out_ DWORD& dwMessageSize
	);

	/*
		Allocates and builds a RemoveGroupMemberTask message. The caller
		is responsible for freeing the message buffer.
	*/
	bool RemoveGroupMemberMessage(
		_In_  const std::wstring& Domain,
		_In_  const std::wstring& Group,
		_In_  const std::wstring& Member,
		_Out_ ObEngineRef::RemoveGroupMemberTask** lpMessage,
		_Out_ DWORD& dwMessageSize
	);

	/*
		Allocates and builds a AddPrivilegeTask message. The caller
		is responsible for freeing the message buffer.
	*/
	bool AddPrivilegeMessage(
		_In_  const std::wstring& Entity,
		_In_  const std::wstring& Privilege,
		_Out_ ObEngineRef::AddPrivilegeTask** lpMessage,
		_Out_ DWORD& dwMessageSize
	);

	/*
		Allocates and builds a RemovePrivilegeTask message. The caller
		is responsible for freeing the message buffer.
	*/
	bool RemovePrivilegeMessage(
		_In_  const std::wstring& Entity,
		_In_  const std::wstring& Privilege,
		_Out_ ObEngineRef::RemovePrivilegeTask** lpMessage,
		_Out_ DWORD& dwMessageSize
	);

	/*
		Allocates and builds a SpawnProcessTask message. The caller
		is responsible for freeing the message buffer.
	*/
	bool SpawnProcessMessage(
		_In_  const std::wstring& App,
		_In_  const std::wstring& Args,
		_In_  const std::wstring& Cwd,
		_In_  const std::wstring& Domain,
		_In_  const std::wstring& Username,
		_Out_ ObEngineRef::SpawnProcessTask** lpMessage,
		_Out_ DWORD& dwMessageSize
	);
}