
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, November 2006
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file layoutOrthogonal.cpp
/// \brief Implementations of orthogonal layout methods

#include "sparseGraph.h"
#include "sparseDigraph.h"
#include "staticQueue.h"
#include "binaryHeap.h"
#include "incrementalGeometry.h"


void abstractMixedGraph::Layout_OrthogonalDeg4(TMethOrthogonal method,
    int bendSep) throw(ERRejected)
{
    #if defined(_FAILSAVE_)

    if (!IsSparse() || !Representation()) NoSparseRepresentation("Layout_OrthogonalDeg4");

    for (TNode u=0;u<n;u++)
    {
        TArc deg = 0;
        TArc a = First(u);

        while (a!=NoArc && (a!=First(u) || deg==0))
        {
            a = Right(a,u);
            deg++;
        }

        if (deg>4)
        {
            sprintf(CT.logBuffer,"Node %ld has degree %ld",u,deg);
            Error(ERR_REJECTED,"Layout_OrthogonalDeg4",CT.logBuffer);
        }
    }

    #endif


    LogEntry(LOG_METH,"Computing orthogonal layout...");

    OpenFold(Mod4Orthogonal,SHOW_TITLE);

    if (bendSep>0) CT.bendSep = bendSep;

    Layout_ConvertModel(LAYOUT_STRAIGHT_2DIM);
    Layout_ConvertModel(LAYOUT_ORTHO_SMALL);

    // Before starting to insert new layout nodes, memorize if there are some in advance
    TNode savedLayoutNodes = ni;

    bool planar = false;

    if (method==ORTHO_4PLANAR && ExtractEmbedding(PLANEXT_DEFAULT)!=NoNode)
    {
        planar = true;
    }

    TNode* nodeColour = GetNodeColours();

    if (planar)
    {
        // In the planar case, take care that the nodes on the outer face are
        // ordered correctly, namely that v_n, v_1, v_2 consecutively occur on
        // the outer face in counter clockwise order. This is achieved by
        // contracting exteriorArc.

        TArc retArc = ExteriorArc()^1;
        TArc delArc = First(EndNode(retArc));
        TNode delNode = EndNode(delArc);

        graph G(*this,OPT_CLONE);
        sparseRepresentation* GR = static_cast<sparseRepresentation*>(G.Representation());
        GR -> ContractArc(delArc);
        GR -> DeleteNode(delNode);

        if ((retArc>>1)==m-1) retArc = (delArc)^(delArc&1)^(retArc&1);

        if (!G.STNumbering(retArc^1))
        {
            Error(ERR_REJECTED,"Layout_Orthogonal4Planar",
                "Graph must be 2-connected");
        }

        for (TNode u=0;u<n-1;u++) nodeColour[u] = G.NodeColour(u)+1;

        if (delNode<n-1) nodeColour[n-1] = G.NodeColour(delNode)+1;

        nodeColour[StartNode(delArc)] = 0;
        nodeColour[EndNode(delArc)]   = 1;
    }
    else if (!STNumbering())
    {
        Error(ERR_REJECTED,"Layout_Orthogonal4Planar",
            "Graph must be 2-connected");
    }


    LogEntry(LOG_METH,"Place nodes...");

    incrementalGeometry Geo(*this,n+3*m+savedLayoutNodes+2);
    sparseRepresentation* X = static_cast<sparseRepresentation*>(Representation());

    goblinQueue<TNode,TFloat> *Q = NULL;

    if (nHeap!=NULL)
    {
        Q = nHeap;
        Q -> Init();
    }
    else Q = NewNodeHeap();

    for (TNode u=0;u<n;u++) Q->Insert(u,nodeColour[u]);


    // Embed the first two nodes v,w and the arc(s) joining them

    TNode v = Q->Delete();
    Geo.Init(v);

    TNode w = Q->Delete();
    Geo.ShareRowWith(v,w);
    Geo.InsertColumnRightOf(v,w);

    TArc a1 = First(v);

    while (EndNode(a1)!=w) a1 = Right(a1,v);

    while (EndNode(Right(a1,v))==w) a1 = Right(a1,v);

    char vDeg = 1;
    TArc a4 = a1;

    while (Right(a4,v)!=a1)
    {
        a4 = Right(a4,v);
        vDeg++;
    }

    TArc a2 = Right(a1,v);

    TArc b1 = a1^1;

    char wDeg = 1;
    TArc b4 = b1;

    while (Right(b4,v)!=b1)
    {
        b4 = Right(b4,w);
        wDeg++;
    }

    TArc b2 = Right(b1,w);

    if (vDeg==4 || wDeg==4)
    {
        TNode z = X->InsertArcLabelAnchor(a1);
        TNode x = X->InsertThreadSuccessor(z);
        TNode y = X->InsertThreadSuccessor(x);

        Geo.InsertRowBelowOf(v,x);
        Geo.ShareRowWith(x,y);

        if (a1&1)
        {
            Geo.ShareColumnWith(v,y);
            Geo.ShareColumnWith(w,x);
        }
        else
        {
            Geo.ShareColumnWith(v,x);
            Geo.ShareColumnWith(w,y);
        }

        Geo.ShareRowWith(x,z);
        Geo.ShareColumnWith(x,z);

        if (vDeg==4 && EndNode(a4)!=w)
        {
            z = X->InsertArcLabelAnchor(a4);
            x = X->InsertThreadSuccessor(z);

            Geo.ShareRowWith(v,x);
            Geo.InsertColumnRightOf(v,x);
            Geo.ShareRowWith(x,z);
            Geo.ShareColumnWith(x,z);
        }

        if (wDeg==4 && EndNode(b2)!=v)
        {
            z = X->InsertArcLabelAnchor(b2);
            x = X->InsertThreadSuccessor(z);

            Geo.ShareRowWith(w,x);
            Geo.InsertColumnLeftOf(w,x);
            Geo.ShareRowWith(x,z);
            Geo.ShareColumnWith(x,z);
        }
    }

    if (vDeg>=3)
    {
        TNode z = X->InsertArcLabelAnchor(a2);
        TNode x = X->InsertThreadSuccessor(z);

        Geo.ShareRowWith(v,x);
        Geo.InsertColumnLeftOf(v,x);
        Geo.ShareRowWith(x,z);
        Geo.ShareColumnWith(x,z);
    }
    else
    {
        // Degree of v is 2 due to the 2-connectivity
        // No bend nodes are required so far
    }

    if (wDeg>=3)
    {
        TNode z = X->InsertArcLabelAnchor(b4);
        TNode x = X->InsertThreadSuccessor(z);

        Geo.ShareRowWith(w,x);
        Geo.InsertColumnRightOf(w,x);
        Geo.ShareRowWith(x,z);
        Geo.ShareColumnWith(x,z);
    }
    else
    {
        // Degree of w is 2 due to the 2-connectivity
        // No bend nodes are required so far
    }


    #if defined(_LOGGING_)

    THandle LH = NoHandle;

    if (CT.logMeth>1)
    {
        sprintf(CT.logBuffer,"Embedded nodes: %ld %ld",v,w);
        LH = LogStart(LOG_METH2,CT.logBuffer);
    }

    #endif


    // Insert the other nodes one-by-one and complete
    // the edges joining with lower nodes

    for (TNode i=2;i<n;i++)
    {
        TNode u = Q->Delete();
        Geo.InsertRowAtopOf(v,u);

        // Determine the incoming arcs a1, a2, a3, ... from right to left

        TArc a2 = NoArc;
        TArc a3 = NoArc;
        TArc a4 = NoArc;

        if (planar)
        {
            // In the planar case, the arcs incident with u are preordered.
            // Just determine the right-most incoming arc

            a1 = First(u);

            if (i!=n-1)
            {
                while (nodeColour[EndNode(a1)]<=nodeColour[u]) a1 = Right(a1,u);
                while (nodeColour[EndNode(a1)]>nodeColour[u]) a1 = Right(a1,u);
            }
            else
            {
                // For the last node
                Geo.AssignNumbers();
                TArc minColumn = NoArc;
                TNode r = NoNode;

                do
                {
                    a1 = Right(a1,u);
                    TNode z = ArcLabelAnchor(a1);

                    if (z==NoNode) r = EndNode(a1);
                    else r = ThreadSuccessor(z);

                    if (Geo.ColumnNumber(r)<=minColumn)
                    {
                        minColumn = Geo.ColumnNumber(r);
                    }
                    else break;
                }
                while (true);
            }

            a2 = Right(a1,u);
            a3 = Right(a2,u);
            a4 = Right(a3,u);
        }
        else
        {
            // In the non-planar case

            TArc aInc[4];
            TArc colNo[4];
            TArc a = First(u);
            TArc deg = 0;

            for (TArc i=0;i<4;i++)
            {
                aInc[i] = NoArc;
                colNo[i] = NoArc;
            }

            do
            {
                TArc thisColNo = NoArc;

                if (nodeColour[EndNode(a)]<nodeColour[u])
                {
                    TNode z = ArcLabelAnchor(a);

                    if (z==NoNode)
                    {
                        thisColNo = NoArc-Geo.ColumnNumber(EndNode(a))-1;
                    }
                    else
                    {
                        thisColNo = NoArc-Geo.ColumnNumber(ThreadSuccessor(z))-1;
                    }
                }

                deg++;

                TArc aSwap = a;

                for (TArc i=0;i<deg;i++)
                {
                    if (thisColNo<colNo[i] || aInc[i]==NoArc)
                    {
                        TArc savedColNo = colNo[i];
                        TArc savedArc = aInc[i];
                        colNo[i] = thisColNo;
                        aInc[i] = aSwap;
                        aSwap = savedArc;
                        thisColNo = savedColNo;
                    }
                }

                a = Right(a,u);
            }
            while (a!=First(u));

            a1 = aInc[0];
            a2 = aInc[1];
            a3 = aInc[2];
            a4 = aInc[3];
        }


        // Determine the indegree and the outdegree of u

        char inDeg = 0;
        char outDeg = 0;
        TArc a = a1;

        while (a!=a1 || inDeg==0)
        {
            a = Right(a,u);

            if (nodeColour[EndNode(a)]<=nodeColour[u])
            {
                inDeg++;
            }
            else
            {
                outDeg++;
            }
        }


        // Place the node u in a column used by an incoming arc
        a = a1;

        if (inDeg>2) a = a2;

        TNode r = EndNode(a);

        if (ArcLabelAnchor(a)!=NoNode) r = ThreadSuccessor(ArcLabelAnchor(a));

        Geo.ShareColumnWith(r,u);

        if (inDeg>2)
        {
            // Complete the embedding of a1

            TNode z = ArcLabelAnchor(a1);
            TNode x = NoNode;

            if (z==NoNode)
            {
                z = X->InsertArcLabelAnchor(a1);
                x = X->InsertThreadSuccessor(z);
                r = EndNode(a1);

                Geo.ShareRowWith(u,z);
                Geo.ShareColumnWith(r,z);
            }
            else
            {
                if (a1&1)
                {
                    r = ThreadSuccessor(z);
                    x = X->InsertThreadSuccessor(r);
                }
                else
                {
                    r = ThreadSuccessor(z);
                    x = X->InsertThreadSuccessor(z);
                }
            }

            Geo.ShareRowWith(u,x);
            Geo.ShareColumnWith(r,x);
        }

        if (inDeg>1)
        {
            a = a2;

            if (inDeg>2) a = a3;

            // Complete the embedding of a

            TNode z = ArcLabelAnchor(a);
            TNode x = NoNode;

            if (z==NoNode)
            {
                z = X->InsertArcLabelAnchor(a);
                x = X->InsertThreadSuccessor(z);
                r = EndNode(a);

                Geo.ShareRowWith(u,z);
                Geo.ShareColumnWith(r,z);
            }
            else
            {
                if (a&1)
                {
                    r = ThreadSuccessor(z);
                    x = X->InsertThreadSuccessor(r);
                }
                else
                {
                    r = ThreadSuccessor(z);
                    x = X->InsertThreadSuccessor(z);
                }
            }

            Geo.ShareRowWith(u,x);
            Geo.ShareColumnWith(r,x);
        }

        if (inDeg==4)
        {
            // Complete the embedding of a4

            TNode z = ArcLabelAnchor(a4);
            TNode x = NoNode;
            TNode y = NoNode;

            if (z==NoNode)
            {
                z = X->InsertArcLabelAnchor(a4);
                r = EndNode(a4);

                if (a4&1)
                {
                    x = X->InsertThreadSuccessor(z);
                    y = X->InsertThreadSuccessor(x);
                }
                else
                {
                    y = X->InsertThreadSuccessor(z);
                    x = X->InsertThreadSuccessor(y);
                }

                Geo.ShareRowWith(u,z);
                Geo.ShareColumnWith(r,z);
            }
            else
            {
                if (a4&1)
                {
                    r = ThreadSuccessor(z);
                    x = X->InsertThreadSuccessor(r);
                    y = X->InsertThreadSuccessor(x);
                }
                else
                {
                    r = ThreadSuccessor(z);
                    y = X->InsertThreadSuccessor(z);
                    x = X->InsertThreadSuccessor(y);
                }
            }

            Geo.InsertRowAtopOf(u,y);
            Geo.ShareColumnWith(u,y);
            Geo.ShareRowWith(y,x);
            Geo.ShareColumnWith(r,x);
        }

        if (outDeg>1)
        {
            a = a4;

            if (outDeg+inDeg==3) a = a3;

            TNode z = X->InsertArcLabelAnchor(a);
            TNode x = X->InsertThreadSuccessor(z);

            Geo.ShareRowWith(u,x);
            Geo.InsertColumnRightOf(u,x);
            Geo.ShareRowWith(x,z);
            Geo.ShareColumnWith(x,z);
        }

        if (outDeg==3)
        {
            TNode z = X->InsertArcLabelAnchor(a2);
            TNode x = X->InsertThreadSuccessor(z);

            Geo.ShareRowWith(u,x);
            Geo.InsertColumnLeftOf(u,x);
            Geo.ShareRowWith(x,z);
            Geo.ShareColumnWith(x,z);
        }

        #if defined(_LOGGING_)

        if (CT.logMeth>1)
        {
            sprintf(CT.logBuffer," %ld",u);
            LogAppend(LH,CT.logBuffer);
        }

        #endif

        v = u;
    }

    #if defined(_LOGGING_)

    if (CT.logMeth>1) LogEnd(LH);

    #endif

    if (nHeap==NULL) delete Q;

    Geo.AssignNumbers();

    for (TNode i=0;i<n+ni;i++)
    {
        // Skip layout nodes which have not been generated by this method
        if (i>=n && i<n+savedLayoutNodes) continue;

        X -> SetC(i,0,Geo.ColumnNumber(i)*CT.nodeSep);
        X -> SetC(i,1,Geo.RowNumber(i)*CT.nodeSep);
    }

    if (   CT.methLocal==LOCAL_OPTIMIZE
        && method==ORTHO_4PLANAR)
    {
        Layout_OrthoSmallLineSweep();
    }

    CloseFold(Mod4Orthogonal);
}


