This is an experimental tool for debugging purposes. The application use ptrace to get the child process information. The Dwarf and the Unwind libraries allows it to obtain additional information on the tracked bytecode execution. There are two plateformes : 

 - the first is the console version of the debugger. It is the most advanced tool of both ; 
 - the second use the NCurses API to improve readability and carries the implementation from the first console version. This version is unstable at the moment but is going to be improved soon. This could be the version 
used for the future demonstration. Here is a screenshot of the application :

![ndb_picture](https://user-images.githubusercontent.com/30315405/159188426-1f9c0234-8a4f-4555-a5fb-3116c30f5263.png)

Hence, there is two versions of the projet. We decided to separate into to files the project until the NCurses tool is stable. 

Here is the protocole for the installation : 

 - for the terminal version
 - The NCurses version is started with the command `bash configure`. This will install  the thread-safe NCurses version, and to install the lib_vec tool from @gabrl. You need to install `libdwarf-dev` and `libunwind-ptrace`  as well. Hence, the application is started with `target/interface`. Yhe Help panel show you how to use the tool. You can enter for example `exec ./test`.





