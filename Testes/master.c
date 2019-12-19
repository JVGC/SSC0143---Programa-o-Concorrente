//compilar: mpicc master.c -o master
//executar: mpirun master -np 1 master

#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include"mpi.h"

#define N 3

int main(int argc, char **argv)  {
    int  tag = 1, my_rank, num_proc, src, dst, root;
    char msg_0[] = "hello slave, i'm your master";
    char msg_1[50], master_data[] = "slaves to work", slave[20];
    int  errcodes[10], i, vet[N], buf_rcv, vet_master[N];
    MPI_Status  status;
    MPI_Comm    inter_comm;
    MPI_Request mpirequest_mr;

	strcpy(slave,"slave");
	
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);

	for (i = 0; i < N; i++)
		vet[ i ] = i;

	src = dst = root = 0;
	MPI_Comm_spawn(slave, MPI_ARGV_NULL, N, MPI_INFO_NULL, root, MPI_COMM_WORLD, &inter_comm, errcodes);
	
	MPI_Send(msg_0, 50, MPI_CHAR, dst, tag, inter_comm);
	MPI_Irecv(msg_1, 50, MPI_CHAR, src, tag, inter_comm, &mpirequest_mr);
	
	MPI_Send(master_data, 50, MPI_CHAR, dst, tag, inter_comm);
	MPI_Scatter(vet, 1, MPI_INT, &buf_rcv, 1, MPI_INT, MPI_ROOT, inter_comm);
	MPI_Gather(vet, 1, MPI_INT, &vet_master, 1, MPI_INT, MPI_ROOT, inter_comm);

	MPI_Wait(&mpirequest_mr, &status);
	
	for (i = 0; i < N; i++) 
		printf("msg_1=%s,vet[%d]=%d,buf_rcv=%d,vet_master[%d]=%d\n",msg_1,i,vet[i],buf_rcv,i,vet_master[i]);

    MPI_Finalize();
	return 0;
}