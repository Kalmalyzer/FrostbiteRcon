
This is a C++ library that talks to the RCON interface for Frostbite game servers (currently BFBC2 PC, MOH2010 PC, and BF3 PC).
It also includes a console application.

The project/solution files will load in Visual Studio 2010.

It is not very well tested, it has no comments, it has no usage instructions, the test programs are very quirky... but I'll give a rundown here.

There are a couple of different projects in there:
RconLibrary is the interface library itself.
...Test are short test programs.
Console is a simple command console.
ServerVersionQuery is a program that issues "version" requests to a bunch of servers in parallel.

Now if we break down RconLibrary a bit, you find three classes for managing packets - see RconPacket.h:
'Words' is how you will likely pass around commands outside of RconLibrary.
When it's time to transmit a command to the server, it is converted to a TextRconPacket. This is essentially the Words, along with sequence info.
The final step before transmission is converting it to a BinaryRconPacket. This is a contigous stream of bytes, ready to be sent over the wire.
When receiving a packet from the server, it will go through the classes in the reverse order until it is presented as plain Words outside of the library.

Then there are three different communications modules:
SynchronousServerConnection is single-threaded, simple to use, but will only allow the user to send commands to the server. It is not suitable for receiving events.
AsynchronousServerConnection is single-threaded. It can be used to send commands and receive events. It requires the user to call an update() method with regular intervals.
ThreadedServerConnection is multithreaded. It can be used to send commands and receive events.
The callback design for the async & threaded communications modules is still quite messy. I hope to work a bit more on it in the future.

Other stuff you'll find in there:
ServerConnectionTrafficBase is used to snoop on what's going on in the communications modules. It is intended for debugging.
Mutex is a win32 CrititcalSection packaged in an OS-agnostic interface.
ThreadBase is a win32 Thread packaged in an OS-agnostic interface.


And yes, there are some known problems. Some pretty bad, other minor.
- If enabling events, then it seems that a BF3 server will send back the command. Either that, or there is some bug in the lower levels - but I checked with Wireshark and it really looks like the server bounces back an 'admin.eventsEnabled true' to the client. This will generate an exception in the communications modules. Ideally the problem should be fixed on the BF3 server side though.
- the ServerVersionQuery program will occasionally complain about a pure function call when querying real servers. This makes me think that there is an obscure race condition hiding in the thread usage, or the Mutex or the ThreadBase classes. Don't trust them just yet.