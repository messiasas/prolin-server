/*------------------------------------------------------------
* FileName: main.c
* Author: LuX
* Date: 2013-10-24
* Example of how to use Prolin OSAL API. It can only run on Prolin OS 2.4 or higher.
*
*                            Warning
* This code is for reference only. I do not guarantee the correctness, safety, efficiency or completeness.
* You are welcome to send an e-mail to me(lux@paxsz.com) if you have any questions.
------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <osal.h>
#include <xui.h>

#include "wifi.h"
#include "clientThread.h"

#include "curl/curl.h"
#include <arpa/inet.h>

#include <pthread.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

#include <dirent.h>
#include <sys/stat.h>

volatile int server_running = 1;
//int ip_y = 0;

#define SERVER_PORT 8080

static int GuiInit(int statusbar_height)
{
    char value[16];
    char rotate_str[32];
    char statusbar_str[32];
    int ret;
    char *xui_argv[10];
    int xui_argc;

    ret = OsRegGetValue("ro.fac.lcd.rotate", value);
    if (ret > 0) {
        snprintf(rotate_str, sizeof(rotate_str), "ROTATE=%s", value);
    }
    else {
        strcpy(rotate_str, "ROTATE=0");
    }

    xui_argv[0] = rotate_str;
    xui_argv[1] = statusbar_str;
    xui_argv[2] = NULL;
    xui_argc = 2;

    ret = XuiOpen(xui_argc, xui_argv);
    if (ret == XUI_RET_OK) {
        return RET_OK;
    }
    else {
        return -1;
    }
}

static void GuiDeinit(void)
{
    XuiClose();
}

static void CrashReportInit(void)
{
    signal(SIGILL,    OsSaveCrashReport);
    signal(SIGABRT,   OsSaveCrashReport);
    signal(SIGBUS,    OsSaveCrashReport);
    signal(SIGFPE,    OsSaveCrashReport);
    signal(SIGSEGV,   OsSaveCrashReport);
    signal(SIGSTKFLT, OsSaveCrashReport);
    signal(SIGPIPE,   OsSaveCrashReport);
}

int wait_connection(int server_fd)
{
	struct sockaddr_in client_addr;
	socklen_t client_len;

	while (server_running)
	{
		// ======================
		// SELECT
		// ======================

		fd_set readfds;

		FD_ZERO(&readfds);

		FD_SET(server_fd, &readfds);

		struct timeval timeout;

		timeout.tv_sec = 0;
		timeout.tv_usec = 100000;

		int activity = select(
			server_fd + 1,
			&readfds,
			NULL,
			NULL,
			&timeout
		);

		if (activity > 0)
		{
			client_len = sizeof(client_addr);

			int *client_fd = malloc(sizeof(int));

			*client_fd = accept(
				server_fd,
				(struct sockaddr *)&client_addr,
				&client_len
			);

			if (*client_fd >= 0)
			{
				OsLog(
					LOG_DEBUG,
					"Cliente conectado: %s",
					inet_ntoa(client_addr.sin_addr)
				);

				pthread_t thread_id;

				pthread_create(
					&thread_id,
					NULL,
					clientThread,
					client_fd
				);

				pthread_detach(thread_id);
			}
			else
			{
				free(client_fd);
			}
		}
	}
}

int Server(void)
{

    XuiColor colorWhite = {0xFF,0xFF,0xFF,0xFF};
    XuiColor colorBlue = {0xFF,0x00,0x00,0xFF};
    XuiColor colorGray = {0xBE,0xBE,0xBE,0xFF};
    XuiColor colorRed = {0x00,0x00,0xFF,0xFF};
    XuiColor colorBlack = {0x00,0x00,0x00,0xFF};
    XuiColor colorGreen = {0x00, 0xFF, 0x00, 0xFF};

    XuiFont *font;
    font = XuiCreateFont("/usr/font/paxfont.ttf",0,0);

    XuiWindow *layUp = XuiCreateCanvas(XuiRootCanvas(),0,0,480,200);
    XuiCanvasDrawRect(layUp,0,0,480,200,colorBlue,0,1);

    XuiWindow *layDown =XuiCreateCanvas(XuiRootCanvas(),0,200,480,700);
    XuiCanvasDrawRect(layDown,0,0,480,700,colorGray,0,1);

    XuiWindow *layMSG = XuiCreateCanvas(layUp,200,100,150,50);
    XuiCanvasDrawRect(layMSG,0,0,480,200,colorBlue,0,1);

    XuiShowWindow(layUp,1,0);
    XuiShowWindow(layDown,1,0);
    XuiShowWindow(layMSG,1,0);

    XuiCanvasDrawText(layUp,10,10,30,font,XUI_TEXT_BOLD,colorGray,"MSP");
    XuiCanvasDrawText(layMSG,0,0,30,font,XUI_TEXT_NORMAL,colorWhite,"AWAIT");

    //short ret = startWifi("AMAZONAS INOVARE 2.4G","987654321");
    short ret = startWifi("WM","123456789");

    mostrarIP();

    if (ret < 0)
    {
        XuiCanvasDrawText(layUp,170,170,30,font,XUI_TEXT_NORMAL,colorWhite,"disconnect");
        XuiShowWindow(layMSG,0,0);
    }else{
    	XuiCanvasDrawText( layUp,180,170,30,font,XUI_TEXT_NORMAL,colorWhite,"network");
    	XuiShowWindow(layMSG,0,0);
    }

    XuiCanvasDrawText(layUp,155,10,30,font,XUI_TEXT_NORMAL,colorWhite,ip);

    // =====================================================
    // FIELD CONNECTION INFORMATION IP
    // =====================================================

    XuiWindow *layConnect =XuiCreateCanvas(layDown,0,0,230,450);

    XuiCanvasDrawRect(layConnect,0,0,230,450,colorGray,0,1);

    XuiShowWindow(layConnect,1,0);

    // =====================================================
    // SERVER
    // =====================================================

    int server_fd;

    struct sockaddr_in server_addr;

    server_fd = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (server_fd < 0)
    {
        OsLog(LOG_DEBUG,"Erro socket");
        //return -1;
    }

    int opt = 1;

    // Allows use the port again even after turning off the server
    setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    memset(
        &server_addr,
        0,
        sizeof(server_addr)
    );

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    if (bind(
            server_fd,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr)
        ) < 0)
    {
        OsLog(LOG_DEBUG,"Erro bind");

        close(server_fd);

        return -1;
    }

    if (listen(server_fd,15) < 0)
    {
        OsLog(LOG_DEBUG,"Erro listen");
        close(server_fd);
        return -1;
    }

    OsLog(LOG_DEBUG,"Servidor aguardando");

    // =====================================================
    // LOOP PRINCIPAL
    // =====================================================

    struct sockaddr_in client_addr;

    socklen_t client_len;

    int ip_y = 0;

    XuiCanvasDrawText(layConnect,0,0,25,font,XUI_TEXT_NORMAL,colorBlack, "IP connect:");

    while (server_running)
    {
        if (XuiHasKey())
        {
            int key = XuiGetKey();

            if (key == XUI_KEYCANCEL)
            {
                server_running = 0;
                break;
            }
        }

        fd_set readfds; // Read files like a number

        FD_ZERO(&readfds);

        FD_SET(server_fd,&readfds); // readfds will monitoring the server_fd
									// In this case, it will monitoring new connections
        struct timeval timeout;

        timeout.tv_sec = 0;
        timeout.tv_usec = 100000; // select() await at most 1 second

        int activity = select(server_fd + 1, &readfds, NULL,NULL,&timeout);

        if (activity > 0)
        {
            client_len = sizeof(client_addr);

            int *client_fd = malloc(sizeof(int));

            *client_fd = accept(
                server_fd,
                (struct sockaddr *)&client_addr,
                &client_len
            );

            if (*client_fd >= 0)
            {
                char *client_ip =inet_ntoa(client_addr.sin_addr);
                OsLog(LOG_DEBUG,"Customer: %s",client_ip);

                XuiCanvasDrawText(layConnect,0,ip_y+35,25,font,XUI_TEXT_NORMAL,colorBlack, client_ip);


                pthread_t thread_id;
                int retconnect = pthread_create(&thread_id,NULL,clientThread,client_fd);

               /* if( retconnect < 0 ){
                	XuiCanvasDrawText(layConnect,200,ip_y+35,25,font,XUI_TEXT_NORMAL,colorRed, "FAILED");
                }else{
                	XuiCanvasDrawText(layConnect,200,ip_y+35,25,font,XUI_TEXT_NORMAL,colorGreen, "STARTED");
                }*/

                pthread_detach(thread_id);
                ip_y += 35;
            }
            else
            {
                free(client_fd);
            }
        }

        usleep(10000);
    }

    close(server_fd);
    return 0;
}


int main(int argc, char **argv)
{
	OsLogSetTag("[  MSP  ]");
    CrashReportInit();
    GuiInit(18);
    Server();
    GuiDeinit();
    return 0;
}
