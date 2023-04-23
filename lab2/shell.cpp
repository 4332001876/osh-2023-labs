// IO
#include <iostream>
// std::string
#include <string>
// std::vector
#include <vector>
// std::string 转 int
#include <sstream>
// PATH_MAX 等常量
#include <climits>
// POSIX API
#include <unistd.h>
// wait
#include <sys/wait.h>
// getenv()
#include <cstdlib>
// 用于logging函数中的可变参数列表
#include <cstdarg>
// open()
#include <fcntl.h>
// strlen()
#include <cstring>

#define LOGGING_LEVEL 3 // 日志级别
#define DEBUGGING 1
#define INFO 2
#define WARNING 3

#define ERRNO_LIBRARY_FUN_FAILED 10
#define ERRNO_EXEC_FAIL 255

#define IS_PIPE 1
#define NOT_PIPE 0

typedef std::vector<std::string> command;
typedef std::vector<command> command_group;

command split(std::string s, const std::string &delimiter);
command_group command_grouping(command args, const std::string &delimiter);

void redirect(command &args);

void exec_command(command args, int is_pipe);
void external_command(command args, int is_pipe);

int main()
{
    // 不同步 iostream 和 cstdio 的 buffer
    std::ios::sync_with_stdio(false);
    // c++中cin，cout效率比较低，是因为先把要输出的东西存入缓冲区与C语言中的stdio同步后，再输出，导致效率降低，
    // 而这段语句的作用是取消缓冲区同步，直接使用，由此可节省时间，使效率与scanf与printf相差无几。
    // 但需要注意的一点是，因为取消与stdio的同步之后，就不建议再使用printf与scanf了，否则实际输出就会与预期不符。只能用cin与cout

    // 用来存储读入的一行命令
    std::string cmd;
    while (true)
    {
        // 打印提示符
        std::cout << "# ";

        // 读入一行。std::getline 结果不包含换行符。
        std::getline(std::cin, cmd);

        // 按空格分割命令为单词
        command args = split(cmd, " ");

        // 没有可处理的命令
        if (args.empty())
        {
            continue;
        }
        // 按管道分隔
        command_group cmd_grp = command_grouping(args, "|");

        /*if (LOGGING_LEVEL <= DEBUGGING)
        {
            int i, j;
            std::cout << cmd_grp.size() << "\n";
            for (i = 0; i < cmd_grp.size(); i++)
            {

                for (j = 0; j < cmd_grp[i].size(); j++)
                {
                    std::cout << cmd_grp[i][j].c_str() << " ";
                }
                std::cout << "\n";
            }
        }*/

        int read_fd; // 上一个管道的读端口，即该条命令的输入
        int cpgid;
        if (cmd_grp.size() == 1)
        {
            int old_stdin_fd = dup(STDIN_FILENO);
            int old_stdout_fd = dup(STDOUT_FILENO);
            redirect(cmd_grp[0]); // 重定向
            exec_command(cmd_grp[0], NOT_PIPE);
            dup2(old_stdin_fd, STDIN_FILENO);
            dup2(old_stdout_fd, STDOUT_FILENO);
            close(old_stdin_fd);
            close(old_stdout_fd);
        }
        else
        {
            for (int i = 0; i < cmd_grp.size(); i++)
            {
                int pipefd[2];               // 0为读出管道端口（接第i+1条指令），1为写入端口（接第i条指令）
                if (i != cmd_grp.size() - 1) // 最后一次循环中不创建管道
                {
                    int pipe_ret = pipe(pipefd); // 创建管道
                    if (pipe_ret < 0)
                    {
                        std::cout << "Failed to create pipe!\n";
                        exit(ERRNO_LIBRARY_FUN_FAILED);
                    }
                }

                int pid = fork();
                if (pid == 0) // 第i条命令
                {
                    if (i == 0)
                    {
                        setpgrp();
                    }
                    // 重定向输出
                    if (i != cmd_grp.size() - 1)
                    {
                        close(pipefd[0]); // 最后一条命令里pipe没有新建，故不需要关闭此端口
                        dup2(pipefd[1], STDOUT_FILENO);
                        close(pipefd[1]);
                    }

                    // 重定向输入
                    if (i != 0)
                    {

                        dup2(read_fd, STDIN_FILENO);
                        close(read_fd);
                    }

                    redirect(cmd_grp[i]); // 重定向，会覆盖管道的重定向

                    exec_command(cmd_grp[i], IS_PIPE);
                    exit(0);
                }
                if (i == 0)
                {
                    cpgid = pid;
                }
                setpgid(pid, cpgid);
                // 关闭父进程无用的管道端口
                if (i != 0)
                    close(read_fd); // 已分发给子进程，可关闭
                if (i != cmd_grp.size() - 1)
                {
                    read_fd = pipefd[0]; // 保存下一条命令需要的读端口
                    close(pipefd[1]);    // 已分发给子进程，可关闭
                }
            }
            tcsetpgrp(STDIN_FILENO, cpgid);
        }
        // 等待所有子进程结束
        while (wait(nullptr) != -1) // wait调用失败则返回-1，表示没有子进程
            ;
        tcsetpgrp(STDIN_FILENO, getpgrp());
    }
}

