서버 - 클라이언트(server - client) 모델로 동작하는 사물함 관리 시스템.
▶ 0. 컴파일

gcc -o server server.c gcc -o client client.c

▶ 1. 프로젝트 내용 소개

▶ 1-1. 내용 소개

유닉스 시스템 수업시간에 배운 내용을 바탕으로 사물함 관리 시스템을 구현. 서버 - 클라이언트(server - client) 모델로 동작하는 사물함 관리 시스템.

▶ 1-2. 개념 설명 서버 : 사물함을 관리하는 역할을 수행. 사물함 관리, 클라이언트들에게 사물함 제공 및 비밀번호 설정을 할 수 있도록 함. 클라이언트 : 사물함 관리 시스템의 고객 역할을 수행. 사물함 사용 및 비밀번호 설정을 수행.

▶ 1-3. 기본 기능

서버

◆ 사용자에게 사물함에 대한 정보를 제공 ◆ 사용자에게 사물함 할당 시 패스워드를 지정하도록 함 ◆ 사용자가 본인 사물함에 접근 시 입력하는 패스워드가 맞는지 확인하고, 맞으면 접근 가능하도록 함

클라이언트

◆ 원하는 사물함에 접근 ◆ 해당 사물함의 비밀번호 설정 및 입력 기능

→ 위 기본 기능에 추가 기능을 삽입하여 프로그램의 완성도를 높임.

▶ 2. 전체 코드의 흐름

서버 = server 클라이언트 = client 라고 지칭.

(0) 서버 클라이언트 실행 과정

서버가 실행 되면서 락커 구조체, 서버 소켓 생성된다.
서버에서 사물함 개수를 동적할당을 이용하여 받고 개수만큼 초기 세팅해서 화면에 출력
클라이언트가 실행 되면서 클라이언트 소켓 생성되고 서버에 연결요청을 함.
서버가 연결요청을 수락함.
(1) server : fd에 사물함개수 s write (1) client : 서버에서 fd에 기록한 사물함개수를 index에 read

(2) server : 현재 사물함 상태 클라이언트에 전달하는데 사물함 비어있을 경우 : emptynum='1'을 clientfd에 기록 사물함이 비어있지 않는 경우 : emptynum='0'을 clientfd에 기록 (2) client : 사물함 정보 출력 서버에서 clientfd에 기록한 값이 '1' 이면 if문 수행, '0' 이면 else-if문 수행 하여 사물함 정보 출력

(3-1) server : 클라이언트로부터 읽은 문자가 c이면 채팅모드 시작 (3-1) client : 채팅모드임을 서버에 전달함

(3-2) server : 클라이언트로부터 읽은 문자가 n이면 사물함 사용모드로 진입 (3-2) client : 사물함 번호를 입력시 -> 사물함 사용모드로 진입 (clientfd에 'n'에 write())

(4) server : clientfd에 기록된 사물함 번호를 useid에 저장 (4) client : 입력한 사물함 번호를 clientfd에 write

여기서 두가지로 갈림

// 사물함이 비어있는 경우

(5) server : clientfd에 'Y'를 write (5) client : 사물함의 사용가능여부를 읽어들인다.

(6) server : clientfd에 write한 password를 Rpassword에 read() (6) client : 비밀번호를 client에 write()

// 사물함이 비어있지 않는 경우

(5) server : clientfd에 'N'를 write

(7) server : 클라이언트에서 입력한 비밀번호를 Inputpwd에 write (7) client : 입력한 비밀번호를 Inputpwd에 write

(8) server : 사물함정보에 기록된 비밀번호를 sendpassword에 write (8) client : 사물함정보에 기록된 비밀번호를 Outputpwd에 read() 만약 inputpwd == outputpwd : 패스 inputpwd != outputpwd : 초기화면으로 돌아감

(9) server : 클라이언트 종료 여부를 QuestionFromClient에 read() -> 클라이언트 종료시 와일문 빠져나옴 (9) client : 클라이언트 종료 여부를 server에 알리기 위해 clientfd에 N write -> N 입력시 클라이언트 종료

