
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, August 2003
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file   solveMaxBalFlow.cpp
/// \brief  Methods for non-weighted balanced network flow problems

#include "staticStack.h"
#include "shrinkingNetwork.h"
#include "balancedToBalanced.h"


TFloat abstractBalancedFNW::MaxBalFlow(TNode s) throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (s>=n) NoSuchNode("MaxBalFlow",s);

    #endif

    OpenFold(ModMaxBalFlow,NO_INDENT);

    LogEntry(LOG_METH,"Computing maximum balanced flow...");

    TFloat val = 0;

    switch (CT.methMaxBalFlow)
    {
        case 0:
        {
            val = BNSAndAugment(s);
            break;
        }
        case 1:
        case 2:
        case 3:
        {
            val = MicaliVazirani(s);
            break;
        }
        case 4:
        {
            val = BalancedScaling(s);
            break;
        }
        case 5:
        {
            val = Anstee(s);
            break;
        }
        default:
        {
            val = BNSAndAugment(s);
        }
    }

    #if defined(_FAILSAVE_)

    try
    {
        if (CT.methFailSave==1 && val!=FlowValue(s,s^1))
        {
            InternalError("MaxBalFlow","Solutions are corrupted");
        }
    }
    catch (...)
    {
        InternalError("MaxBalFlow","Solutions are corrupted");
    }

    #endif

    CloseFold(ModMaxBalFlow,NO_INDENT);

    return val;
}


TFloat abstractBalancedFNW::Anstee(TNode s) throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (s>=n) NoSuchNode("Anstee",s);

    #endif

    OpenFold(ModAnstee,SHOW_TITLE);

    #if defined(_PROGRESS_)

    InitProgressCounter(n*n+n*n1+n1);
    SetProgressNext(n*n);

    #endif

    TFloat val = MaxFlow(s,s^1);

    if (CT.SolverRunning()) SetUpperBound(TimerMaxBalFlow,val);

    #if defined(_PROGRESS_)

    SetProgressCounter(n*n);
    SetProgressNext(n1);

    #endif

    CancelEven();

    #if defined(_PROGRESS_)

    SetProgressCounter(n*n+n1);
    SetProgressNext(n*n1);

    #endif

    val = CancelOdd();

    if (CT.SolverRunning()) SetUpperBound(TimerMaxBalFlow,val);

    CloseFold(ModAnstee);

    return val;
}


TFloat abstractBalancedFNW::CancelOdd() throw()
{
    #if defined(_FAILSAVE_)

    if (Q==NULL) Error(ERR_REJECTED,"CancelOdd","No odd cycles present");

    #endif

    balancedToBalanced G(*this);

    LogEntry(LOG_METH,"Refining balanced flow...");

    return G.BNSAndAugment(G.Source());
}


TFloat abstractBalancedFNW::BNSAndAugment(TNode s) throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (s>=n) NoSuchNode("BNSAndAugment",s);

    #endif

    OpenFold(ModBalAugment,SHOW_TITLE);

    Symmetrize();

    TFloat* dist = InitDistanceLabels();
    TArc* pred = InitPredecessors();
    InitProps();
    InitPetals();
    InitBlossoms();

    TNode t = s^1;

    // Determine bounds and flow value

    TFloat val = 0;
    TCap cap = 0;
    THandle H = Investigate();

    while (Active(H,s))
    {
        TArc a = Read(H,s);

        if (a%2==0)
        {
            val += Flow(a);
            cap += UCap(a);
        }
        else val -= Flow(a^1);
    }

    Close(H);

    SetBounds(TimerMaxBalFlow,val,cap);

    #if defined(_PROGRESS_)

    InitProgressCounter(cap-val);
    SetProgressNext(2);

    #endif

    if (CT.methMaxBalFlow>5)       // Start heuristic a la greedy 
    {
        LogEntry(LOG_METH,"Balanced network is searched...");
        OpenFold();

        while (CT.SolverRunning() && BNSHeuristicsBF(s,t))
        {
            #if defined(_LOGGING_)

            CloseFold();
            LogEntry(LOG_METH2,"Expanding path for augmentation...");
            OpenFold();

            #endif

            Expand(dist,pred,s,t);
            CloseFold();
            TFloat Lambda = FindBalCap(pred,s,t);
            BalAugment(pred,s,t,Lambda);

            val += 2*Lambda;
            SetLowerBound(TimerMaxBalFlow,val);

            Trace((unsigned long)(2*Lambda));

            #if defined(_PROGRESS_)

            SetProgressNext(2);

            #endif

            if (dist[t]>CT.methMaxBalFlow-5) break;

            LogEntry(LOG_METH,"Balanced network is searched...");
            OpenFold();
        }

        CloseFold();
    }

    while (CT.SolverRunning() && BNS(s,t))
    {
        #if defined(_LOGGING_)

        LogEntry(LOG_METH2,"Expanding path for augmentation...");

        #endif

        OpenFold();
        Expand(dist,pred,s,t);
        CloseFold();
        TFloat Lambda = FindBalCap(pred,s,t);
        BalAugment(pred,s,t,Lambda);

        val += 2*Lambda;
        SetLowerBound(TimerMaxBalFlow,val);

        Trace((unsigned long)(2*Lambda));

        #if defined(_PROGRESS_)

        SetProgressNext(2);

        #endif
    }

    if (CT.SolverRunning()) SetUpperBound(TimerMaxBalFlow,val);

    ReleasePredecessors();
    ReleaseProps();
    ReleasePetals();

    CloseFold(ModBalAugment);

    return val;
}


