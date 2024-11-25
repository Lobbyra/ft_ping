FROM debian

WORKDIR /root

RUN apt update && apt install -y inetutils-ping strace

ADD ft_ping .
