
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, June 2002
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file   messenger.cpp
/// \brief  #goblinMessenger class implementation


#include "messenger.h"


// ------------------------------------------------------


goblinMessenger::goblinMessenger(TLog thisSize,goblinController &thisContext)
    throw() : CT(thisContext)
{
    qSize = thisSize;

    qMsg = new msgType[qSize];
    qModule = new TModule[qSize];
    qLevel = new int[qSize];
    qHandle = new THandle[qSize];
    qText = new char*[qSize];
    qNext = new TLog[qSize];
    qHidden = new bool[qSize];

    pthread_mutex_init(&msgLock,NULL);
    pthread_mutex_init(&traceLock,NULL);
    pthread_mutex_init(&traceEventLock,NULL);
    pthread_mutex_init(&solverStateLock,NULL);

    for (TLog i=0;i<qSize;i++) qNext[i] = i+1;

    firstEntry = qSize;
    lastEntry = qSize;
    currentEntry = qSize;
    firstFree = 0;

    traceEvent = false;
    cTraceFile = NULL;
    signalHalt = 2;

    sprintf(CT.logBuffer,"This is the GOBLIN Graph Library, Release %d.%d%s",
             CT.majorVersion,CT.minorVersion,CT.patchLevel);
    LogEventHandler(LOG_SHELL,ModRoot,NoHandle,CT.logBuffer);
    LogEventHandler(LOG_SHELL,ModRoot,NoHandle,"");
    LogEventHandler(LOG_SHELL,ModRoot,NoHandle,
        "(C) 1998-2007  Dr.Christian Fremuth-Paeger and others");
    LogEventHandler(LOG_SHELL,ModRoot,NoHandle,"See reference manual for a complete list of contributions");
    LogEventHandler(LOG_SHELL,ModRoot,NoHandle,"");
}


goblinMessenger::~goblinMessenger() throw()
{
    TLog i = firstEntry;

    while (i!=qSize)
    {
        delete[] qText[i];
        i = qNext[i];
    }

    delete[] qMsg;
    delete[] qModule;
    delete[] qLevel;
    delete[] qHandle;
    delete[] qText;
    delete[] qNext;
    delete[] qHidden;

    pthread_mutex_destroy(&msgLock);
    pthread_mutex_destroy(&traceLock);
    pthread_mutex_destroy(&traceEventLock);
    pthread_mutex_destroy(&solverStateLock);
}


unsigned long goblinMessenger::Size() throw()
{
    unsigned long ret =
          sizeof(goblinMessenger)
        + qSize*sizeof(msgType) // qMsg[]
        + qSize*sizeof(TModule) // qModule[]
        + qSize*sizeof(THandle) // qHandle[]
        + qSize*sizeof(int)     // qLevel[]
        + qSize*sizeof(char*)   // qText[]
        + qSize*sizeof(bool)    // qHidden[]
        + qSize*sizeof(TLog);   // qNext[]

    TLog i = firstEntry;

    while (i!=qSize)
    {
        ret += sizeof(char)*(strlen(qText[i])+1);
        i = qNext[i];
    }

    // mutexes ?

    return ret;
}


void goblinMessenger::Restart() throw()
{
    for (TLog i=0;i<qSize;i++) qNext[i] = i+1;

    firstEntry = qSize;
    lastEntry = qSize;
    currentEntry = qSize;
    firstFree = 0;

    traceEvent = false;
    CT.breakLevel = INT_MAX;
    CT.logLevel = 0;

    if (cTraceFile)
    {
        delete[] cTraceFile;
        cTraceFile = NULL;
    }
}


void goblinMessenger::LogEventHandler(msgType msg,TModule mod,THandle OH,char* logText)
    throw()
{
    CT.DefaultLogEventHandler(msg,mod,OH,logText);
    MsgAppend(msg,mod,OH,logText);
}


