# Obligato
This project is an implant framework designed for long term persistent access to Windows machines. The goals are centered around disassociating implant tasks and network activity from the implant service. The primary intended use if for maintaining or regaining access to compromised endpoints.

The initial release is the code used in the demos for my DEFCON 31 talk [Malware Design - Abusing legacy Microsoft transports and session architecture](https://forum.defcon.org/node/245731). This does not implement the access controls and transport encryption layers. These features will be pushed after integrating the existing functionality of the command-line agent into the GUI. I am currently the only developer, so community help is welcome!

Please checkout my talk or the companion slides to get more details and specifics on the underlying components, functionality, and use cases.

## Build Instructions & Information
I used Microsoft Visual Studios 2019 and the latest version of the Windows 10 SDK to build this project, so make sure you have the C++ dependencies installed in VS. The ```ObEngine``` project builds a library that is configured as a dependency to the other three projects ```ObAgent```, ```ObClient```, and ```ObService```. To build this Obligato, open the solution in Visual Studios, right click the solution, and choose build. This will begin by building the ObEngine library and then proceed to the other projects that leverage it as a dependency. Each of the projects are described in the following sections.

### ObEngine
This is a library that implements all the functionality of the implant tasking protocol. This is where new transports, tasks, routes, notifications, and other core functionality should be created.

### ObService
This is the implant service executable that is to be installed on the target endpoint.

### ObAgent
This is the command-line agent used to send tasks to remote targets that are implanted and receive the optional responses. This agent can also start the same transport listeners implemented by the ObService. Please note this should only be used for testing because certain functionality within the implant depends on running as system in session 0.

### ObClient
This is just like the ObAgent except it offers all the features in an easy to use GUI. There are a couple of features in the ObAgent that I am still integrating into this GUI, such as, receiving the optional response notifications. This functionality and the access controls/encryption will be pushed very soon.

# Disclaimer
This proof-of-concept code has been created for academic research and is not intended to be used against systems except where explicitly authorized. The code is provided as is with no guarantees or promises on its execution. I am not responsible or liable for misuse of this code.
