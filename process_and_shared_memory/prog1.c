#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <math.h>

struct coordinate
{
	int x;
	int y;
};

struct region
{
	int pid;
	struct coordinate result[6];
	struct coordinate buffer[6];
	struct coordinate position;
	int index[6];
	int count;
	int status;
	bool done;
	bool created;
	int mode;
	bool start_test;
};

int i = 1;

void decide_direction(struct region* ptr, struct coordinate target)
{
	if(ptr->result[ptr->count].x == target.x & ptr->result[ptr->count].y < target.y)  //down
		ptr->index[ptr->count] = 1;

	else if(ptr->result[ptr->count].x == target.x && ptr->result[ptr->count].y > target.y)  //up
		ptr->index[ptr->count] = -1;

	else if(ptr->result[ptr->count].x > target.x && ptr->result[ptr->count].y == target.y)  //left
		ptr->index[ptr->count] = 2;

	else if(ptr->result[ptr->count].x < target.x && ptr->result[ptr->count].y == target.y)  //right
		ptr->index[ptr->count]= -2;

	else if(ptr->result[ptr->count].x < target.x && ptr->result[ptr->count].y < target.y)  //down right
		ptr->index[ptr->count] = 3;

	else if(ptr->result[ptr->count].x > target.x && ptr->result[ptr->count].y < target.y)  //down left
		ptr->index[ptr->count] = 4;

	else if(ptr->result[ptr->count].x < target.x && ptr->result[ptr->count].y > target.y)  //up right
		ptr->index[ptr->count] = -4;

	else if(ptr->result[ptr->count].x > target.x && ptr->result[ptr->count].y > target.y)  //up left
		ptr->index[ptr->count] = -3;
}

void decide_coordinate(struct region* ptr)
{
	int range_x = 0, range_y = 0, INDEX = 0;
	if(ptr->mode == 1 && ptr->start_test)
	{
		ptr->position.x = ptr->result[i].x;
		ptr->position.y = ptr->result[i].y;
		INDEX = (-ptr->index[i]);
	}
	else
	{
		ptr->position.x = ptr->result[(ptr->count - 1)].x;
		ptr->position.y = ptr->result[(ptr->count - 1)].y;
		INDEX = ptr->index[(ptr->count - 1)];
	}

	if(INDEX == 1)  //down
	{
		range_y = ptr->position.y + 1;
		if(range_y < 9)
			ptr->position.y = range_y + rand() % (9 - range_y);
		else
			ptr->position.y = 9;
	}
	else if(INDEX == -1)  //up
	{
		range_y = ptr->position.y - 1;
		if(range_y > 0)
			ptr->position.y = range_y - rand() % range_y;
		else
			ptr->position.y = 0;
	}
	else if(INDEX == 2)  //left
	{
		range_x = ptr->position.x - 1;
		if(range_x > 0)
			ptr->position.x = range_x - rand() % range_x;
		else
			ptr->position.x = 0;
	}
	else if(INDEX == -2)  //right
	{
		range_x = ptr->position.x + 1;
		if(range_x < 9)
			ptr->position.x = range_x + rand() % (9 - range_x);
		else
			ptr->position.x = 9;
	}
	else if(INDEX == 3)  //down right (x, y都變大)
	{
		range_x = ptr->position.x + 1;
		range_y = ptr->position.y + 1;
		if(range_x < 9)
			ptr->position.x= range_x + rand() % (9 - range_x);
		else
			ptr->position.x = 9;
		if(range_y < 9)
			ptr->position.y = range_y + rand() % (9 - range_y);
		else
			ptr->position.y = 9;
	}
	else if(INDEX == 4)  //down left (x變小, y變大)
	{
		range_x = ptr->position.x - 1;
		range_y = ptr->position.y + 1;
		if(range_x > 0)
			ptr->position.x = range_x - rand() % range_x;
		else
			ptr->position.x = 0;
		if(range_y < 9)
			ptr->position.y = range_y + rand() % (9- range_y);
		else
			ptr->position.y = 9;
	}
	else if(INDEX == -4)  //up right (x變大, y變小)
	{
		range_x = ptr->position.x + 1;
		range_y = ptr->position.y - 1;
		if(range_x < 9)
			ptr->position.x = range_x + rand() % (9- range_x);
		else
			ptr->position.x = 9;
		if(range_y > 0)
			ptr->position.y = range_y - rand() % range_y;
		else
			ptr->position.y = 0;
	}
	else if(INDEX == -3)  //up left (x, y都變小)
	{
		range_x = ptr->position.x - 1;
		range_y = ptr->position.y - 1;

		if(range_x > 0)
			ptr->position.x = range_x - rand() % range_x;
		else
			ptr->position.x = 0;
		if(range_y > 0)
			ptr->position.y = range_y - rand() % range_y;
		else
			ptr->position.y = 0;
	}

	if(ptr->mode == 1 && ptr->start_test)
	{
		ptr->buffer[i].x = ptr->position.x;
		ptr->buffer[i].y = ptr->position.y;
	}
	else
	{
		ptr->result[ptr->count].x = ptr->position.x;
		ptr->result[ptr->count].y = ptr->position.y;
	}
}

