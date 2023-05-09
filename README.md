# TomentOnline
A Software Rendering Raycaster Engine with multiplayer written in C and SDL2.

![123](https://github.com/silvematt/TomentOnline/assets/20478938/c8d513da-2b94-4704-a8d6-87cac39fc87c)


Video: https://youtu.be/1Z0X3V8cNl4

TomentOnline is an upgraded version of the [TomentRaycaster](https://github.com/silvematt/TomentRaycaster) engine, that adds multiplayer capabilities and more.
For the Online part of it, I've read through many online resouces and the book "Multiplayer Game Programming: Architecting Networked Games" has been of great help.

This was another pilgrimage to my God John Carmack and my starting point for multiplayer game programming. It took me about 1 month.


<h2>Features, on top of TomentRaycaster:</h2>

Rendering:
- Better performances.
- Maps can be as big as 128x128 (but smaller are also supported)
- Linear fog rendering
- 8 angled sprites rendering

Multiplayer:
- TCP socket connections (using Winsock), P2P architecture with the host being responsabile of running the AI
- Structured packets with a max size of 1300 bytes for data (1302 in total)
- Buffered send/receive to allow many packets per frame to be sent/received (while accounting for short receive/send - thanks to [skeeto](https://github.com/skeeto) for pointing me in the right direction)

Gameplay:
- Two players per dungeon
- Lobby and class selection (Tank, Healer, DPS)
- Player skills with cooldowns for each class
- Damage puddles
- 2 new bosses (3 in total) with new mechanics


<h2>Other:</h2>
The players connect to each other using the host/join buttons in the main menu, from there the host inserts its username, while the joiner needs to insert the IP Address, Port (61530) and username.
<br><br>
If you open the port 61530 on your router you can let any other player join your matches, otherwise you can play locally by insert the local IPs of your devices.
As for now the communication is 100% trusted, there are no counter measure or bad-input checking, so if you wish to test it make sure to play with someone who you trust will not cheat.
<br><br>
With "The Frozen End" dungeon I've probably squeezed the engine as much as possible, especially with the bosses behaviors and mechanics, this will likely be the last "Toment" project.
<br><br>
The game data is packed in custom files (.archt) format [GENERATED WITH: [TomentARCH](https://github.com/silvematt/TomentARCH)]
