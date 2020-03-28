
#include "daemonize.hpp"

int daemonize(const char* name, const char* path, const char* outfile, const char* errfile, const char* infile )
{
    if(!path) { path = "/"; }
    if(!name) { name = "daemon"; }
    if(!infile) { infile = "/dev/null"; }
    if(!outfile) { outfile = "/dev/null"; }
    if(!errfile) { errfile = "/dev/null"; }

    //printf("%s %s %s %s\n",name,path,outfile,infile);

    pid_t child;

    //fork, detach from process group leader

    if( (child=fork())<0 ) { //failed fork
        fprintf(stderr,"error: failed fork\n");
        exit(EXIT_FAILURE);
    }

    if (child>0) { //parent
        exit(EXIT_SUCCESS);
    }
    
    if( setsid()<0 ) { //failed to become session leader
        fprintf(stderr,"error: failed setsid\n");
        exit(EXIT_FAILURE);
    }

    //catch/ignore signals
    signal(SIGCHLD,SIG_IGN);
    signal(SIGHUP,SIG_IGN);

    //fork second time
    if ( (child=fork())<0) { //failed fork
        fprintf(stderr,"error: failed fork\n");
        exit(EXIT_FAILURE);
    }
    if( child>0 ) { //parent
        exit(EXIT_SUCCESS);
    }

    //new file permissions
    umask(0);
    //change to path directory
    chdir(path);

    //Close all open file descriptors
    int fd;
    for( fd=sysconf(_SC_OPEN_MAX); fd>0; --fd )
    {
        close(fd);
    }

    //reopen stdin, stdout, stderr
    stdin=fopen(infile,"r");   //fd=0
    stdout=fopen(outfile,"w+");  //fd=1
    stderr=fopen(errfile,"w+");  //fd=2

    //open syslog
    openlog(name,LOG_PID,LOG_DAEMON);
    return(0);
}