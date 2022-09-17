#ifndef __DEF__
#define __DEF__

#define CONNECT 0
#define DISCONNECT 1
#define GET_TIME 2
#define GET_NAME 3
#define LIST_CONNECTER 4
#define SEND 5
#define EXIT 6
typedef struct package PACKAGE;
struct package{
    int type;
    int message_len;
    char buf[128];
};
#endif