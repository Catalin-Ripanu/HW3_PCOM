#include "json.hpp"
#include <netinet/in.h>
#include <cstdint>
#include <any>
#include <string>
#include <vector>
#include <unordered_map>
using namespace std;

/* Clasa care se ocupa de comunicarea cu server-ul*/
class Client
{
public:
    int sockfd;
    vector<uint8_t> buffer;
    unordered_map<string, any> responses;
    string cookie;
    string token;

public:
    ~Client();
    Client();
    /* Metode utile in ceea ce urmeaza */
private:
    void connect();
    void close();
    bool send(string const &);
    bool recv();

public:
    void clear();

    /* Metode ce guverneaza toate tipurile de cereri HTTP */
    pair<nlohmann::json, int>
        get(string);

    pair<nlohmann::json, int>
        post(string, nlohmann::json);

    pair<nlohmann::json, int>
        del(string);
};
