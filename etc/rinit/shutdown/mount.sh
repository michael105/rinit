
function mounted(){
	#echo `mount | sed -nE '/on (.dev)|(.proc)|(.rd.proc)|(.rd)|(.rd.mnt) /d; s/^\S* on (\S\S\S*) type .*/\1/p' | sort -r`
	echo -n `sed -E '/^\S*( \/ )|( \/ )|( \/dev )|( \/rd)|( \/proc )|( \/rd\/mnt)/d;s/^\S* (\S*).*/(echo "\1"|wc -c|tr "\n" " ";echo \1) /e' /proc/mounts |\
			sort -nr | uniq | sed 's/^\S* //'`
}
