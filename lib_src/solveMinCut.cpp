
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, July 2003
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file   solveMinCut.cpp
/// \brief  All codes for determining node and edge connectivity numbers

#include "sparseDigraph.h"
#include "staticQueue.h"
#include "sparseGraph.h"


TCap abstractMixedGraph::NodeConnectivity(TNode source,TNode target,TOptNodeSplitting mode) throw()
{
    #if defined(_FAILSAVE_)

    if (source>=n && source!=NoNode) NoSuchNode("NodeConnectivity",source);

    if (target>=n && target!=NoNode) NoSuchNode("NodeConnectivity",target);

    #endif

    OpenFold(ModMinCut,NO_INDENT);

    LogEntry(LOG_METH,"Computing node connectivity...");
    OpenFold();

    nodeSplitting G(*this,MCC_MAP_UNDERLYING | mode);
    TCap lambda = InfFloat;

    if (source<n)
    {
        #if defined(_FAILSAVE_)

        if (target==NoNode)
            Error(ERR_RANGE,"NodeConnectivity","Missing right-hand node");

        if (Adjacency(source,target,ADJ_SEARCH)!=NoArc)
            Error(ERR_RANGE,"NodeConnectivity","Nodes must be non-adjacent");

        #endif

        lambda = G.MCC_StrongEdgeConnectivity(2*source+1,2*target);
        G.MapEdgeCut();
    }
    else
    {
        #if defined(_FAILSAVE_)

        if (target<n)
        {
            Error(ERR_RANGE,"NodeConnectivity","Missing left-hand node");
        }

        #endif

        #if defined(_PROGRESS_)

        InitProgressCounter((n*(n-1))/2);
        SetProgressNext(1);

        #endif

        for (TNode v=0;v<n && CT.SolverRunning();v++)
        {
            for (TNode w=v+1;w<n;w++)
            {
                if (Adjacency(v,w,ADJ_SEARCH)!=NoArc)
                {
                    #if defined(_PROGRESS_)

                    ProgressStep(1);

                    #endif

                    continue;
                }

                CT.SuppressLogging();
                TCap val = G.MCC_StrongEdgeConnectivity(2*v+1,2*w);
                CT.RestoreLogging();

                #if defined(_PROGRESS_)

                ProgressStep(1);

                #endif

                if (val>=lambda) continue;

                #if defined(_LOGGING_)

                if (CT.logRes>=2)
                {
                    sprintf(CT.logBuffer,
                        "Minimum (%ld,%ld)-cut has capacity %g",v,w,val);
                    LogEntry(LOG_RES2,CT.logBuffer);
                }

                #endif

                SetUpperBound(TimerMinCut,val);

                lambda = val;
                G.MapEdgeCut();

                Trace();
            }
        }
    }

    if (CT.SolverRunning()) SetBounds(TimerMinCut,lambda,lambda);

    CloseFold();
    sprintf(CT.logBuffer,"...Node connectivity is %g",lambda);
    LogEntry(LOG_RES,CT.logBuffer);
    CloseFold(ModMinCut,NO_INDENT);

    return lambda;
}


void nodeSplitting::MapEdgeCut() throw()
{
    TNode* chi = GetNodeColours();
    TNode* chiG = G.RawNodeColours();

    for (TNode u=0;u<G.N();u++)
    {
        if (chi[2*u+1]==CONN_LEFT_HAND)
        {
            chiG[u] = CONN_LEFT_HAND;
        }
        else if (chi[2*u]==CONN_LEFT_HAND)
        {
            chiG[u] = CONN_CUT_NODE;
        }
        else
        {
            chiG[u] = CONN_RIGHT_HAND;
        }
    }

    if (!mapCapacities)
    {
        // By the previous rule, cut edge are still possible, even if
        // there are equivalent node cuts. Do some local modifications,
        // at least for the standerd setting with unit capacities.
        for (TArc a=0;a<2*G.M();a++)
        {
            TNode x = G.StartNode(a);
            TNode y = G.EndNode(a);

            if (   chiG[x]==CONN_LEFT_HAND
                && chiG[y]==CONN_RIGHT_HAND
                && G.Demand(y)==1
                && (!G.Blocking(a) || mapUnderlying)
            )
            {
                chiG[y] = CONN_CUT_NODE;
            }
        }
    }
}


