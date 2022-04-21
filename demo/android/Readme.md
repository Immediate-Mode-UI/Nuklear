# Nuklear Android demos

## Building for SDL beginners

This folder contains several **Nuclear** implementations for **Android**.
Has an Android dev you must be familiar whit JNI, SDL and C/C++ technologies.
**SDL2** source code is mandatory in order to build the demos, yo can get a copy at: https://github.com/libsdl-org/SDL, clone it wherever you want, just don’t forget set the environment variable  `export SDLPATH=/wherever/you/put/SDL`

You may have already settled yours environment vars for build other Android JNI projects `ANDROID_HOME` and `ANDROID_NDK_HOME`, if yours environments vars are different its opt to you add a new ones thats matches whit our build scripts or edit our demo build.sh in order to get the right paths.   

**Structure folder demos:**
`sdl_opengl2`:
This folder contains all the dependencies for build a project based on **OpenGL ES2**, to build the demo just execute ./build.sh, if everything goes ok you will see you demo at folder `build` named as ***com.demogles2.nuklear*** .

`sdl2surface_rawfb`:
This folder contains all the dependencies for build a project based on **Raw SDL Surface**,  OpenGL context will not be accessible inside this project, anyways under the hood SDL works with OpenGL, just like the previous demo you will found the project builded in folder `build` named as ***com.sdl2rawfb.nuklear*** .

`build`:
Has we have mentioned above, if builds were successfully, this folder will contains the demos ready to run on an Android device.        

 ## Building for SDL experts
 If you already are familiar whit SDL you con go strike forward to https://github.com/libsdl-org/SDL under the folder `build-scripts` you will see the script **androidbuild.sh**, this script will lead you to create a projects whit more flexibility.
