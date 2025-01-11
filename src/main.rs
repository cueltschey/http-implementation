use std::env;
use std::fs::File;
use std::io::{Read, Write};
use std::net::{TcpListener, TcpStream};
use std::path::Path;
use std::str::FromStr;
use std::sync::Arc;
use std::thread;

mod http;

fn handle_client(mut stream: TcpStream) {
    let mut buffer = [0; 1024];

    // Read the incoming request
    if let Ok(bytes_read) = stream.read(&mut buffer) {
        let request = String::from_utf8_lossy(&buffer[..bytes_read]);

        // Parse the request to extract the filename
        let request = http::url_decode(&request);
        if let Some(filename) = http::extract_filename(&request) {
            let extension = http::file_ext_from_filename(&filename);
            println!("Requested file: {}, Extension: {:?}", filename, extension);

            // Default response for 404
            let mut response = format!(
                "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\n404 File Not Found: {}\n",
                filename
            );

            let root_dir: String = String::from(".");

            // Check if the requested file exists
            if http::file_exists(&filename, root_dir) {
                let mime: http::MimeType = http::get_mime_type(extension.unwrap_or("text/plain"));
                let path = Path::new(&filename);

                // Try to open the file and read its content
                if let Ok(mut file) = File::open(&path) {
                    let mut file_content = Vec::new();
                    if file.read_to_end(&mut file_content).is_ok() {
                        // Build the HTTP 200 OK response
                        response =
                            format!("HTTP/1.1 200 OK\r\nContent-Type: {}\r\n\r\n", mime.to_str());

                        // Write the header and the file content to the stream
                        if stream.write_all(response.as_bytes()).is_ok() {
                            if stream.write_all(&file_content).is_err() {
                                eprintln!("Failed to send file content");
                            }
                        } else {
                            eprintln!("Failed to send response header");
                        }
                        return; // Successfully handled the request
                    } else {
                        eprintln!("Failed to read from file");
                    }
                } else {
                    eprintln!("Failed to open file: {}", filename);
                }
            }

            // Write the 404 Not Found response if file was not found or couldn't be read
            if let Err(e) = stream.write_all(response.as_bytes()) {
                eprintln!("Failed to send 404 response: {}", e);
            }
        } else {
            eprintln!("Failed to parse filename from request");
        }
    } else {
        eprintln!("Failed to read from stream");
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
