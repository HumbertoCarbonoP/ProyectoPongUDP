Proyecto de comunicación UDP en juego Pong Cliente-Servidor.
Octubre 16 de 2023.
Humberto Antonio Carbonó Pedroza.
David González Idárraga.

Introducción
El proyecto actual busca fortalecer y actualizar la infraestructura asociada con el reconocido juego "Pong". Una de las metas centrales es que el software pueda intercambiar mensajes de manera estructurada, asegurando una interacción fluida entre las diferentes aplicaciones y posibilitando la conexión de varios usuarios al mismo tiempo.
Dado el énfasis en la transferencia de información, hemos optado por un diseño arquitectónico que se rige por el esquema cliente/servidor. En esta configuración, el cliente emite peticiones y el servidor las procesa. Como canal de comunicación entre las aplicaciones, se ha seleccionado el socket Datagram, que trabaja con el protocolo UDP, el cual es el elegido para la realización de este proyecto.
Concluyendo, al integrar todos estos componentes, logramos que el proyecto TelePong cumpla con sus objetivos fundamentales: un software que se interconecte eficazmente por la red y que no se confine a una conexión individual.

Desarrollo
1. Elección e Implementación Inicial del Juego: Empezamos nuestro proyecto investigando cómo llevar a cabo el desarrollo del clásico juego “Pong”. Optamos por usar Python como lenguaje de programación debido a su simplicidad y versatilidad. En el proceso, descubrimos la librería 'pygame', que facilita la creación de juegos. Su intuitiva estructura y funciones permitieron que el proceso de implementación fuera más sencillo, garantizando una base robusta para el proyecto. Con la ayuda de un tutorial, comprendimos la lógica y matemática detrás del juego, asegurando que no existieran errores, como la bola saliendo de la pantalla o puntuaciones erróneas.
2. Desarrollo del Servidor en C: Para la infraestructura del servidor, se vio necesario programar en C. Se nos fue proporcionada una excelente referencia de Geek4Geeks que nos proporcionó una comprensión clara y un punto de partida sólido para el desarrollo del servidor.
3. Conexión y Lógica del Juego: Al profundizar en el proyecto, decidimos colocar la lógica del juego, particularmente los movimientos de las paletas y la bola, dentro del servidor. Esta decisión surgió después de observar retrasos en la comunicación cuando la lógica se encontraba en el cliente. Al trasladar esta lógica al servidor, logramos que los movimientos se reflejaran casi instantáneamente en las pantallas de todos los jugadores, eliminando lag y retrasos.
4. Comunicación y Concurrencia: La concurrencia fue un aspecto crucial en nuestro proyecto. Investigamos y logramos implementar una función en el servidor llamada 'Handle Game', que gestionaba la comunicación entre clientes y el servidor. Esta función también contenía la lógica del juego para garantizar una comunicación rápida entre todos los participantes. Para abordar la concurrencia, desarrollamos otra función que permitía a dos clientes conectarse a un mismo hilo y socket, asegurando así una comunicación fluida y sin interferencias entre partidas diferentes.
5. Estructura de Archivos y Protocolo: El proyecto "Pong" no se basa en un solo archivo. Se compone de varios archivos, incluyendo 'pong' (el juego principal), 'constants' (donde se definen las variables constantes, con la excepción de la IP), y 'server' (que gestiona la creación de sockets y la comunicación). El archivo principal, 'pong', tiene una estructura clara: contiene funciones y clases esenciales para el juego, como la pelota y las paletas. La función principal 'main' inicializa y ejecuta el juego, maneja la interacción del usuario, y se comunica con el servidor a través del archivo 'server'.
El servidor tiene una función vital. No solo gestiona la lógica del juego, sino que también registra movimientos y posibles errores en un 'logger'. Además, se asegura de que no existan interferencias o comunicaciones cruzadas entre distintas partidas.

Conclusión 
Al comienzo del proyecto, nos cuestionamos qué protocolo y socket eran más adecuados para que nuestra aplicación tuviera un rendimiento óptimo. Nuestro equipo buscaba brindar una experiencia de juego fluida y con reacciones casi instantáneas. Por ello, elegimos el protocolo UDP y el socket Datagram, dada su rapidez y eficiencia. Al concluir el desarrollo, constatamos que nuestra elección fue acertada. Aunque esta decisión puede incrementar el riesgo de pérdidas de información, el juego ha operado sin inconvenientes notables, garantizando una buena experiencia para el usuario y un juego ágil.

Referencias 
Amazon web services (AWS) tutorial. (2023, enero 23). GeeksforGeeks. https://www.geeksforgeeks.org/aws-tutorial/

¿Cómo funcionan los hilos en programación? (s/f). ..Ed.team. Recuperado el 16 de octubre de 2023, de https://ed.team/blog/como-funcionan-los-hilos-en-programacion

Get started with C. (s/f). W3schools.com. Recuperado el 16 de octubre de 2023, de https://www.w3schools.com/c/c_getstarted.php

Gorman, B. (2023, febrero 23). TCP frente a UDP: ¿En qué se diferencian y qué protocolo es mejor? TCP frente a UDP: ¿En qué se diferencian y qué protocolo es mejor?; Avast. https://www.avast.com/es-es/c-tcp-vs-udp-difference

Introduction to Amazon web services. (2020, mayo 9). GeeksforGeeks. https://www.geeksforgeeks.org/introduction-to-amazon-web-services/

(S/f). Cloudflare.com. Recuperado el 16 de octubre de 2023, de https://www.cloudflare.com/es-es/learning/network-layer/what-is-a-protocol/

(S/f). Cloudflare.com. Recuperado el 16 de octubre de 2023, de https://www.cloudflare.com/es-es/learning/network-layer/what-is-a-protocol/

