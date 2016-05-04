#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <utime.h>

void WalkDir(char *, char *);
void CopyFile(char *, char *);

int main(int argc, char *argv[])
{
    struct stat statbuf;            // structure info of a file
    struct utimbuf time;            // access and modification time of a file
    
    DIR *dir;                       // structure describing an open directory.
    
    if (argc != 3) {
        printf("arguments error, check input!\n");
    }
    else
    {
        if ((dir = opendir(argv[1])) == NULL) {
            printf("source folder does not exist.\n");
            exit(-1);
        }
        if ((dir = opendir(argv[2])) == NULL) {
            stat(argv[1], &statbuf);                // get info of source folder/file
            mkdir(argv[2], statbuf.st_mode);        // st_mode define the file type and access
            
            time.actime = statbuf.st_atime;         // last access time
            time.modtime = statbuf.st_mtime;        // last modify time
            time
            
            utime(argv[2], &time);
        }
        
        WalkDir(argv[1], argv[2]);
    }
    printf("Copy Finished!\n");
    return 0;
}

void WalkDir(char *source, char*target)
{
    char sourceCopy[1024];
    char targetCopy[1024];
    
    struct stat statbuf;
    struct utimbuf time;
    
    DIR *dir;
    struct dirent *dirEntry;
    
    strcpy(sourceCopy, source);
    strcpy(targetCopy, target);
    
    dir = opendir(source);                          // get info of source
    
    while ((dirEntry = readdir(dir)) != NULL) {
        
        if (strcmp(dirEntry->d_name, ".") == 0 || strcmp(dirEntry->d_name, "..") == 0) {
            continue;                               // jump over the parent and self dir
        }
        
        strcat(sourceCopy, "/");
        strcat(sourceCopy, dirEntry->d_name);
        strcat(targetCopy, "/");
        strcat(targetCopy, dirEntry->d_name);
        
        if (dirEntry->d_type == 4) {                // folder
            
            stat(sourceCopy, &statbuf);             // get source folder info
            mkdir(targetCopy, statbuf.st_mode);     // make target folder
            
            time.actime = statbuf.st_atime;
            time.modtime = statbuf.st_mtime;
            
            utime(targetCopy, &time);
            
            WalkDir(sourceCopy, targetCopy);        // recursively exectute copy operation at child folder
            
        }
        else                                        // file
        {
            CopyFile(sourceCopy, targetCopy);
        }
        
        strcpy(sourceCopy, source);                 // change it back after call WalkDir()
        strcpy(targetCopy, target);
    }
    
    closedir(dir);
}

void CopyFile(char *source, char*target)
{
    int fdSrc = open(source,0);                     // open source file
    int fdTar;
    
    struct stat statbuf;
    struct utimbuf time;
    
    char BUFFER[1024];
    int bitNum;
    
    stat(source, &statbuf);
    fdTar = creat(target, statbuf.st_mode);
    
    while ((bitNum = read(fdSrc, BUFFER, 1024)) > 0) {
        write(fdTar, BUFFER, bitNum);
    }
    
    time.actime = statbuf.st_atime;
    time.modtime = statbuf.st_mtime;
    utime(target, &time);
    
    close(fdSrc);
    close(fdTar);

}