TCap abstractMixedGraph::EdgeConnectivity(TMethMCC method,TNode source,TNode target)
    throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (source>=n && source!=NoNode) NoSuchNode("EdgeConnectivity",source);

    if (target>=n && target!=NoNode) NoSuchNode("EdgeConnectivity",target);

    #endif

    OpenFold(ModMinCut,NO_INDENT);

    LogEntry(LOG_METH,"Computing edge connectivity...");

    TCap lambda = InfCap;
    TNode* chi = RawNodeColours();

    if (method==MCC_DEFAULT) method = TMethMCC(CT.methMCC);

    if (method==MCC_IDENTIFICATION && !(source!=NoNode && target!=NoNode))
    {
        lambda = MCC_NodeIdentification();
    }
    else
    {
        diGraph G(n,CT);
        graphRepresentation* GR = G.Representation();
        GR -> SetCDemand(0);

        for (TArc a=0;a<2*m;a++)
        {
            TNode x = StartNode(a);
            TNode y = EndNode(a);
            G.InsertArc(x,y,UCap(a),0);
        }

        #if defined(_TRACING_)

        for (TNode u=0;u<n && Dim()>0;u++)
        {
            GR -> SetC(u,0,C(u,0));
            GR -> SetC(u,1,C(u,1));
        }

        #endif

        TNode* chiG = G.RawNodeColours();

        if (source!=NoNode && target!=NoNode)
        {
            lambda = G.MCC_StrongEdgeConnectivity(source,target);
        }
        else if (method!=MCC_MAXFLOW)
        {
            if (source!=NoNode)
            {
                lambda = G.MCC_HaoOrlin(method,source);
            }
            else if (target!=NoNode)
            {
                lambda = G.MCC_HaoOrlin(method,target);
            }
            else
            {
                lambda = G.MCC_HaoOrlin(method,0);
            }
        }
        else
        {
            lambda = G.MCC_StrongEdgeConnectivity();
        }

        for (TNode u=0;u<n;u++) chi[u] = chiG[u];
    }

    CloseFold(ModMinCut,NO_INDENT);

    if (   (source!=NoNode && chi[source]==CONN_RIGHT_HAND)
        || (target!=NoNode && chi[target]==CONN_LEFT_HAND) )
    {
        // Revert the cut so that source or target is in the desired part
        for (TNode u=0;u<n;u++)
        {
            chi[u] = (chi[u]==CONN_RIGHT_HAND) ? CONN_LEFT_HAND : CONN_RIGHT_HAND;
        }
    }

    return lambda;
}


TCap abstractMixedGraph::StrongNodeConnectivity(TNode source,TNode target,TOptNodeSplitting mode) throw()
{
    if (m==0 || (COrientation() && Orientation(0)==0))
    {
        // Graph is undirected, at least implicitly
        return NodeConnectivity(source,target,mode);
    }

    #if defined(_FAILSAVE_)

    if (source>=n && source!=NoNode) NoSuchNode("StrongNodeConnectivity",source);

    if (target>=n && target!=NoNode) NoSuchNode("StrongNodeConnectivity",target);

    #endif

    OpenFold(ModStrongConn,NO_INDENT);

    LogEntry(LOG_METH,"Computing strong node connectivity...");
    OpenFold();

    nodeSplitting G(*this,mode & MCC_MAP_DEMANDS);
    TCap lambda = InfFloat;

    if (source<n)
    {
        #if defined(_FAILSAVE_)

        if (target==NoNode)
            Error(ERR_RANGE,"StrongNodeConnectivity","Missing right-hand node");

        if (Adjacency(source,target,ADJ_SEARCH)!=NoArc)
            Error(ERR_RANGE,"StrongNodeConnectivity","Nodes must be non-adjacent");

        #endif

        lambda = G.MCC_StrongEdgeConnectivity(2*source+1,2*target);
        G.MapEdgeCut();
    }
    else
    {
        #if defined(_FAILSAVE_)

        if (target<n)
        {
            Error(ERR_RANGE,"StrongNodeConnectivity","Missing left-hand node");
        }

        #endif

        #if defined(_PROGRESS_)

        InitProgressCounter(n*(n-1));
        SetProgressNext(1);

        #endif

        for (TNode v=0;v<n && CT.SolverRunning();v++)
        {
            for (TNode w=0;w<n && CT.SolverRunning();w++)
            {
                if (v==w) continue;

                TArc a = Adjacency(v,w,ADJ_SEARCH);

                if (a!=NoArc && !Blocking(a))
                {
                    #if defined(_PROGRESS_)

                    ProgressStep(1);

                    #endif

                    continue;
                }

                CT.SuppressLogging();
                TCap val = G.MCC_StrongEdgeConnectivity(2*v+1,2*w);
                CT.RestoreLogging();

                #if defined(_PROGRESS_)

                ProgressStep(1);

                #endif

                if (val>=lambda) continue;

                #if defined(_LOGGING_)

                if (CT.logRes>=2)
                {
                    sprintf(CT.logBuffer,
                        "Minimum (%ld,%ld)-cut has capacity %g",v,w,val);
                    LogEntry(LOG_RES2,CT.logBuffer);
                }

                #endif

                SetUpperBound(TimerMinCut,val);

                lambda = val;
                G.MapEdgeCut();

                Trace();
            }
        }
    }

    if (CT.SolverRunning()) SetBounds(TimerStrongConn,lambda,lambda);

    CloseFold();
    sprintf(CT.logBuffer,"...Strong node connectivity is %g",lambda);
    LogEntry(LOG_RES,CT.logBuffer);
    CloseFold(ModStrongConn,NO_INDENT);

    return lambda;
}


