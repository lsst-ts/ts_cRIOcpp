# OSPL Setup

When running OSPL on a machine where multiple users can login, ospl shall be setup so those can access it. Systemctl ospl.service file is provided in etc/ospl.service.

## Create user and group

As root, execute:

```bash
adduser -Ur ospl
usermod -a -G ospl <users>
```

replace <users> with users that shall be able to access ospl.

## systemd configuration

Copy etc/ospl.service into /etc/systemd/system/ and run it:

```bash
cp etc/ospl.service /etc/systemd/system/
systemctl daemon-reload
systemctl start ospl
```

## Verify it is running:

As user, run:

```
newgrp ospl
```

or logout/login/reboot the machine. Then in environment with correct OSPL setup:

```bash
export OSPL_HOME=/opt/OpenSpliceDDS/V6.10.4/HDE/x86_64.linux
export PATH=$OSPL_HOME/bin:$PATH
export OSPL_URI=file:///usr/lib/python3.10/site-packages/lsst/ts/ddsconfig/data/config/ospl-shmem-debug-no-network.xml
export LD_LIBRARY_PATH=$OSPL_HOME/lib:$LD_LIBRARY_PATH
```

runs ospl status, which shall reply with "..is operational"


```bash
$ ospl status
Vortex OpenSplice System with domain name "ospl_sp_ddsi" is operational
```

## Troubleshooting

If the above doesn't work, make sure ospl services runs:

```bash
$ ps -Af|grep ospl
ospl      101581       1  0 17:06 ?        00:00:01 spliced file:///usr/lib/python3.10/site-packages/lsst/ts/ddsconfig/data/config/ospl-shmem-debug-no-network.xml
ospl      101595  101581  0 17:06 ?        00:00:00 durability durability file:///usr/lib/python3.10/site-packages/lsst/ts/ddsconfig/data/config/ospl-shmem-debug-no-network.xml
root      101828   99977  0 17:09 pts/5    00:00:00 grep --color=auto ospl
```

and make sure /tmp/spddskey_* can be read:

```bash
$ cat /tmp/spddskey_*
ospl_sp_ddsi
140000000
1e179000
SVR4-IPCSHM
101581
0
101581
1
2
```

systemd logs are usually stored in /var/log/messages, so that might help as well.
