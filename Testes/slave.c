//compilar: mpicc slave.c -o slave
//executar: a partir do MPI_Comm_spawn do master

// codigo fonte do slave.c; nome do executavel: slave
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"mpi.h"
#define N 3
int main(int argc, char **argv)  {
    int  tag = 1, my_rank, num_proc;
    char msg_0[50], msg_1[]="hi master", master_data[50], slaves_data[50];
	int  i, vet[N], buf_rcv, src, dst, root;
    MPI_Status  status;
    MPI_Comm    inter_comm; 
	
	MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);

    MPI_Comm_get_parent(&inter_comm);

	src = dst = root =0;
	if ( my_rank == 0 )	{
		MPI_Recv(msg_0, 50, MPI_CHAR, src, tag, inter_comm, &status);
	    MPI_Send(msg_1, 50, MPI_CHAR, dst, tag, inter_comm);

	    MPI_Recv(master_data, 50, MPI_CHAR, src, tag, inter_comm, &status);
	    strcpy(slaves_data, master_data);
    }
	MPI_Bcast(slaves_data, 50, MPI_CHAR, root, MPI_COMM_WORLD);
	MPI_Scatter(vet, 1, MPI_INT, &buf_rcv, 1, MPI_INT, root, inter_comm);

	for (i = 0; i < N; i++) 
		printf("SLV=%d:slaves_data=%s,vet[%d]=%d,buf_rcv=%d\n", my_rank,slaves_data,i,vet[i],buf_rcv);

	MPI_Gather(&buf_rcv, 1, MPI_INT, vet, 1, MPI_INT, root, inter_comm);
	MPI_Finalize();
	return 0;
}