TFloat abstractBalancedFNW::BalancedScaling(TNode s) throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (s>=n) NoSuchNode("BalancedScaling",s);

    #endif

    OpenFold(ModBalScaling,SHOW_TITLE);

    Symmetrize();

    // Determine bounds and flow value

    TFloat val = 0;
    TCap cap = 0;
    THandle H = Investigate();

    while (Active(H,s))
    {
        TArc a = Read(H,s);

        if (a%2==0)
        {
            val += Flow(a);
            cap += UCap(a);
        }
        else val += Flow(a^1);
    }

    Close(H);

    SetBounds(TimerMaxBalFlow,val,cap);

    #if defined(_PROGRESS_)

    InitProgressCounter(cap-val);

    #endif

    delta = MaxUCap();

    if (CT.logMeth)
    {
        sprintf(CT.logBuffer,"Starting with delta = %.0f",delta);
        LogEntry(LOG_METH,CT.logBuffer);
    }

    TNode t = s^1;
    TArc* pred = InitPredecessors();

    while (delta>1 && CT.SolverRunning())
    {
        delta = floor(delta/2);

        #if defined(_PROGRESS_)

        SetProgressNext(2*delta);

        #endif

        if (CT.logMeth)
        {
            sprintf(CT.logBuffer,"Next scaling phase, delta = %.0f",delta);
            LogEntry(LOG_METH,CT.logBuffer);
        }

        OpenFold();

        while (BFS(SPX_RESIDUAL,s,t) && CT.SolverRunning())
        {
            TCap Lambda = FindBalCap(pred,s,t);
            BalAugment(pred,s,t,Lambda);

            val += 2*Lambda;
            SetLowerBound(TimerMaxBalFlow,val);

            Trace((unsigned long)(2*Lambda));

            #if defined(_PROGRESS_)

            SetProgressNext(2*delta);

            #endif
        }

        CloseFold();
    }

    LogEntry(LOG_METH,"Final scaling phase...");

    val = BNSAndAugment(s);

    CloseFold(ModBalScaling);

    return val;
}


bool abstractBalancedFNW::BNS(TNode s,TNode t) throw(ERRange,ERRejected)
{
    LogEntry(LOG_METH,"Balanced network is searched...");

    bool ret = 0;

    switch (CT.methBNS)
    {
        case 0:
        {
            ret = BNSKocayStone(s,t);
            break;
        }
        case 1:
        case 2:
        {
            ret = BNSKamedaMunro(s,t);
            if (!ret) ret = BNSKocayStone(s,t);
            break;
        }
        case 3:
        {
            ret = BNSHeuristicsBF(s,t);
            if (!ret) ret = BNSKocayStone(s,t);
            break;
        }
        default:
        {
            UnknownOption("BNS",CT.methBNS);
            throw ERRejected();
        }
    }

    return ret;
}


