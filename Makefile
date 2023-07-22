NAME		= server

SRCS_PATH	= ./srcs
SRCS_FILES	= main.cpp HashPasswordToken.cpp ApiRequests.cpp

OBJS_PATH	= ./objs
OBJS_FILES	= ${SRCS_FILES:.cpp=.o}
OBJS		= ${addprefix ${OBJS_PATH}/, ${OBJS_FILES}}

INC_DIR		= ./incs
INC			= ${INC_DIR} ./crow
INCLUDES	= ${INC:%=-I %}

LDLIBS		= -lcryptopp -lpqxx -lpq

DEPS		= ${OBJS:%.o=%.d}

CXX			= g++
CXXFLAGS	= -std=c++17 -Wall -Wextra -O2

RM			= rm -rf

# Where we store files
FILES_PATH	= ./files

all: ${NAME}

${NAME}: ${OBJS}
	${CXX} ${CXXFLAGS} ${INCLUDES} ${OBJS} ${LDLIBS} -o ${NAME}

${OBJS_PATH}/%.o : ${SRCS_PATH}/%.cpp | ${OBJS_PATH} ${FILES_PATH}
	${CXX} ${CXXFLAGS} ${INCLUDES} -MMD -MP -c $< -o $@

-include ${DEPS}

${OBJS_PATH}:
	mkdir -p ${OBJS_PATH}

${FILES_PATH}:
	mkdir -p ${FILES_PATH}

clean:
	${RM} ${OBJS_PATH}

fclean: clean
	${RM} ${NAME}

re: fclean all

delete_files:
	${RM} ${FILES_PATH}

format:
	clang-format -i ${SRCS_PATH}/* ${INC_DIR}/*

.PHONY: all clean fclean re delete_files format