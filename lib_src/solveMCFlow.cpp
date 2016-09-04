
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, July 2003
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file solveMCFlow.cpp
/// \brief Mimum cost flow solvers and methods

#include "sparseDigraph.h"
#include "staticQueue.h"
#include "nestedFamily.h"
#include "lpSolver.h"
#include "networkSimplex.h"


TFloat abstractMixedGraph::MinCostSTFlow(TMethMCFST method,TNode s,TNode t)
    throw(ERRange,ERRejected)
{
    #if defined(_FAILSAVE_)

    if (s>=n) NoSuchNode("MinCostSTFlow",s);

    if (t>=n) NoSuchNode("MinCostSTFlow",t);

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

        TFloat* potential = GetPotentials();

        if (potential)
        {
            TFloat* potentialG = G->RawPotentials();
            for (TNode v=0;v<n;++v) potentialG[v] = potential[v];
        }
    }

    #if defined(_FAILSAVE_)

    for (TNode i=0;i<n;i++)
    {
        if (i==s || i==t) continue;

        if (G->Divergence(i)!=G->Demand(i))
        {
            sprintf(CT.logBuffer,"Node %lu is unbalanced",i);
            Error(ERR_REJECTED,"MinCostSTFlow",CT.logBuffer);
        }
    }

    if (G->Demand(s)!=InfCap && G->Demand(t)!=InfCap)
    {
        TFloat delta = G->Demand(t)-G->Divergence(t);

        if (delta<0)
            Error(ERR_REJECTED,"MinCostSTFlow","Target node overflow");

        if (delta!=G->Divergence(s)-G->Demand(s))
            Error(ERR_REJECTED,"MinCostSTFlow","Mismatching node demands");
    }

    #endif


    OpenFold(ModMinCFlow,NO_INDENT);

    if (method==MCF_ST_DEFAULT) method = TMethMCFST(CT.methMCFST);

    if (CT.logMeth)
    {
        sprintf(CT.logBuffer,"Computing minimum cost (%ld,%ld)-flow...",s,t);
        LogEntry(LOG_METH,CT.logBuffer);
    }

    TFloat ret = InfFloat;

    try
    {
        switch (method)
        {
            case MCF_ST_DIJKSTRA:
            {
                ret = G->MCF_EdmondsKarp(s,t);
                break;
            }
            case MCF_ST_SAP:
            {
                ret = G->MCF_BusackerGowen(s,t);
                break;
            }
            case MCF_ST_BFLOW:
            {
                LogEntry(LOG_METH,"Switching to b-flow solver...");

                G -> MaxFlow(s,t);
                ret = G->MinCostBFlow();

                break;
            }
            default:
            {
                #if defined(_FAILSAVE_)

                UnknownOption("MinCostSTFlow",method);

                #endif
            }
        }
    }
    catch (ERRejected)
    {
        CloseFold(ModMinCFlow,NO_INDENT);
        if (!IsDirected()) delete G;
        throw ERRejected();
    }

    if (!IsDirected())
    {
        static_cast<completeOrientation*>(G) -> MapFlowBackward(*this);

        TFloat* potentialG = G->GetPotentials();
        TFloat* potential = RawPotentials();

        for (TNode v=0;v<n;++v) potential[v] = potentialG[v];
    }

    #if defined(_FAILSAVE_)

    if (CT.methFailSave==1 && !G->Compatible())
    {
        if (!IsDirected()) delete G;
        InternalError("MinCostSTFlow","Solutions are corrupted");
    }

    try
    {
        G -> FlowValue(s,t);
    }
    catch (...)
    {
        if (!IsDirected()) delete G;
        InternalError("MinCostSTFlow","Solutions are corrupted");
    }

    #endif

    if (!IsDirected()) delete G;

    CloseFold(ModMinCFlow,NO_INDENT);

    return ret;
}