void redirect(command &args)
// 考虑了一条指令多个重定向的可能性，后面的重定向会覆盖前面的重定向
{
    int i;
    // args.erase(args.begin()+i);
    for (i = 0; i < args.size(); i++) // 这里args.size()是实时更新的，所以即使删了args元素也能保证正确性
    {
        if (args[i].length() == 0)
            continue;

        if (args[i].length() > 1 && args[i].substr(args[i].length() - 2) == ">>") // append
        {
            // fd字符串转数字
            std::stringstream num_stream(args[i].substr(0, args[i].length() - 2)); // attention:remember to revise when copy!
            int redirect_fd = 0;
            num_stream >> redirect_fd;

            int open_fd;
            if (i == args.size() - 1) // 重定向后不带参数则直接舍弃
            {
                args.erase(args.begin() + i);
                i--; // 和i++抵消，因为erase后下一回还要读取i位置的参数
                continue;
            }
            open_fd = open(args[i + 1].c_str(), O_WRONLY | O_CREAT | O_APPEND, S_IRWXU | S_IRWXG | S_IRWXO); // attention:remember to revise when copy!
            // 转换失败，则进行正常重定向
            if (open_fd < 0)
            {
                std::cout << "Failed to open file!\n";
            }
            else if (!num_stream.eof() || num_stream.fail())
            {
                dup2(open_fd, STDOUT_FILENO); // attention:remember to revise when copy!
            }
            else // 否则重定向文件符指向的文件
            {
                dup2(open_fd, redirect_fd); // redirect_fd如果不存在则会自动打开
            }
            close(open_fd);
            // 删除重定向涉及的两个参数
            args.erase(args.begin() + i);
            args.erase(args.begin() + i);
            i--; // 和i++抵消，因为erase后下一回还要读取i位置的参数
        }
        else if (args[i].substr(args[i].length() - 1) == ">") // write
        {
            // fd字符串转数字
            std::stringstream num_stream(args[i].substr(0, args[i].length() - 1)); // attention:remember to revise when copy!
            int redirect_fd = 0;
            num_stream >> redirect_fd;

            int open_fd;
            if (i == args.size() - 1) // 重定向后不带参数则直接舍弃
            {
                args.erase(args.begin() + i);
                i--; // 和i++抵消，因为erase后下一回还要读取i位置的参数
                continue;
            }
            open_fd = open(args[i + 1].c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO); // attention:remember to revise when copy!
            // 转换失败，则进行正常重定向
            if (open_fd < 0)
            {
                std::cout << "Failed to open file!\n";
            }
            else if (!num_stream.eof() || num_stream.fail())
            {
                dup2(open_fd, STDOUT_FILENO); // attention:remember to revise when copy!
            }
            else // 否则重定向文件符指向的文件
            {
                dup2(open_fd, redirect_fd); // redirect_fd如果不存在则会自动打开
            }
            close(open_fd);
            // 删除重定向涉及的两个参数
            args.erase(args.begin() + i);
            args.erase(args.begin() + i);
            i--; // 和i++抵消，因为erase后下一回还要读取i位置的参数
        }
        else if (args[i] == "<<") // EOF read，实现方式为管道
        {
            if (i == args.size() - 1) // 重定向后不带参数则直接舍弃
            {
                args.erase(args.begin() + i);
                i--; // 和i++抵消，因为erase后下一回还要读取i位置的参数
                continue;
            }

            int pipefd[2];               // 0为读出管道端口（接第i+1条指令），1为写入端口（接第i条指令）
            int pipe_ret = pipe(pipefd); // 创建管道
            if (pipe_ret < 0)
            {
                std::cout << "Failed to create pipe!\n";
                exit(ERRNO_LIBRARY_FUN_FAILED);
            }

            std::string str_content;

            std::string str;
            std::getline(std::cin, str);
            while (str != args[i + 1])
            {
                if (LOGGING_LEVEL <= DEBUGGING)
                {
                    std::cerr << str << "\n";
                    std::cerr << args[i + 1] << "\n";
                }
                str_content = str_content + str + "\n";
                std::getline(std::cin, str);
            }
            str_content = str_content + "\4";
            char *buf = (char *)malloc(2048 * sizeof(char));
            strcpy(buf, str_content.c_str());
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            write(pipefd[1], (void *)buf, strlen(buf));
            close(pipefd[1]);
            free(buf);
            args.erase(args.begin() + i);
            args.erase(args.begin() + i);
            i--; // 和i++抵消，因为erase后下一回还要读取i位置的参数
        }
        else if (args[i] == "<<<") // string read，实现方式为管道
        {
            if (i == args.size() - 1) // 重定向后不带参数则直接舍弃
            {
                args.erase(args.begin() + i);
                i--; // 和i++抵消，因为erase后下一回还要读取i位置的参数
                continue;
            }

            int pipefd[2];               // 0为读出管道端口（接第i+1条指令），1为写入端口（接第i条指令）
            int pipe_ret = pipe(pipefd); // 创建管道
            if (pipe_ret < 0)
            {
                std::cout << "Failed to create pipe!\n";
                exit(ERRNO_LIBRARY_FUN_FAILED);
            }
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);

            // ssize_t write(int fd, const void *buf, size_t count);
            std::string str_content = args[i + 1] + "\n\4";
            char *buf = (char *)malloc(2048 * sizeof(char));
            strcpy(buf, str_content.c_str());
            write(pipefd[1], (void *)buf, strlen(buf));
            close(pipefd[1]);
            free(buf);
            args.erase(args.begin() + i);
            args.erase(args.begin() + i);
            i--; // 和i++抵消，因为erase后下一回还要读取i位置的参数
        }
        else if (args[i].substr(args[i].length() - 1) == "<") // read
        {

            // fd字符串转数字
            std::stringstream num_stream(args[i].substr(0, args[i].length() - 1)); // attention:remember to revise when copy!
            int redirect_fd = 0;
            num_stream >> redirect_fd;

            int open_fd;
            if (i == args.size() - 1) // 重定向后不带参数则直接舍弃
            {
                args.erase(args.begin() + i);
                i--; // 和i++抵消，因为erase后下一回还要读取i位置的参数
                continue;
            }
            open_fd = open(args[i + 1].c_str(), O_RDONLY, S_IRWXU | S_IRWXG | S_IRWXO); // attention:remember to revise when copy!
            // 转换失败，则进行正常重定向
            if (open_fd < 0)
            {
                std::cout << "Failed to open file!\n";
            }
            else if (!num_stream.eof() || num_stream.fail())
            {
                dup2(open_fd, STDIN_FILENO); // attention:remember to revise when copy!
            }
            else // 否则重定向文件符指向的文件
            {
                dup2(open_fd, redirect_fd); // redirect_fd如果不存在则会自动打开
            }
            close(open_fd);
            // 删除重定向涉及的两个参数
            args.erase(args.begin() + i);
            args.erase(args.begin() + i);
            i--; // 和i++抵消，因为erase后下一回还要读取i位置的参数
        }
    }
}

