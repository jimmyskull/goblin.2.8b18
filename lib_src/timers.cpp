
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, June 2003
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file   timers.cpp
/// \brief  #goblinTimer class implementation

#include "timers.h"


#if defined(_TIMERS_)


goblinTimer::goblinTimer(goblinTimer** thisGlobalTimer) throw()
{
    clockTick = sysconf(_SC_CLK_TCK)/double(1000);

    if (thisGlobalTimer==NULL)
    {
        savedTime = NULL;
        globalTimer = NULL;
    }
    else
    {
        savedTime = new double[unsigned(NoTimer)];
        globalTimer = thisGlobalTimer;
    }

    Reset();
}


goblinTimer::~goblinTimer() throw()
{
    if (savedTime) delete[] savedTime;
}


void goblinTimer::Reset() throw(ERRejected)
{
    accTime = 0;

    enabled = false;
    nRounds = 0;
    nestingDepth = 0;

    ResetBounds();
}


bool goblinTimer::Enable() throw()
{
    if (nestingDepth==0)
    {
        enabled = true;

        ResetBounds();

        struct tms currentTimes;
        times(&currentTimes);
        startTime = currentTimes.tms_utime+currentTimes.tms_stime;

        if (savedTime)
        {
            for (unsigned i=0;i<NoTimer;i++)
                savedTime[i] = globalTimer[i]->AccTime();
        }

        progressCounter = 0;
        maxCounter = 0;
    }

    nestingDepth++;

    return (nestingDepth==1);
}


bool goblinTimer::Disable() throw()
{
    if (nestingDepth>0)
    {
        nestingDepth--;

        if (nestingDepth==0)
        {
            enabled = false;

            struct tms currentTimes;
            times(&currentTimes);
            prevTime =
                (currentTimes.tms_utime+currentTimes.tms_stime-startTime)
                /clockTick;
            accTime += prevTime;

            if (nRounds==0 || prevTime>maxTime) maxTime = prevTime;
            if (nRounds==0 || prevTime<minTime) minTime = prevTime;

            nRounds++;

            if (savedTime)
            {
                for (unsigned i=0;i<NoTimer;i++)
                    savedTime[i] = globalTimer[i]->AccTime() - savedTime[i];
            }

            return true;
        }
    }

    return false;
}


double goblinTimer::AccTime() const throw()
{
    if (nRounds==0) return 0;
    else return accTime;
}


double goblinTimer::MaxTime() const throw()
{
    if (nRounds==0) return 0;
    else return maxTime;
}


double goblinTimer::MinTime() const throw()
{
    if (nRounds==0) return 0;
    else return minTime;
}


double goblinTimer::AvTime() const throw()
{
    if (nRounds==0) return 0;
    else return accTime/nRounds;
}


double goblinTimer::PrevTime() const throw()
{
    if (nRounds==0) return 0;
    else return prevTime;
}


double goblinTimer::CurrTime() const throw()
{
    if (nestingDepth==0) return 0;

    struct tms currentTimes;
    times(&currentTimes);
    return
        (currentTimes.tms_utime+currentTimes.tms_stime-startTime)/clockTick;
}


double goblinTimer::ChildTime(TTimer tm) const throw()
{
    if (savedTime==NULL || nRounds==0) return 0;

    if (enabled) return globalTimer[tm]->AccTime() - savedTime[tm];
    else return savedTime[tm];
}


void goblinTimer::ResetBounds(TFloat _lower,TFloat _upper) throw()
{
    lowerBound = _lower;
    upperBound = _upper;
}


bool goblinTimer::SetLowerBound(TFloat newBound) throw(ERRejected)
{
    if (newBound>upperBound) newBound = upperBound;

    if (newBound>lowerBound)
    {
        lowerBound = newBound;
        return true;
    }

    return false;
}


bool goblinTimer::SetUpperBound(TFloat newBound) throw(ERRejected)
{
    if (newBound<lowerBound) newBound = lowerBound;

    if (newBound<upperBound)
    {
        upperBound = newBound;
        return true;
    }

    return false;
}


void goblinTimer::InitProgressCounter(double l,TProgressOptions mode)
    throw()
{
    progressCounter = 0;
    maxCounter = l;
    progressMode = mode;
}


void goblinTimer::ProgressStep(double l) throw()
{
    progressCounter += l;
}


void goblinTimer::SetProgressCounter(double l) throw()
{
    progressCounter = l;
}


void goblinTimer::SetProgressMax(double l) throw()
{
    maxCounter = l;
}

#endif
