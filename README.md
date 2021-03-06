work in progress, not finished.

Documentation needs to be (re)written,
the init scripts are subject to changes.

Tested only with gcc 9.3.0.


I'm using this init however myself for more than one year.

I guess, I finish this when I'm happy with the minimal linux system
I'm working at.

Might currently only compile with gcc 9.3 or clang 11.0. 
Only gcc 9.3 (Gentoo) has been tested by me.


#### A minimal init. 

progress: boottime is around 4 seconds from grub to the fully up and running system,
running Xorg, i3, http proxy server, ssh server, dns proxy, separation into several containers,
and kerberos. (kerberos for gaining root/sudo, the default user is 'autologged' in)
Currently waiting for the USB bus to initialize,
and starting Xorg takes most time. 
I did already have a responsible system within about 2 seconds,
but it is work in progress.
(15 year old notebook, Thinkpad t61 with a ssd)

Albite most of the startup time is needed by the bios (maybe 3seconds here),
I'm still aiming at 1 second from grub to a running Xserver including the desktop environment.


shutdown maybe 1/10 second. 
(terminating a list of programs like vi, to save data to disk,
unmounting the harddisk,but leaving other programs running)



##### About

The only "dependency" minilib.h is included.
(www.github.com/michael105/minilib)

The small size (2.2kB) and using vfork do spare some resources.
Especially, when considering context switches for reaping subprocesses.
(Everytime, a child process exits, there's a task switch to init,
in order to "reap" the children's process state.
Having a tiny init (or another subreaper process) has real and enduring performance advantages,
also energy savings. 

Having a statically linked init has further advantages. 
(contigous memory, easy to setup in containers,..)

Readahead could be implemented by the stages,
but didn't speed up here. (Using 'lockfile' from minicore, ssd)

I'm heavily tempted to rule sort of an init language.
Or something like usable routines for C.
Atm the kernel's init phase takes most time.
However, I'd like to have the system ready within <1 second.
(Ready in the meaning of, the X server is started and ready for input)

Yet this might be around 4 seconds, maybe five.

Harddisk mounts, and module loads are done in the background,
after X and the desktop manager(i3) has been started.

As soon as this get's closer to 1 second, it will be possible 
to measure the script parsing overhead.
Atm, it is neglectible.


##### Description of the boot process

(Subject to change, all logging is dumped to the console,
..)

For now, init starts 
/etc/rinit/rd.boot
which runs all scripts in /etc/rinit/rc.boot, starting with 'B'.

Afterwards, 
/etc/rinit/rd.run runlevel
is executed and all scripts in /etc/rinit/rc.runlevel,
starting with 'B' are executed.

B99Lazy waits for the file /tmp/runlazy,
when this has been created (I touch it from the config file of i3),
all scripts, starting with 'L', are executed in order.

Halt/reboot is /etc/rinit/3

The stages' names are defined in config.h


on signals SIGINT, Ctrl+Alt+Del (->reboot) and SIGTERM (->shutdown)
/etc/rinit/rd.run is sent SIGTERM, when not responding SIGKILL,
and after it's termination
/etc/rinit/3 is executed.
`3` is currently a script, which scans /etc/rinit/rc.shutdown
and executes all scripts starting with S.

For a shutdown do "killall init",
for a reboot "killall -s SIGINT init" (or Ctrl-Alt-Del)
(Or use the included programs 'halt' and 'reboot').

When sent SIGQUIT, the currently running stage is signalled with SIGTERM.
When the currently running stages are either 1 or 2, when the processes exit,
stage 2 is (re)started.

On shutdown, after a timeout of (default, change "WAITTIME" within the source) 
30 seconds past the sent SIGTERM,
the currently running stage is sent a SIGKILL.
When still not responding within 30 seconds, 
it's assumed the process is zombified and stage 3 or
the reboot/halt executed.


The init scripts I'm using, are working, but subject to change.
I'm using a stripped down sh interpreter of busybox.

However, the forks for every executed program on boot seem to have their price.
Albite bootup time is below 1 second. (Notebook Amd Turion 2GHz, built 2005)

The bootscripts aren't tidied, there are some instructions left, 
I fiddled around with syncronization.
Showed up, no sync is needed.
To be more exact, even stage 2 is not really needed,
it is yet mainly there to keep a login at tty1 open.
I leave this as it is for now.


##### Unfinished stuff

No proper install.

busybox isn't included, 
yet it doesn't compile with minilib.
However the sh of busybox has the big advantage of 
having most external programs included,
sparing the fork to execute commands in the scripts.

Not possible yet to include other init systems.
Just plain shell scripts.


##### notes

Seems to me, a modular kernel would be advantegeous in matters of boottime.

the modules can initialize the hardware in parallel to the boot process.

(Now, the kernel needs about x seconds to initialize here)

Ok. Modularizing the kernel, and loading
all modules in parallel saves a lot of time.


scripts/process starters: async, delayed, lazy.

Start(run)   Boot
Service      Daemon
Shutdown     Halt

this might change, atm I do see the need for these class properties on boot:

immediate execution,
parallel execution,
needed for X,
can be delayed after X up and running,
lazy (not needed for user input),
daemonized.



---- 



rinit.run needs one argument, 
empty spaces.

On boot, rinit.run reads the kernel command line
and looks for a parameter runlevel= 
When found, or 
when the runlevel is changed,
 rinit.run writes into it's own argument the new runlevel.
 (so the current runlevel shows up with ps)

(Not yet) 
On SIGUSR1 the runlevel should be read from the shared directory within the init ramdisk,
and changed to, when needed.
 (/rd/run/runlevel)

 For changing the runlevel, 
 signal either init (1) or rinit.run with SIGUSR1





rinit.shutdown scans and runs rc.shutdown,
as well as rc.runlevel/S\*




In my opinion, having a small codebase, also read by other people, 
is the biggest plus in the question of security.

Especially for processes like init.

Using minilib is a big advantage, since things like utf8 or localizations
aren't useful at all for init, or the supervisor scripts/binaries.


For having all used sources shown, simply type SHOWSOURCES=1 mini-gcc --config 'prog.c' 'prog.c'.

The other advantage (I guess) is using clang.
Gcc since version 10 quite often creates segfaulting binaries.
I didn't find the cause yet. But might be some dark magic somewhere.




		rinit init tools
    Copyright (C) 2020,2021  Michael (misc) Myer

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
Copyright (c) 2012-2021, Michael (misc) Myer
(misc.myer@zoho.com, www.github.com/michael105)
Donations welcome: Please contact me.
All rights reserved.
The licensing terms of minilib are in the file LICENSE.minilib.
(Opensource, free to use with attribution)




