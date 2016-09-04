
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, December 1998
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file   sparseGraph.cpp
/// \brief  Constructor methods for sparse undirected graph objects

#include "sparseGraph.h"


graph::graph(TNode _n,goblinController& _CT,bool _mode) throw() :
    managedObject(_CT),
    abstractGraph(_n,TArc(0)),
    X(static_cast<const graph&>(*this))
{
    X.SetCDemand(1);
    mode = _mode;

    LogEntry(LOG_MEM,"...Sparse graph instanciated");
}


graph::graph(const char* fileName,goblinController& _CT) throw(ERFile,ERParse) :
    managedObject(_CT),
    abstractGraph(TNode(0),TArc(0)),
    X(static_cast<const graph&>(*this))
{
    #if defined(_TIMERS_)

    CT.globalTimer[TimerIO] -> Enable();

    #endif

    LogEntry(LOG_IO,"Loading graph...");

    if (!CT.logIO && CT.logMem) LogEntry(LOG_MEM,"Loading graph...");

    goblinImport F(fileName,CT);

    F.Scan("graph");
    ReadAllData(F);

    if (CT.sourceNode<n) SetSourceNode(CT.sourceNode);
    else SetSourceNode(NoNode);

    if (CT.targetNode<n) SetTargetNode(CT.targetNode);
    else SetTargetNode(NoNode);

    if (CT.rootNode<n) SetRootNode(CT.rootNode);
    else SetRootNode(NoNode);

    int l = strlen(fileName)-4;
    char* tmpLabel = new char[l+1];
    memcpy(tmpLabel,fileName,l);
    tmpLabel[l] = 0;
    SetLabel(tmpLabel);
    delete[] tmpLabel;
    CT.SetMaster(Handle());

    #if defined(_TIMERS_)

    CT.globalTimer[TimerIO] -> Disable();

    #endif
}


graph::graph(abstractMixedGraph& G,TOption options) throw() :
    managedObject(G.Context()),
    abstractGraph(G.N(),TArc(0)),
    X(static_cast<const graph&>(*this))
{
    X.SetCapacity(G.N(),G.M());

    TIndex* originalArc = (options & OPT_MAPPINGS) ? new TArc[G.M()] : NULL;

    if (options & OPT_CLONE)
    {
        for (TNode u=0;u<n;u++)
        {
            X.SetDemand(u,G.Demand(u));

            for (TDim i=0;i<G.Dim();i++) X.SetC(u,i,G.C(u,i));
        }

        for (TArc a=0;a<G.M();a++)
        {
            TNode u = G.StartNode(2*a);
            TNode v = G.EndNode(2*a);
            TCap tmpCap = (options & OPT_SUB) ? TCap(G.Sub(2*a)) : G.UCap(2*a);

            TArc a1 = InsertArc(u,v,tmpCap,G.Length(2*a),G.LCap(2*a));

            if (originalArc) originalArc[a1] = 2*a;
        }

        for (TNode u=0;u<n;u++)
        {
            TArc a = G.First(u);

            if (a==NoArc) continue;

            do
            {
                TArc a2 = G.Right(a,u);
                X.SetRight(a,a2);
                a = a2;
            }
            while (a!=G.First(u));

            X.SetFirst(u,a);
        }

        if (G.ExteriorArc()!=NoArc)
        {
            face = new TNode[2*m];

            for (TArc i=0;i<2*m;i++) face[i] = G.Face(i);

            SetExteriorArc(G.ExteriorArc());
        }

        LogEntry(LOG_MEM,"...Graph clone generated");
    }
    else
    {
        LogEntry(LOG_MAN,"Computing underlying graph...");

        TNode* adjacent = new TNode[n];
        for (TNode w=0;w<n;w++) adjacent[w] = NoNode;

        THandle H = G.Investigate();
        investigator &I = G.Investigator(H);

        for (TNode u=0;u<n;u++)
        {
            for (TDim i=0;i<G.Dim();i++) X.SetC(u,i,G.C(u,i));

            while (I.Active(u))
            {
                TArc a = I.Read(u);
                TNode v = G.EndNode(a);
                TCap tmpCap = (options & OPT_SUB) ? TCap(G.Sub(a)) : G.UCap(a);

                if (   tmpCap>0 && u<v
                    && (adjacent[v]!=u || (options & OPT_PARALLELS))
                   )
                {
                    TArc a1 = InsertArc(u,v,tmpCap,G.Length(a),G.LCap(a));
                    adjacent[v] = u;

                    if (originalArc) originalArc[a1] = a;
                }
            }
        }
        G.Close(H);

        delete[] adjacent;

        X.SetCapacity(N(),M());
    }

    if (options & OPT_MAPPINGS)
    {
        TIndex* originalArcExport = registers.RawArray<TIndex>(*this,TokRegOriginalArc);
        memcpy(originalArcExport,originalArc,sizeof(TIndex)*m);
        delete[] originalArc;
    }
}


unsigned long graph::Size() const throw()
{
    return
          sizeof(graph)
        + managedObject::Allocated()
        + abstractMixedGraph::Allocated()
        + abstractGraph::Allocated();
}


unsigned long graph::Allocated() const throw()
{
    return 0;
}


graph::~graph() throw()
{
    LogEntry(LOG_MEM,"...Sparse graph disallocated");

    if (CT.traceLevel==2 && !mode) Display();
}


