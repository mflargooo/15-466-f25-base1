# Operation Temperature Control

Author: Matthew Largo

Design: A Wii Play Tanks!-inspired bullet hell, where you must manage your actions to avoid overheating!

Screen Shot:

![Screen Shot](screenshot.png)

How Your Asset Pipeline Works:

Index spritesheets > Python script > 4096 byte fixed tile table > PPU466

Spritesheets ([link](assets/tank.png), [link]assets/explosion.png) were generated in color mode "Indexed" using Aseprite and passed as arguments
to the python script [link](tools/indexed_to_ppu.py) (as python tools/indexed_to_ppu.py assets/tank.png assets/explosion.png) to obtain a 4096 byte fixed tile table.

The python script [link](tools/indexed_to_ppu.py) require spritesheets to have dimensions divisible by 8, from which they are broken up into 8x8 chunks. The script populates the tile table with tiles from the spritesheets in the order of they were provided; within each spritesheet, tiles are indexed in left-to-right, top-to-bottom order. The script maintains the convention that index 0 is a color with alpha value 0, and thus ignores 8x8 empty tiles whose pixels are all colored with index 0. All nonempty tiles are populated contiguously in the tile table. 

The user can provide as many spritesheets to [link](tools/indexed_to_ppu.py) as they desire, so long as the sum of the number of nonempty is less than or equal to 256

Controls:

Arrow Up - Move Up
Arrow Left - Move Left
Arrow Right - Move Right
Arrow Down - Move Down
Space - Shoot
Escape - Quit

How To Play:

Move around to dodge the incoming fire and survive as long as you can! If you are skilled enough, you can launch a projectile of your own to collide with and destroy incoming fire. BUT BEWARE! The more actions you take, the more strain you put on your engine and the hotter it becomes. 

Allow for some breaks in between your actions, and manage them carefully, to prevent your engine from becoming too hot. Build up too much heat and strain and your engine will overheat and you must wait for it to cool down completely.

Keep track of your heat by taking note of the objects' in the scene. The redder the scene is, the hotter your engine; the greener it is, the cooler you are. If the scene is blue, you have overheated and you must sit idle for 5 seconds to cool down completely (to full green).

If you run the game from the terminal, your score (the time you survive in seconds) is displayed upon quitting (with the Escape key).

This game was built with [NEST](NEST.md).

