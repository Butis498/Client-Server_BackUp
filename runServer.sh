rm -r Server/UpdatedServerFolder
mkdir Server/UpdatedServerFolder
gcc Server/server.c -o Server/server
cd Server/
./server $1
rm server
