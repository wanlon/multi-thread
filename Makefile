TARGET = main
CC = gcc
TOP_DIR := $(PWD)
ROOT_SUB_DIR := $(TOP_DIR)/threadpool
SRC_DIR := $(ROOT_SUB_DIR)/src
BIN_DIR := $(ROOT_SUB_DIR)/bin
SUB_DIR := $(SRC_DIR) \
	$(BIN_DIR)
OBJ := $(BIN_DIR)/main.o $(SRC_DIR)/wThread.o
export CC SRC INCLUDE OBJ TOP_DIR SRC_DIR BIN_DIR BIN

all:$(SUB_DIR) $(TARGET)
$(TARGET) : $(OBJ)
	$(CC) $^ -lpthread -o $@

$(SUB_DIR):ECHO
	make -C $@
ECHO:
	@echo $(SUB_DIR)
	@echo begin compile

clean:
	rm -rf $(SRC_DIR)/*.o $(BIN_DIR)/*.o

