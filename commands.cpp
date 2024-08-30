#include "commands.hpp"
#include <iostream>
#include <limits>
#include <string>
#include <unordered_map>
#include <cstdint>
#include <cstdlib>
using namespace std;

/* O metoda care verifica daca string-ul s este numeric (numar natural) */
bool is_number(string s)
{
    return !s.empty() && std::find_if(s.begin(), s.end(), [](unsigned char c)
                                      { return !std::isdigit(c); }) == s.end();
}

/* O clasa care se ocupa de interpretarea informatiilor oferite de catre utilizator */
/* Cazul in care utilizatorul doreste sa se autentifice */
class Login
{
public:
    string username;
    string password;

    friend istream &operator>>(istream &given_string, Login &data)
    {
    try_again_username:
        if (&given_string == &cin)
            cout << "username=";
        getline(given_string, data.username);
        size_t found_username = data.username.find(" ", 0);
        /* Se verifica daca numele de utilizator contine spatii */
        if (found_username != string::npos)
        {
            fprintf(stdout, "Type a valid username!\n");
            goto try_again_username;
        }

    try_again_password:
        if (&given_string == &cin)
            cout << "password=";
        getline(given_string, data.password);

        /* Se verifica daca parola contine spatii */
        size_t found_password = data.password.find(" ", 0);
        if (found_password != string::npos)
        {
            fprintf(stdout, "Type a valid password!\n");
            goto try_again_password;
        }

        return given_string;
    }
};

/* O clasa care se ocupa de interpretarea informatiilor oferite de catre utilizator */
/* Cazul in care utilizatorul doreste sa posteze o carte intr-un cont */
class Book
{
public:
    string genre;
    string publisher;
    string title;
    string author;
    int page_count;
    Book() : page_count(0){};

    friend istream &operator>>(istream &given_string, Book &data)
    {
        if (&given_string == &cin)
            cout << "title=";
        getline(given_string, data.title);

        if (&given_string == &cin)
            cout << "author=";
        getline(given_string, data.author);

        if (&given_string == &cin)
            cout << "genre=";
        getline(given_string, data.genre);

        if (&given_string == &cin)
            cout << "publisher=";
        getline(given_string, data.publisher);

        for (; 1;)
        {
            if (&given_string == &cin)
            {
                cout << "page_count=";
            }
            string str;
            given_string >> str;

            /* Se verifica daca numarul de pagini este un numar natural */
            if (is_number(str))
            {
                data.page_count = stoi(str);
                if (data.page_count >= 0 || given_string.eof())
                {
                    given_string.ignore();
                    break;
                }
            }

            /* Se afiseaza un mesaj corespunzator */
            fprintf(stdout, "Type a valid number!\n");
            given_string.clear();
            given_string.ignore(numeric_limits<streamsize>::max(), '\n');
        }
        return given_string;
    }
};

/* O clasa care se ocupa de interpretarea informatiilor oferite de catre utilizator */
/* Cazul in care utilizatorul doreste sa afle detalii despre o carte */
class Query
{
public:
    int id;
    Query() : id(0){};

    friend istream &operator>>(istream &given_string, Query &query)
    {
        for (; 1;)
        {
            if (&given_string == &cin)
            {
                cout << "id=";
            }
            string str;
            given_string >> str;

            /* Se verifica daca numarul de pagini este un numar natural */
            if (is_number(str))
            {
                query.id = stoi(str);
                if (query.id >= 0 || given_string.eof())
                {
                    given_string.ignore();
                    break;
                }
            }

            /* Se afiseaza un mesaj corespunzator */
            fprintf(stdout, "Type a valid number!\n");
            given_string.clear();
            given_string.ignore(numeric_limits<streamsize>::max(), '\n');
        }

        return given_string;
    }
};

/* Metoda care se ocupa de comanda get_book */
void commands::lib_get_book(Client &client)
{
    Query data;
    cin >> data;

    /* Se proceseaza raspunsul primit */
    pair<nlohmann::json, int> my_pair =
        client.get("/api/v1/tema/library/books/" + to_string(data.id));

    if (my_pair.second == 200)
    {
        for (auto elem : (my_pair.first.is_array() ? my_pair.first.at(0) : my_pair.first).items())
        {
            cout << elem.key() << ": " << elem.value() << endl;
        }
    }
    else if (my_pair.first.contains("error"))
    {
        cout << my_pair.first["error"].get<string>() << endl;
        if (client.cookie.empty())
            fprintf(stdout, "And..you are not authenticated!\n");
    }
    else
    {
        fprintf(stdout, "Error from server !\n");
    }
}

