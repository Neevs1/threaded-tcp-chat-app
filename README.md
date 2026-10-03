# threaded-tcp-chat-app
A threaded chat application using TCP protocols

Uses Linux based POSIX sockets for communication

Initially used pthread, now uses C++ std::thread library due to type safe and object oriented features.

Initially mutex was used, now shifted to std::atomic due as it is more modern and gives better performance.

Threading is done to support concurrency. On server side, each client loop is handled by its own thread, while on the client side, typing and communication are handled by separate threads for seamless communication.

This was based on assignments carried out as a part of my SPPU 2019 Pattern LP-II OS and CNSL subjects.
