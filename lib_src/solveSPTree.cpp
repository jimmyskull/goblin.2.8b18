
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, July 2003
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file   solveSPTree.cpp
/// \brief  A collection of shortest path methods in the directed setting

#include "abstractGraph.h"
#include "abstractDigraph.h"
#include "staticQueue.h"


bool abstractMixedGraph::ShortestPath(TMethSPX method,TOptSPX characteristic,
    TNode s,TNode t) throw(ERRange,ERRejected)
{
    if (s==NoNode)
    {
        s = sourceNode;
        t = targetNode;
    }

    #if defined(_FAILSAVE_)

    if (s>=n) NoSuchNode("ShortestPath",s);

    if (t>=n && t!=NoNode) NoSuchNode("ShortestPath",t);

    if ((characteristic & SPX_RESIDUAL) && !IsDirected())
         Error(ERR_REJECTED,"ShortestPath","Residual graph search applies to directed graphs only");


    #endif

    if (method==SPX_DEFAULT) method = TMethSPX(CT.methSPX);

    OpenFold(ModSPTree,NO_INDENT);

    if (characteristic & SPX_RESIDUAL)
    {
        LogEntry(LOG_METH2,"Computing shortest augmenting path...");
    }
    else
    {
        LogEntry(LOG_METH,"Computing shortest path tree...");
    }

    bool ret = false;

    switch (method)
    {
        case SPX_FIFO:
        {
            ret = SPX_FIFOLabelCorrecting(characteristic,s,t);
            break;
        }
        case SPX_DIJKSTRA:
        {
            if (t==NoNode)
            {
                ret =  (SPX_Dijkstra(characteristic,
                                    singletonIndex<TNode>(s,n,CT),
                                    voidIndex<TNode>(n,CT)) != NoNode);
            }
            else
            {
                ret =  (SPX_Dijkstra(characteristic,
                                    singletonIndex<TNode>(s,n,CT),
                                    singletonIndex<TNode>(t,n,CT)) != NoNode);
            }

            break;
        }
        case SPX_BELLMAN:
        {
            ret = SPX_BellmanFord(characteristic,s,t);
            break;
        }
        case SPX_BFS:
        {
            #if defined(_FAILSAVE_)

            if (!CLength() || MaxLength()<0)
            {
                CloseFold(ModSPTree,NO_INDENT);
                Error(ERR_REJECTED,"ShortestPath","Non-trivial length labels");
            }

            #endif

            ret = CLength()*BFS(characteristic,s,t);
            break;
        }
        case SPX_DAG:
        {
            if (characteristic!=SPX_PLAIN)
            {
                CloseFold(ModSPTree,NO_INDENT);
                Error(ERR_REJECTED,"ShortestPath","Option is not supported for DAG search");
            }
            else if (IsDirected())
            {
                abstractDiGraph* thisDiGraph =
                    static_cast<abstractDiGraph*>(this);
                TNode x = thisDiGraph->DAGSearch(abstractDiGraph::DAG_SPTREE,s,t);

                if (x!=NoNode)
                {
                    #if defined(_FAILSAVE_)

                    CloseFold(ModSPTree,NO_INDENT);
                    Error(ERR_REJECTED,"ShortestPath","Graph is recurrent");

                    #endif
                }

                ret = (t==NoNode || Dist(t)<InfFloat);

                break;
            }
            else
            {
                CloseFold(ModSPTree,NO_INDENT);
                Error(ERR_REJECTED,"ShortestPath","Not a DAG");
            }
        }
        case SPX_TJOIN:
        {
            if (t==NoNode)
            {
                CloseFold(ModSPTree,NO_INDENT);
                NoSuchNode("ShortestPath",t);
            }
            else if (IsUndirected())
            {
                abstractGraph* thisGraph =
                    static_cast<abstractGraph*>(this);

                ret = thisGraph -> SPX_TJoin(s,t);

                break;
            }
            else
            {
                CloseFold(ModSPTree,NO_INDENT);
                Error(ERR_REJECTED,"ShortestPath","Method applies to undirected graphs only");
            }
        }
         default:
        {
            CloseFold(ModSPTree,NO_INDENT);
            UnknownOption("ShortestPath",method);
        }
    }

    CloseFold(ModSPTree,NO_INDENT);

    return ret;
}


