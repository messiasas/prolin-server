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

#include "curl/curl.h"
#include <arpa/inet.h>

#include <pthread.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

volatile int server_running = 1;
int ip_y = 0;

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

char ip[32] = {0};

int startWifi(const char ssid[], const char password[]){

	int i;
	int route;

	ST_WifiApInfo *Aps;
	ST_WifiApSet ApSet;

	OsLog(LOG_DEBUG, "Abrindo modulo WiFi...");

	int ret = OsWifiOpen();
	if (ret != RET_OK)
	{
		OsLog(LOG_DEBUG, "Erro OsWifiOpen = %d", ret);
	}

	OsLog(LOG_DEBUG, "Escaneando redes...");

	int netQuantity = OsWifiScan(&Aps);

	if (netQuantity <= 0)
	{
		OsLog(LOG_DEBUG, "Nenhuma rede encontrada");
	}
	char text[20] = {0};
	sprintf(text, "%s", ssid);

	OsLog(LOG_DEBUG, "Procurando rede: %s", ssid);

	for (i = 0; i < netQuantity; i++)
	{
		if (strcmp(Aps[i].Essid, ssid) == 0)
		{
			OsLog(LOG_DEBUG, "Rede encontrada, conectando...");

			memset(&ApSet, 0, sizeof(ApSet));

			strcpy(ApSet.Essid, Aps[i].Essid);
			strcpy(ApSet.Bssid, Aps[i].Bssid);

			ApSet.Channel  = Aps[i].Channel;
			ApSet.Mode     = Aps[i].Mode;
			ApSet.AuthMode = Aps[i].AuthMode;
			ApSet.SecMode  = Aps[i].SecMode;

			// WPA/WPA2 senha
			strcpy(ApSet.KeyUnion.PskKey.Key, password);
			ApSet.KeyUnion.PskKey.KeyLen = strlen(password);

			ret = OsWifiConnect(&ApSet, 10000);

			if (ret != RET_OK)
			{
				OsLog(LOG_DEBUG, "Erro ao conectar = %d", ret);

			}

			OsLog(LOG_DEBUG, "Conectado com sucesso!");

			route = OsNetGetRoute();
			if (route != NET_LINK_WIFI)
			{
				OsNetSetRoute(NET_LINK_WIFI);
			}

			OsNetStartDhcp(NET_LINK_WIFI);

			OsLog(LOG_DEBUG, "DHCP iniciado");
			return 0;
		}
	}
	OsLog(LOG_DEBUG, "Rede nao encontrada");
	return -1;
}



int mostrarIP(void)
{
    char buffer[256];

    FILE *fp = popen("ifconfig wlan0", "r");
    if(fp == NULL) {
        return -1;
    }

    while(fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        char *ptr = strstr(buffer, "inet addr:");
        if(ptr != NULL) {
            sscanf(ptr, "inet addr:%31s", ip);
            break;
        }
    }

    pclose(fp);

    if(strlen(ip) == 0) {
        //strcpy(ip, "no connect");
    }

    char linha[64];
    OsLog(LOG_DEBUG,"IP: %s", ip);

    return 0;
}


