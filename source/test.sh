#!/bin/sh
mkdir -pv tests

test_standard() { #$1 - name, $2 - server
    dig +short $1 @$2 > tests/$1.ref 2>/dev/null
    ./dns -r -s $2 $1 | grep "IN," | cut -d "," -f 5 | grep "[[:space:]]" | sed -e 's/^ *//g' > tests/$1.out 2>/dev/null
    diff -c tests/$1.out tests/$1.ref > /dev/null 2>/dev/null
    result=$?
    rm -rf tests/$1.*
    printf $1
    if [ ${result} = 0 ]; then
        echo " OK"
    else
        echo " ERROR"
    fi
}

test_ipv6_standard() { #$1 - name, $2 - server
    dig +short AAAA $1 @$2 > tests/$1.ref 2>/dev/null
    ./dns -r -6 -s $2 $1 | grep "IN," | cut -d "," -f 5 | grep "[[:space:]]" | sed -e 's/^ *//g' > tests/$1.out 2>/dev/null
    diff -c tests/$1.out tests/$1.ref > /dev/null 2>/dev/null
    result=$?
    rm -rf tests/$1.*
    printf $1
    printf " AAAA"
    if [ ${result} = 0 ]; then
        echo " OK"
    else
        echo " ERROR"
    fi
}

test_reverse() { #$1 - name, $2 - server
    dig +short -x $1 @$2 > tests/$1.ref 2>/dev/null
    ./dns -r -x -s $2 $1 | grep "IN," | cut -d "," -f 5 | grep "[[:space:]]" | sed -e 's/^ *//g' > tests/$1.out 2>/dev/null
    diff -c tests/$1.out tests/$1.ref > /dev/null 2>/dev/null
    result=$?
    rm -rf tests/$1.*
    printf $1
    printf " PTR"
    if [ ${result} = 0 ]; then
        echo " OK"
    else
        echo " ERROR"
    fi
}

SERVER=kazi.fit.vutbr.cz
test_standard "fit.vut.cz" $SERVER
test_standard "www.fit.vut.cz" $SERVER
test_standard "merlin.fit.vutbr.cz" $SERVER
test_standard "google.com" $SERVER
test_standard "dcvision.cz" $SERVER

test_ipv6_standard "fit.vut.cz" $SERVER
test_ipv6_standard "www.fit.vut.cz" $SERVER
test_ipv6_standard "merlin.fit.vutbr.cz" $SERVER
test_ipv6_standard "google.com" $SERVER

test_reverse "147.229.0.1" $SERVER
test_reverse "::1" $SERVER
test_reverse "2a00:1450:4014:80d::200e" $SERVER