TFloat abstractDiGraph::MCF_BusackerGowen(TNode s,TNode t) throw(ERRejected)
{
    TFloat delta = Demand(t);

    if (delta!=InfCap) delta -= Divergence(t);

    #if defined(_PROGRESS_)

    InitProgressCounter(delta+1);
    SetProgressNext(1);

    #endif

    OpenFold(ModBusackerGowen,SHOW_TITLE);

    TArc* pred = NULL;

    while (delta>0)
    {
        ShortestPath(SPX_DEFAULT,SPX_RED_RES,s,t);

        if (!pred) pred = GetPredecessors();

        if (Dist(t)==InfFloat)
        {
            LogEntry(LOG_RES,"...Problem is infeasible");
            SetBounds(TimerMinCFlow,InfFloat,InfFloat);
            Trace();
            delete nHeap;
            nHeap = NULL;
            CloseFold(ModBusackerGowen);
            return InfFloat;
        }

        TFloat Lambda = FindCap(pred,s,t);

        if (delta<Lambda) Lambda = delta;

        if (Lambda==InfCap)
        {
            LogEntry(LOG_RES,"...Problem is unbounded");
            SetBounds(TimerMinCFlow,-InfFloat,-InfFloat);
            Trace();
            delete nHeap;
            nHeap = NULL;
            CloseFold(ModBusackerGowen);
            return -InfFloat;
        }

        Augment(pred,s,t,Lambda);

        if (delta!=InfCap) delta -= Lambda;

        Trace((unsigned long)Lambda);

        #if defined(_PROGRESS_)

        SetProgressNext(1);

        #endif
    }

    ReleasePotentials();

    LogEntry(LOG_METH,"Computing optimal node potentials...");

    NegativeCycle(SPX_RESIDUAL);
    UpdatePotentials(InfFloat);

    TFloat w = Weight();
    SetBounds(TimerMinCFlow,MCF_DualObjective(),w);

    CloseFold(ModBusackerGowen);

    return w;
}


TFloat abstractDiGraph::MCF_EdmondsKarp(TNode s,TNode t) throw(ERRejected)
{
    TFloat delta = Demand(t);

    if (delta!=InfCap) delta -= Divergence(t);

    OpenFold(ModShortPath2,SHOW_TITLE);

    #if defined(_PROGRESS_)

    InitProgressCounter(delta+1);
    SetProgressNext(1);

    #endif


    // First, check for complementary slackness of the input solutions

    TFloat* potential = GetPotentials();
    bool methodAdmissible = true;

    for (TArc a=0;a<2*m && methodAdmissible;a++)
    {
        if (ResCap(a)>0 && RedLength(potential,a)<0) methodAdmissible = false;
    }


    // If necessary, determine compatible node potentials

    if (!methodAdmissible) try
    {
        LogEntry(LOG_RES,"...Negative length residual arcs detected");
        LogEntry(LOG_METH,"Searching for compatible node potentials...");

        SPX_FIFOLabelCorrecting(SPX_RESIDUAL,s);

        TFloat* dist = GetDistanceLabels();
        potential = RawPotentials();

        for (TNode i=0;i<n;i++) potential[i] = dist[i];
    }
    catch (ERCheck)
    {
        LogEntry(LOG_RES,"...Complementary slacks cannot be satisfied");
        CloseFold(ModShortPath2);
        throw ERRejected();
    }

    #if defined(_PROGRESS_)

    SetProgressCounter(1);

    #endif


    nHeap = NewNodeHeap();

    TArc* pred = NULL;

    while (delta>0)
    {
        SetLowerBound(TimerMinCFlow,MCF_DualObjective());

        ShortestPath(SPX_DIJKSTRA,SPX_RED_RES,s,t);
        TFloat dt = Dist(t);

        if (dt==InfFloat)
        {
            LogEntry(LOG_RES,"...Problem is infeasible");
            SetBounds(TimerMinCFlow,InfFloat,InfFloat);
            Trace();
            delete nHeap;
            nHeap = NULL;
            CloseFold(ModShortPath2);
            return InfFloat;
        }

        if (!pred) pred = GetPredecessors();

        TFloat Lambda = FindCap(pred,s,t);

        if (delta<Lambda) Lambda = delta;

        if (Lambda==InfCap)
        {
            LogEntry(LOG_RES,"...Problem is unbounded");
            SetBounds(TimerMinCFlow,-InfFloat,-InfFloat);
            Trace();
            delete nHeap;
            nHeap = NULL;
            CloseFold(ModShortPath2);
            return -InfFloat;
        }

        Augment(pred,s,t,Lambda);

        if (delta!=InfCap) delta -= Lambda;

        UpdatePotentials(dt);

        Trace((unsigned long)Lambda);

        #if defined(_PROGRESS_)

        SetProgressNext(1);

        #endif
    }

    delete nHeap;
    nHeap = NULL;

    TFloat w = Weight();
    SetBounds(TimerMinCFlow,MCF_DualObjective(),w);

    CloseFold(ModShortPath2);

    return w;
}