int *clientThread(void *arg)
{
    XuiColor colorWhite = {0xFF,0xFF,0xFF,0xFF};
    XuiColor colorBlue = {0xFF,0x00,0x00,0xFF};
    XuiColor colorGray = {0xBE,0xBE,0xBE,0xFF};
    XuiColor colorRed = {0x00,0x00,0xFF,0xFF};
    XuiColor colorBlack = {0x00,0x00,0x00,0xFF};
    XuiColor colorGreen = {0x00, 0xFF, 0x00, 0xFF};

    XuiFont *font;
    font = XuiCreateFont("/usr/font/paxfont.ttf",0,0);

    int client_fd = *(int *)arg;

    free(arg);

    OsLog(LOG_DEBUG, "Thread cliente iniciada");

    char buffer[1024];

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));

        int bytes = recv(client_fd,buffer,sizeof(buffer),0);

        if (bytes <= 0)
        {
            OsLog(LOG_DEBUG, "Cliente desconectado");
            break;
        }

        XuiWindow *layMSG =XuiCreateCanvas(XuiRootCanvas(),200,ip_y + 200,270,30);
        XuiCanvasDrawRect(layMSG,0,0,270,30,colorGray,0,1);
        XuiShowWindow(layMSG,1,0);

        OsLog(LOG_DEBUG, "Recebido: %s", buffer);

        if (strncmp(buffer, "GET ", 4) == 0)
        {
            char filename[256];

            char cwd[512];

            getcwd(cwd, sizeof(cwd));

            OsLog(LOG_DEBUG, "CWD: %s", cwd);

            memset(filename, 0, sizeof(filename));
            sscanf(buffer,"GET %255s",filename);

            filename[strcspn(filename, "\r\n")] = 0;

            OsLog(LOG_DEBUG,"Arquivo solicitado: %s",filename);

            char filepath[512];
            snprintf( filepath, sizeof(filepath), "./res/storage/so/%s",filename);

            OsLog(LOG_DEBUG,"Abrindo: %s",filepath);

            // ==================================
            // ABRIR ZIP
            // ==================================
            OsLog(LOG_DEBUG, "PATH FINAL: %s", filepath);

            FILE *fp = fopen(filepath, "rb");

            if (fp == NULL)
            {
                OsLog(LOG_DEBUG, "Arquivo nao encontrado");

                XuiCanvasDrawText(layMSG,0,0,25,font,XUI_TEXT_BOLD,colorRed, "NOT FOUND");

                send(client_fd,"ERROR FILE_NOT_FOUND", 21,0);
                continue;
            }

            // ==================================
            // TAMANHO ARQUIVO
            // ==================================

            fseek(fp, 0, SEEK_END);

            long filesize = ftell(fp);

            rewind(fp);

            // ==================================
            // ENVIAR HEADER
            // ==================================

            char header[128];

            XuiCanvasDrawText(layMSG,0,0,25,font,XUI_TEXT_NORMAL,colorBlack, "STARTED");

            sprintf(header,"OK\nSIZE %ld\n", filesize);

            send(client_fd,header,strlen(header),0);

            OsLog(LOG_DEBUG,"Enviando arquivo (%ld bytes)",filesize);


            char file_buffer[1024];

            int read_bytes;


            while (
                (read_bytes = fread( file_buffer, 1, sizeof(file_buffer), fp)) > 0)
            {
                send( client_fd,file_buffer,read_bytes,0 );
            }

            fclose(fp);
            OsLog(LOG_DEBUG, "Arquivo enviado");

            XuiCanvasDrawText(layMSG,130,0,25,font,XUI_TEXT_BOLD,colorBlue, "SO SENT");
            return RET_OK;
        }
        else
        {
            send(
                client_fd,
                "ERROR UNKNOWN_COMMAND\n",
                22,
                0
            );
            return -1;
        }
    }

    close(client_fd);
    return NULL;
}


int listenConnect()
{
    int server_fd;

    struct sockaddr_in server_addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        OsLog(LOG_DEBUG, "Erro ao criar socket");

        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;

    server_addr.sin_addr.s_addr = INADDR_ANY;

    server_addr.sin_port = htons(SERVER_PORT);

    if (bind(server_fd,
            (struct sockaddr *)&server_addr,
            sizeof(server_addr)) < 0)
    {
        OsLog(LOG_DEBUG, "Erro no bind");

        close(server_fd);

        return -1;
    }

    if (listen(server_fd, 15) < 0)
    {
        OsLog(LOG_DEBUG, "Erro no listen");

        close(server_fd);

        return -1;
    }

    OsLog(LOG_DEBUG, "Servidor aguardando conexoes...");

    wait_connection(server_fd);

    close(server_fd);

    return 0;
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





void *serverThread(void *arg)
{
    listenConnect();

    return NULL;
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

    // =====================================================
    // WIFI
    // =====================================================
    XuiCanvasDrawText(layMSG,0,0,30,font,XUI_TEXT_NORMAL,colorWhite,"AWAIT");

    short ret = startWifi("AMAZONAS INOVARE 5.0G","987654321");
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

    //int ip_y = 0;

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
	OsLogSetTag("[  PSF  ]");
    CrashReportInit();
    GuiInit(18);
    Server();
    GuiDeinit();
    return 0;
}
