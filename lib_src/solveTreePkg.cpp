
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Markus Schwank, November 2001
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file   solveTreePkg.cpp
/// \brief  Code for packing spanning arborescences

#include "sparseDigraph.h"


TCap abstractDiGraph::TreePacking(TNode root) throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (root>=n && root!=NoNode) NoSuchNode("TreePacking",root);

    #endif

    OpenFold(ModTreePack,NO_INDENT);

    #if defined(_PROGRESS_)

    InitProgressCounter(1);
    SetProgressNext(0);

    #endif

    TArc* edgeColour = InitEdgeColours(NoArc);
    long unsigned step = 0;
    TCap totalMulti = InfCap;

    abstractDiGraph *G = TreePKGInit(&totalMulti,root);

    TNode* nodeColour = RawNodeColours();
    for (TNode v=0;v<n;v++) nodeColour[v] = G->NodeColour(v);

    TCap remMulti = totalMulti;

    LogEntry(LOG_METH,"Packing with arborescences...");
    OpenFold();

    SetBounds(TimerTreePack,0,totalMulti);

    #if defined(_PROGRESS_)

    SetProgressMax(totalMulti*n+1.0);
    ProgressStep(1);
    SetProgressNext(1);

    #endif

    while (remMulti>0)
    {
        step++;

        sprintf(CT.logBuffer,
            "Computing the %ldth spanning %ld-arborescence...",step,root);
        LogEntry(LOG_METH,CT.logBuffer);
        OpenFold();

        TreePKGStripTree(G,&remMulti,root);
        TArc* pred = GetPredecessors();

        for (TNode v=0;v<n;v++)
        {
            if (v!=root) edgeColour[pred[v]>>1] = step-1;
        }

        if (CT.traceLevel==3) Display();

        CloseFold();
        sprintf(CT.logBuffer,"...remaining multiplicity: %g",remMulti);
        LogEntry(LOG_METH,CT.logBuffer);

        SetLowerBound(TimerTreePack,totalMulti-remMulti);

        #if defined(_PROGRESS_)

        SetProgressCounter((totalMulti-remMulti)*n+1.0);

        #endif
    }

    delete G;
    ReleasePredecessors();

    CloseFold();

    CloseFold(ModTreePack,NO_INDENT);

    return totalMulti;
}


abstractDiGraph* abstractDiGraph::TreePKGInit(TCap* totalMulti,TNode root) throw()
{
    abstractDiGraph *G = new completeOrientation(*this);

    LogEntry(LOG_METH,"Computing number of arborescences...");
    OpenFold();

    (*totalMulti) = G->StrongEdgeConnectivity(root);

    sprintf(CT.logBuffer,"...total multiplicity: %g",(*totalMulti));
    LogEntry(LOG_METH,CT.logBuffer);

    CloseFold();

    return G;
}


TCap abstractDiGraph::TreePKGStripTree(abstractDiGraph* G,TCap* multi,TNode root)
    throw(ERRange)
{
    graphRepresentation* GR = G->Representation();

    #if defined(_FAILSAVE_)

    if (root>=n && root!=NoNode) NoSuchNode("TreePKGStripTree",root);

    if (!GR) NoRepresentation("TreePKGStripTree");

    #endif

    CT.SuppressLogging();

    TFloat rank = 0;

    TNode* p = new TNode[n];
    TNode* nr = new TNode[n];

    for (TNode v=0;v<n;v++)
    {
        nr[v] = 0;
        p[v]  = 0;
    }

    nr[root] = 1;
    TNode x = root;

    bool* used = new bool[m];

    for (TArc a=0;a<2*m;a++) used[a>>1] = false;

    THandle H = Investigate();
    investigator &I = Investigator(H);

    while (rank<n-1)
    {
        while (I.Active(x) && rank<n-1)
        {
            TArc a = I.Read(x);

            if (a%2==0 && !used[a>>1] && G->UCap(a)>0)
            {
                used[a>>1] = true;
                TNode w = EndNode(a);

                if (nr[w]==0)
                {
                    GR -> SetUCap(a,G->UCap(a)-1);

                    if (G->StrongEdgeConnectivity(root)>=(*multi)-1)
                    {
                        rank++;
                        p[w] = x;
                        nr[w] = 1;
                        x = w;

                        #if defined(_PROGRESS_)

                        ProgressStep(1);

                        #endif
                    }
                    else
                    {
                        GR -> SetUCap(a,G->UCap(a)+1);
                    }
                }
            }
        }

        x = p[x];

        I.Reset(x);
    }

    Close(H);

    delete[] used;
    delete[] nr;

    CT.RestoreLogging();

    LogEntry(LOG_METH,"Computing the minimum tree arc capacity...");

    TCap minCap = InfCap;

    TArc* pred = InitPredecessors();

    for (TNode v=0;v<n;v++)
    {
        if (v!=root)
        {
            pred[v] = Adjacency(p[v],v);

            TCap thisCap = GR->UCap(pred[v])+1;
            GR -> SetUCap(pred[v],thisCap);

            if (thisCap<minCap) minCap = thisCap;
        }
    }

    delete[] p;

    sprintf(CT.logBuffer,
        "...Minimum arc capacity: %g",minCap);
    LogEntry(LOG_RES,CT.logBuffer);

    LogEntry(LOG_METH,"Computing tree capacity...");
    CT.SuppressLogging();

    TFloat cap = minCap;

    for (TNode v=0;v<n;v++)
    {
        if (v!=root) GR -> SetUCap(pred[v],G->UCap(pred[v])-minCap);
    }

    OpenFold();

    while (G->StrongEdgeConnectivity(root)!=(*multi)-cap && cap>0)
    {
        cap--;

        for (TNode v=0;v<n;v++)
        {
            if (v!=root)
            {
                TArc a = pred[v];
                GR -> SetUCap(a,G->UCap(a)+1);
            }
        }

        #if defined(_PROGRESS_)

        ProgressStep(1);

        #endif
    }

    CloseFold();

    CT.RestoreLogging();

    sprintf(CT.logBuffer,
        "...Tree has capacity: %g",cap);
    LogEntry(LOG_RES,CT.logBuffer);

    (*multi) -= cap;

    return cap;
}