TFloat abstractMixedGraph::MinCostBFlow(TMethMCF method) throw(ERRejected)
{
    #if defined(_FAILSAVE_)

    TCap sumDemands = 0;

    for (TNode i=0;i<n;i++) sumDemands += Demand(i);

    if (sumDemands!=0)
        Error(ERR_REJECTED,"MinCostBFlow","Mismatching node demands");

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

        TFloat* potential = GetPotentials();

        if (potential)
        {
            TFloat* potentialG = G->RawPotentials();
            for (TNode v=0;v<n;++v) potentialG[v] = potential[v];
        }
    }


    OpenFold(ModMinCFlow,NO_INDENT);

    if (method==MCF_BF_DEFAULT) method = TMethMCF(CT.methMCF);

    if (method!=MCF_BF_SAP && method!=MCF_BF_CAPA && !G->AdmissibleBFlow())
    {
        if (!IsDirected()) delete G;
        CloseFold(ModMinCFlow,NO_INDENT);
        LogEntry(LOG_RES,"...Problem is infeasible");
        SetBounds(TimerMinCFlow,InfFloat,InfFloat);
        return InfFloat;
    }

    if (method!=MCF_BF_PHASE1) LogEntry(LOG_METH,"Computing minimum cost circulation...");

    TFloat ret = InfFloat;

    switch (method)
    {
        case MCF_BF_CYCLE:
        {
            ret = G->MCF_CycleCanceling();
            break;
        }
        case MCF_BF_COST:
        case MCF_BF_TIGHT:
        {
            ret = G->MCF_CostScaling(method);
            break;
        }
        case MCF_BF_MEAN:
        {
            ret = G->MCF_MinMeanCycleCanceling();
            break;
        }
        case MCF_BF_SAP:
        {
            ret = G->MCF_ShortestAugmentingPath();
            break;
        }
        case MCF_BF_SIMPLEX:
        {
            ret = G->MCF_NWSimplex();
            break;
        }
        case MCF_BF_LINEAR:
        {
            goblinILPWrapper* XLP  =
                static_cast<goblinILPWrapper*>(G->BFlowToLP());

            if (m>0)
            {
                XLP -> ResetBasis();
                XLP -> SolveLP();
            }

            TFloat* potential = G->RawPotentials();

            for (TNode v=0;v<n;v++)
            {
                potential[v] = -XLP->Y(v,goblinILPWrapper::LOWER)
                               -XLP->Y(v,goblinILPWrapper::UPPER);
            }

            for (TArc a=0;a<G->M();a++)
            {
                G->SetSub(2*a,XLP->X(a));
            }

            ret = XLP -> ObjVal();

            delete XLP;

            SetBounds(TimerMinCFlow,ret,ret);

            break;
        }
        case MCF_BF_CAPA:
        {
            ret = G->MCF_CapacityScaling();
            break;
        }
        case MCF_BF_PHASE1:
        {
            break;
        }
        default:
        {
            if (!IsDirected()) delete G;
            UnknownOption("MinCostBFlow",method);
        }
    }

    if (!IsDirected())
    {
        static_cast<completeOrientation*>(G) -> MapFlowBackward(*this);

        TFloat* potentialG = G->GetPotentials();
        TFloat* potential = RawPotentials();

        for (TNode v=0;v<n;++v) potential[v] = potentialG[v];
    }

    #if defined(_FAILSAVE_)

    for (TNode v=0;v<n && CT.methFailSave==1;v++)
    {
        if (G->Divergence(v)!=G->Demand(v))
        {
            #if defined(_LOGGING_)

            if (!IsDirected()) delete G;
            CloseFold(ModMinCFlow,NO_INDENT);
            InternalError("MinCostBFlow","Not a legal b-flow");

            #endif
        }
    }

    if (CT.methFailSave==1 && CT.SolverRunning() && !G->Compatible())
    {
        if (!IsDirected()) delete G;
        CloseFold(ModMinCFlow,NO_INDENT);
        InternalError("MinCostBFlow","Solutions are corrupted");
    }

    #endif

    if (!IsDirected()) delete G;

    CloseFold(ModMinCFlow,NO_INDENT);

    return ret;
}


