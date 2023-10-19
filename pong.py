import socket
import threading
import pygame
from pygame.locals import QUIT
import settings

start_game = False
score_1 = 0
score_2 = 0
running = True


class BallPong:
    def __init__(self, image_file):
        self.image = pygame.image.load(image_file).convert_alpha()
        self.x = 380
        self.y = 280
class PaddlePong:
    def __init__(self):
        self.image = pygame.image.load("paddle.png").convert_alpha()
        self.x = 0
        self.y = 250
        self.dir_y = 0


pygame.init()
window = pygame.display.set_mode((800, 600))
pygame.display.set_caption("Pong")

ball = BallPong("ball.png")
paddle_1 = PaddlePong()
paddle_1.x = 60
paddle_2 = PaddlePong()
paddle_2.x = 800 - 60 - 22


def update_positions_and_screen():
    font = pygame.font.Font(None, 60)
    window.fill(settings.WHITE)

    window.blit(ball.image, (ball.x, ball.y))
    window.blit(paddle_1.image, (paddle_1.x, paddle_1.y))
    window.blit(paddle_2.image, (paddle_2.x, paddle_2.y))
    text = f"{score_1} : {score_2}"
    sign = font.render(text, False, settings.BLACK)
    window.blit(sign, (800 / 2 - font.size(text)[0] / 2, 50))

    pygame.display.flip()


def listen_for_messages(client_socket):
    global start_game, score_1, score_2
    while True:
        data, _ = client_socket.recvfrom(settings.BUFFER_SIZE)
        if data:
            data = data.decode()
            if data == "START":
                start_game = True
                print("All players connected. Game started!")
                continue
            data_values = list(map(int, data.split()))
            ball.x, ball.y, paddle_1.y, paddle_2.y, score_1, score_2 = data_values
            print(f"Data received from the server:\n    (ball.x: {ball.x}), (ball.y: {ball.y}, (paddle_1: {paddle_1.y}), (paddle_2: {paddle_2.y}), (score_1: {score_1}), (score_2: {score_2})")
            if not running:
                break
            update_positions_and_screen()


def show_message(message, y_offset=0):
    font = pygame.font.Font(None, 60)
    text_surface = font.render(message, True, settings.BLACK)
    text_rect = text_surface.get_rect(center=(400, 300 + y_offset))
    window.blit(text_surface, text_rect)
    pygame.display.flip()


def main():
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    client_socket.sendto("CONNECT".encode(), (settings.SERVER_IP, settings.SERVER_PORT))
    print("Connected to server.")

    listener_thread = threading.Thread(target=listen_for_messages, args=(client_socket,))
    listener_thread.daemon = True
    listener_thread.start()

    window.fill(settings.WHITE)
    show_message("Welcome to Pong!", -20)
    show_message("Waiting for another player...", 20)

    global running
    clock = pygame.time.Clock()
    current_movement = "0"

    while not start_game:
        for event in pygame.event.get():
            if event.type == QUIT:
                pygame.quit()
                return

    while running:
        update_positions_and_screen()
        if score_1 == 5 or score_2 == 5:
            running = False

        for event in pygame.event.get():
            if event.type == QUIT:
                running = False

            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_w:
                    current_movement = "-5"
                elif event.key == pygame.K_s:
                    current_movement = "5"
            elif event.type == pygame.KEYUP:
                if event.key in [pygame.K_w, pygame.K_s]:
                    current_movement = "0"

        client_socket.sendto(current_movement.encode(), (settings.SERVER_IP, settings.SERVER_PORT))
        clock.tick(settings.FPS)

    window.fill(settings.WHITE)
    if score_1 == 5:
        show_message("Player 1 wins!", -20)
    elif score_2 == 5:
        show_message("Player 2 wins!", -20)
    else:
        show_message("Exiting the game...", -20)
    show_message("Closing in 5 seconds...", 20)

    pygame.time.wait(5000)
    pygame.quit()
    client_socket.close()

if __name__ == "__main__":
    main()