void abstractBalancedFNW::Expand(TFloat* dist,TArc* pred,TNode x,TNode y) throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (dist[y]<dist[x]) Error(ERR_REJECTED,"Expand","Missing start node");

    #endif

    if (x!=y)
    {
        #if defined(_LOGGING_)

        THandle LH = NoHandle;

        if (CT.logMeth>1)
        {
            sprintf(CT.logBuffer,"Expand(%ld,%ld) puts ",x,y);
            LH = LogStart(LOG_METH2,CT.logBuffer);
        }

        #endif

        TArc a = prop[y];

        if (a!=NoArc)
        {
            pred[y] = a;

            #if defined(_LOGGING_)

            if (CT.logMeth>1)
            {
                sprintf(CT.logBuffer,"pred[%ld] = %ld (prop)",y,a);
                LogAppend(LH,CT.logBuffer);
            }

            #endif

            Expand(dist,pred,x,StartNode(a));
        }
        else
        {
            a = petal[y];
            TNode u = StartNode(a);
            TNode v = EndNode(a);
            pred[v] = a;

            #if defined(_LOGGING_)

            if (CT.logMeth>1)
            {
                sprintf(CT.logBuffer,"pred[%ld] = %ld (petal)",v,a);
                LogAppend(LH,CT.logBuffer);
            }

            #endif

            Expand(dist,pred,x,u);
            CoExpand(dist,pred,v,y);
        }

        #if defined(_LOGGING_)

        if (CT.logMeth>1) LogEnd(LH);

        #endif
    }
}


void abstractBalancedFNW::CoExpand(TFloat* dist,TArc* pred,TNode v,TNode y) throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (dist[ComplNode(v)]<dist[ComplNode(y)])
        Error(ERR_REJECTED,"CoExpand","Missing end node");

    #endif

    if (y!=v)
    {
        #if defined(_LOGGING_)

        THandle LH = NoHandle;

        if (CT.logMeth>1)
        {
            sprintf(CT.logBuffer,"CoExpand(%ld,%ld) puts ",v,y);
            LogEntry(LOG_METH2,CT.logBuffer);
        }

        #endif

        TNode cv = v^1;
        TArc a = prop[cv];

        if (a!=NoArc)
        {
            a = a^2;
            TNode x = EndNode(a);
            pred[x] = a;

            #if defined(_LOGGING_)

            if (CT.logMeth>1)
            {
                sprintf(CT.logBuffer,"pred[%ld] = %ld (co-prop)",x,a);
                LogAppend(LH,CT.logBuffer);
            }

            #endif

            CoExpand(dist,pred,x,y);
        }
        else
        {
            a = (petal[cv])^2;
            TNode u = StartNode(a);
            TNode w = EndNode(a);
            pred[w] = a;

            #if defined(_LOGGING_)

            if (CT.logMeth>1)
            {
                sprintf(CT.logBuffer,"pred[%ld] = %ld (petal)",w,a);
                LogAppend(LH,CT.logBuffer);
            }

            #endif

            Expand(dist,pred,v,u);
            CoExpand(dist,pred,w,y);
        }

        #if defined(_LOGGING_)

        if (CT.logMeth>1) LogEnd(LH);

        #endif
    }
}


