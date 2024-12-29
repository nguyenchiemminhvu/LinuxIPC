#include <unistd.h>
#include <iostream>
#include <string>
#include <vector>
#include <queue>
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
    std::string process_name;
    std::chrono::time_point<std::chrono::steady_clock> last_heartbeat;
};

std::map<pid_t, ProcessInfo> process_table;
std::queue<ProcessInfo> process_queue;
std::mutex process_table_mut;

void signal_handler(int signum, siginfo_t* siginfo, void* context)
{
    std::cout << "Received signal: " << signum << " from PID: " << siginfo->si_pid << std::endl;
    if (signum == SIGUSR1)
    {
        std::lock_guard<std::mutex> lock(process_table_mut);
        for (auto& process : process_table)
        {
            if (process.first == siginfo->si_pid)
            {
                process.second.last_heartbeat = std::chrono::steady_clock::now();
                break;
            }
        }
    }
    else if (signum == SIGINT)
    {
        std::cout << "Shutting down system service..." << std::endl;
        std::lock_guard<std::mutex> lock(process_table_mut);
        for (const auto& process : process_table)
        {
            kill(process.first, SIGTERM);
        }
        exit(EXIT_SUCCESS);
    }
    else if (signum == SIGCHLD)
    {
        int status;
        waitpid(siginfo->si_pid, &status, 0);
        std::cout << "Process with PID: " << siginfo->si_pid << " has exited." << std::endl;
        std::lock_guard<std::mutex> lock(process_table_mut);
        process_table.erase(siginfo->si_pid);
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
        process_info.process_name = process_name;
        process_info.last_heartbeat = std::chrono::steady_clock::now();

        process_table[pid] = process_info;

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
    for (const auto& entry : std::experimental::filesystem::directory_iterator("."))
    {
        if (std::experimental::filesystem::is_regular_file(entry.path())
         && (std::experimental::filesystem::status(entry.path()).permissions() & std::experimental::filesystem::perms::owner_exec) == std::experimental::filesystem::perms::owner_exec)
        {
            std::string process_name = entry.path().filename().string();
            if (process_name.find("process_") != std::string::npos)
            {
                process_queue.push({process_name, std::chrono::steady_clock::now()});
            }
        }
    }
}

void monitor_processes()
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        auto current_time = std::chrono::steady_clock::now();
        std::cout << "tick..." << std::endl;

        std::lock_guard<std::mutex> lock(process_table_mut);
        for (auto it = process_table.begin(); it != process_table.end();)
        {
            ProcessInfo process_info = it->second;
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(current_time - process_info.last_heartbeat).count();
            if (duration > HEARTBEAT_THRESHOLD)
            {
                process_queue.push(process_info);

                std::cout << "Process: " << process_info.process_name << " with PID: " << it->first << " is not responding. Restarting..." << std::endl;
                kill(it->first, SIGKILL);

                int status;
                waitpid(it->first, &status, 0);
                
                it = process_table.erase(it);
            }
            else
            {
                ++it;
            }
        }

        // Restarting processes
        while (!process_queue.empty())
        {
            ProcessInfo process_info = process_queue.front();
            process_queue.pop();
            start_process(process_info.process_name);
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
    sigaction(SIGINT, &sig_act, nullptr);

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