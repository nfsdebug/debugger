wget https://ftp.gnu.org/pub/gnu/ncurses/ncurses-6.3.tar.gz
tar -xvzf
cd ncurses-6.3.tar.gz
./configure --with-pthread --with-shared --with-normal --with-debug
make all
sudo install