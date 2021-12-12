#include "EventWaitHandle.h"
#include "Exception.h"
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>


EventWaitHandle::EventWaitHandle() {
    struct epoll_event ev;
    int flags;
    
    if ((m_epollFD = epoll_create(1)) <= 0 ){
        int err = errno;
        throw Exception("Create epoll failed: %s (%d)", std::strerror(err), err);
    } //if
        
    if (pipe(m_pipeFD) == -1) {
        int err = errno;
        throw Exception("Create pipe failed: %s (%d)", std::strerror(err), err);
    } //if
            
    if ((flags = fcntl(m_pipeFD[0], F_GETFL)) == -1) {
        int err = errno;
        throw Exception("Getting pipe[0] flags failed: %s (%d)", std::strerror(err), err);
    } //if
    if (fcntl(m_pipeFD[0], F_SETFL, (flags | O_NONBLOCK)) == -1) {
        int err = errno;
        throw Exception("Setting pipe[0] flags failed: %s (%d)", std::strerror(err), err);
    } //if
    
    if ((flags = fcntl(m_pipeFD[1], F_GETFL)) == -1) {
        int err = errno;
        throw Exception("Getting pipe[1] flags failed: %s (%d)", std::strerror(err), err);
    } //if
    if (fcntl(m_pipeFD[1], F_SETFL, (flags | O_NONBLOCK)) == -1) {
        int err = errno;
        throw Exception("Setting pipe[1] flags failed: %s (%d)", std::strerror(err), err);
    } //if

    ev.events = EPOLLIN;
    ev.data.fd = m_pipeFD[0];
    if(epoll_ctl(m_epollFD, EPOLL_CTL_ADD, m_pipeFD[0], &ev) < 0) {
        int err = errno;
        throw Exception("Control operation for epoll failed: %s (%d)", std::strerror(err), err);
    } //if     
    m_waiting = false;
}


EventWaitHandle::~EventWaitHandle() {
    Abort();
    if (m_epollFD > 0)   { close(m_epollFD); }
    if (m_pipeFD[0] > 0) { close(m_pipeFD[0]); }
    if (m_pipeFD[1] > 0) { close(m_pipeFD[1]); }
}


bool EventWaitHandle::IsWaiting() {
    return m_waiting;
}


bool EventWaitHandle::Wait(int timeout_ms) {
    bool result = false;
    m_waiting = true;
    struct epoll_event ev;
    int ret = epoll_wait(m_epollFD, &ev, 1, timeout_ms);
    if( ret > 0) {
        char ch;
        int n;
        while (true) {
            n = read(m_pipeFD[0], &ch, 1);
            if (n == -1) {
                int err = errno;
                if (err == EAGAIN) {
                    //No more bytes
                    break;
                } else {
                    m_waiting = false;
                    throw Exception("Reading pipe failed: %s (%d)", std::strerror(err), err);
                } //if
            } //if
            //read next byte
        } //while   
        result = true;
    } else if (ret == 0) {
        //timeout reached
    } else { //(ret < 0)
        int err = errno;
        if (err == EINTR) {
            //4: Interrupted system call
            //It's okay, system interrupted waiting of thread (e.g. received SIGTERM)
            //result = false - same as timeout
        } else {
            throw Exception("Waiting for epoll failed: %s (%d)", std::strerror(err), err);
        } //if
    } //if
    m_waiting = false;
    return result;
}


void EventWaitHandle::Abort() {
    if (m_waiting) {
        if (write(m_pipeFD[1], "x", 1) == -1 ) {
            int err = errno;
            throw Exception("Abort EventWaitHandle failed: %s (%d)", std::strerror(err), err);
        } //if
    } //if
}