TFloat abstractDiGraph::MCF_CycleCanceling() throw()
{
    OpenFold(ModKleinCanceling,SHOW_TITLE);

    TFloat w = Weight();

    #if defined(_PROGRESS_)

    InitProgressCounter(1);
    TFloat wInit = w;
    TFloat initialLower = MCF_DualObjective();
    SetLowerBound(TimerMinCFlow,initialLower);

    #endif

    InitPotentials();
    TArc* pred = NULL;

    while (CT.SolverRunning())
    {
        SetUpperBound(TimerMinCFlow,w);

        LogEntry(LOG_METH,"Searching for negative length augmenting cycle...");

        TNode x = NegativeCycle(SPX_RESIDUAL);

        if (x==NoNode) break;

        if (!pred) pred = GetPredecessors();

        TFloat Lambda = FindCap(pred,x,x);

        if (Lambda==InfCap)
        {
            LogEntry(LOG_RES,"...Problem is unbounded");
            SetBounds(TimerMinCFlow,-InfFloat,-InfFloat);
            Trace();
            CloseFold(ModKleinCanceling);
            return -InfFloat;
        }

        Augment(pred,x,x,Lambda);

        w = Weight();

        #if defined(_PROGRESS_)

        SetProgressCounter(1 - (w-initialLower) / (wInit-initialLower));

        #endif

        Trace(0);
    }

    UpdatePotentials(0);

    SetLowerBound(TimerMinCFlow,MCF_DualObjective());

    CloseFold(ModKleinCanceling);

    return w;
}


TFloat abstractDiGraph::MCF_MinMeanCycleCanceling() throw()
{
    OpenFold(ModMeanCycleCanceling,SHOW_TITLE);

    TFloat w = Weight();

    #if defined(_PROGRESS_)

    InitProgressCounter(1);
    TFloat wInit = w;
    TFloat initialLower = MCF_DualObjective();
    SetLowerBound(TimerMinCFlow,initialLower);

    #endif

    TArc* pred = NULL;

    while (CT.SolverRunning())
    {
        SetUpperBound(TimerMinCFlow,w);

        LogEntry(LOG_METH,"Searching for minimum mean augmenting cycle...");

        TFloat meanValue = 0;
        TNode x = MinimumMeanCycle(SPX_RESIDUAL,&meanValue);

        if (x==NoNode || meanValue>=0) break;

        Trace();

        if (!pred) pred = GetPredecessors();

        TFloat Lambda = FindCap(pred,x,x);

        if (Lambda==InfCap)
        {
            LogEntry(LOG_RES,"...Problem is unbounded");
            SetBounds(TimerMinCFlow,-InfFloat,-InfFloat);
            Trace();
            CloseFold(ModMeanCycleCanceling);
            return -InfFloat;
        }

        Augment(pred,x,x,Lambda);

        w = Weight();

        #if defined(_PROGRESS_)

        SetProgressCounter(1 - (w-initialLower) / (wInit-initialLower));

        #endif
    }

    if (CT.SolverRunning())
    {
        LogEntry(LOG_METH,"Computing optimum node potentials...");

        NegativeCycle(SPX_RESIDUAL);
    }

    UpdatePotentials(0);

    SetLowerBound(TimerMinCFlow,MCF_DualObjective());

    CloseFold(ModMeanCycleCanceling);

    return w;
}