bool abstractMixedGraph::Layout_OrthoSmallNodeCompaction()
    throw(ERRejected)
{
    #if defined(_FAILSAVE_)

    if (!IsSparse() || !Representation())
    {
        NoSparseRepresentation("Layout_OrthoSmallNodeCompaction");
    }

    #endif

    sparseRepresentation* X = static_cast<sparseRepresentation*>(Representation());

    LogEntry(LOG_METH,"Layout simplification...");
    OpenFold();

    binaryHeap<TNode,TFloat> Q(n+ni,CT);

    bool modified = false;

    do
    {
        modified = false;

        for (TNode v=0;v<n;v++)
        {
            TArc a[4];
            a[0] = First(v);

            if (a[0]==NoArc) continue;

            short deg = 1;
            for (;;++deg)
            {
                TArc ar = Right(a[deg-1],v);

                if (ar==a[0] || deg>4) break;

                a[deg] = ar;
            }

            if (deg>4) continue;

            TNode x[4][5]; // The sequence of bend nodes of the arcs a[0],a[1],a[2]
            TNode k[4];    // The number of bend nodes of the arcs a[0],a[1],a[2]
            TFloat d[4];   // The length of the first segment of a[0],a[1],a[2]

            // Determine x[], k[], d[] and the free side of node v. windRose[] assigns
            // the incident arcs to the four absolute directions in the coordinate system
            short windRose[4] = {-1,-1,-1,-1};

            for (short i=0;i<deg;i++)
            {
                k[i] = GetBendNodes(a[i],x[i],5);

                TFloat dx = floor((C(x[i][1],0) - C(v,0)) / CT.nodeSep + 0.5);
                TFloat dy = floor((C(x[i][1],1) - C(v,1)) / CT.nodeSep + 0.5);

                if (fabs(dx)>0)
                {
                    d[i] = dx;

                    if (dx<0) windRose[0] = i;
                    else windRose[2] = i;
                }
                else
                {
                    d[i] = dy;

                    if (dy<0) windRose[1] = i;
                    else windRose[3] = i;
                }
            }


            if (deg==1)
            {
                TNode w1 = EndNode(a[1]);
                TNode y1 = PortNode(a[1]^1);

                if (y1==NoNode) y1 = v;

                TFloat dx = C(y1,0) - C(w1,0);
                TFloat dy = C(y1,1) - C(w1,1);

                if (fabs(dx)>0)
                {
                    X -> SetC(v,0,C(w1,0)+dx/fabs(dx)*CT.nodeSep);
                    X -> SetC(v,1,C(w1,1));
                }
                else if (fabs(dy)>0)
                {
                    X -> SetC(v,0,C(w1,0));
                    X -> SetC(v,1,C(w1,1)+dy/fabs(dy)*CT.nodeSep);
                }

                #if defined(_LOGGING_)

                sprintf(CT.logBuffer,"...Stranded node %lu found",v);
                LogEntry(LOG_METH2,CT.logBuffer);

                #endif

                X -> ReleaseBendNodes(a[1]);

                continue;
            }

            if (deg==2)
            {
                // The first segements of a[0] and a[1] must be collinear
                if (C(x[0][1],0)!=C(x[1][1],0) && C(x[0][1],1)!=C(x[1][1],1)) continue;

                short bentArcIndex = -1;

                if (k[0]>2) bentArcIndex = 0;
                if (k[1]>2) bentArcIndex = 1;

                if (bentArcIndex<0) continue;

                // Shift v to an adjacent bend node w
                TNode w = x[bentArcIndex][1];
                X -> SetC(v,0,C(w,0));
                X -> SetC(v,1,C(w,1));
                X -> DeleteNode(w);

                #if defined(_LOGGING_)

                sprintf(CT.logBuffer,"...Node %lu moved to an adjacent bend node",v);
                LogEntry(LOG_METH2,CT.logBuffer);

                #endif

                Trace();

                continue;
            }
            else if (deg==3)
            {
                short freeDirection = 0;
                for (;windRose[freeDirection]!=-1;++freeDirection) {}

                short relIndex[3];
                for (short i=0;i<3;i++) relIndex[i] = windRose[(freeDirection+i+1)%4];

                enum TRelDirection {LEFT = 0,STRAIGHT = 1,RIGHT = 2};

                // Shifting v means to shift one or two adjacent edge segments.
                // And these segment may not sweep over other (layout) nodes.
                // So shifting v is safe only if these segments have grid length 1.
                // Shifting in the free direction is excluded for the same reasons.

                if (fabs(d[relIndex[STRAIGHT]])<1.5)
                {
                    // Shifting v in the opposed of the free direction is not possible

                    // If a[relIndex[STRAIGHT]] is unbent, one cannot shift
                    // v at all without introducing a new bend node
                    if (k[relIndex[STRAIGHT]]==2) continue;

                    // Try to shift v in the direction of the second segment of a[relIndex[STRAIGHT]]
                    char i = (freeDirection+1)%2;
                    TFloat d2 = floor((C(x[relIndex[STRAIGHT]][2],i) - C(x[relIndex[STRAIGHT]][1],i)) / CT.nodeSep + 0.5);

                    // Either a[relIndex[LEFT]] or a[relIndex[RIGHT]] points in the same direction
                    TRelDirection dir = RIGHT;
                    if (d2*d[relIndex[LEFT]]>0) dir = LEFT;

                    TFloat stepLength = fabs(d[relIndex[dir]]);
                    bool deleteForwardBendNode = true;
                    bool deleteMovedBendNode = false;

                    // If a[relIndex[dir]] has no bends, one cannot shift v the full stepLength
                    if (k[relIndex[dir]]==2)
                    {
                        stepLength -= 1;
                        deleteForwardBendNode = false;
                    }

                    if (fabs(d2)<stepLength-0.5)
                    {
                        stepLength = fabs(d2);
                        deleteForwardBendNode = false;
                        deleteMovedBendNode = true;
                    }
                    else if (fabs(d2)<stepLength+0.5)
                    {
                        deleteMovedBendNode = true;
                    }

                    if (stepLength>0.5)
                    {
                        // Move v
                        TFloat newPos = C(v,i)+CT.nodeSep*stepLength*d2/fabs(d2);
                        X -> SetC(v,i,newPos);

                        if (deleteMovedBendNode)
                        {
                            if (k[relIndex[STRAIGHT]]>3)
                            {
                                Q.Insert(x[relIndex[STRAIGHT]][2],-x[relIndex[STRAIGHT]][2]);
                            }

                            Q.Insert(x[relIndex[STRAIGHT]][1],-x[relIndex[STRAIGHT]][1]);
                        }
                        else
                        {
                            X -> SetC(x[relIndex[STRAIGHT]][1],i,newPos);
                        }

                        if (deleteForwardBendNode)
                        {
                            Q.Insert(x[relIndex[dir]][1],-x[relIndex[dir]][1]);
                        }

                        // Delete the bend nodes in the order of decreasing indices
                        while (!Q.Empty()) X -> DeleteNode(Q.Delete());

                        modified = true;

                        #if defined(_LOGGING_)

                        sprintf(CT.logBuffer,"...Node %lu shifted by %lu units",v,(unsigned long)(stepLength));
                        LogEntry(LOG_METH2,CT.logBuffer);

                        #endif

                        Trace();
                    }

                    continue;
                }

                if (   fabs(d[relIndex[LEFT]])<1.01  && k[relIndex[LEFT]]>2 
                    && fabs(d[relIndex[RIGHT]])<1.01 && k[relIndex[RIGHT]]>2 )
                {
                    // Try to shift v in the direction of the first segment of a[relIndex[STRAIGHT]]
                    TDim i = freeDirection%2;
                    TFloat d2l = floor((C(x[relIndex[LEFT]][2],i) - C(x[relIndex[LEFT]][1],i)) / CT.nodeSep + 0.5);
                    TFloat d2r = floor((C(x[relIndex[RIGHT]][2],i) - C(x[relIndex[RIGHT]][1],i)) / CT.nodeSep + 0.5);

                    // The second segments of a[relIndex[LEFT]] and a[relIndex[RIGHT]] must
                    // point in the same direction as the first segment of a[relIndex[STRAIGHT]]
                    if (d2l*d[relIndex[STRAIGHT]]<0 || d2l*d[relIndex[STRAIGHT]]<0) continue;

                    TFloat stepLength = fabs(d[relIndex[STRAIGHT]]);
                    bool deleteForwardBendNode = true;
                    bool deleteLeftBendNode = false;
                    bool deleteRightBendNode = false;

                    if (k[relIndex[STRAIGHT]]==2)
                    {
                        // a[relIndex[STRAIGHT]] has no bends, so one cannot shift v the full stepLength
                        stepLength -= 1;
                        deleteForwardBendNode = false;
                    }

                    if (fabs(d2l)<stepLength-0.5)
                    {
                        stepLength = fabs(d2l);
                        deleteForwardBendNode = false;
                        deleteLeftBendNode = true;
                    }
                    else if (fabs(d2l)<stepLength+0.5)
                    {
                        deleteLeftBendNode = true;
                    }

                    if (fabs(d2r)<stepLength-0.5)
                    {
                        stepLength = fabs(d2r);
                        deleteForwardBendNode = false;
                        deleteLeftBendNode = false;
                        deleteRightBendNode = true;
                    }
                    else if (fabs(d2r)<stepLength+0.5)
                    {
                        deleteRightBendNode = true;
                    }

                    if (stepLength>0.5)
                    {
                        // Node v can be moved
                        TFloat newPos = C(v,i)+CT.nodeSep*stepLength*d2l/fabs(d2l);
                        X -> SetC(v,i,newPos);

                        if (deleteForwardBendNode)
                        {
                            Q.Insert(x[relIndex[STRAIGHT]][1],-x[relIndex[STRAIGHT]][1]);
                        }

                        if (deleteLeftBendNode)
                        {
                            Q.Insert(x[relIndex[LEFT]][1],-x[relIndex[LEFT]][1]);

                            if (k[relIndex[LEFT]]>3)
                            {
                                Q.Insert(x[relIndex[LEFT]][2],-x[relIndex[LEFT]][2]);
                            }
                        }
                        else
                        {
                            X -> SetC(x[relIndex[LEFT]][1],i,newPos);
                        }

                        if (deleteRightBendNode)
                        {
                            Q.Insert(x[relIndex[RIGHT]][1],-x[relIndex[RIGHT]][1]);

                            if (k[relIndex[RIGHT]]>3)
                            {
                                Q.Insert(x[relIndex[RIGHT]][2],-x[relIndex[RIGHT]][2]);
                            }
                        }
                        else
                        {
                            X -> SetC(x[relIndex[RIGHT]][1],i,newPos);
                        }

                        // Delete the bend nodes in the order of decreasing indices
                        while (!Q.Empty()) X -> DeleteNode(Q.Delete());

                        modified = true;

                        #if defined(_LOGGING_)

                        sprintf(CT.logBuffer,"...Node %lu shifted by %lu units",v,(unsigned long)(stepLength));
                        LogEntry(LOG_METH2,CT.logBuffer);

                        #endif

                        Trace();
                    }

                    continue;
                }


                continue;
            }
            else // (deg==4)
            {
                short movingDirection = -1;
                TFloat stepLength = 0;

                bool deleteForwardBendNode = true;
                bool deleteLeftBendNode = false;
                bool deleteRightBendNode = false;

                // Compute the coordinate which varies in the first segment of a[0]
                TDim j = (fabs(C(x[0][1],0)-C(v,0))<0.5*CT.nodeSep) ? 1 : 0;
                TDim movingCoordinate = 0;

                for (short i=0;i<2 && stepLength<0.01;++i)
                {
                    // Both of a[i] and a[i+2] must have bends
                    if (k[i]==2 || k[i+2]==2) continue;

                    // The starting segmants of a[i] and a[i+2] must have unit length
                    if (fabs(d[i])>1.5 || fabs(d[i+2])>1.5) continue;

                    movingCoordinate = (j+i+1)%2;

                    TFloat d2l = floor((C(x[i][2],movingCoordinate) - C(x[i][1],movingCoordinate)) / CT.nodeSep + 0.5);
                    TFloat d2r = floor((C(x[i+2][2],movingCoordinate) - C(x[i+2][1],movingCoordinate)) / CT.nodeSep + 0.5);

                    // a[i] and a[i+2] must point into the same direction
                    if (d2l*d2r<0) continue;

                    if (d2l*d[i+1]>0)
                    {
                        movingDirection = i+1;
                    }
                    else
                    {
                        movingDirection = (i+3)%4;
                        TFloat swap = d2l;
                        d2l = d2r;
                        d2r = swap;
                    }

                    stepLength = fabs(d[movingDirection]);

                    if (k[movingDirection]==2)
                    {
                        stepLength -= 1;
                        deleteForwardBendNode = false;
                    }

                    if (fabs(d2l)<stepLength-0.5)
                    {
                        stepLength = fabs(d2l);
                        deleteForwardBendNode = false;
                        deleteLeftBendNode = true;
                    }
                    else if (fabs(d2l)<stepLength+0.5)
                    {
                        deleteLeftBendNode = true;
                    }

                    if (fabs(d2r)<stepLength-0.5)
                    {
                        stepLength = fabs(d2r);
                        deleteForwardBendNode = false;
                        deleteLeftBendNode = false;
                        deleteRightBendNode = true;
                    }
                    else if (fabs(d2r)<stepLength+0.5)
                    {
                        deleteRightBendNode = true;
                    }
                }

                if (stepLength>0.5)
                {
                    // Node v can be moved
                    TFloat newPos = C(v,movingCoordinate)
                                  + CT.nodeSep*stepLength*d[movingDirection]/fabs(d[movingDirection]);
                    X -> SetC(v,movingCoordinate,newPos);

                    if (deleteForwardBendNode)
                    {
                        Q.Insert(x[movingDirection][1],-x[movingDirection][1]);
                    }

                    short left = (movingDirection+3)%4;
                    if (deleteLeftBendNode)
                    {
                        Q.Insert(x[left][1],-x[left][1]);

                        if (k[left]>3)
                        {
                            Q.Insert(x[left][2],-x[left][2]);
                        }
                    }
                    else
                    {
                        X -> SetC(x[left][1],movingCoordinate,newPos);
                    }

                    short right = (movingDirection+1)%4;
                    if (deleteRightBendNode)
                    {
                        Q.Insert(x[right][1],-x[right][1]);

                        if (k[right]>3)
                        {
                            Q.Insert(x[right][2],-x[right][2]);
                        }
                    }
                    else
                    {
                        X -> SetC(x[right][1],movingCoordinate,newPos);
                    }

                    // Delete the bend nodes in the order of decreasing indices
                    while (!Q.Empty()) X -> DeleteNode(Q.Delete());

                    modified = true;

                    #if defined(_LOGGING_)

                    sprintf(CT.logBuffer,"...Node %lu shifted by %lu units",v,(unsigned long)(stepLength));
                    LogEntry(LOG_METH2,CT.logBuffer);

                    #endif

                    Trace();
                }

                continue;
            }
        }


        // Remove unused arc label positions and move the maintained ones
        // to the position of the respective first bend node
        X -> Layout_OrthoCleanupArcLabels();

        // Remove unused grid lines
        for (TDim j=0;j<2;++j)
        {
            for (TNode v=0;v<n+ni;v++) Q.Insert(v,C(v,j));

            TFloat gridLine = 0;
            TFloat preceedingValue = 0;

            for (TNode i=0;i<n+ni;i++)
            {
                TNode u = Q.Delete();

                if (i>0 && C(u,j)-preceedingValue>CT.nodeSep/2) gridLine++;

                preceedingValue = C(u,j);
                X -> SetC(u,j,gridLine*CT.nodeSep);
            }
        }
    }
    while (modified);

    CloseFold();

    TFloat minX =  InfFloat;
    TFloat maxX = -InfFloat;
    TFloat minY =  InfFloat;
    TFloat maxY = -InfFloat;

    for (TNode v=0;v<n+ni;v++)
    {
        TFloat thisX = C(v,0);
        TFloat thisY = C(v,1);

        if (thisX>maxX)
        {
            maxX = thisX;
        }
        else if (thisX<minX)
        {
            minX = thisX;
        }

        if (thisY>maxY)
        {
            maxY = thisY;
        }
        else if (thisY<minY)
        {
            minY = thisY;
        }
    }

    if (CT.logRes)
    {
        sprintf(CT.logBuffer,"...Final layout grid size is (%lu,%lu)",
            (unsigned long)((maxX-minX)/CT.nodeSep),(unsigned long)((maxY-minY)/CT.nodeSep));
        LogEntry(LOG_RES,CT.logBuffer);
    }

    return false;
}


