echo
echo "Building with jai"

time ~/Downloads/jai/bin/jai-linux -exe blurPNG_jai -release blurPNG.jai


echo
echo "Building with gcc"

time gcc -O3 -o blurPNG_c blurPNG.c -lm


echo
echo "Building with clang"
time clang -O3 -o blurPNG_clang blurPNG.c -lm



echo
echo "Building with odin"

time ~/Downloads/odin/odin-linux-amd64-dev-2025-02/odin build . -out=./blurPNG_odin -o:speed -no-bounds-check
