#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <filesystem>
#include <vector>
#include <openssl/sha.h>
using namespace std;

namespace fs = filesystem;

// Function to initialize the repository
void init() {
    fs::path mygit_dir = ".mygit";
    fs::path objects_dir = mygit_dir / "objects";
    fs::path refs_dir = mygit_dir / "refs";
    fs::path head_file = mygit_dir / "HEAD";

    try {
        if (fs::exists(mygit_dir)) {
            cout << "Repository already initialized.\n";
            return;
        }

        fs::create_directory(mygit_dir);
        fs::create_directory(objects_dir);
        fs::create_directory(refs_dir);

        ofstream headFile(head_file);
        if (headFile) {
            headFile << "ref: refs/heads/master";
            headFile.close();
        } else {
            cerr << "Failed to create HEAD file.\n";
        }

        cout << "Initialized empty repository in " << fs::absolute(mygit_dir) << "\n";
    } catch (const fs::filesystem_error& e) {
        cerr << "Error initializing repository: " << e.what() << "\n";
    }
}

// Function to calculate SHA-1 hash of file contents
string calculateSHA1(const string& content) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(content.c_str()), content.size(), hash);

    stringstream sha1stream;
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        sha1stream << hex << setw(2) << setfill('0') << static_cast<int>(hash[i]);
    }
    return sha1stream.str();
}

// Function to read file contents
string readFileContent(const string& filePath) {
    ifstream file(filePath, ios::binary);
    if (!file) {
        throw runtime_error("Unable to open file: " + filePath);
    }
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Function to hash object and optionally write it as a blob
void hashObject(const string& filePath, bool write) {
    try {
        string content = readFileContent(filePath);
        string sha1 = calculateSHA1(content);

        cout << "SHA-1 hash: " << sha1 << "\n";

        if (write) {
            fs::path objectPath = ".mygit/objects/" + sha1;
            ofstream outFile(objectPath, ios::binary);
            if (!outFile) {
                throw runtime_error("Failed to create blob object file: " + objectPath.string());
            }
            outFile << content;
            outFile.close();
            cout << "File stored as blob at: " << objectPath << "\n";
        }
    } catch (const exception& e) {
        cerr << "Error in hash-object: " << e.what() << "\n";
    }
}

// Function to handle cat-file command
void catFile(const string& flag, const string& fileSha) {
    fs::path objectPath = ".mygit/objects/" + fileSha;
    
    if (!fs::exists(objectPath)) {
        cerr << "Object not found for SHA-1: " << fileSha << "\n";
        return;
    }

    if (flag == "-p") { // Print content
        try {
            string content = readFileContent(objectPath);
            cout << content << "\n";
        } catch (const exception& e) {
            cerr << "Error reading file content: " << e.what() << "\n";
        }
    } else if (flag == "-s") { // Display size
        error_code ec;
        auto size = fs::file_size(objectPath, ec);
        if (ec) {
            cerr << "Error reading file size: " << ec.message() << "\n";
        } else {
            cout << "File size: " << size << " bytes\n";
        }
    } else if (flag == "-t") { // Show type
        cout << "Object type: blob\n";
    } else {
        cerr << "Unknown flag: " << flag << "\n";
        cout << "Usage: ./mygit cat-file -p|-s|-t <file_sha>\n";
    }
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        string command = argv[1];
        
        if (command == "init") {
            init();
        } else if (command == "hash-object" && argc >= 3) {
            bool write = (argc == 4 && string(argv[2]) == "-w");
            string filePath = write ? argv[3] : argv[2];
            hashObject(filePath, write);
        } else if (command == "cat-file" && argc == 4) {
            string flag = argv[2];
            string fileSha = argv[3];
            catFile(flag, fileSha);
        } else {
            cout << "Usage:\n"
                      << "./mygit init\n"
                      << "./mygit hash-object [-w] <file_path>\n"
                      << "./mygit cat-file -p|-s|-t <file_sha>\n";
        }
    } else {
        cout << "Usage: ./mygit <command>\n";
    }
    return 0;
}
