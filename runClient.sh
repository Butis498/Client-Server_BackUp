mkdir MonitoredClientFolder
gcc Client/service.c -o serviceBackUp -pthread
./serviceBackUp $1 $2
rm serviceBackUp
