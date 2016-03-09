#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <fstream>
#include <thread>

#include <errno.h>
#include <string.h>
#include <vector>


//std::string pathServer;

void procRequest(int Socket, std::string pathServer)
{
    /*
    std::cout << std::endl << "thread = 0x" << std::hex << std::this_thread::get_id()
              << std::dec << " socket = " << Socket << std::endl;
    */

    /*
    std::ofstream file("server.log");
    file << std::cout << "thread = 0x" << std::hex << std::this_thread::get_id()
         << std::dec << " socket = " << Socket << std::endl;
    */
    


    // recieve data
    // ===============================
    const unsigned  int size_data = 1024 * 10;
    char buffer[size_data];
    int n = recv(Socket, buffer, size_data, 0);
    if( n <= 0)
        return;



    // parse request
    // ===============================
    std::string strBuff(buffer);
    //std::cout << "request: " << strBuff << std::endl;

    std::string result;
    if(strBuff.find("GET") == 0){
        result = strBuff.substr(3, strBuff.find("HTTP") - 3);
        size_t first = result.find_first_not_of(' ');
        size_t last  = result.find_last_not_of(' ');
        if(first == last){
            result = "";
        }
        else{
            result = result.substr(first, (last-first+1));
            for (auto it = result.begin(); it < result.end(); it++)
                if ( *it == '?' )
                     *it = '\0';
        }
    }



    result = pathServer + result;
   // std::cout << "request: " << result << std::endl;


    //send response
    // ===============================
    auto fd = fopen(result.c_str(),"rb");
    if (fd){
        fseek(fd, 0, SEEK_END);
        auto size = ftell( fd );
        fseek(fd, 0, SEEK_SET);

        std::vector<char> buff;
        buff.resize(size);
        fread( buff.data(), size, 1,fd);
        fclose( fd );

        std::string headRes = std::string("HTTP/1.0 200 OK\x0D\x0A") +
                "Content-Type: text/html\x0D\x0A" +
                "Content-Length: "+ std::to_string(buff.size()) +"\x0D\x0A" +
                "\x0D\x0A";
        write(Socket, headRes.c_str(), headRes.size());
        write(Socket, buff.data(), buff.size());

    }
    else{
        char resp[] =  "HTTP/1.0 404 NOT FOUND\x0D\x0A"
                            "Content-Length: 0\x0D\x0A"
                            "Content-Type: text/html\x0D\x0A\x0D\x0A";
        size_t sz_resp = write(Socket, resp, sizeof(resp));
//        std::cout << "send data: " << sz_resp << std::endl;
    }



    // close connection
    // ===============================
    shutdown( Socket, SHUT_RDWR);
    close( Socket );

    //file.close();

}



int main(int argc, char *argv[])
{
    //std::cout << "main thread: 0x" << std::hex << std::this_thread::get_id() << std::endl;

    char* opts = "h:p:d:";          // доступные опции, каждая принимает аргумент
    int port;                       // number of port
    char* host;                     // host name
    char* dir;                      // directory

    int opt;                        // каждая следующая опция попадает сюда
    while((opt = getopt(argc, argv, opts)) != -1) { // вызываем getopt пока она не вернет -1
        switch(opt) {
        case 'h':
            host = optarg;
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 'd':
            dir = optarg;
            break;
        }
    }

    /*
    if (1) {                        // print parameters
        printf("parametrs:\n");
        printf("\t host = %s\n", host);
        printf("\t port = %d\n", port);
        printf("\t dir  = %s\n", dir);
    }
    */

    pid_t par_pid = getpid();
    //printf("Parant PID = %d\n", par_pid);

    pid_t ch_pid = fork();
    if( !ch_pid )                   // child process
    {
        //printf("Child PID = %i (child process).\n", getpid());

        std::string pathServer = dir;
        //std::cout << "path to server: " << pathServer << std::endl;
        int state = chdir (dir);
        //std::cout << "state directory changing:" << state << std::endl;
        if( !state ){
          //  std::cout << "directory changed" << std::endl;
        }
        else
            return 0;


        // open socket
        int MasterSocket = socket(AF_INET,              // IPv4
                                 SOCK_STREAM,           // TCP
                                 IPPROTO_TCP);
        //std::cout << "master socket = " << MasterSocket << std::endl;

        struct in_addr addr;
        inet_aton( host, &addr);
        struct sockaddr_in SockAddr;
        SockAddr.sin_family = AF_INET;
        SockAddr.sin_port   = htons( port );
        SockAddr.sin_addr   = addr;
        //SockAddr.sin_addr.s_addr = htons( INADDR_ANY );


        SO_REUSEADDR;

        int st_bind = bind(MasterSocket, (struct sockaddr *)(&SockAddr), sizeof(SockAddr));
        //std::cout << "[error | bind] " << strerror( errno ) << std::endl;
        int st_listen = listen(MasterSocket, SOMAXCONN);
        //std::cout << "state bind: " << st_bind << std::endl;
        //std::cout << "state listen: " << st_listen << std::endl;

        // loop connects
        while(1){
            //std::cout << "wait connection ..." << std::endl;

            int Socket = accept(MasterSocket, 0, 0);
            //std::cout << "socket = " << Socket << std::endl;

            // run thread of request proccesing
            std::thread thrProccesReq(procRequest, Socket, pathServer);
            //thrProccesReq.join();
            thrProccesReq.detach();

            //sleep(15);
            //close( MasterSocket );
            //return 0;

        }

        sleep(15);
        // close socket
        close( MasterSocket );


        //printf("Process (PID = %d) finished.\n", getpid());
    }


    printf("Process (PID = %d) finished.\n", getpid());
    return 0;
}
