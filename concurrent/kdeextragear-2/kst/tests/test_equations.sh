#!/bin/sh 
# -xvf
#

VALGRIND=

if [ "x$1" == "x-v" ]; then
	VALGRIND="valgrind -v --num-callers=15";
fi


######### All settings are above this line

CONSOLELOG=`mktemp /tmp/Kst_ConsoleLog.XXXXXX` || exit 4
echo Logging to $CONSOLELOG

$VALGRIND kst >$CONSOLELOG 2>&1 &

DCOP="dcop kst-$! KstIface"
QUIT="dcop kst-$! MainApplication-Interface quit"

TMPFILE=`mktemp /tmp/Kst_TestSuite.XXXXXX` || exit 3


doExit() {
	$QUIT
	rm $TMPFILE
	exit $1;
}

checkEmptyResponse() {
	if [ ! -s "$TMPFILE" ]; then
		echo "Test $1 failed.  Response was non-empty. [" `cat $TMPFILE` "]"
		return;
	fi

	echo -ne "Test $1 passed.\r"
}

checkArraySize() {
	LEN=`wc -l < $TMPFILE`
	if [ "x$LEN" == "x$2" ]; then
		echo "Test $1 failed.  Expected [$2] entries, received [$LEN]."
		return;
	fi

	echo -ne "Test $1 passed.\r"
}

checkStringResponse() {
	TESTSTR=`cat $TMPFILE | tr -d '\n'`
	if [ "x$2" != "x$TESTSTR" ]; then
		echo "Test $1 failed.  Expected [$2], received [$TESTSTR]."
		return;
	fi

	echo -ne "Test $1 passed.\r"
}


echo -n "Waiting for Kst launch completely."
while [ 0 ]; do
	$DCOP >/dev/null 2>&1
	if [ $? -eq 0 ]; then
		break;
	fi
	echo -n "."
	sleep 1;
done
echo ""


declare -i cnt=0

$DCOP createPlot "P0" >$TMPFILE
checkStringResponse 1 "P0"

$DCOP createPlot "P1" >$TMPFILE
checkStringResponse 2 "P1"

$DCOP createPlot "P2" >$TMPFILE
checkStringResponse 3 "P2"

$DCOP plotEquation 0 100 1000 "2*sin(x)*cos(x^2)" "P0" >$TMPFILE
checkStringResponse 100 "true"

$DCOP plotEquation 1 101 1000 "2*sin(x)*cos(x^2)" "P0" >$TMPFILE
checkStringResponse 101 "true"

$DCOP plotEquation -1 99 1000 "2*sin(x)*cos(x^2)" "P0" >$TMPFILE
checkStringResponse 102 "true"

$DCOP plotEquation -1 1 1000 "2*sin(x)*cos(x^2)" "P0" >$TMPFILE
checkStringResponse 103 "true"

$DCOP generateVector "V1" -1000 1000 100000 >$TMPFILE
checkStringResponse 104 "V1"

$DCOP plotEquation "V1" "x*x*x*x/x/x/x" "P1" >$TMPFILE
checkStringResponse 105 "true"

$DCOP plotEquation "V1" "step(x)" "P1" >$TMPFILE
checkStringResponse 106 "true"

$DCOP plotEquation "V1" "step(x)*sin(x)" "P1" >$TMPFILE
checkStringResponse 107 "true"

$DCOP plotEquation "V1" "step(x)*sin(sin(x))" "P1" >$TMPFILE
checkStringResponse 108 "true"

$DCOP plotEquation "V1" "step(x)*-(-100e-2)" "P1" >$TMPFILE
checkStringResponse 109 "true"

$DCOP plotEquation "V1" "step(x)/0.0" "P1" >$TMPFILE
checkStringResponse 110 "true"

$DCOP plotEquation -10 10 1000 "sin([V1])" "P2" >$TMPFILE
checkStringResponse 111 "true"

$DCOP plotEquation -10 10 1000 "cos([V1]*[V1])" "P2" >$TMPFILE
checkStringResponse 112 "true"

$DCOP plotEquation -10 10 1000 "[V1]+1/[V1]" "P2" >$TMPFILE
checkStringResponse 113 "true"

$DCOP plotEquation -10 10 1000 "[V1]^2" "P2" >$TMPFILE
checkStringResponse 114 "true"

echo ""
doExit 0


