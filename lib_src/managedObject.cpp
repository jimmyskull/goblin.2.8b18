
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, May 1999
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file   managedObject.cpp
/// \brief  #managedObject class interface

#include "managedObject.h"


managedObject::managedObject(goblinController& thisContext,
    TOption options) throw() : CT(thisContext)
{
    refCounter = 0;

    if (!options) OH = CT.InsertObject(this);
    else OH = NoHandle;

    objectName = NULL;

    LogEntry(LOG_MEM,"...Data object inserted into context");
}


managedObject::~managedObject() throw()
{
    #if defined(_FAILSAVE_)

    if (refCounter)
    {
        InternalError("managedObject","Object is referenced");
    }

    #endif

    if (OH!=NoHandle) CT.DeleteObject(this);

    if (objectName) delete[] objectName;

    LogEntry(LOG_MEM,"...Data object disallocated");
}


unsigned long managedObject::Allocated() const throw()
{
    if (objectName) return strlen(objectName)+1;

    return 0;
}


char* managedObject::Display() const throw(ERFile,ERRejected)
{
    Error(ERR_REJECTED,"Display","This object cannot be viewed");

    throw ERRejected();
}


void managedObject::NoSuchNode(char* methodName,TNode v) const throw(ERRange)
{
    if (v==NoNode) sprintf(CT.logBuffer,"Undefined node");
    else sprintf(CT.logBuffer,"No such node: %ld",v);

    CT.Error(ERR_RANGE,OH,methodName,CT.logBuffer);
}


void managedObject::NoMoreArcs(char* methodName,TNode v) const throw(ERRejected)
{
    sprintf(CT.logBuffer,"No more arcs: %ld",v);

    CT.Error(ERR_REJECTED,OH,methodName,CT.logBuffer);
}


void managedObject::NoSuchArc(char* methodName,TArc a) const throw(ERRange)
{
    if (a==NoArc) sprintf(CT.logBuffer,"Undefined arc");
    else sprintf(CT.logBuffer,"No such arc: %ld",a);

    CT.Error(ERR_RANGE,OH,methodName,CT.logBuffer);
}


void managedObject::CancelledArc(char* methodName,TArc a) const throw(ERRejected)
{
    sprintf(CT.logBuffer,"Cancelled arc: %ld",a);

    CT.Error(ERR_RANGE,OH,methodName,CT.logBuffer);
}


void managedObject::NoSuchHandle(char* methodName,THandle H) const throw(ERRejected)
{
    if (H==NoHandle) sprintf(CT.logBuffer,"Undefined handle");
    else sprintf(CT.logBuffer,"No such handle: %ld",H);

    CT.Error(ERR_RANGE,OH,methodName,CT.logBuffer);
}


void managedObject::NoSuchItem(char* methodName,unsigned long i) const throw(ERRange)
{
    sprintf(CT.logBuffer,"No such item: %ld",i);

    CT.Error(ERR_RANGE,OH,methodName,CT.logBuffer);
}


void managedObject::NoSuchItem(char* methodName,unsigned short i) const throw(ERRange)
{
    sprintf(CT.logBuffer,"No such item: %d",i);

    CT.Error(ERR_RANGE,OH,methodName,CT.logBuffer);
}


void managedObject::NoSuchIndex(char* methodName,unsigned long i) const throw(ERRange)
{
    sprintf(CT.logBuffer,"No such index: %ld",i);

    CT.Error(ERR_RANGE,OH,methodName,CT.logBuffer);
}


void managedObject::NoSuchIndex(char* methodName,unsigned short i) const throw(ERRange)
{
    sprintf(CT.logBuffer,"No such index: %d",i);

    CT.Error(ERR_RANGE,OH,methodName,CT.logBuffer);
}


void managedObject::NoSuchCoordinate(char* methodName,TDim i) const throw(ERRange)
{
    sprintf(CT.logBuffer,"No such coordinate: %d",i);

    CT.Error(ERR_RANGE,OH,methodName,CT.logBuffer);
}


void managedObject::NoRepresentation(char* methodName) const throw(ERRejected)
{
    CT.Error(ERR_REJECTED,OH,methodName,"Graph must be represented");
}


void managedObject::NoSparseRepresentation(char* methodName) const throw(ERRejected)
{
    CT.Error(ERR_REJECTED,OH,methodName,"Graph must have a sparse representation");
}


void managedObject::UnknownOption(char* methodName,int opt) const throw(ERRejected)
{
    sprintf(CT.logBuffer,"Unknown option: %d",opt);

    CT.Error(ERR_REJECTED,OH,methodName,CT.logBuffer);
}


void managedObject::AmountOutOfRange(char* methodName,TFloat alpha) const throw(ERRejected)
{
    sprintf(CT.logBuffer,"Amount out of range: %g",alpha);

    CT.Error(ERR_REJECTED,OH,methodName,CT.logBuffer);
}


const char* managedObject::Label() const throw()
{
    if (objectName) return objectName;

    return "unnamed";
}


void managedObject::SetLabel(const char* l) throw()
{
    if (l!=NULL)
    {
        int newLength = strlen(l)+1;

        if (objectName)
        {
            objectName = static_cast<char*>(GoblinRealloc(objectName,newLength));
        }
        else
        {
            objectName = new char[newLength];
        }

        strcpy(objectName,l);
    }
    else if (objectName)
    {
        delete[] objectName;
        objectName = NULL;
    }
}


void managedObject::OpenFold(const TModule mod,const TOption opt)
    const throw(ERRejected)
{
    CT.OpenFold(mod,opt);

    #if defined(_TIMERS_)

    CT.globalTimer[listOfModules[mod].moduleTimer] -> Enable();

    #endif
}