bool abstractMixedGraph::Eligible(TOptSPX characteristic,TArc a) throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (a>=2*m) NoSuchArc("Eligible",a);

    #endif

    if (characteristic & SPX_SUBGRAPH)
    {
        return (Sub(a)>0 && !(Blocking(a)));
    }
    else if ((characteristic & SPX_RESIDUAL) && Blocking(a|1))
    {
        return ResCap(a)>delta;
    }

    return (UCap(a)>0 && !(Blocking(a)));
}


bool abstractMixedGraph::BFS(TOptSPX characteristic,TNode s,TNode t) throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (s>=n) NoSuchNode("BFS",s);

    if (t>=n && t!=NoNode) NoSuchNode("BFS",t);

    #endif

    OpenFold(ModSPTree,NO_INDENT);

    #if defined(_PROGRESS_)

    InitProgressCounter(n);

    #endif

    TFloat* dist = InitDistanceLabels();
    TArc* pred = InitPredecessors();

    dist[s] = 0;
    staticQueue<TNode,TFloat> Q(n,CT);
    Q.Insert(s);

    if (characteristic & SPX_SUBGRAPH)
    {
        LogEntry(LOG_METH,"Subgraph is searched...");
    }
    else if (characteristic & SPX_RESIDUAL)
    {
        LogEntry(LOG_METH,"Residual graph is searched...");
    }
    else
    {
        LogEntry(LOG_METH,"Graph is searched...");
    }

    OpenFold();

    #if defined(_LOGGING_)

    THandle LH = LogStart(LOG_METH2,"Expanded nodes: ");

    #endif

    THandle H = Investigate();
    investigator &I = Investigator(H);

    bool searching = true;
    while (!(Q.Empty()) && searching)
    {
        Trace(Q); 
        TNode u = Q.Delete();

        #if defined(_LOGGING_)

        if (CT.logMeth>1)
        {
            sprintf(CT.logBuffer,"%ld ",u);
            LogAppend(LH,CT.logBuffer);
        }

        #endif

        while (I.Active(u))
        {
            TArc a = I.Read(u);
            TNode v = EndNode(a);

            if (dist[v]==InfFloat && Eligible(characteristic,a))
            {
                dist[v] = dist[u]+1;
                Trace(Q); 
                Q.Insert(v);
                pred[v] = a;

                if (v==t) searching = false;
            }
        }

        Trace(1);
    }

    #if defined(_LOGGING_)

    if (CT.logMeth>1) LogEnd(LH);

    #endif

    Close(H);

    CloseFold();

//    if (t!=NoNode) SetBounds(TimerSPTree,dist[t],dist[t]);

    CloseFold(ModSPTree,NO_INDENT);

    if (t==NoNode) return 0;
    else return (dist[t]!=InfFloat);
}


TNode abstractMixedGraph::SPX_Dijkstra(TOptSPX characteristic,
    const indexSet<TNode>& S,const indexSet<TNode>& T) throw(ERRange,ERRejected)
{
    OpenFold(ModDikjstra,SHOW_TITLE);

    #if defined(_PROGRESS_)

    InitProgressCounter(n);

    #endif

    TFloat* dist = InitDistanceLabels();
    TFloat* potential = GetPotentials();
    TArc* pred = InitPredecessors();

    goblinQueue<TNode,TFloat> *Q = NULL;

    if (nHeap)
    {
        Q = nHeap;
        Q -> Init();
    }
    else Q = NewNodeHeap();

    for (TNode s=S.First();s<n;s=S.Successor(s))
    {
        Q -> Insert(s,0);
        dist[s] = 0;
    }

    TNode t = NoNode;

    #if defined(_LOGGING_)

    THandle LH = LogStart(LOG_METH2,"Expanded nodes:");

    #endif

    THandle H = Investigate();
    investigator &I = Investigator(H);
    while (!(Q->Empty()))
    {
        TNode u = Q->Delete();

        if (T.IsMember(u))
        {
            t = u;
            break;
        }

        #if defined(_LOGGING_)

        if (CT.logMeth>1 && I.Active(u))
        {
            sprintf(CT.logBuffer," %ld[%g]",u,dist[u]);
            LogAppend(LH,CT.logBuffer);
        }

        #endif

        while (I.Active(u))
        {
            TArc a = I.Read(u);
            TFloat l = Length(a);

            if (characteristic!=SPX_PLAIN) l = RedLength(potential,a);

            TNode v = EndNode(a);
            TFloat dd = dist[u]+l;

            if (dist[v]>dd && Eligible(characteristic,a))
            {
                #if defined(_FAILSAVE_)

                if (l<-CT.epsilon)
                {
                    CloseFold(ModDikjstra);
                    Error(ERR_REJECTED,"SPX_Dijkstra","Negative arc length");
                }

                #endif

                if (dist[v]==InfFloat) Q->Insert(v,dd);
                else Q->ChangeKey(v,dd);

                dist[v] = dd;
                pred[v] = a;
            }
        }

        Trace(1);
    }

    #if defined(_LOGGING_)

    if (CT.logMeth>1) LogEnd(LH);

    #endif

    Close(H);

    if (!nHeap) delete Q;

//    if (t!=NoNode) SetBounds(TimerSPTree,dist[t],dist[t]);

    CloseFold(ModDikjstra);

    return t;
}


