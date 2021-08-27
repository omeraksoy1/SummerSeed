# SummerSeed
This repository contains assignments that I completed during my Summer 2021 internship.


### C Project

To run the main program, you need to have the libxml2-dev package installed, and also link it during compliation. The command for my environment was,

'gcc main-app.c -I/usr/include/libxml2 -lxml2 -o main'

To run the CLI that interacts with the main program, a simple gcc compilation will suffice.

'gcc summerseed_cli.c -o cli'

The CLI will not initialize unless it successfully connects to a "server" listening on the server path, thus the main-app needs to be run first.

The predefined server path is "/tmp/server" and is set at the top of both C files with define statements.
The buffer length for socket communication between the two programs is 1024 and is also predefined.
