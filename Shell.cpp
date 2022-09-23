// CPSC 3500: Shell
// Implements a basic shell (command line interface) for the file system

#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <cstring>
#include <arpa/inet.h>
using namespace std;

#include "Shell.h"
#define LINE_END "\\r\\n"

static const string PROMPT_STRING = "NFS> ";	// shell prompt

// Mount the network file system with server name and port number in the format of server:port
void Shell::mountNFS(string fs_loc) {
    //create the socket cs_sock and connect it to the server and port specified in fs_loc
    //if all the above operations are completed successfully, set is_mounted to true  
    struct addrinfo hints, * servinfo, * p;
    char s[INET6_ADDRSTRLEN];
    int pos = fs_loc.find(":");
    string server = fs_loc.substr(0, pos);
    string port = fs_loc.substr(pos + 1);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int rv = getaddrinfo(server.c_str(), port.c_str(), &hints, &servinfo);
    if (rv != 0) {
        fprintf(stderr, "error with getaddrinfo\n");
        exit(1);
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        cs_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (cs_sock == -1) {
            perror("Client: socket");
            continue;
        }

        if (connect(cs_sock, p->ai_addr, p->ai_addrlen) == -1) {
            close(cs_sock);
            perror("Client: bind");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "Could not connect socket\n");
        exit(1);
    }

    inet_ntop(p->ai_family, &(((struct sockaddr_in*)p->ai_addr)->sin_addr), s, sizeof s);
    cout << "Client: connecting to " << s << endl;

    freeaddrinfo(servinfo);

    is_mounted = true;
}

// Unmount the network file system if it was mounted
void Shell::unmountNFS() {
    // close the socket if it was mounted
    close(cs_sock);
}

// Remote procedure call on mkdir
void Shell::mkdir_rpc(string dname) {
    const int MAX_SIZE = 7680 + 19;
    char buf[MAX_SIZE];
    string cmd = "mkdir " + dname + LINE_END;

    strcpy(buf, cmd.c_str());
    int bytes_sent = send(cs_sock, buf, MAX_SIZE, 0);

    if (bytes_sent == -1 || bytes_sent == 0) {
        perror("write");
        unmountNFS();
    }

    memset(buf, 0, MAX_SIZE);
    int bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    string exit = buf;
    cout << exit << endl;

    int code = stoi(exit.substr(0, exit.find(" ")));

    memset(buf, 0, MAX_SIZE);
    bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    exit = buf;
    cout << exit << endl;

    memset(buf, 0, MAX_SIZE);
    bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    exit = buf;
    cout << exit << endl;
}

// Remote procedure call on cd
void Shell::cd_rpc(string dname) {
    const int MAX_SIZE = 7680 + 19;
    char buf[MAX_SIZE];
    string cmd = "cd " + dname + LINE_END;

    strcpy(buf, cmd.c_str());
    int bytes_sent = send(cs_sock, buf, MAX_SIZE, 0);

    if (bytes_sent == -1 || bytes_sent == 0) {
        perror("write");
        unmountNFS();
    }

    memset(buf, 0, MAX_SIZE);
    int bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    string exit = buf;
    cout << exit << endl;

    memset(buf, 0, MAX_SIZE);
    bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    exit = buf;
    cout << exit << endl;

    memset(buf, 0, MAX_SIZE);
    bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    exit = buf;
    cout << exit << endl;
}

// Remote procedure call on home
void Shell::home_rpc() {
    const int MAX_SIZE = 7680 + 19;
    char buf[MAX_SIZE];
    string fill = "";
    string cmd = "home" + fill + LINE_END;

    strcpy(buf, cmd.c_str());
    int bytes_sent = send(cs_sock, buf, MAX_SIZE, 0);

    if (bytes_sent == -1 || bytes_sent == 0) {
        perror("write");
        unmountNFS();
    }
    memset(buf, 0, MAX_SIZE);
    int bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    string exit = buf;
    cout << exit << endl;

    memset(buf, 0, MAX_SIZE);
    bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    exit = buf;
    cout << exit << endl;

    memset(buf, 0, MAX_SIZE);
    bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    exit = buf;
    cout << exit << endl;
}

