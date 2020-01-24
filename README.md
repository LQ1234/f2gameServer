# f2gameServer

## development on windows:
Download and open in visual studio

## production on linux 
```
git clone https://github.com/LQ1234/f2gameServer
cd f2gameServer
cd f2gameServer

g++ f2gameServer.cpp --> websocketpp/config/asio_no_tls.hpp: No such file or directory
```
### --install websocketpp--
```
sudo apt-get install libasio-dev

cd ../../

mkdir libinstall
cd libinstall
```
#### From: https://airdcpp-web.github.io/docs/installation/websocketpp.html
```
git clone git://github.com/zaphoyd/websocketpp.git
cd websocketpp
sudo apt install cmake
cmake .
sudo make install
cd ../ #We are in libinstall now

g++ f2gameServer.cpp --> Box2D/Box2D.h: No such file or directory
```
### --install liquidfun--
```
sudo apt-get install libglapi-mesa
sudo apt-get install libglu1-mesa-dev

git clone https://github.com/google/liquidfun
cd liquidfun/liquidfun/Box2D
```
#### fix some some errors:
```
nano CMakeLists.txt
```
#### insert “find_package(Threads)” right after “project(Box2D)”
#### change “set(C_FLAGS_WARNINGS "-Wall -Werror -Wno-long-long -Wno-variadic-macros")” to “set(C_FLAGS_WARNINGS "-Wall -Wno-long-long -Wno-variadic-macros")” (remove -Werror)
```
cmake -G'Unix Makefiles'
```
#### run
```
make
```
#### and if it says fatal error: X11/extensions/XInput.h: No such file or directory, run  
```
cd /usr/include/X11/extensions
sudo ln -s XI.h XInput.h
cd [Back into ..../libinstall/liquidfun/liquidfun/Box2D]
```
#### and make again

#### From: https://gist.github.com/WillSams/4b0a2ac1906761a74b6775eb367f7afa
```
cmake -DBOX2D_INSTALL=ON \
    -DBOX2D_INSTALL_DOC=ON \
    -DBOX2D_BUILD_SHARED=ON \
    -DBOX2D_BUILD_STATIC=ON \
    -DBOX2D_BUILD_EXAMPLES=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_VERBOSE_MAKEFILE=ON \

make
make preinstall
sudo cmake -DCMAKE_INSTALL_PREFIX=/usr/local -P cmake_install.cmake
```
#### look in /usr/local/lib and there should be a libliquidfun.a
```
cd ../../../../f2gameServer/f2gameServer
```
### --compile--
```
g++ *.cpp -o f2gameServer /usr/local/lib/libliquidfun.a  -lpthread
```
### --run--
```
./f2gameServer
```
