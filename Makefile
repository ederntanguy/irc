NAME		=	ft_irc

SRCS		=	main.cpp 

SERVER		= 	handleCommands.cpp server.cpp

USER		=	user.cpp

OBJDIR_SRCS		=	obj

OBJS		=	${addprefix ${OBJDIR_SRCS}/,${SRCS:.cpp=.o}} \
				${addprefix ${OBJDIR_SRCS}/,${USER:.cpp=.o}} \
				${addprefix ${OBJDIR_SRCS}/,${SERVER:.cpp=.o}}

CC			=	c++
CFLAGS		=	-Wall -Werror -Wextra -std=c++98

RM			=	rm -f

$(OBJDIR_SRCS)/%.o: %.cpp | $(OBJDIR_SRCS)
				${CC} ${CFLAGS} -c $< -o $@

$(OBJDIR_SRCS)/%.o: server/%.cpp | $(OBJDIR_SRCS)
				${CC} ${CFLAGS} -c $< -o $@

$(OBJDIR_SRCS)/%.o: user/%.cpp | $(OBJDIR_SRCS)
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
