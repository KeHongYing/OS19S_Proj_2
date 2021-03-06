#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#define PAGE_SIZE 4096
#define BUF_SIZE 512
#define MMAP_SIZE PAGE_SIZE * 128

#define IOCTL_MMAP 0x12345678
#define IOCTL_PRINT 0x10101010

int main (int argc, char* argv[])
{
	char buf[BUF_SIZE];
	int i, dev_fd, file_fd;// the fd for the device and the fd for the input file
	size_t ret, file_size = 0, data_size = 0;
	char file_name[50];
	char method[20];
	char ip[20];
	struct timeval start;
	struct timeval end;
	double trans_time; //calulate the time between the device is opened and it is closed
	char *kernel_address, *file_address;


	strcpy(file_name, argv[1]);
	strcpy(method, argv[2]);
	strcpy(ip, argv[3]);

	if( (dev_fd = open("/dev/slave_device", O_RDWR)) < 0)//should be O_RDWR for PROT_WRITE when mmap()
	{
		perror("failed to open /dev/slave_device\n");
		return 1;
	}
	gettimeofday(&start ,NULL);
	if( (file_fd = open (file_name, O_RDWR | O_CREAT | O_TRUNC)) < 0)
	{
		perror("failed to open input file\n");
		return 1;
	}

	if(ioctl(dev_fd, 0x12345677, ip) == -1)	//0x12345677 : connect to master in the device
	{
		perror("ioclt create slave socket error\n");
		return 1;
	}

    //write(1, "ioctl success\n", 14);

	switch(method[0])
	{
		case 'f'://fcntl : read()/write()
			do
			{
				ret = read(dev_fd, buf, sizeof(buf)); // read from the the device
				write(file_fd, buf, ret); //write to the input file
				file_size += ret;
			}while(ret > 0);
			break;

		case 'm':
			while(1){
				ret = ioctl(dev_fd, IOCTL_MMAP);
				//printf("slave ioctl return %ld\n", ret);
				if( ret < 0 ){
					perror("slave ioctl mmap failed\n");
					return 2;
				}else if( ret == 0 ){
					file_size = data_size;
					break;
				}
				posix_fallocate(file_fd, data_size, ret);
				file_address = mmap(NULL, ret, PROT_WRITE, MAP_SHARED, file_fd, data_size);
				kernel_address = mmap(NULL, ret, PROT_READ, MAP_SHARED, dev_fd, 0);
				memcpy(file_address, kernel_address, ret);
				data_size += ret;
			}
			break;
	}

	if( ioctl(dev_fd, IOCTL_PRINT) == -1 ){
		perror("slave ioctl print page descriptor failed\n");
		return 3;
	}

	if(ioctl(dev_fd, 0x12345679) == -1)// end receiving data, close the connection
	{
		perror("ioctl client exits error\n");
		return 1;
	}
	gettimeofday(&end, NULL);
	trans_time = (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)*0.0001;
	printf("Slave Transmission time: %lf ms, File size: %ld bytes\n", trans_time, file_size);


	close(file_fd);
	close(dev_fd);
	return 0;
}