complementaryGraph::complementaryGraph(abstractMixedGraph &G,TOption options)
    throw(ERRejected) :
    managedObject(G.Context()),
    graph(G.N(),G.Context())
{
    #if defined(_FAILSAVE_)

    if (G.N()*(G.N()-1)/2>=CT.MaxArc())
        Error(ERR_REJECTED,"complementaryGraph","Number of arcs is out of range");

    #endif

    LogEntry(LOG_MAN,"Generating complementary graph...");

    if (G.Dim()>0)
    {
        for (TNode v=0;v<n;v++)
            for (TDim i=0;i<G.Dim();i++) X.SetC(v,i,G.C(v,i));
    }

    for (TNode u=0;u<n;u++)
    {
        for (TNode v=u+1;v<n;v++)
        {
            if (G.Adjacency(u,v)==NoArc && G.Adjacency(v,u)==NoArc)
            {
                if (CT.Rand(2)) InsertArc(v,u);
                else InsertArc(u,v);
            }
        }
    }

    if (CT.traceLevel==2) Display();
}


planarLineGraph::planarLineGraph(abstractMixedGraph &G,TOption options) throw(ERRejected) :
    managedObject(G.Context()),
    graph(G.M(),G.Context())
{
    #if defined(_FAILSAVE_)

    if (G.M()>=CT.MaxNode())
        Error(ERR_REJECTED,"planarLineGraph","Number of arcs is out of range");

    #endif

    LogEntry(LOG_MAN,"Generating planar line graph...");

    for (TDim i=0;i<G.Dim();i++)
    {
        for (TNode v=0;v<G.M();v++)
        {
            X.SetC(v,i,(G.C(G.StartNode((TArc)2*v),i)+G.C(G.EndNode((TArc)2*v),i))/2);
        }
    }

    TArc* mapToArcLeft  = new TArc[2*G.M()];
    TArc* mapToArcRight = new TArc[2*G.M()];
    TArc aExtG = G.ExteriorArc();
    TArc aExt  = NoArc;

    for (TArc a1=0;a1<G.M();a1++)
    {
        TArc a2 = G.Right(2*a1,G.StartNode(2*a1));
        TArc aNew = InsertArc((TNode)a1,(TNode)(a2>>1));
        mapToArcLeft[a2] = mapToArcRight[2*a1] = aNew;

        if (aExtG==2*a1+1) aExt = 2*aNew;

        a2 = G.Right(2*a1+1,G.StartNode(2*a1+1));
        aNew = InsertArc((TNode)a1,(TNode)(a2>>1));
        mapToArcLeft[a2] = mapToArcRight[2*a1+1] = aNew;

        if (aExtG==2*a1) aExt = 2*aNew;
    }


    // Order the incidences

    for (TArc a=0;a<G.M();a++)
    {
        TArc a1 = 2*mapToArcRight[2*a];
        TArc a2 = 2*mapToArcLeft[2*a]+1;
        TArc a3 = 2*mapToArcRight[2*a+1];
        TArc a4 = 2*mapToArcLeft[2*a+1]+1;

        X.SetRight(a1,a2);
        X.SetRight(a2,a3);
        X.SetRight(a3,a4);
        X.SetRight(a4,a1);
    }

    delete[] mapToArcLeft;
    delete[] mapToArcRight;

    if (aExt!=NoArc) MarkExteriorFace(aExt);

    if (CT.traceLevel==2) Display();
}


vertexTruncation::vertexTruncation(abstractMixedGraph &G,TOption options) throw(ERRejected) :
    managedObject(G.Context()),
    graph(2*G.M(),G.Context())
{
    #if defined(_FAILSAVE_)

    if (G.ExtractEmbedding(PLANEXT_DEFAULT)==NoNode)
    {
        Error(ERR_REJECTED,"vertexTruncation","Input graph is not embedded");
    }

    #endif

    // Map the original edges

    // Determine the original node degrees (only for coordinate assignment)
    TArc* degG = new TArc[G.N()];
    for (TNode v=0;v<G.N();v++) degG[v] = 0;
    for (TArc a=0;a<2*G.M();a++) degG[G.StartNode(a)]++;

    for (TArc a=0;a<G.M();a++)
    {
        InsertArc(2*a,2*a+1);

        // Determine the coordinates of the generated nodes as convex combinations
        // from the original nodes. The used parameter lambda will lead to regular
        // faces when the original graph is regular
        TNode v = G.StartNode(2*a);
        TNode w = G.EndNode(2*a);

        TFloat alpha = PI*(degG[v]-2)/(2.0*degG[v]);
        TFloat lambda = 0.5 / (1.0+sin(alpha));

        for (TDim i=0;i<G.Dim();i++)
        {
            X.SetC(2*a,i,(1-lambda)*G.C(v,i)+lambda*G.C(w,i));
        }

        alpha = PI*(degG[w]-2)/(2.0*degG[w]);
        lambda = 0.5 / (1.0+sin(alpha));

        for (TDim i=0;i<G.Dim();i++)
        {
            X.SetC(2*a+1,i,lambda*G.C(v,i)+(1-lambda)*G.C(w,i));
        }
    }

    delete[] degG;


    // Map the former nodes to cycles of the adjacent arcs
    TArc* mapToArcRight = new TArc[2*G.M()];

    for (TNode v=0;v<G.N();v++)
    {
        TArc a = G.First(v);

        if (a==NoArc)
        {
            Error(ERR_REJECTED,"vertexTruncation","Isolated node detected");
        }

        do
        {
            TArc a2 = G.Right(a,v);
            TArc aNew = InsertArc(a,a2);
            mapToArcRight[a] = 2*aNew;
            a = a2;
        }
        while (a!=G.First(v));
    }


    // Order the incidences (Note that every node has degree 3)
    for (TArc a=0;a<2*G.M();a++) X.SetRight(a,mapToArcRight[a]);

    if (G.ExteriorArc()!=NoArc) MarkExteriorFace(G.ExteriorArc());

    delete[] mapToArcRight;


    if (CT.traceLevel==2) Display();
}