TNode abstractMixedGraph::VoronoiRegions(const indexSet<TNode>& Terminals)
    throw(ERRejected)
{
    LogEntry(LOG_METH,"Computing Voronoi regions...");

    #if defined(_PROGRESS_)

    InitProgressCounter(n);

    #endif

    SPX_Dijkstra(SPX_PLAIN,Terminals,singletonIndex<TNode>(NoNode,n,CT));

    InitPartition();

    TNode nTerminals = 0;

    for (TNode i=0;i<n;i++)
    {
        Bud(i);

        if (Terminals.IsMember(i)) nTerminals ++;
    }

    for (TNode i=0;i<n;i++)
    {
        if (Pred(i)==NoArc) continue;

        TNode u = Find(StartNode(Pred(i)));
        TNode v = Find(i);

        if (u!=v) Merge(u,v);
    }

    return nTerminals;
}


bool abstractMixedGraph::SPX_BellmanFord(TOptSPX characteristic,TNode s,TNode t)
    throw(ERRange,ERCheck)
{
    #if defined(_FAILSAVE_)

    if (s>=n) NoSuchNode("SPX_BellmanFord",s);

    if (t>=n && t!=NoNode) NoSuchNode("SPX_BellmanFord",t);

    #endif

    OpenFold(ModBellmanFord,SHOW_TITLE);

    #if defined(_PROGRESS_)

    InitProgressCounter(m*(2.0*n-1));

    #endif

    TFloat* dist = InitDistanceLabels();
    TArc* pred = InitPredecessors();
    TFloat* potential = GetPotentials();

    dist[s] = 0;
    TNode i = 1;
    bool Updates = true;

    while (Updates && i<2*n)
    {
        #if defined(_LOGGING_)

        if (CT.logMeth>1)
        {
            sprintf(CT.logBuffer,"Iteration %ld:",i);
            LogEntry(LOG_METH2,CT.logBuffer);
        }

        #endif

        OpenFold();
        Updates = 0;

        for (TArc a=0;a<2*m;a++)
        {
            TNode u = StartNode(a);
            TNode v = EndNode(a);
            TFloat l = Length(a);

            if (characteristic!=SPX_PLAIN) l = RedLength(potential,a);

            TFloat thisLabel = dist[u] + l;

            if (dist[v]>thisLabel && dist[u]!=InfFloat && pred[u]!=(a^1) && Eligible(characteristic,a))
            {
                #if defined(_LOGGING_)

                if (CT.logMeth>1)
                {
                    sprintf(CT.logBuffer,"Label d[%ld] updated to %g",
                        v,thisLabel);
                    LogEntry(LOG_METH2,CT.logBuffer);
                }

                #endif

                dist[v] = thisLabel;
                pred[v] = a;
                Updates = 1;
            }
        }

        i++;
        CloseFold();
        Trace(m);  
    }

    if (i==2*n)
    {
        CloseFold(ModBellmanFord);
        Error(ERR_CHECK,"SPX_BellmanFord","Negative length cycles");
    }

    if (t==NoNode)
    {
        CloseFold(ModBellmanFord);
        return false;
    }

    if (dist[t]!=InfFloat) SetBounds(TimerSPTree,dist[t],dist[t]);
    else SetBounds(TimerSPTree,-InfFloat,-InfFloat);

    CloseFold(ModBellmanFord);

    return (dist[t]!=InfFloat);
}


