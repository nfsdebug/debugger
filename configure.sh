cd ext
git clone https://github.com/dssgabriel/vec
cd vec
make
make test
cd ../../
mkdir target
make
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.ext/vec
