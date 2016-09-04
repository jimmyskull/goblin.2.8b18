
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, February 1999
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file   abstractBalanced.cpp
/// \brief  #abstractBalancedFNW partial class implementation

#include "abstractBalanced.h"
#include "disjointFamily.h"


abstractBalancedFNW::abstractBalancedFNW(TNode _n1,TArc _m1) throw() :
    abstractDiGraph(2*_n1,2*_m1),
    n1(_n1)
{
    Q = NULL;
    prop = NULL;
    petal = NULL;
    base = NULL;

    LogEntry(LOG_MEM,"...Abstract balanced flow network allocated");
}


abstractBalancedFNW::~abstractBalancedFNW() throw()
{
    ReleaseCycles();
    ReleaseProps();
    ReleasePetals();
    ReleaseBlossoms();

    LogEntry(LOG_MEM,"...Abstract balanced flow network disallocated");
}


unsigned long abstractBalancedFNW::Allocated() const throw()
{
    unsigned long tmpSize = 0;

    if (Q!=NULL)            tmpSize += n*sizeof(TArc);
    if (prop!=NULL)         tmpSize += n*sizeof(TArc);
    if (petal!=NULL)        tmpSize += n*sizeof(TArc);
    if (base!=NULL)         tmpSize += n1*sizeof(TArc);

    return tmpSize;
}


TNode abstractBalancedFNW::ComplNode(TNode v) throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (v>=n) NoSuchNode("ComplNode",v);

    #endif

    return v^1;
}


TArc abstractBalancedFNW::ComplArc(TArc a) throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (a>=2*m) NoSuchArc("ComplArc",a);

    #endif

    return a^2;
}


void abstractBalancedFNW::InitCycles() throw()
{
    if (!Q)
    {
        Q = new TArc[n];
        LogEntry(LOG_MEM,"...Odd length cycles allocated");
    }
    else
    {
        #if defined(_LOGGING_)

        Error(MSG_WARN,"InitCycles","Odd length cycles are already present");

        #endif
    }

    for (TNode i=0;i<n;i++) Q[i] = NoArc;
}


void abstractBalancedFNW::ReleaseCycles() throw()
{
    if (Q)
    {
        delete[] Q;
        Q = NULL;
        LogEntry(LOG_MEM,"...Odd length cycles disallocated");
    }
}


void abstractBalancedFNW::InitBlossoms() throw()
{
    if (!base)
    {
        base =  new TNode[n1];
        partition = new disjointFamily<TNode>(n1,CT);
        LogEntry(LOG_MEM,"...Blossoms allocated");
    }
    else
    {
        #if defined(_LOGGING_)

        partition -> Init();
        Error(MSG_WARN,"InitBlossoms","Blossoms are already present");

        #endif
    }
}


void abstractBalancedFNW::ReleaseBlossoms() throw()
{
    if (base)
    {
        delete[] base;
        base = NULL;
        delete partition;
        partition = NULL;
        LogEntry(LOG_MEM,"...Blossoms disallocated");
    }
}


TNode abstractBalancedFNW::Base(TNode v) throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (v>=n) NoSuchNode("Base",v);

    #endif

    TNode u = partition->Find(v/2);

    if (u==NoNode) return NoNode;
    else return base[u];
}


TNode abstractBalancedFNW::Pred(TNode v) throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (v>=n) NoSuchNode("Pred",v);

    #endif

    return Base(StartNode(prop[v]));
}


void abstractBalancedFNW::Bud(TNode v) throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (v>=n) NoSuchNode("Bud",v);

    #endif

    partition -> Bud(v/2);
    base[v/2] = v;
}


void abstractBalancedFNW::Shrink(TNode u,TNode v) throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (u>=n) NoSuchNode("Shrink",u);

    if (v>=n) NoSuchNode("Shrink",v);

    #endif

    TNode x = partition->Find(u/2);
    TNode y = partition->Find(v/2);

    partition -> Merge(x,y);
    base[partition->Find(x)] = base[x];
}


void abstractBalancedFNW::InitProps() throw()
{
    if (!prop)
    {
        prop = new TArc[n];
        LogEntry(LOG_MEM,"...Props allocated");
    }
    else
    {
        #if defined(_LOGGING_)

        Error(MSG_WARN,"InitProps","Odd length cycles are already present");

        #endif
    }

    for (TNode i=0;i<n;i++) prop[i] = NoArc;
}


void abstractBalancedFNW::ReleaseProps() throw()
{
    if (prop)
    {
        delete[] prop;
        prop = NULL;
        LogEntry(LOG_MEM,"...Props disallocated");
    }
}


void abstractBalancedFNW::InitPetals() throw()
{
    if (!petal)
    {
        petal = new TArc[n];
        LogEntry(LOG_MEM,"...Petals allocated");
    }
    else
    {
        #if defined(_LOGGING_)

        Error(MSG_WARN,"InitPetals","Odd length cycles are already present");

        #endif
    }

    for (TNode i=0;i<n;i++) petal[i] = NoArc;
}


