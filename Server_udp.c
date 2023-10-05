#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define CLIENT_COUNT 2

typedef struct {
    int dir_x;
    int dir_y;
    int x;
    int y;
    int puntuacion_1;
    int puntuacion_2;
} Ball;

void moveBall(Ball* ball) {
    ball->x += ball->dir_x;
    ball->y += ball->dir_y;
}

void reset(Ball* ball) {
    srand(time(NULL));
    ball->x = 380;
    ball->y = 280;
    ball->dir_x = -(ball->dir_x);
    ball->dir_y = (rand() % 2) ? -2 : 2;
}

void bounce(Ball* ball) {
    if (ball->x <= -40) {
        reset(ball);
        ball->puntuacion_2 += 1;
    }
    if (ball->x >= 800) {
        reset(ball);
        ball->puntuacion_1 += 1;
    }
    if (ball->y <= 0) {
        ball->dir_y = -(ball->dir_y);
    }
    if (ball->y + 40 >= 600) {
        ball->dir_y = -(ball->dir_y);
    }
}

typedef struct {
    int dir_y;
    int x;
    int y;
} Paddle;

void movePaddle(Paddle* paddle) {
    paddle->y += paddle->dir_y;
    if (paddle->y <= 0) {
        paddle->y = 0;
    }
    if (paddle->y + 100 >= 600) {
        paddle->y = 600 - 100;
    }
}

void hit(Paddle* paddle, Ball* ball, int isPaddle1) {
    if (
        ball->x < paddle->x + 22
        && ball->x > paddle->x
        && ball->y + 40 > paddle->y
        && ball->y < paddle->y + 100
    ) {
        ball->dir_x = -ball->dir_x;
        
        // Si es la paleta 1
        if (isPaddle1) {
            ball->x = paddle->x + 22;
        }
        // Si es la paleta 2
        else {
            ball->x = paddle->x - 40;
        }
    }
}

int find_client_index(struct sockaddr_in *clientAddrs, struct sockaddr_in client)
{
    for (int i = 0; i < CLIENT_COUNT; i++){
        if (clientAddrs[i].sin_addr.s_addr == client.sin_addr.s_addr &&
            clientAddrs[i].sin_port == client.sin_port)
        {
            return i;
        }
    }
    return -1;
}


int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <PORT> <Log File>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    int port = atoi(argv[1]);
    char *logFile = argv[2];

    srand(time(NULL));
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in serverAddr, clientAddr, clientAddrs[CLIENT_COUNT];
    int connected_clients = 0;
    socklen_t addr_size = sizeof(struct sockaddr_in);

    memset(clientAddrs, 0, sizeof(clientAddrs));

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Error al enlazar");
        close(sockfd);
        exit(1);
    }

    printf("Esperando a que se conecten los clientes...\n");

    while (connected_clients < CLIENT_COUNT)
    {
        int bytesReceived = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&clientAddr, &addr_size);
        if (bytesReceived > 0)
        {
            buffer[bytesReceived] = '\0';
            if (strcmp(buffer, "CONNECT") == 0 && find_client_index(clientAddrs, clientAddr) == -1)
            {
                clientAddrs[connected_clients] = clientAddr;
                connected_clients++;
                printf("Cliente %d conectado desde %s:%d\n", connected_clients, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
            }
        }
    }

    char start_msg[] = "START";
    for (int i = 0; i < CLIENT_COUNT; i++) {
        sendto(sockfd, start_msg, strlen(start_msg), 0, (struct sockaddr *)&clientAddrs[i], addr_size);
    }

    printf("Todos los clientes están conectados...\n");

    char initial_msg[] = "5 5 0 0 0 0";
    for (int i = 0; i < CLIENT_COUNT; i++) {
        sendto(sockfd, initial_msg, strlen(initial_msg), 0, (struct sockaddr *)&clientAddrs[i], addr_size);
    }

    Ball ball;

    ball.x = 380;
    ball.y = 280;

    int random_number = rand() % 2;
    ball.dir_x = (random_number == 0) ? -2 : 2;
    random_number = rand() % 2;
    ball.dir_y = (random_number == 0) ? -2 : 2;

    ball.puntuacion_1 = 0;
    ball.puntuacion_2 = 0;

    Paddle paddle_1, paddle_2;

    paddle_1.x = 60;
    paddle_1.y = 250;
    paddle_1.dir_y = 0;

    paddle_2.x = 718;
    paddle_2.y = 250;
    paddle_2.dir_y = 0;

    while (1){
        int bytesReceived = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&clientAddr, &addr_size);
        if (bytesReceived > 0)
        {
            buffer[bytesReceived] = '\0';

            int sendingClientIndex = find_client_index(clientAddrs, clientAddr);
            printf("Cliente que envía: %d\n", sendingClientIndex);

            if(sendingClientIndex == 0){
                paddle_1.dir_y = atoi(buffer);
                printf("Dirección 1: %d\n", paddle_1.dir_y);
            } else if(sendingClientIndex == 1) {
                paddle_2.dir_y = atoi(buffer);
                printf("Dirección 2: %d\n", paddle_2.dir_y);
            }

            moveBall(&ball);
            bounce(&ball);
            movePaddle(&paddle_1);
            movePaddle(&paddle_2);
            hit(&paddle_1, &ball, 1);
            hit(&paddle_2, &ball, 0);

            // Reenviando datos a ambos clientes
            for (int clientIndex = 0; clientIndex < CLIENT_COUNT; clientIndex++)
            {
                char tempBuffer2[256];
                sprintf(tempBuffer2, "%d %d %d %d %d %d", ball.x, ball.y, paddle_1.y, paddle_2.y, ball.puntuacion_1, ball.puntuacion_2);
                sendto(sockfd, tempBuffer2, strlen(tempBuffer2), 0, (struct sockaddr *)&clientAddrs[clientIndex], addr_size);
            }
        }
    }

    close(sockfd);
    return 0;
}