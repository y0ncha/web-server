# TCP Non-Blocking Single-Threaded Async HTTP Server

Always reference these instructions first and fallback to search or bash commands only when you encounter unexpected information that does not match the info here.

This is a C++ educational project implementing a TCP non-blocking, single-threaded, asynchronous HTTP server using Windows Sockets (Winsock2). The project is designed for understanding transport protocols and asynchronous I/O concepts.

## Working Effectively

### Platform Requirements
- **CRITICAL**: This project is Windows-specific and uses Winsock2 APIs
- **Cannot build on Linux/macOS** without cross-compilation tools
- Original design targets Visual Studio on Windows
- **CRITICAL** When asked to document or comment use coding format guilinelines below and do not change code logic.

### Cross-Platform Development Setup
For development on Linux (required for this environment):
- Install mingw-w64 cross-compiler: `sudo apt-get update && sudo apt-get install -y mingw-w64`
- **NEVER CANCEL**: mingw-w64 installation takes 10-15 seconds. Set timeout to 60+ seconds.

### Building the Project
- **Primary build method (Linux cross-compilation)**:
  ```bash
  cd /home/runner/work/web-server/web-server
  x86_64-w64-mingw32-g++ *.cpp -lws2_32 -o web-server.exe
  ```
  - **Build time**: ~7 seconds. NEVER CANCEL - set timeout to 30+ seconds.
  - **Output**: `web-server.exe` (Windows PE32+ executable)

- **Windows build method** (if on Windows with Visual Studio):
  - Open `web-server.sln` in Visual Studio
  - Build solution (Ctrl+Shift+B)
  - **NEVER CANCEL**: Visual Studio build takes 2-5 minutes. Set timeout to 10+ minutes.

### Running the Application
- **Cannot run directly on Linux** - produces Windows executable only
- **Windows execution**: Run `web-server.exe` - starts HTTP server on localhost:8080
- **Default configuration**: IP 127.0.0.1, Port 8080 (hardcoded in main.cpp)

### Testing and Validation
- **No automated tests exist** - this is an educational project
- **Manual validation required**: Build successfully, check executable creation
- **Cannot perform runtime testing** on Linux environment
- **Windows validation scenario**: 
  1. Run `web-server.exe`
  2. Open browser to `http://localhost:8080`
  3. Verify server responds with basic HTTP responses
  4. Test with curl: `curl http://localhost:8080/` should return response

### Code Structure and Navigation

#### Key Files (in order of importance):
1. **main.cpp** - Entry point, creates Server instance on 127.0.0.1:8080
2. **server.cpp/.h** - Core Server class with async event loop
3. **client.cpp/.h** - Client connection state management  
4. **request.cpp/.h** - HTTP request parsing
5. **response.cpp/.h** - HTTP response generation
6. **http_utils.cpp/.h** - HTTP utility functions
7. **utils.cpp/.h** - General utility functions (trim, etc.)

#### Core Architecture:
- **Single-threaded event loop** using select() for I/O multiplexing
- **State machine for clients**: AwaitingRequest → RequestBuffered → ResponseReady → Completed
- **Non-blocking sockets** with Winsock2 APIs
- **Minimal HTTP/1.1 implementation** for educational purposes

#### Important Code Paths:
- `Server::run()` - Main event loop in server.cpp
- `Server::acceptConnection()` - Handles new client connections
- `Server::processClients()` - Processes all client events
- `Client::setRequestBuffered()` - Transitions client state after receiving data
- `Request::Request()` - Parses raw HTTP request strings
- `Response::toString()` - Converts response object to HTTP string

### Development Workflow
1. **Always build first** to verify changes don't break compilation
2. **Code changes**: Focus on .cpp/.h files, avoid modifying .vcxproj unless necessary
3. **Testing approach**: Build validation only (no runtime tests possible on Linux)
4. **Windows-specific code**: Be aware of Winsock APIs, SOCKET types, WSA functions

### Common Issues and Workarounds
- **Missing includes**: If build fails with undefined symbols, check for missing `#include <ctime>` or similar headers
- **Windows-only APIs**: Do not attempt to replace Winsock calls with POSIX equivalents - this changes the project's educational purpose
- **Build dependencies**: Only requires standard C++ library and Winsock2 (included with mingw-w64)
- **Library linking**: The `-lws2_32` flag is **required** - build will fail with undefined Winsock references without it
- **Regular g++**: Will fail immediately due to `#include <winsock2.h>` - must use mingw cross-compiler