facetSeparation::facetSeparation(abstractMixedGraph &G,TOptRotation mode) throw(ERRejected) :
    managedObject(G.Context()),
    graph(2*G.M(),G.Context())
{
    #if defined(_FAILSAVE_)

    if (G.ExtractEmbedding(PLANEXT_DEFAULT)==NoNode)
    {
        Error(ERR_REJECTED,"vertexTruncation","Input graph is not embedded");
    }

    #endif

    // Generate a cycle for every original node. The nodes on this cycle are
    // identified with the original arcs

    for (TArc a=0;a<2*G.M();a++)
    {
        TArc a1 = G.Right(a,G.StartNode(a));

        InsertArc(a,a1);

        // Determine the coordinates of the generated nodes as convex combinations
        // from the original nodes.
        TNode u = G.StartNode(a);
        TNode v = G.EndNode(a);
        TNode w = G.EndNode(a1);

        for (TDim i=0;i<G.Dim();i++)
        {
            if (mode==ROT_LEFT)
            {
                X.SetC(a1,i,0.5*G.C(u,i)+0.3*G.C(v,i)+0.2*G.C(w,i));
            }
            else if (mode==ROT_RIGHT)
            {
                X.SetC(a1,i,0.5*G.C(u,i)+0.2*G.C(v,i)+0.3*G.C(w,i));
            }
            else
            {
                X.SetC(a1,i,0.6*G.C(u,i)+0.2*G.C(v,i)+0.2*G.C(w,i));
            }
        }
    }

    // Map the original arcs and regions
    for (TArc a=0;a<2*G.M();a++)
    {
        TArc a2 = G.Right(a,G.StartNode(a));
        InsertArc(a2,a^1);
    }

    // Order the incidences to support planar representations
    if (mode==ROT_LEFT)
    {
        for (TArc a=0;a<G.M();a++)
        {
            TArc a2 = G.Right(2*a,G.StartNode(2*a));
            TArc a3 = G.Right(2*a+1,G.StartNode(2*a+1));

            InsertArc(a2,a3);
        }

        for (TArc a=0;a<2*G.M();a++)
        {
            TArc a2 = G.Right(a,G.StartNode(a));

            X.SetRight(2*a2,2*a+1);
            X.SetRight(2*a+1,8*G.M()+a);
            X.SetRight(8*G.M()+a,4*G.M()+2*a);
            X.SetRight(4*G.M()+2*a,4*G.M()+2*(a2^1)+1);
            X.SetRight(4*G.M()+2*(a2^1)+1,2*a2);
            X.SetFirst(a2,4*G.M()+2*a);
        }
    }
    else if (mode==ROT_RIGHT)
    {
        for (TArc a=0;a<G.M();a++)
        {
            InsertArc(2*a,2*a+1);
        }

        for (TArc a=0;a<2*G.M();a++)
        {
            TArc a2 = G.Right(a,G.StartNode(a));

            X.SetRight(2*a2,2*a+1);
            X.SetRight(2*a+1,4*G.M()+2*a);
            X.SetRight(4*G.M()+2*a,4*G.M()+2*(a2^1)+1);
            X.SetRight(4*G.M()+2*(a2^1)+1,8*G.M()+a2);
            X.SetRight(8*G.M()+a2,2*a2);
            X.SetFirst(a2,4*G.M()+2*a);
        }
    }
    else
    {
        for (TArc a=0;a<2*G.M();a++)
        {
            TArc a2 = G.Right(a,G.StartNode(a));

            X.SetRight(2*a2,2*a+1);
            X.SetRight(2*a+1,4*G.M()+2*a);
            X.SetRight(4*G.M()+2*a,4*G.M()+2*(a2^1)+1);
            X.SetRight(4*G.M()+2*(a2^1)+1,2*a2);
            X.SetFirst(a2,4*G.M()+2*a);
        }
    }

    if (CT.traceLevel==2) Display();
}


dualGraph::dualGraph(abstractMixedGraph &G,TOption options) throw(ERRejected) :
    managedObject(G.Context()),
    graph(G.M()-G.N()+2,G.Context())
{
    #if defined(_FAILSAVE_)

    if (G.M()-G.N()+2>=CT.MaxNode())
        Error(ERR_REJECTED,"dualGraph","Number of regions is out of range");

    #endif

    // Place nodes
    if (G.ExtractEmbedding(PLANEXT_DUAL,
            reinterpret_cast<void*>(static_cast<abstractMixedGraph*>(this)))==NoNode)
    {
        Error(ERR_REJECTED,"dualGraph","Input graph is not embedded");
    }

    // Insert edges
    TArc* predArc = new TArc[2*G.M()];

    for (TArc a=0;a<G.M();a++)
    {
        TNode u = G.Face(2*a);
        TNode v = G.Face(2*a+1);

        InsertArc(v,u);

        predArc[2*a]   = G.Right(2*a,G.StartNode(2*a));
        predArc[2*a+1] = G.Right(2*a+1,G.StartNode(2*a+1));
    }

    // Set up dual planar representation
    X.ReorderIncidences(predArc);

    delete[] predArc;

    if (G.Dim()>=2) X.Layout_ArcAlignment();

    if (CT.traceLevel==2) Display();
}


