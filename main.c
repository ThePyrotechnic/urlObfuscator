#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>

void toDWord(char *, int size, char *);
void toHex(char*, int size);
//void reverse(char *seg, int start, int end);
void toGarbage(char *, int size, char *);

int main(int argc, char *argv[])
{
    bool help = false;
    bool safe = false;
    bool noURL = false;
    int prefixPos = -1;
    for(int a = 0;a < argc;a++)
    {
        if (strcmp(argv[a],"-h") == 0) help = true;
        else if (strcmp(argv[a],"-s") == 0) safe = true;
        else if (strcmp(argv[a],"-p") == 0 && argv[a+1] != NULL) prefixPos = a + 1;
        else if (strcmp(argv[a],"-S") == 0) noURL = true;
    }
    if(argc < 2 || help)
    {
        printf("usage: urlObfus url(don't include \"http\") [options]\n"
                       "\tOPTIONS:\n"
                       "\t\t-p [prefix]\t\t-Include a prefix before the spoofed URL\n"
                       "\t\t-S\t\t-Do not convert address at all\n"
                       "\t\t-s\t\t-Do not convert address to dword\n"
                       "\t\t-h\t\t-Display this menu");
    }
    else
    {
        char *rawUrl;
        rawUrl = (char *)malloc(20*sizeof(argv[1]));
        strcpy(rawUrl,argv[1]);

        char subs[7];
        for(int a = 0;a < 7; a++)
            subs[a] = rawUrl[a];
        char *url;
        url = (char *)malloc(20*sizeof(argv[1]));

        if(strcmp(subs,"http://") == 0)
        {
            char *start = rawUrl + 7;
            strcpy(url,start);
        }
        else if (strcmp(subs,"https:/") == 0)
        {
            char *start = rawUrl + 8;
            strcpy(url,start);
        }
        else
            strcpy(url,rawUrl);

        char* domain;
        domain = calloc(strlen(url),sizeof(char));
        bool found = false;
        int pos = 0;
        char *dir;
        dir = calloc(strlen(url),sizeof(char));
        int dirDelim = 0;

        for(int a = 0;a < strlen(url);a++)
        {
            if(found)
                dir[dirDelim++] = url[a];
            if(url[a] == '/' && !found)
            {
                found = true;
                pos = a;
            }
        }
        if(found)
            for(int a = 0;a < pos;a++)
                domain[a] = url[a];
        else strcpy(domain,url);

        //URL to IP
        char ip[15];

        struct addrinfo hints, *addrList;
        memset(&hints,0,sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        if(getaddrinfo(domain,NULL,&hints,&addrList) == 0)
            inet_ntop (addrList->ai_family, &((const struct sockaddr_in *)addrList->ai_addr)->sin_addr,ip,sizeof(ip));
        else
        {
            printf("ERROR: Unable to resolve IP address");
            return 0;
        }

        char *garbageDir;
        garbageDir = calloc(3 * strlen(dir), sizeof(char));
        toGarbage(dir,(int) strlen(dir), garbageDir);

        char *new;
        if(prefixPos != -1)
        {
            char *prefix;
            prefix = argv[prefixPos];
            new = calloc(strlen(domain) + strlen(prefix) + strlen(garbageDir) + 2,sizeof(char));
            strcat(new, prefix);
            strcat(new,"@");
        }
        else new = calloc(strlen(domain) + strlen(garbageDir) + 2,sizeof(char));
        if(noURL)
        {
            strcat(new,domain);
        }
        else if(safe)
            strcat(new,ip);
        else
        {
            char *dword;
            dword = (char *)malloc(sizeof(domain));
            toDWord(ip,sizeof(ip),dword);
            strcat(new, dword);
        }

        strcat(new,"/");
        strcat(new,garbageDir);
        printf("%s",new);

        free(dir);
        free(garbageDir);
        free(new);
        free(url);
        free(domain);
    }

    return 0;
}

void toDWord(char *address, int size, char *dword)
{
    char *token, *hex, *addr;
    hex = calloc((size_t) size, sizeof(char));
    addr = strdup(address);
    while((token = strsep(&addr,".")) != NULL)
    {
        toHex(token, (int) strlen(token));
        strcat(hex,token);
    }
    snprintf(dword,(size_t) size,"%li",strtol(hex,NULL,16));
}

void toHex(char *seg, int size)
{
    char *ref = NULL;
    int dec = (int) strtol(seg,&ref,10);
    snprintf(seg, (size_t) size + 1, "%X", dec);
}

void toGarbage(char *dir, int size, char *garbage)
{

    for(int a = 0; a < size; a++)
    {
        char buf[3];
        if((dir[a] > 64 && dir[a] < 91) || (dir[a] > 96 && dir[a] < 123))
        {
            int charNum = dir[a];
            snprintf(buf, 4, "%%%X",charNum);
            strcat(garbage,buf);
        }
        else
        {
            char *tmp;
            tmp = calloc(2,sizeof(char));
            snprintf(tmp,2,"%c",dir[a]);
            strcat(garbage,tmp);
        }
    }
}