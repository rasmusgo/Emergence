Emergence
=========

Small multiplayer sandbox game with growing grass and cute rabbits

Uses SDL and OpenGL for graphics.

License is not determined yet so I guess it is "all rights reserved" until a suitable license is chosen.

Installing and running
------------

    # Build
    sudo apt-get install libsdl-dev libsdl-net1.2 libsdl-net1.2-dev libsdl-image1.2 libsdl-image1.2-dev
    mkdir build
    cd build
    cmake ..
    make
    # Run server
    cd ../server
    ./emergence_server &
    # Run client
    cd ../client
    ../emergence &

