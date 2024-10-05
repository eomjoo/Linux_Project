#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/wait.h>

#define DEFAULT_PROTOCOL 0

struct locker {
    int id;
    int empty;
    char pwd[10];
    char content[10][300];  
    int content_count;      
};

void initializeLockers(struct locker *lockers, int k);
void displayUpdatedLockers(struct locker *lockers, int k);
void handleClient(int client_socket, struct locker *lockers, int k, int pipe_fd[2]);

int main() {
    char str[100];
    int client_socket, server_socket, num_lockers, client_len, status, child_pid;
    struct locker *lockers;
    struct sockaddr_un server_unix_addr, client_unix_addr;

    server_socket = socket(AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
    if (server_socket < 0) {
        perror("소켓 생성 실패");
        exit(1);
    }

    server_unix_addr.sun_family = AF_UNIX;
    strcpy(server_unix_addr.sun_path, "manager");
    unlink("manager");

    if (bind(server_socket, (struct sockaddr *)&server_unix_addr, sizeof(server_unix_addr)) < 0) {
        perror("바인드 실패");
        exit(1);
    }

    printf("사물함 개수를 입력하세요 : ");
    scanf("%s", str);

    num_lockers = atoi(str);

    printf("* 최대 고객수는 5명입니다. *\n");

    lockers = (struct locker *)malloc((num_lockers + 1) * sizeof(struct locker));
    initializeLockers(lockers, num_lockers);

    int file_descriptor;
    mkfifo("index", 0666);
    file_descriptor = open("index", O_WRONLY);

    listen(server_socket, 5);

    while (1) {
        int pipe_fd[2];
        pipe(pipe_fd);

        client_len = sizeof(client_unix_addr);
        client_socket = accept(server_socket, (struct sockaddr *)&client_unix_addr, &client_len);    
        if (client_socket == -1) {
            perror("클라이언트 연결 실패");
            continue;
        }

        if (fork() == 0) {
            close(pipe_fd[0]);
            write(file_descriptor, str, 100);
            handleClient(client_socket, lockers, num_lockers, pipe_fd);
            exit(0);
        }

        child_pid = wait(&status);

        close(pipe_fd[1]);

        for (int i = 1; i <= num_lockers; i++) {
            read(pipe_fd[0], &lockers[i], sizeof(struct locker));
        }

        close(client_socket);
    }

    return 0;
}

void initializeLockers(struct locker *lockers, int k) {
    printf("\n┌───────────────┬───────────────┬───────────────┬────────────────────────────────────────────┐\n");
    printf("│  Locker Number│    Status     │   Password    │                   Content                  │\n");
    printf("├───────────────┼───────────────┼───────────────┼────────────────────────────────────────────┤\n");

    int i, j;
    for (i = 1; i <= k; i++) {
        lockers[i].id = i;
        lockers[i].empty = 1;
        strcpy(lockers[i].pwd, "0000");
        lockers[i].content_count = 0;
        for (j = 0; j < 10; j++) {
            strcpy(lockers[i].content[j], "");
        }
        printf("│      %2d       │    Empty      │  %-10s  │  %-40s  │\n", i, lockers[i].pwd, lockers[i].content[0]);
    }
    printf("└───────────────┴───────────────┴───────────────┴────────────────────────────────────────────┘\n");
}

void handleClient(int client_socket, struct locker *lockers, int k, int pipe_fd[2]) {
    char mode, empty_locker, available;
    char user_id[100], read_password[10], received_password[10], send_password[10];
    char content[300];
    int locker_id;

    int i;
    for (i = 1; i <= k; i++) {    
        empty_locker = lockers[i].empty ? '1' : '0';
        write(client_socket, &empty_locker, sizeof(empty_locker));
    }

    while (1) {
        read(client_socket, &mode, 1);

        if (mode == 'c') {
            char send_message[300], receive_message[300];
            printf("\n\n* 고객으로부터 채팅이 왔습니다. 채팅을 시작합니다.\n");        
            while (1) {
                read(client_socket, receive_message, sizeof(receive_message));
                if (!strcmp(receive_message, "Q")) break;
                printf("Client : %s\n", receive_message);
                printf("Me : ");
                scanf(" %[^\n]s", send_message);
                write(client_socket, send_message, sizeof(send_message));
            }
        } else if (mode == 'n') {
            read(client_socket, user_id, 100);
            locker_id = atoi(user_id);

            if (lockers[locker_id].empty) {
                available = 'Y';
                write(client_socket, &available, 1);
                read(client_socket, read_password, sizeof(read_password));
                strcpy(lockers[locker_id].pwd, read_password);
                lockers[locker_id].empty = 0;

                while (lockers[locker_id].content_count < 10) {
                    read(client_socket, content, sizeof(content));
                    if (strcmp(content, "done") == 0) break;
                    strcpy(lockers[locker_id].content[lockers[locker_id].content_count], content);
                    lockers[locker_id].content_count++;
                }
            } else {
                available = 'N';
                write(client_socket, &available, 1);
                strcpy(send_password, lockers[locker_id].pwd);
                read(client_socket, received_password, sizeof(received_password));
                write(client_socket, send_password, sizeof(send_password));
                if (!strcmp(received_password, lockers[locker_id].pwd)) {
                    lockers[locker_id].empty = 1;
                    strcpy(lockers[locker_id].pwd, "0000");
                    for (i = 0; i < 10; i++) {
                        strcpy(lockers[locker_id].content[i], "");
                    }
                    lockers[locker_id].content_count = 0;
                }
            }

            displayUpdatedLockers(lockers, k);
        } else if (mode == 'd') {
            read(client_socket, user_id, 100);
            locker_id = atoi(user_id);

            if (!lockers[locker_id].empty) {
                strcpy(send_password, lockers[locker_id].pwd);
                read(client_socket, received_password, sizeof(received_password));
                if (!strcmp(received_password, lockers[locker_id].pwd)) {
                    char item_to_delete[300];
                    read(client_socket, item_to_delete, sizeof(item_to_delete));
                    int found = 0;
                    for (i = 0; i < lockers[locker_id].content_count; i++) {
                        if (strcmp(lockers[locker_id].content[i], item_to_delete) == 0) {
                            found = 1;
                            for (int j = i; j < lockers[locker_id].content_count - 1; j++) {
                                strcpy(lockers[locker_id].content[j], lockers[locker_id].content[j + 1]);
                            }
                            lockers[locker_id].content[lockers[locker_id].content_count - 1][0] = '\0';
                            lockers[locker_id].content_count--;
                            write(client_socket, "내용물 삭제 완료", sizeof("내용물 삭제 완료"));
                            break;
                        }
                    }
                    if (!found) {
                        write(client_socket, "해당 내용물이 없습니다", sizeof("해당 내용물이 없습니다"));
                    }
                } else {
                    write(client_socket, "비밀번호가 틀렸습니다", sizeof("비밀번호가 틀렸습니다"));
                }
            } else {
                write(client_socket, "사물함이 비어 있습니다", sizeof("사물함이 비어 있습니다"));
            }

            write(pipe_fd[1], &lockers[locker_id], sizeof(struct locker));
            displayUpdatedLockers(lockers, k);
        }

        char question_from_client;
        read(client_socket, &question_from_client, 1);
        if (question_from_client == 'N') break;
    }

    for (i = 1; i <= k; i++)
        write(pipe_fd[1], &lockers[i], sizeof(struct locker));
}

void displayUpdatedLockers(struct locker *lockers, int k) {
    printf("\n┌───────────────┬───────────────┬───────────────┬────────────────────────────────────────────┐\n");
    printf("│  Locker Number│    Status     │   Password    │                   Content                  │\n");
    printf("├───────────────┼───────────────┼───────────────┼────────────────────────────────────────────┤\n");

    int i, j;
    for (i = 1; i <= k; i++) {
        printf("│      %2d       │  %9s     │  %-10s  │", lockers[i].id, lockers[i].empty ? "Empty" : "Occupied", lockers[i].pwd);
        for (j = 0; j < lockers[i].content_count; j++) {
            if (j != 0) printf(", ");
            printf("%s", lockers[i].content[j]);
        }
        printf("\n");
    }
    printf("└───────────────┴───────────────┴───────────────┴────────────────────────────────────────────┘\n");
}

