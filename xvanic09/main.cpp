/**
* Author: Jozef Vanický
* VUT Login: xvanic09
* Date: 2019-10-19
* Author's comment: N/A
**/

#include <iostream>
#include <string>
#include <algorithm>
#include <iterator>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std;

/*** Function's prototypes ***/
int parseArgs(int argc , char **argv);  //Parse arguments from stdin.
void get_server_info_and_send_a_packet();
void formatStringToDNS();   //Format given address to dns valid dns format. www.vutbr.cz -> 3www5vutbr2cz
void isIPv6();
string reverseIPV4(string ip);
string reverseIPV6(string ip);
string formatDNStoString(unsigned char **ptr_to_end_of_header, void* start_address);
string parseType(uint16_t the_type);   //Parse data in pursuance of the question/answer type
string parseClass(uint16_t the_class);  //Parse data in pursuance of the question/answer class
string parseRDATA(uint16_t atype, unsigned char *ptr_to_pos, void* packet_header);  //Parse data in pursuance of the answer RDATA type
string parseAnswer(void *packet_header, unsigned char **ptr_to_pos);    //Parse answer data to stdout

void err_parseArgs();
void err_duplArgs();
void err_getServerInfo();
void err_connectionFail();

/*** Global variables ***/
bool r_flag, x_flag, six_flag, port_flag, server_flag, address_flag = false;
unsigned int port = 53;
string server;
string address;

bool ipv4_flag = false;
bool ipv6_flag = false;

/*** DNS Packet ***/
struct DNS_header{
    uint16_t id; // identification number
    uint16_t flags; //|QR|Opcode|AA|TC|RD|RA|Z|RCODE|
    uint16_t qd_count; // number of question entries
    uint16_t an_count; // number of answers
    uint16_t ns_count; // number of authorities
    uint16_t ar_count; // number of additional
};

struct DNS_question{
    uint16_t qtype;
    uint16_t qclass;
};

struct DNS_answer{
    uint16_t atype;
    uint16_t aclass;
    uint32_t ttl;
    uint16_t rd_length;
} __attribute((packed));    //attribute needed, otherwise compiler optimizes structure size



int main(int argc, char *argv[]) {
    parseArgs(argc, argv);
    get_server_info_and_send_a_packet();

    return 0;
}

/*
 * Following documents were used for the implementation of this function:
 * Man pages of getaddrinfo(),
 * https://beej.us/guide/bgnet/html//index.html#getaddrinfoprepare-to-launch of author Brian Hall.
 * */