TCap abstractMixedGraph::MCC_StrongEdgeConnectivity() throw()
{
    OpenFold(ModSchnorr,NO_INDENT);

    #if defined(_PROGRESS_)

    InitProgressCounter(n);
    SetProgressNext(1);

    #endif

    LogEntry(LOG_METH,"Computing strong edge connectivity...");
    OpenFold();

    diGraph G(*this,OPT_PARALLELS);
    G.Representation() -> SetCDemand(0);

    TNode* chi = RawNodeColours();
    TNode* chiG = G.RawNodeColours();

    TCap lambda = InfFloat;

    for (TNode source=0;source<n && CT.SolverRunning();source++)
    {
        TNode target = (source+1)%n;
        CT.SuppressLogging();
        TCap val = G.MCC_StrongEdgeConnectivity(source,target);
        CT.RestoreLogging();

        #if defined(_PROGRESS_)

        ProgressStep(1);

        #endif

        if (val>=lambda) continue;

        #if defined(_LOGGING_)

        if (CT.logRes>=2)
        {
            sprintf(CT.logBuffer,
                "Minimum (%ld,%ld)-cut has capacity %g",source,target,val);
            LogEntry(LOG_RES2,CT.logBuffer);
        }

        #endif

        SetUpperBound(TimerStrongConn,val);

        lambda = val;

        for (TNode u=0;u<n;u++) chi[u] = chiG[u];
    }

    if (CT.SolverRunning()) SetLowerBound(TimerStrongConn,lambda);

    CloseFold();

    sprintf(CT.logBuffer,"...Strong edge connectivity is %g",lambda);
    LogEntry(LOG_RES,CT.logBuffer);

    CloseFold(ModSchnorr,NO_INDENT);

    return lambda;
}


TCap abstractMixedGraph::MCC_StrongEdgeConnectivity(TMethMCC method,TNode source)
    throw(ERRange)
{
    OpenFold(ModStrongConn,NO_INDENT);

    sprintf(CT.logBuffer,"Computing strong edge %ld-connectivity...",source);
    LogEntry(LOG_METH,CT.logBuffer);

    diGraph G(*this);
    G.Representation() -> SetCDemand(0);

    TNode* chi = RawNodeColours();
    TNode* chiG = G.RawNodeColours();

    TCap lambda = InfFloat;

    if (method==MCC_DEFAULT) method = TMethMCC(CT.methMCC);

    if (CT.methMCC!=MCC_MAXFLOW)
    {
        lambda = G.MCC_HaoOrlin(method,source);

        for (TNode u=0;u<n;u++) chi[u] = chiG[u];
    }
    else
    {
        #if defined(_PROGRESS_)

        InitProgressCounter(n-1);
        SetProgressNext(1);

        #endif

        OpenFold();

        for (TNode target=0;target<n && CT.SolverRunning();++target)
        {
            if (target==source) continue;

            CT.SuppressLogging();
            TCap val = G.MCC_StrongEdgeConnectivity(source,target);
            CT.RestoreLogging();

            #if defined(_PROGRESS_)

            ProgressStep(1);

            #endif

            if (val>=lambda) continue;

            #if defined(_LOGGING_)

            if (CT.logRes>=2)
            {
                sprintf(CT.logBuffer,
                    "Minimum (%ld,%ld)-cut has capacity %g",source,target,val);
                LogEntry(LOG_RES2,CT.logBuffer);
            }

            #endif

            SetUpperBound(TimerStrongConn,val);

            lambda = val;

            for (TNode u=0;u<n;u++) chi[u] = chiG[u];

            Trace();
        }

        if (CT.SolverRunning()) SetLowerBound(TimerStrongConn,lambda);

        CloseFold();

        sprintf(CT.logBuffer,"...Strong edge %ld-connectivity is %g",source,lambda);
        LogEntry(LOG_RES,CT.logBuffer);
    }

    CloseFold(ModStrongConn,NO_INDENT);

    return lambda;
}


