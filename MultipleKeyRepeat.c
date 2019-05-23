#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <pthread.h> 

#define die(str, args...) do { \
perror(str); \
exit(EXIT_FAILURE); \
} while(0)

#define INITIAL_DELAY 200*1000
#define REPEAT_DELAY 20*1000
#define BUFFER_SIZE 10

int initial_delay = INITIAL_DELAY;
int repeat_delay = REPEAT_DELAY;
int rapid_fire_AcTiVaTeD = 0;

void emit(int fd, int type, int code, int val)
{
    struct input_event ie;
    
    ie.type = type;
    ie.code = code;
    ie.value = val;
    /* timestamp values below are ignored */
    ie.time.tv_sec = 0;
    ie.time.tv_usec = 0;
    
    write(fd, &ie, sizeof(ie));
}

pthread_mutex_t myMutex;

pthread_cond_t ourCondition;

pthread_mutex_t myIntialMutexou;

typedef struct {
    int    myChar;
    int    fd;
    int *  myCond;
} myStruct;


typedef struct myQueue myQueue;

struct myQueue{
    myQueue * prev;
    int    val;
    myQueue * next;
} ;





typedef struct {
    long unsigned int timetable[BUFFER_SIZE];
    myQueue* keyvalues[BUFFER_SIZE];
    myQueue ** pointers[KEY_MAX];
    int a;
    int da;
} sliding_buffer;

typedef struct {
    int    fd;
    int *  conds;
    myQueue * q;
    sliding_buffer * mySB;
} myStructWithQ;

void sliding_buffer_init(sliding_buffer* mySB){
    mySB->a = 0;
    mySB->da = 0;
    for(int i = 0; i < KEY_MAX; i++){
        mySB->pointers[i] = NULL;
        
    }
    for(int i = 0; i < BUFFER_SIZE; i++){
        mySB->keyvalues[i] = NULL;
    }
}


myQueue* enqueue(myQueue* q, myQueue* q2add){
    
    q2add->next = NULL;
    q2add->prev = NULL;
    myQueue * endofq = q;
    if(q == NULL){
        q = q2add;
    }
    else{
        while(endofq->next != NULL){
            //printf("infinite Loop Checker 1 \n");
            endofq = endofq->next;
        }
        endofq->next = q2add;
        q2add->prev = endofq;
    }
    return q;
    
}
myQueue* delspesitem(myQueue *qStart, myQueue* q2del){
    if(q2del == NULL){
        return qStart;
    }
    //if(q2del == qStart){
    //    qStart->next->prev = NULL;
    //    return qStart->;
    //}
    myQueue* tmpprev;
    myQueue* tmpnext;
    
    
    tmpprev = q2del->prev;   
    tmpnext = q2del->next;
    
    myQueue *rtn = NULL;
    if(q2del->prev != NULL){
        q2del->prev->next = tmpnext;
        rtn = qStart;
    }
    
    if(q2del->next != NULL){
        q2del->next->prev = tmpprev;
        if (rtn == NULL){
            rtn = q2del->next;
        }
    }
    return rtn;
    
    
    
}
myQueue * findInQueue(myQueue * q, int valeur){
    while(q != NULL){
        if(q->val == valeur){
            //printf("fait Mon TAf \n");
            return q;
        }
        q = q->next;
    }
    return NULL;
}

int queueLength(myQueue * q){
    //pthread_mutex_lock(&myMutex);
    int rtn = 0;
    while(q != NULL){
        //printf("infinite Loop Checker 7 \n");
        rtn++;
        q = q->next;
    }
    //pthread_mutex_unlock(&myMutex);
    return rtn;
}

void* myThread(void* myVoidPointer){
    myStruct *structo = myVoidPointer;
    usleep(initial_delay);
    while(*(structo->myCond)){
        pthread_mutex_lock(&myMutex);
        emit(structo->fd, EV_KEY, structo->myChar, 1);
        emit(structo->fd, EV_SYN, SYN_REPORT, 0);
        emit(structo->fd, EV_KEY, structo->myChar, 0);
        emit(structo->fd, EV_SYN, SYN_REPORT, 0);
        pthread_mutex_unlock(&myMutex);
        usleep(repeat_delay);
        //printf("infinite Loop Checker 3 \n");
    }
    
    return NULL;
    
}