spreadOutRegular::spreadOutRegular(abstractMixedGraph &G,TOption options) throw(ERRejected) :
    managedObject(G.Context()),
    graph(G.N(),G.Context())
{
    #if defined(_FAILSAVE_)

    if (G.ExtractEmbedding(PLANEXT_DEFAULT)==NoNode)
    {
        Error(ERR_REJECTED,"spreadOutRegular","Input graph is not embedded");
    }

    #endif

    TArc* predG = G.GetPredecessors();

    #if defined(_FAILSAVE_)

    if (!predG)
        Error(ERR_REJECTED,"spreadOutRegular","Missing predecessor labels");

    #endif

    // Determine the start nodes assigned with the (at most) two arcs in
    // the outerplanar graph assigned with each arc in the original graph

    TNode nodeCount = 0;
    TNode* startNode1 = new TNode[2*G.M()];
    TNode* startNode2 = new TNode[2*G.M()];

    for (TNode v=0;v<G.N();v++)
    {
        TArc a = G.First(v);
        TArc thisDegree = 0;

        do
        {
            if (    predG[G.EndNode(a)]==a
                 || predG[G.EndNode(a^1)]==(a^1) )
            {
                thisDegree++;
            }

            a = G.Right(a,v);
        }
        while (a!=G.First(v));

        if (thisDegree==0)
        {

            delete[] startNode1;
            delete[] startNode2;

            Error(ERR_REJECTED,"spreadOutRegular",
                "Predecessor labels do not constitute a spanning tree");
        }

        while (   predG[G.EndNode(a)]!=a
               && predG[G.EndNode(a^1)]!=(a^1) )
        {
            a = G.Right(a,v);
        }

        TArc a0 = a;
        TNode vStart = v;

        do
        {
            if (   predG[G.EndNode(a)]==a
                || predG[G.EndNode(a^1)]==(a^1) )
            {
                startNode2[a] = vStart;
            }
            else
            {
                startNode2[a] = NoNode;
            }

            a = G.Right(a,v);
            startNode1[a] = vStart;

            if (   a!=a0
                && (   predG[G.EndNode(a)]==a
                    || predG[G.EndNode(a^1)]==(a^1) )
               )
            {
                vStart = InsertNode();
                nodeCount++;
            }
        }
        while (a!=a0);
    }


    // Generate the edges 

    TArc* mapToArc1  = new TArc[G.M()];
    TArc* mapToArc2  = new TArc[G.M()];

    for (TArc a=0;a<G.M();a++)
    {
        if (startNode2[2*a]==NoNode)
        {
            // Not a predecessor arc
            mapToArc1[a] = InsertArc(startNode1[2*a],startNode1[2*a+1]);
            mapToArc2[a] = NoArc;
        }
        else
        {
            // Predecessor arcs are mapped twice
            mapToArc1[a] = InsertArc(startNode1[2*a],startNode2[2*a+1]);
            mapToArc2[a] = InsertArc(startNode1[2*a+1],startNode2[2*a]);
        }
    }


    // Order the incidences to obtain an outerplanar representation

    for (TArc a=0;a<2*G.M();a++)
    {
        if (mapToArc2[a>>1]==NoArc) continue;

        TArc firstIndex = NoArc;

        if (a&1)
        {
            firstIndex = 2*mapToArc2[a>>1]+1;
        }
        else
        {
            firstIndex = 2*mapToArc1[a>>1]+1;
        }

        TArc a0 = a^1;
        TNode v = G.StartNode(a0);
        TArc thisIndex = firstIndex;
        TArc rightIndex = NoArc;

        do
        {
            TArc ar  = G.Right(a0,v);

            if (mapToArc2[ar>>1]==NoArc)
            {
                rightIndex = 2*mapToArc1[ar>>1]|(ar&1);
            }
            else if (ar&1)
            {
                rightIndex = 2*mapToArc2[ar>>1];
            }
            else
            {
                rightIndex = 2*mapToArc1[ar>>1];
            }

            if (StartNode(rightIndex)!= StartNode(thisIndex))
            {
                rightIndex ^= 1;
            }

            X.SetRight(thisIndex,rightIndex);
            a0 = ar;
            thisIndex = rightIndex;
        }
        while (mapToArc2[a0>>1]==NoArc);

        X.SetRight(thisIndex,firstIndex);
        X.SetFirst(StartNode(thisIndex),thisIndex);
        SetExteriorArc(firstIndex);
    }

    delete[] mapToArc1;
    delete[] mapToArc2;

    delete[] startNode1;
    delete[] startNode2;

    Layout_Equilateral();

    if (CT.traceLevel==2) Display();
}


tiling::tiling(abstractMixedGraph &G,TOption options,TNode x,TNode y)
    throw() : managedObject(G.Context()),
    graph(TNode((x+1)*(y+1)+x*y*(G.N()-4)),G.Context())
{
    X.SetCapacity(n,2*x*y+x+y+x*y*G.M());

    TNode offset = (x+1)*(y+1);
    for (TNode j=0;j<y;j++)
    {
        for (TNode i=0;i<x;i++)
        {
            for (TNode v0=0;v0<G.N();v0++)
            {
                if ((j==0 || !(v0==1 || v0==0)) &&
                    (i==0 || !(v0==2 || v0==0))    )
                {
                    TNode v = Index(i,j,x,offset,v0);

                    if (G.Dim()>=2)
                    {
                        X.SetC(v,0,i*(G.CMax(0))+(G.C(v0,0)));
                        X.SetC(v,1,j*(G.CMax(1))+(G.C(v0,1)));
                    }
                }
            }

            for (TArc a=0;a<G.M();a++)
            {
                TNode u0 = G.StartNode(2*a);
                TNode v0 = G.EndNode(2*a);

                TNode u = Index(i,j,x,offset,u0);
                TNode v = Index(i,j,x,offset,v0);

                if ((j==0 || !((u0==0 && v0==1) || (v0==0 && u0==1))) &&
                    (i==0 || !((u0==0 && v0==2) || (v0==0 && u0==2)))    )
                {
                    if (CT.Rand(2)) InsertArc(u,v);
                    else InsertArc(v,u);
                }
            }
            offset += G.N()-4;
        }
    }

    if (CT.traceLevel==2) Display();
}


