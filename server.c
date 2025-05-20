#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

char *construct_html_page(const char *directory) {
    const char *header_template =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %d\r\n"
            "Connection: close\r\n"
            "\r\n";

    const char *html_start = "<html><body><h1>Directory Listing</h1><ul>";
    const char *html_end = "</ul></body></html>";

    int html_buffer_size = 4096;
    char *html_body = malloc(html_buffer_size);
    if (!html_body) {
        return NULL;
    }
    strcpy(html_body, html_start);

    char search_path[MAX_PATH];
    snprintf(search_path, sizeof(search_path), "%s\\*", directory);

    WIN32_FIND_DATA find_data;
    HANDLE hFind = FindFirstFile(search_path, &find_data);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                // Generate HTML list item for the file
                char item[1024];
                snprintf(item,
                         sizeof(item),
                         "<li><a href=\"/%s\">%s</a></li>",
                         find_data.cFileName, find_data.cFileName);

                // Resize buffer if necessary
                if (strlen(html_body) + strlen(item) + strlen(html_end) + 1 > html_buffer_size) {
                    html_buffer_size *= 2;
                    html_body = realloc(html_body, html_buffer_size);
                    if (!html_body) {
                        return NULL;
                    }
                }

                strcat(html_body, item);
            }
        } while (FindNextFile(hFind, &find_data));

        FindClose(hFind);
    }

    strcat(html_body, html_end);

    int body_len = (int) strlen(html_body);
    int header_len = snprintf(NULL, 0, header_template, body_len);
    char *response = malloc(header_len + body_len + 1);
    if (!response) {
        free(html_body);
        return NULL;
    }

    sprintf(response, header_template, body_len);
    strcat(response, html_body);

    free(html_body);

    return response;
}

int main() {
    char directory[256];
    printf("Directory to share:\n");
    if (fgets(directory, sizeof(directory), stdin) != NULL) {
        directory[strcspn(directory, "\n")] = 0;
    }
    else {
        printf("Invalid input.\n");
        return 1;
    }

    WSADATA wsa_data;
    int result;
    result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0) {
        printf("WSAStartup() failed: %d\n", result);
        return 1;
    }

    SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_socket == INVALID_SOCKET) {
        printf("socket() failed: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    struct sockaddr_in address = {0};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(listen_socket, (SOCKADDR *) &address, sizeof(address)) == SOCKET_ERROR) {
        printf("bind() failed: %d\n", WSAGetLastError());
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
        printf("listen() failed: %d\n", WSAGetLastError());
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    while (1) {
        SOCKET client_socket = accept(listen_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            printf("accept() failed: %d\n", WSAGetLastError());
            break;
        }

        char buffer[1024] = {0};
        recv(client_socket, buffer, sizeof(buffer) - 1, 0);

        char *response = construct_html_page(directory);
        send(client_socket, response, (int) strlen(response), 0);
        free(response);

        closesocket(client_socket);
    }

    closesocket(listen_socket);
    WSACleanup();
    return 0;
}
