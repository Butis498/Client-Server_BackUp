# Execute Backup Service #

## Give permission to run file ##
```console
chmod +x run.sh
```
## Run executable ##
```console
./run.sh
```
This will create a folder named 'MonitoredFolder' which will be monitored by the service. The service also will start
## TEST FUNCTIONALITY ##

open a linux terminal and type 
```console
grep firstdaemon /var/log/syslog
```
this will open the log of the events in the service.

### Kill the service ###

Open a linux terminal and type 
```console
grep firstdaemon /var/log/syslog
```
this will return something like :
```console
firstdaemon[5852]
```
with other data, you should type in the terminal 
```console 
Kill -11 PID
```
or in my case "Kill -11 5852"  the PID(process id will) not always be the same.
