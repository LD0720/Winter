# End Year Game

### Game description:
The player (name : Winter) is supposed to collect 5 coins to pass the first level and then when he scores 100 and touches the door he wins and within a certain time , if time finished before the target is reached or when the time ends and target isn’t reached then he loses. 

#### Main character model(s):
- Level 1 : Halloween person
- Level 2 : Santa Claus
#### First environment models (obstacles, collectibles, target): All models are textured
- obstacles : lamp post 
- collectables : candy 
- target : score = 50 and reaching the 1st door
#### Second environment models (obstacles, collectibles, target): All models are textured
- obstacles :  tree 
- collectables : carrot 
- target : score = 100 and colliding with a present

#### A first person shooter point of view will be implemented. The camera is the player’s eye.

#### A third person shooter point of view will be implemented. The camera is behind and slightly above the player (the upper part of the player is visible to the camera).

#### Navigation through the game is done using the keyboard and the mouse:

- The keyboard keys are used to move the player and alternate between camera views.
- The mouse buttons will make the player jump.

#### The score is displayed on the screen.
#### Generation of animations with every user interaction will be implemented:
- There is a sound effect or transformations are applied (state what transformations will be used)
- when colliding with an obstacle it will rotate and disappear
- when colliding with a collectable it will stop rotating around itself and disappear
- when reaching a target a confetti will open and the new level will start or you won will appear (based on level ) , if time ends you lost will appear 
- collectables rotate around itself all the time.
#### There is a light source that changes its color intensity and a light source moving in the scene:
- the sun (daylight) is the light source and its intensity changes (as the sun rotates ) from white to darker intensity representing the night.
- light intensity is dark (representing night in Halloween) 
- light intensity is (white representing day in Christmas)
- the light source is rotating around the scene to generate a sense of a day and night ( sunset and sunrise ) 