void sparseRepresentation::Layout_OrthoCleanupArcLabels()
    throw()
{
    for (TArc a=0;a<mAct;++a)
    {
        TNode x = ArcLabelAnchor(2*a);

        if (x==NoNode) continue;

        TNode y = ThreadSuccessor(x);

        if (y==NoNode)
        {
            DeleteNode(x);
        }
        else
        {
            for (TDim j=0;j<2;++j) SetC(x,j,C(y,j)+CT.bendSep);
        }
    }
}


bool abstractMixedGraph::Layout_OrthoSmallLineSweep()
    throw(ERRejected)
{
    #if defined(_FAILSAVE_)

    if (!IsSparse() || !Representation())
        NoSparseRepresentation("Layout_OrthoSmallLineSweep");

    #endif

    sparseRepresentation* X = static_cast<sparseRepresentation*>(Representation());

    LogEntry(LOG_METH,"Reducing layout grid size...");
    OpenFold();

    binaryHeap<TNode,TFloat> Q(n+ni,CT);

    // A buffer for storing the nodes of two consecutive grid lines
    TNode* line[2];
    line[0] = new TNode[n+ni];
    line[1] = new TNode[n+ni];

    unsigned short firstLine = 0;
    short lapsToGo = 2;
    bool improvedAtLeastOnce = false;
    TNode maxGridLine[2] = {0,0};

    TArc* arcRef = new TArc[ni];
    TNode* prevBend = new TNode[ni];

    for (TDim i=0;lapsToGo>0 && CT.SolverRunning();i^=1)
    {
        --lapsToGo;

        if (i==0) LogEntry(LOG_METH,"Sweeping horizontally for line merges...");
        else      LogEntry(LOG_METH,"Sweeping vertically for line merges...");

        OpenFold();

        TNode targetLine = 0;
        TNode sourceLine = 0;
        TNode fixedNodes = 0;
        bool improvedThisTime = false;
        TNode nb = 0;

        for (TNode x=0;x<ni;++x)
        {
            arcRef[x] = NoArc;
            prevBend[x] = NoNode;
        }

        for (TArc a=0;a<m;++a)
        {
            TNode x = ArcLabelAnchor(2*a);

            if (x==NoNode) continue;

            x = ThreadSuccessor(x);
            TNode y = NoNode;

            while (x!=NoNode)
            {
                arcRef[x-n] = 2*a;
                prevBend[x-n] = y;
                y = x;
                x = ThreadSuccessor(x);
                nb++;
            }
        }

        while (fixedNodes<n+nb)
        {
            #if defined(_LOGGING_)

            THandle LH = NoHandle;

            #endif

            fixedNodes = 0;

            for (TNode v=0;v<n+ni;v++)
            {
                if (v>=n && arcRef[v-n]==NoArc) continue;

                TNode thisGridLine = TNode(C(v,i)/CT.nodeSep);

                if (thisGridLine<sourceLine)
                {
                    ++fixedNodes;
                }
                else if (thisGridLine<sourceLine+1)
                {
                    Q.Insert(v,C(v,i^1));

                    #if defined(_LOGGING_)

                    if (CT.logMeth>1)
                    {
                        if (LH==NoHandle)
                        {
                            LH = LogStart(LOG_METH2,"Nodes in line: ");
                        }

                        sprintf(CT.logBuffer,"%ld ",v);
                        LogAppend(LH,CT.logBuffer);
                    }

                    #endif
                }
            }

            #if defined(_LOGGING_)

            if (CT.logMeth>1 && LH!=NoHandle) LogEnd(LH);

            #endif

            if (sourceLine==targetLine)
            {
                for (TNode k=0;;k++)
                {
                    if (Q.Empty())
                    {
                        line[firstLine][k] = NoNode;
                        break;
                    }

                    line[firstLine][k] = Q.Delete();
                }

                ++sourceLine;
                continue;
            }

            for (TNode k=0;;k++)
            {
                if (Q.Empty())
                {
                    line[firstLine^1][k] = NoNode;
                    break;
                }

                line[firstLine^1][k] = Q.Delete();
            }

            if (line[firstLine^1][0]==NoNode)
            {
                ++sourceLine;
                continue;
            }

            if (Layout_OrthoShiftChain(i,line,firstLine,arcRef,prevBend,true))
            {
                // This results in a rescan of the target line:
                sourceLine = targetLine;
                improvedThisTime = true;

                #if defined(_LOGGING_)

                LogEntry(LOG_METH2,"...Lines have been merged");

                #endif
            }
            else if (Layout_OrthoShiftChain(i,line,firstLine,arcRef,prevBend,false))
            {
                // This results in a rescan of the target line:
                sourceLine = targetLine;
                improvedThisTime = true;

                #if defined(_LOGGING_)

                LogEntry(LOG_METH2,"...A chain of collinear edge segments has been shifted");

                #endif
            }
            else if (!improvedThisTime && Layout_OrthoShiftChord(i,line,firstLine,arcRef,prevBend))
            {
                improvedThisTime = true;

                // Abort this sweeping iteration since predBendNode[] and arcRef[] are corrupted
                fixedNodes = NoNode;

                #if defined(_LOGGING_)

                LogEntry(LOG_METH2,"...An edge segment has been shifted");

                #endif
            }
            else
            {
                if (sourceLine>targetLine+1)
                {
                    for (TNode k=0;line[firstLine^1][k]!=NoNode;k++)
                    {
                        X -> SetC(line[firstLine^1][k],i,(targetLine+1)*CT.nodeSep);
                    }

                    improvedThisTime = true;

                    #if defined(_LOGGING_)

                    LogEntry(LOG_METH2,"...Line has been moved");

                    #endif
                }

                // Rotate the line buffers and skip the current target line
                firstLine ^= 1;
                ++targetLine;
                ++sourceLine;
            }
        }

        CloseFold();

        maxGridLine[i] = TNode(C(line[firstLine][0],i)/CT.nodeSep+0.5);
        improvedAtLeastOnce |= improvedThisTime;

        if (improvedThisTime)
        {
            lapsToGo = 2;

            // Remove unused arc label positions and move the maintained ones
            // to the position of the respective first bend node
            X -> ReleaseCoveredBendNodes(canvasBuilder::PORTS_IMPLICIT);
            X -> Layout_OrthoCleanupArcLabels();
        }
    }

    delete[] line[0];
    delete[] line[1];
    delete[] arcRef;
    delete[] prevBend;

    CloseFold();

    if (CT.logRes)
    {
        sprintf(CT.logBuffer,"...Final grid size is (%lu,%lu)",maxGridLine[0],maxGridLine[1]);
        LogEntry(LOG_RES,CT.logBuffer);
    }

    return improvedAtLeastOnce;
}


