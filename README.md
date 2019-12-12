## File transfer using socket programming in C
UDP Protocol has been implemented to transfer files from server to client.

## Buffering Mechanism
Buffers of size **1024** has been used to transfer the datagrams. 
Appropriate delays have been set to minimize the drop in packets. 

## Compiling and running instructions
Make sure that the ports are specified properly in both server and client programme.   
Any port number except 1-1023 can be used. (Make sure that ports in server and client are same if running on same machine). 
- Server:   
Compiling: `gcc server.c -o server`   
Running: `./server`   
File to be transferred: Mention file name in `sendFile()` function in server.
- Client:    
Compiling: `gcc client.c -o client`   
Running: `./client IP`   
`IP`: Mention `0.0.0.0` or `127.0.0.1` if running on same machine else mention the server IP Address. 
