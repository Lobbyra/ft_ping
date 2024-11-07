FROM debian

WORKDIR /root

RUN apt update && apt install -y inetutils-ping

ADD ft_ping .