TCap abstractMixedGraph::MCC_StrongEdgeConnectivity(TNode source,TNode target)
    throw(ERRange)
{
    OpenFold(ModStrongConn,NO_INDENT);

    sprintf(CT.logBuffer,"Computing strong edge (%ld,%ld)-connectivity...",source,target);
    LogEntry(LOG_METH,CT.logBuffer);

    OpenFold();

    abstractDiGraph* G = NULL;

    if (IsDirected() && CDemand() && Demand(0)==0)
    {
        G = static_cast<abstractDiGraph*>(this);
    }
    else
    {
        G = new diGraph(*this);
        G -> Representation() -> SetCDemand(0);
    }

    CT.SuppressLogging();
    G -> InitSubgraph();
    G -> MaxFlow(source,target);
    TCap lambda = G->FlowValue(source,target);
    CT.RestoreLogging();

    TNode* chi = RawNodeColours();
    TFloat* distG = G->GetDistanceLabels();

    for (TNode u=0;u<n;u++)
    {
        chi[u] = (distG[u]<InfFloat) ? CONN_LEFT_HAND : CONN_RIGHT_HAND;
    }

    if (!IsDirected()) delete G;

    CloseFold();

    sprintf(CT.logBuffer,"...Strong edge (%ld,%ld)-connectivity is %g",source,target,lambda);
    LogEntry(LOG_RES,CT.logBuffer);

    CloseFold(ModStrongConn,NO_INDENT);

    return lambda;
}


TCap abstractMixedGraph::StrongEdgeConnectivity(abstractMixedGraph::TMethMCC method,
    TNode source,TNode target) throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (source>=n && source!=NoNode) NoSuchNode("StrongEdgeConnectivity",source);

    if (target>=n && target!=NoNode) NoSuchNode("StrongEdgeConnectivity",target);

    #endif

    if (target==NoNode)
    {
        if (source==NoNode)
        {
            if (m==0 || (COrientation() && Orientation(0)==0))
            {
                // Graph is undirected, at least implicitly
                return EdgeConnectivity();
            }

            return MCC_StrongEdgeConnectivity();
        }

        return MCC_StrongEdgeConnectivity(method,source);
    }

    #if defined(_FAILSAVE_)

    if (source==NoNode)
    {
        Error(ERR_RANGE,"StrongEdgeConnectivity","Missing left-hand node");
    }

    #endif

    TCap lambda = MCC_StrongEdgeConnectivity(source,target);

    SetBounds(TimerStrongConn,lambda,lambda);

    return lambda;
}