bool abstractMixedGraph::SPX_FIFOLabelCorrecting(TOptSPX characteristic,TNode s,TNode t)
    throw(ERRange,ERCheck)
{
    #if defined(_FAILSAVE_)

    if (s>=n) NoSuchNode("SPX_FIFOLabelCorrecting",s);

    if (t>=n && t!=NoNode) NoSuchNode("SPX_FIFOLabelCorrecting",t);

    #endif

    OpenFold(ModFIFOLabelCorrect,NO_INDENT);

    TNode v = NegativeCycle(characteristic,s);

    if (v!=NoNode)
    {
        CloseFold(ModFIFOLabelCorrect,NO_INDENT);
        Error(ERR_CHECK,"SPX_FIFOLabelCorrecting","Negative length cycles");
    }

    if (t==NoNode)
    {
        CloseFold(ModFIFOLabelCorrect,NO_INDENT);
        return false;
    }

    OpenFold(ModFIFOLabelCorrect);

    TFloat dt = Dist(t);

    if (dt!=InfFloat) SetBounds(TimerSPTree,dt,dt);
    else SetBounds(TimerSPTree,-InfFloat,-InfFloat);

    CloseFold(ModFIFOLabelCorrect);

    CloseFold(ModFIFOLabelCorrect,NO_INDENT);

    return (dt!=InfFloat);
}


TNode abstractMixedGraph::NegativeCycle(TOptSPX characteristic,TNode source,TFloat epsilon) throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (source>=n && source!=NoNode) NoSuchNode("NegativeCycle",source);

    #endif

    OpenFold(ModFIFOLabelCorrect,SHOW_TITLE);

    #if defined(_PROGRESS_)

    InitProgressCounter(m*2.0*n);

    #endif

    TFloat* dist = InitDistanceLabels();
    TArc* pred = InitPredecessors();
    TFloat* potential = GetPotentials();

    bool* active = new bool[n];

    if (source!=NoNode)
    {
        dist[source] = 0;
        for (TNode v=0;v<n;v++) active[v] = false;
        active[source] = true;
    }
    else
    {
        for (TNode v=0;v<n;v++)
        {
            active[v] = true;
            dist[v] = 0;
        }
    }

    TNode itCount = 1;
    TNode root = NoNode;

    bool Updates = true;
    THandle H = Investigate();
    investigator &I = Investigator(H);

    while (Updates && itCount<=2*n)
    {
        #if defined(_LOGGING_)

        if (CT.logMeth>1)
        {
            sprintf(CT.logBuffer,"Iteration %ld:",itCount);
            LogEntry(LOG_METH2,CT.logBuffer);
        }

        #endif

        OpenFold();
        Updates = 0;

        for (TNode v=0;v<n;v++)
        {
            if (active[v])
            {
                active[v] = false;

                while (I.Active(v))
                {
                    TArc a = I.Read(v);
                    TNode w = EndNode(a);
                    TFloat l = Length(a);

                    if (characteristic!=SPX_PLAIN) l = RedLength(potential,a);

                    // Active nodes have dist <InfFloat
                    TFloat thisLabel = dist[v] + l + epsilon;

                    if (   dist[w]>thisLabel
                        && pred[v]!=(a^1)
                        && Eligible(characteristic,a) )
                    {
                        #if defined(_LOGGING_)

                        if (CT.logMeth>1)
                        {
                            sprintf(CT.logBuffer,"Label d[%ld] updated to %g",
                                w,thisLabel);
                            LogEntry(LOG_METH2,CT.logBuffer);
                        }

                        #endif

                        dist[w] = thisLabel;
                        pred[w] = a;

                        if (itCount<2*n) Updates = 1;
                        else root = v;

                        active[w] = true;
                    }
                }

                I.Reset(v);
            }
        }

        itCount++;

        CloseFold();
        Trace(m);
    }

    Close(H);
    delete[] active;

    if (root!=NoNode)
    {
        for (TNode i=0;i<n;i++) root = StartNode(pred[root]);

        #if defined(_LOGGING_)

        if (CT.logMeth>1)
        {
            sprintf(CT.logBuffer,"...Node on a negative cycle: %ld",root);
            LogEntry(LOG_RES,CT.logBuffer);
        }

        #endif
    }

    CloseFold(ModFIFOLabelCorrect);

    return root;
}