void get_server_info_and_send_a_packet(){
    struct addrinfo hints{}, *server_info, *available_ip;
    const char * serv = server.c_str();
    const char * port_ptr = to_string(port).c_str();
    int sfd;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC; ///AF_INET or AF_INET6, well who cares? I want atleast one valid IP, its type is irrelevant.
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_NUMERICSERV;
    hints.ai_protocol = IPPROTO_UDP;

    int get_server_info = getaddrinfo(serv, port_ptr, &hints, &server_info);
    if (get_server_info != 0) {
        cerr << "Error: Failed to obtain server info! " << gai_strerror(get_server_info) << endl;
        err_getServerInfo();
    }

    for(available_ip = server_info; available_ip != nullptr; available_ip = available_ip->ai_next) { //Just try to connect to one of received addresses, so we'll use it later
        sfd = socket(available_ip->ai_family, SOCK_DGRAM, IPPROTO_UDP);
        if (sfd == -1) {
            continue;
        }
        if (connect(sfd, available_ip->ai_addr, available_ip->ai_addrlen) == 0) {
            break;  ///Successfully connected!
        }
        close(sfd);
    }
    if (available_ip == nullptr) {  //No address succeeded to connect
        err_connectionFail();
    }

    /* Creating DNS packet and filling it up with info */
    unsigned char packet_buffer[512];
    memset(packet_buffer, 0, sizeof(packet_buffer));
    /*-DNS_Header-*/
    struct DNS_header *packet_header;
    packet_header = (struct DNS_header *)&packet_buffer;
    packet_header->id = htons((int) getpid());
    uint16_t flags = 0x0;
    if(r_flag){
        flags |= 0x1 << 8;
    }
    packet_header->flags = htons(flags);
    packet_header->qd_count = htons((int)1);
    packet_header->an_count=0;
    packet_header->ns_count=0;
    packet_header->ar_count=0;

    if(x_flag){
        isIPv6();
        if(ipv6_flag){
            if(address.find(".ip6.arpa",0) == -1){
                address = reverseIPV6(address);
            }
        }
        else{ //automaticky tu bude ipv4_flag na true
            if(address.find(".in-addr.arpa",0) == -1){
                address = reverseIPV4(address);
            }
        }
    }

    /*-DNS_question-*/
    if(address.back() != '.'){
        address = address + '.';
    }
    formatStringToDNS();

    char* ptr_to_pos = (char *)packet_header + sizeof(struct DNS_header);
    if(address.length() > (sizeof(packet_buffer)- sizeof(struct DNS_header) - sizeof(struct DNS_question))){
        cerr << "Error: DNS Question is too long" << endl;
        exit(EXIT_FAILURE);
    }
    strcpy(ptr_to_pos, address.c_str());
    struct DNS_question *dnsquestion = (struct DNS_question *)(ptr_to_pos + address.length());
    if(six_flag){
        dnsquestion->qtype = htons(28);
    } else{
        dnsquestion->qtype = htons(1);
    }
    if (x_flag) {
        dnsquestion->qtype = htons(12);
    }
    dnsquestion->qclass = htons(1);

    int send_succ = send(sfd, (char*) packet_buffer, sizeof(struct DNS_header) + address.length() + sizeof(struct DNS_question), 0);

    if (send_succ == -1){
        cerr << "Error: Request sending failed!";
        exit(EXIT_FAILURE);
    }

    /**Getting answer**/
    unsigned char answer[1472];
    memset(answer, 0, sizeof(answer));
    int recv_succ = recvfrom(sfd, answer, sizeof(answer), 0, available_ip->ai_addr, (socklen_t*)&(available_ip->ai_addrlen));
    if (recv_succ == -1){
        cerr << "Error: Failed to obtain requested answer!";
        exit(EXIT_FAILURE);
    }
    close(sfd);
    freeaddrinfo(server_info);
    memset(packet_buffer, 0, sizeof(packet_buffer));

    /**Reading answer**/
    packet_header = (struct DNS_header*) answer;
    string header_printout = "Authoritative: ";
    if((ntohs(packet_header->flags) & 0x400) > 0x0){
        header_printout = header_printout + "Yes, ";
    } else{
        header_printout = header_printout + "No, ";
    }
    header_printout = header_printout + "Recursive: ";
    if(r_flag && ((ntohs(packet_header->flags) & 0x80) > 0x0)){
        header_printout = header_printout + "Yes, ";
    } else{
        header_printout = header_printout + "No, ";
    }
    header_printout = header_printout + "Truncated: ";
    if((ntohs(packet_header->flags) & 0x200) > 0x0){
        header_printout = header_printout + "Yes";
    } else{
        header_printout = header_printout + "No";
    }

    ptr_to_pos = (char *)packet_header + sizeof(struct DNS_header);
    if(ntohs(packet_header->qd_count) < 0x0001){
        cerr << "Error: Received zero questions!" << endl;
        exit(EXIT_FAILURE);
    }
    string question_printout = "Question section (";
    question_printout += to_string(ntohs(packet_header->qd_count)) + ")" + "\n "; // Vypiseme pocet dotazu z odpovedi

    struct DNS_question qst = {0, 0};
    for (int i = 0; i < ntohs(packet_header->qd_count); ++i) {
        question_printout += formatDNStoString((unsigned char **) &ptr_to_pos, (void *) packet_header) + ", ";
        memcpy(&qst, ptr_to_pos, sizeof(struct DNS_question));
        question_printout += parseType(ntohs(qst.qtype)) + ", ";
        question_printout += parseClass(ntohs(qst.qclass));
        if(i < ((ntohs(packet_header->qd_count))-1)){
            question_printout += "\n";
        }
        ptr_to_pos += sizeof(struct DNS_question);
    }

    string answer_printout = "Answer section (";
    answer_printout += to_string(ntohs(packet_header->an_count)) + ")" + "\n"; // Vypiseme pocet answers
    for (int i = 0; i < ntohs(packet_header->an_count); i++) {
        answer_printout += parseAnswer(packet_header, (unsigned char **)&ptr_to_pos);

        if(i < ((ntohs(packet_header->an_count))-1)){
            answer_printout += "\n";
        }
    }

    string authority_printout = "Authority section (";
    authority_printout += to_string(ntohs(packet_header->ns_count)) + ")" + "\n"; // Vypiseme pocet authority
    for (int i = 0; i < ntohs(packet_header->ns_count); i++) {
        authority_printout += parseAnswer(packet_header, (unsigned char **)&ptr_to_pos);

        if(i < ((ntohs(packet_header->ns_count))-1)){
            authority_printout += "\n";
        }
    }

    string additional_printout = "Additional section (";
    additional_printout += to_string(ntohs(packet_header->ar_count)) + ")" + "\n"; // Vypiseme pocet additionals
    for (int i = 0; i < ntohs(packet_header->ar_count); i++) {
        additional_printout += parseAnswer(packet_header, (unsigned char **)&ptr_to_pos);

        if(i < ((ntohs(packet_header->ar_count))-1)){
            additional_printout += "\n";
        }
    }

    cout << header_printout << endl << question_printout << endl << answer_printout << endl << authority_printout << endl << additional_printout << endl;
}

