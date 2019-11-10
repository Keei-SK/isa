/**
* Author: Jozef Vanický
* VUT Login: xvanic09
* Date: 2019-10-19
* Author's comment: N/A
*TODO:  Change makefile!
**/

#include <iostream>
#include <string>
#include <algorithm>
#include <vector>

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
void address_to_dns_format();
bool check_address_is_ip();

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
string address_dns_format;
string address_backup;

/* DNS Packet structre*/
struct DNS_header
{
    uint16_t id; // identification number
    uint16_t flags;

    uint16_t qd_count; // number of question entries
    uint16_t an_count; // number of answer entries
    uint16_t ns_count; // number of authority entries
    uint16_t ar_count; // number of resource entries
};

struct DNS_Question
{
    uint16_t qtype;
    uint16_t qclass;
};

int main(int argc, char *argv[]) {
    parse_args(argc, argv);
    get_server_info_and_send_a_packet();

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
    struct addrinfo hints{}, *server_info, *available_ip;
    const char * server = str_server.c_str();
    const char * port_ptr = to_string(port).c_str();
    int sfd;

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
    if (available_ip == nullptr) {  /* No address succeeded to connect */
        err_connect_fail();
    }

    /**Creating DNS packet and filling it up with info**/ //TODO: check which arguments can and cannot be set by user, like are there restrictions to use both -6 and -x ?
    unsigned char packet_buffer[512*8];
    memset(packet_buffer, htons(0), sizeof(packet_buffer));
    /*-DNS_Header-*/
    struct DNS_header *dns_packet_header;
    dns_packet_header = (struct DNS_header *)&packet_buffer;
    dns_packet_header->id = htons((int) getpid());
    uint16_t flags = 0x0;
    if(x_flag) {
        flags |= 0x1 << 11;
    }
    if(r_flag){
        flags |= 0x1 << 8;
    }
    dns_packet_header->flags = htons(flags);
    dns_packet_header->qd_count = htons((int)1);
    dns_packet_header->an_count=0;
    dns_packet_header->ns_count=0; //TODO: Not sure.
    dns_packet_header->ar_count=0; //TODO: Not sure.

    /*-DNS_Question-*/
    if(!check_address_is_ip()){ //pokud adresa neni hostname, preloz ji na dns format
        address_to_dns_format();
    }
    //todo: address not greater than 512bajts-implemented
    char* pointer = (char *)dns_packet_header + sizeof(struct DNS_header);
    strcpy(pointer, address.c_str());
    struct DNS_Question *dnsquestion = (struct DNS_Question *)(pointer + 1 + address.length());
    dnsquestion->qclass = htons(1);
    if(six_flag){
        dnsquestion->qtype = htons(28);
    } else{
        dnsquestion->qtype = htons(1);
    }


    int send_succ = send(sfd, (char*) packet_buffer, sizeof(struct DNS_header)+ 2 + address.length() +
            sizeof(struct DNS_Question), 0);
    /*
     *
     *
     *

    string msg = "hello";
    int send_succ = send(sfd, msg.c_str(), msg.length(), 0);
     */


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

void address_to_dns_format(){
    string rev, rev2;
    for_each(address.crbegin(), address.crend(), [&rev] (char const &c) {
        rev = rev.append(1, c);
    });
    rev = rev + ".";
    int dotter = 0;
    for (int i = 0; i < (int) rev.length(); i++) {
        if((rev[i] != '.')){
            dotter++;
        }
        else{
            rev[i] = (char)dotter;
            dotter = 0;
        }
    }
    for_each(rev.crbegin(), rev.crend(), [&rev2] (char const &c) {
        rev2 = rev2.append(1, c);
    });
    address = rev2;
}

bool check_address_is_ip(){
    string temp = address;
    if(((int) address.find(':', 0)) != -1){ //ak string neobsahuje ":" neni to ipv6
        return true; //bola najdena ":", adresa je ipv6
    }
    else{
        for(int i=0; i< ((int) temp.length()); i++){ //vsetky "." v adrese nahradime cislom a kontrolujeme ci string obsahuje ipba cisla, teda ci je to ipv4
            if(temp[i]== '.'){
                temp[i] = 1;
            }
            if(!(isdigit(temp[i]))){
                return false; //bol najdeny iny znak nez cislo, adresa je teda hostname
            }
        }
        return true; //string obsahuje iba cisla, je to ipv4
    }
}