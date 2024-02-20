BUILD_PATH = build
APP_NAME = rest-client
INCLUDE_DIRS = libs/json-parser include
INCLUDES = $(foreach d, $(INCLUDE_DIRS), -I $d)
export LD_LIBRARY_PATH=/usr/lib/x86_64-linux-gnu
LIBRARIES_PATH=$(PWD)/$(BUILD_PATH)
CFLAGS = -g $(INCLUDES)
LD_FLAGS = -g -static -ljsonparser -L $(LIBRARIES_PATH) -L $(LD_LIBRARY_PATH) -lm 
SRC_DIR = src
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_PATH)/%.o)
CC = gcc


all: clean rest-client

build-folder:
	mkdir build || echo build folder already exists

build: libs rest-client

rest-client-binary: $(OBJS)
	$(CC) -o $(BUILD_PATH)/$(APP_NAME) $^ $(LD_FLAGS) 

rest-client: libs rest-client-binary

libs: json-parser	

json-parser: build-folder
	cd libs/json-parser; make all; cp libjsonparser.a ../../build

run: rest-client
	$(BUILD_PATH)/$(APP_NAME)

experiments: test_json

test_json:
	gcc -o $(BUILD_PATH)/test_json experiments/test_json.c $(LD_FLAGS) 

run_test_json:
	$(BUILD_PATH)/test_json experiments/resources/response_json.txt


$(BUILD_PATH)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $^ -o $@


clean:
	rm -rf build	

rebuild: clean build	