string reverseIPV4(string ip){
    string tmp, final;
    if(ip.back() != '.'){
        ip += '.';
    }
    for (uint i = 0; i < ((uint) ip.length()); i++) {
        tmp += ip[i];
        if(ip[i] == '.'){
            final = tmp + final;
            tmp = "";
        }
    }
    final += "in-addr.arpa.";
    return final;
};

string reverseIPV6(string ip){
    char ip_address[INET6_ADDRSTRLEN];
    struct in6_addr binary;

    strcpy(ip_address, ip.c_str());
    inet_pton(AF_INET6, ip_address, &binary);
    bzero(ip_address, INET6_ADDRSTRLEN);
    //inet_ntop(AF_INET6, &binary, ip_address, INET6_ADDRSTRLEN);
    sprintf(ip_address, "%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x", (int)binary.s6_addr[0], (int)binary.s6_addr[1],
    (int)binary.s6_addr[2], (int)binary.s6_addr[3],
    (int)binary.s6_addr[4], (int)binary.s6_addr[5],
    (int)binary.s6_addr[6], (int)binary.s6_addr[7],
    (int)binary.s6_addr[8], (int)binary.s6_addr[9],
    (int)binary.s6_addr[10], (int)binary.s6_addr[11],
    (int)binary.s6_addr[12], (int)binary.s6_addr[13],
    (int)binary.s6_addr[14], (int)binary.s6_addr[15]);

    string string_ip = ip_address;
    string dotted_ip;
    for_each(string_ip.crbegin(), string_ip.crend(), [&dotted_ip] (char const &c) { //Reverse the address back.
        if (c != ':') {
            dotted_ip = dotted_ip.append(1, c);
            dotted_ip = dotted_ip.append(1, '.');
        }
    });

    dotted_ip += "ip6.arpa.";

    return dotted_ip;
};

string parseAnswer(void *packet_header, unsigned char **ptr_to_pos) {
    struct DNS_answer answr = {0, 0, 0, 0};

    string answer_printout = " ";
    answer_printout += formatDNStoString(ptr_to_pos, packet_header) + ", ";
    memcpy(&answr, *ptr_to_pos, sizeof(struct DNS_answer));
    answer_printout += parseType(ntohs(answr.atype)) + ", ";
    answer_printout += parseClass(ntohs(answr.aclass)) + ", ";
    answer_printout += to_string(ntohl(answr.ttl)) + ", ";
    *ptr_to_pos += sizeof(struct DNS_answer);

    answer_printout += parseRDATA(ntohs(answr.atype), *ptr_to_pos, packet_header);
    *ptr_to_pos += ntohs(answr.rd_length);
    return answer_printout;
}