enum TWindRose {PAR_PLUS = 0,PAR_MINUS = 1,PERP_PLUS = 2,PERP_MINUS = 3};

void abstractMixedGraph::Layout_OrthoGetWindrose(
    TDim i,TNode v,TNode windrose[4],TArc* arcRef,TNode* prevBend) throw()
{
    TNode neighbor[4] = {NoNode,NoNode,NoNode,NoNode};

    if (v<n)
    {
        TArc a = First(v);

        for (short j=0;j<4;++j)
        {
            if (a==NoArc) break;

            neighbor[j] = PortNode(a);

            if (neighbor[j]==NoNode)
            {
                neighbor[j] = EndNode(a);
            }

            a = Right(a,v);

            if (a==First(v)) break;
        }
    }
    else if (v!=NoNode)
    {
        // If v is a bend node, two (bend) nodes have to be considered

        TArc incidentArc = arcRef[v-n];
        neighbor[0] = ThreadSuccessor(v);
        neighbor[1] = prevBend[v-n];

        if (neighbor[0]==NoNode) neighbor[0] = EndNode(incidentArc);
        if (neighbor[1]==NoNode) neighbor[1] = StartNode(incidentArc);
    }

    for (short j=0;j<4;++j)
    {
        if (neighbor[j]==NoNode) break;

        if (fabs(C(neighbor[j],i)-C(v,i))<0.5)
        {
            // The neighbour is also in line[k][]
            if (C(neighbor[j],i^1)<C(v,i^1))
            {
                windrose[PAR_MINUS] = neighbor[j];
            }
            else
            {
                windrose[PAR_PLUS] = neighbor[j];
            }
        }
        else
        {
            if (C(neighbor[j],i)<C(v,i))
            {
                windrose[PERP_MINUS] = neighbor[j];
            }
            else
            {
                windrose[PERP_PLUS] = neighbor[j];
            }
        }
    }
}


