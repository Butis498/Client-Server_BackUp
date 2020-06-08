mkdir MonitoredClientFolder
mkdir Server/UpdatedServerFolder
gcc Client/service.c -o serviceBackUp -pthread
gcc Server/server.c -o Server/server
./serviceBackUp
rm serviceBackUp
