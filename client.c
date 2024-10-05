#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>

#define DEFAULT_PROTOCOL 0

// 함수 선언
void displayLockers(int client_socket, int k);
void processChatMode(int client_socket);
void processLockerUsage(int client_socket, int k);
void deleteLockerContent(int client_socket, int k);

int main() {
    int client_socket;       // 클라이언트 소켓
    int connect_result;      // 연결 결과
    int file_descriptor;     // 파일 디스크립터
    char locker_number[100]; // 사물함 번호를 저장할 버퍼
    char prompt;             // 사용자 입력
    int k;                   // 사물함 개수

    struct sockaddr_un server_unix_addr;

    // 클라이언트 소켓 생성
    client_socket = socket(AF_UNIX, SOCK_STREAM, DEFAULT_PROTOCOL);
    if (client_socket < 0) {
        perror("소켓 생성 실패");
        exit(1);
    }

    // 서버 주소 설정
    server_unix_addr.sun_family = AF_UNIX;
    strcpy(server_unix_addr.sun_path, "manager");

    // 서버와 연결 시도
    do {
        connect_result = connect(client_socket, (struct sockaddr *)&server_unix_addr, sizeof(server_unix_addr));
        sleep(1);
    } while (connect_result != -1);

    // FIFO 파일에서 사물함 개수 읽기
    file_descriptor = open("index", O_RDONLY);
    if (file_descriptor < 0) {
        perror("파일 열기 실패");
        exit(1);
    }
    read(file_descriptor, locker_number, 100);
    k = atoi(locker_number); // 사물함 개수를 정수로 변환

    // 사물함 상태 표시
    displayLockers(client_socket, k);

    // 사용자 메뉴 반복
    while (1) {    
        int choice;
        printf("1. 사물함 사용\n");
        printf("2. 사물함 내용물 삭제\n");
        printf("3. 관리자와 대화\n");
        printf("선택하세요: ");
        scanf("%d", &choice);

        // 사용자의 선택에 따라 함수 호출
        if (choice == 1) {
            processLockerUsage(client_socket, k);
        } else if (choice == 2) {
            deleteLockerContent(client_socket, k);
        } else if (choice == 3) {
            processChatMode(client_socket);
        }

        // 계속 여부 확인
        printf("계속 하시려면 'y'를 입력해 주세요: ");
        scanf(" %c", &prompt);

        // 서버에 사용자 입력 전송
        write(client_socket, &prompt, 1);

        // 'N' 입력 시 프로그램 종료
        if (prompt == 'N') {
            break;
        }
    }

    close(client_socket); // 클라이언트 소켓 닫기
    return 0;
}

// 사물함 상태를 표시하는 함수
void displayLockers(int client_socket, int k) {
    char empty_locker; // 사물함 상태를 저장할 변수
    int j; // 상태를 정수로 변환하기 위한 변수

    // 표 헤더 출력
    printf("┌───────────────┬───────────────┐\n");
    printf("│  Locker Number│    Status     │\n");
    printf("├───────────────┼───────────────┤\n");

    // 각 사물함 상태 출력
    for (int i = 1; i <= k; i++) {
        read(client_socket, &empty_locker, sizeof(empty_locker));
        j = empty_locker - '0';
        if (j == 1) {
            printf("│       %2d      │    Empty      │\n", i);
        } else if (j == 0) {
            printf("│       %2d      │  Occupied     │\n", i);
        }
    }
    printf("└───────────────┴───────────────┘\n");
}

// 관리자와 채팅하는 함수
void processChatMode(int client_socket) {
    char send_message[300], receive_message[300];
    char mode = 'c'; // 채팅 모드

    // 채팅 모드 시작 신호 전송
    write(client_socket, &mode, 1);
    printf("\n관리자와의 채팅이 시작됩니다(종료하려면 Q를 입력)\n");

    while (1) {
        // 메시지 입력 및 전송
        printf("Client : ");
        scanf(" %[^\n]s", send_message);    
        write(client_socket, send_message, sizeof(send_message));

        // 'Q' 입력 시 채팅 종료
        if (!strcmp(send_message, "Q")) break;

        // 관리자 메시지 수신 및 출력
        read(client_socket, receive_message, sizeof(receive_message));
        printf("Manager : %s\n", receive_message);
    }

    // 관리자와의 채팅이 끝나면 메인 메뉴로 돌아가기 위한 메시지
    printf("\n관리자와의 채팅이 종료..메인 메뉴로 돌아갑니다.\n");
}

