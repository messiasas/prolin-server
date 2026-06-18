#include "file_manager.h"

#include <dirent.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

int getFirstFile(char *folder, char *outputPath, size_t outputSize)
{
    DIR *dir;

    struct dirent *entry;

    dir = opendir(folder);

    if (dir == NULL)
    {
        return 0;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        // Ignora "." e ".."
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        // Monta caminho completo
        snprintf(outputPath,outputSize,"%s/%s",folder,entry->d_name);

        // Verifica se é arquivo normal
        struct stat st;

        if (stat(outputPath, &st) == 0)
        {
            if (S_ISREG(st.st_mode))
            {
                closedir(dir);
                return 1;
            }
        }
    }

    closedir(dir);

    return 0;
}
