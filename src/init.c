#ifdef mlconfig
# minilib configuration

COMPILE start,writes,vfork,execve,sleep,exit,waitpid,\
		  sigaction,sigaddset,sigemptyset,sigfillset,raise,setitimer,\
		  reboot,sync,memcpy

# debugging definitions
# COMPILE printf,itodec; mini_buf 256

# GLOBALS onstack

LDSCRIPT text_and_bss

STACK stacksize=512

#SHRINKELF

return
#endif

/*
 misc 2020/06

		rinit init tools
    Copyright (C) 2020-2025  Michael (misc) 

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.



Based on minilib,
Copyright (c) 2012-2025, Michael (misc) 
(www.github.com/michael105)
Donations welcome: Please contact me.
All rights reserved.
The licensing terms of minilib are in the file LICENSE.minilib.
(Opensource, free to use also commercially with attribution)

----------------

 a minimal init
 
 starts /etc/rinit/rinit.boot and hands all over all arguments, invoked with.
 rinit.boot executes all files within /etc/rinit/boot, starting with B,
 in order of the numbering.

 after "rinit.boot" has exited, 
 /etc/rinit/rinit.run is started, and restarted if it should exit.
 rinit.run is supplied with the runlevel to start 
 (defaults to "default", so all files in /etc/rinit/default,
 starting with B, are executed.




 on signals SIGINT, SIGTERM (->shutdown) and Ctrl+Alt+Del (->reboot)
 /etc/rinit/rinit.run is sent SIGTERM, 
 when not responding past WAITTIME seconds SIGKILL,
 and after it's termination
 /etc/rinit/rinit.shutdown is executed.


 SIGQUIT: send the current stage a sigterm.
    when the current stage is stage rinit.run, 
		restart the process after termination


 SIGABRT: cancel a shutdown.
 		if stage 3 is running, send SIGABRT to the process, and restart at stage 1.
		is stage 2 is still running, and a shutdown is in progress,
		send stage 2 SIGABRT. If the process exits restart stage 2
*/
 
#include "config.h"


// global variables, they are placed at the stack and accessed via 
// segment register fs.
// we don't have any input at all, besides plain signals.
// consequently, a buffer overflow just isn't possible.
typedef struct { int shutdown; int stagepid; int zombie; } t_globals;

// get segment register prefix
static inline __seg_fs t_globals* __attribute__((always_inline,no_instrument_function))GLOBAL(){
	return(0);
}

// set segment register
static void __attribute__((no_instrument_function))setglobals(t_globals* ml){
	arch_prctl(ARCH_SET_FS,ml);
}

#ifdef GLOBALS
#undef GLOBALS
#endif

#define GLOBALS GLOBAL()


// main log function
void ___log(const char *pref, int preflen, const char *msg, int msglen){
	char buf[preflen+sizeof(NORM)+msglen+4];

	memcpy(buf,pref,preflen);
	int e = preflen;
	memcpy(buf+e,NORM,sizeof(NORM));
	e+=sizeof(NORM);
	memcpy(buf+e,msg,msglen);
	e+=msglen;
	buf[e] = '\n';

	write(1,buf,e+1);
}

#define __log(_pref,_msg,_len) ___log(_pref,sizeof(_pref),_msg,_len)

void _error(const char *msg,int len){
	__log( COLOR_ERROR "--- init: ERROR: ", msg, len );
}

void _warning(const char *msg, int len){
	__log( COLOR_WARNING "--- init: WARNING: ", msg, len );
}

void _log(const char *msg, int len){
	__log( COLOR_LOG "--- " NORM "init: ", msg, len );
}

// logging definitions with literal strings
#define error(_msg) _error(_msg,sizeof(_msg))
#define warning(_msg) _warning(_msg,sizeof(_msg))
#define log(_msg) _log(_msg,sizeof(_msg))


// set a timer, which calls sigalarm
void settimer(int secs){
	struct itimerval timer = { 
		.it_value.tv_sec  = secs,
		.it_value.tv_usec = 0 };
	setitimer (ITIMER_REAL, &timer, 0);
}

// handle shutdown and reboot
void sighandler(int signal){
	if ( signal == SIGTERM ){
		GLOBALS->shutdown = 1; // halt
		log("Shutdown");
	}
	if ( signal == SIGINT ){
		GLOBALS->shutdown = 2; // reboot
		log("Reboot");
	}

	kill(GLOBALS->stagepid,SIGTERM);

	// set a timer, 
	// and kill the curent stage, 
	// when init is still running after WAITTIME seconds
	settimer(WAITTIME);
}

// timeout after signalling the currently running stage
void sigalarm(int signal){
	if ( GLOBALS->shutdown ){
		warning("Shutdown: timeout reached. Send kill.");
		kill( GLOBALS->stagepid, SIGKILL);
		if ( GLOBALS->zombie == GLOBALS->stagepid ){ // stage process hangs, didn't respond to sigkill
			raise(SIGTERM); // kill ourselves / meaning continue in vexec, waitpid
		}
		GLOBALS->zombie = GLOBALS->stagepid; // save stagepid.
		settimer(WAITTIME); // when the stage process doesn't respond to the sigkill,
		// kill ourselves after "waittime"
	}
}