TNode tiling::Index (TNode i,TNode j,TNode x,TNode offset,TNode w) throw()
{
    switch (w)
    {
        case 0: return j*(x+1)+i;
        case 1: return j*(x+1)+i+1;
        case 2: return (j+1)*(x+1)+i;
        case 3: return (j+1)*(x+1)+i+1;
        default: return offset+w-4;
    }
}


voronoiDiagram::voronoiDiagram(abstractMixedGraph& _G,const indexSet<TNode>& _Terminals) throw() :
    managedObject(_G.Context()),
    graph(_G.VoronoiRegions(_Terminals),_G.Context(),true),
    G(_G),Terminals(_Terminals)
{
    LogEntry(LOG_MAN,"Contracting partial trees...");

    X.SetCapacity(G.N(),G.M());

    TNode* mapNodes = new TNode[G.N()];

    for (TNode v=0;v<G.N();v++) mapNodes[v] = NoNode;

    TNode i = 0;

    for (TNode v=0;v<G.N();v++)
    {
        if (mapNodes[G.Find(v)]==NoNode) mapNodes[G.Find(v)] = i++;
        mapNodes[v] = mapNodes[G.Find(v)];

        if (Terminals.IsMember(v))
        {
            for (TDim i=0;i<G.Dim();i++) X.SetC(mapNodes[v],i,G.C(v,i));
        }
    }

    revMap = new TArc[G.M()];
    goblinHashTable<TArc,TArc> Adj(i*i,G.M(),NoArc,CT);

    for (TArc a=0;a<G.M();a++)
    {
        TNode u = G.StartNode(2*a);
        TNode v = G.EndNode(2*a);
        TNode u2 = mapNodes[G.Find(u)];
        TNode v2 = mapNodes[G.Find(v)];

        if (u2!=v2)
        {
            TFloat l = G.Dist(u)+G.Dist(v)+G.Length(2*a);

            TArc j = i*u2+v2;

            if (u2>v2) j = i*v2+u2;

            TArc a2 = Adj.Key(j);

            if (a2==NoArc)
            {
                a2 = InsertArc(u2,v2,1,l);
                Adj.ChangeKey(j,a2);
                revMap[a2] = a;
            }
            else
            {
                if (Length(2*a2)>l)
                {
                    X.SetLength(2*a2,l);
                    revMap[a2] = a;
                }
            }
        }
    }

    delete[] mapNodes;

    X.SetCapacity(N(),M());

    if (CT.traceLevel==2) Display();
}


TFloat voronoiDiagram::UpdateSubgraph() throw()
{
    LogEntry(LOG_METH,"Mapping tree to original graph...");

    TFloat ret = 0;

    G.InitSubgraph();

    TArc* pred = GetPredecessors();

    for (TNode v=0;v<n;v++)
    {
        if (pred[v]==NoArc) continue;

        TArc a = revMap[pred[v]>>1];

        G.SetSub(2*a,1);

        TNode u = G.StartNode(2*a);

        while (!Terminals.IsMember(u))
        {
            TArc a = G.Pred(u);
            G.SetSub(a,1);
            u = G.StartNode(a);
        }

        u = G.EndNode(2*a);

        while (!Terminals.IsMember(u))
        {
            TArc a = G.Pred(u);
            G.SetSub(a,1);
            u = G.StartNode(a);
        }
    }

    return ret;
}


voronoiDiagram::~voronoiDiagram() throw()
{
    if (CT.traceLevel==2) Display();

    delete[] revMap;

    LogEntry(LOG_MAN,"...Voronoi diagram deleted");
}


triangularGraph::triangularGraph(TNode cardinality,goblinController& _CT) throw() :
    managedObject(_CT),
    graph(TNode(0),_CT)
{
    LogEntry(LOG_MAN,"Generating triangular graph...");

    TFloat radius = CT.nodeSep*cardinality/10.0;
    TNode** node = new TNode*[cardinality];

    for (TNode i=0;i<cardinality;i++)
    {
        node[i] = new TNode[cardinality];
        for (TNode j=i+1;j<cardinality;j++)
        {
            node[i][j] = InsertNode();
            SetC(node[i][j],0,+radius*(7*sin(i*2*PI/cardinality)+3*sin(j*2*PI/cardinality)));
            SetC(node[i][j],1,-radius*(3*cos(i*2*PI/cardinality)+7*cos(j*2*PI/cardinality)));

            for (TNode k=0;k<i;k++)   InsertArc(node[i][j],node[k][j]);
            for (TNode k=i+1;k<j;k++) InsertArc(node[i][j],node[i][k]);
            for (TNode k=0;k<i;k++)   InsertArc(node[i][j],node[k][i]);
        }
    }

    for (TNode i=0;i<cardinality;i++) delete[] node[i];
    delete[] node;

    Layout_ConvertModel(LAYOUT_FREESTYLE_POLYGONES);
}


