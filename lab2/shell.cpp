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

std::vector<std::string> split(std::string s, const std::string &delimiter);
void external_command(std::vector<std::string> args);

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
        std::vector<std::string> args = split(cmd, " ");

        // 没有可处理的命令
        if (args.empty())
        {
            continue;
        }

        // 退出指令，已完工
        if (args[0] == "exit")
        {
            if (args.size() <= 1)
            {
                return 0;
            }

            // std::string 转 int
            std::stringstream code_stream(args[1]);
            int code = 0;
            code_stream >> code;

            // 转换失败
            if (!code_stream.eof() || code_stream.fail())
            {
                std::cout << "Invalid exit code\n";
                continue;
            }

            return code;
        }

        if (args[0] == "pwd") // 打印当前目录
        {
            char pwd_buf[256];
            // char *getcwd(char *buf, size_t size);
            getcwd(pwd_buf, 255);
            std::cout << pwd_buf << "\n";
            continue;
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
            continue;
        }

        // 处理外部命令
        external_command(args);
    }
}

void external_command(std::vector<std::string> args)
{
    pid_t pid = fork();
    // std::vector<std::string> 转 char **
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

        // 所以这里直接报错
        exit(255);
    }

    // 这里只有父进程（原进程）才会进入
    int ret = wait(nullptr);
    if (ret < 0)
    {
        std::cout << "wait failed";
    }
}

// 经典的 cpp string split 实现
// https://stackoverflow.com/a/14266139/11691878
std::vector<std::string> split(std::string s, const std::string &delimiter) // delimiter长度可以大于1
{
    std::vector<std::string> res;
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
