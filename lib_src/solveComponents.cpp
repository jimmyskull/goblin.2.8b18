
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, July 2003
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file   solveComponents.cpp
/// \brief  A collection of methods to determine node and edge fixed order connected componemts

#include "abstractMixedGraph.h"
#include "sparseGraph.h"
#include "sparseDigraph.h"
#include "staticStack.h"


bool abstractMixedGraph::StronglyConnected() throw()
{
    if (m==0 || (COrientation() && Orientation(0)==0))
    {
        // Graph is undirected, at least implicitly
        return Connected();
    }

    OpenFold(ModStrongComponents,NO_INDENT);

    LogEntry(LOG_METH,"Computing strongly connected components...");

    #if defined(_PROGRESS_)

    InitProgressCounter(2*n);

    #endif

    #if defined(_LOGGING_)

    OpenFold();
    OpenFold();

    #endif

    TNode i = 0;

    TNode* nodeColour = InitNodeColours();
    TArc*  pred = InitPredecessors();

    THandle H = Investigate();
    investigator &I = Investigator(H);
    staticStack<TNode,TFloat> S(n,CT);
        // Ordering of the reverse DFS operations

    #if defined(_TRACING_)

    staticStack<TNode,TFloat> Recovery(n,CT);
        // Needed for reinitialization of predecessors

    #endif

    bool* marked = new bool[n];
    for (TNode v=0;v<n;v++) marked[v] = false;

    for (TNode r=0;r<n;r++)
    {
        if (nodeColour[r]!=NoNode) continue;

        #if defined(_LOGGING_)

        CloseFold();
        LogEntry(LOG_METH2,"Generating forward DFS tree...");
        OpenFold();

        #endif

        TNode u = r;

        while (true)
        {
            if (I.Active(u))
            {
                TArc a = I.Read(u);
                TNode v = EndNode(a);

                if (UCap(a^1)>0 && !Blocking(a^1) && pred[v]==NoArc && nodeColour[v]==NoNode && v!=r)
                {
                    pred[v] = a;
                    u = v;
                }
            }
            else
            {
                // Backtracking

                S.Insert(u);

                #if defined(_TRACING_)

                Recovery.Insert(u);

                #endif

                I.Reset(u);
                marked[u] = true;

                if (u==r) break;
                else u = StartNode(pred[u]);
            }
        }

        Trace(S.Cardinality());

        #if defined(_TRACING_)

        while (!Recovery.Empty())
        {
            u = Recovery.Delete();
            pred[u] = NoArc;
        }

        #endif

        while (!S.Empty())
        {
            TNode s = S.Delete();

            if (nodeColour[s]!=NoNode || !marked[s]) continue;

            u = s;

            #if defined(_LOGGING_)

            THandle LH = NoHandle;

            if (CT.logMeth>1)
            {
                sprintf(CT.logBuffer,"Component %ld: %ld",i,u);
                LH = LogStart(LOG_METH2,CT.logBuffer);
            }

            #endif

            while (true)
            {
                if (I.Active(u))
                {
                    TArc a = I.Read(u);

                    TNode v = EndNode(a);

                    if (UCap(a)>0 && !Blocking(a) && nodeColour[v]==NoNode && marked[v] && v!=s)
                    {
                        pred[v] = a;
                        u = v;
                        marked[u] = false;

                        #if defined(_LOGGING_)

                        if (CT.logMeth>1)
                        {
                            sprintf(CT.logBuffer,",%ld",u);
                            LogAppend(LH,CT.logBuffer);
                        }

                        #endif
                    }
                }
                else
                {
                    // Backtracking

                    #if defined(_TRACING_)

                    Recovery.Insert(u);

                    #endif

                    I.Reset(u);
                    nodeColour[u] = i;

                    if (u==s) break;
                    else u = StartNode(pred[u]);
                }
            }

            #if defined(_LOGGING_)

            if (CT.logMeth>1) LogEnd(LH);

            #endif

            Trace(Recovery.Cardinality());

            #if defined(_TRACING_)

            while (!Recovery.Empty())
            {
                u = Recovery.Delete();
                pred[u] = NoArc;
            }

            #endif

            i++;
        }
    }

    Close(H);
    delete[] marked;

    #if defined(_LOGGING_)

    CloseFold();
    CloseFold();

    #endif

    if (CT.logRes)
    {
        sprintf(CT.logBuffer,"...Graph has %ld strongly connected components",i);
        LogEntry(LOG_RES,CT.logBuffer);
    }

    SetBounds(TimerComponents,i,i);

    CloseFold(ModStrongComponents,NO_INDENT);

    return (i<=1);
}