TCap abstractDiGraph::MCC_HaoOrlin(TMethMCC method,TNode r) throw(ERRange)
{
    OpenFold(ModHaoOrlin,SHOW_TITLE);

    #if defined(_PROGRESS_)

    InitProgressCounter(2*n*n);

    #endif

    // A queue for the active nodes

    goblinQueue<TNode,TFloat> *Q = NULL;

    if (method!=MCC_PREFLOW_FIFO)
    {
        Q = NewNodeHeap();
    }
    else
    {
        Q = new staticQueue<TNode,TFloat>(n,CT);
    }

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

    // The sink node, changed in every main loop iteration

    TNode t = 0;
    if (r==0) t = 1;

    TFloat* dist = InitDistanceLabels(1);
    dist[t] = 0;

    // Store the minimum cut and its capacity

    TNode *chi = new TNode[n];
    TCap minCap = InfCap;

    // Keep the dormient nodes in a tracable data structure

    TNode *dorm = InitNodeColours(NoNode);
    dorm[r] = 0;
    TNode dormMax = 0;

    // Push all flow away from the root node

    THandle H = Investigate();
    investigator &I = Investigator(H);
    bool unbounded = false;
    while (I.Active(r))
    {
        TArc a = I.Read(r);
        TNode v = EndNode(a);
        if (Divergence(v)==0 && v!=r && v!=t && ResCap(a)>0)
            Q -> Insert(v,-dist[v]);
        if (UCap(a)!=InfCap) Push(a,ResCap(a));
        else unbounded = true;
    }

    Trace(n);

    // Initialize the incidence lists

    I.Reset(t);
    while (I.Active(t))
    {
        TArc a = I.Read(t);
        TNode v = EndNode(a);
        if (ResCap(a^1)>0 && v!=r) Admissible[v]->Insert(a^1);
    }

    for (TNode itc=1;itc<n && !unbounded;itc++)
    {
        sprintf(CT.logBuffer,"New sink node is %ld",t);
        LogEntry(LOG_METH,CT.logBuffer);

        while (!(Q->Empty()) && !unbounded)
        {
            TNode u = Q->Delete();

            if (dorm[u]!=NoNode || u==t) continue;

            while (!Admissible[u]->Empty() && Divergence(u)>0)
            {
                TArc a = Admissible[u]->Delete();
                TNode v = EndNode(a);

                TFloat Lambda = ResCap(a);

                if (ResCap(a)>Divergence(u)) Lambda = Divergence(u);

                if (dist[u]==dist[v]+1 && dorm[v]==NoNode)
                {
                    if (Divergence(v)==0 && v!=t) Q->Insert(v,-dist[v]);

                    #if defined(_LOGGING_)

                    if (CT.logMeth>1)
                    {
                        sprintf(CT.logBuffer,
                            "Pushed %g flow units from node %ld to node %ld",Lambda,u,v);
                        LogEntry(LOG_METH2,CT.logBuffer);
                    }

                    #endif

                    Push(a,Lambda);

                    if (ResCap(a)>0)
                        try {Admissible[u]->Insert(a);} catch(ERCheck) {};

                    Trace();
                }
            }

            if (Divergence(u)>0)
            {
                // Check if there are other awake nodes with the same
                // distance label

                bool isUnique = true;
                for (TNode v=0;v<n && isUnique;v++)
                    if (v!=u && dist[v]==dist[u] && dorm[v]==NoNode)
                        isUnique = false;

                if (isUnique)
                {
                    dormMax++;

                    for (TNode v=0;v<n;v++)
                        if (dist[v]>=dist[u] && dorm[v]==NoNode)
                        {
                            dorm[v] = dormMax;

                            #if defined(_LOGGING_)

                            if (CT.logMeth>1)
                            {
                                sprintf(CT.logBuffer,"dorm[%ld] = %ld",v,dormMax);
                                LogEntry(LOG_METH2,CT.logBuffer);
                            }

                            #endif
                        }
                }
                else
                {
                    I.Reset(u);
                    TFloat dMin = NoNode;

                    while (I.Active(u))
                    {
                        TArc a = I.Read(u);
                        TNode v = EndNode(a);

                        if (dorm[v]!=NoNode || v==t) continue;

                        if (ResCap(a)>0 && dist[v]<=dMin)
                        {
                            dMin = dist[v];
                            try {S1.Insert(a);} catch(ERCheck) {};
                        }

                        if (ResCap(a^1)>0 && dMin+2>=dist[v])
                        {
                            try {S2.Insert(a^1);} catch(ERCheck) {};
                        }
                    }

                    if (dMin<NoNode)
                    {
                        while (!S1.Empty())
                        {
                            TArc a = S1.Delete();
                            TNode v = EndNode(a);

                            if (dMin==dist[v])
                            {
                                Admissible[u]->Insert(a,INSERT_NO_THROW);
                            }
                        }

                        while (!S2.Empty())
                        {
                            TArc a = S2.Delete();
                            TNode v = StartNode(a);

                            if (dMin+2==dist[v])
                            {
                                Admissible[v]->Insert(a,INSERT_NO_THROW);
                            }
                        }

                        #if defined(_LOGGING_)

                        if (CT.logMeth>1)
                        {
                            sprintf(CT.logBuffer,
                                "Relabelled node %ld: %g -> %g",u,dist[u],dMin+1);
                            LogEntry(LOG_METH2,CT.logBuffer);
                        }

                        #endif

                        Trace(TNode(dMin+1-dist[u]));

                        dist[u] = dMin+1;
                        Q->Insert(u,-dist[u]);
                    }
                    else
                    {
                        dormMax++;
                        dorm[u] = dormMax;

                        #if defined(_LOGGING_)

                        if (CT.logMeth>1)
                        {
                            sprintf(CT.logBuffer,"dorm[%ld] = %ld",u,dormMax);
                            LogEntry(LOG_METH2,CT.logBuffer);
                        }

                        #endif
                    }
                }
            }
        }

        // Check if the new cut improves the bound

        TCap thisCap = 0;
        for (TArc a=0;a<m;a++)
        {
            if (dorm[StartNode(2*a)]!=NoNode && dorm[EndNode(2*a)]==NoNode)
                thisCap += UCap(2*a);
        }

        #if defined(_LOGGING_)

        if (CT.logRes>1)
        {
            sprintf(CT.logBuffer,"Found cut with capacity %g",thisCap);
            LogEntry(LOG_RES2,CT.logBuffer);
        }

        #endif

        if (thisCap<minCap)
        {
            minCap = thisCap;

            for (TNode i=0;i<n;i++)
            {
                chi[i] = (dorm[i]!=NoNode) ? CONN_LEFT_HAND : CONN_RIGHT_HAND;
            }

            SetUpperBound(TimerStrongConn,minCap);
        }

        // Push all flow towards the right hand side

        dorm[t] = 0;
        I.Reset(t);

        while (I.Active(t))
        {
            TArc a = I.Read(t);
            TNode v = EndNode(a);
            if (UCap(a)!=InfCap)
            {
                if (ResCap(a)>0 && dorm[v]!=0)
                {
                    if (Divergence(v)==0 && dorm[v]==NoNode)
                        Q -> Insert(v,-dist[v]);
                    Push(a,ResCap(a));
                }
            }
            else unbounded = true;
        }

        Trace(n);

        // Select a new sink node

        t = NoNode;
        for (TNode i=1;i<n;i++)
            if (dorm[i]==NoNode && (t==NoNode || dist[i]<dist[t])) t = i;

        if (t==NoNode && itc<n-1)
        {
            #if defined(_LOGGING_)

            if (CT.logMeth>1) LogEntry(LOG_METH2,"Awaked nodes:");

            #endif

            for (TNode i=1;i<n;i++)
            {
                if (dorm[i]==dormMax)
                {
                    if (t==NoNode || dist[i]<dist[t]) t = i;
                    dorm[i] = NoNode;
                    if (Divergence(i)>0) Q -> Insert(i,-dist[i]);

                    I.Reset(i);
                    while (I.Active(i))
                    {
                        TArc a = I.Read(i);
                        TNode v = EndNode(a);
                        if (dorm[v]!=NoNode) continue;

                        if (ResCap(a)>0 && dist[v]+1==dist[i])
                            Admissible[i]->Insert(a,INSERT_NO_THROW);

                        if (ResCap(a^1)>0 && dist[v]==dist[i]+1)
                            Admissible[v]->Insert(a^1,INSERT_NO_THROW);
                    }

                    #if defined(_LOGGING_)

                    if (CT.logMeth>1)
                    {
                        sprintf(CT.logBuffer," %ld",i);
                        LogEntry(MSG_APPEND,CT.logBuffer);
                    }

                    #endif
                }
            }

            dormMax--;
        }
    }

    Close(H);

    delete Q;

    for (TNode i=1;i<n;i++) delete Admissible[i];
    delete Admissible[0];
    delete[] Admissible;

    ReleasePredecessors();

    // Move the minimum cut into an exported data structure
    for (TNode i=0;i<n;i++) dorm[i] = chi[i];
    delete[] chi;

    if (unbounded) Error(ERR_RANGE,"MCC_HaoOrlin",
        "Arcs emanating from source must have finite capacities");

    SetLowerBound(TimerStrongConn,minCap);

    CloseFold(ModHaoOrlin);

    sprintf(CT.logBuffer,"...Strong edge %ld-connectivity is %g",r,minCap);
    LogEntry(LOG_RES,CT.logBuffer);

    return minCap;
}


