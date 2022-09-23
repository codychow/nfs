# nfs

//// Functionality ///////////////////
Originally tested using my university's server for computer science with their provided ports.
I have not tested the functionality on separate servers yet. Using a compiler, (I used Makefile
as included in repository) run "nfsserver [port]" first, then run "nfsclient [server]:[port]" in
a separate console to connect. All working commands are listed below. After a command is entered,
a response message will be sent to the client from the server with either an OK message or an
error message with an error code. The length of the message will also be displayed. To quit,
simply type "quit", and the connection will be terminated.

//// Commands ///////////////////
mkdir [dname]: creates a working directory
ls: lists all directories and files existing in current directory.
cd [dname]: switches current directory to a specified directory.
home: switches to home directory.
rmdir [dname]: removes a specified directory (must be empty).
create [fname]: creates a file.
append [fname] [data]: appends data to a specified file in current directory.
stat [dname/fname]: takes either a directory or file name and lists its stats. 
  (name and directory block for directories) 
  (inode block, bytes, num of blocks file consumes, and block num of first block for files)
cat [fname]: lists the contents of a file. 
head [fname] [n]: displays the first n bytes of a file.
rm [fname]: removes a file.

//// Functionality issues ///////////////////
Most of the NFS functions work correctly and as expected.
The main issues stem from the append function and namesharing between
a file and directory in the same directory. Append is only able to append
text without spaces or /n. Additionally, the function responsible for
parsing a command allows text under ~4000 characters to appended at a time.
Namesharing is possible for a file and directory, but the user is only able
to use create/mkdir and rm/rmdir on the nameshared file and directory. 
...

//// Test cases ///////////////////////////////////////////
Test case #1: mkdir abc
Results: directory abc is created
200 OK/r/n
Length:0/r/n
/r/n

Test case #2: cd abc
Results: changed working directory to abc
200 OK/r/n
Length:0/r/n
/r/n

Test case #3: create one
Results: empty file named one is created
200 OK\r\n
Length:0\r\n
\r\n

Test case #4: mkdir two
Results: directory two is created
200 OK\r\n
Length:0\r\n
\r\n

Test case #4: append one foo
Results: foo is appended to one
200 OK\r\n
Length:0\r\n
\r\n

Test case #5: head one 2
Results: first two bytes of one are printed
200 OK\r\n
Length:2\r\n
\r\n
fo

Test case #6: cat one
Results: contents of one are printed
200 OK\r\n
Length:3\r\n
\r\n
foo

Test case #7: append one foo, stat one
Results: foo is appended to one without allocating unnecessary blocks
200 OK\r\n
Length:0\r\n
\r\n
200 OK\r\n
Length:66\r\n
\r\n
Inode block: 6
Bytes in file: 6
Number of blocks: 2
First block: 8

Test case #8: ls
Results: contents of working directory are printed 
200 OK\r\n
Length:9\r\n
\r\n
one two/

Test case #9: mkdir three, rmdir two, ls
Results: directory two is removed if empty without affecting other entries
200 OK\r\n
Length:0\r\n
\r\n
200 OK\r\n
Length:0\r\n
\r\n
200 OK\r\n
Length:11\r\n
\r\n
one three/

...