bool abstractMixedGraph::Layout_OrthoShiftChord(
    TDim i,TNode** line,TNode firstLine,TArc* arcRef,TNode* prevBend)
    throw()
{
    sparseRepresentation* X = static_cast<sparseRepresentation*>(Representation());

    TNode indexInLine[2] = {0,0};
    TNode lineAhead = firstLine;

    if (C(line[lineAhead^1][0],i^1)>C(line[lineAhead][0],i^1))
    {
        lineAhead = firstLine^1;
    }

    TFloat linePos[2];
    linePos[0] = C(line[0][0],i);
    linePos[1] = C(line[1][0],i);


    while (line[lineAhead][indexInLine[lineAhead]]!=NoNode)
    {
        TNode v = line[lineAhead]  [indexInLine[lineAhead]];
        TNode w = line[lineAhead^1][indexInLine[lineAhead^1]];

        TNode x[2];
        x[lineAhead]   = v;
        x[lineAhead^1] = w;

        TNode windRose[2][4] = {{NoNode,NoNode,NoNode,NoNode},{NoNode,NoNode,NoNode,NoNode}};

        for (short k=0;k<2;++k)
        {
            if (x[k]!=NoNode)
            {
                Layout_OrthoGetWindrose(i,x[k],windRose[k],arcRef,prevBend);
            }
        }

        // Inspect the ordered pair (vr,vb). Verify if this pair forms an edge
        // segment and -if possible - shift it in the direction of the two grid
        // lines.
        TNode vr    = (v>n) ? w           : v;
        TNode vb    = (v>n) ? v           : w;
        TNode rLine = (v>n) ? lineAhead^1 : lineAhead;

        TWindRose segmentDir = (windRose[rLine][PERP_PLUS]==vb) ? PERP_PLUS : PERP_MINUS;

        // Allow shifting if vr is a degree 3 graph node and if vb is a bend
        // node. The equivalent configurations with two graph nodes and with
        // two bend nodes are handled in Layout_OrthoShiftChain()
        if (   fabs(C(vr,i^1)-C(vb,i^1))<0.5
            && vr<n && vb>=n
            && windRose[rLine][segmentDir]==vb
            && windRose[rLine^1][segmentDir]==NoNode
            && (   windRose[rLine][segmentDir^1]==NoNode
                || windRose[rLine][PAR_PLUS]==NoNode
                || windRose[rLine][PAR_MINUS]==NoNode
               )
           )
        {
            TWindRose shiftingDir = (windRose[rLine^1][PAR_PLUS]==NoNode) ? PAR_MINUS : PAR_PLUS;
            short sign = (shiftingDir==PAR_PLUS) ? +1 : -1;

            TNode pr = windRose[rLine][shiftingDir];
            TNode pb = windRose[rLine^1][shiftingDir];

            if (pr!=NoNode)
            {
                TNode restrictingNode = NoNode;

                if (   sign*(C(pb,i^1)-C(pr,i^1))<-0.5
                    && sign*(C(vb,i^1)-C(pb,i^1))<-0.5
                   )
                {
                    // Shifting the edge segment makes vb redundant

                    // The following addresses a further restriction in the non-planar case
                    TNode pWindRose[4] = {NoNode,NoNode,NoNode,NoNode};
                    Layout_OrthoGetWindrose(i,pb,pWindRose,arcRef,prevBend);

                    if (pWindRose[segmentDir^1]==NoNode) restrictingNode=pb;
                }
                else if (   pr>=n
                         && sign*(C(pr,i^1)-C(pb,i^1))<+0.5
                         && sign*(C(vb,i^1)-C(pr,i^1))<-0.5
                        )
                {
                    // Shifting the edge segment makes pr redundant

                    // The following addresses a further restriction in the non-planar case
                    TNode pWindRose[4] = {NoNode,NoNode,NoNode,NoNode};
                    Layout_OrthoGetWindrose(i,pr,pWindRose,arcRef,prevBend);

                    if (pWindRose[segmentDir]==NoNode) restrictingNode=pr;
                }

                if (restrictingNode!=NoNode)
                {
                    // The edge segment can be shifted

                    TNode wr = windRose[rLine][segmentDir^1];

                    if (wr!=NoNode)
                    {
                        // Shifting the segment requires to add a bend node at the
                        // former position of vr. So the only immediate benefit is
                        // a decrease of the length of the edge of vb

                        // Recover the arc index of the line segment (vr,wr)
                        TArc a = NoArc;

                        if (wr>=n)
                        {
                            a = arcRef[wr-n];
                            if (vr==EndNode(a)) a ^= 1;
                        }
                        else
                        {
                            a = First(vr);

                            if (EndNode(a)!=wr)
                            {
                                a = Right(a,vr);

                                if (EndNode(a)!=wr) a = Right(a,vr);
                            }
                        }

                        TNode newBendNode = X->InsertBendNode(a,vr);
                        X -> SetC(newBendNode,0,C(vr,0));
                        X -> SetC(newBendNode,1,C(vr,1));
                    }

                    X -> SetC(vr,i^1,C(restrictingNode,i^1));
                    X -> SetC(vb,i^1,C(restrictingNode,i^1));
                    return true;
                }
                else if (   sign*(C(pr,i^1)-C(pb,i^1))<-0.5
                         && sign*(C(vb,i^1)-C(pr,i^1))<-0.5
                        )
                {
                    // The edge segment cannot be shifted. But sometimes, it is
                    // possible to place vr in the other grid line, saving a bend

                    TNode qr = windRose[rLine][shiftingDir^1];
                    TNode qb = NoNode;
                    if (   (sign>0 && indexInLine[rLine^1]>0)
                        || (sign<0 && line[rLine^1][indexInLine[rLine^1]]!=NoNode)
                       )
                    {
                        qb = line[rLine^1][indexInLine[rLine^1]-sign];
                    }

                    TNode qWindRose[4] = {NoNode,NoNode,NoNode,NoNode};
                    Layout_OrthoGetWindrose(i,qr,qWindRose,arcRef,prevBend);

                    TNode pWindRose[4] = {NoNode,NoNode,NoNode,NoNode};
                    Layout_OrthoGetWindrose(i,pr,pWindRose,arcRef,prevBend);

                    if (pWindRose[segmentDir]==NoNode) restrictingNode = pr;

                    if (   qr!=NoNode
                        && (qb==NoNode || sign*(C(qb,i^1)-C(qr,i^1))<-0.5)
                        && qWindRose[shiftingDir^1]==NoNode
                        && qWindRose[segmentDir^1]!=NoNode
                        && pWindRose[segmentDir]==NoNode
                       )
                    {
                        X -> SetC(vr,i^1,C(pr,i^1));
                        X -> SetC(vb,i^1,C(pr,i^1));
                        X -> SetC(vr,i,C(vb,i));
                        X -> SetC(qr,i,C(vb,i));
                        return true;
                    }
                }
            }
        }

        ++indexInLine[lineAhead^1];
        w = line[lineAhead^1][indexInLine[lineAhead^1]];

        if (w==NoNode || (v!=NoNode && C(w,i^1)>C(v,i^1)))
        {
            lineAhead ^= 1;
        }
    }

    return false;
}


