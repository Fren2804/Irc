CPP			= c++
CPPFLAGS	= -Wall -Wextra -Werror -std=c++98 -Iincludes
NAME		= IRC
SRC			= ./src/
OBJS_DIR	= ./objs/
FILES		= \
				$(SRC)Channel.cpp \
				$(SRC)Client.cpp \
				$(SRC)Parser.cpp \
				$(SRC)Server.cpp \
				$(SRC)Utils.cpp \
				$(SRC)Irc.cpp


OBJS = $(FILES:$(SRC)%.cpp=$(OBJS_DIR)%.o)

$(OBJS_DIR)%.o: $(SRC)%.cpp
	@echo "🔧Compiling $< 🔨"
	@$(CPP) $(CPPFLAGS) -c $< -o $@

all: mkdir_objs show_name $(NAME)

mkdir_objs:
	@mkdir -p $(OBJS_DIR)

$(NAME): $(OBJS)
	@$(CPP) $(CPPFLAGS) $(OBJS) -o $(NAME)
	@echo "$(GREEN)Created $(NAME) ✓$(RESET)"

clean:
	@rm -rf $(OBJS)
	@echo "$(RED)Objects IRC Removed ✓$(RESET)"

fclean: clean 
	@rm -f $(NAME)
	@echo "$(RED)$(NAME) Removed ✓$(RESET)"

re: fclean all


GREEN = \033[0;32m
YELLOW = \033[1;33m
RED = \033[0;31m
BLUE = \033[0;34m
RESET = \033[0m

show_name:
	@echo "$(BLUE)==================================================$(RESET)"
	@echo "$(YELLOW)[........[.......          [.       [...     [.."
	@echo "[..      [..    [..       [. ..     [. [..   [.."
	@echo "[..      [..    [..      [.  [..    [.. [..  [.."
	@echo "[......  [. [..         [..   [..   [..  [.. [.."
	@echo "[..      [..  [..      [...... [..  [..   [. [.."
	@echo "[..      [..    [..   [..       [.. [..    [. .."
	@echo "[..      [..      [..[..         [..[..      [..$(RESET)"
	@echo "$(BLUE)==================================================$(RESET)"
	@echo "$(GREEN)Compiling IRC...$(RESET)"

.PHONY: all clean fclean re show_name mkdir_objs