bool abstractMixedGraph::Connected() throw()
{
    OpenFold(ModComponents,NO_INDENT);

    LogEntry(LOG_METH,"Computing connected components...");

    #if defined(_PROGRESS_)

    InitProgressCounter(n);

    #endif

    #if defined(_LOGGING_)

    OpenFold();

    #endif

    TNode i = 0;

    TNode* nodeColour = InitNodeColours();
    TArc*  pred = InitPredecessors();

    THandle H = Investigate();
    investigator &I = Investigator(H);

    for (TNode r=0;r<n;r++)
    {
        if (nodeColour[r]!=NoNode) continue;

        TNode u = r;

        #if defined(_LOGGING_)

        THandle LH = NoHandle;

        if (CT.logMeth>1)
        {
            sprintf(CT.logBuffer,"Component %ld: %ld",i,u);
            LH = LogStart(LOG_METH2,CT.logBuffer);
        }

        #endif

        while (true)
        {
            if (I.Active(u))
            {
                TArc a = I.Read(u);

                TNode v = EndNode(a);

                if (UCap(a)>0 && pred[v]==NoArc && v!=r)
                {
                    pred[v] = a;
                    u = v;

                    #if defined(_LOGGING_)

                    if (CT.logMeth>1)
                    {
                        sprintf(CT.logBuffer,",%ld",u);
                        LogAppend(LH,CT.logBuffer);
                    }

                    #endif
                }
            }
            else
            {
                // Backtracking

                nodeColour[u] = i;

                if (u==r) break;
                else u = StartNode(pred[u]);
            }
        }

        #if defined(_LOGGING_)

        if (CT.logMeth>1) LogEnd(LH);

        #endif

        i++;
    }

    Close(H);

    Trace(n);

    #if defined(_LOGGING_)

    CloseFold();

    #endif

    if (CT.logRes)
    {
        sprintf(CT.logBuffer,"...Graph has %ld connected components",i);
        LogEntry(LOG_RES,CT.logBuffer);
    }

    SetBounds(TimerComponents,i,i);

    CloseFold(ModComponents,NO_INDENT);

    return (i<=1);
}


