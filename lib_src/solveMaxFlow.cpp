
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, July 2003
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file solveMaxFlow.cpp
/// \brief Maximum flow solver and methods

#include "sparseDigraph.h"
#include "denseDigraph.h"
#include "staticQueue.h"
#include "auxiliaryNetwork.h"


TFloat abstractMixedGraph::MaxFlow(TMethMXF method,TNode s,TNode t) throw(ERRange,ERRejected)
{
    #if defined(_FAILSAVE_)

    if (s>=n) NoSuchNode("MaxFlow",s);

    if (t>=n) NoSuchNode("MaxFlow",t);

    #endif

    // Methods apply to directed graphs only. In all other cases,
    // a complete orientation must be generated
    abstractDiGraph* G = NULL;

    if (IsDirected())
    {
        G = static_cast<abstractDiGraph*>(this);
    }
    else
    {
        G = new completeOrientation(*this);
        static_cast<completeOrientation*>(G) -> MapFlowForward(*this);
    }

    OpenFold(ModMaxFlow,NO_INDENT);

    if (CT.logMeth)
    {
        sprintf(CT.logBuffer,"Computing maximum (%ld,%ld)-flow...",s,t);
        LogEntry(LOG_METH,CT.logBuffer);
    }

    if (method==MXF_DEFAULT) method = TMethMXF(CT.methMXF);

    TFloat ret = InfCap;

    switch (method)
    {
        case MXF_SAP:
        {
            ret = G->MXF_EdmondsKarp(s,t);
            break;
        }
        case MXF_DINIC:
        {
            ret = G->MXF_Dinic(s,t);
            break;
        }
        case MXF_PREFLOW_FIFO:
        case MXF_PREFLOW_HIGH:
        case MXF_PREFLOW_SCALE:
        {
            ret = G->MXF_PushRelabel(s,t);
            G->BFS(SPX_RESIDUAL,s,t);
            break;
        }
        case MXF_SAP_SCALE:
        {
            ret = G->MXF_CapacityScaling(s,t);
            break;
        }
        default:
        {
            if (!IsDirected()) delete G;

            CloseFold(ModMaxFlow,NO_INDENT);
            UnknownOption("MaxFlow",method);
        }
    }

    if (!IsDirected())
    {
        static_cast<completeOrientation*>(G) -> MapFlowBackward(*this);

        TFloat* distG = G->GetDistanceLabels();
        TFloat* dist = InitDistanceLabels();

        for (TNode v=0;v<n;++v) dist[v] = distG[v];
    }

    #if defined(_FAILSAVE_)

    try
    {
        if (ret!=InfCap && CT.methFailSave==1 && G->CutCapacity()!=G->FlowValue(s,t))
        {
            if (!IsDirected()) delete G;
            InternalError("MaxFlow","Solutions are corrupted");
        }
    }
    catch (...)
    {
        if (!IsDirected()) delete G;
        InternalError("MaxFlow","Solutions are corrupted");
    }

    #endif

    if (!IsDirected()) delete G;

    CloseFold(ModMaxFlow,NO_INDENT);

    return ret;
}


TFloat abstractDiGraph::MXF_EdmondsKarp(TNode s,TNode t) throw(ERRange)
{
    OpenFold(ModEdmondsKarp,SHOW_TITLE);

    #if defined(_PROGRESS_)

    InitProgressCounter(m*(n-1.0)*m);
    TNode phase = 1;

    #endif

    TFloat val = 0;
    TArc a = First(s);
    do
    {
        if (!Blocking(a)) val += Flow(a);
        a = Right(a,s);
    }
    while (a!=First(s));

    SetLowerBound(TimerMaxFlow,val);

    TArc* pred = InitPredecessors();

    while (CT.SolverRunning() && BFS(SPX_RESIDUAL,s,t))
    {
        #if defined(_PROGRESS_)

        TFloat dt = Dist(t);

        if (dt>phase)
        {
            SetProgressCounter(m*(dt-1.0)*m);
            phase = TNode(dt);
        }

        #endif

        TCap Lambda = FindCap(pred,s,t);

        if (Lambda==InfCap)
        {
            CloseFold(ModMaxFlow);
            LogEntry(LOG_RES,"...Problem is unbounded");
            Trace(m);
            return InfCap;
        }

        Augment(pred,s,t,Lambda);
        val += Lambda;
        SetLowerBound(TimerMaxFlow,val);
        Trace(m);
    }

    ReleasePredecessors();

    if (CT.SolverRunning()) SetUpperBound(TimerMaxFlow,val);

    CloseFold(ModEdmondsKarp);

    return val;
}