void goblinMessenger::MsgAppend(msgType msg,TModule mod,THandle OH,char* logText)
    throw()
{
    pthread_mutex_lock(&msgLock);

    if (msg==MSG_APPEND && lastEntry<qSize)
    {
        qText[lastEntry] = (char*)GoblinRealloc(qText[lastEntry],
            strlen(qText[lastEntry])+strlen(logText)+1);
        strcat(qText[lastEntry],logText);
    }
    else
    {
        TLog nextEntry = firstFree;
        if (nextEntry==qSize)
        {
            nextEntry = firstEntry;
            firstEntry = qNext[firstEntry];
            delete[] qText[nextEntry];
        }
        else firstFree = qNext[firstFree];

        if (lastEntry==qSize)
        {
            firstEntry = nextEntry;
            currentEntry = nextEntry;
        }
        else qNext[lastEntry] = nextEntry;

        lastEntry = nextEntry;
        qText[lastEntry] = new char[strlen(logText)+1];
        sprintf(qText[lastEntry],"%s",logText);
        qHandle[lastEntry] = OH;
        qLevel[lastEntry] = CT.logLevel;
        qMsg[lastEntry] = msg;
        qModule[lastEntry] = mod;
        qHidden[lastEntry] = 0;
    }

    pthread_mutex_unlock(&msgLock);
}


void goblinMessenger::MsgReset() throw()
{
    pthread_mutex_lock(&msgLock);
    currentEntry = firstEntry;
    pthread_mutex_unlock(&msgLock);
}


bool goblinMessenger::MsgEOF() throw()
{
    pthread_mutex_lock(&msgLock);
    bool ret = (currentEntry==qSize || currentEntry==lastEntry);
    pthread_mutex_unlock(&msgLock);

    return ret;
}


bool goblinMessenger::MsgVoid() throw()
{
    pthread_mutex_lock(&msgLock);
    bool ret = (firstEntry==qSize);
    pthread_mutex_unlock(&msgLock);

    return ret;
}


void goblinMessenger::MsgSkip() throw(ERRejected)
{
    pthread_mutex_lock(&msgLock);

    if (currentEntry==qSize || qNext[currentEntry]==qSize)
    {
        pthread_mutex_unlock(&msgLock);
        CT.Error(ERR_REJECTED,NoHandle,"MsgSkip","No more queued messages");
    }

    currentEntry = qNext[currentEntry];
    pthread_mutex_unlock(&msgLock);
}


char* goblinMessenger::MsgText() throw(ERRejected)
{
    pthread_mutex_lock(&msgLock);

    if (currentEntry==qSize)
    {
        pthread_mutex_unlock(&msgLock);
        CT.Error(ERR_REJECTED,NoHandle,"MsgText","Message queue is empty");
    }

    char* ret = (char*)malloc(sizeof(char)*(strlen(qText[currentEntry])+1));
    strcpy(ret,qText[currentEntry]);
    pthread_mutex_unlock(&msgLock);

    return ret;
}


msgType goblinMessenger::MsgClass() throw(ERRejected)
{
    pthread_mutex_lock(&msgLock);

    if (currentEntry==qSize)
    {
        pthread_mutex_unlock(&msgLock);
        CT.Error(ERR_REJECTED,NoHandle,"MsgClass","Message queue is empty");
    }

    pthread_mutex_unlock(&msgLock);

    return qMsg[currentEntry];
}


TModule goblinMessenger::MsgModule() throw(ERRejected)
{
    pthread_mutex_lock(&msgLock);

    if (currentEntry==qSize)
    {
        pthread_mutex_unlock(&msgLock);
        CT.Error(ERR_REJECTED,NoHandle,"MsgModule","Message queue is empty");
    }

    pthread_mutex_unlock(&msgLock);

    return qModule[currentEntry];
}


int goblinMessenger::MsgLevel() throw(ERRejected)
{
    pthread_mutex_lock(&msgLock);

    if (currentEntry==qSize)
    {
        pthread_mutex_unlock(&msgLock);
        CT.Error(ERR_REJECTED,NoHandle,"MsgLevel","Message queue is empty");
    }

    pthread_mutex_unlock(&msgLock);

    return qLevel[currentEntry];
}