/* Metoda care se ocupa de comanda register */
void commands::register_client(Client &client)
{
    Login data;
    cin >> data;

    /* Se proceseaza raspunsul primit */
    pair<nlohmann::json, int> my_pair =
        client.post("/api/v1/tema/auth/register",
                    {
                        {"username", data.username},
                        {"password", data.password},
                    });

    if (my_pair.second == 201)
    {
        printf("Account created.\n");
    }
    else if (my_pair.first.contains("error"))
    {
        cout << my_pair.first["error"].get<string>() << endl;
    }
    else
    {
        fprintf(stdout, "Error from server!\n");
    }
}

/* Metoda care se ocupa de comanda login */
void commands::login(Client &client)
{
    Login data;
    cin >> data;

    /* Se proceseaza raspunsul primit */
    pair<nlohmann::json, int> my_pair =
        client.post("/api/v1/tema/auth/login",
                    {
                        {"username", data.username},
                        {"password", data.password},
                    });

    if (my_pair.second == 200)
    {
        printf("Authenticated.\n");
    }
    else if (my_pair.second == 204)
    {
        fprintf(stdout, "Already authenticated.\n");
    }
    else if (my_pair.first.contains("error"))
    {
        cout << my_pair.first["error"].get<string>() << endl;
    }
    else
    {
        fprintf(stdout, "Error from server!\n");
    }
}

/* Metoda care se ocupa de comanda get_books */
void commands::lib_get_books(Client &client)
{
    /* Se proceseaza raspunsul primit */
    pair<nlohmann::json, int> my_pair =
        client.get("/api/v1/tema/library/books");

    if (my_pair.second == 200 && my_pair.first.is_array())
    {
        if (my_pair.first.empty())
        {
            printf("There are no books to show.\n");
        }

        for (auto book : my_pair.first)
        {
            cout << book["id"].get<int>() << ": " << book["title"] << endl;
        }
    }
    else if (my_pair.first.contains("error"))
    {
        cout << my_pair.first["error"].get<string>() << endl;
        if (client.cookie.empty())
            fprintf(stdout, "And..you are not authenticated!\n");
    }
    else
    {
        fprintf(stdout, "Error from server!\n");
    }
}

/* Metoda care se ocupa de comanda exit */
void commands::exit(Client &client)
{
    (void)client;

    /* Se incheie executia programului */
    ::exit(0);
}

/* Metoda care se ocupa de comanda logout */
void commands::logout(Client &client)
{
    /* Se proceseaza raspunsul primit */
    pair<nlohmann::json, int> my_pair =
        client.get("/api/v1/tema/auth/logout");

    if (my_pair.second == 200)
    {
        client.clear();

        printf("Deauthenticated.\n");
    }
    else if (my_pair.first.contains("error"))
    {
        cout << my_pair.first["error"].get<string>() << endl;
    }
    else
    {
        fprintf(stdout, "Error from server!\n");
    }
}

/* Metoda care se ocupa de comanda enter_library */
void commands::lib_enter(Client &client)
{
    if (client.token.empty())
    {
        /* Se proceseaza raspunsul primit */
        pair<nlohmann::json, int> my_pair =
            client.get("/api/v1/tema/library/access");

        if (my_pair.second == 200)
        {
            printf("Access granted.\n");
        }
        else if (my_pair.first.contains("error"))
        {
            cout << my_pair.first["error"].get<string>() << endl;
        }
        else
        {
            fprintf(stdout, "Error from server!\n");
        }
    }
    else
        fprintf(stdout, "You already have access!\n");
}

/* Metoda care se ocupa de comanda delete_book */
void commands::lib_del_book(Client &client)
{
    Query data;
    cin >> data;

    /* Se proceseaza raspunsul primit */
    pair<nlohmann::json, int> my_pair =
        client.del("/api/v1/tema/library/books/" + to_string(data.id));

    if (my_pair.second == 200)
    {
        printf("Book deleted.\n");
    }
    else if (my_pair.first.contains("error"))
    {
        cout << my_pair.first["error"].get<string>() << endl;
    }
    else
    {
        fprintf(stdout, "Error from server!\n");
    }
}

/* Metoda care se ocupa de comanda add_book */
void commands::lib_add_book(Client &client)
{
    Book data;
    cin >> data;
    
    /* Se proceseaza raspunsul primit */
    pair<nlohmann::json, int> my_pair =
        client.post("/api/v1/tema/library/books",
                    {
                        {"title", data.title},
                        {"author", data.author},
                        {"genre", data.genre},
                        {"publisher", data.publisher},
                        {"page_count", data.page_count},
                    });

    if (my_pair.second == 200)
    {
        printf("Book added.\n");
    }
    else if (my_pair.first.contains("error"))
    {
        cout << my_pair.first["error"].get<string>() << endl;
    }
    else
    {
        fprintf(stdout, "Error from server!\n");
    }
}
