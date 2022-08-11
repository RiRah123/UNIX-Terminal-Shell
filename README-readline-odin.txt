Get readline to work on odin:
 option 1: get the libraries via a package download package and install it 
  	in your homedirectory
 option 2: use the instructores instsalled library - Makefile is set up for this.
 option 3: or wait until installed by root by system staff (we have make a request)
		--> support has instsalled libraries - 4/1 - not April Fool's


Example:

{cf7:maria:1} cd ${HOME}/1730/P4
{cf7:maria:6} make clean
rm -f shell *~ 
rm -f rlbasic rlbasic.o
rm -f histexamp histexamp.o
{cf7:maria:7} make shell
gcc -g -Wall -o shell shell.o parse.o /home/myid/ingrid/usr/local/lib/libreadline.a -ltermcap -lcurses

