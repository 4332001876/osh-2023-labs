use std::cmp::Ordering;
use std::fs;
use std::io::{prelude::*, BufReader}; //prelude引入一堆trait
use std::net::{TcpListener, TcpStream};
use std::sync::mpsc; //Multi-producer, single-consumer

#[derive(Debug)]
struct RequestInfo {
    method: String,
    uri: String,
    protocol: String,
    host: String,
}
impl RequestInfo {
    fn new() -> Self {
        RequestInfo {
            method: String::from(""),
            uri: String::from(""),
            protocol: String::from(""),
            host: String::from(""),
        }
    }
}

fn main() {
    let listener = TcpListener::bind("127.0.0.1:8000").unwrap(); //绑定监听端口

    for stream in listener.incoming() {
        let stream = stream.unwrap();
        handle_clnt(stream);
        println!("Connection established!");
    }

    /*let (tx, rx) = mpsc::channel();
    thread::spawn(move || {
        let val = String::from("hi");
        tx.send(val).unwrap();
    });
    let received = rx.recv().unwrap();
    println!("Got: {}", received);*/
}

fn handle_clnt(mut stream: TcpStream) {
    let buf_reader = BufReader::new(&mut stream); //接受流输入
    let http_request: Vec<_> = buf_reader
        .lines()
        .map(|result| result.unwrap()) //lines操作的result
        .take_while(|line| !line.is_empty()) //类似于filter的trait行为, std::iter::Iterator::take_while
        .collect();

    /*println!("Request: {:#?}", http_request); //Vec<String>
    let http_request1 = vec![
        "GET /hello.html HTTP/1.0".to_string(),
        "Host: 127.0.0.1:8000".to_string(),
        "User-Agent: curl/7.68.0".to_string(),
        "Accept:... ".to_string(),
    ];
    let request_info = parse_request(&http_request1);
    println!("{:?}",request_info);*/
    let mut status_code = 200;
    /*200 OK 请求已成功，请求所希望的响应头或数据体将随此响应返回。
    404 Not Found 请求失败，请求所希望得到的资源未被在服务器上发现。
    500 Internal Server Error 本实验中未定义的各种错误。
    */

    let mut request_info = RequestInfo::new();
    match parse_request(&http_request) {
        Some(rq_info) => request_info = rq_info,
        None => status_code = 500, //500 Internal Server Error
    }
    println!("{:?}", request_info);
    let mut status_line = String::from("HTTP/1.0 ");
    let mut content_len = 0;
    let mut contents: Vec<u8> = Vec::new();
    if (status_code == 200) {
        let content_read = get_uri_content(&request_info.uri, "./webroot");
        //println!("{:?}", content_read);
        contents = match content_read {
            Ok(content) => {
                content_len = contents.len();
                content
            }
            Err(_) => {
                status_code = 404;
                Vec::new()
            }
        };
    }
    if (status_code == 200) {
        status_line.push_str("200 OK");
    } else if (status_code == 404) {
        status_line.push_str("404 Not Found");
    } else {
        status_line.push_str("500 Internal Server Error");
    }
    let response = format!("{status_line}\r\nContent-Length: {content_len}\r\n\r\n");
    stream
        .write_all(&[response.as_bytes(), &contents].concat())
        .unwrap();
    //println!("{:?}", &[response.as_bytes(), &contents].concat());
    //stream.write_all(&contents).unwrap();
    //bytes: &[u8]
    //vec.append(&mut vec2);
}
fn get_uri_content(uri: &str, web_root: &str) -> Result<Vec<u8>, std::io::Error> {
    let mut final_uri = String::from(web_root);
    final_uri.push_str(uri);
    let idx = (final_uri.len() - 1);
    //println!("{:?}", &final_uri[idx..]);
    //assert_eq!("l",&final_uri[idx..]);
    if &final_uri[idx..] == "/" {
        final_uri.push_str("index.html");
    }
    //println!("{:?}", final_uri);
    fs::read(final_uri)
}
fn parse_request(request_lines: &Vec<String>) -> Option<RequestInfo> {
    if request_lines.len() < 2 {
        return None;
    }
    let request: Vec<_> = request_lines[0].split(" ").collect(); //请求头
    if request.len() != 3 {
        return None;
    }

    let method_name;
    match &request[0].cmp("GET") {
        Ordering::Equal => method_name = &request[0],
        _ => return None, //not GET
    }

    let host_info: Vec<_> = request_lines[1].split("Host:").collect(); //请求头
                                                                       //println!("{:?}", host_info);
    if host_info.len() != 2 {
        return None;
    } //["", " 127.0.0.1:8000"]

    Some(RequestInfo {
        method: String::from(*method_name),
        uri: String::from(request[1]),
        protocol: String::from(request[2]),
        host: String::from(host_info[1].trim()),
    })
}