TNode abstractDiGraph::MinimumMeanCycle(TOptSPX characteristic,TFloat* meanValue) throw()
{
    OpenFold(ModKarpMeanCycle,SHOW_TITLE);

    InitPotentials();
    TFloat *dist = new TFloat[(n+1)*n];
    TArc* pred = new TArc[(n+1)*n];

    for (TNode v=0;v<n;v++) dist[0*n+v] = 0;

    for (TNode k=1;k<=n;k++)
    {
        for (TNode v=0;v<n;v++) dist[k*n+v] = InfFloat;

        for (TArc a=0;a<2*m;a++)
        {
            TNode u = StartNode(a);
            TNode v = EndNode(a);
            TFloat l = RedLength(NULL,a);

            if (Eligible(characteristic,a) && dist[(k-1)*n+u]!=InfFloat && (dist[k*n+v]>dist[(k-1)*n+u]+l))
            {
                dist[k*n+v] = dist[(k-1)*n+u]+l;
                pred[k*n+v] = a;
            }
        }
    }

    TNode minNode = NoNode;
    TFloat muLocal = InfFloat;

    for (TNode v=0;v<n;v++)
    {
        TFloat thisMaxMean = -InfFloat;

        if ((dist[n*n+v])<InfFloat)
        {
            for (TNode k=0;k<n;k++)
            {
                if ((dist[k*n+v])<InfFloat)
                {
                    TFloat thisMean = (dist[n*n+v]-dist[k*n+v])/(n-k);
                    if (thisMean>thisMaxMean) thisMaxMean = thisMean;
                }
            }
        }

        if (thisMaxMean<muLocal && thisMaxMean>-InfFloat)
        {
            muLocal = thisMaxMean;
            minNode = v;
        }
    }

    if (muLocal<InfFloat)
    {
        #if defined(_LOGGING_)

        if (CT.logMeth>1)
        {
            sprintf(CT.logBuffer,"...Minimum ratio: %g",muLocal);
            LogEntry(LOG_RES,CT.logBuffer);
            LogEntry(LOG_METH2,"Extracting minimum mean cycle...");
        }

        #endif

        TArc* predExport = InitPredecessors();

        TNode k = n;

        while (predExport[minNode]==NoArc)
        {
            predExport[minNode] = pred[k*n+minNode];
            minNode = StartNode(predExport[minNode]);
            k--;
        }
    }
    else
    {
        minNode = NoNode;
        LogEntry(LOG_RES,"...Graph is acyclic");
    }

    delete[] dist;
    delete[] pred;

    CloseFold(ModKarpMeanCycle);

    if (meanValue) *meanValue = muLocal;

    return minNode;
}


TNode abstractDiGraph::TopSort() throw()
{
    LogEntry(LOG_METH,"Computing topological ordering...");

    OpenFold(ModSPTree,NO_INDENT);

    TNode ret = DAGSearch(DAG_TOPSORT);

    CloseFold(ModSPTree,NO_INDENT);

    return ret;
}


TNode abstractDiGraph::CriticalPath() throw()
{
    LogEntry(LOG_METH,"Computing critical path...");

    OpenFold(ModSPTree,NO_INDENT);

    TNode ret = DAGSearch(DAG_CRITICAL);

    CloseFold(ModSPTree,NO_INDENT);

    return ret;
}


