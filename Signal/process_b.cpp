#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <thread>
#include <mutex>
#include <experimental/filesystem>

#include <csignal>
#include <sys/wait.h>

void signal_handler(int signum, siginfo_t* siginfo, void* context)
{
    if (signum == SIGUSR1)
    {
        std::cout << "Process B received SIGUSR1. Exiting..." << std::endl;
        exit(EXIT_SUCCESS);
    }
}

int main(int argc, char** argv)
{
    struct sigaction sig_act;
    sig_act.sa_flags = SA_SIGINFO;
    sig_act.sa_sigaction = signal_handler;
    sigemptyset(&sig_act.sa_mask);

    sigaction(SIGUSR1, &sig_act, nullptr);

    std::cout << "Process B started with PID: " << getpid() << std::endl;
    while (true)
    {
        pause();
    }
    return 0;
}