// Remote procedure call on rmdir
void Shell::rmdir_rpc(string dname) {
    const int MAX_SIZE = 7680 + 19;
    char buf[MAX_SIZE];

    string cmd = "rmdir " + dname + LINE_END;

    strcpy(buf, cmd.c_str());
    int bytes_sent = send(cs_sock, buf, MAX_SIZE, 0);

    if (bytes_sent == -1 || bytes_sent == 0) {
        perror("write");
        unmountNFS();
    }
    memset(buf, 0, MAX_SIZE);
    int bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    string exit = buf;
    cout << exit << endl;

    memset(buf, 0, MAX_SIZE);
    bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    exit = buf;
    cout << exit << endl;

    memset(buf, 0, MAX_SIZE);
    bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    exit = buf;
    cout << exit << endl;
}

// Remote procedure call on ls
void Shell::ls_rpc() {
    const int MAX_SIZE = 7680 + 19;
    char buf[MAX_SIZE];

    string fill = "";
    string cmd = "ls" + fill + LINE_END;

    strcpy(buf, cmd.c_str());
    int bytes_sent = send(cs_sock, buf, MAX_SIZE, 0);

    if (bytes_sent == -1 || bytes_sent == 0) {
        perror("write");
        unmountNFS();
    }
    memset(buf, 0, MAX_SIZE);
    int bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    string exit = buf;
    cout << exit << endl;

    int code = stoi(exit.substr(0, exit.find(" ")));
    memset(buf, 0, MAX_SIZE);
    bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    exit = buf;
    cout << exit << endl;

    memset(buf, 0, MAX_SIZE);
    bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    exit = buf;
    cout << exit << endl;

    memset(buf, 0, MAX_SIZE);
    bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    exit = buf;
    cout << exit << endl;
}

// Remote procedure call on create
void Shell::create_rpc(string fname) {
    const int MAX_SIZE = 7680 + 19;
    char buf[MAX_SIZE];

    string cmd = "create " + fname + LINE_END;

    strcpy(buf, cmd.c_str());
    int bytes_sent = send(cs_sock, buf, MAX_SIZE, 0);

    if (bytes_sent == -1 || bytes_sent == 0) {
        perror("write");
        unmountNFS();
    }
    memset(buf, 0, MAX_SIZE);
    int bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    string exit = buf;
    cout << exit << endl;

    memset(buf, 0, MAX_SIZE);
    bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    exit = buf;
    cout << exit << endl;

    memset(buf, 0, MAX_SIZE);
    bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    exit = buf;
    cout << exit << endl;
}

// Remote procedure call on append
void Shell::append_rpc(string fname, string data) {
    const int MAX_SIZE = 7680 + 19;
    char buf[MAX_SIZE];

    string cmd = "append " + fname + " " + data + LINE_END;
    strcpy(buf, cmd.c_str());
    int bytes_sent = send(cs_sock, buf, MAX_SIZE, 0);

    if (bytes_sent == -1 || bytes_sent == 0) {
        perror("write");
        unmountNFS();
    }
    memset(buf, 0, MAX_SIZE);
    int bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    string exit = buf;
    cout << exit << endl;

    int code = stoi(exit.substr(0, exit.find(" ")));
    memset(buf, 0, MAX_SIZE);
    bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    exit = buf;
    cout << exit << endl;

    memset(buf, 0, MAX_SIZE);
    bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    exit = buf;
    cout << exit << endl;

}

