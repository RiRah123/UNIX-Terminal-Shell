# UNIX-Shell-Terminal

## Project Summary
`Yosh` is a rudimentary shell built in C, which micks the core functionality of the UNIX shell. When the shell is started, the user
will be prompted to enter a UNIX command. The shell supports several commands some of which were built-in, including `jobs`, `history`, 
`kill`, `cd`, `history`, `help`, and `exit`. Additionally, the shell allows for input/output redirection, piping, and background/foreground
processing for a given external or built-in command when applicable. After the command is entered, the shell will excuete the command and 
display any appropiate output for the command in the terminal.

## How to Compile?
First, ensure that you are in the `UNIX-Shell-Terminal` directory and then run the following commands:
```
$ make clean
$ make
```

If the Makefile isn't working, then you make try the following commands as well in the same directory:
```
$ gcc -g -Wno-parentheses -Wno-format-security -DHAVE_CONFIG_H -DRL_LIBRARY_VERSION='"8.1"' -I/home/myid/ingrid/usr/local/include -c yosh.c
$ gcc -g -Wno-parentheses -Wno-format-security -DHAVE_CONFIG_H -DRL_LIBRARY_VERSION='"8.1"' -I/home/myid/ingrid/usr/local/include -c parse.c
$ gcc -g -Wall -o yosh yosh.o parse.o /home/myid/ingrid/usr/local/lib/libreadline.a -ltermcap -lcurses
```

## How to Run?
First, ensure that you are in the `UNIX-Shell-Terminal` directory and then run the following command: 
```
$ ./yosh
```

## Technologies Used
Programming Lanugages: C, Makefile, Valgrind

## Demo Video
[![Unix Shell Terminal Demo](https://img.youtube.com/vi/dlJn5VeZZUw/maxresdefault.jpg)](https://youtu.be/dlJn5VeZZUw "Unix Shell Terminal Demo")
