
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, July 2003
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file   solveSteiner.cpp
/// \brief  Codes for Steiner tree enumeration and heuristics

#include "sparseGraph.h"
#include "sparseDigraph.h"


TFloat abstractMixedGraph::SteinerTree(const indexSet<TNode>& Terminals,TNode root) throw(ERRange,ERRejected)
{
    #if defined(_FAILSAVE_)

    if (root>=n && root!=NoNode) NoSuchNode("SteinerTree",root);

    #endif

    OpenFold(ModSteiner,NO_INDENT);

    if (root!=NoNode)
    {
        sprintf(CT.logBuffer,"Computing minimum %ld-Steiner tree...",root);
        LogEntry(LOG_METH,CT.logBuffer);
    }
    else
    {
        LogEntry(LOG_METH,"Computing minimum Steiner tree...");
    }

    TArc* savedTree = NULL;
    TFloat savedLength = 0;

    TArc* pred = GetPredecessors();

    if (pred)
    {
        bool feasible = true;

        TNode rank = 0;
        for (TNode v=0;v<n;v++)
        {
            TArc a = pred[v];
            if (a!=NoArc)
            {
                savedLength += Length(a);
                rank++;
                if (v==root) feasible = false;
            }
        }

        if (rank<n-1) feasible = false;

        // Must add a check for connectivity

        if (feasible)
        {
            savedLength -= STT_TrimLeaves(Terminals,pred);
            savedTree = new TArc[n];

            for (TNode v=0;v<n;v++)  savedTree[v] = pred[v];

            if (CT.logMeth)
            {
                sprintf(CT.logBuffer,"Initial tree has length %g",savedLength);
                LogEntry(LOG_METH,CT.logBuffer);
            }
            SetUpperBound(TimerSteiner,savedLength);
        }
    }

    LogEntry(LOG_METH,"Applying Heuristic...");
    TFloat bestUpper = STT_Heuristic(Terminals,root);

    if (savedTree)
    {
        if (savedLength<bestUpper)
        {
            for (TNode v=0;v<n;v++)  pred[v] = savedTree[v];

            bestUpper = savedLength;
        }

        delete[] savedTree;
    }

    if (CT.methSolve==2)
    {
        LogEntry(LOG_METH,"Exhaustive search...");
        bestUpper = STT_Enumerate(Terminals,root);
    }

    CloseFold(ModSteiner,NO_INDENT);

    if (bestUpper==InfFloat) throw ERRejected();

    return bestUpper;
}


TFloat abstractMixedGraph::STT_Heuristic(const indexSet<TNode>& Terminals,TNode root) throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (root!=NoNode && (root>=n || !Terminals.IsMember(root)))
    {
        sprintf(CT.logBuffer,"Inappropriate root node: %ld",root);
        Error(ERR_RANGE,"STT_Heuristic",CT.logBuffer);
    }

    #endif

    OpenFold(ModSteiner);

    #if defined(_PROGRESS_)

    InitProgressCounter(n+1);
    SetProgressNext(n);

    #endif

    for (TNode v=0;root==NoNode && v<n;v++)
        if (Terminals.IsMember(v)) root = v;

    TFloat ret = MinTree(root);

    #if defined(_PROGRESS_)

    ProgressStep(n);
    SetProgressNext(1);

    #endif

    ret -= STT_TrimLeaves(Terminals,GetPredecessors());

    SetUpperBound(TimerSteiner,ret);

    CloseFold(ModSteiner);

    sprintf(CT.logBuffer,"...Steiner tree has length %g",ret);
    LogEntry(LOG_RES,CT.logBuffer);

    return ret;
}


TFloat abstractMixedGraph::STT_TrimLeaves(const indexSet<TNode>& Terminals,TArc* pred) throw()
{
    LogEntry(LOG_METH,"Trimming the leaves...");
    OpenFold(ModSteiner);

    TNode* odg = new TNode[n];

    for (TNode v=0;v<n;v++) odg[v] = 0;

    for (TNode v=0;v<n;v++)
        if (pred[v]!=NoArc) odg[StartNode(pred[v])]++;

    TFloat diff = 0;
    for (TNode v=0;v<n;v++)
    {
        TNode w = v;
        while (pred[w]!=NoArc && !Terminals.IsMember(w) && odg[w]==0)
        {
            diff += Length(pred[w]);
            TNode x = StartNode(pred[w]);
            odg[x]--;
            pred[w] = NoArc;

            sprintf(CT.logBuffer,"Node %ld deleted",w);
            LogEntry(LOG_METH,CT.logBuffer);

            w = x;
        }
    }

    delete[] odg;

    Trace();

    CloseFold(ModSteiner);

    if (CT.logRes && diff!=0)
    {
        sprintf(CT.logBuffer,"...Tree length decreases by %g",diff);
        LogEntry(LOG_RES,CT.logBuffer);
    }

    return diff;
}


