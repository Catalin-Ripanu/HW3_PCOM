# HW3_PCOM

A Project which demanded implementing a Client in C++ connecting to a server using REST API.

In the source files are the implementations, with the more important parts of the program logic being commented.
In the header files are the definitions/declarations of structures and functions used in the source files.
There are also comments accompanying the definitions/declarations meant to facilitate understanding of their purpose.

## User Interface:
Here, the processing of commands given by the user at the command line and the execution of corresponding code for an entered command, if it exists, is done by searching in an unordered_map (another structure could have been used here, the map was chosen for simplicity).

## Command Processing:
Here, data entered by the user for the requested command is read from the keyboard (example: password, username, book_title, book_id, book_author, etc.), the JSON payload is constructed, the request is sent to the server via a Client object, and then the response from the server is received, interpreted, and displayed. For JSON parsing, the JSON for modern C++ library created by Niels Lohmann was used.

This library was used because:
1. We wanted to use a library, not create one.
2. The documentation is intuitive.
3. The library is in a single header file.

## Client Implementation
A Client object is used for communication with the server. It exposes the following methods:
- del: sends an HTTP DELETE request to the server
- get: sends an HTTP GET request to the server
- post: sends an HTTP POST request to the server
- clear: deletes the authorization token and cookie (used for the logout command in the user interface)

All the methods mentioned above will return a pair containing as the first element a nlohmann::json object that is used to display a corresponding message to the user, and as the second element the HTTP status code of the response.

## Minor Observations:
- All HTTP communication is done manually through TCP sockets.
- A connection to the server is made only when it is necessary.
- There are no checks done on the client related to state, so the user can, for example, send a logout request to the server even if not authenticated or access the library before authenticating. Absolutely all requests are sent to the server, then the received response is displayed, so the management of these errors is done exclusively by the server.
- If there is already a valid connection to the server, it will be used. Otherwise, the old connection is closed and a new one is created, all these things being processed within the private send and recv methods in the Client class.

An interesting assignment, it's nice to send requests to a server that exists and responds.
Also, on this occasion, I used less intuitive things from C++.

Catalin-Alexandru Ripanu