TFloat abstractDiGraph::MXF_CapacityScaling(TNode s,TNode t) throw(ERRange)
{
    OpenFold(ModMaxFlow);

    LogEntry(LOG_METH,"(Capacity scaling method)");

    TFloat val = 0;
    TArc a = First(s);
    do
    {
        if (!Blocking(a)) val += Flow(a);
        a = Right(a,s);
    }
    while (a!=First(s));

    SetLowerBound(TimerMaxFlow,val);

    delta =  MaxUCap();

    if (delta>0) delta -= 1;

    #if defined(_PROGRESS_)

    double nPhases = 1;
    if (delta>0) nPhases = floor(log(delta)/log(double(2)))+2;
    InitProgressCounter(m*nPhases*m);

    #endif

    if (CT.logMeth)
    {
        sprintf(CT.logBuffer,"Starting with delta = %.0f",delta);
        LogEntry(LOG_METH,CT.logBuffer);
    }

    TArc* pred = InitPredecessors();

    while (CT.SolverRunning())
    {
        if (CT.logMeth)
        {
            sprintf(CT.logBuffer,"Next scaling phase, delta = %.0f",delta);
            LogEntry(LOG_METH,CT.logBuffer);
        }

        #if defined(_PROGRESS_)

        double newPhases = 1;
        if (delta>0) newPhases = floor(log(delta)/log(double(2)))+2;
        SetProgressCounter(m*(nPhases-newPhases)*m);

        #endif

        while (CT.SolverRunning() && BFS(SPX_RESIDUAL,s,t))
        {
            TCap Lambda = FindCap(pred,s,t);

            if (Lambda==InfCap)
            {
                CloseFold(ModMaxFlow);
                LogEntry(LOG_RES,"...Problem is unbounded");
                Trace(m);
                delta = 0;
                return InfCap;
            }

            Augment(pred,s,t,Lambda);
            val += Lambda;
            SetLowerBound(TimerMaxFlow,val);
            Trace(m);
        }

        if (delta==0) break;

        delta = floor(delta/2);
    }

    ReleasePredecessors();

    if (CT.SolverRunning()) SetUpperBound(TimerMaxFlow,val);

    CloseFold(ModMaxFlow);

    delta = 0;

    return val;
}


TFloat abstractDiGraph::MXF_Dinic(TNode s,TNode t) throw(ERRange)
{
    OpenFold(ModDinic,SHOW_TITLE);

    TArc* pred = InitPredecessors();
    layeredAuxNetwork Aux(*this,s);

    TFloat val = 0;
    TArc a = First(s);
    do
    {
        if (!Blocking(a)) val += Flow(a);
        a = Right(a,s);
    }
    while (a!=First(s));

    SetLowerBound(TimerMaxFlow,val);

    #if defined(_PROGRESS_)

    InitProgressCounter((n+1.0)*m*(n-1.0));

    #endif

    while (CT.SolverRunning())
    {
        TFloat* dist = InitDistanceLabels();
        dist[s] = 0;
        staticQueue<TNode,TFloat> Q(n,CT);
        Q.Insert(s);

        LogEntry(LOG_METH,"Graph is searched...");

        #if defined(_LOGGING_)

        OpenFold();
        THandle LH = LogStart(LOG_METH2,"Found props: ");

        #endif

        THandle H = Investigate();
        investigator &I = Investigator(H);

        while (!(Q.Empty()))
        {
            TNode u = Q.Delete();
            if (dist[u]==dist[t]) break;

            while (I.Active(u))
            {
                TArc a = I.Read(u);
                TNode v = EndNode(a);
                if (dist[v]>=dist[u]+1 && ResCap(a)>0)
                {
                    if (dist[v]==InfFloat)
                    {
                        dist[v] = dist[u]+1;
                        Q.Insert(v);
                    }
                    Aux.InsertProp(a);

                    #if defined(_LOGGING_)

                    if (CT.logMeth>1)
                    {
                        sprintf(CT.logBuffer,"%ld(%ld)%ld ",u,a,v);
                        LogAppend(LH,CT.logBuffer);
                    }

                    #endif
                }
            }
        }

        Close(H);

        #if defined(_LOGGING_)

        if (CT.logMeth>1) LogEnd(LH);
        CloseFold();

        #endif

        if (dist[t]==InfFloat) break;

        #if defined(_PROGRESS_)

        SetProgressCounter((n+1.0)*m*(dist[t]-1.0));

        #endif

        Trace(m);

        if (CT.logMeth)
        {
            sprintf(CT.logBuffer,"Phase %g augmenting...",dist[t]);
            LogEntry(LOG_METH,CT.logBuffer);
        }

        OpenFold();

        Aux.Phase2();

        while (!(Aux.Blocked(t)))
        {
            Trace(Aux,n);

            TFloat Lambda = Aux.FindPath(t);
            if (Lambda==InfCap)
            {
                LogEntry(LOG_RES,"...Problem is unbounded");
                Trace(n*m);
                CloseFold(ModDinic);
                return InfCap;
            }

            val += Lambda;

            TArc a;
            TNode w = t;

            OpenFold();

            while (w!=s)
            {
                a = pred[w];
                Push(a,Lambda);
                pred[w] = NoArc;
                w = StartNode(a);

                if (ResCap(a)==0) Aux.TopErasure(a);
            }

            CloseFold();
        }

        CloseFold();
        SetLowerBound(TimerMaxFlow,val);

        Aux.Phase1();
        Aux.Init();
    }

    ReleasePredecessors();

    if (CT.SolverRunning()) SetUpperBound(TimerMaxFlow,val);

    CloseFold(ModDinic);

    Trace();

    return val;
}


