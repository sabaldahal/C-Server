#include <iostream>
#include<vector>
#include<algorithm>
#include <winsock2.h>
#include "GlobalMutex.h"

using namespace std;

//User class to hold the instance of each user connected to the server
//stores information about username, joined groups and user socket to send messages to users
class User{
    public:
        string username;
        vector<int> joinedGroups {};
        SOCKET usersocket;
        User(string username, SOCKET usersocket){
            this->username = username;
            this->usersocket = usersocket;
        }
        
        bool addToGroup(int gid){
            if(isPresentInGroup(gid)) return false;
            joinedGroups.push_back(gid);
            return true;
        }

        bool isPresentInGroup(int gid){
            for(const auto& num : joinedGroups){
                if(num == gid){
                    return true;
                }
            }
            return false;
        }

        //should only be called from the instance of the Group class
        bool removeFromGroup(int gid){          
            auto it = find(joinedGroups.begin(), joinedGroups.end(), gid);
            if(it != joinedGroups.end()){
                joinedGroups.erase(it);
                return true;
            }else{
                return false;
            }
        }

};