abstractMixedGraph::TRetDFS abstractMixedGraph::CutNodes(TArc rootArc,TNode* order,TArc* lowArc) throw()
{
    OpenFold(ModBiconnectivity,NO_INDENT);

    LogEntry(LOG_METH,"Computing blocks and cut nodes... ");

    #if defined(_PROGRESS_)

    InitProgressCounter(n);

    #endif

    #if defined(_LOGGING_)

    OpenFold();

    #endif

    // If the graph is planar, start the search with an exterior arcs preferably
    TArc localRootArc = rootArc;
    for (TNode v=0;v<n && localRootArc>=2*m;v++) localRootArc = First(v);

    if (localRootArc==NoArc)
    {
        LogEntry(LOG_RES,"...Graph is void");
        CloseFold(ModBiconnectivity,NO_INDENT);

        if (n<=1) return DFS_BICONNECTED;
        else return DFS_DISCONNECTED;
    }

    TNode i = 1; // Current ordinal number
    TNode nBlocks = 0; // Current edge colour index
    TNode c = 0; // Current node colour index
    TNode nComponents = 1;
    TNode nLeafs = 0;

    TArc*  pred       = InitPredecessors();
    TNode* nodeColour = InitNodeColours();
    TArc*  edgeColour = InitEdgeColours();

    bool localOrder = (order==NULL);
    if (localOrder) order = new TNode[n];

    bool localLow = (lowArc==NULL);
    if (localLow) lowArc = new TArc[n];

    for (TNode v=0;v<n;v++) lowArc[v] = NoArc;

    TNode* low = new TNode[n];

    staticStack<TNode,TFloat> Q(n,CT);

    THandle H = Investigate();
    investigator &I = Investigator(H);
    bool firstIteration = true;
    TNode r = 0;

    while (r<n)
    {
        TNode u = r;

        if (firstIteration)
        {
            // Force the root arc to be the first DFS tree arc
            // (this is needed for the st-numbering algorithm)

            TNode t = StartNode(localRootArc);
            TNode s = EndNode(localRootArc);

            low[t] = order[t] = i++;
            low[s] = order[s] = i++;
            pred[s] = localRootArc;

            r = t;
            u = s;

            Q.Insert(t);
            if (t!=s) Q.Insert(s);
        }
        else if (nodeColour[r]!=NoNode)
        {
            // Node r has already been reached
            r++;
            continue;
        }
        else
        {
            nComponents++;
            low[r] = order[r] = i++;
        }

        while (true)
        {
            if (I.Active(u))
            {
                TArc a = I.Read(u);

                TNode v = EndNode(a);

                if (UCap(a)>0)
                {
                    if (pred[v]==NoArc && v!=r)
                    {
                        // Tree edge

                        pred[v] = a;
                        order[v] = i++;
                        low[v] = order[v];
                        lowArc[v] = NoArc;
                        Q.Insert(v);
                        u = v;
                    }
                    else if (pred[u]!=(a^1) && low[u]>order[v])
                    {
                        // Backward edge

                        low[u] = order[v];
                        lowArc[u] = a;
                    }
                }
            }
            else
            {
                // Backtracking

                if (u==r)
                {
                    I.Reset(u);

                    while (I.Active(u))
                    {
                        TArc a = I.Read(u);

                        if (edgeColour[a>>1]==NoArc) edgeColour[a>>1] = nBlocks;
                    }

                    break;
                }

                TNode w = StartNode(pred[u]);
                if (low[u]<order[w])
                {
                    if (low[w]>low[u])
                    {
                        low[w] = low[u];
                        lowArc[w] = lowArc[u];
                    }
                }
                else
                {
                    // A block has just been completed
                    // w is the root of the new block

                    nBlocks++;

                    #if defined(_LOGGING_)

                    THandle LH = NoHandle;

                    if (CT.logMeth>1)
                    {
                        sprintf(CT.logBuffer,"Block %ld: %ld,",nBlocks,w);
                        LH = LogStart(LOG_METH2,CT.logBuffer);
                    }

                    #endif

                    // Colour the nodes and arcs of the block
                    // Check if the block forms a leaf in the cut tree

                    TNode v = Q.Delete();
                    bool leaf = true;
                    TNode cardinality = 0;

                    while (v!=u)
                    {
                        if (nodeColour[v]==NoNode)
                        {
                            nodeColour[v] = nBlocks;
                            cardinality++;
                        }
                        else leaf = false;

                        I.Reset(v);
                        while (I.Active(v))
                        {
                            TArc a = I.Read(v);

                            if (edgeColour[a>>1]==NoArc) edgeColour[a>>1] = nBlocks;
                        }

                        #if defined(_LOGGING_)

                        if (CT.logMeth>1)
                        {
                            sprintf(CT.logBuffer,"%ld,",v);
                            LogAppend(LH,CT.logBuffer);
                        }

                        #endif

                        v = Q.Delete();
                    }

                    if (nodeColour[u]==NoNode)
                    {
                        nodeColour[u] = nBlocks;
                        cardinality++;
                    }
                    else leaf = false;

                    if (leaf) nLeafs++;

                    I.Reset(u);

                    while (I.Active(u))
                    {
                        TArc a = I.Read(u);

                        if (edgeColour[a>>1]==NoArc) edgeColour[a>>1] = nBlocks;
                    }

                    #if defined(_LOGGING_)

                    if (CT.logMeth>1)
                    {
                        sprintf(CT.logBuffer,"%ld",u);
                        LogEnd(LH,CT.logBuffer);
                    }

                    #endif

                    if (w==r && nodeColour[r]==NoNode)
                    {
                        nodeColour[r] = nBlocks;
                        cardinality++;
                    }
                    else
                    {
                        // Mark w as a cut node

                        if (nodeColour[w]==NoNode) cardinality++;

                        nodeColour[w] = CONN_CUT_NODE;
                        c++;
                    }

                    Trace(cardinality);
                }

                u = w;
            }
        }

        if (firstIteration)
        {
            firstIteration = false;
            r = 0;
        }
        else r++;
    }

    Close(H);

    delete[] low;

    if (localLow) delete[] lowArc;
    if (localOrder) delete[] order;

    #if defined(_LOGGING_)

    CloseFold();

    #endif

    if (CT.logRes)
    {
        sprintf(CT.logBuffer,
            "...Graph has %ld components, %ld blocks and %ld cut nodes",
            nComponents,nBlocks,c);
        LogEntry(LOG_RES,CT.logBuffer);
    }

    if (localLow) SetBounds(TimerComponents,nBlocks,nBlocks);

    CloseFold(ModBiconnectivity,NO_INDENT);

    if (nComponents>1)                          return DFS_DISCONNECTED;
    if (nBlocks==1)                             return DFS_BICONNECTED;
    if (   nLeafs>1
        || rootArc==NoArc
        || nodeColour[StartNode(rootArc)]==0
       )                                        return DFS_MULTIPLE_BLOCKS;

    return DFS_ALMOST_BICONNECTED;
}


