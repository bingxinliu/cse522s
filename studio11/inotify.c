#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>

#define BUF_LEN 1024

const int num_expected_args = 2;

typedef struct map_wd_path_t
{
    int wd;
    char path[BUF_LEN];
} map_wd_path;

int
main(int argc, char *argv[])
{
    int inotifyfd, wd;
    map_wd_path wds[256];
    char path[256];
    char buf[BUF_LEN] __attribute__((aligned(__alignof__(struct inotify_event))));
    ssize_t len, i, j;

    if ( argc != num_expected_args )
    {
        printf("Usage: ./inotify <>\n");
        return -1;
    }

    inotifyfd = inotify_init1(0);
    if ( inotifyfd == -1 )
    {
        printf("Error: inotify_init1 fail.\n");
        return -1;
    }
    printf("Initialize the inotify instance successfully. The value of file descriptor is %d.\n", inotifyfd);

    wd = inotify_add_watch(inotifyfd, argv[1], IN_ALL_EVENTS);
    if ( wd == -1 )
    {
        printf("Error: inotify_add_watch fail.\n");
        return -1;
    }
    printf("Adding a watch successfully. The file descriptor is %d, and the command line argument is %s.\n", wd, argv[1]);
    wds[wd].wd = wd;
    strcpy(wds[wd].path, argv[1]);

    /* Q5
    wd = inotify_add_watch(inotifyfd, argv[1], IN_MOVE);
    if ( wd == -1 )
    {
        printf("Error: inotify_add_watch IN_MOVE fail.\n");
        return -1;
    }
    printf("Adding a watch for moving successfully. The file descriptor is %d, and the command line argument is %s.\n", wd, argv[1]);
    */
    j = 0;

    while (1)
    {
        i = 0;
        len = read(inotifyfd, buf, BUF_LEN);
        printf("===========================================================\n");
        while ( i < len )
        {
            struct inotify_event *event = (struct inotify_event *) &buf[i];
            printf("wd=%d mask=%d cookie=%d len=%d dir=%s\n",\
                event->wd, event->mask, event->cookie, event->len, (event->mask & IN_ISDIR) ? "yes" : "no");
            if ( event->len )
            {
                printf("name=%s\n", event->name);
            }
            if (event->mask & IN_ACCESS)
                printf("File was accessed\n");
            if (event->mask & IN_MODIFY)
                printf("File was modified\n");
            if (event->mask & IN_ATTRIB)
                printf("Metadata changed\n");
            if (event->mask & IN_CLOSE_WRITE)
                printf("File opened for writing was closed.\n");
            if (event->mask & IN_CLOSE_NOWRITE)
                printf("File or directory not opened for writing was closed.\n");
            if (event->mask & IN_OPEN)
                printf("File or directory was opened.\n");
            if (event->mask & IN_MOVED_FROM)
                printf("File was move away from the watched directory.\n");
            if (event->mask & IN_MOVED_TO)
                printf("File was move into the watched directory.\n");
            if (event->mask & IN_DELETE)
                printf("File or directory deleted from watched directory.\n");
            if (event->mask & IN_DELETE_SELF)
                printf("Watched file or directory was itself deleted.\n");
            if (event->mask & IN_MOVE_SELF)
                printf("Watched file or directory was itself moved.\n");
            if (event->mask & IN_CREATE)
            {
                printf("File/directory created in watched directory\n");
                wd = inotify_add_watch(inotifyfd, strcat(strcat(strcpy(path, wds[event->wd].path), "/"), event->name), IN_ALL_EVENTS);
                if ( wd > 256)
                {
                    printf("Error: wds is full.\n");
                    return 0;
                }
                if ( wd == -1 )
                {
                    printf("Error: inotify_add_watch fail. The path is %s\n", path);
                    return -1;
                }
                printf("Adding a watch successfully. The file descriptor is %d, and the path is %s.\n", wd, path);
                wds[wd].wd = wd;
                strcpy(wds[wd].path, path);
                
            }
                
            i += sizeof(struct inotify_event) + event->len;
        }
        
    }
    

    return 0;
}