TFloat abstractMixedGraph::STT_Enumerate(const indexSet<TNode>& Terminals,TNode root) throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (root!=NoNode && (root>=n || !Terminals.IsMember(root)))
    {
        sprintf(CT.logBuffer,"Inappropriate root node: %ld",root);
        Error(ERR_RANGE,"STT_Enumerate",CT.logBuffer);
    }

    #endif

    OpenFold(ModSteiner);

    LogEntry(LOG_METH,"(Steiner node enumeration)");

    sprintf(CT.logBuffer,"Root node is %ld",root);
    LogEntry(LOG_METH,CT.logBuffer);

    TNode k = 0;
    for (TNode v=0;v<n;v++)
        if (!Terminals.IsMember(v)) k++;

    sprintf(CT.logBuffer,"%ld Steiner nodes detected",k);
    LogEntry(LOG_METH,CT.logBuffer);
    CT.SuppressLogging();

    TFloat ret = InfFloat;

    if (k==0) ret = MinTree(root);

    if (k==n)
    {
        InitPredecessors();
        ret = 0;
    }

    TNode* nodeColour = InitNodeColours(0);
    bool searching = (k!=0 && k!=n);
    unsigned long itCount = 0;

    while (CT.SolverRunning() && searching)
    {
        itCount++;

        completeOrientation G(*this);
        graphRepresentation* GR = G.Representation();

        for (TArc a=0;a<G.M();a++)
        {
            TArc a0 = G.OriginalArc(2*a);

            if (nodeColour[StartNode(a0)]==1)
            {
                GR -> SetLength(2*a,InfFloat);
            }

            if (nodeColour[EndNode(a0)]==1)
            {
                GR -> SetLength(2*a,0);
            }
        }

        TFloat thisRet = G.MinTree(root);
        if (thisRet<ret)
        {
            TNode uncovered = 0;
            TArc* thisPred = G.GetPredecessors();

            for (TNode v=0;v<n;v++)
                if (Terminals.IsMember(v) && thisPred[v]==NoArc) uncovered++;

            if (uncovered==1)
            {
                ret = thisRet;
                TArc* pred = InitPredecessors();

                for (TNode v=0;v<n;v++)
                    if (nodeColour[v]==0 && v!=root && thisPred[v]!=NoArc)
                        pred[v] = G.OriginalArc(thisPred[v]);

                CT.RestoreLogging();
                SetUpperBound(TimerSteiner,ret);
                CT.SuppressLogging();
            }
        }

        if (LowerBound(TimerSteiner)>=ret) break;

        TNode v = n;
        searching = false;

        while (v>0)
        {
            v--;

            if (Terminals.IsMember(v)) continue;

            if (nodeColour[v]==1)
            {
                nodeColour[v] = 0;
            }
            else
            {
                nodeColour[v] = 1;
                searching = true;
                break;
            }
        }
    }

    CT.RestoreLogging();
    sprintf(CT.logBuffer,"%ld subproblems solved",itCount);
    LogEntry(LOG_METH,CT.logBuffer);

    CloseFold(ModSteiner);

    sprintf(CT.logBuffer,"...Best tree has length %g",ret);
    LogEntry(LOG_RES,CT.logBuffer);

    return ret;
}


TFloat abstractGraph::STT_Heuristic(const indexSet<TNode>& Terminals,TNode root) throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (root!=NoNode && (root>=n || !Terminals.IsMember(root)))
    {
        sprintf(CT.logBuffer,"Inappropriate root node: %ld",root);
        Error(ERR_RANGE,"STT_Heuristic",CT.logBuffer);
    }

    for (TNode v=0;root==NoNode && v<n;v++)
        if (Terminals.IsMember(v)) root = v;

    if (root==NoNode)
        Error(ERR_REJECTED,"STT_Heuristic","No terminal node found");

    #endif

    OpenFold(ModMehlhorn);

    #if defined(_PROGRESS_)

    InitProgressCounter(3);
    SetProgressNext(1);

    #endif

    voronoiDiagram G(*this,Terminals);

    #if defined(_PROGRESS_)

    ProgressStep(1);

    #endif

    LogEntry(LOG_METH,"Computing minimum spanning tree...");

    TFloat lower =
        ceil(G.MST_Prim(MST_PRIM2,MST_PLAIN)/(2*(1-1/TFloat(G.N()))));
    SetLowerBound(TimerSteiner,lower);

    G.UpdateSubgraph();

    Trace(1);

    #if defined(_LOGGING_)

    LogEntry(LOG_METH2,"Extracting tree from subgraph...");

    #endif

    TArc* pred = InitPredecessors();
    TFloat length = 0;

    THandle H = Investigate();
    investigator &I = Investigator(H);
    TNode w = root;

    while (I.Active(w) || w!=root)
    {
        if (I.Active(w))
        {
            TArc a = I.Read(w);
            TNode u = EndNode(a);

            if (Sub(a)>0 && a!=(pred[w]^1))
            {
                if (pred[u]==NoArc)
                {
                    pred[u] = a;
                    length += Length(a);
                    if (u!=root) w = u;
                }
            }
        }
        else w = StartNode(pred[w]);
    }

    Close(H);

    SetUpperBound(TimerSteiner,length);

    Trace(1); 

    CloseFold(ModMehlhorn);

    if (CT.logRes)
    {
        sprintf(CT.logBuffer,"...Steiner tree has length %g",length);
        LogEntry(LOG_RES,CT.logBuffer);
    }

    return length;
}


