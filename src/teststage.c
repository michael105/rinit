#ifdef mlconfig

COMPILE printv prints itofmt memset sleep


return
#endif



int __attribute__((used))main(int argc,char*argv[],char*envp[]){
	writes("\033[33;1mteststage\n");
	printvl("\033[0;33mTeststage, pid: ",getpid(),"\nArgs: ",
			FMT(.group=1,.groupsep=' '),argv);
	if ( *envp )
			printsl("envp(1,2,3):\n ",envp[0],"\n ",envp[1],"\n ",envp[2]);
	else 
		writesl("*envp == 0");


	int s = 0;
	if ( argc > 1 ){
		for ( char *p = argv[1]; isdigit( *p ); p++ )
			s = s*10 + *p - '0';
	}

	printvl( "sleep: ",s);
	sleep(s);

	exit(0);
}
