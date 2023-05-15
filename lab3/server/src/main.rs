use std::io::{prelude::*, BufReader}; //prelude引入一堆trait
use std::net::{TcpListener, TcpStream};
use std::sync::mpsc; //Multi-producer, single-consumer

fn main() {
    let listener = TcpListener::bind("127.0.0.1:8000").unwrap(); //绑定监听端口

    for stream in listener.incoming() {
        let stream = stream.unwrap();
        handle_clnt(stream);
        println!("Connection established!");
    }

    let (tx, rx) = mpsc::channel();
    thread::spawn(move || {
        let val = String::from("hi");
        tx.send(val).unwrap();
    });
    let received = rx.recv().unwrap();
    println!("Got: {}", received);
}

fn handle_clnt(mut stream: TcpStream) {
    let buf_reader = BufReader::new(&mut stream); //接受流输入
    let http_request: Vec<_> = buf_reader
        .lines()
        .map(|result| result.unwrap()) //lines操作的result
        .take_while(|line| !line.is_empty()) //类似于filter
        .collect();

    println!("Request: {:#?}", http_request);
    /*
    Request: [
    "GET /hello.html HTTP/1.0",
    "Host: 127.0.0.1:8000",
    "User-Agent: curl/7.68.0",
     "Accept: ",
    ]
    */
}