sierpinskiTriangle::sierpinskiTriangle(TNode depth,goblinController &_CT) throw() :
    managedObject(_CT),
    graph(TNode(0),_CT)
{
    LogEntry(LOG_MAN,"Generating Sierpinski triangle...");

    graph* smallTriangle = NULL;
    graph* bigTriangle = NULL;
    TFloat radius = (CT.nodeSep>0) ? CT.nodeSep : 1.0;

    if (depth==0)
    {
        smallTriangle = this;
    }
    else
    {
        smallTriangle = new graph(TNode(0),CT);
    }

    for (int j=0;j<3;j++)
    {
        smallTriangle -> InsertNode();
        smallTriangle -> SetC(j,0,radius*sin(j*2*PI/3));
        smallTriangle -> SetC(j,1,-radius*cos(j*2*PI/3));
    }


    smallTriangle -> InsertArc(0,1);
    smallTriangle -> InsertArc(1,2);
    smallTriangle -> InsertArc(2,0);

    for (TNode i=0;i<depth;i++)
    {
        TNode smallN = smallTriangle->N();
        TNode bigN = 3*smallN-3;

        if (depth==i+1)
        {
            for (TNode v=0;v<bigN;v++) InsertNode();

            bigTriangle = this;
        }
        else
        {
            bigTriangle = new graph(bigN,CT);
        }


        for (unsigned j=0;j<3;j++)
        {
            bigTriangle -> SetC(j,0,2*radius*sin(j*2*PI/3));
            bigTriangle -> SetC(j,1,-2*radius*cos(j*2*PI/3));
            bigTriangle -> SetC(3+j,0,smallTriangle->C(j,0));
            bigTriangle -> SetC(3+j,1,-smallTriangle->C(j,1));

            for (TNode u=3;u<smallN;++u)
            {
                bigTriangle -> SetC(6+j*(smallN-3)+(u-3),0,smallTriangle->C(u,0)+radius*sin(j*2*PI/3));
                bigTriangle -> SetC(6+j*(smallN-3)+(u-3),1,smallTriangle->C(u,1)-radius*cos(j*2*PI/3));
            }

            for (TArc a=0;a<smallTriangle->M();++a)
            {
                TNode v[2],w[2];
                v[0] = smallTriangle->StartNode(2*a);
                v[1] = smallTriangle->EndNode(2*a);

                for (int k=0;k<2;k++)
                {
                    if (v[k]==j)
                    {
                        w[k] = j;
                    }
                    else if (v[k]<3)
                    {
                        w[k] = 3+(v[k]+j)%3;
                    }
                    else
                    {
                        w[k] = 6+j*(smallN-3)+v[k]-3;
                    }
                }

                bigTriangle -> InsertArc(w[0],w[1]);
            }
        }

        radius *= 2;

        delete smallTriangle;
        smallTriangle = bigTriangle;
    }

    IncidenceOrderFromDrawing();

    Layout_ConvertModel(LAYOUT_FREESTYLE_POLYGONES);
}


openGrid::openGrid(TNode _k,TNode _l,TOptGrid shape,goblinController &_CT) throw(ERRejected) :
    managedObject(_CT),
    graph(TNode(0),_CT)
{
    LogEntry(LOG_MAN,"Generating open grid...");

    Layout_ConvertModel(LAYOUT_FREESTYLE_POLYGONES);

    _k = (_k<2) ? 2 : _k;
    _l = (_l<1) ? 1 : _l;

    if (shape==GRID_HEXAGONAL)
    {
        _l |= 1;
        _k -= (_k&1);
    }

    for (TNode v=0;v<_k*_l;++v) InsertNode();

    if (shape==GRID_HEXAGONAL)
    {
        for (TNode k=0;k<_k;k++)
        {
            for (TNode l=0;l<_l;l++)
            {
                if (l<_l-1) InsertArc(k*_l+l,k*_l+(l+1));

                if ((k+l)%2==0 && k<_k-1)
                {
                    InsertArc(k*_l+l,(k+1)*_l+l);
                }

                X.SetC(k*_l+l,0,CT.nodeSep*cos(PI/6.0)*l);
                X.SetC(k*_l+l,1,CT.nodeSep*(k+(cos(PI/6.0)*0.5)*(k+(k+l+1)%2)));
            }
        }
    }
    else // (shape==GRID_SQUARE || shape==GRID_TRIANGULAR)
    {
        for (TNode k=0;k<_k;k++)
        {
            for (TNode l=0;l<_l;l++)
            {
                if (l<_l-1) InsertArc(k*_l+l,k*_l+(l+1));
                if (k<_k-1) InsertArc(k*_l+l,(k+1)*_l+l);

                if (shape==GRID_TRIANGULAR)
                {
                    if (l<_l-1 && k<_k-1)
                    {
                        InsertArc(k*_l+l,(k+1)*_l+(l+1));
                    }

                    X.SetC(k*_l+l,0,CT.nodeSep*(k+0.5*(_l-l)));
                    X.SetC(k*_l+l,1,CT.nodeSep*cos(PI/6.0)*l);
                }
                else
                {
                    X.SetC(k*_l+l,0,CT.nodeSep*k);
                    X.SetC(k*_l+l,1,CT.nodeSep*l);
                }
            }
        }
    }

    IncidenceOrderFromDrawing();
}


