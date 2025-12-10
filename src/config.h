// Change the configuration here.

#define RC_BOOT_PATH "/etc/rinit/rc.boot"
//#define RC_BOOT_PATH "./test/"

// The default runlevel,
// when not supplied via kernel / cmdline
#define RC_DEFAULT "default"

#define RC_BASE_PATH "/etc/rinit/rc."

// max len of the kernel's commandline, which is read in
#define CMDLINEMAX 256

//  oth to getty, which is kept open at tty1
#define GETTY "/rd/bin/getty"


#define PATH "/bin:/sbin:/usr/bin:/usr/sbin:/rd/bin"


#define SERVICEPATH "/rd/run/"


// size of the stack of pid 1, env is removed, if present
#define INIT_STACKSIZE 4*4096

#define CHILD_STACKSIZE_CUR 1024*256  // 256 kB
#define CHILD_STACKSIZE_MAX 1024*1024*64  // 64MB


#if 1
#define STAGE1 "./teststage"
#define STAGE2 "./teststage"
#define STAGE3 "./teststage"
//#define STAGE2 "/hd/sda8/home/micha/prog/minilib/minicore/rinit/test2.sh"
//#define STAGE3 "/hd/sda8/home/micha/prog/minilib/minicore/rinit/test3.sh"
#else
#define STAGE1 "/etc/rinit/rinit.boot"
#define STAGE2 "/etc/rinit/rinit.run"
#define STAGE3 "/etc/rinit/3"
#endif

// After signalling the current stage with SIGTERM,
// wait this time in seconds. If the stage still runs,
// kill it with SIGKILL, when still not responding within WAITTIME seconds
// continue anyways
#define WAITTIME 30


// ansi colors
#define RED "\033[31m"
#define YELLOW "\033[33;1m"
#define NORM "\033[0;37m"

#define COLOR_WARNING RED
#define COLOR_ERROR RED
#define COLOR_LOG "\033[1;36m"
#define PREF COLOR_LOG "--- " NORM




