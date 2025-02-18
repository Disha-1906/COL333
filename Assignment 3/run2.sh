chmod 777 main2
INPUT="$1.graph"
OUTPUT="$1.mapping"
MINOUT="$1.satoutput"
./main2 $MINOUT < $INPUT > $OUTPUT
