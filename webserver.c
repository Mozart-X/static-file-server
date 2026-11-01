#include <stdio.h>
#include <winsock2.h>
#include <string.h>

int main()
{

    // powering on Winsock to receive data
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    // metadata for what the server will use -- AF_INET means IPv4,
    // AF_INET6 for IPv6, SOCK_STREAM means TCP, and 0 lets the OS pick the default protocol

    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);

    // finalize the metadata of what the server should look like and
    // what port it should be working with

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(9000);

    // fix or reserve what server port it is working on

    bind(server, (struct sockaddr *)&addr, sizeof(addr));

    // now this should act like a server
    listen(server, 10);
    printf("Server is listening on http://localhost:9000 ...\n");

    while (1)
    {
        // program will sleep and the OS will interrupt or poke when a connection appears
        SOCKET client = accept(server, NULL, NULL);

        char req[2048];
        recv(client, req, sizeof(req), 0); // drain what the browser said BEFORE replying

        // keeping a response .. it's probably metadata about what the browser will be getting
        // data buffer

        char method[8], path[256];
        sscanf(req, "%s %s", method, path); // method="GET", path="/style.css"

        char *filename = path + 1; // "path + 1" skips the leading '/'
        if (strcmp(path, "/") == 0)
            filename = "index.html"; // the rule you discovered on day one

        char *type = "text/plain";
        if (strstr(filename, ".html"))
            type = "text/html";
        else if (strstr(filename, ".css"))
            type = "text/css";

        FILE *f = fopen(filename, "rb");
        if (f == NULL) // file not found
        {
            char *nf = "HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\n";
            send(client, nf, strlen(nf), 0);
            shutdown(client, SD_SEND);
            closesocket(client);
            continue; // skip to the next connection instead of crashing
        }
        fseek(f, 0, SEEK_END);
        long size = ftell(f); // how many bytes the file is
        fseek(f, 0, SEEK_SET);
        char filebuf[100000];
        fread(filebuf, 1, size, f); // store the bytes
        fclose(f);

        char header[256];
        snprintf(header, sizeof(header),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: %s\r\n"
                 "Content-Length: %ld\r\n"
                 "Connection: close\r\n"
                 "\r\n",
                 type, size);

        send(client, header, strlen(header), 0); // headers: a text string, strlen is fine
        send(client, filebuf, size, 0);          // body: RAW bytes, counted by SIZE — never strlen

        // kinda like fclose to close the file or free the resources and start the new loop
        shutdown(client, SD_SEND); // say "I'm done sending" — flush, then clean close
        closesocket(client);
    }
}