void exec_command(command args, int is_pipe)
{
    // 退出指令，已完工
    if (args[0] == "exit")
    {
        if (args.size() <= 1)
        {
            exit(0);
        }

        // std::string 转 int
        std::stringstream code_stream(args[1]);
        int code = 0;
        code_stream >> code;

        // 转换失败
        if (!code_stream.eof() || code_stream.fail())
        {
            std::cout << "Invalid exit code\n";
            return;
        }

        exit(code);
    }

    if (args[0] == "pwd") // 打印当前目录
    {
        char pwd_buf[256] = {0};
        // char *getcwd(char *buf, size_t size);
        getcwd(pwd_buf, 255);
        // std::cout << pwd_buf << "\n";
        // ssize_t write(int fd, const void *buf, size_t count);
        write(STDOUT_FILENO, (void *)pwd_buf, strlen(pwd_buf));
        return;
    }

    if (args[0] == "cd")
    {
        // int chdir(const char *path);
        if (args.size() > 1)
        {
            chdir(args[1].c_str());
            // std::cout << "To be done!\n";
        }
        else
        {
            chdir(getenv("HOME"));
        }
        return;
    }

    if (args[0] == "wait") // 等待所有后台命令终止
    {
        // TODO:
        return;
    }

    external_command(args, is_pipe); // 处理外部命令
}

