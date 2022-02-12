# Homework #2
<font color=#AFB2B1>Intro. to Network Programming</font>
<font color=#D65151 size=5>Due: Nov 21, 2021</font>
=
## Description
Continuing the first part, you are asking to implement boards and posts in the Bulletin Board System (BBS). The server should be **single process** and use **select** or **poll** to handle **multiple connections**.

The TAs will use the **diff** tool to compare your output directly against our prepared sample testdata. Spaces and tabs are compressed into a single character when comparing the outputs.
## Requirement
* The service accepts the following commands and <font color=#D65151>at least 10 clients</font>.
* When client enter command incompletely E.g., missing parameters, the server should show the following **command format** for client. **(11/16 Updated)**
    * `Usage: <command format>` 
* If command is in the right format, the first failure message will have the higher priority, for example of the “create-board” command, the user didn’t login, then the result should be “Please login first”.

### Basic
* **`create-board <name>`** : 
    * Create a board which named `<name>`. `<name>` must be unique. 
    * `<name>` will not contain any spaces. **(11/16 Updated)**
    * If Board's name is already used, show failed message, otherwise it is success. 
    * Must be logged in when creating board’s name. 
    * Success: `Create board successfully.`
    * Fail (1): `Please login first.`
    * Fail (2): `Board already exists.`
* **`create-post <board-name> --title <title> --content <content>`** : 
    * Create a post which title is `<title>` and content is `<content>`.
    * Use `--title` and `--content` to separate titles and content.
    * `--title` and `--content` can switch, so `create-post <board-name> --content <content> --title <title>` is also valid. **(11/16 Updated)**
    * For usage, you only need to output `Usage: create-post <board-name> --title <title> --content <content>`. **(11/16 Updated)**
    * `<title>` can have space but only in one line.
    * `<content>` can have space, and key in `<br>` to indicate a new line.
    * `<title>` and `<content>` won't contain multiple continuous spaces, there is only single space between their content. **(11/16 Updated)**
    * Assign a **globally unique** serial number `<post-S/N>` to each post. The serial number will start from 1 and increase by creating post.
    * For example, creating a post in A board get serial number 1. After that, createing a post in B board get serial number 2 instead of serial number 1.
    * Success: `Create post successfully.`
    * Fail (1): `Please login first.`
    * Fail (2): `Board does not exist.`    
* **`list-board`** : 
    * List all boards in BBS.
    ```
    Index    Name    Moderator
    <Index1> <Name1> <Moderator1>
    ```
* **`list-post <board-name>`** : 
    * List all posts in a board named `<board-name>`
    * Display the date when the post creates.
    * Success:
    ```
    S/N     Title     Author     Date
    <S/N 1> <Title 1> <Author 1> <Date1>
    ```
    * Fail: `Board does not exist.`
* **`read <post-S/N>`** : 
    * Show the post which S/N is <post-S/N>.
    * Success: 
    ```
    Author: <Author1>
    Title: <Title1>
    Date: <Date1>
    --
    <content>
    --
    <User1>: <Comment1>
    ```
    * Fail: `Post does not exist.`
* **`delete-post <post-S/N>`** : 
    * Delete the post which S/N is `<post-S/N>`.
    * Only the post owner can delete the post.
    * If the user is not the post owner, show failed
message, otherwise it is success.
    * Success: `Delete successfully.`
    * Fail (1): `Please login first.`
    * Fail (2): `Post does not exist.`
    * Fail (3): `Not the post owner.`
* **`update-post <post-S/N> --title/content <new>`**:
    * Update the post which S/N is `<post-S/N>`.
    * Use `--` to decide which to modify, title or
content, and replaced by `<new>`.
    * `update-post` won't update title and content at the same time. **(11/16 Updated)**
    * Only the post owner can update the post. If the user is not the post owner, show failed
message, otherwise it is success.
    * Success: `Update successfully.`
    * Fail (1): `Please login first.`
    * Fail (2): `Post does not exist.`
    * Fail (3): `Not the post owner.`
* **`comment <post-S/N> <comment>`**:
    * Leave a comment `<comment>` at the post which S/N is `<post-S/N>`.
    * Success: `Comment successfully.`
    * Fail (1): `Please login first.`
    * Fail (2): `Post does not exist.`