THandle goblinMessenger::MsgHandle() throw(ERRejected)
{
    pthread_mutex_lock(&msgLock);

    if (currentEntry==qSize)
    {
        pthread_mutex_unlock(&msgLock);
        CT.Error(ERR_REJECTED,NoHandle,"MsgHandle","Message queue is empty");
    }

    pthread_mutex_unlock(&msgLock);

    return qHandle[currentEntry];
}


bool goblinMessenger::MsgHidden() throw(ERRejected)
{
    pthread_mutex_lock(&msgLock);

    if (currentEntry==qSize)
    {
        pthread_mutex_unlock(&msgLock);
        CT.Error(ERR_REJECTED,NoHandle,"MsgHidden","Message queue is empty");
    }

    pthread_mutex_unlock(&msgLock);

    return qHidden[currentEntry];
}


void goblinMessenger::TraceAppend(char* fileName) throw()
{
    pthread_mutex_lock(&traceLock);

    pthread_mutex_lock(&traceEventLock);
    traceEvent = true;
    pthread_mutex_unlock(&traceEventLock);

    delete[] cTraceFile;

    if (fileName==NULL) cTraceFile = NULL;
    else
    {
        cTraceFile = new char[strlen(fileName)+1];
        sprintf(cTraceFile,"%s",fileName);
    }

    pthread_mutex_unlock(&traceLock);
}


void goblinMessenger::TraceSemTake() throw()
{
    bool searching = true;

    while (searching)
    {
        pthread_mutex_lock(&traceEventLock);
        searching = traceEvent;
        pthread_mutex_unlock(&traceEventLock);
    }
}


char* goblinMessenger::TraceFilename() throw(ERRejected)
{
    pthread_mutex_lock(&traceLock);

    if (cTraceFile==NULL)
    {
        pthread_mutex_unlock(&traceLock);
        return NULL;
    }

    char* ret = (char*)malloc(sizeof(char)*(strlen(cTraceFile)+1));
    strcpy(ret,cTraceFile);
    pthread_mutex_unlock(&traceLock);

    return ret;
}


bool goblinMessenger::TraceEvent() throw()
{
    pthread_mutex_lock(&traceEventLock);
    bool ret = traceEvent;
    pthread_mutex_unlock(&traceEventLock);

    return ret;
}


void goblinMessenger::TraceUnblock() throw()
{
    pthread_mutex_lock(&traceEventLock);
    traceEvent = false;
    pthread_mutex_unlock(&traceEventLock);
}


bool goblinMessenger::SolverIdle() throw()
{
    pthread_mutex_lock(&solverStateLock);
    bool ret = (signalHalt==2);
    pthread_mutex_unlock(&solverStateLock);

    return ret;
}


bool goblinMessenger::SolverRunning() throw()
{
    pthread_mutex_lock(&solverStateLock);
    bool ret = (signalHalt==0);
    pthread_mutex_unlock(&solverStateLock);

    return ret;
}


void goblinMessenger::SolverSignalPending() throw()
{
    pthread_mutex_lock(&solverStateLock);
    signalHalt = 3;
    pthread_mutex_unlock(&solverStateLock);
}


void goblinMessenger::SolverSignalStarted() throw()
{
    pthread_mutex_lock(&solverStateLock);
    signalHalt = 0;
    pthread_mutex_unlock(&solverStateLock);
}


void goblinMessenger::SolverSignalStop() throw()
{
    pthread_mutex_lock(&solverStateLock);
    if (signalHalt!=2) signalHalt = 1;
    pthread_mutex_unlock(&solverStateLock);
}


void goblinMessenger::SolverSignalIdle() throw()
{
    pthread_mutex_lock(&solverStateLock);
    signalHalt = 2;
    pthread_mutex_unlock(&solverStateLock);
}
