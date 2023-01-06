
tar xzf Crypt-Rijndael-0.05.tar.gz

cd Crypt-Rijndael-0.05

perl Makefile.PL

make

cd ..

mkdir auto
mkdir auto/Crypt
mkdir auto/Crypt/Rijndael

cp Crypt-Rijndael-0.05/blib/arch/auto/Crypt/Rijndael/Rijndael.so auto/Crypt/Rijndael

cp Crypt-Rijndael-0.05/blib/arch/auto/Crypt/Rijndael/Rijndael.bs auto/Crypt/Rijndael

cp Crypt-Rijndael-0.05/blib/lib/Crypt/Rijndael.pm Crypt


rm -rf Crypt-Rijndael-0.05