# Key Value in-memory store in c

A small in-memory key/value server written in C, built from scratch as a learning project.

The goal is to understand how databases and network servers work under the hood, starting with the basics and gradually adding more features.

Right now, the server is single-threaded, handles one client at a time, and stores everything in memory.

## Features

- TCP server using sockets
- In-memory key/value storage
- Basic commands: `SET`, `GET`, `DELETE`, `PING`, `QUIT`
- Support for values containing spaces
- Simple text-based protocol
- Customizable listening port

## Getting Started

### Requirements

- C compiler (GCC or Clang)
- Make
- Netcat (`nc`) for testing
- Unix-like environment (Linux/macOS)

### Build

```bash
make
```

### Run

Start the server on the default port (`7331`):

```bash
./server
```

Or specify a custom port:

```bash
./server 9000
```

### Connect

Open another terminal and connect using Netcat:

```bash
nc 127.0.0.1 7331
```

You can now send commands directly to the server.

## Usage

```text
SET name bob
OK

GET name
bob

SET msg hello there
OK

GET msg
hello there

DELETE msg
DELETED

GET msg
NOT_FOUND

PING
PONG

QUIT
BYE
```

## Supported Commands

| Command         | Description                 | Response                 |
| --------------- | --------------------------- | ------------------------ |
| `SET key value` | Store or update a value     | `OK`                     |
| `GET key`       | Retrieve a value            | Value or `NOT_FOUND`     |
| `DELETE key`    | Remove a key                | `DELETED` or `NOT_FOUND` |
| `PING`          | Check server responsiveness | `PONG`                   |
| `QUIT`          | Close the connection        | `BYE`                    |

Unknown or invalid commands return an `ERR ...` response.

### Protocol

The server uses a simple line-based text protocol.

- Commands are separated by newlines.
- Values can contain spaces.
- Values cannot contain newlines.
- Every response ends with `\n`.

## Current Limitations

This is an early-stage project, so there are a few intentional limitations:

- **Single client:** Only one client is handled at a time.
- **No persistence:** All data is lost when the server stops.
- **Single-threaded:** No multithreading or asynchronous I/O yet.
- **Simple protocol:** No binary data or Redis protocol compatibility.
- **Limited commands:** Only basic key/value operations are supported.

## Future prospects

- Add multiple clients
- Add a WAL as a form of persistence with crash recovery
