#issue:
# need to wait for all to be idle:
# semaphore with 0 or negative 1 - NUM_MASTER  initial value (i.e. - (NUM_MASTER - 1 ))
# code:
# release does not work as the lock will be removed after the release.
# if acquire not false:
#  implement shift
# else
#   acquire

NUM_MASTER=100
# p1:
# release
#  (=0)
#  acquire(block)
#
# p2:
# release



# https://docs.python.org/3.5/library/threading.html#barrier-objects barrier, releases all semaphores simultaneously not supported in multiprocessing

# acquire:

# else:



from multiprocessing import Process, Manager

class ShiftQueue(object):
    def __init__(self, queueLock, idleSemaphore, waitingEvent, queue, time, i):
        self.queueLock = queueLock
        self.idleSemaphore = idleSemaphore
        self.waitingEvent = waitingEvent
        self.queue = queue
        self.time = time
        self.i = i

    def setShift(self, element):
        self.queueLock.acquire()
        self.queue.append(element)
        self.queue.sort()
        self.queueLock.release()

    def implementShift(self):
        res = 0
        if self.queue:
            nextVal = self.queue.pop(0)
            toRemove = []
            for i, x in enumerate(self.queue):
                newVal = x - nextVal
                self.queue[i] = newVal
                if newVal <= 0:
                    toRemove.append(newVal)
            for j in toRemove:
                self.queue.remove(j)
            self.queue.sort()
            res = nextVal
        return res

    def getTime(self):
        return self.time[0]

    def setIdle(self):
        weShouldDoTheJob = not self.idleSemaphore.acquire(False)
        if weShouldDoTheJob:
            self.time[0] = self.time[0] + self.implementShift()
            # let's reinit the idleSemaphores
            for i in range(NUM_MASTER - 1):
                self.idleSemaphore.release()
            # and release other threads
            self.waitingEvent.set()
        else:
            if self.waitingEvent.is_set():
                self.waitingEvent.clear()
            self.waitingEvent.wait()

def f(queueLock, idleSemaphore, waitingEvent, queue, time, i):
    s = ShiftQueue(queueLock, idleSemaphore, waitingEvent, queue, time, i)
    s.setShift(3)
    s.setIdle()
    assert(s.getTime() == 3)
    s.setShift(9)
    s.setShift(4)
    s.setIdle()
    assert(s.getTime() == 7)
    s.setIdle()
    assert(s.getTime() == 12)
    return

if __name__ == '__main__':
    manager = Manager()

    queueLock = manager.Lock()
    idleSemaphore = manager.Semaphore(NUM_MASTER - 1)
    waitingEvent = manager.Event()
    queue = manager.list()
    time = manager.list([0])
    processes = [Process(target=f, args=(queueLock, idleSemaphore, waitingEvent, queue, time, i)) for i in range(0, NUM_MASTER)]
    for p in processes:
        p.start()
    for p in processes:
        p.join()
