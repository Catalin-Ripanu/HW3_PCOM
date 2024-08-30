#include "Client.hpp"
#include "utils.hpp"
#include "json.hpp"
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdint>
#include <any>
#include <algorithm>
#include <sstream>
#include <vector>

/* O metoda care realizeaza conexiunea cu server-ul*/
void Client::connect()
{
    int ret;
    close();
    string host = "34.241.4.235";
    string port = to_string(8080);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    DIE(sockfd == -1, "socket");
    struct sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    memset(client_addr.sin_zero, 0, 8);
    client_addr.sin_addr =
        {
            parse_addr_str(host.c_str())};
    client_addr.sin_port = parse_port_str(port.c_str());
    ret = ::connect(sockfd, (struct sockaddr *)&client_addr, sizeof(client_addr));
    DIE(ret == -1, "connect");
}

/* O metoda care incheie conexiunea cu server-ul */
void Client::close()
{
    int ret;

    if (sockfd == -1)
    {
        return;
    }

    ret = ::close(sockfd);
    DIE(ret == -1, "close");

    sockfd = -1;
}

Client::Client()
{
    sockfd = -1;
}

/* O metoda care transmite date catre server */
bool Client::send(string const &d)
{
    int ret;

    if (sockfd == -1)
    {
        connect();
    }

    size_t length = 0;

    for (; length < d.length();)
    {
        ret = ::send(sockfd, d.c_str() + length, d.length() - length, MSG_NOSIGNAL);
        if (ret == -1)
        {
            DIE(errno != EPIPE, "send");
            close();
            return false;
        }

        length = length + ret;
    }

    return true;
}

/* O metoda care se ocupa de parsarea datelor venite din partea server-ului */
bool Client::recv()
{
    int ret;
    string separator = "\r\n";
    string double_separator = "\r\n\r\n";
    string header_separator = ":";

    if (sockfd == -1)
    {
        connect();
    }
    size_t length = 0;
    size_t total_length = buffer.size();
    fill(buffer.begin(), buffer.end(), 0);
    responses.clear();
    bool header = false;
    uint8_t *current = buffer.data() + length;
    uint8_t *end_string = buffer.data() + total_length;
    size_t header_length = 0;
    uint8_t *header_end = buffer.data() + header_length;
    /* Daca exista o conexiune valida, aceasta se va folosi */

    for (; !header || length < total_length;)
    {
        if (length >= buffer.size())
        {
            buffer.resize(buffer.size() ? 2 * buffer.size() : 1);

            if (!header)
            {
                total_length = buffer.size();
            }

            current = buffer.data() + length;
            end_string = buffer.data() + total_length;

            header_end = buffer.data() + header_length;
        }

        ret = ::recv(sockfd, current, buffer.size() - length, 0);
        if (ret == 0 || ret == -1)
        {
            DIE(ret == -1 && errno != ECONNRESET, "recv");
            close();
            return false;
        }

        if (!header)
        {
            header_end = search(current, end_string, double_separator.begin(), double_separator.end());
        }

        if (!header && header_end != end_string)
        {
            string proto_ver;
            int status_code;
            string status_text;

            header_end = header_end + double_separator.length();
            header_length = header_end - buffer.data();

            uint8_t *aux_pointer = buffer.data();
            uint8_t *aux_pointer_next = search(aux_pointer, header_end, separator.begin(), separator.end());

            istringstream string_buf(string(aux_pointer, aux_pointer_next));

            string_buf >> proto_ver >> status_code >> status_text;

            DIE(proto_ver != "HTTP/1.1",
                "incompatible protocol version '%s'", proto_ver.c_str());

            responses["Status"] = status_code;

            do
            {
                aux_pointer = aux_pointer_next + separator.length();
                aux_pointer_next = search(aux_pointer, header_end, separator.begin(), separator.end());

                if (aux_pointer == aux_pointer_next)
                {
                    break;
                }

                uint8_t *sep = search(aux_pointer, aux_pointer_next, header_separator.begin(), header_separator.end());

                string key(aux_pointer, sep);
                string value(sep + header_separator.length() + 1, aux_pointer_next);

                if (key == "Content-Length")
                {
                    responses[key] = stoi(value);
                }
                else if (key == "Set-Cookie")
                {
                    cookie = string(
                        value.begin(),
                        value.begin() + value.find(";"));

                    responses[key] = value;
                }
                else
                {
                    responses[key] = value;
                }
            } while (!equal(double_separator.begin(), double_separator.end(), aux_pointer_next));
            total_length = 0;

            total_length = total_length + header_length;
            if (responses.count("Content-Length"))
            {
                total_length = total_length + any_cast<int>(responses["Content-Length"]);
            }

            end_string = buffer.data() + total_length;

            header = true;
        }
        current = current + ret;
        length = length + ret;
    }

    auto json_object = nlohmann::json::parse(header_end, end_string, nullptr, false);

    if (json_object.is_discarded())
    {
        json_object = {
            {"error", string(header_end, end_string)},
        };
    }

    if (json_object.contains("token"))
    {
        token = json_object["token"];
    }

    responses["Content"] = json_object;

    return true;
}