string parseRDATA(uint16_t atype, unsigned char *ptr_to_pos, void* packet_header){
    struct RDATA_SOA{
        uint32_t serial;
        int32_t refresh;
        int32_t retry;
        uint32_t expire;
        uint32_t minimum;
    };

    struct RDATA_SOA soa = {0, 0, 0, 0, 0};
    string soa_struct;
    char ip_address[INET6_ADDRSTRLEN];
    unsigned int count = (unsigned int) *ptr_to_pos;
    char buffer[1472];

    switch (atype) {
        case 1:
            inet_ntop(AF_INET, ptr_to_pos, ip_address, INET_ADDRSTRLEN);
            return ip_address;
        case 2:
        case 5:
        case 12:
            return formatDNStoString((unsigned char **) &ptr_to_pos, (void *) packet_header);
        case 6:
            soa_struct = formatDNStoString((unsigned char **) &ptr_to_pos, (void *) packet_header) + " "; //mname
            soa_struct += formatDNStoString((unsigned char **) &ptr_to_pos, (void *) packet_header) + " "; //rname
            memcpy(&soa, ptr_to_pos, sizeof(struct RDATA_SOA));
            soa_struct += to_string(ntohl(soa.serial)) + " " +to_string(ntohl(soa.refresh)) + " " +
                    to_string(ntohl(soa.retry)) + " " +to_string(ntohl(soa.expire)) + " " +
                    to_string(ntohl(soa.minimum));
            return soa_struct;
        case 16:
            ptr_to_pos++;
            for (uint i = 0; i < count; ++i) {
                buffer[i] = *ptr_to_pos;
            }
            return buffer;
        case 28:
            inet_ntop(AF_INET6, ptr_to_pos, ip_address, INET6_ADDRSTRLEN);
            return ip_address;
        default:
            return "?";
    }
}

string parseType(uint16_t the_type){
    switch (the_type) {
        case 1:
            return "A";
        case 2:
            return "NS";
        case 5:
            return "CNAME";
        case 6:
            return "SOA";
        case 12:
            return "PTR";
        case 16:
            return "TXT";
        case 28:
            return "AAAA";
        default:
            return "?";
    }
}

string parseClass(uint16_t the_class){
    switch (the_class) {
        case 1:
            return "IN";
        case 2:
            return "CS";
        default:
            return "?";
    }
}

/*
 * Modifies address from DNS format to normal hostname.
 * 3www5vutbr2cz -> www.vutbr.cz */
string formatDNStoString(unsigned char **ptr_to_end_of_header, void* start_address) {
    string domain_name;

    while (**ptr_to_end_of_header != 0) {
        uint16_t number = ntohs((**ptr_to_end_of_header << 8) | *(*ptr_to_end_of_header+1));
        unsigned char label[2] = {0};
        memcpy(label, &number, 2);

        if((label[0] & 0xC0) == 0){
            for (int j = 0; j < label[0]; ++j) {
                domain_name += (*ptr_to_end_of_header)[1+j];
            }
            domain_name += '.';

            *ptr_to_end_of_header += label[0] + 1;
        }
        else if((label[0] & 0xC0) == 0xC0){
            uint16_t offset = ((label[0] << 8) | label[1]) & 0x3FFF;
            unsigned char *ptr_to_label = (unsigned char *)start_address + offset;
            domain_name += formatDNStoString(&ptr_to_label, start_address);
            (*ptr_to_end_of_header)++;
            break;
        }
    }
    (*ptr_to_end_of_header)++;

    return domain_name;
}

