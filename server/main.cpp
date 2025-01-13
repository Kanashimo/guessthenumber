#include <asm-generic/errno.h>
#include <cstring>
#include <string>

#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <thread>

using namespace std;

//int clients = 0; @Deprecated
int sockets[2] = {0};

bool ids_taken[2] = {false, false};
//string pending_messages[2] = {"", ""}; @Deprecated

string id_0_buffer[50] = {""};
string id_1_buffer[50] = {""};

bool alive[2] = {false, false};

bool isSocketAlive(int clientSocket) {
    char testByte = 0;
    int result = send(clientSocket, &testByte, 0, 0); // Wysyłanie pustej wiadomości
    if (result < 0) {
        if (errno == EPIPE || errno == ECONNRESET) {
            // Połączenie zostało przerwane
            return false;
        }
    }
    return true;
}

void clearMyBuffer(int my_id) {
    if(my_id == 0) {
        for(int i = 0; i<50; i++) {
            id_0_buffer[i] = "";
        }
    } else {
        for(int i = 0; i<50; i++) {
            id_1_buffer[i] = "";
        }
    }
}

void finishGracefully() {
    if (sockets[0] != 0 || alive[0] || ids_taken[0]){
        int clientSocket = sockets[0];
        const char* bye = "finished-";

        send(clientSocket, bye, strlen(bye), 0);
        close(clientSocket);

        int my_id = 0;

        alive[my_id] = false;
        ids_taken[my_id] = false;
        sockets[my_id] = 0;

        clearMyBuffer(my_id);
    }
    if (sockets[1] != 0 || alive[1] || ids_taken[1]) {
        int clientSocket = sockets[1];
        const char* bye = "finished-";

        send(clientSocket, bye, strlen(bye), 0);
        close(clientSocket);

        int my_id = 1;

        alive[my_id] = false;
        ids_taken[my_id] = false;
        sockets[my_id] = 0;

        clearMyBuffer(my_id);
    }
}

void closeMySocket(int clientSocket, int my_id) {
    int second_client = (my_id == 0) ? 1 : 0;

    if(second_client == 0 && (alive[second_client] || ids_taken[second_client])) {
        for(int i = 0; i<50; i++) {
            if(id_0_buffer[i] == "") {
                id_0_buffer[i] = "left-";
                break;
            }
        }
    } else if (second_client == 1 && (alive[second_client] || ids_taken[second_client])) {
        for(int i = 0; i<50; i++) {
            if(id_1_buffer[i] == "") {
                id_1_buffer[i] = "left-";
                break;
            }
        }
    }

    const char* bye = "bye-";

    send(clientSocket, bye, strlen(bye), 0);
    close(clientSocket);

    alive[my_id] = false;
    ids_taken[my_id] = false;
    sockets[my_id] = 0;

    clearMyBuffer(my_id);
}

void handle_write(int clientSocket, int my_id) {
    while(alive[my_id]) {

        for(int i = 0; i<50; i++) {
            if(my_id == 0) {
                //id_0_buffer

                string wait_message = id_0_buffer[i];

                if(wait_message == "") continue;

                const char* message = wait_message.c_str();

                if(wait_message.find("end-") != std::string::npos) {
                    closeMySocket(clientSocket, my_id);
                    return;
                }

                send(clientSocket, message, strlen(message), 0); //wysyłamy wiadomość do odpowiedniego clienta

                id_0_buffer[i] = ""; //czyścimy bufor
            } else {
                //id_1_buffer

                string wait_message = id_1_buffer[i];

                if(wait_message == "") continue;

                const char* message = wait_message.c_str();

                if(wait_message.find("end-") != std::string::npos) {
                    closeMySocket(clientSocket, my_id);
                    return;
                }

                send(clientSocket, message, strlen(message), 0); //wysyłamy wiadomość do odpowiedniego clienta

                id_1_buffer[i] = ""; //czyścimy bufor
            }
        }

        // if(pending_messages[my_id] != "") { //jeśli oczekuje na mnie jakaś wiadomość
        //     const char* message = wait_message.c_str();

        //     if(wait_message.find("end") != std::string::npos) {
        //         send(clientSocket, message, strlen(message), 0);

        //         pending_messages[my_id] = "";
        //         close(clientSocket);
        //         return;
        //     }

        //     send(clientSocket, message, strlen(message), 0); //wysyłamy wiadomość do odpowiedniego clienta

        //     pending_messages[my_id] = ""; //czyścimy bufor
        // }
    }

    close(clientSocket);
}