TCap abstractMixedGraph::MCC_LegalOrdering(TNode r,TNode &x,TNode &y)
    throw(ERRange,ERRejected)
{
    #if defined(_FAILSAVE_)

    if (r>=n) NoSuchNode("MinCutLegalOrdering",r);

    #endif

    OpenFold(ModNodeIdentification,NO_INDENT);

    #if defined(_PROGRESS_)

    InitProgressCounter(n);

    #endif

    LogEntry(LOG_METH,"Computing legal ordering...");

    TFloat* dist = InitDistanceLabels(InfFloat);
    TNode* index = InitNodeColours(NoNode);

    goblinQueue<TNode,TFloat> *Q = NULL;

    if (nHeap!=NULL)
    {
        Q = nHeap;
        Q -> Init();
    }
    else Q = NewNodeHeap();

    Q -> Insert(r,0);
    index[r] = 0;

    OpenFold();

    #if defined(_LOGGING_)

    THandle LH = LogStart(LOG_METH2,"Expanded nodes: ");

    #endif

    TNode _index = 0;
    TFloat ret = InfCap;
    THandle H = Investigate();
    investigator &I = Investigator(H);

    while (!(Q->Empty()))
    {
        TNode u = Q->Delete();

        x = y;
        y = u;
        ret = dist[u];

        index[u] = _index;
        dist[u] = -InfFloat;

        #if defined(_LOGGING_)

        if (CT.logMeth>1 && I.Active(u))
        {
            sprintf(CT.logBuffer,"%ld ",u);
            LogAppend(LH,CT.logBuffer);
        }

        #endif

        while (I.Active(u))
        {
            TArc a = I.Read(u);
            TNode v = EndNode(a);

            if (index[v]==NoNode)
            {
                TFloat l = UCap(a);

                if (dist[v]==InfFloat)
                {
                    dist[v] = l;
                    Q->Insert(v,-dist[v]);
                }
                else
                {
                    dist[v] += l;
                    Q->ChangeKey(v,-dist[v]);
                }
            }
        }

        Trace(1);

        _index++;
    }

    #if defined(_LOGGING_)

    if (CT.logMeth>1) LogEnd(LH);

    #endif

    Close(H);

    if (nHeap==NULL) delete Q;

    CloseFold();

    if (CT.logRes)
    {
        sprintf(CT.logBuffer,"...delta(%ld)-cut has capacity: %g",x,ret);
        LogEntry(LOG_RES2,CT.logBuffer);
    }

    CloseFold(ModNodeIdentification,NO_INDENT);

    return TCap(ret);
}


