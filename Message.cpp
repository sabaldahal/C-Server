#include <iostream>
#include <string>

using namespace std;

class Message{
    private:

    public:
        int id;
        string date;
        string name;
        string subject;
        string body;
        
        Message(int id, string date, string name, string subject, string body){
            this->id = id;
            this->date = date;
            this->name = name;
            this->subject = subject;
            this->body = body;
        }
};