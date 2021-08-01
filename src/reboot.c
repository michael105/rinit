#if 0
COMPILE start kill
LDSCRIPT textandbss
INCLUDESRC

return
#endif

int main(){
	return( - kill(1,SIGINT) );
}
