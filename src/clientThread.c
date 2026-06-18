#include "clientThread.h"
#include "file_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <osal.h>
#include <xui.h>

int ip_y = 35;

int *clientThread(void *arg)
{
    XuiColor colorWhite = {0xFF,0xFF,0xFF,0xFF};
    XuiColor colorBlue = {0xFF,0x00,0x00,0xFF};
    XuiColor colorGray = {0xBE,0xBE,0xBE,0xFF};
    XuiColor colorRed = {0x00,0x00,0xFF,0xFF};
    XuiColor colorBlack = {0x00,0x00,0x00,0xFF};
    XuiColor colorGreen = {0x00,0xFF,0x00,0xFF};

    XuiFont *font;
    font = XuiCreateFont("/usr/font/paxfont.ttf",0,0);

    int client_fd = *(int *)arg;
    free(arg);

    char buffer[1024];

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));

        int bytes = recv(client_fd, buffer, sizeof(buffer)-1, 0);

        if (bytes <= 0)
        {
            OsLog(LOG_DEBUG, "Cliente desconectado");
            break;
        }

        buffer[bytes] = '\0';

        OsLog(LOG_DEBUG, "Recebido: %s", buffer);

        // ==========================================
        // CRIAR JANELA STATUS
        // ==========================================

        XuiWindow *layMSG = XuiCreateCanvas( XuiRootCanvas(), 200,ip_y + 200,270,30);

        XuiCanvasDrawRect(layMSG, 0, 0, 270,30,colorGray,0,1);
        XuiShowWindow(layMSG,1,0);

        ip_y += 35;

        // ==========================================
        // PEGAR SOMENTE O COMANDO
        // ==========================================

        char command[32];

        memset(command,0,sizeof(command));

        sscanf(buffer, "%31s", command);

        command[strcspn(command, "\r\n")] = 0;

        OsLog(LOG_DEBUG,"Comando: %s",command);

        // ==========================================
        // DEFINIR PASTA BASE
        // ==========================================

        char filepath[512];

        if (strcmp(command, "GET_SO") == 0)
        {
			if (!getFirstFile("./res/storage/so",filepath,sizeof(filepath)))
			{
				OsLog(LOG_DEBUG,"Nenhum arquivo encontrado");

				send(client_fd,"FILE_NOT_FOUND",23,0);
				continue;
			}

			XuiCanvasDrawText(layMSG,0,0,25,font,XUI_TEXT_NORMAL,colorBlack,"SENDING SO");
        }
        else if (strcmp(command, "GET_FWP") == 0)
        {
			if (!getFirstFile("./res/storage/fwp",filepath,sizeof(filepath)))
			{
				OsLog(LOG_DEBUG,"Nenhum arquivo encontrado");

				send(client_fd,"FWP_NOT_FOUND",23,0);
				continue;
			}

			XuiCanvasDrawText(layMSG,0,0,25,font,XUI_TEXT_NORMAL,colorBlack,"SENDING FWP");
        }
        else if (strcmp(command, "GET_BOOT") == 0)
        {
			/*if (!getFirstFile("./res/storage/boot",filepath,sizeof(filepath)))
			{
				OsLog(LOG_DEBUG,"Nenhum arquivo encontrado");

				send(client_fd,"FILE_NOT_FOUND",23,0);
				continue;
			}

			XuiCanvasDrawText(layMSG,0,0,25,font,XUI_TEXT_NORMAL,colorBlack,"SENDING SO");*/

        }        else if (strcmp(command, "GET_APP") == 0)
        {
			/*if (!getFirstFile("./res/storage/app",filepath,sizeof(filepath)))
			{
				OsLog(LOG_DEBUG,"Nenhum arquivo encontrado");

				send(client_fd,"FILE_NOT_FOUND",23,0);
				continue;
			}

			XuiCanvasDrawText(layMSG,0,0,25,font,XUI_TEXT_NORMAL,colorBlack,"SENDING SO");*/
        }

        else
        {
            OsLog(LOG_DEBUG,"Comando desconhecido");
            XuiCanvasDrawText(layMSG,0,0,25,font,XUI_TEXT_NORMAL,colorRed,"COMMAND ERROR");
            continue;
        }

        OsLog(LOG_DEBUG,"Abrindo: %s",filepath);

        FILE *fp = fopen(filepath, "rb");

        if (fp == NULL)
        {
            OsLog(LOG_DEBUG,"Arquivo nao encontrado");

            XuiCanvasDrawText(layMSG,130, 0,25,font,XUI_TEXT_BOLD,colorRed,"NOT FOUND");

            send(client_fd,"ERROR FILE_NOT_FOUND\n",23,0);
            continue;
        }

        // ==========================================
        // TAMANHO ARQUIVO
        // ==========================================

        fseek(fp, 0, SEEK_END);

        long filesize = ftell(fp);

        rewind(fp);

        char header[128];

        sprintf(header,"OK\nSIZE %ld\n",filesize);

        send(client_fd,header, strlen(header),0);

        OsLog(LOG_DEBUG,"Enviando arquivo (%ld bytes)",filesize);

        // ==========================================
        // ENVIAR ARQUIVO
        // ==========================================

        char file_buffer[1024];

        int read_bytes;

        while (
            (read_bytes =fread(file_buffer,1, sizeof(file_buffer),fp) ) > 0
        )
        {
            send(client_fd,file_buffer,read_bytes,0);
        }

        fclose(fp);

        OsLog(LOG_DEBUG, "Arquivo enviado");
        XuiCanvasDrawText(layMSG,160,0,25,font,XUI_TEXT_BOLD,colorBlue,"SENT!");
    }

    close(client_fd);
    return NULL;
}

