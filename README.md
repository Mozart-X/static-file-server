# A static web server, from scratch in C

A tiny HTTP/1.1 static file server written by hand in C using the Windows socket
API (Winsock) — no frameworks, no libraries beyond `ws2_32`. It serves a small
static site (`index.html` + `style.css`) over the network, the same core job that
nginx and Apache do behind all their features.

Built as a learning project to actually *see* how a backend works at the byte level:
ports, sockets, the HTTP request/response format, and graceful connection handling.

## What it does

- Opens a TCP socket, binds to port **9000**, and listens for connections.
- For each incoming request:
  - reads the raw HTTP request off the socket,
  - parses the request line to find which file is being asked for,
  - maps `/` to `index.html` (the directory-index convention),
  - reads that file off disk as raw bytes,
  - stamps the correct `Content-Type` from the file extension (`.html` → `text/html`, `.css` → `text/css`),
  - sends back a proper HTTP response (status line + headers + body),
  - returns **404 Not Found** for files that don't exist (instead of crashing),
  - closes the connection cleanly.

## The interesting bits I learned

- **A "port" isn't hardware** — it's just a number the OS uses to route network
  traffic to the right program.
- **HTTP is just text** — a request and a response, with a mandatory blank line
  (`\r\n\r\n`) separating headers from body.
- **`Content-Length` and `Content-Type`** have to be computed and sent by the server;
  the body is sent as raw bytes counted by file **size**, not `strlen` (which would
  truncate at a zero byte).
- **The TCP close race** — closing a socket with unread data, or without
  `Connection: close`, causes intermittent "connection reset" errors. Fixed with
  `recv` (drain the request), the `Connection: close` header, and
  `shutdown()` before `closesocket()`.

## Build

Requires a C compiler with Winsock (MinGW/MSYS2 `gcc` on Windows):

```
gcc webserver.c -o webserver.exe -lws2_32
```

## Run

```
./webserver.exe
```

Then open <http://localhost:9000> in a browser.

## Files

| File | What it is |
|------|------------|
| `webserver.c` | the server (the whole project) |
| `index.html` | the page being served |
| `style.css` | its styling |
| `BLUEPRINT.md` | design notes / build plan |

## Roadmap

- [ ] Serve more file types (JS, images) by extending the Content-Type table
- [ ] A reverse proxy in front of it (forward `:8080` → `:9000`)
- [ ] One thread per connection for true concurrency