TFloat abstractDiGraph::MCF_CostScaling(TMethMCF method) throw()
{
    OpenFold(ModCostScaling,SHOW_TITLE);

    #if defined(_PROGRESS_)

    InitProgressCounter(1);
    SetProgressNext(0);
    TFloat initialEpsilon = -1;
    TFloat nIterations = 0;

    #endif

    TFloat w = Weight();
    SetUpperBound(TimerMinCFlow,w);

    InitDistanceLabels();

    TFloat* potential = GetPotentials();
    if (!potential) potential = InitPotentials(0);

    TNode* nodeColour = InitNodeColours();
    // Colours maintain a topological ordering of the admissible graph


    // A queue for the active nodes

    goblinQueue<TNode,TFloat> *Q = NewNodeHeap();

    // Additional incidence structure which is initialized by relabel(u)
    // operations. Admissible[u] contains of all arcs a=uv such that
    // rescap(a)>0 and dist[u]=dist[v]+1.

    staticQueue<TNode,TFloat> **Admissible = new staticQueue<TNode,TFloat>*[n];
    Admissible[0] = new staticQueue<TArc,TFloat>(2*m,CT);

    for (TNode i=1;i<n;i++)
        Admissible[i] = new staticQueue<TArc,TFloat>(*Admissible[0]);

    THandle H = Investigate();
    investigator &I = Investigator(H);

    TFloat epsilon = InfFloat;

    while (CT.SolverRunning())
    {
        bool searching = true;
        bool setPotentials = false;
        TFloat meanValue = 0;

        if (method==MCF_BF_TIGHT)
        {
            LogEntry(LOG_METH,"Checking for tightness...");

            MinimumMeanCycle(SPX_RESIDUAL,&meanValue);
            epsilon = -meanValue;

            if (meanValue>=0) searching = false;

            setPotentials = true;
        }
        else
        {
            epsilon = 0;

            for (TArc a=0;a<2*m;a++)
            {
                if (ResCap(a)>0 && fabs(Length(a))!=InfFloat && RedLength(potential,a)<(-epsilon))
                {
                    epsilon = -RedLength(potential,a);
                }
            }

            if (n*epsilon<CT.tolerance)
            {
                searching = false;
                setPotentials = true;
            }
        }

        if (setPotentials)
        {
            if (searching)
            {
                NegativeCycle(SPX_RESIDUAL,NoNode,meanValue);
            }
            else
            {
                // Don't be satisfied with near-optimial potentials.
                // At least, the matching solver could be confused with that!
                InitPotentials();
                NegativeCycle(SPX_RESIDUAL);
            }

            UpdatePotentials(0);
        }

        SetLowerBound(TimerMinCFlow,MCF_DualObjective());

        if (!searching) break;

        #if defined(_PROGRESS_)

        if (initialEpsilon==-1)
        {
            initialEpsilon = epsilon;
            nIterations = -log(1/(epsilon*n));
        }

        SetProgressCounter(-log(epsilon/initialEpsilon)/
                                nIterations*log(2.0));

        #endif

        if (CT.logMeth)
        {
            sprintf(CT.logBuffer,
                "Starting scaling phase with epsilon = %.3f...",epsilon);
            LogEntry(LOG_METH,CT.logBuffer);
        }

        OpenFold();

        Trace();

        for (TArc a=0;a<2*m;a++)
        {
            if (ResCap(a)>0 && RedLength(potential,a)<0)
            {
                #if defined(_LOGGING_)

                if (CT.logMeth>1)
                {
                    sprintf(CT.logBuffer,
                        "Pushed %g flow units from node %ld to node %ld",
                        ResCap(a),StartNode(a),EndNode(a));
                    LogEntry(LOG_METH2,CT.logBuffer);
                }

                #endif

                Push(a,ResCap(a));
            }
        }

        for (TNode v=0;v<n;v++)
        {
            nodeColour[v] = 0;

            if (Divergence(v)>Demand(v))
            {
                Q -> Insert(v,nodeColour[v]);

                I.Reset(v);

                while (I.Active(v))
                {
                    TArc a = I.Read(v);

                    if (ResCap(a)>0 && RedLength(potential,a)<0)
                        Admissible[v]->Insert(a);
                }
            }
        }

        while (!Q->Empty())
        {
            TNode v = Q->Delete();
            I.Reset(v);

            while (Divergence(v)>Demand(v) && !Admissible[v]->Empty())
            {
                TArc a = Admissible[v]->Delete();

                if (ResCap(a)>0 && RedLength(potential,a)<0)
                {
                    TNode w = EndNode(a);
                    TFloat Lambda = ResCap(a);

                    if (Lambda>Divergence(v)-Demand(v))
                        Lambda = Divergence(v)-Demand(v);

                    Push(a,Lambda);

                    if (ResCap(a)>0) Admissible[v]->Insert(a);

                    #if defined(_LOGGING_)

                    if (CT.logMeth>1)
                    {
                        sprintf(CT.logBuffer,
                            "Pushed %g flow units from node %ld to node %ld",
                            Lambda,v,w);
                        LogEntry(LOG_METH2,CT.logBuffer);
                    }

                    #endif

                    if (Divergence(w)>Demand(w)
                        && Divergence(w)-Demand(w)<=Lambda)
                        Q -> Insert(w,nodeColour[w]);
                }
            }

            if (Divergence(v)>Demand(v))
            {
                #if defined(_LOGGING_)

                if (CT.logMeth>1)
                {
                    sprintf(CT.logBuffer,"Relabelled node %ld: %g -> %g",
                        v,potential[v],potential[v]-epsilon/2);
                    LogEntry(LOG_METH2,CT.logBuffer);
                }

                #endif

                potential[v] -= epsilon/2;

                I.Reset(v);

                while (I.Active(v))
                {
                    TArc a = I.Read(v);
                    TNode w = EndNode(a);

                    if (ResCap(a)>0 && RedLength(potential,a)<0)
                        Admissible[v]->Insert(a,INSERT_NO_THROW);

                    if (ResCap(a^1)>0 && RedLength(potential,a^1)<0)
                        Admissible[w]->Insert(a^1,INSERT_NO_THROW);
                }

                for (TNode w=0;w<n;w++)
                {
                    if (nodeColour[w]<nodeColour[v])
                    {
                        nodeColour[w]--;

                        if (Divergence(w)>Demand(w))
                            Q->ChangeKey(w,nodeColour[w]); 
                    }
                }

                nodeColour[v] = 0;
                Q -> Insert(v,nodeColour[v]);
            }
        }

        CloseFold();

        w = Weight();
        SetUpperBound(TimerMinCFlow,w);

        if (CT.logRes>1)
        {
            sprintf(CT.logBuffer,
                "...Feasible flow of weight %.3f found",w);
            LogEntry(LOG_RES2,CT.logBuffer);
        }
    }

    Close(H);

    for (TNode i=1;i<n;i++) delete Admissible[i];

    delete Admissible[0];
    delete[] Admissible;
    delete Q;
    ReleaseNodeColours();

    CloseFold(ModCostScaling);

    return w;
}


