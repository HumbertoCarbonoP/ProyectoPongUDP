import socket
import threading
import pygame
from pygame.locals import QUIT
import socket

FPS = 60
BLANCO = (255, 255, 255)
NEGRO = (0, 0, 0)

SERVER_IP = '34.204.8.255'
SERVER_PORT = 8080
BUFFER_SIZE = 1024

start_game = False
score_1 = 0
score_2 = 0
running = True


class ballPong:
    def __init__(self, fichero_imagen):
        self.imagen = pygame.image.load(fichero_imagen).convert_alpha()

        self.x = 380
        self.y = 280


class paddlePong:
    def __init__(self):
        self.imagen = pygame.image.load("paddle.png").convert_alpha()

        self.x = 0
        self.y = 250

        self.dir_y = 0

pygame.init()
ventana = pygame.display.set_mode((800, 600))
pygame.display.set_caption("Pong")

ball = ballPong("ball.png")
paddle_1 = paddlePong()
paddle_1.x = 60
paddle_2 = paddlePong()
paddle_2.x = 800 - 60 - 22

def update_positions_and_screen():
    font = pygame.font.Font(None, 60)
    ventana.fill(BLANCO)
    
    ventana.blit(ball.imagen, (ball.x, ball.y))
    ventana.blit(paddle_1.imagen, (paddle_1.x, paddle_1.y))
    ventana.blit(paddle_2.imagen, (paddle_2.x, paddle_2.y))
    text = f"{score_1} : {score_2}"
    sign = font.render(text, False, NEGRO)
    ventana.blit(sign, (800 / 2 - font.size(text)[0] / 2, 50))
    
    pygame.display.flip()

def listen_for_messages(client_socket):
    global start_game
    global score_1
    global score_2
    while True:
        data, _ = client_socket.recvfrom(BUFFER_SIZE)
        if data:
            data = data.decode()
            if data == "START":
                start_game = True
                print("Todos los jugadores están conectados. ¡El juego ha comenzado!")
                continue
            print("Posiciones: " + data)
            data = data.split()
            ball.x, ball.y, paddle_1.y, paddle_2.y, score_1, score_2 = map(int, data)
            
            print(f"ball_x: {ball.x}, ball_y: {ball.y}, paddle_1_y: {paddle_1.y}, paddle_2_y: {paddle_2.y}")
            update_positions_and_screen()

def main():
    fuente = pygame.font.Font(None, 60)
    
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    client_socket.sendto("CONNECT".encode(), (SERVER_IP, SERVER_PORT))
    print("Conectado al servidor.")
    
    listener_thread = threading.Thread(target=listen_for_messages, args=(client_socket,))
    listener_thread.daemon = True
    listener_thread.start()
    
    print("Esperando a que todos los jugadores se conecten...")
    while not start_game:
        pass
    
    global running
    clock = pygame.time.Clock()
    current_movement = "0"

    while running:
        update_positions_and_screen()
        if (score_1 == 5 or score_2 == 5):
            running = False
            continue
        
        for event in pygame.event.get():
            if (event.type == QUIT):
                running = False

            # Detecta que se ha pulsado una tecla
            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_w:
                    current_movement = "-5"
                if event.key == pygame.K_s:
                    current_movement = "5"

            # Detecta que se ha soltado la tecla
            if event.type == pygame.KEYUP:
                if event.key == pygame.K_w or event.key == pygame.K_s:
                    current_movement = "0"

        # Enviar movimiento actual al servidor
        client_socket.sendto(current_movement.encode(), (SERVER_IP, SERVER_PORT))
            
        pygame.display.flip()
        clock.tick(FPS)

    pygame.quit()
    client_socket.close()

if __name__ == "__main__":
    main()