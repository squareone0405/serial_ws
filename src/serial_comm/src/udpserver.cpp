/*
* udpserver.cpp
* A simple template udpserver to send designated messages
* 2018.8.22
* Zhang Xiang - zx1239856@gmail.com
*/

#include "udpserver.h"
#include <functional>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <time.h>
#include <arpa/inet.h>
#include <linux/ip.h>
#include "ros/ros.h"
#include "../include/mavlink/v1.0/common/mavlink.h"

udpserver::udpserver(unsigned int port) : listen_port(port)
{
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(listen_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
}

udpserver::~udpserver()
{
    stop = true;
    close(sock);
    if (!started)
        return;
    if (handlerThread.joinable())
        handlerThread.join();
}

bool udpserver::start()
{
    if (sock = socket(AF_INET, SOCK_DGRAM, 0) < 0)
    {
        ROS_ERROR("Error create socket.\n");
        throw "Error create socket.";
        return false;
    }
    if (bind(sock, (sockaddr *)&addr, sizeof(addr)) < 0)
    {
        ROS_ERROR("Failed to bind the port.\n");
        exit(EXIT_FAILURE);
        return false;
    }
    handlerThread = std::thread(std::bind(&udpserver::handler, this));
    handlerThread.detach();
    started = true;
    return true;
}

bool udpserver::isStart() const
{
    return started;
}

// main processor
void udpserver::handler()
{
    struct sockaddr_in clientAddr;
    while (true)
    {
        if (stop)
            break;
        unsigned int curr_sockaddr_len = sizeof(sockaddr);
        int n = recvfrom(sock, (void *)recvBuffer, 512, 0, (struct sockaddr *)&clientAddr, &curr_sockaddr_len);
        if (n > 0)
        {
            if (!msg_q.empty())
            {
                // read from queue
                mavlink_message_t msg = msg_q.front();
                msg_q.pop_front();
                unsigned int len = mavlink_msg_to_send_buffer((unsigned char *)sendBuffer, &msg);
                sendto(sock, sendBuffer, len, 0, (struct sockaddr *)&clientAddr, sizeof(struct sockaddr_in));
            }
        }
        // sleep for 1ms
        // std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void udpserver::addMsg(const mavlink_message_t &msg)
{
    // set max limit here
    if (msg_q.size() == 1000)
    {
        msg_q.pop_front();
    }
    msg_q.push_back(msg);
}