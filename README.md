# REST API Client-Server Application

This project involves the implementation of a Client in C++ that can connect to a server using a REST API.

## User Interface

The processing of commands given by the user at the command line and the execution of corresponding code for an entered command is done by searching in an `unordered_map`. This structure was chosen for its simplicity, but other data structures could have been used as well.

## Command Processing

The program reads the data entered by the user for the requested command (e.g., password, username, book_title, book_id, book_author, etc.) from the keyboard, constructs the JSON payload, sends the request to the server via a `Client` object, and then receives, interprets, and displays the response from the server. For JSON parsing, the "JSON for Modern C++" library created by Niels Lohmann was used, as it has intuitive documentation, is in a single header file, and is a well-established library.

## Client Implementation

The `Client` object is used for communication with the server. It exposes the following methods:

- `del`: Sends an HTTP DELETE request to the server.
- `get`: Sends an HTTP GET request to the server.
- `post`: Sends an HTTP POST request to the server.
- `clear`: Deletes the authorization token and cookie (used for the logout command in the user interface).

All the methods mentioned above will return a pair containing a `nlohmann::json` object as the first element (used to display a corresponding message to the user) and the HTTP status code of the response as the second element.

## Minor Observations

1. All HTTP communication is done manually through TCP sockets.
2. A connection to the server is made only when it is necessary.
3. There are no checks done on the client related to state, so the user can, for example, send a logout request to the server even if not authenticated or access the library before authenticating. Absolutely all requests are sent to the server, and the management of these errors is done exclusively by the server.
4. If there is already a valid connection to the server, it will be used. Otherwise, the old connection is closed, and a new one is created, all these things being processed within the private `send` and `recv` methods in the `Client` class.