void handle_read(int clientSocket, int my_id) {
    while(alive[my_id]) {

        char buffer[512] = { 0 };
        recv(clientSocket, buffer, sizeof(buffer), 0);

        string human_message = buffer;

        if(human_message.find("exit") != std::string::npos) { //Client "grzecznie" prosi o wyjście. Zwolnić miejsce i socket.
            closeMySocket(clientSocket, my_id);
            return;
        }

        if(human_message.find("finished") != std::string::npos) { //Client oświadcza, że wszystkie gry się skończyły
            // const char* ok = "ok";
            // send(clientSocket, ok, strlen(ok), 0);

            // int second_client = (my_id == 0) ? 1 : 0;

            // if(second_client == 0) {
            //     for(int i = 0; i<50; i++) {
            //         if(id_0_buffer[i] == "") {
            //             //id_0_buffer[i] = "left";
            //             id_0_buffer[i+1] = "finished";
            //             break;
            //         }
            //     }
            // } else {
            //     for(int i = 0; i<50; i++) {
            //         if(id_1_buffer[i] == "") {
            //             //id_1_buffer[i] = "left";
            //             id_1_buffer[i+1] = "finished";
            //             break;
            //         }
            //     }
            // }

            finishGracefully();

            // closeMySocket(clientSocket, my_id);
            // closeMySocket(sockets[second_client], second_client);

            return;
        }

        int second_client = (my_id == 0) ? 1 : 0;

        if(second_client == 0) {
            for(int i = 0; i<50; i++) {
                if(id_0_buffer[i] == "") {
                    id_0_buffer[i] = human_message;
                    break;
                }
            }
        } else {
            for(int i = 0; i<50; i++) {
                if(id_1_buffer[i] == "") {
                    id_1_buffer[i] = human_message;
                    break;
                }
            }
        }
        //pending_messages[second_client] = human_message;
    }

ids_taken[my_id] = false;
    close(clientSocket);
}

void handle_clients(int clientSocket) {

        int my_id = -1; //wątek musi wiedzieć, jakie otrzymało ID

        if(!ids_taken[0]) {
            my_id = 0;
            ids_taken[0] = true;
        } else if(!ids_taken[1]) {
            my_id = 1;
            ids_taken[1] = true;
        }


        cout << "Dostałem nowego clienta! Najlepsze ID to: " << my_id << endl;

        if(my_id == -1) {
            cout << "Client nie zakfalfikował się do żadnego ID. Wyrzucam." << endl;
            const char* message = "full-";
            send(clientSocket, message, strlen(message), 0);
            close(clientSocket);
            return;
        }

        cout << "Client dostał ID = " << my_id << ", wpuszczam." << endl;

        sockets[my_id] = clientSocket; //zapisywanie socketu clienta (na wszelki wypadek :) )
        alive[my_id] = true;

        string message = to_string(my_id) + "-";
        const char* user_id = message.c_str();

        //int second_client = (my_id == 0) ? 1 : 0;

        // if(alive[second_client]) {
        //     if(second_client == 0) {
        //         for(int i = 0; i<50; i++) {
        //             if(id_0_buffer[i] == "") {
        //                 id_0_buffer[i] = "joined";
        //                 break;
        //             }
        //         }
        //     } else {
        //         for(int i = 0; i<50; i++) {
        //             if(id_1_buffer[i] == "") {
        //                 id_1_buffer[i] = "joined";
        //                 break;
        //             }
        //         }
        //     }
        // }

        if(ids_taken[0] && ids_taken[1]) {
            for(int i = 0; i<50; i++) {
                if(id_0_buffer[i] == "") {
                    id_0_buffer[i] = "ready-";
                    break;
                }
            }
            for(int i = 0; i<50; i++) {
                if(id_1_buffer[i] == "") {
                    id_1_buffer[i] = "ready-";
                    break;
                }
            }
        }

        send(clientSocket, user_id, strlen(user_id), 0);

        thread send(handle_write, clientSocket, my_id);
        send.detach();

        thread receive(handle_read, clientSocket, my_id);
        receive.detach();
}

int main() //Pokazać na szóstkę
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(4000);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));

    listen(serverSocket, 5);

    int sockets[2] = {};
    int clients = 0;

    cout << "Słucham na porcie: 4000" << endl;

    while(true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);

        thread handle(handle_clients, clientSocket); //osobny wątek dla klienta, ale upewnić się, że akceptujemy połączenia
        handle.detach();
    }

    close(serverSocket);

    return 0;
}
