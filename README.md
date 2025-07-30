# 🧵 Multithreaded Client-Server Chat Application  
This project implements a multi-client chat system in C using POSIX threads and TCP sockets on a Unix/Linux environment. Each client is handled by a dedicated thread on the server, allowing concurrent private messaging between connected users.  
  
🚀 Features:  
- Multi-client support with thread-per-client model  
- Unique ID assignment to each user  
- Real-time session-based private messaging  
- Command-based interactions:  
  - `~list` – Show available users  
  - `~connect_to_XX` – Connect to user with ID XX  
  - `~my_id` – Display your user ID  
  - `~stop` – End current session  
  - `~quit` – Exit the server  
  
🛠️ Compilation:  
```bash  
gcc server.c -o server -lpthread  
```  
  
▶️ Running the Server:  
```bash  
./server <PORT>  
# Example:  
./server 5000  
```  
  
🧪 Testing with Clients:  
Use multiple `telnet` instances to simulate clients:  
```bash  
telnet localhost <PORT>  
# Example:  
telnet localhost 5000  
```  
  
📝 Example Usage:  
1. Connect multiple terminals to the server using `telnet`  
2. Use `~list` to view available users  
3. Use `~connect_to_02` to start a session with user ID 2  
4. Type any message to chat with the connected user  
5. Use `~stop` to end the session, `~quit` to disconnect  
Messages are relayed with sender ID like `01: Hello`.  
  
👨‍💻 Author:  
**Krishna Kumar Kondooru**  
- [GitHub](https://github.com/KrishnakumarKPK)  
- [LinkedIn](https://www.linkedin.com/in/krishna-kumar-kondooru-731044256/)  