void* myTheardUnique(void* myVoidPointer){
    myStructWithQ *structo = myVoidPointer;
    sliding_buffer* mySB = structo->mySB;
    while(1){
        pthread_mutex_lock(&myIntialMutexou);
        pthread_cond_wait(&ourCondition, &myIntialMutexou);
        pthread_mutex_unlock(&myIntialMutexou);
        //printf("retest lets wait \n");
        //pthread_mutex_unlock(&myIntialMutexou);
        //printf("check0 \n");
        unsigned long int time2sleep = initial_delay;
        unsigned long int currtime;
        pthread_mutex_lock(&myMutex);
        while(mySB->da >0){
            //printf("check0,5 \n");
            pthread_mutex_unlock(&myMutex);
            usleep(time2sleep);
            pthread_mutex_lock(&myMutex);
            //printf("check1 \n");
            if(mySB->keyvalues[structo->mySB->a] != NULL){
                //printf("STAAARTEEED \n");
                
                while(mySB->da >0){
                    
                    if(mySB->keyvalues[structo->mySB->a] != NULL){
                        //printf("enQ \n");
                        structo->q = enqueue(structo->q, mySB->keyvalues[structo->mySB->a]);
                        mySB->keyvalues[structo->mySB->a] = NULL;
                    }
                    mySB->a = (mySB->a + 1)%BUFFER_SIZE;
                    mySB->da--;
                    
                }
                //we start rapid FIYA
                //printf("ACTIVATE \n");
                rapid_fire_AcTiVaTeD = 1;
                while(queueLength(structo->q)){
                    //printf("not NULL I guess \n");
                    myQueue *qU = structo->q;
                    while(qU != NULL){
                        
                        //printf("we are in important part \n");
                        
                        if(structo->conds[qU->val]){
                            emit(structo->fd, EV_KEY, qU->val, 1);
                            emit(structo->fd, EV_SYN, SYN_REPORT, 0);
                            emit(structo->fd, EV_KEY, qU->val, 0);
                            emit(structo->fd, EV_SYN, SYN_REPORT, 0);
                            qU = qU->next;
                        }
                        else{
                            //structo->q = delspesitem(qU);
                            qU = qU->next;
                        }
                        
                    }
                    pthread_mutex_unlock(&myMutex);
                    usleep(repeat_delay);
                    pthread_mutex_lock(&myMutex);
                }
                rapid_fire_AcTiVaTeD = 0;
                //printf("DEACTIVATED \n");
                
                
            }
            
            currtime = mySB->timetable[mySB->a];
            
            while(mySB->keyvalues[structo->mySB->a] == NULL && mySB->da >0){
                //printf("check2 \n");
                mySB->a = (mySB->a + 1)%BUFFER_SIZE;
                mySB->da--;
                //printf("check3 \n");
            }
            //printf("check4 \n");
            if(mySB->keyvalues[structo->mySB->a] != NULL && mySB->da >0){
                time2sleep =  mySB->timetable[mySB->a] - currtime;
            }
        }
        pthread_mutex_unlock(&myMutex);
        
        
        
    }
}


