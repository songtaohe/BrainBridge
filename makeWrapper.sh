#!/bin/bash
if [ -f $1real ]; then
	echo wrapper exists!
	exit
fi
echo "Target $1"
mv $1 "$1real"
echo "#!/bin/bash" > $1
echo "DIR=\`pwd\`" >> $1
echo "$1real \`compilerWrapper \$DIR \$@ \` " >> $1 
chmod a+x $1


