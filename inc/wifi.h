#ifndef WIFI_H
#define WIFI_H

extern char ip[32];

int startWifi(const char ssid[], const char password[]);
int mostrarIP(void);

#endif
