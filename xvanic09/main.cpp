/**
* Author: Jozef Vanický
* VUT Login: xvanic09
* Date: 2019-10-19
* Author's comment: N/A
*TODO:  Change makefile!
**/

#include <iostream>
#include <string>

#include "unistd.h"

using namespace std;

/* Function prototypes */
int parse_args(int argc ,char *argv[]);
void err_parse_args();
void err_dupl_args();

/* Global variables */
bool r_flag = false;
bool x_flag = false;
bool six_flag = false;
bool p_flag = false;

bool s_flag = false;
bool address_flag = false;


int port = 53;
string server;
string address;



int main(int argc, char *argv[]) {
    parse_args(argc, argv);

    return 0;

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

        if(i != 1){
            old_param = argv[i-1];
        }
        if(i != argc-1){
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
        else if((param == "-s") && (i < argc-1) && (upcoming_param[0] != '-')){
            if(s_flag){
                err_dupl_args();
            }
            s_flag = true;
        }
        else if((old_param == "-s") && (param[0] != '-') && s_flag){
            server = param;
        }
        else if((param == "-p") && (i < argc-1) && (upcoming_param[0] != '-')){
            if(p_flag){
                err_dupl_args();
            }
            p_flag = true;
        }
        else if((old_param == "-p") && (param[0] != '-') && p_flag){
            for (unsigned long j = 0; j < strlen(argv[i]); j++) //param to neberie, preto argv[i]
            {
                if (!isdigit(optarg[j])) {
                    cerr << "Error: invalid character was located at the position of port number!" << endl;
                    err_parse_args();
                }
            }
            port = stoi(param);
            if(port < 1 || port > 65535){ //TODO: CHECK VALID PORT RANGE
                cerr << "Error: invalid port number. Use port in range 1-65535 or don't use the -p parameter" << endl;
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
    if(!(address_flag) && !(s_flag)){
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