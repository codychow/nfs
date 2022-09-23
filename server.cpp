#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "FileSys.h"
using namespace std;

#define LINE_END "\\r\\n"
#define MAX_FILE_SIZE 7680
#define MAX_RESPONSE_SIZE 29

void* get_in_addr(struct sockaddr* s);
short hashstr(const string& str);

enum command_code {
    mkdir_num,
    ls_num,
    cd_num,
    home_num,
    rmdir_num,
    create_num,
    append_num,
    stat_num,
    cat_num,
    head_num,
    rm_num
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: ./nfsserver port#\n";
        return -1;
    }
    const int MAX_SIZE = 7680 + 19;
    string port = argv[1];

    //networking part: create the socket and accept the client connection
    int sock, client_sock; //change this line when necessary!
    struct addrinfo hints, * servinfo, * p;
    struct sockaddr_storage client_addr;
    socklen_t sin_size;
    char s[INET6_ADDRSTRLEN];
    int rv;
    int yes = 1;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    rv = getaddrinfo(NULL, port.c_str(), &hints, &servinfo);
    if (rv != 0) {
        fprintf(stderr, "error with getaddrinfo\n");
        exit(1);
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sock == -1) {
            perror("server: socket");
            continue;
        }

        if (bind(sock, p->ai_addr, p->ai_addrlen) == -1) {
            close(sock);
            perror("server: bind");
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL) {
        fprintf(stderr, "Could not bind socket\n");
        exit(1);
    }

    if (listen(sock, 1) == -1) {
        fprintf(stderr, "Listen() error\n");
        exit(1);
    }

    cout << "Waiting for client...\n";

    sin_size = sizeof(client_addr);
    client_sock = accept(sock, (struct sockaddr*)&client_addr, &sin_size);
    if (client_sock == -1) {
        perror("accept");
        exit(1);
    }

    //close the listening socket
    close(sock);

    inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr*)&client_addr), s, sizeof s);
    cout << "Got connection from " << s << endl;

    //mount the file system
    FileSys fs;
    fs.mount(client_sock); //assume that sock is the new socket created 
                    //for a TCP connection between the client and the server

    //loop: get the command from the client and invoke the file
    //system operation which returns the results or error messages back to the clinet
    //until the client closes the TCP connection.
    char buf[MAX_SIZE];
    int bytes_recv, bytes_sent;
    while (1) {
        memset(buf, 0, MAX_SIZE);
        bytes_recv = recv(client_sock, buf, MAX_SIZE, 0);
        if (bytes_recv == -1) {
            perror("read");
            break;
        }
        if (bytes_recv == 0) {
            cout << "Connection terminated by client\n";
            break;
        }

        string cmd = buf;
        string parsed_cmd;

        if(cmd.find(" ") == string::npos)
            parsed_cmd = cmd.substr(0, cmd.length() - 4);
        else
            parsed_cmd = cmd.substr(0, cmd.find(" ")); 
        string filename, data;
       
        switch (hashstr(parsed_cmd)) {
        case mkdir_num:
            filename = buf;
            filename = filename.substr(filename.find(" ") + 1);
            filename = filename.substr(0, filename.find('\\'));
            fs.mkdir(filename.c_str());
            break;
        case ls_num:
            fs.ls();
            break;
        case cd_num:
            filename = buf;
            filename = filename.substr(filename.find(" ") + 1);
            filename = filename.substr(0, filename.find('\\'));
            fs.cd(filename.c_str());
            break;
        case home_num:
            fs.home();
            break;
        case rmdir_num:
            filename = buf;
            filename = filename.substr(filename.find(" ") + 1);
            filename = filename.substr(0, filename.find('\\'));
            fs.rmdir(filename.c_str());
            break;
        case create_num:
            filename = buf;
            filename = filename.substr(filename.find(" ") + 1);
            filename = filename.substr(0, filename.find('\\'));
            fs.create(filename.c_str());
            break;
        case append_num:
            filename = buf;
            filename = filename.substr(filename.find(" ") + 1);
            data = filename.substr(filename.find(" ") + 1);
            data = data.substr(0, data.find('\\'));
            filename = filename.substr(0, filename.find(" "));
            fs.append(filename.c_str(), data.c_str());
            break;
        case stat_num:
            filename = buf;
            filename = filename.substr(filename.find(" ") + 1);
            filename = filename.substr(0, filename.find('\\'));
            fs.stat(filename.c_str());
            break;
        case cat_num:
            filename = buf;
            filename = filename.substr(filename.find(" ") + 1);
            filename = filename.substr(0, filename.find('\\'));
            fs.cat(filename.c_str());
            break;
        case head_num:
            filename = buf;
            filename = filename.substr(filename.find(" ") + 1);
            data = filename.substr(filename.find(" ") + 1);
            data = data.substr(0, data.find('\\'));
            filename = filename.substr(0, filename.find(" "));
            fs.head(filename.c_str(), stoi(data));
            break;
        case rm_num:
            filename = buf;
            filename = filename.substr(filename.find(" ") + 1);
            filename = filename.substr(0, filename.find('\\'));
            fs.rm(filename.c_str());
            break;
        }
    }

    //unmout the file system
    fs.unmount();

    return 0;
}

short hashstr(const string &str) {
    if (str == "mkdir")
        return mkdir_num;
    if (str == "ls")
        return ls_num;
    if (str == "cd")
        return cd_num;
    if (str == "home")
        return home_num;
    if (str == "rmdir")
        return rmdir_num;
    if (str == "create")
        return create_num;
    if (str == "append")
        return append_num;
    if (str == "stat")
        return stat_num;
    if (str == "cat")
        return cat_num;
    if (str == "head")
        return head_num;
    if (str == "rm")
        return rm_num;
    return 0;
}

void* get_in_addr(struct sockaddr* s)
{
    if (s->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)s)->sin_addr);
    }

    return &(((struct sockaddr_in6*)s)->sin6_addr);
}