polarGrid::polarGrid(TNode _k,TNode _l,TNode _p,TOptPolar facets,TOptPolar dim,
        goblinController& _CT) throw(ERRejected) :
    managedObject(_CT),
    graph(TNode(0),_CT)
{
    LogEntry(LOG_MAN,"Generating spheric grid...");

    Layout_ConvertModel(LAYOUT_FREESTYLE_POLYGONES);

    _k = (_k<1) ? 1 : _k;
    _l = (_l<3) ? 3 : _l;
    _p = (_p>2) ? 2 : _p;

    for (TNode v=0;v<_k*_l+_p;++v) InsertNode();

    TFloat r = CT.nodeSep/PI*_l;
    TFloat R = r*_k;
    TArc m0 = 0;

    if (_p>0)
    {
        // Connect the "south" pole to the nodes in the first row
        for (TNode l=0;l<_l;l++) InsertArc(_k*_l,l);

        X.SetC(_k*_l,0,0);
        X.SetC(_k*_l,1,0);
        X.SetC(_k*_l,2,-R);

        for (TNode l=0;l<_l;l++) X.SetRight(2*l,2*((l+1)%_l));

        X.SetFirst(_k*_l,0);

        m0 = _l;
    }


    for (TNode k=0;k<_k;k++)
    {
        for (TNode l=0;l<_l;l++)
        {
            TFloat shift = (facets==POLAR_TRIANGULAR) ? 0.5*k : 0;
            TFloat thisAngle = 2*PI*(l-shift)/_l;

            if (dim==POLAR_CONE)
            {
                TFloat thisRadius = (k+1)*r;

                X.SetC(k*_l+l,0,-sin(thisAngle)*thisRadius);
                X.SetC(k*_l+l,1, cos(thisAngle)*thisRadius);
                X.SetC(k*_l+l,2,thisRadius);
            }
            else if (dim==POLAR_TUBE)
            {
                X.SetC(k*_l+l,0,-sin(thisAngle)*r);
                X.SetC(k*_l+l,1, cos(thisAngle)*r);
                X.SetC(k*_l+l,2,2*R*(k+1.0)/(_k+1)-R);
            }
            else if (dim==POLAR_HEMISPHERE)
            {
                TNode K =2*_k+1;
                TFloat thisRadius = sin(PI*(k+1)/K)*R;

                X.SetC(k*_l+l,0,-sin(thisAngle)*thisRadius);
                X.SetC(k*_l+l,1, cos(thisAngle)*thisRadius);
                X.SetC(k*_l+l,2,cos(PI*(k+1)/K)*R);
            }
            else
            {
                TFloat thisRadius = sin(PI*(k+1)/(_k+1))*R;

                X.SetC(k*_l+l,0,-sin(thisAngle)*thisRadius);
                X.SetC(k*_l+l,1, cos(thisAngle)*thisRadius);
                X.SetC(k*_l+l,2,cos(PI*(k+1)/(_k+1))*R);
            }

            // Connect this node to a node in a neighboring column
            InsertArc(k*_l+l,k*_l+((l+1)%_l));
        }

        for (TNode l=0;l<_l && k<_k-1;l++)
        {
            TArc a[6];
            int idx = 0;

            // Connect this node to a node in the subsequent row
            a[idx++] = 2*InsertArc(k*_l+l,((k+1)%_k)*_l+l);

            if (facets==POLAR_SQUARE)
            {
                a[idx++] = 2*(m0+2*k*_l+l);

                if (k>0 || _p>0)
                {
                    a[idx++] = 2*(m0+2*k*_l-_l+l)+1;
                }

                a[idx++] = 2*(m0+2*k*_l+(_l+l-1)%_l)+1;
            }
            else // (facets==POLAR_TRIANGULAR)
            {
                // Connect this node to another node in the subsequent row
                a[idx++] = 2*InsertArc(k*_l+l,((k+1)%_k)*_l+((l+1)%_l));

                a[idx++] = 2*(m0+3*k*_l+l);

                if (k>0)
                {
                    a[idx++] = 2*(m0+3*k*_l-2*_l+2*l)+1;
                    a[idx++] = 2*(m0+3*k*_l-2*_l+2*((_l+l-1)%_l)+1)+1;
                }
                else if (_p>0)
                {
                    a[idx++] = 2*l+1;
                }

                a[idx++] = 2*(m0+3*k*_l+(_l+l-1)%_l)+1;
            }

            for (int jdx=0;jdx<idx;++jdx)
            {
                X.SetRight(a[jdx],a[(jdx+1)%idx]);
            }

            X.SetFirst(k*_l+l,a[0]);
        }
    }


    if (_p>1)
    {
        // Connect the nodes in the final row to the "north" pole
        for (TNode l=0;l<_l;++l) InsertArc((_k-1)*_l+l,_k*_l+1);

        X.SetC(_k*_l+1,0,0);
        X.SetC(_k*_l+1,1,0);
        X.SetC(_k*_l+1,2,R);

        for (TNode l=_l;l>0;--l) X.SetRight(2*(m-_l+(_l+l-1)%_l)+1,2*(m-_l+(_l+l-2)%_l)+1);

        X.SetFirst(_k*_l+1,2*(m-_l)+1);
    }


    for (TNode l=0;l<_l;l++)
    {
        TArc a[5];
        int idx = 0;

        if (_p>1)
        {
            a[idx++] = 2*(m-_l+l);
        }

        if (facets==POLAR_SQUARE)
        {
            a[idx++] = 2*(m0+2*(_k-1)*_l+l);

            if (_k>1 || _p>0)
            {
                a[idx++] = 2*(m0+2*(_k-1)*_l-_l+l)+1;
            }

            a[idx++] = 2*(m0+2*(_k-1)*_l+(_l+l-1)%_l)+1;
        }
        else // (facets==POLAR_TRIANGULAR)
        {
            a[idx++] = 2*(m0+3*(_k-1)*_l+l);

            if (_k>1)
            {
                a[idx++] = 2*(m0+3*(_k-1)*_l-2*_l+2*l)+1;
                a[idx++] = 2*(m0+3*(_k-1)*_l-2*_l+2*((_l+l-1)%_l)+1)+1;
            }
            else if (_p>0)
            {
                a[idx++] = 2*l+1;
            }

            a[idx++] = 2*(m0+3*(_k-1)*_l+(_l+l-1)%_l)+1;
        }

        for (int jdx=0;jdx<idx;++jdx)
        {
            X.SetRight(a[jdx],a[(jdx+1)%idx]);
        }

        X.SetFirst((_k-1)*_l+l,a[0]);
    }
}


