/**
* Author: Jozef Vanický
* VUT Login: xvanic09
* Date: 2019-10-19
* Author's comment: N/A
*TODO:  Change makefile! remove http:// and https:// from server on stdin and add www. Check inet_pton in main returns 1, as succ translate, else error.
**/

#include <iostream>
#include <string>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std;

/* Function's prototypes */
int parse_args(int argc ,char *argv[]);
void err_parse_args();
void err_dupl_args();
void get_server_info_and_send_a_packet();
void err_get_server_info();
void err_connect_fail();
//bool is_valid_ipv4(const string& str_ip);

/* Global variables */
bool r_flag = false;
bool x_flag = false;
bool six_flag = false;
bool p_flag = false;
bool s_flag = false;
bool address_flag = false;
unsigned int port = 53;
string str_server;
string address;

/* DNS Packet structre*/
struct DNS_header
{
    unsigned short id :16; // identification number //is the ID number neened there ?
    unsigned char qr :1; // query/response flag
    unsigned char opcode :4; // purpose of message
    unsigned char aa :1; // authoritive answer
    unsigned char tc :1; // truncated message
    unsigned char rd :1; // recursion desired
    unsigned char ra :1; // recursion available
    unsigned char z :3; // its z! reserved //that guy had it set at 1 bit, why ?
    unsigned char rcode :4; // response code

    //unsigned char cd :1; // checking disabled
    //unsigned char ad :1; // authenticated data

    unsigned short q_count; // number of question entries
    unsigned short an_count; // number of answer entries
    unsigned short ns_count; // number of authority entries
    unsigned short ar_count; // number of resource entries
};

struct DNS_Question //Constant sized fields of query structure
{
    //question name ???
    unsigned short qtype;
    unsigned short qclass;
};

int main(int argc, char *argv[]) {
    parse_args(argc, argv);
    get_server_info_and_send_a_packet();


    /*
     * int sfd;
    const char * server_ip = str_server_ip.c_str();
    if (is_valid_ipv4(str_server_ip) == 1){ //IT IS IPV4, ELSE IPV6! Validity of both already checked.
        server_is_ipv4 = true;

        if(inet_pton(AF_INET, server_ip, &(sa4.sin_addr)) != 1){
            err_invalid_ip();
        }
        sa4.sin_port = htons(uint16_t (port));
        sa4.sin_family = AF_INET;
        sfd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }
    else{

        if(inet_pton(AF_INET6, server_ip, &(sa6.sin6_addr)) != 1){
            err_invalid_ip();
        }
        sa6.sin6_port = htons(uint16_t (port));
        sa6.sin6_family = AF_INET6;
        sfd = socket(PF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    }

    if (sfd == -1){
        cerr << "Error: Failed to create a socket!" << endl;
        err_connect_fail();
    }

    connect_to_server();
    */




   //Temporary debug listing
    cout << "Debug:" << endl << "r_flag: " << r_flag << endl << "x_flag: " << x_flag << endl << "six_flag: " << six_flag
         << endl << "s_flag: " << s_flag << endl << "Server: " << str_server << endl
         << "p_flag: " << p_flag << endl << "Port: " << port << endl << "address_flag: "
         << address_flag << endl << "Address: " << address << endl << "*** END OF DEBUG ***" << endl;

    return 0;
}

/*** Created using the informations obtained on the manual pages of function getaddrinfo() and getnameinfo().
 * Plus with the informations on website https://beej.us/guide/bgnet/html//index.html#getaddrinfoprepare-to-launch ***/
void get_server_info_and_send_a_packet(){
    const char * port_ptr = to_string(port).c_str();
    int sfd;
    struct addrinfo hints{}, *server_info, *available_ip;
    const char * server = str_server.c_str();

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC; ///AF_INET or AF_INET6, well who cares? I want atleast one valid IP, its type is irrelevant.
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_NUMERICSERV;
    hints.ai_protocol = IPPROTO_UDP;

    int get_server_info = getaddrinfo(server, port_ptr, &hints, &server_info); //for future use
    if (get_server_info != 0) {
        cerr << "Error: Failed to obtain server info! " << gai_strerror(get_server_info) << endl;
        err_get_server_info();
    }

    for(available_ip = server_info; available_ip != nullptr; available_ip = available_ip->ai_next) {
        sfd = socket(available_ip->ai_family, SOCK_DGRAM, IPPROTO_UDP);
        if (sfd == -1) {
            continue;
        }
        if (connect(sfd, available_ip->ai_addr, available_ip->ai_addrlen) == 0) {
            break;  ///Successfully connected!
        }
        close(sfd);
    }
    if (available_ip == nullptr) {               /* No address succeeded to connect */
        err_connect_fail();
    }
    string msg = "hello";

    int send_succ = send(sfd, msg.c_str(), msg.length(), 0);


    /* PROBABLY NOT NEEDED ANYMORE!
    char host[256];

    int get_name_return_code = getnameinfo(infoptr->ai_addr, infoptr->ai_addrlen, host, sizeof (host), nullptr, 0, NI_NUMERICHOST);
    if (get_name_return_code != 0) {
        cerr << "Error: Resolving numeric host name gone wrong! " << gai_strerror(get_name_return_code) << endl;
        err_get_server_ip();
    }
    str_server_ip = host;



     freeaddrinfo(server_info);
    */



}

void err_connect_fail(){
    cerr << "Connection attempt failed. " << endl;
    exit(EXIT_FAILURE);
}

void err_get_server_info(){
    cerr << "Getting server IP failed. " << endl;
    exit(EXIT_FAILURE);
}

/*bool is_valid_ipv4(const string& str_ip)
{
    struct sockaddr_in sockaddr{};
    return inet_pton(AF_INET, str_ip.c_str(), &(sockaddr.sin_addr))==1;
}*/

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
            if(port < 1 || port > 65535){
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

