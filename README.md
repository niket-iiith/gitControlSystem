# download openssl libraries 
- sudo apt-get install libssl-dev
# compile
g++ -std=c++17 mygit.cpp -o mygit -lssl -lcrypto
# init - initialises an empty folder
./mygit init
# 