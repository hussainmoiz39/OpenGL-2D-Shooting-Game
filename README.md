Instructions for compiling and running

    To compile, open your terminal.
    cd OGL3Sample2D/GLFW
    Type make
    ./sample2D

Abstract

A laser-cannon game where the objective is to score as many points as possible by destroying the black bricks before they reach the baskets on the floor. Destroying black bricks gives you points and collecting the red and green bricks in their respective baskets gives you points too. If you successfully score more than 100 points, you win the game.
Controls:
Mouse:
    Right click and drag to move the baskets.
    Left click anywhere on the screen to get the cannon ready, and release to launch a laser.

Keyboaard Controls:
    'N' to increase the speed of the bricks
    'M' to decrease the speed of the bricks
    'SPACE' to launch the laser
    'S' to move the cannon upwards
    'F' to move the cannon downwards
    'A' to increase the launch angle
    'B' to decrease the launch angle

About the game:
    Destroying a black brick gives you 3 points.
    Collecting red and green bricks in their respective baskets would give you 2 points.
    Collecting wrong bricks will decrease your score by -1.
    If a black brick is collected in any of the buckets, you lose the game.
    Maximise your score. Goodluck :)
Features:

    No images used anywhere in the game. Everything is create by using shapes in openGL. This ensures the loading of the game is quick and efficient.
    Rendered numbers without the help of any libraries (only using shapes).
    Mouse drag for firing a laser.
    Used laws or reflection for collision of laser with the mirror.

Note:

All objects are sorted into different layers. Each layer is drawn one at a time. Some layers are more prefered and will be drawn last whereas others will be drawn earlier (Like the background layer). Within a layer objects are drawn in a lexicographical order. These two together give you the ability to draw complex objects with ease.
Dependencies:
Linux/Windows/ Mac OSX - Dependencies: (Recommended)

    GLFW
    GLAD
    GLM

Linux - Dependencies: (alternative)

    FreeGLUT
    GLEW
    GLM
