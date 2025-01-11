use std::fs;

pub enum MimeType {
    MIME_TEXT_HTML,
    MIME_TEXT_PLAIN,
    MIME_IMAGE_JPEG,
    MIME_IMAGE_PNG,
    MIME_APPLICATION_JSON,
    MIME_APPLICATION_XML,
    MIME_APPLICATION_OCTET,
}

pub fn file_ext_from_filename(filename: &str) -> Option<&str> {
    let dot = filename.rfind('.')?;
    if dot == filename.len() {
        None
    } else {
        Some(&filename[dot..])
    }
}

pub fn get_mime_type(file_ext: &str) -> MimeType {
    match file_ext {
        "html" | "htm" => MimeType::MIME_TEXT_HTML,
        "txt" => MimeType::MIME_TEXT_PLAIN,
        "jpg" | "jpeg" => MimeType::MIME_IMAGE_JPEG,
        "png" => MimeType::MIME_IMAGE_PNG,
        _ => MimeType::MIME_APPLICATION_OCTET,
    }
}

fn file_exists(file_path: &str, root_dir: &str) -> bool {
    let dir = fs::read_dir(root_dir);
    if let Err(e) = dir {
        eprintln!("Error opening directory: {}", e);
        return false;
    }

    for entry in dir.unwrap() {
        if let Ok(entry) = entry {
            if let Some(name) = entry.file_name().to_str() {
                if name == file_path {
                    return true;
                }
            }
        }
    }

    false
}

fn url_decode(src: &str) -> String {
    let mut decoded = Vec::with_capacity(src.len());
    let bytes = src.as_bytes();

    let mut i = 0;
    while i < bytes.len() {
        if bytes[i] == b'%' && i + 2 < bytes.len() {
            if let Ok(hex_val) = u8::from_str_radix(&src[i + 1..i + 3], 16) {
                decoded.push(hex_val);
                i += 3;
                continue;
            }
        }
        decoded.push(bytes[i]);
        i += 1;
    }

    String::from_utf8(decoded).unwrap_or_else(|_| String::new())
}
