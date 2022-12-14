# UNIX-Terminal-Shell
[![C](https://img.shields.io/badge/C-A8B9CC?style=for-the-badge&logo=c&logoColor=white)]()

## Project Summary
`Yosh` is a rudimentary shell built in C, which mimicks the core functionality of the UNIX shell. When the shell is started, the user
will be prompted to enter a UNIX command. The shell supports several commands some of which were built-in, including `jobs`, `history`, 
`kill`, `cd`, `help`, and `exit`. Additionally, the shell allows for input/output redirection, piping, and background/foreground
processing for a given external or built-in command when applicable. After the command is entered, the shell will excuete the command and 
display any appropiate output for the command in the terminal.

## How to Compile?
First, ensure that you are in the `UNIX-Terminal-Shell` directory and then run the following commands:
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
First, ensure that you are in the `UNIX-Terminal-Shell` directory and then run the following command: 
```
$ ./yosh
```

## Demo Video

https://user-images.githubusercontent.com/83044307/208279480-03a888bb-1fc8-4aa5-b9c4-a16c6bd7399b.mp4