TFloat abstractGraph::STT_Enumerate(const indexSet<TNode>& Terminals,TNode root) throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (root!=NoNode && (root>=n || !Terminals.IsMember(root)))
    {
        sprintf(CT.logBuffer,"Inappropriate root node: %ld",root);
        Error(ERR_RANGE,"STT_Enumerate",CT.logBuffer);
    }

    for (TNode v=0;root==NoNode && v<n;v++)
        if (Terminals.IsMember(v)) root = v;

    if (root==NoNode)
        Error(ERR_REJECTED,"STT_Enumerate","No terminal node found");

    #endif

    OpenFold(ModSteiner);

    LogEntry(LOG_METH,"(Steiner node enumeration)");

    sprintf(CT.logBuffer,"Root node is %ld",root);
    LogEntry(LOG_METH,CT.logBuffer);

    TNode k = 0;
    for (TNode v=0;v<n;v++)
        if (!Terminals.IsMember(v)) k++;

    sprintf(CT.logBuffer,"%ld Steiner nodes detected",k);
    LogEntry(LOG_METH,CT.logBuffer);
    CT.SuppressLogging();

    TFloat ret = InfFloat;

    if (k==0) ret = MST_Prim(MST_PRIM2,MST_PLAIN,root);

    if (k==n)
    {
        InitPredecessors();
        ret = 0;
    }

    TNode* nodeColour = InitNodeColours(0);
    bool searching = (k!=0 && k!=n);
    unsigned long itCount = 0;

    while (CT.SolverRunning() && searching)
    {
        itCount++;

        graph G(*this,OPT_CLONE);
        graphRepresentation* GR = G.Representation();

        for (TArc a=0;a<2*m;a++)
        {
            if (nodeColour[StartNode(a)]==1)
            {
                GR -> SetLength(a,InfFloat);
            }
        }

        if (G.MST_Prim(MST_PRIM2,MST_PLAIN,root)<InfFloat)
        {
            TArc* thisPred = G.GetPredecessors();
            TFloat thisRet = 0;

            for (TNode v=0;v<n;v++)
                if (nodeColour[v]==0 && v!=root && thisPred[v]!=NoArc)
                    thisRet += Length(thisPred[v]);

            if (thisRet<ret)
            {
                TNode uncovered = 0;
                for (TNode v=0;v<n;v++)
                    if (Terminals.IsMember(v) && thisPred[v]==NoArc) uncovered++;

                if (uncovered==1)
                {
                    ret = thisRet;
                    TArc* pred = InitPredecessors();

                    for (TNode v=0;v<n;v++)
                        if (nodeColour[v]==0 && v!=root && thisPred[v]!=NoArc)
                            pred[v] = thisPred[v];
                    CT.RestoreLogging();
                    SetUpperBound(TimerSteiner,ret);
                    CT.SuppressLogging();
                }
            }
        }

        if (LowerBound(TimerSteiner)>=ret) break;

        TNode v = n;
        searching = false;

        while (v>0)
        {
            v--;

            if (Terminals.IsMember(v)) continue;

            if (nodeColour[v]==1)
            {
                nodeColour[v] = 0;
            }
            else
            {
                nodeColour[v] = 1;
                searching = true;
                break;
            }
        }
    }

    CT.RestoreLogging();
    sprintf(CT.logBuffer,"%ld subproblems solved",itCount);
    LogEntry(LOG_METH,CT.logBuffer);

    CloseFold(ModSteiner);

    sprintf(CT.logBuffer,"...Best tree has length %g",ret);
    LogEntry(LOG_RES,CT.logBuffer);

    return ret;
}