(10) server : 클라이언트에서 업데이트된 사물함 정보를 파이프에 기록 후 업데이트된 사물함 상태를 서버에 저장 후 출력

(11) server : 클라이언트 종료시 다음 실행할 클라이언트 진행 (while 문)

▶ 3. 추가기능 ◆ 1000원을 넣어야 사물함이 열리는 시스템 ◆ 동적할당으로 사물함 크기를 지정 ◆ 서버/클라이언트 채팅 기능 ◆ 비밀번호 설정 시 두 번 입력하여 제대로 비밀번호를 설정할 수 있도록 함

▶ 4. 우수성 ◆ 동적할당으로 사물함 크기를 지정할 수 있어 사용자가 원하는 만큼의 사물함 개수를 만들 수 있다.

// 서버 최초실행시 사물함 세팅
printf("사물함 개수를 입력하세요 : ");
// 사물함 개수 입력 ( 문자열 )
scanf("%s",s);

// 구조체 포인터 동적할당 ( 사물함 개수 만큼 )
rec2 = (struct rocker *) malloc((k+1)*sizeof(struct rocker));
    
printf("\n* * * * * * * * * * * * *  Locker * * * * * * * * * * * * * \n");
printf("*ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ*\n");
// 1번 사물함부터 k번 사물함까지 사물함 초기상태 setting
for(int i=1;i<=k;i++)
{
rec2[i].id = i;
rec2[i].empty = 1;
strcpy(rec2[i].pwd,"0000");
printf("|      Rocker Num : %d        |       Empty :     Yes       |\n",i);
printf("*ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ * ㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡㅡ*\n");
}
◆ 서버/클라이언트 간의 채팅 기능을 이용하여 클라이언트 사용자가 서버 관리자에게 궁금한 점을 질문할 수 있다.

// 입력한 번호가 66096301 -> 채팅 시작 
        if(!strcmp(useid,"66096301"))
        {   
            // (3)-1. 채팅모드임을 서버에 전달 
            m='c';
            write(clientfd,&m,1);
            printf("\n관리자와의 채팅을 시작합니다. 종료하시려면 Q를 입력하세요 \n");
            while(1)
            {
                printf("Me : ");
                scanf(" %[^\n]s",Smessage); 
                write(clientfd,&Smessage,sizeof(Smessage));
                if(!strcmp(Smessage,"Q")) break;
                else
                {   
                read(clientfd, &Rmessage, sizeof(Rmessage));
                printf("Manager : %s\n",Rmessage);
                }
            }
        continue;   
        }

// (3)-1. 클라이언트로부터 읽은 문자가 c면 채팅모드 시작
            read(clientfd,&r,1);
            if(r=='c')
            {
                printf("\n\n* 고객으로부터 채팅이 왔습니다. 채팅을 시작합니다.\n");      
                while(1)
                {
                    read(clientfd,&Rmessage,sizeof(Rmessage));
                    printf("Client : %s\n",Rmessage);
                    if(!strcmp(Rmessage,"Q")) break;
                    else
                    {
                    printf("Me : ");
                    scanf(" %[^\n]s",Smessage);
                    write(clientfd,&Smessage,sizeof(Smessage));
                    }
                }
                continue;
            }
◆ 비밀번호를 설정 시 두 번 입력하여 혹여나 생각한 비밀번호를 잘못 입력하여 나중에 비밀번호를 찾거나 변경하는 문제를 미연에 방지할 수 있다.

// 비밀번호 입력 
                printf("%s번 사물함의 비밀번호를 설정해주세요 : ",useid);
                scanf("%s",password);
                
                // + 추가기능 : 비밀번호확인을위해 재입력 
                while(1)
                {
                printf("설정하실 비밀번호를 다시 입력해주세요 : ");
                scanf("%s",password2);
                if(!strcmp(password,password2)) break;
                }
