# Homework #3
<font color=#AFB2B1>Intro. to Network Programming</font>
<font color=#D65151 size=5>Due: Dec 20, 2021</font>


## Description
In homework 3, we introduce a customized **chat protocol** supporting features including **chat history**, **content filtering** and **blacklist** into the Bulletin Board System (BBS) server. The server should be implemented as a **single process** server and use [select](https://man7.org/linux/man-pages/man2/select.2.html) or [poll](https://man7.org/linux/man-pages/man2/poll.2.html), [epoll](https://man7.org/linux/man-pages/man7/epoll.7.html) to handle **multiple connections**.


## Command Requirements
The service accepts the following commands from users. When a client enters an incomplete command, e.g., missing parameters, the server should send a command format message to the client.

### TCP
In the following cases, a client can simply send user input commands as-is to the server in a **TCP connection**.

- register
    > register \<username> \<password>
    1. if there is any missing parameter
        - **`Usage: register <username> <password>`**
    2. if `<username>` already exists in database
        - **`Username is already used.`**
    3. if a client successfully registers an account
        - **`Register successfully.`**

    
- login
    > login \<username> \<password>
    1. if there is any missing parameter
        - **`Usage: login <username> <password>`**
    2. if the client has already logged in an account
        - **`Please logout first.`**
    3. if the username dose not exist
        - **`Login failed.`**
    4. if the username is in the blacklist
        - **`We don't welcome <username>!`**
    5. if the password is wrong 
        - **`Login failed.`**
    6. if a client successfully logs in
        - **`Welcome, <username>.`**
        <br>
    :::info
    :bulb: If client A has already successfully logged in as userA, other clients (including client A) cannot log in as userA until client A logs out.
    :::
- logout
    > logout
    1. if there is any missing parameter
        - **`Usage: logout`**
    2. if the client has not logged in
        - **`Please login first.`**
    3. if the client successfully logs out
        - **`Bye, <username>.`**


- exit
    > exit
    1. if there is any missing parameter
        - **`Usage: exit`**
    2. if the client has not logged out, run `logout` command first and then `exit` command implicitly, and then show
        - **`Bye, <username>.`**
    3. if exit successfully, just close the connection
    
    <br>
    
    :::info
    :bulb: User should leave the chat room after logout or exit.
    :::
- Enter a chat room
    > enter-chat-room \<port> \<version>
    1. if there is any missing parameter
        - **`Usage: enter-chat-room <port> <version>`**
    2. if `<port>` is not in 1~65535 or is not a number
        - **`Port <port> is not valid.`**
    3. if `<version>` is not 1, 2, nor a number
        - **`Version <version> is not supported.`**
    4. if the client has not logged in
        - **`Please login first.`**
    5. if the client successfully enters the chat room
        - **`Welcome to public chat room.\nPort:<port>\nVersion:<version>\n<chat_history>`**

    <br>
    
    :::info
    :bulb: A user can send multiple **enter-chat-room** to the server. The latter **enter-chat-room** overwrites the former one and **closes former UDP listening port**. Check out the [example](#message-formatversion-1-VS-message-formatversion-2) for more details.
    :::
    
    :::info
    :bulb: \<port> specifies the **UDP port** for chat room that the client ~~and the server~~ will listen on. (<font color="red">We have decreased the implementation difficulty so the server will only use a single UDP port to receive all chat messages.</font>) 
    &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;\<version> specifies the version of the client's chat protocol.
    :::
    
    :::info
    :bulb: In the current design, the server will bind only one **TCP port** and one **UDP port** using the same port number passed to the server via the command line.
    :::
    
- format of **<chat_history>**: 
    - Chat hisotry is composed of serveral **Record**s.
        - `Record1` + `Record2` + ... + `Recordn`
    - The format of a **Record** is
        - **`<username>:<message>\n`**
        <br>
    :::info
    :bulb: Every **\<message>** in **\<chat_history>** should also be filtered.
    :::
    [Exmaple](#Enter-chat-room)
### UDP
In the following cases, client script that we provide will transform **chat \<message>** to the **format of chat protocol**, and will send these messages by **UDP packets**.
- chat
    > chat **`<message>`**
    

    <br>
    
    :::info
    :bulb: The server should send **\<message>** in **the chat protocol format** to each client (including the one that sends the **\<message>**) who has successfully joined the chat room via **enter-chat-room**.
    :::
    :::info
    :bulb: In our testcases, **\<message>** contains all printable ascii characters except `\0` and `\n`.
    :::
    [Example](#Exmaple-chat)

    
    
- **Special Case**: If any part of the **`<message>`** matches a word in the **[filtering list](#filtering-list)**, the server masks the matched words from the message and consider it a rule violation. A user can only violate the rule at most two times. For the 3rd time rule violation, the server should send the message below via the **TCP connection** to the corresponding user and log him/her out. More details about **content filtering** are explained in the next section.
    - **`Bye, <username>.`**
    
    [Example](#Example-Message-filter-and-encounter-3rd-rule-violation)
    

    
    
## Required Features
In this section, we explain details about the required features including the `Chat Protocol`, `Chat History`, `Content Filtering` and `Blacklist`.



### Chat Protocol
#### General format of the chat protocol
| Flag(8bit) | data |
| ---------- | ---- |

#### message format (version 1)
```
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 (bits)
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|      flag     |    version    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|         length of name        |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                               |
+                               +
|                               |
+              name             +
|                               |
+                               +
|                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|       length of message       |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                               |
+                               +
|                               |
+            message            +
|                               |
+                               +
|                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

| Field Name            | Size                       | Description                         |
| --------------------- | -------------------------- | ----------------------------------- |
| **flag**              | 8 bits                     | it must be `0x1`                    |
| **version**           | 8 bits                     | it must be `0x1`                    |
| **length of name**    | 16 bits                    | length of message in **big endian** |
| **name**              | `strlen(name)` * 8 bits    |                                     |
| **length of message** | 16 bits                    | length of message in **big endian** |
| **message**           | `strlen(message)` * 8 bits |                                     |




#### message format (version 2)
```
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 (bits)
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|      flag     |    version    |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                               |
+                               +
|                               |
+                               +
|                               |
+                               +
|              name             |
+               +-+-+-+-+-+-+-+-+
|               |   separator   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                               |
+                               +
|                               |
+                               +
|                               |
+                               +
|            message            |
+               +-+-+-+-+-+-+-+-+
|               |   separator   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

| Field Name    | Size                               | Description                            |
| ------------- | ---------------------------------- | -------------------------------------- |
| **flag**      | 8 bits                             | its must be `0x1`                     |
| **version**   | 8 bits                             | it must be `0x2`                     |
| **name**      | `strlen(base64(name))` * 8 bits    | name encoded in base64    |
| **separator** | 8 bits                             | it must be `'\n'`                    |
| **message**   | `strlen(base64(message))` * 8 bits | message encoded in base64 |
| **separator** | 8 bits                             | its value is `'\n'`                    |



:::info
:bulb: Our testcases do not use `\0` and `\n` in **message**. The total length of \<username> and \<message> does not exceed 1500 bytes.
:::
:::info
:bulb: A client tells the server which version of **message format** is used on sending **enter-chat-room** command.
:::

#### A simple example (C-like pseudo code) for message format (version 1)

```c
/*
 * username: Pekora
 * instruction: chat peko peko
 */
struct a {
    unsigned char flag;
    unsigned char version;
    unsigned char payload[0];
} __attribute__((packed));

struct b {
    unsigned short len;
    unsigned char data[0];
} __attribute__((packed));

#define name "Pekora"
#define mesg "peko peko"
uint16_t name_len = (uint16_t)strlen(name);
uint16_t mesg_len = (uint16_t)strlen(mesg);

unsigned char buf[4096];
struct a *pa = (struct a*) buf;
struct b *pb1 = (struct b*) (buf + sizeof(struct a));
struct b *pb2 = (struct b*) (buf + sizeof(struct a) + sizeof(struct b) + name_len);
pa->flag = 0x01;
pa->version = 0x01;
pb1->len = htons(name_len);
memcpy(pb1->data, name, name_len);
pb2->len = htons(mesg_len);
memcpy(pb2->data, mesg, mesg_len);
```

#### A simple example (C-like pseudo code) for message format (version 2)
```c
/*
 * username: Pekora
 * instruction: chat peko peko
 */
#define name base64("Pekora")
#define mesg base64("peko peko")

sprintf(packet, "\x01\x02%s\n%s\n", name, mesg);
```
### Chat History
When a user joins the public (`enter-chat-room`) chat room, the server should immediately encapsulate all messages in **chat history** in **`the format of <chat_history>`** and sends it to the client in the **TCP connection**. The **`format of <chat_history>`** is:

```
<username>:<message>\n<username>:<message>\n...
```

[Example](#Enter-chat-room)

### Content Filtering

To keep our chat room a place filling with joy and happiness, some keywords should not be used. Below is the filtering list of those keywords. The server must mask each matched word in `<message>` sent by the `chat` command by replacing the word with a string of `*` characters of equal length. 
**You must hard code filtering list in your C/C++ program file.**

#### filtering list
```=
how
you
or
pek0
tea
ha
kon
pain
Starburst Stream
```

:::info
:bulb: Keyword matching is **case sensitive**.
:::

[Example](#Example-Message-filter-and-encounter-3rd-rule-violation)



### Blacklist
To maintain the quality of our chat room, we introduce a blacklist mechanism to ban those people who violate our **content filtering** policy three or more times.




[Example](#Example-Message-filter-and-encounter-3rd-rule-violation)

## Demonstration
In this homework, **we suggest you use [wireshark](https://www.wireshark.org/)** to capture the traffic and debug. 
[Simple lesson of wireshark](https://hackmd.io/@ZU1Ii7IHRuWGpOkPKu-v5w/Byv7i0gtK)
### Basic login, logout, register and exit case
```
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
% logout
Bye, Toyz.
% logout
Please login first.
% register Godtone 777777
Register successfully.
% exit
```

### Special case
Exit after logged in
```
********************************
** Welcome to the BBS server. **
********************************
% register Toyz 420420
Register successfully.
% login Toyz 420420 
Welcome, Toyz.
% exit
Bye, Toyz.
```
    
### Exmaple: chat
[\[Download\] example3.pcapng](https://drive.google.com/file/d/1nrXqu6yv-8-Ya0n7v15yJ2NDkuSi8__4/view?usp=sharing)
In the following example, `chat hello world` must be transformed to chat protocol first, and then send to server by **UDP packet**. 
`Toyz: hello world` is received from server and it is also send by **UDP packet**.
```
********************************
** Welcome to the BBS server. **
********************************
% register Toyz 420420
Register successfully.
% login Toyz 420420
Welcome, Toyz.
% enter-chat-room 42042 1
Welcome to public chat room.
Port:42042
Version:1
% chat hello world
Toyz:hello w**ld
% exit
Bye, Toyz.
```
Client send the UDP packet below right after executing `chat hello world`.
![](https://i.imgur.com/U7JEnTT.png)
After server receives No.20 packet, it sends the UDP packet below to client.
![](https://i.imgur.com/Smrh3DW.png)

### Example: Message filter and encounter 3rd rule violation
[\[Download\] example5.pcapng](https://drive.google.com/file/d/1nrYSRApPL51GGpCa0fvSuzQqvesqnH6s/view?usp=sharing)
kirito
```
********************************
** Welcome to the BBS server. **
********************************
% register kirito 20070930
Register successfully.
% login kirito 20070930
Welcome, kirito.
% enter-chat-room 8763 1
Welcome to public chat room.
Port:8763
Version:1
% chat Hold 10 seconds!!!
kirito:Hold 10 seconds!!!
% chat Starburst Stream
kirito:****************
% logout
Bye, kirito.
% login kirito 20070930
Welcome, kirito.
% enter-chat-room 8763 1
Welcome to public chat room.
Port:8763
Version:1
kirito:Hold 10 seconds!!!
kirito:****************
%  chat Starburst Stream?????
kirito:****************?????
% chat Starburst Stream!!!???
Bye, kirito.
% login kirito 20070930
We don't welcome kirito!
% exit
```
klein
```
********************************
** Welcome to the BBS server. **
********************************
% register klein klein
Register successfully.
% login klein klein
Welcome, klein.
% enter-chat-room 12345 2
Welcome to public chat room.
Port:12345
Version:2
% kirito:Hold 10 seconds!!!
% kirito:****************
% kirito:****************?????
% kirito:****************!!!???
% exit
Bye, klein.
```
:::info
:bulb: In this case, After kirito execute `chat Starburst Stream!!!???`, kirito won't receive `kirito:****************!!!???` but kelin will.
:::
### Multiple users
[\[Download\] example4.pcapng](https://drive.google.com/file/d/1nn-ZC6TzTyCBvZUYSR0zqOojG-so6S2y/view?usp=sharing)
unknown
```
********************************
** Welcome to the BBS server. **
********************************
% register unknown TA
Register successfully.
% login unknown TA
Welcome, unknown.
% enter-chat-room 12345 1
Welcome to public chat room.
Port:12345
Version:1
% chat hey, siri!
unknown:hey, siri!
% siri:W**t can I help *** with?
% chat Who is the best Taiwan vtuber?
unknown:Who is the best Taiwan vtuber?
% siri:Mochikomame
% chat wo hao xing fen!!!!
unknown:wo **o xing fen!!!!
% exit
Bye, unknown.
```
siri (Login after unknown send "hey, siri!")
```
********************************
** Welcome to the BBS server. **
********************************
% register siri siri
Register successfully.
% login siri siri
Welcome, siri.
% enter-chat-room 23456 2
Welcome to public chat room.
Port:23456
Version:2
unknown:hey, siri!
% chat What can I help you with?
siri:W**t can I help *** with?
% unknown:Who is the best Taiwan vtuber?
% chat Mochikomame
siri:Mochikomame
% unknown:wo **o xing fen!!!!
% exit
Bye, siri.
```
### message format (version 1) V.S. message format (version 2)
[\[Download\] example1.pcapng](https://drive.google.com/file/d/1aP8f2dRtJh3cgpK0EyjldGyP7bBo8n5C/view?usp=sharing)
```
********************************
** Welcome to the BBS server. **
********************************
% register t t
Register successfully.
% login t t
Welcome, t.
% enter-chat-room 12345 1
Welcome to public chat room.
Port:12345
Version:1
% chat hello world
t:hello w**ld
% enter-chat-room 23456 2
Welcome to public chat room.
Port:23456
Version:2
t:hello w**ld
% chat hello
t:hello
% exit
Bye, t.
```
1. message format(version 1)
    **`chat hello world`**
    ![](https://i.imgur.com/AAlo7RN.png)


3. message format(version 2)
4. 
    **`chat hello`**
    ![](https://i.imgur.com/j2IYkuR.png)
 

### Enter chat room
[[Download] example2.pcapng](https://drive.google.com/file/d/19ggtB0GnTNoUwcO5heWU7m0mA0MO8Z0A/view?usp=sharing)
TA1 (TA1 enter chat room after "chat three")
```
********************************
** Welcome to the BBS server. **
********************************
% register TA1 TA1
Register successfully.
% login TA1 TA1
Welcome, TA1.
% enter-chat-room 12345 2
Welcome to public chat room.
Port:12345
Version:2
TA2:one
TA2:two
TA2:three
% exit
Bye, TA1.
```
TA2
```
********************************
** Welcome to the BBS server. **
********************************
% register TA1 TA1
Username is already used.
% register TA2 TA2
Register successfully.
% login TA2 TA2
Welcome, TA2.
% enter-chat-room 12334 1
Welcome to public chat room.
Port:12334
Version:1
% chat one
TA2:one
% chat two
TA2:two
% chat three
TA2:three
% exit
Bye, TA2.
```
Response of "enter-chat-room 12345 2"
![](https://i.imgur.com/Ai5uix5.png)



## Build and Execute
This section explains how to use the Docker environment we provide to build and test your homework. Noted that TA will use the same enviroment and scripts to grade your homework, so make sure your code can be compiled in the environment.

### Project Structure
Below is the overiew of the whole project.
```
.
├── docker
│   ├── docker-compose.yml
│   └── Dockerfile
├── Makefile
├── src
│   ├── hw3
│   ├── hw3.c / hw3.cpp
│   ├── Makefile
│   └── (any other c/c++ files if needed)
└── test
    ├── advance
    ├── basic
    │   ├── output
    │   │   ├── 1-register
    │   │   ├── 1-register.diff
    │   │   └── ...
    │   ├── solution
    │   │   ├── 1-register
    │   │   └── ...
    │   └── test-cases
    │       ├── 1-register
    │       └── ...
    ├── client.py
    ├── Makefile
    └── test.py
```

There are three folders at the root of the project:

| File      | Description                                                                                                                                                                                                            |
| --------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `docker/` | Files in the folder define the Docker environment for this homework.                                                                                                                                                   |
| `src/`    | This is the folder containing **server source code**, `Makefile` and the built server binary. Also it is the folder you should submit to E3. More details is described in [Homework Submission](#Homework-Submission). |
| `test/`   | This is the folder for testing. Details are described below.                                                                                                                                                           |

### `test/` folder


| File                | Description                                                                                                                   |
| ------------------- | ----------------------------------------------------------------------------------------------------------------------------- |
| `client.py`         | The client which can interact with server.                                                                                    |
| `test.py`           | The python script for testing.                                                                                                |
| `basic/`            | The folder containing basic test cases.                                                                                       |
| `advance/`          | The folder containing advance test cases.                                                                                     |
| `basic/output/`     | After testing, the standard output of the first client and the result of the `diff` command will be stored in this folder.    |
| `basic/solution/`   | The solution to the test cases.                                                                                               |
| `basic/test-cases/` | The test cases are stored here. The format of theses files is described in the [Test Case Format](#Test-Case-Format) section. |



### Command Usage
This is the simple show case of the commands, and the details will be described in the following sections.
- Activate Docker environment：`make docker` or `sudo make docker`（if you don't have enough privilege）
- Build Project：`make`
- Test：`make test`

:::info
:bulb: You should execute these commands at the root of the project.
:::

### Activate Docker
> make docker 

We use `docker` and `docker-compose` to build up the enviroment, so make sure you have those two things installed on your computer. If it is the first time you build the environment, it may takes quite a long time for downloading and installing.

When everything is done, you will get a shell of the docker environment.

![](https://i.imgur.com/oolA980.png)

`/project` is the path to the root of the project, and data inside is the **same** as those outside the Docker environment. This means you can write your codes outside then build and test inside the Docker environment.

If you want to quit the shell, simply use `exit` command.

### Build Project
> make 

This command will compile your server code (`src/hw3.c` or `src/hw3.cpp`) to `test/hw3`

### Test Project
> make test

We will simply build and test your project (in the Docker environment) by these stpes:
```bash=
make
make test
```

After doing that, the score shown on the screen will be your final grade. Below is the example:

![](https://i.imgur.com/TH7N2eG.png)

### Test Case Format

Below is the format of test case files. The first line contains **one** integer number indicate the `<number_of_client>`. After that, there will be several lines indicating the commands sent from client（or server）by steps. Each line contains `<id>` and `<command>` **seperated by one space**. `<id>` is an integer number: `0` means `server`, `1` means the first client, and so on.

```=
<number_of_client>
<id> <command>
<id> <command>
...
```

#### Example:
```=
2
1 register first_client p1
2 register second_client p2
1 login first_client p1
1 login second_client p2
1 exit
2 exit
```

## Homework Submission
Please rename your `src/` to `{id}_hw3/` then pack your C/C++ code and `Makefile` into a zip archive. The directory structure should follow the below illustration. The `id` is your student id. Please note that you don't need to enclose your id with the braces.

```bash
{id}_hw3.zip
└── {id}_hw3/  # This is your `src/` folder
    ├── hw3.c / hw3.cpp
    ├── Makefile
    └── (any other c/c++ files if needed)
```

For example, if your student id is `310551xxx`, then:
1. Change your `src/` folder name to `310551xxx_hw3/`
2. Be aware that there are C/C++ code and `Makefile` in the folder
3. Zip `310551xxx_hw3/`, and the whole folder structure will be:
    <br>
    ```bash
    310551xxx_hw3.zip
    └── 310551xxx_hw3/  # This is your `src/` folder
        ├── hw3.c / hw3.cpp
        ├── Makefile
        └── (any other c/c++ files if needed)
    ```

:::warning
:warning: Unlike HW1 and HW2, both C/C++ code and Makefile is **mandatory**. It's welcome to modify Makefile to whatever you like. However it's also your duty to make sure the compilation and testing won't fail.
:::

:::warning
:warning: Noted that even if you use the default Makefile, it's also **necessary** to submit the file to E3.
:::

## Scoring Rules
As mentioned above, TA will use `make docker` to generate a Docker environment and use the commands below to test your project. The score shown on the terminal will be your grade.
```bash=
make
make test
```

TA will test your homework **3 times**, and the **highest socre** will be your final grade.

### Test Cases
There will be 7 **basic** and 5 **advance** test cases in our test. 

The basic test cases are open-sourced. The first four basic cases is `5` points for each cases, and the others are `10` points.

The advance test cases are not published. There are five test cases, and `10` points for each testcase.

We use the `diff` command to test the difference between your `first client output`(the client with id equals to 1) and the solution. If they are the same, you will get all point of the test case. Otherwise, you will get no point.

### Other Rules
- You should submit **both** **`Makefile`** and your C/C++ code. Any missing files will make your final grade <span class='qq'>0</span>.
- Be aware that if the **`make`** process fails or the executable **`hw3`** is not found, you will get <span class='qq'>0 point</span>.
- No copycats. Please do not use codes from others (even open source projects). Otherwise, you'll get <span class='qq'>0 point</span>.
- You have to set you TCP socket to **`SO_REUSEADDR`**. Otherwise, we will <span class='qq'>take 10 points off</span> from your final grade.
- Your should use **`select`**, **`poll`** or **`epoll`** to complete your implementation. Otherwire,  we will <span class='qq'>take 20 points off</span> from your final grade.
- We will <span class='qq'>take 5 points off</span> from your final grade if your homework directory structure or folder name is wrong.

## Remarks
* Please implement your homework in **C** or **C++**.
* Using any non-standard libraries and any external binaries (e.g., via `system()` or other functions that will launch a process) are not allowed.
* Our test program will run on localhost environment, so you can assume that UDP  package drop and disorder won't occur.
* The maximum number of  TCP connection is 10.
* There is only one public chat room on the server, and every messages sent from clients should be saved into chat history.
* Every **message** appear in our specification should not be considered as **UDP packet** or **TCP packet**. The **message** here specify to the **\<message>** of instruction [chat](#UDP).


<style>
.hr {
    width: 100%;
    border: 2px solid black;
    text-align: center;
    padding-top: 7px;
    padding-bottom: 7px;
    background-color: #FFCE44;
    margin-top: 45px;
    margin-bottom: 15px;
    font-weight: 600;
}
h5 {
    color: red;
    font-weight: 900;
    font-size: 16px;
    font-family: mono;
}
.qq {
    color: red;
    font-weight: bold;
}
h5::before {
    content: '[TODO] '
}
h3::before {
    content: '# ';
}
</style>
    

    