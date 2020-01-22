
#include <connectionHandler.h>
#include <fromKB.h>
#include <ClientData.h>
#include <fromServer.h>

using namespace std;

int main() {
    vector<string> words;
    bool login = false;
    while (!login) {
        const short bufsize = 1024;
        char buf[bufsize];
        cin.getline(buf, bufsize);
        string line(buf);
  //       login 132.72.40.238:9999 naama 222
  //        login 127.0.0.1:9999 hillel 222
        if (line.substr(0, 5)==("login")) {
            bool wantedLogout = false;
            size_t pos = 0;
            std::string token;
            string delimiter = " ";
            while ((pos = line.find(delimiter)) != std::string::npos) {
                token = line.substr(0, pos);
                words.push_back(token);
                line.erase(0, pos + delimiter.length());
                words.push_back(line);
                login = true;
            }
        }
    }
    //login user
    string newLine = "\n";
    int endPos = words[1].find(":");
    string host = words[1].substr(0, endPos);
    string port = words[1].substr((endPos + 1));
    string frame = "CONNECT" + newLine + "accept-version:1.2" + newLine + "host: " + host + newLine + "login: " + words[4] + newLine + + "passcode: " + words[5] + newLine+newLine + '\0';
    //create ClientData with username and password
    ClientData clientData(words[4], words[5]);
    //create Connection handler with user's host and port
    ConnectionHandler connectionHandler(host, short(stoi(port)));
    connectionHandler.connect();
//    clientData.setConnected(true);

    mutex mutex;
    fromKB fromKb(connectionHandler, true, clientData);
    fromServer fromserver(connectionHandler, true, clientData);
    thread th1(fromKb);
    thread th2(fromserver);
    connectionHandler.sendLine(frame);
    th1.join();
    th2.join();

//    int main(){
//        std::mutex mutex;
//        Task task1(1, mutex);
//        Task task2(2, mutex);
//
//        std::thread th1(&Task::run, &task1);
//        std::thread th2(&Task::run, &task2);
//        th1.join();
//        th2.join();
//        return 0;


//
//        int *isConnected = new int(0);
//
//
//
//        if (!connectionHandler.connect()) { //handeling fail in connection
//            std::cerr << "Cannot connect to " << host << ":" << port << std::endl;
//            return 1;
//        }
//
//        fromKB readTask(&connectionHandler, isConnected);
//        std::thread th1(readTask);
//
//        while (1) {
//            Packet *p = connectionHandler.decodeNextMessage();
//            protocol.process(p);
//            delete p;
//            if (connectionHandler.IsDisconnectApproved()) {
//                break;
//            }
//        }
//
//        th1.join();
//        delete islogged;
//        return 0;



    return 0;
}