void external_command(command args, int is_pipe) // 处理外部命令
{
    if (args.empty())
    {
        return;
    }
    pid_t pid = fork();
    // command 转 char **
    char *arg_ptrs[args.size() + 1];
    for (auto i = 0; i < args.size(); i++)
    {
        arg_ptrs[i] = &args[i][0];
    }
    // exec p 系列的 argv 需要以 nullptr 结尾
    arg_ptrs[args.size()] = nullptr;

    if (pid == 0)
    {
        // 这里只有子进程才会进入
        // execvp 会完全更换子进程接下来的代码，所以正常情况下 execvp 之后这里的代码就没意义了
        // 如果 execvp 之后的代码被运行了，那就是 execvp 出问题了
        execvp(args[0].c_str(), arg_ptrs);
        if (is_pipe == NOT_PIPE)
        {
            setpgrp();
        }

        // 所以这里直接报错
        exit(ERRNO_EXEC_FAIL);
    }
    if (is_pipe == NOT_PIPE)
    {
        setpgid(pid, pid);
        tcsetpgrp(STDIN_FILENO, pid);
    }

    // 这里只有父进程（原进程）才会进入
    int ret = wait(nullptr);
    if (ret < 0)
    {
        std::cout << "wait failed";
    }
    if (is_pipe == NOT_PIPE)
    {
        tcsetpgrp(STDIN_FILENO, getpgrp());
    }
}

// 经典的 cpp string split 实现
// https://stackoverflow.com/a/14266139/11691878
command split(std::string s, const std::string &delimiter) // delimiter长度可以大于1
{
    command res;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos)
    {
        token = s.substr(0, pos);
        res.push_back(token);
        s = s.substr(pos + delimiter.length());
    }
    res.push_back(s);
    return res;
}

command_group command_grouping(command args, const std::string &delimiter) // 用于管道等指令分组情形
{
    command_group cmd_grp;
    command cmd = command();
    int i;
    for (i = 0; i < args.size(); i++)
    {
        if (args[i] != delimiter) // 非分隔符
        {
            cmd.push_back(args[i]);
        }
        else
        {
            cmd_grp.push_back(cmd);
            /*if (LOGGING_LEVEL <= DEBUGGING)
            {
                int i;
                std::cout << cmd.size() << "\n";
                for (i = 0; i < cmd.size(); i++)
                {

                    std::cout << cmd[i].c_str() << " ";
                }
                std::cout << "\n";
            }*/
            cmd = command();
        }
    }
    if (cmd.size() != 0)
        cmd_grp.push_back(cmd);
    return cmd_grp;
}
