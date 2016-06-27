
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/fcntl.h> /* 以下为新加 */
#include <sys/ioctl.h>
#define ADC_WRITE( ch, prescale )	( (ch) << 16 | (prescale) )
#define ADC_WRITE_GETCH( data )		( ( (data) >> 16) & 0x7)
#define ADC_WRITE_GETPRE( data )	( (data) & 0xff)
#define ADC_DEV			"/dev/adc/0raw"
#define DCM_DEV			"/dev/dcm/0raw"
#define STEP_DEV		"/dev/exio/0raw"
#define DCM_IOCTRL_SETPWM	(0x10)
#define DCM_TCNTB0		(16384)
#define STEPMOTOR_IOCTRL_PHASE	0x13
static int	dcm_fd		= -1;
static int	adc_fd		= -1;
static int	step_fd		= -1;
char		stepdata[]	= { 0x10, 0x30, 0x20, 0x60, 0x40, 0xc0, 0x80, 0x90 };
static int init_ADdevice( void )
{
	if ( (adc_fd = open( ADC_DEV, O_RDWR ) ) < 0 )
	{
		printf( "Error opening %s adc device\n", ADC_DEV );
		return(-1);
	}
}


static int GetADresult( int channel )
{
	int	PRESCALE	= 0XFF;
	int	data		= ADC_WRITE( channel, PRESCALE );
	write( adc_fd, &data, sizeof(data) );
	read( adc_fd, &data, sizeof(data) );
	return(data);
}


main()
{
	int			sockfd, client_fd;
	int			sin_size;
	struct sockaddr_in	my_addr;
	struct sockaddr_in	remote_addr;
	char			recv_buf[1024], cmd[20];
	char			adc_result[35]; /* "123456\0"共7个字符"{"temp":30,"humi":40,"light":50}\r\n\0"共35个字符 */
	int			value = 0;
	int			i, j;
	int			factor = DCM_TCNTB0 / 1024;
	if ( init_ADdevice() < 0 )
		printf( "init ADC error!\r\n" );
	if ( (dcm_fd = open( DCM_DEV, O_WRONLY ) ) < 0 )
		printf( "Error opening %s device\n", DCM_DEV );
	if ( (step_fd = open( STEP_DEV, O_WRONLY ) ) < 0 )
		printf( "Error opening /dev/exio/0raw device\n" );
	if ( (sockfd = socket( AF_INET, SOCK_STREAM, 0 ) ) == -1 )
	{
		printf( "socket error!" );
		/* exit(1); */
	}
	my_addr.sin_family	= AF_INET;
	my_addr.sin_port	= htons( 5000 );
	my_addr.sin_addr.s_addr = INADDR_ANY;
	if ( bind( sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr) ) == -1 )
	{
		printf( "Port has been used!" );
	}
	if ( listen( sockfd, 10 ) == -1 )
	{
		printf( "listen error!" );
		/* exit(1); */
	}

	sin_size = sizeof(struct sockaddr_in);
	while ( 1 )
	{       /* //////////////////////////////////////////////////////////////////////////////////////////// */
		if ( (client_fd = accept( sockfd, &remote_addr, &sin_size ) ) == -1 )
		{
			printf( "accept error!" );
			/* exit(1); */
		}
		memset( recv_buf, 0, 1024 );
		memset( cmd, 0, 20 );
		read( client_fd, recv_buf, 1024 );
		/* 只要http请求头的第一行 */
		for ( i = 0; i < 200; i++ )
		{
			if ( recv_buf[i] == '\n' )
			{
				recv_buf[i] = 0;
			}
		}
		printf( "1.recv_buf %s\r\n", recv_buf );
		sscanf( recv_buf, "GET /%s HTTP/1.1", cmd );                                                                                                                    /* 读取命令 */
		printf( "2.param from get methord: %s ----\r\n", cmd );
		/* get */
		if ( strstr( cmd, "get" ) != NULL )
		{
			printf( "3.param = get----\r\n" );
			/* ----------------------------------------------------------- */
			sprintf( adc_result, "{\"temp\":%2d,\"humi\":%2d,\"light\":%2d}\r\n",(int)(GetADresult( 0 ) / 10.24), (int)(GetADresult( 1 ) / 10.24), (int)(GetADresult( 2 ) / 10.24 ));   /*三路ad，原值范围0-1024，转换为0-100， */
			write( client_fd, "HTTP/1.1 200 OK\r\n", strlen( "HTTP/1.1 200 OK\r\n" ) );
			write( client_fd, "Server:lf_server\r\n\r\n", strlen( "Server:lf_server\r\n\r\n" ) );
			write( client_fd, adc_result, 34 );                                                                                                                     /* 只需要往回写34位，不包括"\0" */
			/*
			 * -----------------------------------------------------------
			 * return fake data
			 * write(client_fd,"{\"temp\":30,\"humi\":40,\"light\":50}\r\n",strlen("{\"temp\":30,\"humi\":40,\"light\":50}\r\n"));
			 */
		}else if ( strstr( cmd, "kongtiao" ) != NULL )
		{
			sscanf( cmd, "kongtiao(%2d)", &value );
			printf( "3.kongtiao value : %d", value );
			ioctl( dcm_fd, DCM_IOCTRL_SETPWM, value * 512 * factor / 100 );
			write( client_fd, "HTTP/1.1 200 OK\r\n", strlen( "HTTP/1.1 200 OK\r\n" ) );
			write( client_fd, "Server:lf_server\r\n\r\n", strlen( "Server:lf_server\r\n\r\n" ) );
			write( client_fd, "{\"result\":\"ok\"}\r\n", strlen( "{\"result\":\"ok\"}\r\n" ) );
		}else if ( strstr( cmd, "chuanglian" ) != NULL )
		{
			sscanf( cmd, "chuanglian(%2d)", &value );
			printf( "3.chuanglian value : %d", value );
			for ( j = 0; j < 512 * value / 100; j++ )
				for ( i = 0; i < sizeof(stepdata) / sizeof(stepdata[0]); i++ )
					ioctl( step_fd, STEPMOTOR_IOCTRL_PHASE, stepdata[i] );
			write( client_fd, "HTTP/1.1 200 OK\r\n", strlen( "HTTP/1.1 200 OK\r\n" ) );
			write( client_fd, "Server:lf_server\r\n\r\n", strlen( "Server:lf_server\r\n\r\n" ) );
			write( client_fd, "{\"result\":\"ok\"}\r\n", strlen( "{\"result\":\"ok\"}\r\n" ) );
		}


		/*else if{
		 * //空调 不用指针了，使用sscanf
		 * sscanf(cmd,"kongtiao(%2d)",&value);
		 * }*/
		else{
			/* return error */
			write( client_fd, "HTTP/1.1 200 OK\r\n", strlen( "HTTP/1.1 200 OK\r\n" ) );
			write( client_fd, "Server:lf_server\r\n\r\n", strlen( "Server:lf_server\r\n\r\n" ) );
			write( client_fd, "{\"result\":\"error\"}\r\n", strlen( "{\"result\":\"error\"}\r\n" ) );
		}
		close( client_fd );
	} /* while end */
	close( sockfd );
	close( adc_fd ); /*有打开就要有关闭，试验箱例程没有 */
	close( dcm_fd );
	close( step_fd );
}
