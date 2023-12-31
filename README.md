# C++ Networked Dungeon Team Project

This was an open-ended team project for a Networking class at NHTI. 

Our focus (and most of our time) was getting a C++ socket-based networking project up and running for the first time rather than just the game logic itself. Because of this, the code quality is not where I would ideally have liked it to be, but I am proud that we reached our deadline with an understanding of the topics covered. (Given more time I have listed some things I would clean up below)

For our project, I and my teammate wanted to try and create a basic text-based game using what we had learned that semester about C++ and Sockets. Other teams worked on networked programs of their choosing such as a chat room, or tic-tac-toe for example. We ended up settling on a dungeon crawler-type game where you choose your class, and can run into various types of enemies.

Project Team:
- Connor Lundstedt
- Ryan Black

## 	:world_map: Project Navigation

Server code (gameplay loop) [can be found here](https://github.com/clundstedt225/CppNetworkedDungeon/blob/main/Lundstedt_Black_Sockets/UDP/serverOutline/Source.cpp).

## 	:hammer_and_wrench: Tools & Languages Used
- Microsoft Visual Studio IDE
- Programmed using C++
- Github for Source Control

##	:stopwatch: Things I would refactor with more time
- Relocate code definitions such as enums to header files
- Clean up repeated print statements into simpler helper functions
- Attempt to simplify the game loop flow to reduce the amount of if-else statements for readability
- Print games state client-side rather than on the server only
