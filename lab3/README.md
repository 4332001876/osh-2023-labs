# OSH lab3
**罗浩铭PB21030838**

## 编译和运行方法说明
直接在项目路径内打开terminal，运行指令`cargo run`即可编译运行，按ctrl+C可直接中止
此处实现了异步IO，若要使用multi_thread版本，请将`main_multi_thread.rs`改名为`main.rs`再编译运行

注意：网络资源的根目录为：lab3\server\webroot！！！！！

打开另一个terminal，用于输入HTTP请求测试指令，便可以看到HTTP响应结果：
<img src="./pic/open page success 3.png" width="90%">


### 使用注意
以"/"结尾的网址会默认在后面补上"index.html"
如："https://osh-2023.github.io/lab3/score/" => "https://osh-2023.github.io/lab3/score/index.html"


## 整体设计与所用技术
### 建立连接
Rust提供的std::net提供了非常方便的建立连接的方式，将一个TcpListener与监听端口绑定,并从中接受TcpStream进行处理即可。
### 处理请求
用户输入被读入后，在parse_request()函数里先解析请求参数。函数使用字符串的split函数对请求前两行进行解析，并将参数存于RequestInfo结构体中，若请求格式错误或不为GET请求则函数立即中断并返回Option::None.
若参数解析正常，将根据参数的uri读取请求的文件，并将其附在HTTP响应之后传回给客户端。HTTP响应根据状态码情况，读取内容长度来设定参数。

### 错误处理
由于rust的语言特性本身，其编译器就能保证只要通过编译，程序中出现的错误将会极少，则我们需要检查的地方不多，主要集中在可能出错的文件读写，以及用户输入格式不规范方面。请求格式错误或不为GET请求则设错误状态码500，否则读入资源失败（即找不到资源）则设错误状态码404，再将状态码返回给客户端。

### 多线程并发
rust的多线程开发十分方便，并且rust的所有权的特性也使得线程间通信十分方便。
只需用`let handle = thread::spawn(move || handle_clnt(stream));`语句就可以新建线程处理一个请求。
此处将所有handle装入一个Vec中，并在程序最后用`handle.join()`语句阻塞程序以等待所有线程执行完毕，否则程序会在某些线程未执行完时退出，并会导致这些线程直接退出。

### 使用async/await
将所有必要的std库改为async_std库，给耗时较长的函数加上async标识符，并在所有异步函数调用的返回值后加上.await.unwrap()，并对少量与异步类型不匹配的代码进行修改，即可将程序变为异步版本。

## 使用 siege 测试的结果和分析
测试使用我的个人主页与一个3.4MB大小的Linux内核镜像bzImage。
### 测试1
测试方式为以并发数50测试10次。
对多线程版本进行测试的结果如下：
个人主页：
<img src="./pic/siege%20html%20simple%20thread.png" width="90%">
bzImage：
<img src="./pic/siege%20bzimage%20simple%20thread.png" width="90%">
对异步版本进行测试的结果如下：
个人主页：
<img src="./pic/siege%20html%20async.png" width="90%">
bzImage：
<img src="./pic/siege%20bzimage%20async.png" width="90%">
可以看出异步版本的测试结果稍慢于

### 测试2







