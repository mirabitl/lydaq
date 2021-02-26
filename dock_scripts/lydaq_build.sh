#!/bin/bash
docker build -t lydaq:1 -f- https://github.com/mirabitl/zdaq.git <<EOF
FROM zdaq:1
RUN set -eux; \
    apt update; \
    apt -y install wget;\
    wget https://raw.githubusercontent.com/mirabitl/lydaq/master/docker_lydaq_install.sh;\
    chmod +x docker_lydaq_install.sh;\
    ./docker_lydaq_install.sh;
RUN  set -eux;\ 
     echo 'PARTITION=CMSRPC\n\
WEBLOGIN=cmsLyon:RPC_2008\n\
LD_LIBRARY_PATH=/usr/lib:/usr/local/lib:$LD_LIBRARY_PATH\n\
CONFDB_WEB=cmsLyon/RPC_2008@ilcconfdb.ipnl.in2p3.fr\n\
MGDBLOGIN=acqilc/RPC_2008@lyocmsmu04:27017@LYONROC\n'\	
>> /etc/ljc.conf;
EOF
