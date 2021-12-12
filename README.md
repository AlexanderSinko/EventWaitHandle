# EventWaitHandle

## About

This class allows threads to communicate with each other by signaling.

An instance can hold a thread for a given number of milliseconds or even indefinitely. The thread will continue after timeout reached or waiting is aborted. An abort is triggered by calling the method `Abort()` or by system sending an interrupted (e.g. SIGTERM).

__Remark:__ Not tested in Windows environment.

## Usage

Create an instance of the class for each position the thread should be holden or decelerated. Insert at the position a `Wait(…)` with a given timeout (or constant `INDEFINITELY`). Insert an `Abort()` at every position the waiting thread should be told to continue. Beware, if no Wait is ongoing, the signal will disappear. Therefore, a call of `Abort()` beforehand a `Wait(INDEFINITELY)` may occur a deadlock!

### Attention

There may occur a __deadlock__, if `Abort()` is called before a `Wait(INDEFINITELY)` is reached.

### Example 1

A simple example for handling received messages.

Keep in mind, if new messages received during `ProcessMessages(…)` is working, the abort may be called before program reaches `_EventWaitHandle.Wait(…)`. Therefore, it's more safety to use a timeout and check for empty queue. Another option is to use an additional mutex to block `Abort()` as long the Process-Loop is running.

``` C++
class MessageHandler {
    public:
        void Process();
        void Stop();
    
        // To be called from other thread
        void Callback_MessageReceived(Message* newMessage);
    
    private:
        volatile bool        _StopSignal;
        std::queue<Message*> _MessageQueue;
        std::mutex           _MessageQueue_Mutex;
}; //class
```

``` C++
void MessageHandler::Process() {    
    while(!_StopSignal) {
        // […]
        ProcessMessages(_MessageQueue);
        // […]
        _EventWaitHandle.Wait(_EventWaitHandle::INDEFINITELY);
    } //while
}
    
void MessageHandler::Callback_MessageReceived(Message* newMessage) {
    _MessageQueue_Mutex.lock();
    _MessageQueue.Add(newMessage);
    _MessageQueue_Mutex.unlock();
    _EventWaitHandle.Abort();  //Run Process-loop to handle new message;
}

void MessageHandler::Stop() {
    _StopSignal = true;
   _EventWaitHandle.Abort(); //Run Process-loop
}
```

### Example 2

A simple example to show how to work with timeouts.

``` C++
    // […]
    if (_EWH_ReceivedAcknowledgement(5000)) {
        // got acknowledgement
        // […]
    } else {
        // timeout reached
        // […]
    }
    // […]
```

## License

[MIT](https://choosealicense.com/licenses/mit/)

Copyright (c) 2018 – Alexander Sinko