void abstractBalancedFNW::ReleasePetals() throw()
{
    if (petal)
    {
        delete[] petal;
        petal = NULL;
        LogEntry(LOG_MEM,"...Petals disallocated");
    }
}


TFloat abstractBalancedFNW::BalCap(TArc a) const throw(ERRange,ERRejected)
{
    #if defined(_FAILSAVE_)

    if (a>=2*m) NoSuchArc("BalCap",a);

    #endif

    if (a&1)
    {
        return BalFlow(a)-LCap(a);
    }
    else if (UCap(a)<InfCap)
    {
        return UCap(a)-BalFlow(a);
    }
    else if (BalFlow(a)<InfCap)
    {
        return InfCap;
    }
    else
    {
        return 0;
    }
}


void abstractBalancedFNW::BalAugment(TArc* pred,TNode u,TNode v,TFloat Lambda)
    throw(ERRange,ERRejected)
{
    #if defined(_FAILSAVE_)

    if (u>=n) NoSuchNode("BalAugment",u);

    if (v>=n) NoSuchNode("BalAugment",v);

    if (Lambda<=0) Error(ERR_REJECTED,"BalAugment","Amount should be positive");

    if (!pred) Error(ERR_REJECTED,"BalAugment","No path specified");

    TNode PathLength = 0;

    #endif

    TNode w = v;

    #if defined(_LOGGING_)

    THandle LH = NoHandle;

    if (CT.logMeth>1)
    {
        sprintf(CT.logBuffer,"Augmenting by %g units of flow...",2*Lambda);
        LogEntry(LOG_METH2,CT.logBuffer);
        LogEntry(LOG_METH2,"Path in reverse order:");
        OpenFold();
        sprintf(CT.logBuffer,"(%ld",w);
        LH = LogStart(LOG_METH2,CT.logBuffer);
    }

    #endif

    while (w!=u)
    {
        TArc a = pred[w];

        #if defined(_FAILSAVE_)

        if (PathLength>=n || a==NoArc)
            InternalError("BalAugment","Missing start node");

        PathLength++;

        #endif

        BalPush(a,Lambda);
        pred[w] = NoArc;
        w = StartNode(a);

        #if defined(_LOGGING_)

        if (CT.logMeth>1)
        {
            sprintf(CT.logBuffer,"[%ld]%ld",a,w);
            LogAppend(LH,CT.logBuffer);
        }

        #endif
    }

    #if defined(_LOGGING_)

    if (CT.logMeth>1)
    {
        LogEnd(LH,")");
        CloseFold();
    }

    #endif
}


TFloat abstractBalancedFNW::FindBalCap(TArc* pred,TNode u,TNode v) throw(ERRange,ERRejected)
{
    #if defined(_FAILSAVE_)

    if (u>=n) NoSuchNode("FindBalCap",u);

    if (v>=n) NoSuchNode("FindBalCap",v);

    if (!pred) Error(ERR_REJECTED,"FindBalCap","No path specified");

    TNode PathLength = 0;

    #endif

    TNode w = v;
    TFloat Lambda = InfFloat;

    while (w!=u)
    {
        TArc a = pred[w];

        #if defined(_FAILSAVE_)

        if (PathLength>=n || a==NoArc)
        {
            InternalError("FindBalCap","Missing start node");
        }

        PathLength++;

        #endif

        TFloat LambdaNeu = BalCap(a);
        w = StartNode(a);

        if (pred[w^1]==(a^2)) LambdaNeu = floor(LambdaNeu/2);
        if (LambdaNeu<Lambda) Lambda = LambdaNeu;

        #if defined(_FAILSAVE_)

        if (Lambda==0)
        {
            sprintf(CT.logBuffer,"Arc %ld has capacity %g",a,BalCap(a));
            Error(ERR_CHECK,"FindBalCap",CT.logBuffer);
        }

        #endif
    }

    return Lambda;
}


