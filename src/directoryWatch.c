#include <sys/inotify.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

int gQuit = 0;

void leave(int sig)
{
    gQuit = 1;
}


void print_event(struct inotify_event* event)
{
    // if there is no filename, then that just means the directory was opened
    // or touched, so we don't need to bother printing anything
    if(event->len)
        printf ("%s,", event->name);
    else
        return;

    if(event->mask & IN_ACCESS)
        printf("IN_ACCESS");
    if(event->mask & IN_MODIFY)
        printf("IN_MODIFY");
    if(event->mask & IN_ATTRIB)
        printf("IN_ATTRIB");
    if(event->mask & IN_CLOSE_WRITE)
        printf("IN_CLOSE_WRITE");
    if(event->mask & IN_CLOSE_NOWRITE)
        printf("IN_CLOSE_NOWRITE");
    if(event->mask & IN_OPEN)
        printf("IN_OPEN");
    if(event->mask & IN_MOVED_FROM)
        printf("IN_MOVED_FROM");
    if(event->mask & IN_MOVED_TO)
        printf("IN_MOVED_TO");
    if(event->mask & IN_CREATE )
        printf("IN_CREATE");
    if(event->mask & IN_DELETE)
        printf("IN_DELETE");
    if(event->mask & IN_DELETE_SELF)
        printf("IN_DELETE_SELF");
    if(event->mask & IN_MOVE_SELF)
        printf("IN_MOVE_SELF");

    printf("\n");
}


int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        printf("usage directoryWatch path1 path2 path3 ...\n");
        exit(1);
    }

    int* dWatch = (int* )malloc( (argc-1)*sizeof(int) );
    if(!dWatch)
    {
        printf("failed to allocate descriptor array\n");
        exit(1);
    }

    // create an inotify instance and get a fd that we can read from
    int fdNotify = inotify_init();
    if( fdNotify < 0)
    {
        perror("inotify_init");
        exit(1);
    }

    int i;
    for(i=1; i < argc; i++)
        dWatch[i-1] = inotify_add_watch(fdNotify, argv[i], IN_ALL_EVENTS);

    const int   event_size = sizeof(struct inotify_event);
    const int   buf_size   = 1024*(event_size+16);

    char    buf[buf_size];
    int     len;

    struct inotify_event *event;

    signal(SIGINT,leave);

    while(!gQuit)
    {
        len = read (fdNotify, buf, buf_size);
        if (len < 0)
        {
            if (errno == EINTR)
                continue;
            else
            {
                perror ("read");
                exit(1);
            }
        } else if (!len)
            exit(1);

        for(i=0; i < len; i += event_size + event->len)
        {
            event = (struct inotify_event *) &buf[i];

            //printf ("wd=%d mask=%u cookie=%u len=%u\n",
            //        event->wd,
            //        event->mask,
            //        event->cookie,
            //        event->len);

            print_event(event);
        }
    }

    for(i=1; i < argc; i++)
        if(dWatch[i-1] > 0)
            inotify_rm_watch(fdNotify, dWatch[i-1]);

    printf("\n");
    free(dWatch);
    return 0;
}