bool abstractMixedGraph::Biconnected() throw()
{
    OpenFold(ModBiconnectivity,NO_INDENT);

    LogEntry(LOG_METH,"Computing 2-connected components...");

    #if defined(_PROGRESS_)

    InitProgressCounter(n+m);

    #endif

    #if defined(_LOGGING_)

    OpenFold();

    #endif

    TNode* order = new TNode[n];
    TArc* low = new TArc[n];

    CutNodes(NoArc,order,low);

    InitPartition();
    for (TNode u=0;u<n;u++) Bud(u);

    TArc* pred = GetPredecessors();
    TNode* nodeColour = GetNodeColours();

    TNode ncomponents = n;
    SetBounds(TimerComponents,1,n);

    for (TArc a=0;a<m;a++)
    {
        TNode v = StartNode(2*a);
        TNode w = EndNode(2*a);

        if ((nodeColour[v]==nodeColour[w] && nodeColour[v]!=CONN_CUT_NODE) ||
            (pred[w]!=2*a && pred[v]!=2*a+1) ||
            (pred[w]==2*a && low[w]!=NoArc && order[EndNode(low[w])]<order[w]) ||
            (pred[v]==2*a+1 && low[v]!=NoArc && order[EndNode(low[v])]<order[v]))
        {
            if (Find(v)!=Find(w))
            {
                ncomponents--;
                Merge(w,v);
                SetUpperBound(TimerComponents,ncomponents);
            }
        }
    }

    for (TNode u=0;u<n;u++) nodeColour[u] = nodeColour[Find(u)];

    Trace(m);

    #if defined(_LOGGING_)

    CloseFold();

    #endif

    delete[] order;
    delete[] low;

    SetLowerBound(TimerComponents,ncomponents);

    if (CT.logRes)
    {
        sprintf(CT.logBuffer,"...Graph has %ld 2-edge connected components",
            ncomponents);
        LogEntry(LOG_RES,CT.logBuffer);
    }

    CloseFold(ModBiconnectivity,NO_INDENT);

    return (ncomponents==1);
}