// abort a shutdown
void sigabrt(int signal){
	if ( GLOBALS->shutdown ){
		GLOBALS->shutdown = 0;
		warning("Abort shutdown");
		kill(GLOBALS->stagepid, SIGABRT);
	}
}

// execute 'exec' and wait for it's termination
// reap all children
int vexec( const char* exec, char* const* argv, char* const* envp ){
	GLOBALS->stagepid = vfork();

	if ( GLOBALS->stagepid == 0 ){
		execve(exec, argv, envp );
		error("Couldn't execute");
		error(exec);
		sleep(3);
		exit(1); // exit with failure
	}

	int ws;
	int pid;
	// the main loop, while running a stage ( pid != stagepid )
	do {
		pid = waitpid( -1, &ws, 0 ); // wait for any child (reap zombies)
	} while ( !( 
				( (pid == GLOBALS->stagepid) && (WIFEXITED(ws) || WIFSIGNALED(ws) ) ) 
				|| GLOBALS->zombie ) );

	return(0);
}



int __attribute__((used)) main(int argc, char **argv, char **envp){

	log("start init");

	
	// shrink the stack, get rid of all environmental variables
	// this spares about 100 kB of runtime memory usage
	//
	struct rlimit rl = { 
		.rlim_cur=INIT_STACKSIZE,
		.rlim_max=INIT_STACKSIZE
	};

	if ( *envp != 0 ){

		int ret = 0;
		ret = setrlimit(RLIMIT_STACK,&rl);
		if ( ret==0 ){
			// there is no input, so there aren't any buffer ovflows possible,
			// we can spare the random bytes at the stack
			personality( ADDR_NO_RANDOMIZE );
			log("process restart");
			execve(*argv,argv,0);
		}
		error("self restart/setrlimit failed");
	} 

	// restore default personality and configurated stacksize 
	// for started childs
	rl.rlim_cur=CHILD_STACKSIZE_CUR;
	rl.rlim_max=CHILD_STACKSIZE_MAX;

	int ret = 0;
	ret = setrlimit(RLIMIT_STACK,&rl);
	if ( ret!=0 ){
		warning("cannot set stacksize for children");
	}

	personality( 0 );

	// allocate and initiate global vars
	t_globals globals = {0};
	setglobals(&globals);

	// install signal handlers
	struct sigaction sa;

	sigfillset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = sighandler;

	if ( sigaction (SIGTERM, &sa, 0) ||
			sigaction (SIGQUIT, &sa, 0) ||
			sigaction (SIGINT, &sa, 0) ){
		error("Couldn't install signal handler");
		// try to continue anyways.
	}

	sa.sa_handler = sigalarm;
	if ( sigaction (SIGALRM, &sa, 0) ){
		error("Couldn't install alarm handler");
	}

	sa.sa_handler = sigabrt;
	if ( sigaction (SIGABRT, &sa, 0) ){
		error("Couldn't install sigabrt handler");
	}

	// Ctrl-Alt-Del: send sigint to init, but do not reboot
	reboot(LINUX_REBOOT_MAGIC1,LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_CAD_OFF,0);

	while (1){
		// stage 1
		GLOBALS->zombie = 0;
		log("Run " STAGE1);
		vexec( STAGE1, argv, envp );

		int a = 0;
		char *st2av[3] = { STAGE2, "                ",0 };

		// stage 2
		while (!GLOBALS->shutdown){
			log("Run " STAGE2);
			vexec(STAGE2, st2av, envp);
			if ( (a++) > 1 ){ // prevent spinning 
				warning("stage2 seems to die unexpectedly\nSleeping");
				a=0;
				sleep(5);
			}
		};

		// stage 3
		if ( GLOBALS->shutdown==1 )
			log("Shutdown");
		else
			log("Reboot");

		log("Run " STAGE3);
		settimer(WAITTIME);
		GLOBALS->zombie = 0;
		vexec(STAGE3, argv, envp);

		log("Sync remaining file systems");
		sync();

		// shutdown
		if ( GLOBALS->shutdown == 1 ){
			log("Power off");
			sync();
			sync();
			reboot(LINUX_REBOOT_MAGIC1,LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_POWER_OFF,0);
			int ret = reboot(LINUX_REBOOT_MAGIC1,LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_HALT,0);
			error("Shutdown failed");
			exit(ERRNO(ret));
		}

		if ( GLOBALS->shutdown == 2 ){
			log("Reboot");
			sync();
			sync();
			int ret = reboot(LINUX_REBOOT_MAGIC1,LINUX_REBOOT_MAGIC2, LINUX_REBOOT_CMD_RESTART,0);
			error("Reboot failed");
			exit(ERRNO(ret));
		}

		// ( shutdown == 0 )
	}; // shutdown aborted. start with stage1 again

	__builtin_unreachable(); // silence compiler warning
}