// 사물함을 사용하는 함수
void processLockerUsage(int client_socket, int k) {
    char user_id[100], password[10], confirm_password[10];
    char content[300];
    char mode, available;
    int input_amount, total_amount;
    int locker_id;

    printf("\n\n사용할 사물함 번호를 입력해주세요. - 사물함 번호 (1 - %d)\n : ", k);
    scanf("%s", user_id);
    locker_id = atoi(user_id);

    // 유효하지 않은 사물함 번호 처리
    if (locker_id > k) {
        printf("사용할 수 없는 사물함 번호입니다.\n");
        return;
    }

    mode = 'n'; // 사물함 사용 모드
    write(client_socket, &mode, 1);
    sleep(1);
    write(client_socket, user_id, sizeof(user_id));
    read(client_socket, &available, 1);

    // 사물함이 비어 있는 경우
    if (available == 'Y') {
        total_amount = 1000;
        while (total_amount > 0) {
            printf("사물함을 이용하러면 %d원을 넣으세요 : ", total_amount);
            scanf("%d", &input_amount);
            total_amount -= input_amount;
            if (total_amount < 0) {
                printf("거스름돈을 받으세요 ===> %d원 \n", -total_amount);
                total_amount = 0;
            }
        }    

        // 비밀번호 설정
        printf("%s번 사물함의 비밀번호를 설정하시오 : ", user_id);
        scanf("%s", password);

        do {
            printf("설정할 비밀번호를 다시 입력하시오 : ");
            scanf("%s", confirm_password);
        } while (strcmp(password, confirm_password) != 0);

        write(client_socket, password, sizeof(password));

        // 사물함 내용물 입력
        printf("사물함에 넣을 내용물을 하나씩 입력하세요 (입력을 마치려면 'done'을 입력하세요):\n");
        while (1) {
            printf("내용물: ");
            scanf(" %[^\n]s", content);
            write(client_socket, content, sizeof(content));
            if (strcmp(content, "done") == 0) break;
        }
    } else if (available == 'N') {
        char input_password[10], output_password[10];
        // 비밀번호 확인 후 사용 종료
        printf("%s번 사물함 이용을 종료하려면 비밀번호를 입력하시오 : ", user_id);
        scanf("%s", input_password);
        write(client_socket, input_password, sizeof(input_password));
        sleep(1);
        read(client_socket, output_password, sizeof(output_password));

        if (!strcmp(output_password, input_password)) {
            printf("사용을 종료합니다..\n\n");
        } else {
            printf("비밀번호가 틀렸습니다.\n 초기화면으로 돌아갑니다.\n\n");
        }
    }
}

// 사물함 내용물을 삭제하는 함수
void deleteLockerContent(int client_socket, int k) {
    char user_id[100], password[10], item_to_delete[300];
    char mode = 'd'; // 삭제 모드
    char response[100];
   
    // 삭제할 사물함 번호 입력
    printf("내용물을 삭제할 사물함 번호를 입력하세요 (1 to %d): ", k);
    scanf("%s", user_id);
   
    write(client_socket, &mode, 1); // 삭제 모드 설정
    sleep(1);
    write(client_socket, user_id, sizeof(user_id));
   
    // 비밀번호 입력
    printf("%s번 사물함의 비밀번호를 입력하세요: ", user_id);
    scanf("%s", password);
   
    write(client_socket, password, sizeof(password));
   
    // 삭제할 내용물 입력
    printf("삭제할 내용물을 입력하세요: ");
    scanf(" %[^\n]s", item_to_delete);
    write(client_socket, item_to_delete, sizeof(item_to_delete));
   
    // 서버의 응답 출력
    read(client_socket, response, sizeof(response));
   
    printf("%s\n", response);
}

