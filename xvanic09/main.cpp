/**
* Author: Jozef Vanický
* VUT Login: xvanic09
* Date: 2019-10-19
* Author's comment: N/A
*TODO:  Change makefile! remove http:// and https:// from server on stdin and add www.
**/

#include <iostream>
#include <string>

#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

using namespace std;

/* Function's prototypes */
int parse_args(int argc ,char *argv[]);
void err_parse_args();
void err_dupl_args();
void get_server_ip();
void err_get_server_ip();

/* Global variables */
bool r_flag = false;
bool x_flag = false;
bool six_flag = false;
bool p_flag = false;

bool s_flag = false;
bool address_flag = false;

int port = 53;
string str_server;
string address;
string str_server_ip;






int main(int argc, char *argv[]) {
    parse_args(argc, argv);
    get_server_ip();



   //Temporary debug listing
    cout << "Debug:" << endl << "r_flag: " << r_flag << endl << "x_flag: " << x_flag << endl << "six_flag: " << six_flag
         << endl << "s_flag: " << s_flag << endl << "Server: " << str_server << endl << "Server IP: " << str_server_ip << endl
         << "p_flag: " << p_flag << endl << "Port: " << port << endl << "address_flag: "
         << address_flag << endl << "Address: " << address << endl << "*** END OF DEBUG ***" << endl;

    return 0;
}

/*** Created using the informations obtained on the manual pages of function getaddrinfo() and getnameinfo().
 * Plus with the informations on website https://beej.us/guide/bgnet/html//index.html#getaddrinfoprepare-to-launch ***/
void get_server_ip(){
    struct addrinfo hints{}, *infoptr; // So no need to use memset global variables
    const char * server = str_server.c_str();

    hints.ai_family = AF_UNSPEC; // AF_INET means IPv4 only addresses

    int get_addr_return_code = getaddrinfo(server, nullptr, &hints, &infoptr);

    if (get_addr_return_code != 0) {
        cerr << "Error: Failed to obtain server info! " << gai_strerror(get_addr_return_code) << endl;
        err_get_server_ip();
    }

    char host[256];

    int get_name_return_code = getnameinfo(infoptr->ai_addr, infoptr->ai_addrlen, host, sizeof (host), nullptr, 0, NI_NUMERICHOST);
    if (get_name_return_code != 0) {
        cerr << "Error: Resolving numeric host name gone wrong! " << gai_strerror(get_name_return_code) << endl;
        err_get_server_ip();
    }

    str_server_ip = host;
    freeaddrinfo(infoptr);
}

void err_get_server_ip(){
    cerr << "Getting server IP failed." << endl;
    exit(EXIT_FAILURE);
}

bool is_valid_ipv4(const string& str)
{
    struct sockaddr_in sa{};
    return inet_pton(AF_INET, str_server.c_str(), &(sa.sin_addr))!=0;
}

bool is_valid_ipv6(const string& str)
{
    struct sockaddr_in6 sa;
    return inet_pton(AF_INET6, str_server.c_str(), &(sa.sin6_addr))!=0;
}

int parse_args(int argc ,char *argv[]){
    if(argc < 4){
        cerr << "Error: required arguments missing!" << endl;
        err_parse_args();
    }
    if(argc > 9){
        cerr << "Error: too many arguments!" << endl;
        err_parse_args();
    }

    for (int i = 1; i < argc; ++i) {
        string old_param;
        string upcoming_param;
        string param = argv[i];

        if(i > 1){
            old_param = argv[i-1];
        }
        if(i < argc-1){
            upcoming_param = argv[i+1];
        }

        if(param == "-r"){
            if(r_flag){
                err_dupl_args();
            }
            r_flag = true;
        }
        else if(param == "-x"){
            if(x_flag){
                err_dupl_args();
            }
            x_flag = true;
        }
        else if(param == "-6"){
            if(six_flag){
                err_dupl_args();
            }
            six_flag = true;
        }
        else if((param == "-s") && (i < argc-1)){
            if(upcoming_param[0] == '-'){
                cerr << "Error: -s parameter has invalid or missing argument!" << endl;
                err_parse_args();
            }
            if(s_flag){
                err_dupl_args();
            }
            s_flag = true;
        }
        else if((old_param == "-s") && (param[0] != '-') && s_flag){
            str_server = param;
        }
        else if((param == "-p") && (i < argc-1)){
            if(upcoming_param[0] == '-'){
                cerr << "Error: -p parameter has invalid or missing argument!" << endl;
                err_parse_args();
            }
            if(p_flag){
                err_dupl_args();
            }
            p_flag = true;
        }
        else if((old_param == "-p") && (param[0] != '-') && p_flag){
            for (unsigned long j = 0; j < strlen(argv[i]); j++) //param to neberie, preto argv[i]
            {
                if (!isdigit(param[j])) {
                    cerr << "Error: invalid character was located at the position of port number!" << endl;
                    err_parse_args();
                }
            }
            port = stoi(param);
            if(port < 1 || port > 65535){ //TODO: CHECK VALID PORT RANGE
                cerr << "Error: invalid port number! Use port in range 1-65535 or don't use the -p parameter." << endl;
            }
        }
        else if(((old_param != "-p") || old_param != "-s") && (param[0] != '-')){
            if(address_flag){
                err_dupl_args();
            }
            address_flag = true;
            address = param;
        }
        else{
            cerr << "Error: unknown parameter was used or one of the parameters is missing an argument!" << endl;
            err_parse_args();
        }
    }
    if(!(address_flag) || !(s_flag)){ ///Skoci sem vobec niekedy ? Myslim ze nie
        cerr << "Error: server or address was not entered!" << endl;
        err_parse_args();
    }
    return 0;
}

void err_parse_args(){
    cerr << "Run program in the following construct:" << endl
         << "   ./dns [-r] [-x] [-6] -s server [-p port] address" << endl;
    exit(EXIT_FAILURE);
}

void err_dupl_args(){
    cerr << "Error: argument duplicity!" << endl;
    err_parse_args();
}

