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

#define HEARTBEAT_THRESHOLD (3)

struct ProcessInfo
{
    pid_t pid;
    std::chrono::time_point<std::chrono::steady_clock> last_heartbeat;
};

std::map<std::string, ProcessInfo> process_table;
std::mutex process_table_mut;

void signal_handler(int signum, siginfo_t* siginfo, void* context)
{
    std::cout << "Received signal: " << signum << " from PID: " << siginfo->si_pid << std::endl;
    if (signum == SIGUSR1)
    {
        std::lock_guard<std::mutex> lock(process_table_mut);
        for (auto& process : process_table)
        {
            if (process.second.pid == siginfo->si_pid)
            {
                process.second.last_heartbeat = std::chrono::steady_clock::now();
                break;
            }
        }
    }
    else if (signum == SIGCHLD)
    {
        int status;
        pid_t pid = waitpid(-1, &status, WNOHANG);
        if (pid > 0)
        {
            std::lock_guard<std::mutex> lock(process_table_mut);
            for (auto it = process_table.begin(); it != process_table.end();)
            {
                if (it->second.pid == pid)
                {
                    it = process_table.erase(it);
                }
                else
                {
                    ++it;
                }
            }
        }
    }
    else
    {
        std::cerr << "Received unknown signal: " << signum << std::endl;
    }
}

void start_process(const std::string& process_name)
{
    std::cout << "Starting process: " << process_name << std::endl;
    pid_t pid = fork();
    if (pid == 0)
    {
        execl(process_name.c_str(), process_name.c_str(), nullptr);
        std::cerr << "Failed to exec process: " << process_name << std::endl;
        exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    {
        ProcessInfo process_info;
        process_info.pid = pid;
        process_info.last_heartbeat = std::chrono::steady_clock::now();

        std::lock_guard<std::mutex> lock(process_table_mut);
        process_table[process_name] = process_info;

        std::cout << "Started process: " << process_name << " with PID: " << pid << std::endl;
    }
    else
    {
        std::cerr << "Failed to start process: " << process_name << std::endl;
    }
}

void init_processes()
{
    std::cout << "Initializing processes..." << std::endl;
    std::vector<std::string> processes = { "process_a", "process_b", "process_c" };
    for (const auto& process_name : processes)
    {
        if (std::experimental::filesystem::exists(process_name))
        {
            start_process(process_name);
        }
    }
}

void monitor_processes()
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        auto current_time = std::chrono::steady_clock::now();

        std::lock_guard<std::mutex> lock(process_table_mut);
        for (auto it = process_table.begin(); it != process_table.end();)
        {
            std::string process_name = it->first;
            ProcessInfo process_info = it->second;
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(current_time - process_info.last_heartbeat).count();
            if (duration > HEARTBEAT_THRESHOLD)
            {
                std::cout << "Process: " << process_name << " with PID: " << process_info.pid << " is not responding. Restarting..." << std::endl;
                kill(process_info.pid, SIGKILL);

                int status;
                waitpid(process_info.pid, &status, 0);

                it = process_table.erase(it);
                start_process(process_name);
            }
            else
            {
                ++it;
            }
        }
    }
}

int main(int argc, char** argv)
{
    struct sigaction sig_act;
    sig_act.sa_flags = SA_SIGINFO | SA_NOCLDWAIT;
    sig_act.sa_sigaction = signal_handler;
    sigemptyset(&sig_act.sa_mask);

    sigaction(SIGUSR1, &sig_act, nullptr);
    sigaction(SIGCHLD, &sig_act, nullptr);

    init_processes();

    std::thread monitor_thread(monitor_processes);
    std::cout << "System service started..." << std::endl;
    monitor_thread.detach();

    while (true)
    {
        pause();
    }

    return 0;
}