#ifndef EventWaitHandle_H
#define EventWaitHandle_H

#include <atomic>

/**
 *  \brief     The class allows threads to communicate with each other by signaling.
 *  \details   An instance can hold the thread for a given number of milliseconds
 *             or even indefinitely. The thread will continue after timeout reached 
 *             or waiting is aborted. An abort is triggered by calling the method 
 *             Abort() or  by system sending an interrupted (e.g. SIGTERM).
 *  \author    Alexander Sinko
 *  \date      2018-2021
 *  \copyright 2021 by Alexander Sinko
 *  \license   MIT (https://choosealicense.com/licenses/mit/)
 */
class EventWaitHandle {
    public:
        EventWaitHandle();
        virtual ~EventWaitHandle();
        
        /**
         * Check if instance is currently waiting.
         * 
         * @return true, if Wait() is ongoing
         */
        bool IsWaiting();
        
        
        /**
         * Wait for an abort signal the given number of milliseconds (or
         * indefinitely). Abort signal can be send by using methode Abort() or
         * from system by sending an interrupt (e.g. SIGTERM).
         * 
         * @param timeout_ms Timeout in milliseconds to wait before return. Use
         *        the constant INDEFINITELY for waiting indefinitely.
         * @return true if got an abort signal, false if timeout reached
         */
        bool Wait(int timeout_ms = INDEFINITELY);
        
        
        /**
         * Sending an abort signal to method Wait(…). Beware, if no Wait is
         * ongoing, the signal will disappear. This may occur a deadlock if
         * Wait afterwards is called with timeout of indefinitely.
         */
        void Abort();
        
        static const int INDEFINITELY = -1; ///< Const for methode Wait(…) to indefinitely
        
    private:
        std::atomic<bool> m_waiting;
        int m_epollFD;   ///< file descriptor for epoll
        int m_pipeFD[2]; ///< file descriptors for pipes [0]=read, [1]=write

};

#endif /* WAITEVENTHANDLE_H */
