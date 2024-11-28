NAME		=	ft_ping

SRCS		=	./main.c			\
				./getDestInfo.c		\
				./sendPacket.c		\
				./ft_ping.c

CC			=	gcc

OBJS		=	$(SRCS:.c=.o)

CFLAGS		+=	-Wall -Wextra -Werror --std=gnu17 -pthread -I ../lib

all			:	$(NAME)

$(NAME)		:	$(OBJS) 
				$(CC) -o $(NAME) $(CFLAGS) $(OBJS) 

dbuild 		:	all
				docker build -t ft_ping .

run			:	all dbuild
				docker run -it ft_ping bash

asan		:	$(OBJS)
				$(CC) -o $(NAME) -fsanitize=address $(CFLAGS) $(OBJS)

clean		:
				rm -rf $(OBJS)

fclean		:	clean
				rm -f $(NAME)

re			:	fclean all