int parseArgs(int argc , char **argv){
    if(argc < 4){
        cerr << "Error: required arguments missing!" << endl;
        err_parseArgs();
    }
    if(argc > 9){
        cerr << "Error: too many arguments!" << endl;
        err_parseArgs();
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
                err_duplArgs();
            }
            r_flag = true;
        }
        else if(param == "-x"){
            if(x_flag){
                err_duplArgs();
            }
            x_flag = true;
        }
        else if(param == "-6"){
            if(six_flag){
                err_duplArgs();
            }
            six_flag = true;
        }
        else if((param == "-s") && (i < argc-1)){
            if(upcoming_param[0] == '-'){
                cerr << "Error: -s parameter has invalid or missing argument!" << endl;
                err_parseArgs();
            }
            if(server_flag){
                err_duplArgs();
            }
            server_flag = true;
        }
        else if((old_param == "-s") && (param[0] != '-') && server_flag){
            server = param;
        }
        else if((param == "-p") && (i < argc-1)){
            if(upcoming_param[0] == '-'){
                cerr << "Error: -p parameter has invalid or missing argument!" << endl;
                err_parseArgs();
            }
            if(port_flag){
                err_duplArgs();
            }
            port_flag = true;
        }
        else if((old_param == "-p") && (param[0] != '-') && port_flag){
            for (unsigned long j = 0; j < strlen(argv[i]); j++)
            {
                if (!isdigit(param[j])) {
                    cerr << "Error: invalid character was located at the position of port number!" << endl;
                    err_parseArgs();
                }
            }
            port = stoi(param);
            if(port < 1 || port > 65535){
                cerr << "Error: invalid port number! Use port in range 1-65535 or don't use the -p parameter." << endl;
            }
        }
        else if(((old_param != "-p") || old_param != "-s") && (param[0] != '-')){
            if(address_flag){
                err_duplArgs();
            }
            address_flag = true;
            address = param;
        }
        else{
            cerr << "Error: unknown parameter was used or one of the parameters is missing an argument!" << endl;
            err_parseArgs();
        }
    }
    if(!(address_flag) || !(server_flag)){
        cerr << "Error: server or address was not entered!" << endl;
        err_parseArgs();
    }
    if(x_flag && six_flag){
        err_parseArgs();
    }
    return 0;
}

/*
 * Modifies address to valid DNS format.
 * www.vutbr.cz -> 3www5vutbr2cz */
void formatStringToDNS(){
    string rev, rev2;
    for_each(address.crbegin(), address.crend(), [&rev] (char const &c) { //Reverse the address.
        rev = rev.append(1, c);
    });
    rev = rev + ".";
    int dotter = 0;
    for (int i = 0; i < (int) rev.length(); i++) { //Replace every dot with the number of chars before it.
        if((rev[i] != '.')){
            dotter++;
        }
        else{
            rev[i] = (char)dotter;
            dotter = 0;
        }
    }
    for_each(rev.crbegin(), rev.crend(), [&rev2] (char const &c) { //Reverse the address back.
        rev2 = rev2.append(1, c);
    });
    address = rev2;
}

/*** Auxiliary error functions ***/
void err_parseArgs(){
    cerr << "Run program in the following construct:" << endl
         << "   ./dns [-r] [-x] [-6] -s server [-p port] address" << endl
         << "    It's not allowed to use -x and -6 flag together !" << endl
         << "[-r] Recursion desired." << endl
         << "[-x] Reverse query." << endl
         << "[-6] AAAA query type instead of default A type." << endl
         << " -s  The IP address or domain name of the server where the query is to be sent." << endl
         << "[-p port] Port number where the query is to be sent. Default option is 53." << endl
         << "address means Address queried." << endl;
    exit(EXIT_FAILURE);
}

void err_duplArgs(){
    cerr << "Error: argument duplicity!" << endl;
    err_parseArgs();
}

void err_getServerInfo(){
    cerr << "Getting server IP failed. " << endl;
    exit(EXIT_FAILURE);
}

void err_connectionFail(){
    cerr << "Connection attempt failed. " << endl;
    exit(EXIT_FAILURE);
}

void isIPv6(){
    string temp = address;
    if(((int) address.find(':', 0)) != -1){ //ak string neobsahuje ":" neni to ipv6
        ipv6_flag = true;
    }
    ipv4_flag = true;
}