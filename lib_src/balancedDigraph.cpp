
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, January 2000
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file   balancedDigraph.cpp
/// \brief  #balancedFNW class implementation

#include "balancedDigraph.h"


balancedFNW::balancedFNW(TNode _n1,goblinController& _CT) throw() :
    managedObject(_CT),
    abstractBalancedFNW(_n1,TArc(0)),
    X(static_cast<const balancedFNW&>(*this))
{
    X.SetCOrientation(1);
    X.NewSubgraph();

    LogEntry(LOG_MEM,"...Balanced flow network instanciated");
}


balancedFNW::balancedFNW(const char* fileName,goblinController& _CT)
    throw(ERFile,ERParse) :
    managedObject(_CT),
    abstractBalancedFNW(TNode(0),TArc(0)),
    X(static_cast<const balancedFNW&>(*this))
{
    #if defined(_TIMERS_)

    CT.globalTimer[TimerIO] -> Enable();

    #endif

    LogEntry(LOG_IO,"Loading balanced flow network...");
    if (!CT.logIO && CT.logMem)
        LogEntry(LOG_MEM,"Loading balanced flow network...");

    goblinImport F(fileName,CT);
    F.Scan("balanced_fnw");
    ReadAllData(F);

    n1 = n>>1;

    if (CT.sourceNode<n) SetSourceNode(CT.sourceNode);
    else SetSourceNode(NoNode);

    if (CT.targetNode<n) SetTargetNode(CT.targetNode);
    else SetTargetNode(NoNode);

    if (CT.rootNode<n) SetRootNode(CT.rootNode);
    else SetRootNode(NoNode);

    X.SetCOrientation(1);

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


unsigned long balancedFNW::Size() const throw()
{
    return
          sizeof(balancedFNW)
        + managedObject::Allocated()
        + abstractMixedGraph::Allocated()
        + abstractDiGraph::Allocated()
        + abstractBalancedFNW::Allocated();
}


unsigned long balancedFNW::Allocated() const throw()
{
    return 0;
}


TFloat balancedFNW::Length(TArc a) const throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (a>=2*m) NoSuchArc("Length",a);

    #endif

    if (X.CLength()) return X.MaxLength();

    return X.Length(a);
}


void balancedFNW::Symmetrize() throw()
{
    LogEntry(LOG_MAN,"Symmetrising flow...");

    TArc m0 = m>>1;
    for (TArc a=0;a<m0;a++)
    {
        TFloat Lambda = (Flow(4*a+2)-Flow(4*a))/2;
        if (Lambda>0)
        {
            Push(4*a,Lambda);
            Push(4*a+3,Lambda);
        }
        else
        {
            Push(4*a+1,-Lambda);
            Push(4*a+2,-Lambda);
        }
    }
}


void balancedFNW::Relax() throw()
{
    LogEntry(LOG_MAN,"Relaxing symmetry...");
}


balancedFNW::~balancedFNW() throw()
{
    LogEntry(LOG_MEM,"...Balanced flow network disallocated");

    if (CT.traceLevel==2) Display();
}


splitGraph::splitGraph(const abstractDiGraph &G,TNode s,TNode t) throw() :
    managedObject(G.Context()),
    balancedFNW(G.N()+1)
{
    #if defined(_FAILSAVE_)

    if (s>=n) NoSuchNode("SplitGraph",s);

    if (t>=n) NoSuchNode("SplitGraph",t);

    #endif

    LogEntry(LOG_MAN,"Generating split graph...");

    X.SetCapacity(2*G.N()+2,2*G.M()+4);

    if (G.Dim()>=2)
    {
        for (TNode v=0;v<G.N();v++)
        {
            X.SetC(2*v,0,G.C(v,0)+10);
            X.SetC(2*v,1,G.C(v,1));
            X.SetC(2*v+1,0,G.CMax(0)-G.C(v,0)+10);
            X.SetC(2*v+1,1,2*G.CMax(1)-G.C(v,1)+10);
        }

        X.SetC(2*G.N(),0,G.CMax(0)+20);
        X.SetC(2*G.N(),1,G.CMax(1)+5);
        X.SetC(2*G.N()+1,0,0);
        X.SetC(2*G.N()+1,1,G.CMax(1)+5);
    }

    TFloat scap = 0;
    TFloat tcap = 0;

    for (TArc a=0;a<G.M();a++)
    {
        TNode u = G.StartNode(2*a);
        TNode v = G.EndNode(2*a);
        InsertArc(2*u,2*v,G.UCap(2*a),G.Length(2*a),G.LCap(2*a));
        if (u==s) scap += G.UCap(2*a);
        if (v==t) tcap += G.UCap(2*a);
    }

    InsertArc(n-1,2*s,scap,0);
    InsertArc(2*t,n-2,tcap,0);
}
