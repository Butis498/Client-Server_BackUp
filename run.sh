mkdir MonitoredFolder

gcc Client/service.c -o serviceBackUp
./serviceBackUp
rm serviceBackUp
