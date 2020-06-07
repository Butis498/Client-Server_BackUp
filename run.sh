mkdir MonitoredFolder

gcc service.c -o serviceBackUp
./serviceBackUp
rm serviceBackUp
