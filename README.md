# TCP Non-Blocking Single-Threaded Async HTTP Server

This repository contains a C++ implementation of a TCP non-blocking, single-threaded, asynchronous HTTP server.  
It is developed as a Computer Networks course assignment by [y0ncha](https://github.com/y0ncha), with the primary objective being a **deep conceptual understanding** of transport protocols and asynchronous I/O—not real-world or hands-on deployment.

## Objectives

- Gain in-depth knowledge of TCP, including connection management and socket operations in C++.
- Understand non-blocking I/O and how it enables a single-threaded server to handle multiple clients.
- Explore asynchronous event-driven server architectures using techniques such as `select`, `poll`, or `epoll`.
- Learn the basics of HTTP request parsing and response generation.

## Features

- Event loop for managing multiple client connections in one thread
- Non-blocking TCP socket operations
- Asynchronous handling of HTTP requests and responses
- Minimal HTTP protocol implementation
- Clean, educational C++ codebase

## Educational Focus

This project is intended to help students develop a **deep theoretical understanding** of transport-layer protocols (specifically TCP) and asynchronous server design in C++.  
It is **not** meant as a real-world application or for hands-on deployment.

## License

See [LICENSE](LICENSE) for details.

---

For questions, feedback, or contributions, please open an issue or submit a pull request.
