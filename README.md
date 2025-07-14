# Multithreaded TCP Chat Server and Client in C

A **terminal-based chat application** implemented in C using sockets and POSIX threads.  
Supports multiple clients connecting simultaneously to a server and exchanging messages in real-time over TCP.  
Ideal for learning network programming and concurrency concepts in C.

---

## Current Implementation

- Server-Proxy-Client architecture with message forwarding through a proxy.
- Multithreaded server handles multiple clients concurrently.
- Basic text-based messaging commands supported.

---

## Roadmap & Planned Features

1. **Authentication & Authorization**  
   - User login with username/password.  
   - Access control and command restrictions.

2. **Encryption & Security**  
   - TLS/SSL encrypted connections between client, proxy, and server.  
   - Protection against man-in-the-middle attacks.

3. **Logging & Monitoring**  
   - Connection and command logging on server and proxy.  
   - Error tracking and suspicious activity detection.

4. **Load Balancing**  
   - Proxy distributes requests among multiple backend servers.

5. **Caching**  
   - Proxy caches responses to speed up repeated requests.

6. **Improved Concurrency**  
   - Use of `select()`, async I/O, or thread pools for scalable multi-client support.

7. **Protocol Enhancements**  
   - Custom protocol with extended commands and better robustness.

8. **Health Checks & Auto-Restart**  
   - Automatic monitoring and restart of server/proxy components.

9. **Configurable Settings**  
   - Use of config files for ports, IPs, and other parameters instead of hardcoded values.

10. **Web Interface / Dashboard**  
    - Simple web UI for monitoring, logging, and command control.

---

## How to Compile and Run

(coming soon...)
