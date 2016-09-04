
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, January 2005
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file   branchMIP.cpp
/// \brief  Branch & bound implementation for Mixed integer programming

#include "branchMIP.h"


branchMIP::branchMIP(goblinILPWrapper &RX,TTimer thisTimerModule) throw() :
    branchNode<TVar,TFloat>(RX.L(),RX.Context()), X(RX)
{
    // Generate a plain copy of the original problem
    // for solving the LP relaxation

    Y = X.Clone();
    timerModule = thisTimerModule;

    if (CT.traceLevel==3) X.Display();

    LogEntry(LOG_MEM,"(mixed integer problem)");
}


branchMIP::branchMIP(branchMIP &Node) throw() :
    branchNode<TVar,TFloat>(Node.X.L(),Node.Context(),Node.scheme), X(Node.X)
{
    Y = Node.Y->Clone();
    timerModule = Node.timerModule;

    LogEntry(LOG_MEM,"(mixed integer problem)");
}


branchMIP::~branchMIP() throw()
{
    delete Y;

    LogEntry(LOG_MEM,"(mixed integer problem)");
}


unsigned long branchMIP::Size() const throw()
{
    return
          sizeof(branchMIP)
        + managedObject::Allocated()
        + branchNode<TVar,TFloat>::Allocated()
        + Allocated();
}


unsigned long branchMIP::Allocated() const throw()
{
    return 0;
}


TVar branchMIP::SelectVariable() throw()
{
    TVar i = NoVar;
    TFloat maxFrac = 0;

    for (TVar j=0;j<n;j++)
    {
        if (Y->VarType(j)!= goblinILPWrapper::VAR_INT) continue;

        if (Y->VarType(j)!= goblinILPWrapper::VAR_INT) continue;

        TFloat val = Y->X(j);
        TFloat thisFrac = fabs(val-floor(val));

        if (thisFrac>0.5) thisFrac = 1-thisFrac;

        if (thisFrac>maxFrac)
        {
            maxFrac = thisFrac;
            i = j;
        }
    }

    if (i!=NoVar && maxFrac>CT.epsilon) return i;

    #if defined(_FAILSAVE_)

    InternalError("SelectVariable","Solution is fixed");

    #endif

    throw ERInternal();
}


branchNode<TVar,TFloat>::TBranchDir branchMIP::DirectionConstructive(TVar i)
    throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (i>=n) NoSuchNode("DirectionConstructive",i);

    #endif

    return RAISE_FIRST;
}


branchNode<TVar,TFloat>::TBranchDir branchMIP::DirectionExhaustive(TVar i)
    throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (i>=n) NoSuchNode("DirectionExhaustive",i);

    #endif

    return RAISE_FIRST;
}


branchNode<TVar,TFloat> *branchMIP::Clone() throw()
{
    return new branchMIP(*this);
}


void branchMIP::Raise(TVar i) throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (i>=n) NoSuchNode("Raise",i);

    #endif

    TFloat l = Y->LRange(i);
    Y -> SetLRange(i,l+1);

    if (Y->URange(i)<=l+1) unfixed--;

    solved = false;
}


void branchMIP::Lower(TVar i) throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (i>=n) NoSuchNode("Lower",i);

    #endif

    TFloat u = Y->URange(i);
    Y -> SetURange(i,u-1);

    if (Y->LRange(i)>=u-1) unfixed--;

    solved = false;
}


TFloat branchMIP::SolveRelaxation() throw()
{
    CT.SuppressLogging();

    TFloat ret = Y->SolveLP();

    CT.RestoreLogging();

    return ret;
}


TFloat branchMIP::Infeasibility() const throw()
{
    if (ObjectSense()==MINIMIZE)
        return InfFloat;
    else return -InfFloat;
}


bool branchMIP::Feasible() throw()
{
    for (TVar j=0;j<n;j++)
    {
        if (Y->VarType(j)!= goblinILPWrapper::VAR_INT) continue;

        TFloat val = Y->X(j);
        TFloat thisFrac = fabs(val-floor(val));

        if (thisFrac>CT.epsilon && thisFrac<1-CT.epsilon) return false;
    }

    return true;
}


void branchMIP::SaveSolution() throw()
{
    for (TVar i=0;i<n;i++) X.SetVarValue(i,Y->X(i));

    if (CT.traceLevel==3) X.Display();
}


TFloat goblinILPWrapper::SolveMIP() throw(ERRejected)
{
    LogEntry(LOG_METH,"Solving mixed integer problem...");
    OpenFold(ModDakin);

    branchMIP* rootNode = new branchMIP(*this,TimerMIP);

    TFloat infeasibilty = InfFloat;

    if (ObjectSense()==MAXIMIZE) infeasibilty = -InfFloat;

    branchScheme<TVar,TFloat> scheme(rootNode,infeasibilty);

    TFloat ret = scheme.savedObjective;

    if (CT.logRes)
    {
        if (ret!=infeasibilty)
        {
            sprintf(CT.logBuffer,
                "...Mixed integer solution with cost %g found",ret);
            LogEntry(LOG_RES,CT.logBuffer);
        }
        else
        {
            LogEntry(LOG_RES,"...No mixed integer solution found");
        }
    }

    SetLowerBound(TimerMIP,ret);
    CloseFold(ModDakin);

    return TVar(ret);
}
