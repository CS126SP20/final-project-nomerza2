# Robot Revolt
(Formerly Called Two-Dimensional Traveler)

Author: Nathan Omerza

---

I want to build a 2D platformer game. The game will be single player, and will require the player to navigate through
obstacles and fight enemies to reach the end. The player will be able to move side to side, jump, and shoot. The goal 
will be for the player to move to the end of the map without dying. I want to do this project because, quite simply, I
enjoy playing games. 2D platformer format is similar to a lot of simple games I played online as a kid, so I would like
to make something similar to this. My motivation for trying to learn how to do this and to complete the project is that 
I want to create a game that I would enjoy playing.

In order to set up the environment with moving objects, I am going to use Box2D (www.box2d.org), a 2-dimensional physics
simulator. This will allow me to handle collisions between the player, enemies, and walls, floors, ceilings, or other
obstacles. There are examples available online that integrate Box2D with Cinder, so it should be straightforward to
handle the connection between these two libraries.

My plan for the first week is to implement basic movement. The user should be able to move, jump, and not walk through
walls or fall through floors. I will also try to begin using the user interface immediately. The user should be printed
out to the screen, and show its movement. I will implement the user interface to accomplish this, so I can test if using 
the keyboard to control the player works. The main goal of this week is to become familiar with Box2D.
The second week is for adding the ability to shoot and creating enemies that will shoot back. This should also allow for
handling of bullet interaction with environment objects (e.g. disappearing when it hits a wall). The final week will
focus on organizing the different elements into a complete level. This will require the creation of obstacles, adding
enemies to different points, and adding a system for losing lives and winning the game. If I have extra time, I will
continue to expand the project by adding different elements to the level, such as using Box2D to find new obstacles or
challenges to the map. These could include moving hazards or moving walkways, to add a more complete challenge to the
game. I could also add new types of enemies that have unique behaviors. (Edit from after development) The final project
also included music and sound effects, as well as a title screen. I did create a new enemy, but did not add moving 
hazards. Additions from extra time also include adding in more elements and challenges to the game, making the level
longer.