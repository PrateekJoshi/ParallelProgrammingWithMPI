# Compile : make
# Run : make run

CFLAGS+=-lmpi
MPI_EXEC:=mpiexec
PROCESS:=4
TARGET:=parellel_dot_mpi_all_reduce.o

all : $(TARGET)


%.o : %.c
	gcc $< $(CFLAGS) -o $@ -g 
	
run:
	$(MPI_EXEC) -np $(PROCESS) $(TARGET)

clean:
	rm -rf ./$(TARGET)