TNode abstractDiGraph::DAGSearch(TOptDAGSearch opt,TNode s,TNode t)
    throw(ERRange)
{
    if (opt==DAG_TOPSORT) OpenFold(ModDAGSearch);
    else OpenFold(ModDAGSearch,SHOW_TITLE);

    staticQueue<TNode,TFloat> Q(n,CT);
    TArc* idg = new TArc[n];

    for (TNode v=0;v<n;v++) idg[v] = 0;

    for (TArc a=0;a<2*m;a++)
    {
        if (Eligible(SPX_PLAIN,a)) 
        {
            TNode w = EndNode(a);
            idg[w]++;
        }
    }

    #if defined(_LOGGING_)

    THandle LH = NoHandle;

    #endif

    TFloat* dist = NULL;
    TArc* pred = NULL;
    TNode* nodeColour = NULL;

    switch (opt)
    {
        case DAG_TOPSORT:
        {
            nodeColour = InitNodeColours(NoNode);
            break;
        }
        case DAG_CRITICAL:
        {
            pred = InitPredecessors();
            dist = InitDistanceLabels(-InfFloat);

            #if defined(_LOGGING_)

            LH = LogStart(LOG_METH2,"Expanded nodes:");

            #endif

            break;
        }
        case DAG_SPTREE:
        {
            nodeColour = InitNodeColours(NoNode);
            dist   = InitDistanceLabels(InfFloat);
            pred   = InitPredecessors();

            if (s!=NoNode) dist[s] = 0;

            #if defined(_LOGGING_)

            LH = LogStart(LOG_METH2,"Expanded nodes:");

            #endif

            break;
        }
    }

    for (TNode v=0;v<n;v++)
    {
        if (idg[v]==0)
        {
            Q.Insert(v);

            if (opt==DAG_CRITICAL || (opt==DAG_SPTREE && s==NoNode)) dist[v] = 0;
        }
    }

    TNode nr = 0;
    THandle H = Investigate();
    investigator &I = Investigator(H);

    while (!Q.Empty())
    {
        TNode v = Q.Delete();

        if (opt!=DAG_CRITICAL) nodeColour[v] = nr;

        nr++;

        #if defined(_LOGGING_)

        if (opt!=DAG_TOPSORT && CT.logMeth>1 && dist[v]<InfFloat)
        {
            sprintf(CT.logBuffer," %ld[%g]",v,dist[v]);
            LogAppend(LH,CT.logBuffer);
        }

        #endif

        while (I.Active(v))
        {
            TArc a = I.Read(v);

            if (!Eligible(SPX_PLAIN,a)) continue;

            TNode w = EndNode(a);
            idg[w]--;

            if ((opt==DAG_SPTREE && dist[v]<InfFloat && dist[w]>dist[v]+Length(a)) ||
                (opt==DAG_CRITICAL && dist[w]<dist[v]+Length(a))
               )
            {
                dist[w] = dist[v]+Length(a);
                pred[w] = a;
            }

            if (idg[w]==0) Q.Insert(w);
        }
    }

    #if defined(_LOGGING_)

    if (CT.logMeth>1) LogEnd(LH);

    #endif

    Close(H);
    delete[] idg;

    Trace(m);

    CloseFold(ModDAGSearch);

    if (CT.logRes && nr<n) LogEntry(LOG_RES,"...Graph contains cycles");

    switch (opt)
    {
        case DAG_TOPSORT:
        case DAG_SPTREE:
        {
            if (nr<n)
            {
                for (TNode v=0;v<n;v++)
                    if (nodeColour[v]==NoNode) return v;
            }

            if (opt==DAG_SPTREE)
            {
                if (t!=NoNode) SetBounds(TimerSPTree,dist[t],dist[t]);
            }
            else
            {
                if (CT.logRes) LogEntry(LOG_RES,"...Graph is acyclic");
            }

            return NoNode;
        }
        case DAG_CRITICAL:
        {
            if (nr<n) return NoNode;

            TNode maxNode = NoNode;
            for (TNode v=0;v<n;v++)
            {
                if ((maxNode==NoNode && dist[v]>-InfFloat) ||
                    (maxNode!=NoNode && dist[v]>dist[maxNode])
                   )
                {
                    maxNode = v;
                }
            }

            if (CT.logRes)
            {
                sprintf(CT.logBuffer,"...Critical path length is: %g",dist[maxNode]);
                LogEntry(LOG_RES,CT.logBuffer);
            }

            return maxNode;
        }
    }

    return NoNode;
}