void abstractBalancedFNW::CancelEven() throw()
{
    LogEntry(LOG_METH,"Cancelling even length cycles...");
    OpenFold(ModCycleCancel);

    TArc* pred = InitPredecessors();
    InitCycles();

    if (CT.methFailSave)
    {
        for (TArc a=0;a<m;++a)
        {
            TFloat thisCap = ResCap(2*a);

            if (floor(thisCap+0.01)-floor(thisCap-0.01)<0.5)
            {
                sprintf(CT.logBuffer,"Arc %ld is non-integral",a);
                InternalError("CancelEven",CT.logBuffer);
            }
        }
    }

    Symmetrize();

    #if defined(_LOGGING_)

    THandle LH = NoHandle;

    if (CT.logMeth>1) LogEntry(LOG_METH2,"Traversed nodes (arcs):");

    TNode k = 0;

    #endif

    THandle H = Investigate();
    investigator &I = Investigator(H);

    for (TNode i=0;i<n1;i++)
    {
        TNode root = 2*i;
        TNode v = root;
/*
cout << "===========================\n";
    Symmetrize();
    for (TArc a=0;a<m;++a)
    {
        TNode u = StartNode(2*a);
        TNode w = EndNode(2*a);
        TFloat thisCap = BalCap(2*a);

        if (floor(thisCap+0.01)-floor(thisCap-0.01)<0.5)
        {
            printf("%4.li %4.li %7.li %8.2g %8.2g %8.2g\n", u, w, a, thisCap, BalFlow(2*a), UCap(2*a));
        }
    }
*/
        while (v!=root || I.Active(v))
        {
            TArc a = I.Read(v);
            TNode u = EndNode(a);
            TFloat thisCap = BalCap(a);

            if (   floor(thisCap+0.51)-floor(thisCap-0.51)<1.5
                || pred[v]==(a^1) || Q[v]==(a^1) || Q[u]==a
               )
            {
                continue;
            }

            #if defined(_LOGGING_)

            if (CT.logMeth>1)
            {
                sprintf(CT.logBuffer," %ld (%ld)",v,a);

                if (LH==NoHandle)
                {
                    LH = LogStart(LOG_METH2,CT.logBuffer);
                }
                else LogAppend(LH,CT.logBuffer);
            }

            #endif

            if (pred[u]!=NoArc || u==root)
            {
                BalPush(a^1,0.5);
                MakeIntegral(pred,u,v);
                v = u;

                #if defined(_LOGGING_)

                if (CT.logMeth>1)
                {
                    sprintf(CT.logBuffer," %ld",u);
                    LogEnd(LH,CT.logBuffer);
                    LogEntry(LOG_METH2,"...Fractional arcs cancelled");
                    LH = NoHandle;
                }

                #endif

                Trace();

                continue;
            }

            TNode cu = u^1;

            if (pred[cu]==NoArc && cu!=root)
            {
                pred[u] = a;
                v = u;

                continue;
            }

            pred[u] = a;
            TNode w = u;

            while (w!=cu && Q[w]==NoArc) w = StartNode(pred[w]);

            if (Q[w]==NoArc)
            {
                v = u;

                while (v!=cu)
                {
                    a = pred[v];
                    Q[v] = a;
                    Q[v^1] = a^3;
                    pred[v] = NoArc;
                    v = StartNode(a);
                }

                #if defined(_LOGGING_)

                if (CT.logMeth>1)
                {
                    sprintf(CT.logBuffer," %ld",u);
                    LogEnd(LH,CT.logBuffer);
                    LogEntry(LOG_METH2,"...New odd cycle found");
                    LH = NoHandle;
                    k++;
                }

                #endif
            }
            else
            {
                MakeIntegral(Q,w^1,w);
                TNode x = u;

                while (x!=w)
                {
                    TArc a = pred[x];
                    BalPush(a^1,0.5);
                    pred[x] = NoArc;
                    x = StartNode(a);   
                }

                while (w!=cu)
                {
                    a = pred[w];
                    BalPush(a,0.5);
                    pred[w] = NoArc;
                    if (w!=u) pred[w^1] = NoArc;
                    w = StartNode(a);   
                }

                v = cu;

                #if defined(_LOGGING_)

                if (CT.logMeth>1)
                {
                    sprintf(CT.logBuffer," %ld",u);
                    LogEnd(LH,CT.logBuffer);
                    LogEntry(LOG_METH2,"...Odd cycle cancelled");
                    LH = LogStart(LOG_METH2,"");
                    k--;
                }

                #endif

                Trace();
            }
        }

        #if defined(_PROGRESS_)

        ProgressStep(1);

        #endif
    }

    Close(H);
    ReleasePredecessors();

    #if defined(_LOGGING_)

    if (CT.logMeth>1)
    {
        LogEnd(LH);
        sprintf(CT.logBuffer,"...Keeping %ld odd cycles uncancelled",k);
        LogEntry(LOG_METH2,CT.logBuffer);
    }

    #endif

    CloseFold(ModCycleCancel);

    TFloat* potential = GetPotentials();

    if (potential)
    {
        LogEntry(LOG_METH,"Symmetrizing node potentials...");

        for (TNode i=0;i<n1;i++)
        {
            TFloat thisPi = (potential[2*i]-potential[2*i+1])/2;
            potential[2*i] = thisPi;
            potential[2*i+1] = -thisPi;
        }
    }
}


void abstractBalancedFNW::MakeIntegral(TArc* pred,TNode u,TNode v) throw(ERRange)
{
    TNode w = v;
    while (w!=u)
    {
        TArc a = pred[w];
        BalPush(a^1,0.5);
        pred[w] = NoArc;
        pred[w^1] = NoArc;
        w = StartNode(a);
    }
}
