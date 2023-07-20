# Quiz Game Project

This project was developed as part of a class project by Timothé Dupuch and myself. The aim of the project was to create a multi-threaded application inspired by the popular game "Questions pour un Champion" to simulate a quiz game.

## Project Overview

The quiz game application is designed to provide an interactive and challenging experience for players. The server thread is responsible for managing the game flow, communicating questions to clients, and collecting their responses and thinking times. The clients compete by buzzing in and providing answers to the questions posed by the server.

The game follows a set of rules, where a correct answer earns the player 5 points, and an incorrect answer results in a deduction of 1 point. The buzzer of a client who answers incorrectly is blocked for the current question, while other clients can continue to participate. The server displays real-time scores and rankings as the game progresses.

## Features

- Multi-threaded architecture: The application utilizes a server thread and dedicated client threads for efficient communication and interaction.
- Communication: The server thread sends questions to clients and receives their responses, facilitating real-time gameplay.
- Synchronization: Proper synchronization mechanisms, such as mutex and semaphores, are implemented to ensure coordinated actions among clients and the server.
- Pseudo-random number generation: Each client generates a random number independently, simulating thinking times before buzzing in with an answer.
- Score tracking and ranking: The server keeps track of scores and displays real-time rankings to create a competitive environment.

## Installation

1. Clone the repository to your local machine.
2. Compile the source code using the provided `Makefile`.
3. Run the server executable.
4. Connect multiple clients to the server using separate terminals.

## Usage

1. Start the server by executing the server executable.
2. Connect clients to the server by running the client executable in separate terminals.
3. The server will initiate the game and present questions to the connected clients.
4. Clients can buzz in with their answers and receive feedback from the server.
5. The server will update scores in real-time and display rankings.
6. The game continues with multiple rounds until a certain number of questions have been answered.

## Challenges Faced

During the development of this project, we encountered several challenges, including:

- Communication between the server and clients: Implementing bidirectional communication required careful handling of messages and synchronization to ensure smooth gameplay.
- Synchronization of client actions: Coordinating client actions, such as waiting for all clients to connect or allowing only one client to buzz in at a time, required effective use of mutexes and semaphores.
- Pseudo-random number generation: Generating independent random numbers for each client was necessary to simulate individual thinking times and create a fair gameplay experience.
- Managing the sequence of actions: Properly sequencing the steps of the game, such as sending questions, collecting responses, and updating scores, was crucial to maintain a coherent gameplay flow.

## Future Enhancements

- Graphical User Interface (GUI): Enhance the user experience by developing a GUI to display questions, scores, and rankings.
- Expanded question database: Increase the variety and depth of questions by integrating a larger question database or allowing customization of question sets.
- Difficulty levels: Implement different difficulty levels to cater to players of varying skill levels.
- Multiplayer support: Enable players to compete with each other over a network, adding a competitive multiplayer aspect to the game.

## Contributions

Contributions to the project are welcome. If you encounter any issues or have suggestions for improvements, please feel free to submit a pull request or open an issue in the project repository.

