#include "segel.h"
#include "request.h"

// 
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//


/*
* shared memory among all threads
*/
Queue* waitingQueue = NULL;
Queue* runningQueue = NULL;
pthread_mutex_t mutex_lock;
pthread_cond_t cond;
pthread_cond_t cond_master;


// HW3: Parse the new arguments too
void getargs(int *port, int* numOfThreads, int* queueSize, int argc, char *argv[])
{
    if (argc < 2) {
	fprintf(stderr, "Usage: %s <port>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
    *numOfThreads = atoi(argv[2]); // TODO: send error if not positive integer? https://piazza.com/class/lfgtde16jfs1xm/post/410
    *queueSize = atoi(argv[3]); // TODO: send error if not positive integer? https://piazza.com/class/lfgtde16jfs1xm/post/410
}


/*
 *  Function to be run by each thread from threads pool
 */
void* threadRoutine(void* threadNum)
{
    while(1)
    {
        // check if there is work to do
        pthread_mutex_lock(&mutex_lock);
        while (waitingQueue->size == 0) {
            pthread_cond_wait(&cond, &mutex_lock);
        }

        // choose the oldest request from waiting queue to be executed
        int oldestConnfd;
        returnAndRemoveOldest(waitingQueue, &oldestConnfd);
        printf("waiting queue size is: %d\n", waitingQueue->size); //TODO: for debug, delete later
        printf("running queue size is: %d\n", runningQueue->size); //TODO: for debug, delete later
        addToQueue(runningQueue, oldestConnfd);
        printf("running queue size is: %d\n", runningQueue->size); //TODO: for debug, delete later
        pthread_mutex_unlock(&mutex_lock);

        // execute request
        requestHandle(oldestConnfd);
        Close(oldestConnfd);

        // work is finished - remove from running queue
        pthread_mutex_lock(&mutex_lock);
        removeByConnfd(runningQueue, oldestConnfd);
        printf("running queue size is: %d\n", runningQueue->size); //TODO: for debug, delete later
        pthread_mutex_unlock(&mutex_lock);
    }
    return NULL;
};


/*
 * Create threads pool
 */
pthread_t* initThreadsPool(int numOfThreads)
{
    pthread_t* threadsPool = malloc(numOfThreads*sizeof(pthread_t));
    int* threadIDs = malloc(numOfThreads*sizeof(int));
    for (int i=0; i<numOfThreads; i++)
    {
        threadIDs[i] = i;
        pthread_create(&threadsPool[i], NULL, threadRoutine, (void*)&threadIDs[i] );
    }
    free(threadIDs);
    return threadsPool;
}


/*
 * Delete all allocated memory
 */
// TODO: understand when to use the function https://piazza.com/class/lfgtde16jfs1xm/post/410
void destroy(pthread_t* threadsPool)
{
    destroyQueue(waitingQueue);
    destroyQueue(runningQueue);
    free(threadsPool);
}


int main(int argc, char *argv[])
{
    // variables declaration
    int listenfd, connfd, port, clientlen, numOfThreads, queueSize;
    struct sockaddr_in clientaddr;

    // reading arguments
    getargs(&port, &numOfThreads, &queueSize, argc, argv);

    // init for requests queues and threads pool
    runningQueue = initQueue();
    waitingQueue = initQueue();
    pthread_t* threadsPool = initThreadsPool(numOfThreads);

    // creating locks and conditions for later
    pthread_mutex_init(&mutex_lock, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_cond_init(&cond_master, NULL);

    listenfd = Open_listenfd(port);

    // main loop - catch requests and add them to waiting queue for thread execution
    while (1)
    {
	    clientlen = sizeof(clientaddr);
	    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
        pthread_mutex_lock(&mutex_lock); // lock so threads don't acquire requests from queues
        if (waitingQueue->size + runningQueue->size == queueSize)
        {
            // TODO: complete according to policies (part 2 of wet)
        }

	    // HW3: In general, don't handle the request in the main thread.
	    // Save the relevant info in a buffer and have one of the worker threads
	    // do the work.
	    //
        printf("waiting queue size is: %d\n", waitingQueue->size); //TODO: for debug, delete later
        addToQueue(waitingQueue, connfd);
        printf("waiting queue size is: %d\n", waitingQueue->size); //TODO: for debug, delete later
        //signal about it with cond variable
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex_lock);
    }
}






