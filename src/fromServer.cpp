#include <fromServer.h>
#include <vector>
#include <connectionHandler.h>
#include <iostream>
#include <mutex>
#include <thread>
#include <string>
#include <ClientData.h>

using namespace std;

fromServer::fromServer(ConnectionHandler &ch, bool isConnected, ClientData &clientData):
        ch(ch),
        isConnected(isConnected),
        clientData(&clientData){
}

    void fromServer::operator()() {
        // we want to read as long as there is a connection to the server  string line;
        while (isConnected){
        string line;
        cout << "fromServer is operation" << endl;
        ch.getLine(line);
        cout<< flush << endl;
        std::vector<std::string> lines;
        split(lines, line, "\n");
        //gets command
        string command = lines[0];
        // gets headers
        bool head = true;
        int i = 1;
        map<string, string> headers;
        string body;
        string header;
        while (i<lines.size()-1){ //splits headers line to <key,value> map
            while(head){
                header = lines[i];
                if (header==(""))
                    head = false;
                int index = header.find(":");
                string key = header.substr(0, index);
                if (key!=""){
                    string value = header.substr(index + 1, header.length());
                    headers.emplace(key, value);
                }
                i++;
            }

            //gets body
            if (lines[i]!=("")){
                body = lines[i];
                cout << body << endl;
                break;
            }
            i++;
        }
        std::string newLine = "\n";

            if (command == "CONNECTED") {
                clientData->setConnected(true);
                cout << "Login successful" << endl;
             }
            if (command == "ERROR") {
                //extracts a message to print on the client's screen
                string message = headers.at("message");
                //prints message
                cout << message<< endl;
            }
            if (clientData->isConnected()&&command == "RECEIPT") {
                int receiptid = stoi(headers.at("receipt-id")); //add the same split algorithm as in the server side.
                string action = clientData->getAction(receiptid);
                vector<string> act;
                split(act, action, " ");
                if (act[1]==("join")){
                    string genre = act[2];
                    //adds genre to ClientData - topicsID and inventory
                    clientData->setSub(stoi(act[0]), genre);
                    //prints the required message on the client screen
                    cout << "Joined club " + genre << endl;
                } else if (act[0]==("Exited")){
                    string genre = act[1];
                    //deletes genre from all the DBs
                    clientData->exitClub(genre);
                    //prints the required message on the client screen
                    cout << "Exited club " + genre << endl;
                } else if (act[0]==("disconnect")){
                    //delete client's data
                    delete(clientData);
                    //changes connected to exit the loop and stop getting Server commands
                    isConnected = false;
                }
                else{
                }
            }
            if (clientData->isConnected()&& command == "MESSAGE") {
                cout << "got MESSAGE" << endl;
                string genre = headers.at("destination");
                vector<string> message;
                split(message, body, " ");
                //borrow wish message
                string book;
                while (i<message.size()) {
                    book = book +  message[i] + " ";
                    i++;
                }
                if (message.size()>3 && message[3]==("borrow")){
                    if (clientData->getName()!=(message[0])){
                        if(clientData->checkBookInventory(genre, message[4])) {
                            string frame = "SEND" + newLine + "destination:" + genre  + newLine+ newLine+
                                           clientData->getName() + " has " + book  +
                                           newLine + '\0';
                            (ch.sendLine(frame));
                        }
                    }
                }

                //someone has a wanted book
                else if(message.size() == 3 && message[1] =="has"){
                    string owner = message[0];
                    string book = message[2];
                    //if i'm the one with the book
                    if(clientData->getName() == owner){
                        clientData->lendBook(genre, book);
                    }
                    //check if this book is in my WL
                    else{
                        if (clientData->checkBookWL(genre, book)){
                            string frame = "SEND" + newLine + "destination:" + genre +  newLine + newLine +
                                           "Taking " +book+ " from " + owner  +
                                           newLine + '\0';
                            ch.sendLine(frame);
                            clientData->removeBookWL(genre, book);
                        }
                    }
                }
                //someone is returning a book
                else if (message[0]=="Returning"){
                    string book = message[1];
                    string lender = message[3];
                    if(lender == clientData->getName()){
                        string owner = clientData->getOwner(genre,book);
                        if (owner == clientData->getName()){
                            clientData->returnBooktoMe(genre,book);
                        } else{
                            string frame = "SEND" + newLine + "destination:" + genre + newLine + newLine+ "Returning " + book + " to" + "owner" + newLine + '\0';
                            //if succeed to send the frame, add the book to user's inventory
                            ch.sendLine(frame);
                            clientData->removeBookInventory(genre, book);
                        }
                    }
                }
                else if (message.size()>1 && message[1] == "status"){
                    string status = clientData->genreStatus(genre);
                    string frame = "SEND" + newLine + "destination:" + genre+ newLine + newLine + status + newLine + '\0';
                    ch.sendLine(frame);
                }
                else{
                    cout << "don't care" << endl;
                    message.clear();
                }

            }
          headers.clear();
        }
    }
//    void run() {
//        for (int i = 0; i < 100; i++) {
//            std::lock_guard<std::mutex>
//            lock(mutex); // constructor locks the mutex while destructor (out of scope) unlocks it
//            std::cout << i << ") Task " <</* _id << */" is working" << std::endl;
//        }
//    }

    void fromServer::split(std::vector<std::string> &vector, std::string s, std::string delimiter) {
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        vector.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    vector.push_back(s);
    }