torusGrid::torusGrid(TNode _k,TNode _l,TOptTorus facets,TOptTorus dim,
    goblinController &_CT) throw(ERRejected) :
    managedObject(_CT),
    graph(TNode(0),_CT)
{
    LogEntry(LOG_MAN,"Generating torus grid...");

    Layout_ConvertModel(LAYOUT_FREESTYLE_POLYGONES);

    _k = (_k<1) ? 3 : _k;
    _l = (_l<1) ? 3 : _l;

    if (facets==TORUS_HEXAGONAL)
    {
        _l += (-_l)%4;
    }
    else if (facets==TORUS_TRIANGULAR)
    {
        _k += (_k%2);
    }

    for (TNode v=0;v<_k*_l;++v) InsertNode();

    TNode K = _k;

    if (dim==TORUS_HEMISPHERE)
    {
        // Place the nodes on a half-surface so that the xy-projection becomes readable
        K = 2*_k-2;
    }

    if (facets==TORUS_SQUARE)
    {
        TFloat r = CT.nodeSep/2.0/PI*_k;
        TFloat R = r*2;

        for (TNode k=0;k<_k;k++)
        {
            for (TNode l=0;l<_l;l++)
            {
                InsertArc(k*_l+l,k*_l+((l+1)%_l));
                InsertArc(k*_l+l,((k+1)%_k)*_l+l);

                TFloat thisRadius = R + cos(2*PI*k/K)*r;
                TFloat thisAngle = -2*PI*l/_l;

                X.SetC(k*_l+l,0,sin(thisAngle)*thisRadius);
                X.SetC(k*_l+l,1,cos(thisAngle)*thisRadius);
                X.SetC(k*_l+l,2,sin(2*PI*k/K)*r);
            }
        }

        for (TNode k=0;k<_k;k++)
        {
            for (TNode l=0;l<_l;l++)
            {
                TArc a1 = 4*(k*_l+l);
                TArc a2 = 4*(k*_l+l)+2;
                TArc a3 = 4*(k*_l+((l+_l-1)%_l))+1;
                TArc a4 = 4*(((k+_k-1)%_k)*_l+l)+3;

                X.SetRight(a1,a2);
                X.SetRight(a2,a3);
                X.SetRight(a3,a4);
                X.SetRight(a4,a1);
                X.SetFirst(k*_l+l,a1);
            }
        }
    }
    else if (facets==TORUS_TRIANGULAR)
    {
        TFloat r = CT.nodeSep/2.0/PI*_k;
        TFloat R = r*2;

        for (TNode k=0;k<_k;k++)
        {
            for (TNode l=0;l<_l;l++)
            {
                TNode lNext = l;

                if (k==_k-1) lNext = (_l+l-_k/2)%_l;

                InsertArc(k*_l+l,k*_l+((l+1)%_l));
                InsertArc(k*_l+l,((k+1)%_k)*_l+((lNext+1)%_l));
                InsertArc(k*_l+l,((k+1)%_k)*_l+lNext);

                TFloat thisRadius = R + cos(2*PI*k/K)*r;
                TFloat thisAngle = -2*PI*(2*l-k+_k)/(2*_l);

                X.SetC(k*_l+l,0,sin(thisAngle)*thisRadius);
                X.SetC(k*_l+l,1,cos(thisAngle)*thisRadius);
                X.SetC(k*_l+l,2,sin(2*PI*k/K)*r);
            }
        }

        for (TNode k=0;k<_k;k++)
        {
            for (TNode l=0;l<_l;l++)
            {
                TArc a1 = 6*(k*_l+l);
                TArc a2 = 6*(k*_l+l)+2;
                TArc a3 = 6*(k*_l+l)+4;
                TArc a4 = 6*(k*_l+((l+_l-1)%_l))+1;

                TNode lPrev = l;

                if (k==0) lPrev = (_l+l+_k/2)%_l;

                TArc a5 = 6*(((k+_k-1)%_k)*_l+((lPrev+_l-1)%_l))+3;
                TArc a6 = 6*(((k+_k-1)%_k)*_l+lPrev)+5;

                X.SetRight(a1,a2);
                X.SetRight(a2,a3);
                X.SetRight(a3,a4);
                X.SetRight(a4,a5);
                X.SetRight(a5,a6);
                X.SetRight(a6,a1);
                X.SetFirst(k*_l+l,a1);
            }
        }
    }
    else // (facets==TORUS_HEXAGONAL)
    {
        TFloat r = CT.nodeSep/2.0/PI*_k;
        TFloat R = r*2;

        for (TNode k=0;k<_k;k++)
        {
            for (TNode l=0;l<_l;l++)
            {
                InsertArc(k*_l+l,k*_l+((l+1)%_l));

                if ((l%4)==1)
                {
                    InsertArc(((k+1)%_k)*_l+((l-1)%_l),k*_l+l);
                }
                else if ((l%4)==2)
                {
                    InsertArc(k*_l+l,((k+1)%_k)*_l+((l+1)%_l));
                }

                TFloat thisRadius = R + cos(2*PI*(2*k+(((l+1)%4)>>1))/(2.0*K))*r;
                TFloat thisAngle = -2*PI*(6*(l/4)+(l%4)+((l%4)/2))/(1.5*_l);

                X.SetC(k*_l+l,0,sin(thisAngle)*thisRadius);
                X.SetC(k*_l+l,1,cos(thisAngle)*thisRadius);
                X.SetC(k*_l+l,2,sin(2*PI*(2*k+(((l+1)%4)>>1))/(2.0*K))*r);
            }
        }

        for (TNode k=0;k<_k;k++)
        {
            for (TNode l=0;l<_l;l++)
            {
                TArc a1 = First(k*_l+l);

                while (EndNode(a1)!=k*_l+((l+1)%_l)) a1 = Right(a1);

                TArc a2 = Right(a1);
                TArc a3 = Right(a2);

                if ((EndNode(a2)!=k*_l+((l+_l-1)%_l)) ^ (((l+1)%4)>>1))
                {
                    a3 = Right(a1);
                    a2 = Right(a3);
                }

                X.SetRight(a1,a2);
                X.SetRight(a2,a3);
                X.SetRight(a3,a1);
                X.SetFirst(k*_l+l,a1);
            }
        }
    }
}