bool abstractBalancedFNW::BNSKocayStone(TNode s,TNode t) throw(ERRange)
{
    OpenFold(ModBNSExact,SHOW_TITLE);

    #if defined(_PROGRESS_)

    InitProgressCounter(n-1);

    #endif

    InitBlossoms();
    TFloat* dist = InitDistanceLabels();
    InitProps();
    InitPetals();
    dist[s] = 0;
    Bud(s);
    THandle H = Investigate();
    investigator &I = Investigator(H);
    dynamicStack<TNode,TFloat> Support(n,CT);
    staticQueue<TNode,TFloat> Q(n,CT);
    Q.Insert(s);
    bool searching = true;

    while (!Q.Empty() && searching)
    {
        TNode u = Q.Delete();
        TNode cu = u^1;

        #if defined(_LOGGING_)

        if (CT.logMeth>1)
        {
            sprintf(CT.logBuffer,"Expanding node %ld",u);
            LogEntry(LOG_METH2,CT.logBuffer);
        }

        #endif

        OpenFold();

        while (I.Active(u) && searching)
        {
            TArc a = I.Read(u);
            TNode v = EndNode(a);
            TNode cv = v^1;

            if (dist[cv]==InfFloat)
            {
                if (dist[v]==InfFloat && BalCap(a)>0)
                {
                    // Generate Bud

                    #if defined(_LOGGING_)

                    if (CT.logMeth>1)
                    {
                        sprintf(CT.logBuffer,"New bud {%ld}",v);
                        LogEntry(LOG_METH2,CT.logBuffer);
                    }

                    #endif

                    dist[v] = dist[u]+1;
                    prop[v] = a;
                    Bud(v);
                    Q.Insert(v);
                    Trace(1);
                }
            }
            else
            {
                TNode x = Base(u);
                TNode y = Base(v);

                if (prop[u]!=(a^1) && prop[cu]!=(a^2) && BalCap(a)>0
                    && (x!=y || dist[v]==InfFloat))
                {
                    // Shrink Blossom //

                    TFloat tenacity = dist[u]+dist[cv]+1;

                    // Find common predecessor //

                    while (x!=y)
                    {
                        if (dist[x]>dist[y])
                        {
                            TNode z = x^1;

                            if (dist[z]==InfFloat)
                            {
                                petal[z] = a^2;
                                dist[z] = tenacity-dist[x];
                                Q.Insert(z);
                                Trace(1);
                            }

                            Support.Insert(x);
                            x = Pred(x); 
                        }
                        else
                        {
                            TNode z = y^1;

                            if (dist[z]==InfFloat)
                            {
                                petal[z] = a;
                                dist[z] = tenacity-dist[y];
                                Q.Insert(z);
                                Trace(1);
                            }

                            Support.Insert(y);
                            y = Pred(y); 
                        }
                    }

                    // Find base node //

                    while (x!=s && BalCap(prop[x])>1)
                    {
                        TNode z = x^1;

                        if (dist[z]==InfFloat)
                        {
                            petal[z] = a;
                            dist[z] = tenacity-dist[x];
                            Q.Insert(z);
                            Trace(1);
                        }

                        Support.Insert(x);
                        x = Pred(x); 
                    }

                    TNode z = x^1;

                    if (dist[z]==InfFloat)
                    {
                        petal[z] = a;
                        dist[z] = tenacity-dist[x];
                        Q.Insert(z);
                        Trace(1);
                    }

                    // Unify Blossoms //

                    #if defined(_LOGGING_)

                    THandle LH = NoHandle;

                    if (CT.logMeth>1)
                    {
                        sprintf(CT.logBuffer,"Shrinking %ld",x);
                        LH = LogStart(LOG_METH2,CT.logBuffer);
                    }

                    #endif

                    while (!Support.Empty())
                    {
                        y = Support.Delete();
                        Shrink(x,y);

                        #if defined(_LOGGING_)

                        if (CT.logMeth>1)
                        {
                            sprintf(CT.logBuffer,",%ld",y);
                            LogAppend(LH,CT.logBuffer);
                        }

                        #endif
                    }

                    #if defined(_LOGGING_)

                    if (CT.logMeth>1)
                    {
                        sprintf(CT.logBuffer,"(tenacity %g)",tenacity);
                        LogEnd(LH,CT.logBuffer);
                    }

                    #endif
                }
            }

            if (t!=NoNode && dist[t]<InfFloat) searching = false;
        }

        CloseFold();
    }

    Close(H);

    CloseFold(ModBNSExact);

    return dist[t]<InfFloat;
}


