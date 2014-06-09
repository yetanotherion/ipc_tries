make
./erase_sem
NUM_MASTER=`grep "define NUM_MASTER" utils.h | cut -d' ' -f3`
for i in `seq $NUM_MASTER`; do
    ./multi_test &
done