bool abstractMixedGraph::STNumbering(TArc rootArc,TNode source,TNode target) throw()
{
    OpenFold(ModBiconnectivity,NO_INDENT);

    LogEntry(LOG_METH,"Computing st-numbering...");

    #if defined(_PROGRESS_)

    InitProgressCounter(m+2*n);

    #endif

    #if defined(_LOGGING_)

    OpenFold();

    #endif

    TNode* order = new TNode[n];
    TArc* low = new TArc[n];

    // Obtain a consistent configuration of source, target and rootArc
    if (rootArc>=2*m)
    {
        if (source<n)
        {
            rootArc = First(source);
        }
        else if (ExteriorArc()!=NoArc)
        {
            rootArc = ExteriorArc();
            source = StartNode(rootArc);
        }
        else
        {
            rootArc = First(0);
            source = 0;
        }

        if (target>=n || source==target) target = EndNode(rootArc);
    }
    else
    {
        source = StartNode(rootArc);
        target = EndNode(rootArc);
    }


    TRetDFS retDFS = CutNodes(rootArc,order,low);

    bool feasible = (retDFS==DFS_BICONNECTED);
    TNode* nodeColour = GetNodeColours();

    if (   retDFS==DFS_ALMOST_BICONNECTED
        && nodeColour[source]!=nodeColour[target]
        && nodeColour[source]!=CONN_CUT_NODE
        && nodeColour[target]!=CONN_CUT_NODE
       )
    {
        // source and target are no cut nodes and occur in different blocks.
        // Verify that these blocks form the leaves in the block-cut tree
        feasible = true;
        TArc* edgeColour = GetEdgeColours();
        TNode targetCutNode = NoNode;
        TNode sourceCutNode = NoNode;

        for (TArc a=0;a<2*m && feasible;a++)
        {
            TNode v = StartNode(a);

            if (nodeColour[v]!=0) continue; 

            if (edgeColour[a>>1]==nodeColour[source])
            {
                if (sourceCutNode!=NoNode && v!=sourceCutNode)
                {
                    feasible = false;
                }
                else sourceCutNode = v;
            }

            if (edgeColour[a>>1]==nodeColour[target])
            {
                if (targetCutNode!=NoNode && v!=targetCutNode)
                {
                    feasible = false;
                }
                else targetCutNode = v;
            }
        }
    }

    if (feasible)
    {
        LogEntry(LOG_METH,"Computing ear decomposition...");

        #if defined(_LOGGING_)

        OpenFold();

        #endif

        TNode* nodeColour = InitNodeColours();
        TArc* edgeColour = InitEdgeColours();
        TArc* pred = GetPredecessors();
        TArc* Q = new TArc[n];

        for (TNode v=0;v<n;v++) Q[v] = NoArc;

        TNode nrVertex = 0;
        TArc nrPath = 1;

        staticStack<TNode,TFloat> S1(n,CT);

        // The initial path is the (source,target)-path in the DFS tree defined by pred[]

        #if defined(_LOGGING_)

        THandle LH = NoHandle;

        if (CT.logMeth>1)
        {
            sprintf(CT.logBuffer,"Path 0 : %ld",target);
            LH = LogStart(LOG_METH2,CT.logBuffer);
        }

        #endif

        TNode w = target;
        TArc lenPath = 0;

        while (w!=source)
        {
            TArc a2 = pred[w];
            Q[w] = a2;
            edgeColour[a2>>1] = 0;
            w = StartNode(a2);
            Q[w] = a2^1;
            S1.Insert(w);
            lenPath++;

            #if defined(_LOGGING_)

            if (CT.logMeth>1)
            {
                sprintf(CT.logBuffer,"[%ld]%ld",a2^1,w);
                LogAppend(LH,CT.logBuffer);
            }

            #endif
        }

        #if defined(_LOGGING_)

        if (CT.logMeth>1) LogEnd(LH);

        #endif

        Trace(lenPath);

        THandle H = Investigate();
        investigator &I = Investigator(H);

        while (!S1.Empty())
        {
            TNode u = S1.Delete();

            while (I.Active(u))
            {
                TArc a = I.Read(u);

                if (pred[u]==(a^1) || edgeColour[a>>1]!=NoArc) continue;

                // Construct path Q from u to another visited node v

                TNode v = EndNode(a);

                TNode w = v;

                if (pred[v]==a)
                {
                    // Use the tree arc pred[v], the non-tree arc low[v]
                    // and the unused tree path connecting these arcs
                    w = EndNode(low[v]);
                    Q[w] = low[v];

                    v = StartNode(low[v]);

                    while (v!=u)
                    {
                        Q[v] = pred[v];
                        v = StartNode(pred[v]);
                    }
                }
                else if (order[v]>order[u])
                {
                    // v is a descendent of u, and some descendents of u
                    // have been already visited and charged with a Q[] label.
                    // Use the arc a and the tree path back to a visited node
                    while (Q[w]==NoArc) w = StartNode(pred[w]);

                    Q[v] = a;

                    while (v!=w)
                    {
                        TArc a2 = pred[v];

                        v = StartNode(a2);
                        Q[v] = a2^1;
                    }
                }
                else
                {
                    Q[w] = a;
                }

                // Push path nodes onto stack S1

                #if defined(_LOGGING_)

                THandle LH = NoHandle;

                if (CT.logMeth>1)
                {
                    sprintf(CT.logBuffer,"Path %ld : %ld",nrPath,w);
                    LH = LogStart(LOG_METH2,CT.logBuffer);
                }

                #endif

                TArc lenPath = 0;

                while (w!=u)
                {
                    TArc a2 = Q[w];
                    edgeColour[a2>>1] = nrPath;
                    w = StartNode(a2);
                    lenPath++;

                    if (w!=u) S1.Insert(w);

                    #if defined(_LOGGING_)

                    if (CT.logMeth>1)
                    {
                        sprintf(CT.logBuffer,"[%ld]%ld",a2^1,w);
                        LogAppend(LH,CT.logBuffer);
                    }

                    #endif
                }

                #if defined(_LOGGING_)

                if (CT.logMeth>1) LogEnd(LH);

                #endif

                nrPath++;

                Trace(lenPath);
            }

            nodeColour[u] = nrVertex;
            nrVertex++;
        }

        nodeColour[target] = nrVertex;

        Close(H);

        delete[] Q;

        #if defined(_LOGGING_)

        CloseFold();

        #endif

        LogEntry(LOG_RES,"...Graph has been split into paths");
    }

    #if defined(_LOGGING_)

    CloseFold();

    #endif

    delete[] order;
    delete[] low;

    if (feasible)
    {
        sprintf(CT.logBuffer,"...(%lu,%lu)-numbering found",source,target);
        LogEntry(LOG_RES,CT.logBuffer);
    }
    else
    {
        sprintf(CT.logBuffer,"...No (%lu,%lu)-numbering exists",source,target);
        LogEntry(LOG_RES,CT.logBuffer);
    }

    CloseFold(ModBiconnectivity,NO_INDENT);

    return feasible;
}