// Remote procesure call on cat
void Shell::cat_rpc(string fname) {
    const int MAX_SIZE = 7680 + 19;
    char buf[MAX_SIZE];

    string cmd = "cat " + fname + LINE_END;

    strcpy(buf, cmd.c_str());
    int bytes_sent = send(cs_sock, buf, MAX_SIZE, 0);

    if (bytes_sent == -1 || bytes_sent == 0) {
        perror("write");
        unmountNFS();
    }
    memset(buf, 0, MAX_SIZE);
    int bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    string exit = buf;
    cout << exit << endl;

    int code = stoi(exit.substr(0, exit.find(" ")));
    memset(buf, 0, MAX_SIZE);
    bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    exit = buf;
    cout << exit << endl;

    memset(buf, 0, MAX_SIZE);
    bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    exit = buf;
    cout << exit << endl;

    if (code == 200) {
        memset(buf, 0, MAX_SIZE);
        bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
        if (bytes_recv == -1 || bytes_recv == 0) {
            perror("read");
            return;
        }
        exit = buf;
        cout << exit << endl;
    }
}

// Remote procedure call on head
void Shell::head_rpc(string fname, int n) {
    const int MAX_SIZE = 7680 + 19;
    char buf[MAX_SIZE];

    string cmd = "head " + fname + " " + to_string(n) + LINE_END;

    strcpy(buf, cmd.c_str());
    int bytes_sent = send(cs_sock, buf, MAX_SIZE, 0);

    if (bytes_sent == -1 || bytes_sent == 0) {
        perror("write");
        unmountNFS();
    }
    memset(buf, 0, MAX_SIZE);
    int bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    string exit = buf;
    cout << exit << endl;

    int code = stoi(exit.substr(0, exit.find(" ")));
    memset(buf, 0, MAX_SIZE);
    bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    exit = buf;
    cout << exit << endl;

    memset(buf, 0, MAX_SIZE);
    bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    exit = buf;
    cout << exit << endl;

    if (code == 200) {
        memset(buf, 0, MAX_SIZE);
        bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
        if (bytes_recv == -1 || bytes_recv == 0) {
            perror("read");
            return;
        }
        exit = buf;
        cout << exit << endl;
    }
}

// Remote procedure call on rm
void Shell::rm_rpc(string fname) {
    const int MAX_SIZE = 7680 + 19;
    char buf[MAX_SIZE];

    string cmd = "rm " + fname + LINE_END;

    strcpy(buf, cmd.c_str());
    int bytes_sent = send(cs_sock, buf, MAX_SIZE, 0);

    if (bytes_sent == -1 || bytes_sent == 0) {
        perror("write");
        unmountNFS();
    }
    memset(buf, 0, MAX_SIZE);
    int bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    string exit = buf;
    cout << exit << endl;

    memset(buf, 0, MAX_SIZE);
    bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    exit = buf;
    cout << exit << endl;

    memset(buf, 0, MAX_SIZE);
    bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    exit = buf;
    cout << exit << endl;
}

// Remote procedure call on stat
void Shell::stat_rpc(string fname) {
    const int MAX_SIZE = 7680 + 19;
    char buf[MAX_SIZE];

    string cmd = "stat " + fname + LINE_END;

    strcpy(buf, cmd.c_str());
    int bytes_sent = send(cs_sock, buf, MAX_SIZE, 0);

    if (bytes_sent == -1 || bytes_sent == 0) {
        perror("write");
        unmountNFS();
    }
    memset(buf, 0, MAX_SIZE);
    int bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    string exit = buf;
    cout << exit << endl;

    int code = stoi(exit.substr(0, exit.find(" ")));
    memset(buf, 0, MAX_SIZE);
    bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    exit = buf;
    cout << exit << endl;

    memset(buf, 0, MAX_SIZE);
    bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
    if (bytes_recv == -1 || bytes_recv == 0) {
        perror("read");
        return;
    }
    exit = buf;
    cout << exit << endl;

    if (code == 200) {
        memset(buf, 0, MAX_SIZE);
        bytes_recv = recv(cs_sock, buf, MAX_SIZE, 0);
        if (bytes_recv == -1 || bytes_recv == 0) {
            perror("read");
            return;
        }
        exit = buf;
        cout << exit << endl;
    }
}

// Executes the shell until the user quits.
void Shell::run()
{
    // make sure that the file system is mounted
    if (!is_mounted)
        return;

    cout << "HELLO\n";
    // continue until the user quits
    bool user_quit = false;
    while (!user_quit) {

        // print prompt and get command line
        string command_str;
        cout << PROMPT_STRING;
        getline(cin, command_str);

        // execute the command
        user_quit = execute_command(command_str);
    }

    // unmount the file system
    unmountNFS();
}