bool abstractBalancedFNW::BNSKamedaMunro(TNode s,TNode t) throw(ERRange)
{
    OpenFold(ModBNSDepth,SHOW_TITLE);

    #if defined(_PROGRESS_)

    InitProgressCounter(n-1);

    #endif

    TFloat* dist = InitDistanceLabels();
    InitProps();
    InitPetals();
    dist[s] = 0;
    THandle H = Investigate();
    investigator& I = Investigator(H);
    dynamicStack<TNode,TFloat> S1(n,CT);
    staticStack<TNode,TFloat> S2(n,CT);
    TNode u = s;
    TNode* timeStamp = NULL;
    TNode tsCount = 1;

    if (CT.methBNS==2)
    {
        timeStamp = new TNode[n];
        timeStamp[s] = 0;
    }

    while (u!=NoNode)
    {
        TNode cu = u^1;

        if (!(I.Active(u)))
        {
            // Backtracking

            if (dist[cu]==InfFloat)
            {
                if (u==s) u = NoNode;
                else u = S1.Delete();
            }
            else
            {
                if (S2.Peek()==u) S2.Delete();

                if (!S2.Empty() && dist[S1.Peek()]<=dist[S2.Peek()]) u = S2.Peek();
                else
                {
                    u = S1.Delete();
                    if (S1.Empty()) u = NoNode;
                    else u = S1.Delete();
                }
            }

            #if defined(_LOGGING_)

            if (u!=NoNode && CT.logMeth>1)
            {
                sprintf(CT.logBuffer,"Backtracking to %ld",u);
                LogEntry(LOG_METH2,CT.logBuffer);
            }

            #endif

            continue;
        }

        TArc a = I.Read(u);
        TNode v = EndNode(a);
        TNode cv = ComplNode(v);

        if (dist[cv]==InfFloat)
        {
            if (dist[v]==InfFloat && BalCap(a)>0)
            {
                // Generate Bud

                dist[v] = dist[u]+1;

                if (CT.methBNS==2)
                {
                    timeStamp[v] = tsCount++;

                    #if defined(_LOGGING_)

                    if (CT.logMeth>1)
                    {
                        sprintf(CT.logBuffer,"New bud {%ld}, timestamp = %ld",
                            v,timeStamp[v]);
                        LogEntry(LOG_METH2,CT.logBuffer);
                    }

                    #endif
                }
                else
                {
                    #if defined(_LOGGING_)

                    if (CT.logMeth>1)
                    {
                        sprintf(CT.logBuffer,"New bud {%ld}",v);
                        LogEntry(LOG_METH2,CT.logBuffer);
                    }

                    #endif
                }

                prop[v] = a;
                S1.Insert(u);
                u = v;

                Trace(1);
            }
        } 
        else
        {
            if (BalCap(a)>0 && prop[cu]!=(a^2) && 
                ((CT.methBNS==2 && prop[u]!=(a^1)) ||
                 (CT.methBNS!=2 && dist[v]==InfFloat)
                )
               )
            {
                // Shrink Blossom //

                TFloat tenacity = dist[u]+dist[cv]+1;
                bool shrunk = false;

                #if defined(_LOGGING_)

                THandle LH = NoHandle;

                #endif

                if (dist[cu]!=InfFloat) u = S1.Delete();
                else
                {
                    #if defined(_LOGGING_)

                    if (CT.logMeth>1)
                    {
                        sprintf(CT.logBuffer,"Shrinking %ld",u);
                        LH = LogStart(LOG_METH2,CT.logBuffer);
                    }

                    #endif

                    shrunk = true;
                    petal[cu] = a^2;
                    dist[cu] = tenacity-dist[u];

                    if (CT.methBNS==2) timeStamp[cu] = tsCount++;

                    S2.Insert(cu);
                    S2.Insert(u);

                    Trace(1);
                }

                while (!S1.Empty() &&
                        (BalCap(prop[u])>1
                         || (CT.methBNS!=2 && dist[u]>dist[cv])
                         || (CT.methBNS==2 && timeStamp[u]>timeStamp[cv]) ) )
                {
                    if (!shrunk)
                    {
                        #if defined(_LOGGING_)

                        if (CT.logMeth>1)
                        {
                            sprintf(CT.logBuffer,"Shrinking %ld",u);
                            LH = LogStart(LOG_METH2,CT.logBuffer);
                        }

                        #endif

                        shrunk = true;
                    }

                    u = S1.Delete();
                    cu = u^1;

                    if (dist[cu]!=InfFloat) u = S1.Delete();
                    else
                    {
                        petal[cu] = a^2;
                        dist[cu] = tenacity-dist[u];

                        if (CT.methBNS==2) timeStamp[cu] = tsCount++;

                        S2.Insert(cu);
                        S2.Insert(u);

                        Trace(1);
                    }

                    #if defined(_LOGGING_)

                    if (CT.logMeth>1)
                    {
                        sprintf(CT.logBuffer,",%ld",u);
                        LogAppend(LH,CT.logBuffer);
                    }

                    #endif
                }

                #if defined(_LOGGING_)

                if (shrunk && CT.logMeth>1)
                {
                    sprintf(CT.logBuffer," (tenacity %g, target %ld)",
                        tenacity,v);
                    LogEnd(LH,CT.logBuffer);
                }

                #endif

                S1.Insert(u);
                u = S2.Peek();

                #if defined(_LOGGING_)

                if (shrunk && CT.logMeth>1)
                {
                    sprintf(CT.logBuffer,"Considering node %ld",u);
                    LogEntry(LOG_METH2,CT.logBuffer);
                }

                #endif
            }
        }

        if (t!=NoNode && dist[t]<InfFloat) break;
    }

    Close(H);

    if (CT.methBNS==2) delete[] timeStamp;

    CloseFold(ModBNSDepth);

    return dist[t]<InfFloat;
}