void managedObject::CloseFold(const TModule mod,const TOption opt)
    const throw(ERRejected)
{
    #if defined(_TIMERS_)

    goblinTimer* TM = CT.globalTimer[listOfModules[mod].moduleTimer];

    if (TM->Disable() && CT.logTimers && TM->AccTime()>0.001)
    {
        sprintf(CT.logBuffer,"Timer report (%s)",
            listOfTimers[listOfModules[mod].moduleTimer].timerName);
        LogEntry(LOG_TIMERS,CT.logBuffer);

        sprintf(CT.logBuffer,"  Cumulated times : %9.0f ms",TM->AccTime());
        LogEntry(LOG_TIMERS,CT.logBuffer);
        CT.IncreaseLogLevel();

        if (TM->PrevTime()+0.001<TM->AccTime())
        {
            sprintf(CT.logBuffer,"Previous round  : %9.0f ms",TM->PrevTime());
            LogEntry(LOG_TIMERS,CT.logBuffer);
            sprintf(CT.logBuffer,"Minimum         : %9.0f ms",TM->MinTime());
            LogEntry(LOG_TIMERS,CT.logBuffer);
            sprintf(CT.logBuffer,"Average         : %9.0f ms",TM->AvTime());
            LogEntry(LOG_TIMERS,CT.logBuffer);
            sprintf(CT.logBuffer,"Maximum         : %9.0f ms",TM->MaxTime());
            LogEntry(LOG_TIMERS,CT.logBuffer);
        }

        if (TM->FullInfo())
        {
            for (unsigned i=0;i<NoTimer;i++)
            {
                double thisTime = TM->ChildTime(TTimer(i));

                if (TTimer(i)!=listOfModules[mod].moduleTimer
                    && thisTime>0.001)
                {
                    sprintf(CT.logBuffer,"%-15s : %9.0f ms (%4.1f %%)",
                        listOfTimers[i].timerName,
                        thisTime,thisTime/TM->PrevTime()*100);
                    LogEntry(LOG_TIMERS,CT.logBuffer);
                }
            }
        }

        CT.DecreaseLogLevel();
    }

    #endif

    CT.CloseFold(mod,opt);
}


void managedObject::SetLowerBound(const TTimer tm,const TFloat newBound)
    const throw(ERRejected)
{
    #if defined(_TIMERS_)

    SetBounds(tm,newBound,InfFloat);

    #endif
}


void managedObject::SetUpperBound(const TTimer tm,const TFloat newBound)
    const throw(ERRejected)
{
    #if defined(_TIMERS_)

    SetBounds(tm,-InfFloat,newBound);

    #endif
}


void managedObject::SetBounds(const TTimer tm,const TFloat lBound,
    const TFloat uBound) const throw(ERRejected)
{
    #if defined(_TIMERS_)

    #if defined(_FAILSAVE_)

    if (uBound<CT.globalTimer[tm]->LowerBound()-CT.epsilon)
    {
        sprintf(CT.logBuffer,"Trying to override lower bound %g with %g",
            CT.globalTimer[tm]->LowerBound(),uBound);
        Error(ERR_REJECTED,"SetBounds",CT.logBuffer);
    }

    if (lBound>CT.globalTimer[tm]->UpperBound()+CT.epsilon)
    {
        sprintf(CT.logBuffer,"Trying to override upper bound %g with %g",
            CT.globalTimer[tm]->UpperBound(),lBound);
        Error(ERR_REJECTED,"SetBounds",CT.logBuffer);
    }

    #endif

    bool ret = CT.globalTimer[tm]->SetLowerBound(lBound);
    ret |= CT.globalTimer[tm]->SetUpperBound(uBound);

    if (CT.logGaps && CT.logEventHandler && ret)
    {
        if (CT.globalTimer[tm]->LowerBound()>-InfFloat)
        {
            if (CT.globalTimer[tm]->LowerBound()<InfFloat)
            {
                sprintf(CT.logBuffer,"Gap (%s) changes to [%.3f",
                    listOfTimers[tm].timerName,
                    CT.globalTimer[tm]->LowerBound());
            }
            else
            {
                sprintf(CT.logBuffer,"Gap (%s) changes to [infinity",
                    listOfTimers[tm].timerName);
            }
        }
        else
        {
            sprintf(CT.logBuffer,"Gap (%s) changes to [-infinity",
                listOfTimers[tm].timerName);
        }

        if (CT.globalTimer[tm]->UpperBound()>-InfFloat)
        {
            if (CT.globalTimer[tm]->UpperBound()<InfFloat)
            {
                sprintf(CT.logBuffer,"%s,%.3f]",
                    CT.logBuffer,
                    CT.globalTimer[tm]->UpperBound());
            }
            else
            {
                sprintf(CT.logBuffer,"%s,infinity]",CT.logBuffer);
            }
        }
        else
        {
            sprintf(CT.logBuffer,"%s,-infinity]",CT.logBuffer);
        }

        LogEntry(LOG_GAPS,CT.logBuffer);
    }

    #endif
}


void managedObject::ResetBounds(const TTimer tm,const TFloat _lower,const TFloat _upper) const throw()
{
    #if defined(_TIMERS_)

    CT.globalTimer[tm]->ResetBounds(_lower,_upper);

    #endif
}


TFloat managedObject::LowerBound(const TTimer tm) const throw()
{
    #if defined(_TIMERS_)

    return CT.globalTimer[tm]->LowerBound();

    #else

    return -InfFloat;

    #endif
}


TFloat managedObject::UpperBound(const TTimer tm) const throw()
{
    #if defined(_TIMERS_)

    return CT.globalTimer[tm]->UpperBound();

    #else

    return InfFloat;

    #endif
}