// Execute a script.
void Shell::run_script(char* file_name)
{
    // make sure that the file system is mounted
    if (!is_mounted)
        return;
    // open script file
    ifstream infile;
    infile.open(file_name);
    if (infile.fail()) {
        cerr << "Could not open script file" << endl;
        return;
    }


    // execute each line in the script
    bool user_quit = false;
    string command_str;
    getline(infile, command_str, '\n');
    while (!infile.eof() && !user_quit) {
        cout << PROMPT_STRING << command_str << endl;
        user_quit = execute_command(command_str);
        getline(infile, command_str);
    }

    // clean up
    unmountNFS();
    infile.close();
}


// Executes the command. Returns true for quit and false otherwise.
bool Shell::execute_command(string command_str)
{
    // parse the command line
    struct Command command = parse_command(command_str);

    // look for the matching command
    if (command.name == "") {
        return false;
    }
    else if (command.name == "mkdir") {
        mkdir_rpc(command.file_name);
    }
    else if (command.name == "cd") {
        cd_rpc(command.file_name);
    }
    else if (command.name == "home") {
        home_rpc();
    }
    else if (command.name == "rmdir") {
        rmdir_rpc(command.file_name);
    }
    else if (command.name == "ls") {
        ls_rpc();
    }
    else if (command.name == "create") {
        create_rpc(command.file_name);
    }
    else if (command.name == "append") {
        append_rpc(command.file_name, command.append_data);
    }
    else if (command.name == "cat") {
        cat_rpc(command.file_name);
    }
    else if (command.name == "head") {
        errno = 0;
        unsigned long n = strtoul(command.append_data.c_str(), NULL, 0);
        if (0 == errno) {
            head_rpc(command.file_name, n);
        }
        else {
            cerr << "Invalid command line: " << command.append_data;
            cerr << " is not a valid number of bytes" << endl;
            return false;
        }
    }
    else if (command.name == "rm") {
        rm_rpc(command.file_name);
    }
    else if (command.name == "stat") {
        stat_rpc(command.file_name);
    }
    else if (command.name == "quit") {
        return true;
    }

    return false;
}

// Parses a command line into a command struct. Returned name is blank
// for invalid command lines.
Shell::Command Shell::parse_command(string command_str)
{
    // empty command struct returned for errors
    struct Command empty = { "", "", "" };

    // grab each of the tokens (if they exist)
    struct Command command;
    istringstream ss(command_str);
    int num_tokens = 0;
    if (ss >> command.name) {
        num_tokens++;
        if (ss >> command.file_name) {
            num_tokens++;
            if (ss >> command.append_data) {
                num_tokens++;
                string junk;
                if (ss >> junk) {
                    num_tokens++;
                }
            }
        }
    }

    // Check for empty command line
    if (num_tokens == 0) {
        return empty;
    }

    // Check for invalid command lines
    if (command.name == "ls" ||
        command.name == "home" ||
        command.name == "quit")
    {
        if (num_tokens != 1) {
            cerr << "Invalid command line: " << command.name;
            cerr << " has improper number of arguments" << endl;
            return empty;
        }
    }
    else if (command.name == "mkdir" ||
        command.name == "cd" ||
        command.name == "rmdir" ||
        command.name == "create" ||
        command.name == "cat" ||
        command.name == "rm" ||
        command.name == "stat")
    {
        if (num_tokens != 2) {
            cerr << "Invalid command line: " << command.name;
            cerr << " has improper number of arguments" << endl;
            return empty;
        }
    }
    else if (command.name == "append" || command.name == "head")
    {
        if (num_tokens != 3) {
            cerr << "Invalid command line: " << command.name;
            cerr << " has improper number of arguments" << endl;
            return empty;
        }
    }
    else {
        cerr << "Invalid command line: " << command.name;
        cerr << " is not a command" << endl;
        return empty;
    }

    return command;
}