int main(int argc, char *argv[])
{
	const char* NAME = "/sharedmemory";
	const size_t region_size = sysconf(_SC_PAGESIZE);

	int fd = shm_open(NAME, O_CREAT | O_RDWR, 0600);
	if(fd < 0)
	{
		perror("shm_open\n");
		exit(EXIT_FAILURE);
	}

	int r = ftruncate(fd, region_size);

	struct region *ptr = mmap(0, region_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);

	if(atoi(argv[1]) > 100 || atoi(argv[1]) < 0)
		exit(1);

	srand(atoi(argv[1]));

	if(atoi(argv[2]) != 1 && atoi(argv[2]) != 0)
		exit(1);
	ptr->mode = atoi(argv[2]);  //0:基本功能; 1:進階功能
	ptr->count = 1;
	ptr->done = false;
	ptr->index[ptr->count] == -1;
	ptr->status = 1;
	ptr->start_test = false;

	pid_t pid = fork();

	if(pid == 0)  //child process
	{
		srand(atoi(argv[1] + 1));

		struct coordinate target;
		target.x = rand() % 10 + 0;
		target.y = rand() % 10 + 0;
		//printf("start: [%d,%d]\n", target.x, target.y);
		int decide_cheating = 0;
		bool finish_cheating = false;

		while(!ptr->done)
		{
			while(ptr->status == 1)
				;

			if(ptr->created)
			{
				printf("[%d] Child: OK\n", getpid());
				ptr->created = false;
			}
			else
			{
				if(ptr->count == 5 || (ptr->result[ptr->count].x == target.x && ptr->result[ptr->count].y == target.y))
				{
					ptr->done = true;
					if(ptr->result[ptr->count].x == target.x && ptr->result[ptr->count].y == target.y)
					{
						if(ptr->mode == 1 && decide_cheating >= ptr->count && ptr->count <= 3)
							decide_cheating = 0;
						printf("[%d] Child: You win\n", getpid());
					}
					else
					{
						printf("[%d] Child: Miss, you lose\n", getpid());
						ptr->result[ptr->count].x = target.x;
						ptr->result[ptr->count].y = target.y;
					}
				}
				else
				{
					decide_direction(ptr, target);  //child決定猜測結果

					if(ptr->mode == 1 && !finish_cheating)
					{
						decide_cheating = rand() % 4 + 0;
						finish_cheating = true;
					}

					if(ptr->mode == 1 && decide_cheating > 0 && ptr->count == decide_cheating)
							ptr->index[ptr->count] = -(ptr->index[ptr->count]);

					if(ptr->index[ptr->count] == 1)  //child輸出猜測結果
						printf("[%d] Child: Miss, down\n", getpid());
					else if(ptr->index[ptr->count] == -1)
						printf("[%d] Child: Miss, up\n", getpid());
					else if(ptr->index[ptr->count] == 2)
						printf("[%d] Child: Miss, left\n", getpid());
					else if(ptr->index[ptr->count] == -2)
						printf("[%d] Child: Miss, right\n", getpid());
					else if(ptr->index[ptr->count] == 3)
						printf("[%d] Child: Miss, down right\n", getpid());
					else if(ptr->index[ptr->count] == 4)
						printf("[%d] Child: Miss, down left\n", getpid());
					else if(ptr->index[ptr->count] == -4)
						printf("[%d] Child: Miss, up right\n", getpid());
					else if(ptr->index[ptr->count] == -3)
						printf("[%d] Child: Miss, up left\n", getpid());
				}
				ptr->count++;
			}

			ptr->status = 1;
		}
	}
	else  //parent process
	{
		ptr->pid = pid;
		printf("[%d] Parent: Create a child %d\n", getpid(), pid);
		ptr->created = true;
		ptr->status = 0;

		double dis[2];
		int find_cheating;
		while(!ptr->done)
		{
			while(ptr->status == 0)
				;

			if(ptr->count == 1)  //parent第一次隨便猜
			{
				ptr->result[1].x = rand() % 10 + 0;
				ptr->result[1].y = rand() % 10 + 0;
			}
			else
			{
				if(!ptr->done)
				{
					if(ptr->mode == 1)
					{
						decide_coordinate(ptr);
						if(ptr->count == 4)
						{
							ptr->start_test = true;
							for(int j = 1; j <= 3; j++)
							{
								decide_coordinate(ptr);
								dis[0] = sqrt((ptr->result[j].x - ptr->result[4].x)*(ptr->result[j].x - ptr->result[4].x) + (ptr->result[j].y - ptr->result[4].y)*(ptr->result[j].y - ptr->result[4].y));
								dis[1] = sqrt((ptr->buffer[j].x - ptr->result[4].x)*(ptr->buffer[j].x - ptr->result[4].x) + (ptr->buffer[j].y - ptr->result[4].y)*(ptr->buffer[j].y - ptr->result[4].y));

								if(dis[0] > dis[1])
									find_cheating = j;

								i++;
							}

							ptr->start_test = false;
						}
					}
					else
						decide_coordinate(ptr);
				}
			}

			if(!ptr->done)
				printf("[%d] Parent: Guess [%d,%d]\n", getpid(), ptr->result[ptr->count].x, ptr->result[ptr->count].y);

			ptr->status = 0;
		}

		if(ptr->mode == 0 && ptr->done)
			printf("[%d] Parent: Target [%d,%d]\n", getpid(), ptr->result[(ptr->count - 1)].x, ptr->result[(ptr->count - 1)].y);
		else if(ptr->mode == 1)
		{
			if(find_cheating == 0)
				printf("[%d] Parent: No cheating\n", getpid());
			else
				printf("[%d] Parent: Guess [%d,%d] is cheating\n", getpid(), ptr->result[find_cheating].x, ptr->result[find_cheating].y);
		}

		munmap(ptr, region_size);
		shm_unlink(NAME);
	}
}
