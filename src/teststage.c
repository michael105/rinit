#ifdef mlconfig

COMPILE printv prints itofmt memset


return
#endif



int __attribute__((used))main(int argc,char*argv[],char*envp[]){
	printvl("Teststage, pid: ",getpid(),"\nArgs: ",
			FMT(.group=1,.groupsep=' '),argv,
			"\nenvp(1,2,3): ",envp[0]," - ",envp[1]," - ",envp[2]);



	exit(0);
}
