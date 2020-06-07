## Execute Backup Service ##

# Give permission to run file #

chmod +x run.sh

# run executable #

./run.sh

# TEST FUNCTIONALITY #

open a linix terminal and type "grep firstdaemon /var/log/syslog"
this will open the log of the events in the service.

# Kill the service #

open a linix terminal and type "grep firstdaemon /var/log/syslog"
this will return something like "firstdaemon[5852]" with other data, you should type in the terminal "Kill -11 PID" or in my case "Kill -11 5852"  the PID(process id will) not always be the same.