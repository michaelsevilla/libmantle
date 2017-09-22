from ubuntu:trusty

ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update -y && \
    apt-get install -y vim cmake make libopenmpi-dev g++ openmpi-bin
RUN apt-get install -y openssh-server

ADD .ssh /root/.ssh
RUN mkdir -p /var/run/sshd && \
    sed -i "s/UsePrivilegeSeparation.*/UsePrivilegeSeparation no/g" /etc/ssh/sshd_config && \
    sed -i "s/UsePAM.*/UsePAM no/g" /etc/ssh/sshd_config && \
    sed -i "s/PermitRootLogin.*/PermitRootLogin yes/g" /etc/ssh/sshd_config && \
    mkdir -p /root/.ssh && \
    echo 'Host *' > /root/.ssh/config && \
    echo 'StrictHostKeyChecking no' >> /root/.ssh/config && \
    echo 'LogLevel quiet' >> /root/.ssh/config && \
    chmod 600 /root/.ssh/config /root/.ssh/id_rsa && \
    wget --no-check-certificate https://raw.githubusercontent.com/ivotron/docker-openssh/d1db8b1c29b532896b8a91009820f732dcae6b79/entrypoint.sh && \
    mv entrypoint.sh /root/.ssh/ && chmod 750 /root/.ssh/entrypoint.sh && \
    wget --no-check-certificate https://raw.githubusercontent.com/ivotron/docker-openssh/d1db8b1c29b532896b8a91009820f732dcae6b79/stopsshd && \
    mv stopsshd /usr/bin/ && chmod 750 /usr/bin/stopsshd

ENV COMPILER     "mpic++"

ENTRYPOINT ["/bin/bash"]
