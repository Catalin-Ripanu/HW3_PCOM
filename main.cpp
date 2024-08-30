#include "commands.hpp"
#include <cstdlib>
#include <iostream>
#include <sstream>
using namespace std;

/* Interfata cu utilizatorul */
int main()
{
    Client client;
    string string_line;
    while (getline(cin, string_line))
    {
        /* Se proceseaza comenzile */
        auto info = commands::functions.find(string_line);

        if (string_line.empty())
        {
            fprintf(stdout, "No command\n");
            continue;
        }

        if (info == commands::functions.end())
        {
            fprintf(stdout, "Not a valid command\n");
            continue;
        }

        /* Se apeleaza functia corespunzatoare */
        info->second(client);
    }

    return 0;
}