bool abstractMixedGraph::Layout_OrthoShiftChain(
    TDim i,TNode** line,TNode firstLine,TArc* arcRef,TNode* prevBend,bool mergeWholeLine)
    throw()
{
    sparseRepresentation* X = static_cast<sparseRepresentation*>(Representation());

    bool wholeLineMergable = true;

    TNode indexInLine[2] = {0,0};
    TNode lineAhead = firstLine;

    if (C(line[lineAhead^1][0],i^1)>C(line[lineAhead][0],i^1))
    {
        lineAhead = firstLine^1;
    }

    TFloat linePos[2];
    linePos[0] = C(line[0][0],i);
    linePos[1] = C(line[1][0],i);


    // In what follows, chains of line segments in the two grid lines are
    // considered. It is checked if one of these chains can be shifted to
    // the other grid line. For the latter purpose, the index of the first
    // chain (bend) node in the line[] buffer is maintained.
    TNode firstOfChain[2] = {0,0};

    // This tells how the total edge length changes if the current chain
    // is shifted to the other grid line.
    int profit[2] = {0,0};

    while (line[lineAhead^1][indexInLine[lineAhead^1]]!=NoNode)
    {
        TNode v = line[lineAhead]  [indexInLine[lineAhead]];
        TNode w = line[lineAhead^1][indexInLine[lineAhead^1]];

        TNode x[2];
        x[lineAhead]   = v;
        x[lineAhead^1] = w;

        TNode windRose[2][4] = {{NoNode,NoNode,NoNode,NoNode},{NoNode,NoNode,NoNode,NoNode}};

        for (short k=0;k<2;++k)
        {
            if (x[k]!=NoNode) Layout_OrthoGetWindrose(i,x[k],&(windRose[k][0]),arcRef,prevBend);
        }

        TFloat maxPos = C(w,i^1);
        if (windRose[lineAhead^1][PAR_PLUS]!=NoNode) maxPos = C(windRose[lineAhead^1][PAR_PLUS],i^1);

        TFloat minPos = InfFloat;
        if (v!=NoNode)
        {
            minPos = C(v,i^1);
            if (windRose[lineAhead][PAR_MINUS]!=NoNode) minPos = C(windRose[lineAhead][PAR_MINUS],i^1);
        }

        bool mergable = true;

        if (v==NoNode)
        {
            TNode u = line[lineAhead][indexInLine[lineAhead]-1];

            if (u<n && w<n && C(w,i^1)-C(u,i^1)<0.5)
            {
                // Shifting the chain would result in coincident graph nodes
                mergable = false;
            }
        }
        else if (C(v,i^1)>C(w,i^1)+0.5 && maxPos>minPos+0.5)
        {
            // Shifting the chain introduces overlapping edge segments
            mergable = false;
        }
        else if (maxPos>minPos-0.5)
        {
            TNode u = windRose[lineAhead^1][PAR_PLUS];
            TNode z = windRose[lineAhead][PAR_MINUS];

            if (C(v,i^1)<C(w,i^1)+0.5)
            {
                u = w;
                z = v;
            }
            else
            {
                if (u==NoNode) u = w;
                if (z==NoNode) z = v;
            }

            if (z<n && u<n)
            {
                // Shifting the chain would result in coincident graph nodes
                mergable = false;
            }
            else if (z<n && u>=n)
            {
                if (   (z==StartNode(arcRef[u-n]) && prevBend[u-n]!=NoNode)
                    || (z==EndNode(arcRef[u-n])   && ThreadSuccessor(u)!=NoNode)
                )
                {
                    // Shifting the chain would result in node-edge crossings
                    mergable = false;
                }
            }
            else if (z>=n && u<n)
            {
                if (   (u==StartNode(arcRef[z-n]) && prevBend[z-n]!=NoNode)
                    || (u==EndNode(arcRef[z-n])   && ThreadSuccessor(z)!=NoNode)
                )
                {
                    // Shifting the chain would result in node-edge crossings
                    mergable = false;
                }
            }
            else
            {
                // v and w are bend nodes

                if (prevBend[z-n]!=u && prevBend[u-n]!=z)
                {
                    // Shifting the chain would introduce new edge-edge crossings
                    mergable = false;
                }
            }
        }

        if (   indexInLine[lineAhead^1]>0
            && C(line[lineAhead^1][indexInLine[lineAhead^1]-1],i^1) > C(w,i^1)-0.5
           )
        {
            // This can only happen after another shifting operation which
            // has moved another node to the same position as w, and before
            // double bend nodes can be deleted
            mergable = false;
        }

        if (   indexInLine[lineAhead]>0
            && line[lineAhead][indexInLine[lineAhead]-1]<n && w<n
            && fabs(C(line[lineAhead][indexInLine[lineAhead]-1],i^1)-C(w,i^1))<0.5
           )
        {
            // This can only happen after another shifting operation which
            // has moved another node to the same position as w, and before
            // double bend nodes can be deleted
            mergable = false;
        }

        if (mergeWholeLine)
        {
            wholeLineMergable &= mergable;

            if (!wholeLineMergable) return false;
        }
        else if (!mergable)
        {
            // No way to shift the current chain (if there is one)
            firstOfChain[lineAhead] = NoNode;
            firstOfChain[lineAhead^1] = NoNode;
        }
        else
        {
            // Determine the shifting direction
            TWindRose shiftingDir = PERP_PLUS;
            TWindRose oppositeDir = PERP_MINUS;

            if (lineAhead==firstLine)
            {
                shiftingDir = PERP_MINUS;
                oppositeDir = PERP_PLUS;
            }

            if (windRose[lineAhead^1][PAR_MINUS]==NoNode)
            {
                // Start a new chain
                firstOfChain[lineAhead^1] = indexInLine[lineAhead^1];
                profit[lineAhead^1] = 0;
            }

            if (windRose[lineAhead^1][oppositeDir]!=NoNode) profit[lineAhead^1]--;

            TNode y = windRose[lineAhead^1][shiftingDir];

            if (y!=NoNode)
            {
                // If the chain was shifted, the length of this segment would decrease
                profit[lineAhead^1]++;

                if (fabs(C(y,i)-linePos[lineAhead])<0.5)
                {
                    // Shifting the chain can save the potential bend nodes y and w
                    if (w>=n) profit[lineAhead^1] += 2;
                    if (y>=n) profit[lineAhead^1] += 2;
                }
            }

            if (   firstOfChain[lineAhead^1]!=NoNode
                && windRose[lineAhead^1][PAR_PLUS]==NoNode
               )
            {
                // A chain has been completed and shifting is possible

                if (profit[lineAhead^1]>0)
                {
                    // Shift the current chain to the other grid line since this will
                    // reduce the total edge length and, possibly, the number of bend nodes
                    for (TNode k=firstOfChain[lineAhead^1];k<=indexInLine[lineAhead^1];k++)
                    {
                        X -> SetC(line[lineAhead^1][k],i,linePos[lineAhead]);
                    }

                    return true;
                }

                // Invalidate this chain
                firstOfChain[lineAhead^1] = NoNode;
            }
        }

        ++indexInLine[lineAhead^1];
        w = line[lineAhead^1][indexInLine[lineAhead^1]];

        if (w==NoNode || (v!=NoNode && C(w,i^1)>C(v,i^1)))
        {
            lineAhead ^= 1;
        }
    }

    if (mergeWholeLine)
    {
        for (TNode k=0;line[firstLine^1][k]!=NoNode;k++)
        {
            X -> SetC(line[firstLine^1][k],i,linePos[firstLine]);
        }

        return true;
    }

    return false;
}


