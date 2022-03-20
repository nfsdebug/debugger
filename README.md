This is an experimental tool for debugging purposes. The application uses ptrace to get the child process information. The Dwarf and the Unwind libraries allows it to obtain additional information on the tracked byte code execution. There are two platforms :

 - the first one is the console version of the debugger. It is the most advanced tool of both ;
 - the second one use the NCurses API to improve readability and carries the implementation from the first console version. This version is unstable at the moment, but is going to be improved soon. This could be the version
used for the future demonstration. Here is a screenshot of the application :

![ndb_picture](https://user-images.githubusercontent.com/30315405/159188426-1f9c0234-8a4f-4555-a5fb-3116c30f5263.png)

Hence, there are two versions of the project. We decided to separate into two branchs the project until the NCurses version is stable.

Here is the protocol for the installation :

 - The terminal version required to launch `make`. The tool is launched with the command `./debug YourApp`;
 - The NCurses version is started with the command `bash configure.sh`. This will install  the thread-safe NCurses version, and to install the lib_vec tool from @dss_gabriel. You need to install `libdwarf-dev` and `libunwind-ptrace`  as well. Hence, the application is started with `target/interface`. The Help panel show you how to use the tool. You can enter, for example, `exec ./test`. Note that you should load the vec library with the command `export LD_LIBRARY_PATH=/home/YourUserName/debugger/ext/vec`.

Here are some implemented functionalities :
 - sighandler, which can know on what signal the traced program failed ;
 - loaded lib viewer and the associated ranges ;
 - backtrack viewer ;
 - breakpoint ;
 - DWARF knowledge ;
 - registers viewer ;
 - etc


Note : The NCurses tool is experimental, and its behavior depends on the platform you use.



