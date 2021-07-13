boot
====


In this directory the bootup onetimetasks are stored.


They are executed sequentially, 
and have to start with B or L.

Files, starting with another letter, 
are ignored.
Scripts starting with B are executed first,
scripts starting with L are executed 
when all other scripts, also of the started runlevel,
have finished and the system is up.

In order to start scripts parallel, 
just append the '&' symbol to commands started. 


They are callen by runit's '1' process,
which executes the scripts in this directory,
and waits for the last script in this directory (named S99done.sh) 
to write to the named pipe 'wait.fifo' a single char.

(so process 1 doesn't need to poll any state.
TODO. add a timeout handler)