TFloat abstractDiGraph::MCF_CapacityScaling(bool doScale) throw(ERRejected)
{
    // Check if node demands resolve and compute the total imbalance defPlus

    TFloat defPlus  = 0;
    TFloat defMinus = 0;

    for (TNode i=0;i<n;i++)
    {
        TFloat deficiency = Demand(i)-Divergence(i);

        if (deficiency>0)
        {
            defPlus += deficiency;
        }
        else
        {
            defMinus -= deficiency;
        }
    }

    #if defined(_FAILSAVE_)

    if (defPlus!=defMinus)
    {
        Error(ERR_REJECTED,"MCF_CapacityScaling",
            "Node demands do not resolve");
    }

    #endif

    if (doScale)
    {
        OpenFold(ModMCFCapScaling,SHOW_TITLE);

        delta =  MaxUCap();

        if (delta>0) delta -= 1;
    }
    else
    {
        OpenFold(ModShortPath2,SHOW_TITLE);

        delta = 0;
    }

    TFloat* potential = RawPotentials();

    supersaturatedNodes S(*this);
    deficientNodes T(*this);

    nHeap = NewNodeHeap();


    #if defined(_PROGRESS_)

    double nPhases = 1;
    if (delta>0) nPhases = floor(log(delta)/log(double(2)))+2;

    if (doScale)
    {
        InitProgressCounter(2*(m+n)*nPhases);
    }
    else
    {
        InitProgressCounter(defPlus);
    }

    SetProgressNext(1);

    #endif

    TArc* pred = NULL;

    while (CT.SolverRunning())
    {
        // Set initial pseudoflow to satisfy the reduced optimality criterion

        for (TArc a=0;a<2*m;a++)
        {
            if (ResCap(a)>delta && RedLength(potential,a)<0) Push(a,ResCap(a));
        }

        if (CT.logMeth && doScale)
        {
            sprintf(CT.logBuffer,"Next scaling phase, delta = %.0f",delta);
            LogEntry(LOG_METH,CT.logBuffer);
        }

        #if defined(_PROGRESS_)

        double newPhases = 1;
        if (delta>0) newPhases = floor(log(delta)/log(double(2)))+2;
        SetProgressCounter(2*(m+n)*(nPhases-newPhases));

        #endif

        if (doScale) OpenFold();

        while (CT.SolverRunning() && (doScale || defPlus>0))
        {
            SetLowerBound(TimerMinCFlow,MCF_DualObjective());
            LogEntry(LOG_METH2,"Computing shortest augmenting path...");

            TNode t = SPX_Dijkstra(SPX_RED_RES,S,T);

            if (t==NoNode) break;

            TNode s = t;
            TFloat Lambda = InfFloat;

            if (!pred) pred = GetPredecessors();

            do
            {
                TArc a = pred[s];

                if (ResCap(a)<Lambda) Lambda = ResCap(a);

                s = StartNode(a);
            }
            while (Demand(s)-Divergence(s) >= 0);

            TFloat defS = Demand(s)-Divergence(s);
            TFloat defT = Demand(t)-Divergence(t);

            if (defT<Lambda) Lambda = defT;
            if (-defS<Lambda) Lambda = -defS;

            Augment(pred,s,t,Lambda);

            UpdatePotentials(Dist(t));

            if (doScale)
            {
                Trace(1);
            }
            else
            {
                defPlus -= Lambda;

                Trace((unsigned long)Lambda);

                #if defined(_PROGRESS_)

                ProgressStep(1);

                #endif
            }
        }

        if (doScale) CloseFold();

        if (delta==0) break;

        delta = floor(delta/2);
    }

    delete nHeap;
    nHeap = NULL;
    delta = 0;

    for (TNode v=0;v<n;v++)
    {
        if (Demand(v)!=Divergence(v))
        {
            LogEntry(LOG_RES,"...Problem is infeasible");
            SetBounds(TimerMinCFlow,InfFloat,InfFloat);
            CloseFold(ModShortPath2);
            return InfFloat;
        }
    }

    TFloat w = Weight();
    SetBounds(TimerMinCFlow,MCF_DualObjective(),w);

    if (doScale)
    {
        CloseFold(ModMCFCapScaling);
    }
    else
    {
        CloseFold(ModShortPath2);
    }

    return w;
}


