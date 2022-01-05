/*
  Fida's Master Tech
  Youtube Channel link: https://www.youtube.com/channel/UCXSTJwhMjqtGPa-YzOQUyUQ?sub_confirmation=1 
*/
#ifndef G_SENDER
#define G_SENDER
#define GS_SERIAL_LOG_1        
//#define GS_SERIAL_LOG_2       
#include <WiFiClientSecure.h>

class Gsender
{
    protected:
        Gsender();
    private:
        const int SMTP_PORT = 465;
        const char* SMTP_SERVER = "smtp.gmail.com";
        // replace the below three lines with your information
        const char* EMAIL_LOGIN = "advisorcryptocurrency@gmail.com"; //sender's gmail ID
        const char* EMAIL_PASSWORD = "Cryptocare"; //sender's gmail account's password
        const char* FROM = "advisorcryptocurrency@gmail.com"; // sender's gmail ID
        /*------------------------------------------------------*/
        const char* _error = nullptr;
        char* _subject = nullptr;
        String _serverResponce;
        static Gsender* _instance;
        bool AwaitSMTPResponse(WiFiClientSecure &client, const String &resp = "", uint16_t timeOut = 10000);

    public:
        static Gsender* Instance();
        Gsender* Subject(const char* subject);
        Gsender* Subject(const String &subject);
        bool Send(const String &to, const String &message);
        String getLastResponce();
        const char* getError();
};
#endif // G_SENDER
