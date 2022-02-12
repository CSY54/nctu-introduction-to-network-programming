# Homework #1
<font color=#AFB2B1>Intro. to Network Programming</font>
<font color=#D65151 size=5>Due: Oct 27, 2021</font>
## Update
* Change from **~~multiuser~~** to a **single** user. Only allow one user connect to the server at the same time.
* The original multiuser scenario would be bonus feature of the homewoek. (additonal 20% score)
* Output of `list-user` should be sorted by user name alphabetically.
## Description
In this project, you are asked to design Bulletin Board System (BBS) server. Your program should be able to handle **single connection** and receive user command from client socket. After receiving command, the server send the corresponding message back. 

The TAs will use the **diff** tool to compare your output directly against our prepared sample testdata. Spaces and tabs are compressed into a single character when comparing the outputs.
## Requirement
The service accepts the following commands with single user:
When client enter command incompletely E.g., missing parameters, the server should show command format for client.

### Basic
* **register <username> <password>** : Register with username and password. <username> must be unique. <password> have no limitation. If username is already used, show failed message, otherwise it is success.
* **login <username> <password>** : Login with username and password. Fail (1): User already login. Fail (2): Username or password is incorrect.
* **logout** : Logout account. If you haven’t logged in yet, show failed message, otherwise logout successfully.
* **whoami** : Show your username. If you haven’t logged in yet, show failed message. Otherwise, show username.
* **list-user** : List all users in BBS. The Output is sorted by user name alphabetically. **(Update 10/15)**
* **exit** : Close connection.
### Message Box
If you haven’t logined yet, show failed message.
* **send <username> <message>** : leave message to a user. If <username> is not existed, show user not exist error message.
* **list-msg** : list the message state of Message Box, the message state is sorted by user name alphabetically. 
* **receive <username>** : Show the first message from <username>, and pop the message. If <username> is not existed, show user not exist error message.

For more details about the implementation, please check the demonstration section for the sample input and the corresponding output.


Use **‘’% “** as the command line prompt. Notice that there is only one space after the prompt. The server close connection if client use exit command, but server still running and client can connect again.

To run your server, you must to provide port number for your program. you can use `nc or telnet` command line tool to connect to your server.

`server usage: ./hw1 [port number]`
## Demonstration
### Basic
```shell
bash$ nc localhost 1234
********************************
** Welcome to the BBS server. **
********************************
% register
Usage: register <username> <password>
% register Toyz 420420
Register successfully.
% register Toyz 420420
Username is already used.
% login
Usage: login <username> <password>
% login Toyz
Usage: login <username> <password>
% login Toyz 000000
Login failed.
% login Tom 420420
Login failed.
% login Toyz 420420
Welcome, Toyz.
% login Toyz 420420
Please logout first.
% whoami
Toyz
% logout
Bye, Toyz.
% logout
Please login first.
% whoami
Please login first.
% register Godtone 777777
Register successfully.
% list-user
Godtone
Toyz
% exit
```
### Message Box
After basic senario. 
```shell
bash$ nc localhost 1234
********************************
** Welcome to the BBS server. **
********************************
% register Liang 555555
Register successfully.
% send
Usage: send <username> <message>
% send Godtone "Hello! my dog."
Please login first.
% login Toyz 420420
Welcome, Toyz.
% send nobody "Hello! my dog"
User not existed.
% send Godtone "Hello! my dog"
% send Godtone "(licking lips)"
% logout
Bye, Toyz.
% login Liang 555555
Welcome, Liang.
% list-msg
Your message box is empty.
% send Godtone "where is my 50000?"
% logout
Bye, Liang.
% login Godtone 777777
Welcome, Godtone.
% receive nobody
User not existed.
% list-msg
1 message from Liang.
2 message from Toyz.
% receive Toyz
Hello! my dog
% list-msg
1 message from Liang.
1 message from Toyz.
% send Toyz "I'm your drug detection dog"
% logout
Bye, Godtone.
% exit
```
### Special case
Exit when login
```shell
% login Toyz 420420
Welcome, Toyz.
% exit
Bye, Toyz.
```

## Homework Submission
We will compile your homework by simply typing 'make' (if you provide a Makefile) or following commands in your homework directory.
`gcc hw1.c -o hw1`
`g++ hw1.cpp -o hw1`

Please pack your C/C++ code into a zip archive. The directory structure should follow the below illustration. The id is your student id. Please note that you don't need to enclose your id with the braces.

```
{id}_hw1.zip
└── {id}_hw1/
    ├── hw1.c or hw1.cpp
    ├── (Makefile)
    └── (any other c/c++ files if needed)
```

Scores will be graded based on the completeness of your implementation.

## Remarks
* Please implement your homework in C or C++.
* Using any non-standard libraries and any external binaries (e.g., via system()) are not allowed.
* No copycats. Please do not use codes from others (even open source projects).
* We will test your program in **Ubuntu 20.04 LTS** Linux with the default gcc version (9.3.0).

## Bonus (20% score)
The original multi-user senario will be bonus part of the homework, if your program can handle multi-user senario, you will get **20% bonus score**.