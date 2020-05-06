# Development

---
**4/18/2020**
Basic Box2D world_ and object creation and drawing established.

**4/21/2020**
Established response to user interface with keyboard controls, allowing 
moving side to side and jumping in an effective manner.

**4/21/2020 (2)**
Organized Player data and output into the player class.

**4/24/2020**
Added ability for player to shoot bullets.

**4/25/2020**
Implemented enemies into the game, as well as the abstraction of a game "entity."

**4/26/2020**
Basic Combat has been added to the game. Enemies can shoot the player,
causing them to lose lives, and the player can kill enemies by shooting them.
The game can now be lost when the player is out of lives. May still improve
combat by allowing enemies to move or target player.

**4/27/2020**
Combat has been finished. Enemies now move back and forth patrolling the area.
Enemies are immune to each others' bullets. Enemy hitboxes have fine-tuned. A counter
for tracking the player's lives has been added. Due to the structure of enemy
movement, I have chosen to not implement player targeting. The current enemy
setup will be enough of a challenge to defeat as a starting enemy.

**4/29/2020**
Updated game display to now play in fullscreen. Implemented view scrolling.
The view of the game shifts with the player as the player moves to the far
left or right edges of the window.

**5/01/2020**
All game functionality has been completed. The game is now fullscreen, with a
view window that scrolls to follow the player. The player and enemies can die
by falling off of ledges. The game can be won by reaching the end of the level. 
The game can be restarted by pressing r. Walls have been abstracted to the wall
class to improve ease of development. Now that the game is functional, only the
design for the level and aesthetics remain.

**5/02/2020**
A level has been created and the game is now in a complete, playable state.
Extensions to the level or changes in aesthetics may be added later, but the
requirements of the game are now complete.

**5/03/2020**
The game has been expanded to become more complete. Music, sound effects,
a title screen, and a finish line marker have all been added. Also the text
displayed during the game has been enhanced, and more obstacles have been 
added to the level. 

**5/04/2020**
Added in new type of Enemy, the Flying Enemy, which uses a jetpack to fly
randomly during gameplay. The Flying Enemy has been added into new obstacles
at the end of the level. Since flying is random, the new enemy is less predictable,
making gameplay more exciting and helps the game feel more complete.

**5/06/2020**
Improved organization and formatting of code. Completed documentation of project,
including code comments, README, and PROPOSAL. Project is now ready for final turn-in.