### Project Limitations and Simplifications
- **Educational purpose only** - not for production use
- **Windows networking only** - uses Winsock2 exclusively  
- **No cross-platform support** by design
- **Minimal HTTP implementation** - basic GET requests only
- **Single-recv assumption**: Code assumes complete HTTP request fits in one recv() call (simplified for educational purposes)
- **No SSL/TLS support**
- **Hardcoded configuration**: IP (127.0.0.1), Port (8080), buffer sizes (1024 bytes), timeouts (120 seconds)

### File Modification Guidelines
- **Never modify** .vcxproj/.sln files unless adding/removing source files
- **Core logic** primarily in server.cpp (lines 130-282 contain main functionality)
- **HTTP parsing** in request.cpp - handles query parameters and headers
- **Response building** in response.cpp - creates proper HTTP responses
- **Client state management** in client.cpp - tracks connection lifecycle

### Build Validation Commands
Run these commands to validate your changes:
```bash
# Verify mingw-w64 is installed
x86_64-w64-mingw32-g++ --version

# Clean and build (always run from project root)
cd /home/runner/work/web-server/web-server
rm -f web-server.exe
x86_64-w64-mingw32-g++ *.cpp -lws2_32 -o web-server.exe

# Verify executable creation
ls -la web-server.exe
file web-server.exe
```

Expected output: PE32+ executable for MS Windows, approximately 390KB in size.

### Troubleshooting Build Issues
- **"x86_64-w64-mingw32-g++: command not found"**: Run `sudo apt-get install -y mingw-w64`
- **"undefined reference to `__imp_WSAStartup`"**: Missing `-lws2_32` linker flag
- **"winsock2.h: No such file or directory"**: Trying to use regular g++ instead of mingw cross-compiler
- **"undefined reference to `time`"**: Missing `#include <ctime>` in client.cpp (add it after `#include "client.h"`)
- **Build time longer than expected**: Normal for first build, subsequent builds are faster due to caching

### Repository Structure Quick Reference
```
/home/runner/work/web-server/web-server/
├── main.cpp           # Entry point (127.0.0.1:8080)
├── server.cpp/.h      # Main server class with event loop
├── client.cpp/.h      # Client connection management
├── request.cpp/.h     # HTTP request parsing
├── response.cpp/.h    # HTTP response generation  
├── http_utils.cpp/.h  # HTTP utility functions
├── utils.cpp/.h       # General utilities
├── web-server.sln     # Visual Studio solution
├── web-server.vcxproj # Visual Studio project
└── README.md          # Project documentation
```

**Remember**: This is Windows-specific educational code. Do not attempt to port to POSIX sockets as it changes the learning objectives around Windows networking APIs.


## Code Formatting Guidelines

To ensure readability, maintainability, and consistency across all source files, follow these conventions throughout the project:

### Naming Conventions

| Element            | Format           | Example                          |
|--------------------|------------------|----------------------------------|
| Classes / Structs  | PascalCase       | HttpRequest, Client              |
| Functions          | camelCase        | handleFileRequest(), resolveFilePath() |
| Member Variables   | camelCase        | clientSocket, inBuffer           |
| Local Variables    | camelCase        | filePath, lang                   |
| Constants / Macros | UPPER_SNAKE_CASE | MAX_TIMEOUT, RECV_BUFFER_SIZE    |
| Filenames          | snake_case       | http_utils.cpp, client_state.h   |

### Indentation & Bracing
- Use 4 spaces per indentation level (no tabs)
- Always use braces, even for one-line bodies
- Style:
```bash
  if (condition) {
	// ...
  } 
  else {
	// ...
  }
```

### Spacing
- One space between keywords and parentheses: if (x > 0)
- No trailing whitespace
- Leave blank lines between logical blocks

### Comments and Documentation
- Use //... style above declarations (especially in .h files)
- Use /** ... */ block format for function and classes definitions:
	  /**
	   * @breif Sends an HTTP response to the client
	   * @param clientSocket The socket to respond on
	   * @returns true if success, false otherwise
	   */
	   bool sendResponse(int clientSocket) { ... }
- Use consistent style:
	- // for inline or short explanatory comments
	- /** ... */ for “nutshell-style” explanations

### General Best Practices
- Include only necessary headers
- Keep headers lean: only declare what’s needed
- Keep functions short and focused
- Group related helpers and declarations together

