#include <fromKB.h>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <string>
#include <ClientData.h>

using namespace std;


fromKB::fromKB(ConnectionHandler &ch, bool isConnected, ClientData &clientData):
        ch(ch),
        isConnected(isConnected),
        clientData(&clientData){
}


    void fromKB::operator()() {
        while (isConnected) {// we wants to read as long as there is a connection to the server
            string line;
            getline(cin, line);

            std::vector<std::string> words;
            split(words, line, " ");
            std::string newLine = "\n";
            if (words[0] == "login") {
                int endPos = words[1].find(":");
                string host = words[1].substr(0, endPos);
                string port = words[1].substr((endPos + 1));
                string frame = "CONNECT" + newLine + "accept-version:1.2" + newLine + "host: " + host + newLine + "login: " + words[4] + newLine + + "passcode: " + words[5] + newLine+newLine + '\0';
                ch.sendLine(frame);
            }
            if (clientData->isConnected() && words[0] == "join") {
                int receiptid = clientData->getReceiptID();
                int subid = clientData->getSubID();
                string action = to_string(subid) + " " + "join" + " " + words[1];
                string frame =
                        "SUBSCRIBE" + newLine + "destination:" + words[1] + newLine + "id: " + to_string(subid) +
                        newLine + +"receipt:" + to_string(receiptid) + newLine + '\0';

                ch.sendLine(frame);
                    clientData->addReceipt(receiptid, action);

            }
            if (clientData->isConnected() && words[0] == "exit") {
                string genre = words[1];
                int subID = clientData->getGenreSubID(genre);
                //create SUBSCRIBE frame
                string frame = "UNSUBSCRIBE" + newLine + "id:" + to_string(subID) + newLine + '\0';
                //if succeed to send the frame, add an action for getting SUBSCRIBED frame from the server
                ch.sendLine(frame);
                    clientData->addReceipt(clientData->getReceiptID(), "Exited " + genre);

            }
            if (clientData->isConnected() && words[0] == "add") {
                string genre = words[1];
                int i = 2;
                string book;
                while (i<words.size()) {
                    book = book +  words[i] + " ";
                    i++;
                }
                string name = clientData->getName();
                //create SEND frame
                string frame = "SEND" + newLine + "destination:" + genre + newLine + newLine + name +
                               " has added the book " + book + newLine +  newLine + '\u0000';
                //if succeed to send the frame, add the book to user's inventory
                ch.sendLine(frame);
                clientData->addBook(genre, book, name);

            }
            if (clientData->isConnected() && words[0] == "borrow") {
                string genre = words[1];
                string book = words[2];
                string name = clientData->getName();
                //create SEND frame
                string frame =
                        "SEND" + newLine + "destination:" + genre + newLine + newLine + name + " wish to borrow " +
                        book + newLine + '\0';
                //if succeed to send the frame, add the book to user's inventory
                ch.sendLine(frame);
                    clientData->addToWL(genre, book);

            }
            if (clientData->isConnected() && words[0] == "return") {
                string genre = words[1];
                string book = words[2];
                string name = clientData->getName();
                string owner = clientData->getInventory().at(genre).at({book, true});
                //create SEND frame
                string frame = "SEND" + newLine + "destination:" + genre + newLine + newLine + "Returning " + book +
                               " to" + "owner" + newLine + '\0';
                //if succeed to send the frame, add the book to user's inventory
                ch.sendLine(frame);
                    clientData->removeBookInventory(genre, book);

            }
            if (words[0] == "status") {
                string genre = words[1];
                string name = clientData->getName();
                //create SEND frame
                string frame =
                        "SEND" + newLine + "destination:" + genre + newLine + newLine + "book status" + newLine +
                        '\0';
                ch.sendLine(frame);
            }
            if (words[0] == "logout") {
                cout << "i'm loging out" << endl;
                int receiptid = clientData->getReceiptID();
                string frame = "DISCONNECT" + newLine + "receipt:" + to_string(receiptid) + newLine + '\0';
                ch.sendLine(frame);
                    //add receiptID and action to act when getting a receipt with this id.
                    clientData->addReceipt(receiptid, "disconnect");
                    //update client's connection status for stop getting KB commands
                    clientData->setConnected(false);
                    isConnected = false;
            }
            line.clear();
        cout << "while finished" << endl;
        }
    }


    void fromKB::split(std::vector<std::string> &vector, std::string s, std::string delimiter) {
        size_t pos = 0;
        std::string token;
        while ((pos = s.find(delimiter)) != std::string::npos) {
            token = s.substr(0, pos);
            vector.push_back(token);
            s.erase(0, pos + delimiter.length());
        }
        vector.push_back(s);
    }


//    void run() {
//            for (int i = 0; i < 100; i++) {
//                std::lock_guard<std::mutex>
//                lock(mutex); // constructor locks the mutex while destructor (out of scope) unlocks it
//                std::cout << i << ") Task " << /*_id << */" is working" << std::endl;
//            }
//    }






