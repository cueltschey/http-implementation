use std::env;
use std::io::{Read, Write};
use std::net::{TcpListener, TcpStream};
use std::str::FromStr;
use std::sync::Arc;
use std::thread;

mod http;

fn handle_client(mut stream: TcpStream) {
    let mut buffer = [0; 1024];
    if let Ok(bytes_read) = stream.read(&mut buffer) {
        let request = String::from_utf8_lossy(&buffer[..bytes_read]);
        let request = http::url_decode(&request);
        if let Some(filename) = http::extract_filename(&request) {
            let extension = http::file_ext_from_filename(&filename);
            println!("Requested file: {}, Extension: {:?}", filename, extension);

            let mut response = format!(
                "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n404 File Not Found: {},\n",
                filename
            );
            let root_dir: String = String::from(".");

            if http::file_exists(&filename, root_dir) {
                let mime: http::MimeType = http::get_mime_type(extension.unwrap_or("text/plain"));
                response = format!(
                    "HTTP/1.1 200 OK\r\nContent-Type: {}\r\nRequested file: {}\n",
                    mime.to_str(),
                    filename
                );
            }
            if let Err(e) = stream.write(response.as_bytes()) {
                eprintln!("Failed to send response: {}", e);
            }
        } else {
            eprintln!("Failed to parse filename from request");
        }
    }
}

fn main() -> std::io::Result<()> {
    // Get the port from command-line arguments
    let args: Vec<String> = env::args().collect();
    if args.len() < 2 {
        eprintln!("Usage: main <port>");
        std::process::exit(1);
    }

    let port: u16 = args[1].parse().expect("Invalid port number");

    // Create the server socket
    let address = format!("0.0.0.0:{}", port);
    let listener = TcpListener::bind(&address)?;
    println!("Listening on port: {}", port);

    // Handle incoming connections
    let listener = Arc::new(listener);
    for stream in listener.incoming() {
        match stream {
            Ok(stream) => {
                thread::spawn(move || {
                    handle_client(stream);
                });
            }
            Err(e) => {
                eprintln!("Failed to accept client: {}", e);
            }
        }
    }

    Ok(())
}
