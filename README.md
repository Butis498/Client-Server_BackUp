# Execute Backup Service #

## Give permission to run file ##
```console
chmod +x runClient.sh
chmod +x runServer.sh
```
## Run server executable ##
```console
./runServer.sh 8080
```
This willcreate a folder Service/UpdatedServerFolder/ that will hold the backup of the client directory. It also start running the server for the UpdaterServerFolder updates.The 8080 argument represents the port to be used by the server and can be changed depending on user needs. Don't close this terminal. Open a new terminal.

## Run client executable ##
```console
./runClient.sh 8080 [THE SERVER IP]
```
This will create a folder named 'MonitoredFolder' which will be monitored by the client service. The client monitoring service also will start. This service can be found with the name "serviceBackUp". The 8080 argument represents the port to be used to call the server. You must replace the [THE SERVER IP] argument with the actual server IP. (example: 127.0.0.1 if you are running the server on the same computer as the client)


## TEST FUNCTIONALITY ##

open a linux terminal and type 
```console
grep serviceBackUp /var/log/syslog

or

grep firstdaemon /var/log/syslog
```
this will open the log of the events in the service. (use grep serviceBackUp preferently)

### Kill the service ###

Open a linux terminal and type 
```console
grep serviceBackUp /var/log/syslog
```
this will return something like :
```console
serviceBackUp[5852]
```
with other data, you should type in the terminal 
```console 
Kill -11 PID
```
or in my case "Kill -11 5852"  the PID(process id will) not always be the same.
