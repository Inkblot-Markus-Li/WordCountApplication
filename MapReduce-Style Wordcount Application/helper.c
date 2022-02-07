#include "buffer.h"


// Creates a buffer with the given capacity
state_t* buffer_create(int capacity)
{
    state_t* buffer = (state_t*)malloc(sizeof(state_t));
    buffer->fifoQ = (fifo_t*)malloc(sizeof(fifo_t));
    fifo_init(buffer->fifoQ, capacity);
    buffer->isopen = true;

    if (pthread_mutex_init(&buffer->chmutex, NULL) != 0) {
        printf("Error in initializing mutex variable chmutex\n");
    }
    if (pthread_cond_init(&buffer->chconrec, NULL) != 0) {
        printf("Error in initializing mutex variable chconrec\n");
    }
    if (pthread_cond_init(&buffer->chconsend, NULL) != 0) {
        printf("Error in initializing mutex variable chconsend\n");
    }
    if (pthread_mutex_init(&buffer->chclose, NULL) != 0) {
        printf("Error in initializing mutex variable chmutex\n");
    }
    return buffer;
}


// Writes data to the given buffer
// This is a blocking call i.e., the function only returns on a successful completion of send
// In case the buffer is full, the function waits till the buffer has space to write the new data
// Returns BUFFER_SUCCESS for successfully writing data to the buffer,
// CLOSED_ERROR if the buffer is closed, and
// BUFFER_ERROR on encountering any other generic error of any sort
enum buffer_status buffer_send(state_t* buffer, void* data)
{
    pthread_mutex_lock(&buffer->chmutex);
    if (!buffer->isopen)
    {
        //pthread_cond_wait(&buffer->chconclose,&buffer->chclose);
        //pthread_mutex_unlock(&buffer->chclose);
        pthread_mutex_unlock(&buffer->chmutex);
        return CLOSED_ERROR;
    }

    //pthread_mutex_unlock(&buffer->chmutex);
    //pthread_mutex_unlock(&buffer->chclose);

    int msg_size = get_msg_size(data);

    //pthread_mutex_lock(&buffer->chmutex);
    //pthread_mutex_lock(&buffer->chclose);
    while (fifo_avail_size(buffer->fifoQ) < msg_size) { //while there isnt enough room
        pthread_cond_wait(&buffer->chconrec, &buffer->chmutex); //wait till there is space
        if (!buffer->isopen) {
            pthread_mutex_unlock(&buffer->chmutex);
            //pthread_mutex_unlock(&buffer->chclose);
            return CLOSED_ERROR;
        }
    }

    buffer_add_Q(buffer, data);//add to buffer
    pthread_cond_signal(&buffer->chconsend); //send signal saying its not empty
    pthread_mutex_unlock(&buffer->chmutex);
    //pthread_mutex_unlock(&buffer->chclose);
    return BUFFER_SUCCESS; //return



    //pthread_cond_wait(&buffer->chconrec,&buffer->chmutex); //wait till there is space
    //pthread_mutex_unlock(&buffer->chclose);
    //pthread_mutex_unlock(&buffer->chmutex);
    //return CLOSED_ERROR;

//pthread_cond_wait(&buffer->chconclose,&buffer->chmutex);

//if(pthread_mutex_unlock(&buffer->chmutex) != 0)
       //printf("Error in unlocking chmutex in buffer_send\n");

//return CLOSED_ERROR;
}
// test_send_correctness 1
// Reads data from the given buffer and stores it in the function’s input parameter, data (Note that it is a double pointer).
// This is a blocking call i.e., the function only returns on a successful completion of receive
// In case the buffer is empty, the function waits till the buffer has some data to read
// Return BUFFER_SPECIAL_MESSSAGE for successful retrieval of special data "splmsg"
// Returns BUFFER_SUCCESS for successful retrieval of any data other than "splmsg"
// CLOSED_ERROR if the buffer is closed, and
// BUFFER_ERROR on encountering any other generic error of any sort

enum buffer_status buffer_receive(state_t* buffer, void** data)
{
    if (pthread_mutex_lock(&buffer->chmutex) != 0) {
        printf("Error in locking chmutex in buffer_rec\n");
    }

    if (!buffer->isopen)
    {
        pthread_mutex_unlock(&buffer->chmutex);
        return CLOSED_ERROR;
    }

    while (buffer->fifoQ->avilSize >= buffer->fifoQ->size) { //while the buffer is not empty
        pthread_cond_wait(&buffer->chconsend, &buffer->chmutex); //wait so its not empty
        if (!buffer->isopen) {
            pthread_mutex_unlock(&buffer->chmutex);
            return CLOSED_ERROR;
        }
    }

    buffer_remove_Q(buffer, data); //remove the item

    if (strcmp(*(char**)(data), "splmsg") == 0) //capture special case
    {
        printf("buffer special message in buffer_receive()\n");
        return BUFFER_SPECIAL_MESSSAGE;
    }

    pthread_cond_signal(&buffer->chconrec); //send signal saying that the buffer is not full

    if (pthread_mutex_unlock(&buffer->chmutex) != 0) { //unlock
        printf("Error in unlocking chmutex in buffer_rec\n");
    }

    return BUFFER_SUCCESS; //return


}


// Closes the buffer and informs all the blocking send/receive/select calls to return with CLOSED_ERROR
// Once the buffer is closed, send/receive/select operations will cease to function and just return CLOSED_ERROR
// Returns BUFFER_SUCCESS if close is successful,
// CLOSED_ERROR if the buffer is already closed, and
// BUFFER_ERROR in any other error case
enum buffer_status buffer_close(state_t* buffer)
{
    pthread_mutex_lock(&buffer->chmutex);

    if (!buffer->isopen)
    {
        pthread_mutex_unlock(&buffer->chmutex);
        return CLOSED_ERROR;
    }

    buffer->isopen = false;
    pthread_mutex_unlock(&buffer->chmutex);
    pthread_cond_broadcast(&buffer->chconrec);
    pthread_cond_broadcast(&buffer->chconsend);

    return BUFFER_SUCCESS;

}

// Frees all the memory allocated to the buffer , using own version of sem flags
// The caller is responsible for calling buffer_close and waiting for all threads to finish their tasks before calling buffer_destroy
// Returns BUFFER_SUCCESS if destroy is successful,
// DESTROY_ERROR if buffer_destroy is called on an open buffer, and
// BUFFER_ERROR in any other error case

enum buffer_status buffer_destroy(state_t* buffer)
{
    if (buffer->isopen)
    {
        return DESTROY_ERROR;
    }

    if (pthread_mutex_destroy(&buffer->chmutex) != 0) {
        printf("Error destorying muext variable chmutex\n");
    }
    if (pthread_cond_destroy(&buffer->chconrec) != 0) {
        printf("Error destroying mutex variable chconrec\n");
    }
    if (pthread_cond_destroy(&buffer->chconsend) != 0) {
        printf("Error destroying mutex variable chconsend\n");
    }
    if (pthread_mutex_destroy(&buffer->chclose) != 0) {
        printf("Error destroying mutex variable chconsend\n");
    }
    fifo_free(buffer->fifoQ);
    free(buffer);


    return BUFFER_SUCCESS;
}