TFloat abstractDiGraph::MCF_NWSimplex() throw()
{
    OpenFold(ModNetworkSimplex,SHOW_TITLE);

    RawPotentials();

    #if defined(_PROGRESS_)

    InitProgressCounter(1);
    SetProgressNext(0);

    #endif

    // Compute basic solution
    MCF_NWSimplexCancelFree();

    // Set up basis structure
    MCF_NWSimplexStrongTree();

    networkSimplex NWSimplexData(*this);

    NWSimplexData.InitThreadIndex();
    NWSimplexData.ComputePotentials();

    SetBounds(TimerMinCFlow,MCF_DualObjective(),Weight());

    #if defined(_PROGRESS_)

    TFloat initialGap = UpperBound(TimerMinCFlow)-LowerBound(TimerMinCFlow);

    #endif

    // Main loop
    LogEntry(LOG_METH,"Pivoting...");
    Trace();

    TArc a = NWSimplexData.PivotArc();
    unsigned long count = 0;
    bool unbounded = false;

    while (CT.SolverRunning() && a!=NoArc)
    {
        unbounded = NWSimplexData.PivotOperation(a);

        static TArc i = 1;
        i++;

        if (i*100>m)
        {
            SetBounds(TimerMinCFlow,MCF_DualObjective(),Weight());

            #if defined(_PROGRESS_)

            SetProgressCounter(1 - (UpperBound(TimerMinCFlow)-LowerBound(TimerMinCFlow)) / initialGap);

            #endif

            Trace();
            i = 1;
        }

        if (unbounded) a = NoArc;
        else a = NWSimplexData.PivotArc();

        count++;
    }

    sprintf(CT.logBuffer,"...%ld pivots in total",count);
    LogEntry(LOG_METH,CT.logBuffer);

    TFloat w = -InfFloat;

    if (!unbounded) w = Weight();

    SetUpperBound(TimerMinCFlow,w);

    if (CT.SolverRunning() && !unbounded) SetLowerBound(TimerMinCFlow,w);
    else SetLowerBound(TimerMinCFlow,MCF_DualObjective());

    CloseFold(ModNetworkSimplex);

    return w;
}


void abstractDiGraph::MCF_NWSimplexCancelFree() throw()
{
    LogEntry(LOG_METH,"Cancelling free cycles...");
    OpenFold();

    TArc* pred = InitPredecessors();
    THandle H = Investigate();
    investigator &I = Investigator(H);
    unsigned long k = 0;

    for (TNode r=0;r<n;r++)
    {
        TNode v = r;

        while (v!=r || I.Active(v))
        {
            if (I.Active(v))
            {
                TArc a = I.Peek(v);
                TNode u = EndNode(a);

                if (ResCap(a)>0 && ResCap(a^1)>0 && pred[v]!=(a^1))
                {
                    if (pred[u]!=NoArc || u==r)
                    {
                        Trace();
                        TArc savedPu = pred[u];
                        pred[u] = a;
                        Trace();

                        TFloat length = 0;
                        TNode w = u;

                        while (true)
                        {
                            a = pred[w];

                            length += RedLength(NULL,a);
                            w = StartNode(a);
                            if (w==u) break;
                        }

                        if (length>0)
                        {
                            TArc a = pred[u];
                            TArc final = a;
                            w = StartNode(a);

                            while (pred[w]!=final)
                            {
                                TArc a2 = pred[w];
                                pred[w] = (a^1);
                                w = StartNode(a2);
                                a = a2;
                            }

                            pred[w] = (a^1);
                        }

                        TFloat Lambda = FindCap(pred,u,u);
                        Augment(pred,u,u,Lambda);
                        pred[u] = savedPu;
                        v = u;
                        k++;
                    }
                    else
                    {
                        pred[u] = a;
                        v = u;
                    }
                }
                else I.Skip(v);
            }
            else
            {
                TArc a = pred[v];
                pred[v] = NoArc;
                v = StartNode(a);
                I.Skip(v);
            }
        }
    }

    Close(H);
    ReleasePredecessors();

    CloseFold();
    sprintf(CT.logBuffer,"...%ld cycles eliminated",k);
    LogEntry(LOG_METH,CT.logBuffer);
}