TFloat abstractDiGraph::MXF_PushRelabel(TNode s,TNode t) throw(ERRange)
{
    // Push-and-Relabel method which supports the following selection
    // strategies:
    //
    // methMXF == 2 : First-in first-out, O(n^3)
    // methMXF == 3 : Highest label, O(n^2 m^(1/2))
    // methMXF == 4 : Excess scaling, O(nm + n^2 log(U))

    #if defined(_PROGRESS_)

    InitProgressCounter(1);

    #endif

    OpenFold(ModPushRelabel,SHOW_TITLE);

    // A queue for the active nodes

    goblinQueue<TNode,TFloat> *Q = NULL;

    if (CT.methMXF!=2) Q = NewNodeHeap();
    else Q = new staticQueue<TNode,TFloat>(n,CT);

    // Additional incidence structure which is updated by relabel(u)
    // operations. Admissible[u] contains of all arcs a=uv such that
    // rescap(a)>0 and dist[u]=dist[v]+1.

    staticQueue<TNode,TFloat> **Admissible = new staticQueue<TNode,TFloat>*[n];
    Admissible[0] = new staticQueue<TArc,TFloat>(2*m,CT);

    for (TNode i=1;i<n;i++)
        Admissible[i] = new staticQueue<TArc,TFloat>(*Admissible[0]);

    // Temporary incidence lists used in relabel operations

    staticQueue<TArc,TFloat> S1(2*m,CT);
    staticQueue<TArc,TFloat> S2(S1);

    TNode* dist = InitNodeColours();
    for (TNode i=0;i<n;i++) dist[i]=0;
    dist[s] = n;

    // Push all flow away from the root node

    THandle H = Investigate();
    investigator &I = Investigator(H);
    bool unbounded = false;
    while (I.Active(s))
    {
        TArc a = I.Read(s);

        if (UCap(a)!=InfCap) Push(a,ResCap(a));
        else unbounded = true;
    }


    if (CT.methMXF==4)
    {
        delta = 0;

        for (TNode v=0;v<n;v++)
        {
            if (v!=t && v!=s && Divergence(v)>delta)
            {
                delta = Divergence(v);
            }
        }

        if (CT.logMeth)
        {
            sprintf(CT.logBuffer,"Starting with delta = %.0f",delta);
            LogEntry(LOG_METH,CT.logBuffer);
        }

        #if defined(_LOGGING_)

        THandle LH = LogStart(LOG_METH2,"Active nodes: ");

        #endif

        for (TNode v=0;v<n;v++)
        {
            if (v!=t && v!=s && 2*Divergence(v)>=delta)
            {
                Q -> Insert(v,0);

                #if defined(_LOGGING_)

                if (CT.logMeth>1)
                {
                    sprintf(CT.logBuffer,"%ld ",v);
                    LogAppend(LH,CT.logBuffer);
                }

                #endif
            }
        }

        #if defined(_LOGGING_)

        if (CT.logMeth>1) LogEnd(LH);

        #endif
    }
    else
    {
        for (TNode v=0;v<n;v++)
        {
            if (v!=t && v!=s && Divergence(v)>0) Q -> Insert(v,0);
        }
    }

    SetBounds(TimerMaxFlow,Divergence(t),-Divergence(s));

    #if defined(_PROGRESS_)

    SetProgressCounter(LowerBound(TimerMaxFlow) / UpperBound(TimerMaxFlow));

    #endif

    Trace();

    while (!(Q->Empty()) && !unbounded)
    {
        TNode u = Q->Delete();
        bool canRelabel = true;

        while (!Admissible[u]->Empty() && Divergence(u)>0)
        {
            TArc a = Admissible[u]->Delete();
            TNode v = EndNode(a);

            TFloat lambda = ResCap(a);
            if (lambda>Divergence(u)) lambda = Divergence(u);

            if (CT.methMXF==4)
            {
                if (v!=t && lambda>delta-Divergence(v))
                {
                    lambda = delta-Divergence(v);
                }

                // All non-saturating pushes must carry at least delta/2 units
                if (2*lambda<delta && lambda<ResCap(a))
                {
                    if (dist[u]==dist[v]+1) canRelabel = false;
                    continue;
                }
            }

            if (dist[u]==dist[v]+1)
            {
                #if defined(_LOGGING_)

                if (CT.logMeth>1)
                {
                    sprintf(CT.logBuffer,
                        "Push %g flow units from node %ld to node %ld",lambda,u,v);
                    LogEntry(LOG_METH2,CT.logBuffer);
                }

                #endif

                if (v!=t && v!=s)
                {
                    if (CT.methMXF!=4)
                    {
                        if (Divergence(v)==0) Q->Insert(v,-dist[v]);
                    }
                    else if (    2*Divergence(v)<delta
                              && 2*(Divergence(v)+lambda)>=delta
                            )
                    {
                        Q->Insert(v,dist[v]);

                        #if defined(_LOGGING_)

                        sprintf(CT.logBuffer,"Activating node %ld ",v);
                        LogEntry(LOG_METH2,CT.logBuffer);

                        #endif
                    }
                }

                Push(a,lambda);

                if (v==s) SetUpperBound(TimerMaxFlow,-Divergence(s));
                if (v==t) SetLowerBound(TimerMaxFlow,Divergence(t));

                #if defined(_PROGRESS_)

                SetProgressCounter(LowerBound(TimerMaxFlow)
                                    / UpperBound(TimerMaxFlow));

                #endif

                Trace();

                if (ResCap(a)>0) Admissible[u]->Insert(a);

                if (   CT.methMXF==4
                    && 2*Divergence(u)<delta
                   )
                {
                    canRelabel = false;
                    break;
                }
            }
        }

        if (canRelabel && Divergence(u)>0)
        {
            I.Reset(u);
            TNode dMin = NoNode;

            while (I.Active(u))
            {
                TArc a = I.Read(u);
                TNode v = EndNode(a);
                if (ResCap(a)>0 && dist[v]<=dMin)
                {
                    dMin = dist[v];
                    S1.Insert(a);
                }

                if (ResCap(a^1)>0 && dMin+2>=dist[v]) S2.Insert(a^1);
            }

            while (!S1.Empty())
            {
                TArc a = S1.Delete();
                TNode v = EndNode(a);

                if (dMin==dist[v]) Admissible[u]->Insert(a);
            }

            while (!S2.Empty())
            {
                TArc a = S2.Delete();
                TNode v = StartNode(a);

                if (dMin+2==dist[v]) Admissible[v]->Insert(a);
            }

            #if defined(_LOGGING_)

            if (CT.logMeth>1)
            {
                sprintf(CT.logBuffer,"Relabelled node %lu: %lu -> %lu",u,dist[u],dMin+1);
                LogEntry(LOG_METH2,CT.logBuffer);
            }

            #endif

            dist[u] = dMin+1;

            if (CT.methMXF!=4)
            {
                Q->Insert(u,-dist[u]);
            }
            else
            {
                Q->Insert(u,dist[u]);
            }

            Trace();
        }

        if (CT.methMXF==4 && Q->Empty() && delta>2)
        {
            delta = ceil(delta/2);

            if (CT.logMeth)
            {
                sprintf(CT.logBuffer,"Next scaling phase, delta = %.0f",delta);
                LogEntry(LOG_METH,CT.logBuffer);
            }

            #if defined(_LOGGING_)

            THandle LH = LogStart(LOG_METH2,"Active nodes: ");

            #endif

            for (TNode v=0;v<n;v++)
            {
                if (v!=t && v!=s && 2*Divergence(v)>=delta)
                {
                    Q->Insert(v,dist[v]);

                    #if defined(_LOGGING_)

                    if (CT.logMeth>1)
                    {
                        sprintf(CT.logBuffer,"%ld ",v);
                        LogAppend(LH,CT.logBuffer);
                    }

                    #endif
                }
            }

            #if defined(_LOGGING_)

            if (CT.logMeth>1) LogEnd(LH);

            #endif
        }
    }

    Close(H);

    delete Q;

    for (TNode i=1;i<n;i++) delete Admissible[i];
    delete Admissible[0];
    delete[] Admissible;

    ReleasePredecessors();

    if (unbounded) Error(ERR_RANGE,"MXF_PushRelabel",
        "Arcs emanating from source must have finite capacities");

    CloseFold(ModPushRelabel);

    return Divergence(t);
}
