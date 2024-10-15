#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <algorithm>
#include <ctime>
#include "GlobalMutex.h"
#include "Message.cpp"


using namespace std;

//Stores information about the group: groupid, users present, all time users, all time messages
class Group{
    public:
        vector<User*> users{};
        vector<Message*> messages{};
        vector<string> cacheusers;
        int id;
        int currentmessageid = 1;
        Group(int id){
            this->id = id;
        }

        ~Group(){
            for(auto& msg : messages){
                delete msg;
                msg = nullptr;
            }
        }

        //add user and broadcast it to everyone
        bool addUser(User* user){
            if(isUserPresent(user->username)) return false;
            {                       
                lock_guard<mutex> lock(gmutex);
                users.push_back(user);
                user->addToGroup(id);
                string cachename = user->username;
                cacheusers.push_back(cachename);
            }
            string userjoinedmessage = "User " + user->username + " joined the group: Group ID: " + to_string(id) +"\r\n" ;
            BroadcastMessage(userjoinedmessage);
            return true;
        }

        //check if the user is present
        bool isUserPresent(string username){
            for(auto userptr : users){
                if(userptr->username == username){
                    return true;
                }
            }
            return false;
        }

        //add message and broadcast it to everyone in the group
        void addMessage(string username, string subject, string body){
            string dt = getDate();
            int mid = currentmessageid;
   
            Message* newmsg = new Message(mid, dt, username, subject, body);        
            {
                lock_guard<mutex> lock(gmutex);
                
                messages.push_back(newmsg);
                currentmessageid++;
            }
            string message = "Message on GroupID: " + to_string(id) + "\n" + to_string(mid) + " " + username + " " + dt + " " + subject + "\r\n";
            BroadcastMessage(message);
        }

        //retrieve the list of active users in the group
        string retrieveUsers(){
            string message = "Users in the group: GroupID: " + to_string(id) + "\n";
            for(auto usr : users){
                message += usr->username + "\r\n";
            }
            return message;
        }

        //retrieve the list of all users who have joined this group until now
        string retrieveAllNewAndOldUsers(){
            string message = "";
            for(auto str : cacheusers){
                message += str + "\r\n";
            }
            return message;
        }

        //retrieve the recent two messages
        string retrieveLastTwoMessage(){
            string message = "Last two messages: GroupID: " + to_string(id) + "\n";
            int len = messages.size();
            if(len > 1){
                message += "Message on GroupID: " + to_string(id) + "\n" + to_string(messages[len-2]->id) + " " + messages[len-2]->name + " " + messages[len-2]->date + " " + messages[len-2]->subject + "\r\n";
                message += "Message on GroupID: " + to_string(id) + "\n" + to_string(messages[len-1]->id) + " " + messages[len-1]->name + " " + messages[len-1]->date + " " + messages[len-1]->subject + "\r\n";
            }else if(len == 1){

                message += "Message on GroupID: " + to_string(id) + "\n" + to_string(messages[len-1]->id) + " " + messages[len-1]->name + " " + messages[len-1]->date + " " + messages[len-1]->subject + "\r\n";
            }else{
                message += "\r\n";
            }
            return message;
        }

        //retrieve message by ID
        string retrieveMessageById(int mid){
            string message = "Message on GroupID: " + to_string(id) + " with message ID: " + to_string(mid) + ":\n";
            for(auto& msg : messages){
                if(msg->id == mid){
                    message += msg->body + "\r\n";
                    return message;
                }
            }
            return "Message Not Found\r\n";
        }

        //Broadcast Message to All users in this group
        void BroadcastMessage(string msg){
                const char* message = msg.c_str();
                for(auto usr : users){
                    send(usr->usersocket, message, strlen(message), 0);
                }
        }

        //remove user from the group the remove the association of the group from user
        bool removeUser(User* user){
            vector<User*>::iterator it;
            it = find(users.begin(), users.end(), user);
            if(it != users.end()){
                if(user->removeFromGroup(id)){
                    {
                        lock_guard<mutex> lock(gmutex);
                        users.erase(it);
                    }
                    string userleftmessage = "User " + user->username + " left the group: Group ID: " + to_string(id) +"\r\n" ;
                    BroadcastMessage(userleftmessage);                  
                    return true;
                }                                 
            }
            return false;
        }

        //get current date
        string getDate(){
            time_t now = time(0);
            char* dt = ctime(&now);
            string str(dt);
            return str;
        }



};