int main(int argc, char* argv[])
{
    int                    fdo, fdi;
    struct uinput_user_dev uidev;
    struct input_event     ev;
    int                    i;
    
    sleep(1);
    
    printf("prout \n");
    if(argc < 2) die("error: specify input device");
    
    char* fdiarg;
    
    int argcounter = 1;
    while(argcounter < argc){
        if(!strcmp("-d", argv[argcounter]) || !strcmp("--initial-delay", argv[argcounter])){
            //printf("clecaca 1 \n");
            argcounter++;
            if(argcounter < argc){
                initial_delay = atoi(argv[argcounter]) * 1000;
            }
            else{
                printf("NOT VALID ARGS \n");
                return 1;
            }
        }
        else if(!strcmp("-r", argv[argcounter]) || !strcmp("--repeat-rate", argv[argcounter])){
            //printf("clecaca 2 \n");
            argcounter++;
            if(argcounter < argc){
                repeat_delay = 1000000 / atoi(argv[argcounter]);
            }
            else{
                printf("NOT VALID ARGS \n");
                return 1;
            }
        }
        else{
            fdiarg = argv[argcounter];
        }
        argcounter++;
    }
    
    
    fdo = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if(fdo < 0) die("error: open");
    
    fdi = open(fdiarg, O_RDONLY);
    if(fdi < 0) die("error: open");
    
    if(ioctl(fdi, EVIOCGRAB, 1) < 0) die("error: ioctl");
    
    if(ioctl(fdo, UI_SET_EVBIT, EV_SYN) < 0) die("error: ioctl");
    if(ioctl(fdo, UI_SET_EVBIT, EV_KEY) < 0) die("error: ioctl");
    if(ioctl(fdo, UI_SET_EVBIT, EV_MSC) < 0) die("error: ioctl");
    
    for(i = 0; i < KEY_MAX; ++i){
        if(ioctl(fdo, UI_SET_KEYBIT, i) < 0) die("error: ioctl");
    }
    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "uinput-sample");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x1;
    uidev.id.product = 0x1;
    uidev.id.version = 1;
    
    if(write(fdo, &uidev, sizeof(uidev)) < 0) die("error: write");
    if(ioctl(fdo, UI_DEV_CREATE) < 0) die("error: ioctl");
    
    int ij= 3000000;
    
    int truth[KEY_MAX] = {0};
    
    for(int j = 16; j <= 25; j++){
        truth[j] = 1;
    }
    for(int j = 30; j <= 41; j++){
        truth[j] = 1;
    }
    for(int j = 44; j <= 53; j++){
        truth[j] = 1;
    }
    truth[KEY_SPACE] = 1;
    truth[KEY_BACKSPACE] = 1;
    
    int data[KEY_MAX] = {0};
    myStruct structArray[KEY_MAX];
    pthread_t myLittleThreads[KEY_MAX];
    
    if (pthread_mutex_init(&myMutex, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }
    
    pthread_t myUniqueThreddou;
    
    myStructWithQ structor;
    structor.fd =fdo;
    structor.conds = data;
    structor.q = NULL;
    
    pthread_create(&(myUniqueThreddou),NULL, myTheardUnique, &(structor));
    
    myQueue myQArray[KEY_MAX];
    
    
    sliding_buffer mySlidingBuffer;
    
    sliding_buffer_init(&mySlidingBuffer);
    structor.mySB = &mySlidingBuffer;
    
    
    while(ij > 0)
    {
        if(read(fdi, &ev, sizeof(struct input_event)) < 0)
            die("error: read");
        
        //ev.time.tv_sec = 0; //TODO check if can actually do this..
        //ev.time.tv_usec = 0;
        //printf("The value is %d\n", ev.time.tv_sec);
        //printf("The value is %d\n", ev.time.tv_usec);
        //if(ev.value != 2)
        //printf("The value is %d\n", ev.value);
        //printf("The code is %d\n", ev.code);
        //printf("The type is %d\n", ev.type);
        if(ev.type == EV_KEY && ev.value != 2){
            pthread_mutex_lock(&myMutex);
            if(write(fdo, &ev, sizeof(struct input_event)) < 0) die("error: write");
            emit(fdo, EV_SYN, SYN_REPORT, 0);
            if(ev.value >= KEY_MAX){
                printf("WTF on va crever \n");
                return 1;
            }
            if(truth[ev.code]){
                
            
                if(ev.value == 1){
                    data[ev.code] = 1;
                    if(!rapid_fire_AcTiVaTeD && mySlidingBuffer.pointers[ev.code] == NULL && mySlidingBuffer.da < BUFFER_SIZE){
                        int tmp = (mySlidingBuffer.a + mySlidingBuffer.da)%BUFFER_SIZE;
                        myQArray[ev.code].val = ev.code;
                        mySlidingBuffer.keyvalues[tmp] = &(myQArray[ev.code]);
                        mySlidingBuffer.pointers[ev.code] = &(mySlidingBuffer.keyvalues[tmp]);
                        mySlidingBuffer.timetable[tmp] = (ev.time.tv_sec%1000000)*1000000 + ev.time.tv_usec;
                        mySlidingBuffer.da++;
                        pthread_cond_signal(&ourCondition); //TODO to uncomment
                    }    
                    
                    if(rapid_fire_AcTiVaTeD){
                        myQArray[ev.code].val = ev.code;
                        structor.q = enqueue(structor.q, &(myQArray[ev.code]));
                        //structor.q = &myQArray[ev.code];
                        //printf("we Enqueued the shitte1 \n");
                        //printf("The value is %d\n", structor.q->val);
                        //pthread_mutex_trylock(&myIntialMutexou);
                        //pthread_mutex_unlock(&myIntialMutexou);
                         
                        //printf("we aren't blocked at mutex");
                        //structArray[ev.code].myChar = ev.code;
                        //structArray[ev.code].fd = fdo;
                        //structArray[ev.code].myCond = &(data[ev.code]);
                        //pthread_create(&(myLittleThreads[ev.code]),NULL, myThread, &(structArray[ev.code]));
                    }
                }
                else if(ev.value == 0){
                    data[ev.code] = 0;
                    
                    if(rapid_fire_AcTiVaTeD){
                        structor.q = delspesitem(structor.q, findInQueue(structor.q, ev.code));
                    }
                    if((mySlidingBuffer.pointers[ev.code]) != NULL){
                        (*(mySlidingBuffer.pointers[ev.code])) = NULL;
                        (mySlidingBuffer.pointers[ev.code]) = NULL;
                    }
                    
                    
                    
                    
                }
                else{
                    printf("The value is %d\n", ev.value);
                }
            }
            //else{printf("not truth.. \n");}
            pthread_mutex_unlock(&myMutex);
            
            //emit(fdo, EV_KEY, ev.code, 2);
        }
        if(ev.type == EV_KEY && ev.value == 2){
            //printf("The value is %d\n", ev.value);
            //printf("The code is %d\n", ev.code);
            //printf("The type is %d\n", ev.type);
            //printf("aletter \n");
        }
        //if(write(fdo, ev, sizeof(struct input_event)) < 0) die("error: write");
        //printf("caca \n");
        //printf("The integer is %d\n", ij);
        //ij--;
    }
    
    if(ioctl(fdo, UI_DEV_DESTROY) < 0) die("error: ioctl");
    
    close(fdi);
    close(fdo);
    
    return 0;
} 