void abstractDiGraph::MCF_NWSimplexStrongTree() throw()
{
    LogEntry(LOG_METH,"Computing strongly feasible spanning tree...");
    OpenFold();

    TNode nTotal = 2*n;
    nestedFamily<TNode> S(n,nTotal,CT);

    TArc* inArc = new TArc[nTotal];

    for (TNode v=0;v<nTotal;v++) inArc[v] = NoArc;

    staticQueue<TArc,TFloat> Q(2*m,CT);

    for (TArc a=0;a<2*m;a++)
        if (ResCap(a^1)>0 && ResCap(a)>0) Q.Insert(a);

    for (TArc a=0;a<2*m;a++)
        if (ResCap(a^1)>0 && ResCap(a)==0) Q.Insert(a);

    LogEntry(LOG_METH2,"Shrinking cycles...");

    OpenFold();

    TNode rank = 0;
    TNode nCycles = 0;
    TArc edgeCount = Q.Cardinality();

    while (true)
    {
        TArc  a = NoArc;
        TNode u = NoNode;
        TNode v = NoNode;

        while (!Q.Empty() && edgeCount>0)
        {
            a = Q.Delete();
            edgeCount--;

            u = S.Set(StartNode(a));
            v = S.Set(EndNode(a));

            if (u==v)
            {
                continue;
            }
            else if (inArc[v]==NoArc)
            {
                // 
                inArc[v] = a;

                #if defined(_LOGGING_)

                if (CT.logMeth>1)
                {
                    sprintf(CT.logBuffer,"inArc[%ld] = %ld",v,a);
                    LogEntry(LOG_METH2,CT.logBuffer);
                }

                #endif

                break;
            }
            else
            {
                // Arc a must be reconsidered after v is shrunk
                Q.Insert(a);
            }
        }

        if (v==NoNode || inArc[v]!=a) break;


        TNode w = u;

        while (w!=v && inArc[w]!=NoArc) w = S.Set(StartNode(inArc[w]));

        if (w==v)
        {
            #if defined(_LOGGING_)

            THandle LH = NoHandle;

            if (CT.logMeth>1)
            {
                sprintf(CT.logBuffer,"Merging cycle (%ld",u);
                LH = LogStart(LOG_METH2,CT.logBuffer);
            }

            #endif

            TNode cycle = S.MakeSet();
            S.Merge(cycle,u);
            w = u;

            while (w!=v)
            {
                w = S.Set(StartNode(inArc[w]));
                S.Merge(cycle,w);

                #if defined(_LOGGING_)

                if (CT.logMeth>1)
                {
                    sprintf(CT.logBuffer,",%ld",w);
                    LogAppend(LH,CT.logBuffer);
                }

                #endif
            }
            S.FixSet(cycle);

            #if defined(_LOGGING_)

            if (CT.logMeth>1)
            {
                sprintf(CT.logBuffer,",%ld)",u);
                LogEnd(LH,CT.logBuffer);
            }

            #endif

            nCycles++;

            // Reconsider all arcs in Q after shrinking this cycle
            edgeCount = Q.Cardinality();
        }
        else rank++;
    }

    CloseFold();

    LogEntry(LOG_METH2,"Expanding cycles...");

    OpenFold();

    for (TNode v=n+nCycles-1;v>=n;v--)
    {
        TArc a = inArc[v];
        if (a==NoArc)
        {
            TNode u = S.First(v);
            S.Split(v);

            TNode thisRoot = u;
            TNode w = S.Set(StartNode(inArc[u]));

            while (w!=u)
            {
                if (ResCap(inArc[w])==0)
                    thisRoot = w;
                w = S.Set(StartNode(inArc[w]));
            }

            inArc[thisRoot] = NoArc;

            #if defined(_LOGGING_)

            if (CT.logMeth>1 && IsDirected())
            {
                sprintf(CT.logBuffer,"inArc[%ld] = *",thisRoot);
                LogEntry(LOG_METH2,CT.logBuffer);
            }

            #endif
        }
        else
        {
            S.Split(v);

            TNode u = S.Set(EndNode(a));
            inArc[u] = a;

            #if defined(_LOGGING_)

            if (CT.logMeth>1)
            {
                sprintf(CT.logBuffer,"inArc[%ld] = %ld",u,a);
                LogEntry(LOG_METH2,CT.logBuffer);
            }

            #endif
        }
    }

    CloseFold();

    TArc* pred = InitPredecessors();

    for (TNode v=0;v<n;v++) pred[v] = inArc[v];

    delete[] inArc;

    CloseFold();

    if (CT.logRes && n>rank+1)
    {
        sprintf(CT.logBuffer,"...Network splits into %ld independent problems",n-rank);
        LogEntry(LOG_METH,CT.logBuffer);
    }
}
