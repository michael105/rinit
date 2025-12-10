#ifdef mlconfig

COMPILE exit_errno strlcpy strcpy open

OPTFLAG -Os
#STRIPFLAG
#LDSCRIPT text_and_bss

SHRINKELF

return
#endif

		// misc 2020/06
		// public domain / BSD 3clause
		//

// create /rd/run/servicename, write a single 1
// announce service has been started.

#include "config.h"

void usage(){
	writes("Usage: rd.note service\n");
	writes("Announce 'service' has been started\n");
	exit(1);
}


int main(int argc, char *argv[]){
	if (argc < 2) {
		usage();
	}
	char fn[256]; 
	strcpy(fn,SERVICEPATH);
	strlcpy(fn+sizeof(SERVICEPATH)-1,argv[1],256-sizeof(SERVICEPATH));

	int fd = open( fn, O_RDWR|O_CREAT, 0664 );

	if ( fd<0 ){
		exit_errno(fd);
	}
	write(fd,"1",1);
	close(fd);

	exit(0);
}