bool abstractMixedGraph::Connected(TCap k) throw()
{
    if (k==1) return Connected();
    if (k==2) return (CutNodes()==DFS_BICONNECTED);

    OpenFold(ModComponents,NO_INDENT);

    ReleasePredecessors();

    TCap kAct = NodeConnectivity();

    CloseFold(ModComponents,NO_INDENT);

    return (kAct>=k);
}


bool abstractMixedGraph::EdgeConnected(TCap k) throw()
{
    if (k==1) return Connected();
    if (k==2) return Biconnected();

    OpenFold(ModComponents,NO_INDENT);

    sprintf(CT.logBuffer,"Computing %g-edge connected components...",k);
    LogEntry(LOG_METH,CT.logBuffer);
    OpenFold();

    #if defined(_PROGRESS_)

    InitProgressCounter(n-1);
    SetProgressNext(1);

    #endif

    SetBounds(TimerComponents,1,n);

    ReleasePredecessors();
    TNode* nodeColour = InitNodeColours(0);

    TNode cNext = 1;
    TNode cCurrent = 0;
    TNode nMerged = 0;
    TNode* map = new TNode[n]; 
    TNode* revMap = new TNode[n]; 

    while (cCurrent<cNext && CT.SolverRunning())
    {
        TNode nCurrent = 0;
        for (TNode v=0;v<n;v++)
        {
            if (nodeColour[v]==cCurrent)
            {
                revMap[nCurrent] = v;
                map[v] = nCurrent;
                nCurrent++;
            }
            else map[v] = NoNode;
        }

        if (nCurrent>1)
        {
            graph G(nCurrent,CT);

            for (TArc a=0;a<m;a++)
            {
                TNode x = StartNode(2*a);
                TNode y = EndNode(2*a);

                if (nodeColour[x]==cCurrent && nodeColour[y]==cCurrent)
                    G.InsertArc(map[x],map[y],UCap(2*a),0);
            }

            #if defined(_TRACING_)

            graphRepresentation* GR = G.Representation();

            for (TNode u=0;u<nCurrent && Dim()>0;u++)
            {
                TNode v = revMap[u];

                GR -> SetC(u,0,C(v,0));
                GR -> SetC(u,1,C(v,1));
            }

            #endif

            #if defined(_LOGGING_)

            if (CT.logMeth>1)
            {
                sprintf(CT.logBuffer,"Trying to separate: %ld",revMap[0]);
                THandle LH = LogStart(LOG_METH2,CT.logBuffer);

                for (TNode u=1;u<nCurrent;u++)
                {
                    sprintf(CT.logBuffer,",%ld",revMap[u]);
                    LogAppend(LH,CT.logBuffer);
                }

                LogEnd(LH);
            }

            #endif

            if (G.EdgeConnectivity()>=k)
            {
                nMerged += nCurrent;
                cCurrent++;

                if (n-nMerged+cCurrent>=LowerBound(TimerComponents))
                    SetUpperBound(TimerComponents,n-nMerged+cCurrent);
            }
            else
            {
                for (TNode u=0;u<nCurrent;u++)
                {
                    TNode v = revMap[u];

                    if (G.NodeColour(u)==CONN_RIGHT_HAND) nodeColour[v] = cNext;
                }

                cNext++;

                if (cNext<=UpperBound(TimerComponents))
                     SetLowerBound(TimerComponents,cNext);
            }
        }
        else
        {
            nMerged += nCurrent;
            cCurrent++;

            if (n-nMerged+cCurrent>=LowerBound(TimerComponents))
                SetUpperBound(TimerComponents,n-nMerged+cCurrent);
        }

        #if defined(_PROGRESS_)

        SetProgressCounter(n-1+nMerged-cCurrent+cNext);

        #endif
    }

    CloseFold();

    delete[] map;
    delete[] revMap;

    sprintf(CT.logBuffer,"...Graph has %ld %g-edge connected components",cNext,k);
    LogEntry(LOG_RES,CT.logBuffer);

    CloseFold(ModComponents,NO_INDENT);

    return cNext==1;
}


