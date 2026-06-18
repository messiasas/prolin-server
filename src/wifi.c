#include "wifi.h"
#include <stdio.h>
#include <string.h>
#include <osal.h>


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


