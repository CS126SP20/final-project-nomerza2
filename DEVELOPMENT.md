# Development

---
**4/18/2020**
Basic Box2D world and object creation and drawing established.

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