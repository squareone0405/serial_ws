#pragma once
/*
* udpserver.h
* A simple template udpserver to send designated messages
* 2018.8.22
* Zhang Xiang - zx1239856@gmail.com
*/

#include <deque>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <thread>
#include "../include/mavlink/v1.0/common/mavlink.h"

class udpserver
{
  private:
    unsigned int listen_port;
    // buffer
    char recvBuffer[512];
    char sendBuffer[1024];
    // msg queue
    std::deque<mavlink_message_t> msg_q;
    // server status controlling variable
    bool started = false;
    bool stop = false;
    // udp socket for listening
    struct sockaddr_in addr;
    int sock;
    // thread and corresponding handler
    std::thread handlerThread;
    void handler();
  public:
    udpserver(unsigned int listen_port = 14555);
    virtual ~udpserver();
    bool start();
    bool isStart() const;
    void addMsg(const mavlink_message_t& msg);
};
