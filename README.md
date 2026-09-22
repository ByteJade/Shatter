# Shatter

(WIP) Linux userspace x64 -> ARM dynarec  
Goal: Make a simple and fast x64 dynamic recompiler on c++  
## How to build

git clone https://github.com/ByteJade/Shatter
cd ByteShatter && make

## Use example

cd build
./shatter ../tests/glxgears

## Dependencies

Arch linux for example:

sudo pacman -S readline sdl2 sdl2_image libx11 mesa