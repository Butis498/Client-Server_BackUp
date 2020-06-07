mkdir MonitoredFolder

gcc Client/service.c -o serviceBackUp -pthread
./serviceBackUp
rm serviceBackUp