bool abstractBalancedFNW::BNSHeuristicsBF(TNode s,TNode t) throw(ERRange)
{
    OpenFold(ModBNSBreadth,SHOW_TITLE);

    #if defined(_PROGRESS_)

    InitProgressCounter(n-1);

    #endif

    #if defined(_LOGGING_)

    THandle LH =  LogStart(LOG_METH2,"Expanded nodes: ");

    #endif

    TFloat* dist = InitDistanceLabels();
    InitProps();
    dist[s] = 0;
    THandle H = Investigate();
    investigator &I = Investigator(H);
    staticQueue<TNode,TFloat> Q(n,CT);
    Q.Insert(s);
    bool searching = true;
    TNode cs = s^1;

    while (!Q.Empty() && searching)
    {
        TNode u = Q.Delete();

        #if defined(_LOGGING_)

        if (CT.logMeth>1)
        {
            sprintf(CT.logBuffer,"%ld,",u);
            LogAppend(LH,CT.logBuffer);
        }

        #endif

        while (I.Active(u) && searching)
        {
            TArc a = I.Read(u);
            TNode v = EndNode(a);

            if (dist[v]!=InfFloat || BalCap(a)==0) continue;

            TNode cv = v^1;

            if (dist[cv]==InfFloat)
            {
                dist[v] = dist[u]+1;
                prop[v] = a;

                if (v==t) searching = false;
                else Q.Insert(v);

                Trace(1);

                continue;
            }

            TNode x = u;
            TNode y = cv;
            bool blocked = false;

            while (x!=y && !blocked)
            {
                if (prop[x]==(a^2) && BalCap(prop[x])==1)
                       blocked = true;

                if (dist[x]>dist[y]) x = StartNode(prop[x]); 
                else y = StartNode(prop[y]); 
            }

            if (!blocked)
            {
                while (x!=s && BalCap(prop[x])>1) x = StartNode(prop[x]);

                if (x==s && t==cs && cv!=s)
                {
                    petal[t] = a;
                    dist[t] = dist[u]+dist[cv]+1;
                    searching = false;
                }
                else
                {
                    dist[v] = dist[u]+1;
                    prop[v] = a;

                    if (v==t) searching = false;
                    else Q.Insert(v);

                    Trace(1);
                }
            }
        }
    }

    #if defined(_LOGGING_)

    if (CT.logMeth>1) LogEnd(LH);

    #endif

    Close(H);

    CloseFold(ModBNSBreadth);

    return dist[t]<InfFloat;
}


bool abstractBalancedFNW::BNSMicaliVazirani(TNode s,TNode t) throw(ERRange)
{
    MicaliVazirani(s,t);

    return Dist(t)<InfFloat;
}