void abstractMixedGraph::Layout_VisibilityRepresentation(TMethOrthogonal option,
    int bendSep) throw(ERRejected)
{
    #if defined(_FAILSAVE_)

    if (!IsSparse() || !Representation())
        NoSparseRepresentation("Layout_VisibilityRepresentation");

    if (MetricType()!=METRIC_DISABLED)
        Error(ERR_REJECTED,"Layout_VisibilityRepresentation", "Coordinates are fixed");

    for (TArc a=0;a<2*m;a++)
    {
        if (StartNode(a)==EndNode(a))
            Error(ERR_REJECTED,
                "Layout_VisibilityRepresentation","Graph contains loops");
    }

    #endif

    sparseRepresentation* X = static_cast<sparseRepresentation*>(Representation());

    graph G(*this,OPT_CLONE);

    // Connectivity augmentation
    G.PlanarConnectivityAugmentation();

    #if defined(_PROGRESS_)

    // ProgressStep(n);
    // Trace();

    #endif


    // Biconnectivity augmentation
    G.PlanarBiconnectivityAugmentation();

    #if defined(_PROGRESS_)

    // ProgressStep(n);
    // Trace();

    #endif

    Layout_ConvertModel(LAYOUT_STRAIGHT_2DIM);

    bool trimNodes = (option!=ORTHO_VISIBILITY_RAW);
    bool smallNodes = (option==ORTHO_VISIBILITY_GIOTTO);
    bool bigNodes = false;

    G.Layout_Visibility2Connected(bendSep,!smallNodes);

    for (TNode v=0;v<n;v++)
    {
        TFloat minX = InfFloat;
        TFloat minX2 = InfFloat;
        TFloat maxX = -InfFloat;
        TFloat maxX2 = -InfFloat;
        TArc thisDeg = 0;

        TArc a = First(v);

        while (trimNodes && First(v)!=NoArc && (a!=First(v) || minX==InfFloat))
        {
            TFloat thisX = G.C(G.ThreadSuccessor(G.ArcLabelAnchor(a)),0);

            if (thisX >= maxX)
            {
                maxX2 = maxX;
                maxX = thisX;
            }
            else if (thisX > maxX2)
            {
                maxX2 = thisX;
            }

            if (thisX <= minX)
            {
                minX2 = minX;
                minX = thisX;
            }
            else if (thisX < minX2)
            {
                minX2 = thisX;
            }

            a = Right(a,v);
            thisDeg++;
        }

        if (trimNodes && minX!=InfFloat)
        {
            if (smallNodes)
            {
                if (maxX2>minX2)
                {
                    X -> SetC(v,0,(maxX2+minX2)/2);
                }
                else if (minX2!=InfFloat)
                {
                    X -> SetC(v,0,minX2);
                }
                else
                {
                    X -> SetC(v,0,minX);
                }
            }
            else
            {
                X -> SetC(v,0,(maxX+minX)/2);
            }
        }
        else
        {
            X -> SetC(v,0,G.C(v,0));
        }

        X -> SetC(v,1,G.C(v,1));


        if (smallNodes)
        {
            if (maxX2>minX2)
            {
                TNode w = X->InsertThreadSuccessor(v);

                X -> SetC(w,0,(maxX2-minX2)/2);
                X -> SetC(w,1,0);

                bigNodes = true;
            }
        }
        else if (trimNodes && minX!=InfFloat)
        {
            if (minX<maxX)
            {
                TNode w = X->InsertThreadSuccessor(v);

                X -> SetC(w,0,(maxX-minX)/2);
                X -> SetC(w,1,0);
            }
        }
        else
        {
            TNode u = G.ThreadSuccessor(v);

            if (u!=NoNode)
            {
                TNode w = X->InsertThreadSuccessor(v);

                X -> SetC(w,0,G.C(u,0));
                X -> SetC(w,1,0);
            }
        }
    }

    // Extract the arc routing from G
    X -> Layout_AdoptArcRouting(G);

    // Eliminate all port nodes which coincide with the adjacent graph nodes
    X -> ReleaseDoubleBendNodes();

    if (!smallNodes)
    {
        Layout_ConvertModel(LAYOUT_VISIBILITY);
    }
    else if (bigNodes)
    {
        Layout_ConvertModel(LAYOUT_ORTHO_BIG);
    }
    else
    {
        Layout_ConvertModel(LAYOUT_ORTHO_SMALL);

        if (CT.methLocal==LOCAL_OPTIMIZE) Layout_OrthoSmallLineSweep();
    }
}


void abstractMixedGraph::Layout_Visibility2Connected(int bendSep,bool alignPorts)
    throw(ERRejected)
{
    LogEntry(LOG_METH,"Computing visibility representation...");

    OpenFold(ModVisibilityRepr,SHOW_TITLE);


    if (ExtractEmbedding(PLANEXT_DEFAULT)==NoNode)
    {
        Error(ERR_REJECTED,"Layout_Visibility2Connected","Graph must be planar");
    }

    TArc retArc = ExteriorArc()^1;
    TNode s = EndNode(retArc);
    TNode t = StartNode(retArc);

    if (!STNumbering(retArc^1))
    {
        Error(ERR_REJECTED,"Layout_Visibility2Connected","Graph must be 2-connected");
    }

    inducedOrientation GX(*this,OPT_CLONE);
    GX.Representation() -> SetCLength(1);
    LogEntry(LOG_METH,"Vertical layering...");
    GX.DAGSearch(GX.DAG_CRITICAL);

    TNode* nodeColour = GetNodeColours();

    for (TNode v=0;v<n;v++) GX.SetNodeColour(v,nodeColour[v]);

    directedDual GY(GX);

    GY.Representation() -> SetCLength(1);
    LogEntry(LOG_METH,"Horizontal layering...");
    GY.DAGSearch(GY.DAG_CRITICAL);


    LogEntry(LOG_METH,"Place nodes...");
    OpenFold();

    if (CT.logMeth>1)
    {
        sprintf(CT.logBuffer,"Source node %ld on the bottom line",s);
        LogEntry(LOG_METH2,CT.logBuffer);
        sprintf(CT.logBuffer,"Target node %ld on the top line",t);
        LogEntry(LOG_METH2,CT.logBuffer);
    }

    if (bendSep>0) CT.bendSep = bendSep;
    CT.nodeSep = CT.bendSep*4;
    CT.fineSep = CT.bendSep;
    CT.nodeSize = 500;

    sparseRepresentation* X = static_cast<sparseRepresentation*>(Representation());

    for (TNode v=0;v<n;v++)
    {
        X -> SetC(v,1,(GX.Dist(t)-GX.Dist(v))*CT.nodeSep);

        if (v==s || v==t)
        {
            TNode x = X->InsertThreadSuccessor(v);
            X -> SetC(x,1,0);
            X -> SetC(v,0,GY.Dist(Face(ExteriorArc()))/2.0*CT.nodeSep);
            X -> SetC(x,0,GY.Dist(Face(ExteriorArc()))/2.0*CT.nodeSep);

            continue;
        }

        TArc a = First(v);
        TNode leftMost = NoNode;
        TNode rightMost = NoNode;

        bool pointsUp = false;

        if (nodeColour[EndNode(a)]>nodeColour[v]) pointsUp = true;

        do
        {
            a = Right(a,v);

            if (pointsUp)
            {
                if (nodeColour[EndNode(a)]<nodeColour[v])
                {
                    pointsUp = false;
                    rightMost = Face(a);
                }
            }
            else
            {
                if (nodeColour[EndNode(a)]>nodeColour[v])
                {
                    pointsUp = true;
                    leftMost = Face(a);
                }
            }
        }
        while (leftMost==NoNode || rightMost==NoNode);

        X -> SetC(v,0,(GY.Dist(leftMost)+GY.Dist(rightMost)+1)/2.0*CT.nodeSep);
        TFloat thisWidth = GY.Dist(rightMost)-1-GY.Dist(leftMost);

        if (thisWidth>0)
        {
            TNode x = X->InsertThreadSuccessor(v);
            X -> SetC(x,0,thisWidth/2.0*CT.nodeSep);
            X -> SetC(x,1,0);
        }

        if (CT.logMeth>1)
        {
            sprintf(CT.logBuffer,"Node %ld from face %ld to %ld (width %ld)",
                v,leftMost,rightMost,TNode(GY.Dist(rightMost)-1-GY.Dist(leftMost)));
            LogEntry(LOG_METH2,CT.logBuffer);
        }
    }

    for (TArc a=0;a<m;a++)
    {
        TArc a0 = 2*a;
        TNode u = StartNode(a0);
        TNode v = EndNode(a0);

        TNode w = X->InsertArcLabelAnchor(2*a);
        TNode x = X->InsertThreadSuccessor(w);
        TNode y = X->InsertThreadSuccessor(x);

        TFloat sign = 0;

        if (alignPorts) sign = (C(v,1)>C(u,1)) ? 1 : -1;

        if (nodeColour[u]>nodeColour[v])
        {
            a0 = a0^1;
            v = StartNode(a0);
            u = EndNode(a0);
        }

        TFloat cx = (GY.Dist(Face(a0))+1)*CT.nodeSep;

        if (a==(ExteriorArc()>>1)) cx = 0;

        X -> SetC(w,0,cx+CT.fineSep);
        X -> SetC(x,0,cx);
        X -> SetC(y,0,cx);

        X -> SetC(w,1,(2*GX.Dist(t)-GX.Dist(u)-GX.Dist(v))*CT.nodeSep/2.0);
        X -> SetC(x,1,(GX.Dist(t)-GX.Dist(u))*CT.nodeSep + sign*CT.bendSep);
        X -> SetC(y,1,(GX.Dist(t)-GX.Dist(v))*CT.nodeSep - sign*CT.bendSep);
    }

    CloseFold();

    if (CT.logRes)
    {
        sprintf(CT.logBuffer,"...Layout size is %ld x %ld",
            TNode(GY.Dist(Face(ExteriorArc()))-1),TNode(GX.Dist(t)));
        LogEntry(LOG_RES,CT.logBuffer);
    }

    CloseFold(ModVisibilityRepr);
}