bool abstractMixedGraph::StronglyConnected(TCap k) throw()
{
    if (k==1) return StronglyConnected();

    OpenFold(ModComponents,NO_INDENT);

    ReleasePredecessors();

    TCap kAct = StrongNodeConnectivity();

    CloseFold(ModComponents,NO_INDENT);

    return (kAct>=k);
}


bool abstractMixedGraph::StronglyEdgeConnected(TCap k) throw()
{
    if (k==1) return StronglyConnected();

    OpenFold(ModComponents,NO_INDENT);

    sprintf(CT.logBuffer,"Computing strong %g-edge connected components...",k);
    LogEntry(LOG_METH,CT.logBuffer);
    OpenFold();

    #if defined(_PROGRESS_)

    InitProgressCounter(n-1);
    SetProgressNext(1);

    #endif

    SetBounds(TimerComponents,1,n);

    ReleasePredecessors();
    TNode* nodeColour = InitNodeColours(0);

    TNode cNext = 1;
    TNode cCurrent = 0;
    TNode nMerged = 0;
    TNode* map = new TNode[n]; 
    TNode* revMap = new TNode[n]; 

    while (cCurrent<cNext && CT.SolverRunning())
    {
        TNode nCurrent = 0;

        for (TNode v=0;v<n;v++)
        {
            if (nodeColour[v]==cCurrent)
            {
                revMap[nCurrent] = v;
                map[v] = nCurrent;
                nCurrent++;
            }
            else map[v] = NoNode;
        }

        if (nCurrent>1)
        {
            diGraph G(nCurrent,CT);

            for (TArc a=0;a<m;a++)
            {
                TNode x = StartNode(2*a);
                TNode y = EndNode(2*a);

                if (nodeColour[x]==cCurrent && nodeColour[y]==cCurrent)
                    G.InsertArc(map[x],map[y],UCap(2*a),0);
            }

            #if defined(_TRACING_)

            graphRepresentation* GR = G.Representation();

            for (TNode u=0;u<nCurrent && Dim()>0;u++)
            {
                TNode v = revMap[u];

                GR -> SetC(u,0,C(v,0));
                GR -> SetC(u,1,C(v,1));
            }

            #endif

            #if defined(_LOGGING_)

            if (CT.logMeth>1)
            {
                sprintf(CT.logBuffer,"Trying to separate: %ld",revMap[0]);
                THandle LH = LogStart(LOG_METH2,CT.logBuffer);

                for (TNode u=1;u<nCurrent;u++)
                {
                    sprintf(CT.logBuffer,",%ld",revMap[u]);
                    LogAppend(LH,CT.logBuffer);
                }

                LogEnd(LH);
            }

            #endif

            if (G.StrongEdgeConnectivity()>=k)
            {
                nMerged += nCurrent;
                cCurrent++;

                if (n-nMerged+cCurrent>=LowerBound(TimerComponents))
                    SetUpperBound(TimerComponents,n-nMerged+cCurrent);
            }
            else
            {
                for (TNode u=0;u<nCurrent;u++)
                {
                    TNode v = revMap[u];

                    if (G.NodeColour(u)==CONN_RIGHT_HAND) nodeColour[v] = cNext;
                }

                cNext++;

                if (cNext<=UpperBound(TimerComponents))
                     SetLowerBound(TimerComponents,cNext);
            }
        }
        else
        {
            nMerged += nCurrent;
            cCurrent++;

            if (n-nMerged+cCurrent>=LowerBound(TimerComponents))
                SetUpperBound(TimerComponents,n-nMerged+cCurrent);
        }

        #if defined(_PROGRESS_)

        SetProgressCounter(n-1+nMerged-cCurrent+cNext);

        #endif
    }

    CloseFold();

    delete[] map;
    delete[] revMap;

    sprintf(CT.logBuffer,
        "...Graph has %ld strong %g-edge connected components",cNext,k);
    LogEntry(LOG_RES,CT.logBuffer);

    CloseFold(ModComponents,NO_INDENT);

    return cNext==1;
}
