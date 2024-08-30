#include "Client.hpp"
#include <functional>
#include <string>
#include <unordered_map>
using namespace std;

/* Toate comenzile implementate */
namespace commands
{
    void lib_get_book(Client &);
    void lib_add_book(Client &);
    void register_client(Client &);
    void login(Client &);
    void lib_enter(Client &);
    void lib_get_books(Client &);
    void logout(Client &);
    void exit(Client &);
    void lib_del_book(Client &);
    
    /* O structura de date care se ocupa de executarea functiei corespunzatoare */
    unordered_map<string, function<void(Client &)>> const functions = {
        {"register", register_client},
        {"login", login},
        {"enter_library", lib_enter},
        {"get_books", lib_get_books},
        {"get_book", lib_get_book},
        {"logout", logout},
        {"exit", exit},
        {"delete_book", lib_del_book},
        {"add_book", lib_add_book},
    };
}
