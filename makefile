CC = gcc

CFLAGS = -g -O0 -fsanitize=address
NCURSESFLAGS = -lncurses
POSTGRESFLAGS = -I/usr/include/postgresql -lpq
CJSON = -lcjson
MATH = -lm

OBJ_DIR = objects
OBJECTS = $(OBJ_DIR)/main.o $(OBJ_DIR)/server.o $(OBJ_DIR)/database.o $(OBJ_DIR)/sensors.o

MAIN_INCLUDE = include/main
SERVER_INCLUDE = include/server
DATABASE_INCLUDE = include/database
GLOBAL_INCLUDE = include/globalStructures

INCLUDE = -I$(GLOBAL_INCLUDE) -I$(MAIN_INCLUDE) -I$(SERVER_INCLUDE) -I$(DATABASE_INCLUDE)

MAIN = src/main
SERVER = src/server
DATABASE = src/database

irrigatorServer: $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(POSTGRESFLAGS) $(NCURSESFLAGS) $(CJSON) $(MATH)

$(OBJ_DIR)/main.o: $(MAIN)/main.c
	$(CC) $(CFLAGS) $^ -c $(INCLUDE) -o $@

$(OBJ_DIR)/server.o: $(SERVER)/server.c
	$(CC) $(CFLAGS) $^ -c $(INCLUDE) -o $@

$(OBJ_DIR)/database.o: $(DATABASE)/database.c
	$(CC) $(CFLAGS) $^ -c $(INCLUDE) -o $@

$(OBJ_DIR)/sensors.o: $(DATABASE)/sensors.c
	$(CC) $($CFLAGS) $^ -c $(INCLUDE) -o $@
