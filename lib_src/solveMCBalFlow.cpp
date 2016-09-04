
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, August 2003
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file   solveMCBalFlow.cpp
/// \brief  Methods for weighted balanced network flow problems

#include "abstractBalanced.h"
#include "balancedToBalanced.h"
#include "surfaceGraph.h"


TFloat abstractBalancedFNW::MinCBalFlow(TNode s) throw(ERRange,ERRejected)
{
    #if defined(_FAILSAVE_)

    if (s>=n) NoSuchNode("MinCBalFlow",s);

    if (MaxLCap()>0)
        Error(ERR_REJECTED,"MinCBalFlow","Non-trivial lower bounds");

    #endif

    OpenFold(ModMinCBalFlow,NO_INDENT);

    LogEntry(LOG_METH,"Computing minimum cost balanced flow...");

    TFloat ret = InfFloat;

    switch (CT.methMinCBalFlow)
    {
        case 0:
        {
            ret = PrimalDual(s);
            break;
        }
        case 1:
        {
            ret = EnhancedPD(s);
            break;
        }
        default:
        {
            UnknownOption("MinCBalFlow",CT.methMinCBalFlow);
            CloseFold(ModMinCBalFlow,NO_INDENT);
            throw ERRejected();
        }
    }

    CloseFold(ModMinCBalFlow,NO_INDENT);

    return ret;
}


TFloat abstractBalancedFNW::PrimalDual(TNode s) throw(ERRange,ERRejected)
{
    #if defined(_FAILSAVE_)

    if (s>=n) NoSuchNode("PrimalDual",s);

    if (MaxLCap()>0) Error(ERR_REJECTED,"PrimalDual","Non-trivial lower bounds");

    #endif

    OpenFold(ModPrimalDual,SHOW_TITLE);

    TFloat ret = Weight();
    SetLowerBound(TimerPrimalDual,ret);

    Symmetrize();

    surfaceGraph G(*this);

    switch (CT.methPrimalDual)
    {
        case 0:
        {
            G.PrimalDual0(s);
            break;
        }
        case 1:
        case 2:
        {
            G.PrimalDual1(s);
            break;
        }
        default:
        {
            UnknownOption("PrimalDual",CT.methPrimalDual);
        }
    }

    CloseFold(ModPrimalDual);

    #if defined(_FAILSAVE_)

    if (CT.methFailSave==1 && !G.Compatible())
    {
        InternalError("PrimalDual","Solutions are corrupted");
    }

    #endif

    ret = Weight();

    SetBounds(TimerPrimalDual,ret,ret);

    return ret;
}


TFloat abstractBalancedFNW::EnhancedPD(TNode s) throw(ERRange,ERRejected)
{
    #if defined(_FAILSAVE_)

    if (s>=n) NoSuchNode("EnhancedPD",s);

    if (MaxLCap()>0)
        Error(ERR_REJECTED,"EnhancedPD","Non-trivial lower bounds");

    #endif

    OpenFold(ModEnhancedPD,SHOW_TITLE);

    #if defined(_PROGRESS_)

    TFloat deficiency = 0;

    THandle H1 = Investigate();
    while (Active(H1,s))
    {
        TArc a = Read(H1,s);
        deficiency += BalCap(a);
    }

    Close(H1);

    InitProgressCounter(deficiency);

    TFloat approxFirstStep = deficiency-n/3;
    if (approxFirstStep<0) approxFirstStep = deficiency/2;

    SetProgressNext(approxFirstStep);

    #endif

    MinCostSTFlow(s,s^1);

    #if defined(_PROGRESS_)

    SetProgressCounter(approxFirstStep);
    SetProgressNext(0);

    #endif

    CancelEven();

    TFloat ret = CancelPD(deficiency);

    CloseFold(ModEnhancedPD);

    return ret;
}


TFloat abstractBalancedFNW::CancelPD(TFloat deficiency) throw()
{
    #if defined(_FAILSAVE_)

    if (Q==NULL) Error(ERR_REJECTED,"CancelPD","No odd cycles present");

    #endif

    balancedToBalanced G(*this);

    #if defined(_PROGRESS_)

    THandle H1 = G.Investigate();
    TNode s = G.Source();

    while (G.Active(H1,s))
    {
        TArc a = G.Read(H1,s);
        deficiency -= G.BalCap(a);
    }

    G.Close(H1);

    SetProgressCounter(deficiency);
    SetProgressNext(1);

    #endif

    LogEntry(LOG_METH,"Refining balanced flow...");

    return G.PrimalDual(G.Source());
}