TCap abstractMixedGraph::MCC_NodeIdentification()
    throw(ERRejected)
{
    OpenFold(ModNodeIdentification,SHOW_TITLE);

    #if defined(_PROGRESS_)

    InitProgressCounter(n*(3*n-1)/2-1);

    #endif

    graph G(*this);
    sparseRepresentation* GR = static_cast<sparseRepresentation*>(G.Representation());

    #if defined(_TRACING_)

    for (TNode u=0;u<n;u++)
    {
        if (Dim()>0)
        {
            GR -> SetC(u,0,C(u,0));
            GR -> SetC(u,1,C(u,1));
        }
    }

    #endif

    TCap lambda = InfCap;
    TNode* chi = InitNodeColours();

    InitPartition();
    for (TNode i=0;i<n;i++) Bud(i);

    for (TNode i=1;CT.SolverRunning() && i<n;i++)
    {
        TNode x = NoNode;
        TNode y = NoNode;

        TCap thisCap = G.MCC_LegalOrdering(Find(0),x,y);

        if (thisCap<lambda)
        {
            // Update minimum cut

            lambda = thisCap;

            for (TNode j=0;j<n;j++)
            {
                chi[j] = (Find(j)==Find(y)) ? CONN_RIGHT_HAND : CONN_LEFT_HAND;
            }

            Trace();

            SetUpperBound(TimerMinCut,lambda);
        }

        Merge(x,y);
        GR -> IdentifyNodes(x,y);

        if ((i%5)==0)
        {
            TArc a = 0;
            while (a<G.M())
            {
                if (G.StartNode(2*a)==G.EndNode(2*a)) GR -> DeleteArc(2*a);
                else a++;
            }
        }

        #if defined(_TRACING_)

        GR -> Layout_ArcAlignment(0,false);

        #endif

        #if defined(_PROGRESS_)

        ProgressStep(n);

        #endif
    }

    SetLowerBound(TimerMinCut,lambda);

    CloseFold(ModNodeIdentification);

    if (CT.logRes)
    {
        sprintf(CT.logBuffer,"...Edge connectivity is %g",lambda);
        LogEntry(LOG_RES,CT.logBuffer);
    }

    return lambda;
}
