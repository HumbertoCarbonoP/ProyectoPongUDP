#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

FILE *logfd;

typedef struct
{
    int dir_x;
    int dir_y;
    int x;
    int y;
    int score_1;
    int score_2;
} Ball;

void moveBall(Ball *ball)
{
    ball->x += ball->dir_x;
    ball->y += ball->dir_y;
}

void reset(Ball *ball)
{
    srand(time(NULL));
    ball->x = 380;
    ball->y = 280;
    ball->dir_x = -(ball->dir_x);
    ball->dir_y = (rand() % 2) ? -2 : 2;
}

void bounce(Ball *ball)
{
    if (ball->x <= -40)
    {
        reset(ball);
        ball->score_2 += 1;
    }
    if (ball->x >= 800)
    {
        reset(ball);
        ball->score_1 += 1;
    }
    if (ball->y <= 0 || ball->y + 40 >= 600)
    {
        ball->dir_y = -(ball->dir_y);
    }
}

typedef struct
{
    int dir_y;
    int x;
    int y;
} Paddle;

void movePaddle(Paddle *paddle)
{
    paddle->y += paddle->dir_y;
    if (paddle->y <= 0)
    {
        paddle->y = 0;
    }
    if (paddle->y + 100 >= 600)
    {
        paddle->y = 600 - 100;
    }
}

void hit(Paddle *paddle, Ball *ball, int isPaddle1)
{
    if (ball->x < paddle->x + 22 && ball->x > paddle->x && ball->y + 40 > paddle->y && ball->y < paddle->y + 100)
    {
        ball->dir_x = -ball->dir_x;
        ball->x = isPaddle1 ? paddle->x + 22 : paddle->x - 40;
    }
}

int findClientIndex(struct sockaddr_in *clientAddrs, struct sockaddr_in client)
{
    for (int i = 0; i < 2; i++)
    {
        if (clientAddrs[i].sin_addr.s_addr == client.sin_addr.s_addr &&
            clientAddrs[i].sin_port == client.sin_port)
        {
            return i;
        }
    }
    return -1;
}

void logInfo(const char *message)
{
    fprintf(logfd, "[INFO] %s\n", message);
    fflush(logfd);
}

void logError(const char *message)
{
    fprintf(logfd, "[ERROR] %s\n", message);
    fflush(logfd);
}

typedef struct
{
    int sockfd;
    struct sockaddr_in serverAddr, clientAddr, clientAddrs[2];
    int addr_size;
} GameThreadArgs;

void *handleGame(void *arg)
{
    GameThreadArgs *args = (GameThreadArgs *)arg;
    char startMsg[] = "START";
    for (int i = 0; i < 2; i++)
    {
        sendto(args->sockfd, startMsg, strlen(startMsg), 0, (struct sockaddr *)&args->clientAddrs[i], args->addr_size);
    }

    printf("All clients connected...\n");
    logInfo("All clients connected...");

    Ball ball = { .x = 380, .y = 280, .dir_x = (rand() % 2) ? -2 : 2, .dir_y = (rand() % 2) ? -2 : 2, .score_1 = 0, .score_2 = 0 };
    Paddle paddle_1 = { .x = 60, .y = 250, .dir_y = 0 };
    Paddle paddle_2 = { .x = 718, .y = 250, .dir_y = 0 };

    char buffer[BUFFER_SIZE];

    while (1)
    {
        int bytesReceived = recvfrom(args->sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&args->clientAddr, &args->addr_size);
        if (bytesReceived > 0)
        {
            buffer[bytesReceived] = '\0';

            int sendingClientIndex = findClientIndex(args->clientAddrs, args->clientAddr);
            char logMessage[256];
            sprintf(logMessage, "Client %d sends: %d.", sendingClientIndex, atoi(buffer));
            printf("%s\n", logMessage);
            logInfo(logMessage);

            if (sendingClientIndex == 0)
            {
                paddle_1.dir_y = atoi(buffer);
            }
            else if (sendingClientIndex == 1)
            {
                paddle_2.dir_y = atoi(buffer);
            }

            moveBall(&ball);
            bounce(&ball);
            movePaddle(&paddle_1);
            movePaddle(&paddle_2);
            hit(&paddle_1, &ball, 1);
            hit(&paddle_2, &ball, 0);

            for (int clientIndex = 0; clientIndex < 2; clientIndex++)
            {
                char tempBuffer2[256];
                sprintf(tempBuffer2, "%d %d %d %d %d %d", ball.x, ball.y, paddle_1.y, paddle_2.y, ball.score_1, ball.score_2);
                sendto(args->sockfd, tempBuffer2, strlen(tempBuffer2), 0, (struct sockaddr *)&args->clientAddrs[clientIndex], args->addr_size);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <PORT> <Log File>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    char *logFile = argv[2];

    logfd = fopen(logFile, "a");
    if (!logfd)
    {
        perror("Error opening log file");
        exit(EXIT_FAILURE);
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Binding error");
        close(sockfd);
        exit(1);
    }

    struct sockaddr_in clientAddr;
    socklen_t addr_size = sizeof(clientAddr);
    struct sockaddr_in clientAddrs[2];

    char buffer[BUFFER_SIZE];

    while (1)
    {
        int bytesReceived = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&clientAddr, &addr_size);
        if (bytesReceived > 0)
        {
            buffer[bytesReceived] = '\0';

            if (strcmp(buffer, "CONNECT") == 0)
            {
                int clientIndex = findClientIndex(clientAddrs, clientAddr);
                if (clientIndex == -1)
                {
                    for (int i = 0; i < 2; i++)
                    {
                        if (clientAddrs[i].sin_port == 0)
                        {
                            clientAddrs[i] = clientAddr;
                            printf("Client %d connected...\n", i);
                            logInfo("Client connected...");

                            if (i == 1)
                            {
                                GameThreadArgs args = { .sockfd = sockfd, .serverAddr = serverAddr, .clientAddr = clientAddr, .addr_size = addr_size };
                                memcpy(args.clientAddrs, clientAddrs, sizeof(clientAddrs));
                                pthread_t gameThread;
                                pthread_create(&gameThread, NULL, handleGame, &args);
                                pthread_detach(gameThread);
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    close(sockfd);
    fclose(logfd);
    return 0;
}
