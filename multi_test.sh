make clean
make
./erase_sem
remaining=`ps aux | grep multi_test | grep -v grep | grep -v bash`
if [[ ! -z $remaining ]]; then
    echo "wait for all previous instances to close: $remaining";
    exit 0
fi
NUM_MASTER=`grep "define NUM_MASTER" utils.h | cut -d' ' -f3`
rm -f ./log*.txt
for i in `seq $NUM_MASTER`; do
    rm -f ./log_$i.txt
    ./multi_test 2>&1 1>./log_$i.txt &
done
