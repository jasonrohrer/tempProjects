time ~/Downloads/jai/bin/jai-linux -release blurPNG.jai


time gcc -O3 -o blurPNG_c blurPNG.c -lm


time clang -O3 -o blurPNG_clang blurPNG.c -lm



time ~/Downloads/odin/odin-linux-amd64-dev-2025-02/odin build . -out=./blurPNG_odin -o:speed -no-bounds-check