/* O metoda care se ocupa de 'curatarea' cookie-ului/token-ului */
void Client::clear()
{
    token.clear();
    cookie.clear();
}

/* Un destructor */
Client::~Client()
{
    close();
}

/* Metoda care realizeaza cererea HTTP de tip GET */
pair<nlohmann::json, int>
Client::get(string target)
{
    ostringstream aux_string;
    /* Se formeaza cererea */
    aux_string << "GET"
               << " " << target << " "
               << "HTTP/1.1"
               << "\r\n";
    aux_string << "Host"
               << ":"
               << " "
               << "34.241.4.235"
               << "\r\n";
    aux_string << "Accept"
               << ":"
               << " "
               << "application/json"
               << "\r\n";

    if (!cookie.empty())
    {
        aux_string << "Cookie"
                   << ":"
                   << " " << cookie << "\r\n";
    }

    if (!token.empty())
    {
        aux_string << "Authorization"
                   << ":"
                   << " "
                   << "Bearer"
                   << " " << token << "\r\n";
    }
    aux_string << "\r\n";

    string req = aux_string.str();

    for (; !send(req) || !recv();)
        ;

    return {
        /* Un obiect json folosit pentru afisarea unui mesaj corespunzator */
        any_cast<nlohmann::json>(responses["Content"]),
        /* Un element care contine codul de stare HTTP al raspunsului */
        any_cast<int>(responses["Status"])

    };
}

/* Metoda care realizeaza cererea HTTP de tip POST */
pair<nlohmann::json, int>
Client::post(string target, nlohmann::json j)
{
    ostringstream aux_string;
    string content = j.dump();
    /* Se formeaza cererea */
    aux_string << "POST"
               << " " << target << " "
               << "HTTP/1.1"
               << "\r\n";
    aux_string << "Host"
               << ":"
               << " "
               << "34.241.4.235"
               << "\r\n";
    aux_string << "Accept"
               << ":"
               << " "
               << "application/json"
               << "\r\n";

    if (!cookie.empty())
    {
        aux_string << "Cookie"
                   << ":"
                   << " " << cookie << "\r\n";
    }

    if (!token.empty())
    {
        aux_string << "Authorization"
                   << ":"
                   << " "
                   << "Bearer"
                   << " " << token << "\r\n";
    }
    aux_string << "Content-Type"
               << ":"
               << " "
               << "application/json"
               << "\r\n";
    aux_string << "Content-Length"
               << ":"
               << " " << content.length() << "\r\n";
    aux_string << "\r\n";
    aux_string << content;

    string req = aux_string.str();

    for (; !send(req) || !recv();)
        ;

    return {
        /* Un obiect json folosit pentru afisarea unui mesaj corespunzator */
        any_cast<nlohmann::json>(responses["Content"]),
        /* Un element care contine codul de stare HTTP al raspunsului */
        any_cast<int>(responses["Status"])

    };
}

/* Metoda care realizeaza cererea HTTP de tip DELETE */
pair<nlohmann::json, int>
Client::del(string target)
{
    ostringstream aux_string;
    /* Se formeaza cererea */
    aux_string << "DELETE"
               << " " << target << " "
               << "HTTP/1.1"
               << "\r\n";
    aux_string << "Host"
               << ":"
               << " "
               << "34.241.4.235"
               << "\r\n";
    aux_string << "Accept"
               << ":"
               << " "
               << "application/json"
               << "\r\n";

    if (!cookie.empty())
    {
        aux_string << "Cookie"
                   << ":"
                   << " " << cookie << "\r\n";
    }

    if (!token.empty())
    {
        aux_string << "Authorization"
                   << ":"
                   << " "
                   << "Bearer"
                   << " " << token << "\r\n";
    }
    aux_string << "\r\n";

    string req = aux_string.str();

    for (; !send(req) || !recv();)
        ;

    return {
        /* Un obiect json folosit pentru afisarea unui mesaj corespunzator */
        any_cast<nlohmann::json>(responses["Content"]),
        /* Un element care contine codul de stare HTTP al raspunsului */
        any_cast<int>(responses["Status"])

    };
}
