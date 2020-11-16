# Compile : make
# Run : make run
PROCESS:=1
TARGET:=integral_seq.o

all : $(TARGET)


%.o : %.c
	gcc $< -o $@ -g 
	
run:
	./$(TARGET)

clean:
	rm -rf ./$(TARGET)