bool abstractMixedGraph::Layout_HorizontalVerticalTree(TNode root,int spacing)
    throw(ERRejected)
{
    #if defined(_FAILSAVE_)

    if (!IsSparse() || !Representation())
        NoSparseRepresentation("Layout_HorizontalVerticalTree");

    if (MetricType()!=METRIC_DISABLED)
        Error(ERR_REJECTED,"Layout_HorizontalVerticalTree", "Coordinates are fixed");

    for (TArc a=0;a<2*m;a++)
    {
        if (StartNode(a)==EndNode(a))
        {
            Error(ERR_REJECTED,
                "Layout_HorizontalVerticalTree","Graph contains loops");
        }
    }

    #endif

    double aspectRatio = 1.33;

    if (spacing>0) CT.bendSep = spacing;

    TArc* pred = GetPredecessors();
    bool fixedPredecessors = false;

    if (pred)
    {
        LogEntry(LOG_METH,"Starting with existing predecessor tree...");

        root = NoNode;

        for (TNode v=0;v<n;++v)
        {
            if (pred[v]!=NoArc)
            {
                fixedPredecessors = true;
                continue;
            }

            if (root!=NoNode)
            {
                Error(ERR_REJECTED,
                    "Layout_HorizontalVerticalTree","Multiple root nodes");
            }

            root = v;
        }

        if (root==NoNode)
        {
            Error(ERR_REJECTED,"Layout_HorizontalVerticalTree","Missing root node");
        }
    }
    else
    {
        pred = InitPredecessors();

        if (root==NoNode)
        {
            for (TNode v=0;v<n;++v)
            {
                TArc a = First(v);

                if (a==NoArc)
                {
                    Error(ERR_REJECTED,
                        "Layout_HorizontalVerticalTree","Graph contains isolated nodes");
                }

                if (Right(a,v)==a)
                {
                    root = v;
                }
                else if (Right(Right(a,v),v)==a)
                {
                    root = v;
                    break;
                }
            }

            if (root==NoNode)
            {
                Error(ERR_REJECTED,"Layout_HorizontalVerticalTree","Missing low degree node");
            }
        }
        else
        {
            TArc a0 = First(root);
            if (a0==NoArc || (Right(a0,root)!=a0 && Right(Right(a0,root),root)!=a0))
            {
                Error(ERR_REJECTED,"Layout_HorizontalVerticalTree","Root node must be degree 1 or 2");
            }
        }
    }

    if (First(root)==NoArc)
    {
        Error(ERR_REJECTED,"Layout_HorizontalVerticalTree","Root node is isolated");
    }


    LogEntry(LOG_METH,"Checking for tree...");
    OpenFold();

    #if defined(_LOGGING_)

    THandle LH = LogStart(LOG_METH2,"Expanded nodes: ");

    #endif

    TNode* leftChild = new TNode[n];
    TNode* rightChild = new TNode[n];
    TNode* ordered = new TNode[n];

    staticQueue<TNode,TFloat> Q(n,CT);
    Q.Insert(root);

    bool isBinaryTree = true;
    TNode nVisited = 0;

    for (TNode v=0;v<n;++v) leftChild[v] = rightChild[v] = NoNode;

    while (!(Q.Empty()))
    {
        TNode u = Q.Delete();
        ordered[nVisited] = u;
        ++nVisited;

        #if defined(_LOGGING_)

        if (CT.logMeth>1)
        {
            sprintf(CT.logBuffer,"%ld ",u);
            LogAppend(LH,CT.logBuffer);
        }

        #endif

        TArc a = First(u);

        do
        {
            // By construction, u is not an isolated node

            TNode v = EndNode(a);

            if (!fixedPredecessors && pred[v]==NoArc && v!=root) pred[v] = a;

            if (pred[v]==a)
            {
                if (leftChild[u]==NoNode)
                {
                    leftChild[u] = v;
                    Q.Insert(v);
                }
                else if (rightChild[u]==NoNode)
                {
                    rightChild[u] = v;
                    Q.Insert(v);
                }
                else
                {
                    isBinaryTree = false;
                }
            }

            a = Right(a,u);
        }
        while (a!=First(u));
    }

    #if defined(_LOGGING_)

    if (CT.logMeth>1) LogEnd(LH);

    #endif

    CloseFold();

    if (nVisited==n)
    {
        // height[v] denotes the height of the subtree rooted at v
        TNode* height = new TNode[n];

        // width[v] denotes the width of the subtree rooted at v
        TNode* width  = new TNode[n];

        // Second pass: Bottom up determination of the width[] and height[] of subtrees
        LogEntry(LOG_METH,"Determine subtree heights and widths...");

        for (TNode i=n;i>0;)
        {
            --i;
            TNode v = ordered[i];

            if (leftChild[v]==NoNode)
            {
                width[v] = height[v] = 0;
            }
            else if (rightChild[v]==NoNode)
            {
                TNode w = leftChild[v];

                if (height[w]==0 || width[w]/height[w]>aspectRatio)
                {
                    // Place w below of v
                    width[v]  = width[w];
                    height[v] = height[w]+1;
                }
                else
                {
                    // Place w right-hand of v
                    width[v]  = width[w]+1;
                    height[v] = height[w];
                }
            }
            else
            {
                TNode wl = leftChild[v];
                TNode wr = rightChild[v];

                if ((width[wl]+width[wr]+1)/(height[wl]+height[wr]+1)>aspectRatio)
                {
                    // Place wl below all descendants of wr
                    width[v]  = (width[wr]+1>width[wl]) ? width[wr]+1 : width[wl];
                    height[v] = height[wl]+height[wr]+1;
                }
                else
                {
                    // Place wr right-hand of all descendants of wl
                    width[v]  = width[wl]+width[wr]+1;
                    height[v] = (height[wl]+1>height[wr]) ? height[wl]+1 : height[wr];
                }
            }
        }


        // Final pass: Assign coordinates according to the width[] and height[]
        // computations as applied in the previous pass

        Layout_ConvertModel(LAYOUT_STRAIGHT_2DIM);
        Layout_ConvertModel(LAYOUT_ORTHO_SMALL);

        LogEntry(LOG_METH,"Assigning coordinates...");

        sparseRepresentation* X = static_cast<sparseRepresentation*>(Representation());

        X -> SetC(root,0,0);
        X -> SetC(root,1,0);

        for (TNode i=0;i<n;++i)
        {
            TNode v = ordered[i];

            if (leftChild[v]==NoNode) continue;

            if (rightChild[v]==NoNode)
            {
                TNode w = leftChild[v];

                if (height[w]==0 || width[w]/height[w]>aspectRatio)
                {
                    // Place w below of v
                    X -> SetC(w,0,C(v,0));
                    X -> SetC(w,1,C(v,1)+CT.nodeSep);
                }
                else
                {
                    // Place w right-hand of v
                    X -> SetC(w,0,C(v,0)+CT.nodeSep);
                    X -> SetC(w,1,C(v,1));
                }
            }
            else
            {
                TNode wl = leftChild[v];
                TNode wr = rightChild[v];

                if ((width[wl]+width[wr]+1)/(height[wl]+height[wr]+1)>aspectRatio)
                {
                    // Place wl below all descendants of wr
                    X -> SetC(wl,0,C(v,0));
                    X -> SetC(wl,1,C(v,1)+CT.nodeSep*(height[wr]+1));
                    X -> SetC(wr,0,C(v,0)+CT.nodeSep);
                    X -> SetC(wr,1,C(v,1));
                }
                else
                {
                    // Place wr right-hand of all descendants of wl
                    X -> SetC(wl,0,C(v,0));
                    X -> SetC(wl,1,C(v,1)+CT.nodeSep);
                    X -> SetC(wr,0,C(v,0)+CT.nodeSep*(width[wl]+1));
                    X -> SetC(wr,1,C(v,1));
                }
            }
        }

        if (CT.logRes)
        {
            sprintf(CT.logBuffer,"...Layout size is %ld x %ld",
                width[root],height[root]);
            LogEntry(LOG_RES,CT.logBuffer);
        }

        delete[] height;
        delete[] width;

        if (!isBinaryTree)
        {
            LogEntry(LOG_RES,"...Graph is not a binary tree");
        }

        if (CT.methLocal==LOCAL_OPTIMIZE) Layout_OrthoSmallLineSweep();
    }
    else
    {
        if (!isBinaryTree)
        {
            LogEntry(LOG_RES,"...Graph is not a binary tree");
        }
        else
        {
            LogEntry(LOG_RES,"...Graph is disconnected");
        }
    }

    delete[] leftChild;
    delete[] rightChild;
    delete[] ordered;

    return (nVisited==n);
}
