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

int main(int argc, char** argv)
{
    std::cout << "Process A started with PID: " << getpid() << std::endl;
    while (true)
    {
        pause();
    }
    return 0;
}