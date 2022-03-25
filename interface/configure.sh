cd ext
git clone https://github.com/dssgabriel/vec
cd vec
make
make test
cd ../../
wget https://ftp.gnu.org/pub/gnu/ncurses/ncurses-6.3.tar.gz
tar -xvzf  ncurses-6.3.tar.gz
cd ncurses-6.3/
./configure --with-pthread --with-shared --with-normal --with-debug
make all
sudo make install
cd ../
mkdir target
make 
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.ext/vec
