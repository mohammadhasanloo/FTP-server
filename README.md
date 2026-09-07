# FTP Server

A simplified FTP server and client in C++, using the two-channel design the real
protocol uses: a command channel carrying requests and responses, and a separate
data channel carrying file contents.

## Requirements

A C++ compiler and `make`. POSIX sockets, so Linux or macOS.

## Building

```bash
cd Code/server && make
cd ../client && make
```

Produces `server_side.out` and `client.out`.

## Running

Start the server, then the client in another terminal:

```bash
./Code/server/server_side.out
./Code/client/client.out
```

Users, passwords, ports and per-user permissions are read from
`Code/config.json`. `Code/server/help.txt` lists the commands the server accepts.

## The two channels

FTP separates control from content. Commands and their numeric responses travel
on one connection; a file transfer opens a second. Keeping them apart means a
transfer in progress does not block the command stream, so the client can still
be spoken to while data is moving.

Both sockets are bound and listened on at startup, and each accepted client is
tracked with its own authentication state.

## Features

| area | behaviour |
| --- | --- |
| authentication | Username then password, checked against the config file |
| authorisation | Per-user file access, so a user sees only what is theirs |
| transfer | Upload and download over the data channel |
| management | List, rename and delete, subject to permission |
| logging | Every command and response recorded with its outcome |

## Project structure

```
Code/
    config.json           users, ports and permissions
    server/
        server.{h,cpp}    sockets, accept loop, session state
        FTP.{h,cpp}       command parsing and the protocol responses
        user.{h,cpp}      accounts and permission checks
        JsonParser.{h,cpp} configuration reading
        LoggerHandler.{h,cpp} the activity log
        help.txt          the command reference the server serves
        main.cpp          entry point
        makefile
    client/
        client.{h,cpp}    connection, session and command entry
        JsonParser.{h,cpp} configuration reading
        main.cpp          entry point
        makefile
```

## Components

| unit | responsibility |
| --- | --- |
| `Server` | Owns both sockets, accepts clients, dispatches their commands |
| `FTP` | Turns a command line into an action and a numeric response |
| `User` | Accounts, credentials, and what each user may reach |
| `JsonParser` | Reads the configuration without a third-party library |
| `LoggerHandler` | Appends every exchange to the log |
| `Client` | Connects, authenticates, and drives the two channels |