TFloat abstractBalancedFNW::MicaliVazirani(TNode s,TNode tt) throw(ERRange)
{
    OpenFold(ModMicaliVazirani,SHOW_TITLE);

    Symmetrize();

    // Determine bounds and flow value

    TFloat val = 0;
    TCap cap = 0;
    THandle H = Investigate();

    while (Active(H,s))
    {
        TArc a = Read(H,s);

        if (a%2==0)
        {
            val += Flow(a);
            cap += UCap(a);
        }
        else val += Flow(a^1);
    }

    Reset(H);

    SetBounds(TimerMaxBalFlow,val,cap);

    #if defined(_PROGRESS_)

    if (tt!=NoNode) InitProgressCounter(m);
    else InitProgressCounter(n*(n-1.0)*m);

    SetProgressNext(0.0);

    #endif

    InitBlossoms();
    TFloat* dist = InitDistanceLabels();
    TArc* pred = InitPredecessors();

    staticQueue<TNode,TFloat>** Q = new staticQueue<TNode,TFloat>*[n];
    staticQueue<TArc,TFloat>** Anomalies = new staticQueue<TArc,TFloat>*[n];
    staticQueue<TArc,TFloat>** Bridges = new staticQueue<TArc,TFloat>*[2*n];

    Q[0] = new staticQueue<TNode,TFloat>(n,CT);
    Bridges[0] = new staticQueue<TArc,TFloat>(2*m,CT);

    for (TNode i=1;i<n;i++)
    {
        Q[i] = NULL;
        Anomalies[i] = NULL;
    }

    for (TNode i=1;i<2*n;i++) Bridges[i] = NULL;

    Anomalies[0] = NULL;
    TNode t = s^1;
    Anomalies[t] = new staticQueue<TArc,TFloat>(2*m,CT);

    layeredShrNetwork Aux(*this,s,Q,Anomalies,Bridges);

    TNode augmentations = 1;
    investigator &I = Investigator(H);

    while (augmentations>0 && CT.SolverRunning())
    {
        LogEntry(LOG_METH,"Building layered shrinking graph...");

        InitBlossoms();
        InitDistanceLabels();
        InitPredecessors();
        dist[s] = 0;
        Bud(s);
        Q[0] -> Insert(s);

        I.Reset();
        augmentations = 0;

        for (TNode i=0;i<n && augmentations==0;i++)
        {
            // Exploring minlevel nodes

            if (CT.logMeth && Q[i]!=NULL && !Q[i]->Empty())
            {
                sprintf(CT.logBuffer,
                    "Exploring minlevel nodes with distance %ld...",i+1);
                LogEntry(LOG_METH2,CT.logBuffer);
            }

            #if defined(_LOGGING_)

            if (CT.logMeth>1) OpenFold();

            #endif

            while (Q[i]!=NULL && !Q[i]->Empty())
            {
                TNode v = Q[i]->Delete();
                TNode cv = v^1;

                #if defined(_LOGGING_)

                if (CT.logMeth>1)
                {
                    sprintf(CT.logBuffer,"Expanding node %ld",v);
                    LogEntry(LOG_METH2,CT.logBuffer);
                    OpenFold();
                }

                #endif

                while (I.Active(v))
                {
                    TArc a = I.Read(v);
                    TNode w = EndNode(a);
                    TNode cw = w^1;
                    if (dist[cw]==InfFloat && dist[w]>i)
                    {
                        if (BalCap(a)>0)
                        {
                            if (dist[w]==InfFloat && BalCap(a)>0)
                            {
                                // Generate Bud

                                #if defined(_LOGGING_)

                                if (CT.logMeth>1)
                                {
                                    sprintf(CT.logBuffer,"Node %ld explored",w);
                                    LogEntry(LOG_METH2,CT.logBuffer);
                                }

                                #endif

                                dist[w] = i+1;
                                Bud(w);

                                if (Q[i+1]==NULL)
                                    Q[i+1] = new staticQueue<TNode,TFloat>(*Q[0]);

                                Q[i+1] -> Insert(w);
                            }

                            Aux.InsertProp(a);

                            #if defined(_LOGGING_)

                            if (CT.logMeth>1)
                            {
                                sprintf(CT.logBuffer,
                                    "Prop %ld with end node %ld inserted",a,w);
                                LogEntry(LOG_METH2,CT.logBuffer);
                            }

                            #endif
                        }
                    }
                    else
                    {
                        if (BalCap(a)>0 && (dist[cv]!=dist[cw]+1 || dist[cv]>dist[v]))
                        {
                            if (dist[cw]==InfFloat)
                            {
                                #if defined(_LOGGING_)

                                if (CT.logMeth>1)
                                {
                                    sprintf(CT.logBuffer,
                                        "Anomaly %ld with end node %ld detected",a,w);
                                    LogEntry(LOG_METH2,CT.logBuffer);
                                }

                                #endif

                                if (Anomalies[cw]==NULL)
                                    Anomalies[cw] = new  staticQueue<TArc,TFloat>(*Anomalies[t]);

                                Anomalies[cw] -> Insert(a);
                            }
                            else
                            {
                                TNode index = (TNode)(dist[v]+dist[cw]+1);

                                if (Bridges[index]==NULL)
                                    Bridges[index] = new staticQueue<TArc,TFloat>(*Bridges[0]);

                                if (index>=dist[v]) Bridges[index] -> Insert(a);

                                #if defined(_LOGGING_)

                                if (CT.logMeth>1)
                                {
                                    sprintf(CT.logBuffer,
                                        "Bridge %ld with tenacity %ld detected",a,index);
                                    LogEntry(LOG_METH2,CT.logBuffer);
                                }

                                #endif
                            }
                        }
                    }
                }

                #if defined(_LOGGING_)

                if (CT.logMeth>1) CloseFold();

                #endif

            }

            #if defined(_LOGGING_)

            if (CT.logMeth>1) CloseFold();

            #endif

            if (i>0) {delete Q[i]; Q[i]=NULL;};

            // Exploring maxlevel nodes, shrinking blossoms, augmentation 

            Aux.Phase2();

            TNode tenacity = 2*i;
            if (Bridges[tenacity]!=NULL)
            {
                if (CT.logMeth && !Bridges[tenacity]->Empty())
                {
                    sprintf(CT.logBuffer,
                        "Exploring maxlevel nodes with tenacity %ld...",tenacity);
                    LogEntry(LOG_METH2,CT.logBuffer);
                }

                #if defined(_LOGGING_)

                OpenFold();

                #endif

                while (!Bridges[tenacity]->Empty())
                {
                    TArc a = Bridges[tenacity]->Delete();
                    TNode u = Base(StartNode(a));
                    TNode v = Base(EndNode(a));
                    if ((u!=v || dist[u^1]==InfFloat) && BalCap(a)>0 &&
                        !Aux.Blocked(u) && !Aux.Blocked(v)
                       )
                    {
                        TNode b = Aux.DDFS(a);

                        if (b!=s || tt!=NoNode)
                            Aux.ShrinkBlossom(b,a,tenacity);
                        else
                        {
                            Aux.Augment(a);
                            augmentations ++;

                            if (BalCap(a)>0 && !Aux.Blocked(u) && !Aux.Blocked(v))
                                Bridges[tenacity] -> Insert(a);
                        }
                    }
                }

                #if defined(_LOGGING_)

                CloseFold();

                #endif
            }

            tenacity++;

            if (Bridges[tenacity]!=NULL)
            {
                if (CT.logMeth && !Bridges[tenacity]->Empty())
                {
                    sprintf(CT.logBuffer,
                        "Exploring maxlevel nodes with tenacity %ld...",tenacity);
                    LogEntry(LOG_METH2,CT.logBuffer);
                }

                #if defined(_LOGGING_)

                OpenFold();

                #endif

                while (!Bridges[tenacity]->Empty())
                {
                    TArc a = Bridges[tenacity]->Delete();
                    TNode u = Base(StartNode(a));
                    TNode v = Base(EndNode(a));

                    if ((u!=v || dist[u^1]==InfFloat) && BalCap(a)>0 &&
                        !Aux.Blocked(u) && !Aux.Blocked(v)
                       )
                    {
                        TNode b = Aux.DDFS(a);

                        if (b!=s || tt!=NoNode)
                            Aux.ShrinkBlossom(b,a,tenacity);
                        else
                        {
                            TFloat Lambda = Aux.Augment(a);
                            augmentations ++;

                            if (BalCap(a)>0 && !Aux.Blocked(u) && !Aux.Blocked(v))
                                Bridges[tenacity] -> Insert(a);

                            val += 2*Lambda;
                            SetLowerBound(TimerMaxBalFlow,val);

                            #if defined(_PROGRESS_)

                            ProgressStep(n);

                            #endif
                        }
                    }
                }

                #if defined(_LOGGING_)

                CloseFold();

                #endif
            }

            if (i>0) {delete Bridges[2*i+1]; Bridges[2*i+1]=NULL;};

            if (augmentations>0)
            {
                if (CT.logMeth)
                {
                    sprintf(CT.logBuffer,
                        "...Phase %ld complete",i);
                    LogEntry(LOG_METH,CT.logBuffer);
                }

                #if defined(_PROGRESS_)

                SetProgressCounter(n*(tenacity+1.0)*m);

                #endif
            }

            Trace();

            Aux.Phase1();
        }

        if ((CT.methMaxBalFlow==2 || CT.methMaxBalFlow==3) && augmentations>0)
        {
            LogEntry(LOG_METH,"Checking for maximality...");

            if (!BNS(s,t)) augmentations = 0;

            if (CT.methMaxBalFlow==3 && augmentations==1)
            {
                #if defined(_LOGGING_)

                LogEntry(LOG_METH2,"Performing immediate augmentation...");

                #endif

                Expand(dist,pred,s,t);
                TFloat Lambda = FindBalCap(pred,s,t);
                BalAugment(pred,s,t,Lambda);

                val += 2*Lambda;
                SetLowerBound(TimerMaxBalFlow,val);
            }
        }

        for (TNode i=0;i<n;i++)
        {
            if (i>0 && Q[i]!=NULL)
            {
                delete Q[i];
                Q[i] = NULL;
            }

            if (i!=t && Anomalies[i]!=NULL)
            {
                delete Anomalies[i];
                Anomalies[i] = NULL;
            }
        }

        for (TNode i=0;i<2*n;i++)
            if (i>0 && Bridges[i]!=NULL)
            {
                delete Bridges[i];
                Bridges[i] = NULL;
            }

        while (!Anomalies[t]->Empty()) Anomalies[t]->Delete();

        if (tt!=NoNode && dist[tt]!=InfFloat)
        {
            #if defined(_LOGGING_)

            LogEntry(LOG_METH2,"Extracting valid path of minimum length...");

            #endif

            Aux.Expand(s,tt);
        }
        else Aux.Init();
    }

    Close(H);

    delete Q[0];
    delete Anomalies[t];
    delete Bridges[0];

    delete[] Anomalies;
    delete[] Q;
    delete[] Bridges;

    ReleasePredecessors();

    CloseFold(ModMicaliVazirani);

    return val;
}
