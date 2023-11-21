NAME		=	ft_irc

SRCS		=	main.cpp server/handleCommands.cpp server/server.cpp

OBJDIR_SRCS		=	obj

OBJS		=	${addprefix ${OBJDIR_SRCS}/,${SRCS:.cpp=.o}}

CC			=	c++
CFLAGS		=	-Wall -Werror -Wextra -std=c++98

RM			=	rm -f

$(OBJDIR_SRCS)/%.o: %.cpp | $(OBJDIR_SRCS)
				${CC} ${CFLAGS} -c $< -o $@

all:			${NAME}

$(OBJDIR_SRCS):
				mkdir -p $(OBJDIR_SRCS)

${NAME}:		${OBJS} Makefile
				${CC} ${CFLAGS} -o ${NAME} ${OBJS}
clean:
				rm -rf $(OBJS) $(OBJDIR_SRCS)
fclean:			clean
				${RM} ${NAME}
re:				fclean all

.PHONY:			all clean fclean re
