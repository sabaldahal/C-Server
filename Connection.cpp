#include <iostream>
#include <string>
#include <mutex>
#include <winsock2.h>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include "GlobalMutex.h"
#include "GlobalValues.h"
//#include "User.cpp"
//change in the future to include .h files instead of .cpp
//at the moment be careful not to duplicate them

using namespace std;

//convert entire string to lowercase
string to_lower(string &str){
    string result = str;
    transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

//convert entire string to uppercase
string to_upper(string &str){
    string result = str;
    transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

//different types of request made by the client
enum Request {
        GET,
        UCMD,
        CONNECT,
        CREDENTIAL,
        JOIN,
        POST,
        USERS,
        LEAVE,
        MESSAGE,
        EXIT,
        GROUPS,
        GROUPJOIN,
        GROUPPOST,
        GROUPUSERS,
        GROUPLEAVE,
        GROUPMESSAGE,
        INVALID
};

//takes a string and returns the type of request
Request stringToRequest(string& str){
    static const std::unordered_map<std::string, Request> stringToRequestMap = {
        {"GET", Request::GET},
        {"UCMD", Request::UCMD},
        {"CONNECT", Request::CONNECT},
        {"CREDENTIAL", Request::CREDENTIAL},
        {"JOIN", Request::JOIN},
        {"POST", Request::POST},
        {"USERS", Request::USERS},
        {"LEAVE", Request::LEAVE},
        {"MESSAGE", Request::MESSAGE},
        {"EXIT", Request::EXIT},
        {"GROUPS", Request::GROUPS},
        {"GROUPJOIN", Request::GROUPJOIN},
        {"GROUPPOST", Request::GROUPPOST},
        {"GROUPUSERS", Request::GROUPUSERS},
        {"GROUPLEAVE", Request::GROUPLEAVE},
        {"GROUPMESSAGE", Request::GROUPMESSAGE}
    };

    string upperstr = to_upper(str);
    auto it = stringToRequestMap.find(upperstr);
    if (it != stringToRequestMap.end()) {
        return it->second;
    } else {
        return Request::INVALID;
    }
}

//Represents each unique connection with the client
class Connection{
    private:
        SOCKET clientSocket;
        User* clientuser = nullptr; //holds the information about the user
        string crlf = "\r\n";
        string doublecrlf = "\r\n\r\n";

    public:
    Connection(SOCKET clientSocket){
        this->clientSocket = clientSocket;
    }
    //checks if it is a HTTP request and processes it
    bool ProcessIfHTTPRequest(string req){
        //separate request line, header and body
        size_t pos = 0;
        string requestline;
        string header;
        string body;
        pos = req.find(crlf);
        if(pos != string::npos){
            requestline = req.substr(0, pos);
            istringstream iss(requestline);
            string word;
            vector<string> commands;
            while(iss >> word){
                commands.push_back(word);
            }
            if(commands.size() != 0){
                if(to_upper(commands[0]) == "GET"){
                    Get(commands);
                    return true;
                }                   
            }
        }
        return false;
    }

    //main function of the client connection
    void run(){
        char buffer[1024] = { 0 };
        int bytesReceived, bytesSent;
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        
        if(bytesReceived > 0){
            string req(buffer, bytesReceived);           
            if(ProcessIfHTTPRequest(req)){
                
            }else{
                //send success status for connecting to the server
                SendActionSuccessful();
                memset(buffer, 0, sizeof(buffer));
                //keep looping until the client connection is closed -- keep the connection active
                while((bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0){
                    string receiveddata(buffer, bytesReceived);
                    //separate request line, header and body
                    size_t pos = 0;
                    string requestline;
                    string header;
                    string body;
                    pos = receiveddata.find(crlf);
                    if(pos != string::npos){
                        requestline = receiveddata.substr(0, pos);
                        receiveddata.erase(0, pos + crlf.length());
                        if((pos = receiveddata.find(doublecrlf)) != string::npos){
                            header = receiveddata.substr(0, pos);
                            receiveddata.erase(0, pos + doublecrlf.length());
                            body = receiveddata;
                        }
                        istringstream iss(requestline);
                        string word;
                        vector<string> commands;
                        while(iss >> word){
                            commands.push_back(word);
                        }
                        if(commands.size() != 0){
                            ProcessRequest(requestline, header, body);
                        }
                    }
                    memset(buffer, 0, sizeof(buffer));
                }

            }
        
        }

        memset(buffer, 0, sizeof(buffer));
        cout << "closing connection with the client" << endl;
        CleanUp();
        closesocket(clientSocket);
    }

    void ProcessRequest(string request, string &header, string &body){
        istringstream iss(request);
        string word;
        vector<string> commands;
        while(iss >> word){
            commands.push_back(word);
        }
        if(commands.size() == 0){
            SendActionUnsuccessful();
            return;
        }

        string firstreq = to_upper(commands[0]);
        if(firstreq == "GET"){
            
        }else if(firstreq == "UCMD"){
            ProcessUCMDRequest(commands);
        }else{
            SendActionUnsuccessful();
            return;
        }
            
    }

    void ProcessUCMDRequest(vector<string> &commands){
        if(commands.size() < 2){
            SendActionUnsuccessful();
            return;
        }
        Request req;
        req = stringToRequest(commands[1]);
        if(req != CREDENTIAL && clientuser == nullptr){
            SendActionUnsuccessful("Credentials required before proceeding\r\n");
            return;
        }
        switch(req){
                case CREDENTIAL:
                    Credential(commands);
                    break;
                case JOIN:
                    Join(commands);
                    break;
                case POST:
                    Post(commands);
                    break;
                case USERS:
                    Users(commands);
                    break;
                case LEAVE:
                    Leave(commands);
                    break;
                case MESSAGE:
                    Message(commands);
                    break;
                case EXIT:
                    Exit();
                    break;
                case GROUPS:
                    Groups();
                    break;
                case GROUPJOIN:
                    GroupJoin(commands);
                    break;
                case GROUPPOST:
                    GroupPost(commands);
                    break;
                case GROUPUSERS:
                    GroupUsers(commands);
                    break;
                case GROUPLEAVE:
                    GroupLeave(commands);
                    break;
                case GROUPMESSAGE:
                    GroupMessage(commands);
                    break;
                default:
                    SendActionUnsuccessful();
                    break;
            }
    }

    void CleanUp(){
        //remove user from any groups if present
        //remove user from allclients
         Exit();
    }

    void Get(vector<string> &commands){
        if(commands.size() < 2){
            SendActionUnsuccessful("404 Not Found\r\n");
            return;
        }
        string filename = commands[1];
        filename = "./static" + filename;
        ifstream file(filename, ios::in | ios::binary);
        string response;
        string responseline;
        string header;
        string body;
        if(!file.is_open()) {
            responseline = "HTTP/1.1 404 Not Found" + crlf;
            header = "Content-type: " + GetContentType(filename) + doublecrlf;
            body = "File not found";
            response = responseline + header + body;
            SendActionUnsuccessful(response);
            return;
        }
        ostringstream fileContents;
        fileContents << file.rdbuf();
        file.close();
        stringstream make_response;
        make_response << "HTTP/1.1 200 OK\r\n";
        make_response << "Cache-Control: no-cache, private\r\n";
        make_response << "Content-Type: ";
        make_response << GetContentType(filename);
        make_response << crlf;
        make_response << "Content-Length: " << fileContents.str().size() << crlf;
        make_response << crlf;
        make_response << fileContents.str();
        response = make_response.str();
        SendActionSuccessful(response);
    }

    string GetContentType(string fileName){
        size_t dotPos = fileName.find_last_of('.');
        string fileext;
        if (dotPos != string::npos) {
            fileext = fileName.substr(dotPos);
        }else{
            return "application/octet-stream";
        }
        if(fileext == ".htm" || fileext == ".html") {
            return "text/html";
        }
        if(fileext == ".css") {
            return "text/css";
        }
        if(fileext == ".js") {
            return "text/javascript";
        }
        if(fileext == ".jpeg") {
            return "image/jpeg";
        }
        if(fileext == ".png") {
            return "image/png";
        }
        if(fileext == ".gif") {
            return "image/gif";
        }
        if(fileext == ".txt") {
            return "text/txt";
        }
        return "application/octet-stream";
    }

    void Credential(vector<string> &commands){
        if(clientuser != nullptr){
            SendActionUnsuccessful();
            return;
        }
        if(commands.size() < 3){
            SendActionUnsuccessful();
            return;
        }
        bool clientalreadyregistered = false;
        string username = commands[2];
        for(const auto& user : allclients){
            if(user->username == username){
                clientalreadyregistered = true;
                break;
            }
        }
        if(clientalreadyregistered){
            SendActionUnsuccessful();
            return;
        }
        clientuser = new User(username, clientSocket);
        {
            lock_guard<mutex> lock(gmutex);
            allclients.push_back(clientuser);
        }
        SendActionSuccessful();
    }

    void Join(vector<string> &commands){
        if(allgroups[0]->addUser(clientuser)){
            SendActionSuccessful();
            string listusers = allgroups[0]->retrieveUsers();
            SendActionSuccessful(listusers);
            string previoustwomessage = allgroups[0]->retrieveLastTwoMessage();
            SendActionSuccessful(previoustwomessage);

        }else{
            SendActionUnsuccessful("Could not add user to the group. User could already be present in the group\r\n");
        }
      
    }

    void Post(vector<string> &commands){
        if(commands.size() < 4){
            SendActionUnsuccessful("Invalid Arguments\r\n");
            return;
        }
        if(!allgroups[0]->isUserPresent(clientuser->username)){
            SendActionUnsuccessful("Access Denied\r\n");
            return;
        }
        string subject = commands[2];
        string body = commands[3];
        allgroups[0]->addMessage(clientuser->username, subject, body);
        SendActionSuccessful();
        
    }

    void Users(vector<string> &commands){
        if(!allgroups[0]->isUserPresent(clientuser->username)){
            SendActionUnsuccessful("Access Denied\r\n");
            return;
        }
        string listusers = allgroups[0]->retrieveUsers();
        SendActionSuccessful(listusers);
    }

    void Leave(vector<string> &commands){
        if(!allgroups[0]->isUserPresent(clientuser->username)){
            SendActionUnsuccessful("User is not in the group\r\n");
            return;
        }
        if(allgroups[0]->removeUser(clientuser)){
            SendActionSuccessful();
            return;
        }
        SendActionUnsuccessful();
    }

    void Message(vector<string> &commands){
        if(commands.size() < 3){
            SendActionUnsuccessful("Invalid Arguments\r\n");
            return;
        }
        if(!allgroups[0]->isUserPresent(clientuser->username)){
            SendActionUnsuccessful("Access Denied\r\n");
            return;
        }
        string messageid = commands[2];
        try{
            int msgid = stoi(messageid);
            string message = allgroups[0]->retrieveMessageById(msgid);
            SendActionSuccessful(message);
        }catch(...){
            SendActionUnsuccessful("Invalid Message ID\r\n");
        }

    }

    void Exit(){
        if(clientuser==nullptr){
            return;
        }
        for(auto grp : allgroups){
            grp->removeUser(clientuser);
        }
        auto it = find(allclients.begin(), allclients.end(), clientuser);
        if(it != allclients.end()){
            {
                lock_guard<mutex> lock(gmutex);
                allclients.erase(it);
            }
        }
        //finally deallocate the clientuser
        if(clientuser != nullptr){
            delete clientuser;
            clientuser = nullptr;
        }
    }

    void Groups(){
        string message = "Group Lists: \n";
        for(auto grp : allgroups){
            if(grp->id == 0){
                message += "GroupID: " + to_string(grp->id) + " --> Public Group\n";
            }else{
                message += "GroupID: " + to_string(grp->id) + "\n";
            }         
        }
        SendActionSuccessful(message);

    }

    void GroupJoin(vector<string> &commands){
        if(commands.size() < 3){
            SendActionUnsuccessful("Invalid Arguments\r\n");
            return;
        }
        string groupid = commands[2];
        int gid;
        try{
            gid = stoi(groupid);
        }catch(...){
            SendActionUnsuccessful("Invalid Arguments\r\n");
            return;
        }
        if(gid < 0 || gid > allgroups.size() -1){
            SendActionUnsuccessful("Invalid GroupID\r\n");
            return;
        }
        if(allgroups[gid]->addUser(clientuser)){
            SendActionSuccessful();
            string listusers = allgroups[gid]->retrieveUsers();
            SendActionSuccessful(listusers);
            string previoustwomessage = allgroups[gid]->retrieveLastTwoMessage();
            SendActionSuccessful(previoustwomessage);

        }else{
            SendActionUnsuccessful("Could not add user to the group. User could already be present in the group\r\n");
        }

    }

    void GroupPost(vector<string> &commands){
        if(commands.size() < 5){
            SendActionUnsuccessful("Invalid Arguments\r\n");
            return;
        }
        string groupid = commands[2];
        int gid;
        try{
            gid = stoi(groupid);
        }catch(...){
            SendActionUnsuccessful("Invalid Arguments\r\n");
            return;
        }
        if(gid < 0 || gid > allgroups.size() -1){
            SendActionUnsuccessful("Invalid GroupID\r\n");
            return;
        }
        if(!allgroups[gid]->isUserPresent(clientuser->username)){
            SendActionUnsuccessful("Access Denied\r\n");
            return;
        }
        string subject = commands[3];
        string body = commands[4];
        allgroups[gid]->addMessage(clientuser->username, subject, body);
        SendActionSuccessful();
    }

    void GroupUsers(vector<string> &commands){
        if(commands.size() < 3){
            SendActionUnsuccessful("Invalid Arguments\r\n");
            return;
        }
        string groupid = commands[2];
        int gid;
        try{
            gid = stoi(groupid);
        }catch(...){
            SendActionUnsuccessful("Invalid Arguments\r\n");
            return;
        }
        if(gid < 0 || gid > allgroups.size() -1){
            SendActionUnsuccessful("Invalid GroupID\r\n");
            return;
        }
        if(!allgroups[gid]->isUserPresent(clientuser->username)){
            SendActionUnsuccessful("Access Denied\r\n");
            return;
        }
        string listusers = allgroups[gid]->retrieveUsers();
        SendActionSuccessful(listusers);
    }

    void GroupLeave(vector<string> &commands){
        if(commands.size() < 3){
            SendActionUnsuccessful("Invalid Arguments\r\n");
            return;
        }
        string groupid = commands[2];
        int gid;
        try{
            gid = stoi(groupid);
        }catch(...){
            SendActionUnsuccessful("Invalid Arguments\r\n");
            return;
        }
        if(gid < 0 || gid > allgroups.size() -1){
            SendActionUnsuccessful("Invalid GroupID\r\n");
            return;
        }
        if(!allgroups[gid]->isUserPresent(clientuser->username)){
            SendActionUnsuccessful("Access Denied\r\n");
            return;
        }
        if(allgroups[gid]->removeUser(clientuser)){
            SendActionSuccessful();
            return;
        }
        SendActionUnsuccessful();

    }

    void GroupMessage(vector<string> &commands){
        if(commands.size() < 4){
            SendActionUnsuccessful("Invalid Arguments\r\n");
            return;
        }
        string groupid = commands[2];
        int gid;
        try{
            gid = stoi(groupid);
        }catch(...){
            SendActionUnsuccessful("Invalid Arguments\r\n");
            return;
        }
        if(gid < 0 || gid > allgroups.size() -1){
            SendActionUnsuccessful("Invalid GroupID\r\n");
            return;
        }
        if(!allgroups[gid]->isUserPresent(clientuser->username)){
            SendActionUnsuccessful("Access Denied\r\n");
            return;
        }

        string messageid = commands[3];
        try{
            int msgid = stoi(messageid);
            string message = allgroups[0]->retrieveMessageById(msgid);
            SendActionSuccessful(message);
        }catch(...){
            SendActionUnsuccessful("Invalid Message ID\r\n");
        }
    }

    void SendActionSuccessful(string msg = "211 OK\r\n"){
        send(clientSocket, msg.c_str(), msg.length(), 0);
    }

    void SendActionUnsuccessful(string msg = "915 Bad Request\r\n"){
        const char* message = msg.c_str();
        send(clientSocket, message, strlen(message), 0);
    }
};