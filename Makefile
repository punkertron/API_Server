NAME		= server

SRCS_PATH	= ./srcs
SRCS_FILES	= main.cpp

OBJS_PATH	= ./objs
OBJS_FILES	= ${SRCS_FILES:.cpp=.o}
OBJS		= ${addprefix ${OBJS_PATH}/, ${OBJS_FILES}}

INC_DIR		= ./incs
INC			= ${INC_DIR} ./crow
INCLUDES	= ${INC:%=-I %}

DEPS		= ${OBJS:%.o=%.d}

CXX			= g++
CXXFLAGS	= -std=c++17

RM			= rm -rf


all: ${NAME}

${NAME}: ${OBJS}
	${CXX} ${CXXFLAGS} ${INCLUDES} ${OBJS} -o ${NAME}

${OBJS_PATH}/%.o : ${SRCS_PATH}/%.cpp | ${OBJS_PATH}
	${CXX} ${CXXFLAGS} ${INCLUDES} -MMD -MP -c $< -o $@

-include ${DEPS}

${OBJS_PATH}:
	mkdir -p ${OBJS_PATH}

clean:
	${RM} ${OBJS_PATH}

fclean: clean
	${RM} ${NAME}

re: fclean all

format:
	clang-format -i ${SRCS_PATH}/* ${INC_DIR}/*

.PHONY: all clean fclean re format