### HW1 command
* The following commands in HW1 should also work in HW2:
    * `register`
    * `login` **(11/19 Updated)**
        * The priority of `Please logout first.` is higher than `Login failed.`    
    * `logout` 
    * `exit`
* Multi-user account management: **(11/19 Updated)**
    * All the client share the same account system. 
    * For example, client A can't register the same username that is already registered by another client.  
    * For another example, client B can login as user `test` registered by client A if there is no one login as `test`.

For more details about the implementation, please check the demonstration section for the sample input and the corresponding output.

To run your server, you must to provide port number for your program. you can use `nc or telnet` command line tool to connect to your server.

`server usage: ./hw2 [port number]`
## Demonstration
```shell
bash$ client 127.0.0.1 7890
********************************
** Welcome to the BBS server. **
********************************
% create-board LoL
Please login first.
% register Maoan 123456
Register successfully.
% register Toyz 420420
Register successfully.
% login Maoan 123456
Welcome, Maoan.
% create-board LoL
Create board successfully.
% create-board LoL
Board already exists.
% create-board 3Q
Create board successfully.
% create-board Meta
Create board successfully.
% list-board
Index Name Moderator
1     LoL  Maoan
2     3Q   Maoan
3     Meta Maoan
% create-post Gamble --title About My Dream --content Chances are high<br>Sylas or Orianna
Board does not exist.
% create-post LoL --title About My Dream --content Chances are high<br>Sylas or Orianna
Create post successfully.
% create-post LoL --title R: About My Dream --content Hey!<br>Don't buy too much!
Create post successfully.
% list-post Rescue_3Q
Board does not exist.
% list-post LoL
S/N Title             Author Date
1   About My Dream    Maoan  10/8
2   R: About My Dream Maoan  10/8
% read 100
Post does not exist.
% read 1
Author: Maoan
Title: About My Dream
Date: 10/8
--
Chances are high
Sylas or Orianna
--
% update-post 100 --title This is BYG. We got his smartphone. 
Post does not exist.
% update-post 1 --title This is BYG. We got his smartphone.
Update successfully.
% read 1
Author: Maoan
Title: This is BYG. We got his smartphone.
Date: 10/8
--
Chances are high
Sylas or Orianna
--
% logout
Bye, Maoan.
% login Toyz 420420
Welcome, Toyz.
% update-post 1 --content To win the championship, BYG can hire me from prison.
Not the post owner.
% delete-post 1
Not the post owner.
% comment 100 To win the championship, BYG can hire me from prison.
Post does not exist.
% comment 1 To win the championship, BYG can hire me from prison.
Comment successfully.
% read 1
Author: Maoan
Title: This is BYG. We got his smartphone.
Date: 10/8
--
Chances are high
Sylas or Orianna
--
Toyz: To win the championship, BYG can hire me from prison.
% create-board Drug
Create board successfully.
% list-board
Index Name Moderator
1     LoL  Maoan
2     3Q   Maoan
3     Meta Maoan
4     Drug Toyz
% logout
Bye, Toyz.
% login Maoan 123456
Welcome, Maoan.
% delete-post 1
Delete successfully.
% read 1
Post does not exist.
% logout
Bye, Maoan.
% exit
```

## Homework Submission
We will compile your homework by simply typing 'make' (if you provide a Makefile) or following commands in your homework directory.
`gcc hw2.c -o hw2`
`g++ hw2.cpp -o hw2`

Please pack your C/C++ code into a zip archive. The directory structure should follow the below illustration. The id is your student id. Please note that you don't need to enclose your id with the braces.

```
{id}_hw2.zip
└── {id}_hw2/
    ├── hw2.c or hw2.cpp
    ├── (Makefile)
    └── (any other c/c++ files if needed)
```

Scores will be graded based on the completeness of your implementation.

## Remarks
* Please implement your homework in C or C++.
* Using any non-standard libraries and any external binaries (e.g., via system()) are not allowed.
* No copycats. Please do not use codes from others (even open source projects).
* We will test your program in **Ubuntu 20.04 LTS** Linux with the default gcc version (9.3.0).
* **For late submission, the deadline is 11/28 (updated) and your grade will get 30% off.**