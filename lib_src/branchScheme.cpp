
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, February 2001
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file   branchScheme.cpp
/// \brief  Implementation of a universal branch & bound solver

#include "branchScheme.h"


template <class TIndex,class TObj>
branchNode<TIndex,TObj>::branchNode(TIndex nn,goblinController &thisContext,
        branchScheme<TIndex,TObj> *thisScheme) throw() :
    managedObject(thisContext)
{
    n = unfixed = nn;
    depth = n;
    solved = false;
    index = NoNode;
    scheme = thisScheme;

    LogEntry(LOG_MEM,"...Branch node instanciated");
}


template <class TIndex,class TObj>
branchNode<TIndex,TObj>::~branchNode() throw()
{
    LogEntry(LOG_MEM,"...Branch node disallocated");
}


template <class TIndex,class TObj>
unsigned long branchNode<TIndex,TObj>::Allocated() const throw()
{
    return 0;
}


template <class TIndex,class TObj>
TObj branchNode<TIndex,TObj>::Objective() throw()
{
    if (!solved)
    {
        objective = SolveRelaxation();
        solved = true;
    }

    return objective;
}


#if defined(_TRACING_)

branchTree::branchTree(goblinController &thisContext) throw() :
    managedObject(thisContext),
    diGraph(TNode(0),thisContext)
{
    LogEntry(LOG_MAN,"Generating branch tree...");
}


void branchTree::ConfigDisplay(goblinController &CFG) const throw()
{
    CFG.predecessors = 0;
    CFG.arcLabels = 4;
    CFG.nodeColours = 2;
    CFG.subgraph = 2;
}

#endif


template <class TIndex,class TObj>
branchScheme<TIndex,TObj>::branchScheme(branchNode<TIndex,TObj>* root,
    TObj aPrioriBound,TSearchLevel thisLevel) throw() :
    managedObject(root->Context())
{
    LogEntry(LOG_METH,"Branching...");
    OpenFold(ModBranch);

    nIterations = 0;
    nActive = 0;
    maxActive = 0;
    nDFS = 0;
    depth = root->depth;
    firstActive = NULL;
    savedObjective = aPrioriBound;
    bestBound = root->Objective();

    sign = 1;
    TTimer thisTimer = root->TimerModule();

    if (root->ObjectSense()==MAXIMIZE)
    {
        sign = -1;

        SetLowerBound(thisTimer,savedObjective);

        if (bestBound>=savedObjective)
            SetUpperBound(root->TimerModule(),bestBound);
    }
    else
    {
        SetUpperBound(thisTimer,savedObjective);

        if (bestBound<=savedObjective)
            SetLowerBound(root->TimerModule(),bestBound);
    }

    feasible = !(savedObjective==root->Infeasibility());
    level = thisLevel;
    root->scheme = this;

    #if defined(_TRACING_)

    if (CT.traceLevel>1) Tree = new branchTree(CT);

    #endif

    LogEntry(LOG_MEM,"...B&B scheme instanciated");

    #if defined(_LOGGING_)

    if (CT.logMeth>1 && CT.logGaps==0)
    {
        LogEntry(LOG_METH2,"");
        LogEntry(LOG_METH2,"Iteration        Objective    Free  "
                  "Status      Saved Obj       Best Bound  Active  Select");
        LogEntry(LOG_METH2,"------------------------------------------------------------------------------------------");
    }

    #endif

    if (Inspect(root))
    {
         if (CT.logMeth>1 && CT.logGaps==0) LogEnd(LH,"  STOP");
         delete root;
    }
    else Optimize();

    if (CT.logMeth>1 && CT.logGaps==0) LogEntry(LOG_METH2,"");
    CloseFold(ModBranch);

    if (sign*bestBound<=sign*(savedObjective+CT.epsilon)-1 ||
        (sign*bestBound<=sign*(savedObjective+CT.epsilon) && !feasible && nActive>0))
    {
        LogEntry(LOG_METH,"...Interrupted!");
    }
    else
    {
        bestBound = savedObjective;

        if (sign==-1)
        {
            SetUpperBound(thisTimer,bestBound);
        }
        else
        {
            SetLowerBound(thisTimer,bestBound);
        }
    }

    if (CT.logMeth==1 || CT.logGaps>0)
    {
        sprintf(CT.logBuffer,"...Total number of branch nodes: %ld",nIterations);
        LogEntry(LOG_METH,CT.logBuffer);
    }

    #if defined(_TRACING_)

    if (CT.traceLevel==2 && nIterations>2)
    {
        Tree -> Layout_PredecessorTree();
        Tree -> Display();
    }

    #endif
}


template <class TIndex,class TObj>
branchScheme<TIndex,TObj>::~branchScheme() throw()
{
    branchNode<TIndex,TObj> *thisNode = firstActive;

    while (thisNode!=NULL)
    {
        branchNode<TIndex,TObj>* predNode = thisNode;
        thisNode = thisNode->succNode;
        delete predNode;
    }

    #if defined(_TRACING_)

    if (CT.traceLevel>1)
    {
        int savedLevel = CT.traceLevel;
        CT.traceLevel = 1;
        delete Tree;
        CT.traceLevel = savedLevel;
    }

    #endif

    LogEntry(LOG_MEM,"...B&B scheme disallocated");
}


template <class TIndex,class TObj>
unsigned long branchScheme<TIndex,TObj>::Size() const throw()
{
    return
          sizeof(branchScheme<TIndex,TObj>)
        + Allocated();
}


template <class TIndex,class TObj>
unsigned long branchScheme<TIndex,TObj>::Allocated() const throw()
{
    return 0;
}


template <class TIndex,class TObj>
void branchScheme<TIndex,TObj>::Optimize() throw()
{
    branchNode<TIndex,TObj> *activeNode = NULL;
    branchNode<TIndex,TObj> *leftChild = NULL;
    branchNode<TIndex,TObj> *rightChild = NULL;

    #if defined(_PROGRESS_)

    InitProgressCounter(1);

    double combEstimate = 0;

    #endif

    while (   CT.SolverRunning() && nActive>0
           && nActive<unsigned(CT.maxBBNodes)*100
           && (level!=SEARCH_FEASIBLE || !feasible)
           && (   sign*bestBound<=sign*savedObjective+CT.epsilon-1
               || (sign*bestBound<=sign*savedObjective+CT.epsilon && !feasible) )
           && (   CT.maxBBIterations<0
               || nIterations<unsigned(CT.maxBBIterations)*1000 )
          )
    {
        activeNode = SelectActiveNode();

        #if defined(_TRACING_)

        TNode activeIndex = activeNode->index;

        #endif

        TIndex i = activeNode->SelectVariable();

        typedef typename branchNode<TIndex,TObj>::TBranchDir TBranchDir;
        TBranchDir dir =
            activeNode->DirectionConstructive(i);

        if (feasible) dir = activeNode->DirectionExhaustive(i);

        leftChild = activeNode;
        rightChild = activeNode->Clone();

        rightChild -> Raise(i);
        leftChild -> Lower(i);

        bool deleteLeft = false;
        bool deleteRight = false;

        if (dir==branchNode<TIndex,TObj>::RAISE_FIRST)
        {
            deleteLeft = Inspect(leftChild);
            if (CT.logMeth>1 && CT.logGaps==0) LogEnd(LH);
            deleteRight = Inspect(rightChild);
        }
        else
        {
            deleteRight = Inspect(rightChild);
            if (CT.logMeth>1 && CT.logGaps==0) LogEnd(LH);
            deleteLeft = Inspect(leftChild);
        }

        if (leftChild->ObjectSense()==MAXIMIZE)
        {
            if (LowerBound(leftChild->TimerModule())<=bestBound)
                SetUpperBound(leftChild->TimerModule(),bestBound);
        }
        else
        {
            if (UpperBound(leftChild->TimerModule())>=bestBound)
                SetLowerBound(leftChild->TimerModule(),bestBound);
        }

        #if defined(_TRACING_)

        if (CT.traceLevel>1)
        {
            Tree->SetNodeColour(activeIndex,TNode(PROCESSED));

            TArc a = 2*(Tree->InsertArc(activeIndex,leftChild->index));
            Tree -> SetPred(leftChild->index,a);
            Tree->Representation() -> SetLength(a,i);

            a = 2*(Tree->InsertArc(activeIndex,rightChild->index));
            Tree -> SetPred(rightChild->index,a);
            Tree->Representation() -> SetLength(a,i);

            if (CT.traceLevel==3 && nIterations>1)
            {
                Tree->Layout_PredecessorTree();
            }
        }

        #endif

        if (deleteLeft) delete leftChild;
        if (deleteRight) delete rightChild;

        #if defined(_PROGRESS_)

        double thisEstimate =
            sqrt((nIterations+1-2*nActive)/(nIterations+1.0));

        // Filter the progress counter used for display
        combEstimate = 0.85*combEstimate + 0.15*thisEstimate;
        double thisEstimate2 =
                combEstimate*combEstimate*combEstimate*combEstimate;

        if (CT.maxBBIterations>0)
        {
            // At least, in the DFS case, a different estimation is required
            double thisEstimate3 = nIterations/(CT.maxBBIterations*1000.0);

            if (thisEstimate2<thisEstimate3)
            {
                thisEstimate2 = thisEstimate3;
            }
        }

        SetProgressCounter(thisEstimate2);

        #endif
    }

    if (CT.logMeth>1 && CT.logGaps==0) LogEnd(LH,"  STOP");
}


template <class TIndex,class TObj>
bool branchScheme<TIndex,TObj>::Inspect(branchNode<TIndex,TObj> *thisNode)
    throw()
{
    TObj thisObjective = thisNode->Objective();

    #if defined(_TRACING_)

    if (CT.traceLevel>1) thisNode->index = Tree->InsertNode();

    #endif

    #if defined(_LOGGING_)

    if (CT.logMeth>=2 && CT.logGaps==0)
    {
        sprintf(CT.logBuffer,"%9.1ld  ",nIterations);
        LH = LogStart(LOG_METH2,CT.logBuffer);

        if (thisObjective!=thisNode->Infeasibility())
        {
            sprintf(CT.logBuffer,"%15.10g",thisObjective);
            LogAppend(LH,CT.logBuffer);
        }
        else LogAppend(LH,"     INFEASIBLE");

        sprintf(CT.logBuffer,"  %6.1ld  ",thisNode->Unfixed());
        LogAppend(LH,CT.logBuffer);
    }

    #endif

    bool ret = false;

    if (sign*thisObjective<=sign*savedObjective+CT.epsilon-1 ||
        (sign*thisObjective<=sign*savedObjective+CT.epsilon &&
            thisObjective!=thisNode->Infeasibility() && !feasible))
    {
        if (thisNode->Feasible())
        {
            #if defined(_LOGGING_)

            if (CT.logMeth>=2 && CT.logGaps==0) LogAppend(LH,"SAVED ");

            #endif

            #if defined(_TRACING_)

            if (CT.traceLevel>1) Tree -> SetNodeColour(thisNode->index,TNode(SAVED));

            #endif

            feasible = true;
            savedObjective = thisNode->LocalSearch();

            if (thisNode->ObjectSense()==MAXIMIZE)
            {
                SetLowerBound(thisNode->TimerModule(),savedObjective);
            }
            else
            {
                SetUpperBound(thisNode->TimerModule(),savedObjective);
            }

            thisNode -> SaveSolution();
            StripQueue();
            ret = true;
            nDFS = 0;
        }
        else
        {
            QueueExploredNode(thisNode);

            #if defined(_LOGGING_)

            if (CT.logMeth>=2 && CT.logGaps==0) LogAppend(LH,"QUEUED");

            #endif

            #if defined(_TRACING_)

            if (CT.traceLevel>1) Tree -> SetNodeColour(thisNode->index,TNode(QUEUED));

            #endif
        }
    }
    else
    {
        ret = true;

        #if defined(_LOGGING_)

        if (CT.logMeth>=2 && CT.logGaps==0) LogAppend(LH,"CUTOFF");

        #endif

        #if defined(_TRACING_)

        if (CT.traceLevel>1) Tree -> SetNodeColour(thisNode->index,TNode(CUTOFF));

        #endif
    }

    #if defined(_LOGGING_)

    if (CT.logMeth>=2 && CT.logGaps==0)
    {
        if (savedObjective==thisNode->Infeasibility())
            sprintf(CT.logBuffer,"       UNSOLVED");
        else sprintf(CT.logBuffer,"%15.10g",savedObjective);

        if (bestBound==thisNode->Infeasibility())
            sprintf(CT.logBuffer,"%s       INFEASIBLE",CT.logBuffer);
        else sprintf(CT.logBuffer,"%s  %15.10g",CT.logBuffer,bestBound);

        sprintf(CT.logBuffer,"%s  %6.1ld",CT.logBuffer,nActive);
        LogAppend(LH,CT.logBuffer);
    }

    #endif

    nIterations++;

    return ret;
}


template <class TIndex,class TObj>
void branchScheme<TIndex,TObj>::QueueExploredNode(branchNode<TIndex,TObj> *thisNode)
    throw()
{
    thisNode->succNode = firstActive;
    firstActive = thisNode;
    nActive++;

    if (nActive>maxActive) maxActive = nActive;

    if (sign*(thisNode->Objective())<sign*bestBound)
    {
        bestBound = thisNode->Objective();
    }
}


template <class TIndex,class TObj>
typename branchScheme<TIndex,TObj>::TSearchState branchScheme<TIndex,TObj>::SearchState() throw()
{
    if (nIterations<5*depth && level!=SEARCH_EXHAUSTIVE)
    {
        // Start branching with a DFS strategy for
        // quick construction of feasible solutions.
        return INITIAL_DFS;
    }

    if (nActive+depth+2>TIndex(CT.maxBBNodes)*100)
    {
        // When reaching the possible memory usage,
        // restrict nActive by switch to DFS again.
        return EXHAUSTIVE_DFS;
    }

    if (   level!=SEARCH_EXHAUSTIVE && depth>0
        && nIterations%(depth/2)>depth/20 )
    {
        // During constructive search, it is useful to
        // apply DFS. Sometimes, interrupt the DFS and
        // restart it from another subproblem which
        // is then determined by a best first strategy.
        return EXHAUSTIVE_DFS;
    }

    if (   level==SEARCH_EXHAUSTIVE
        || 3*nActive>2*TIndex(CT.maxBBNodes)*100 )
    {
        // Apply best first search. When getting close
        // the possible memory usage, or during exhaustive
        // search (required for optimality proofs)
        // determine the best possible lower bounds.
        return EXHAUSTIVE_BFS;
    }

    // Apply best first search and and any lower
    // bounding procedure which is convenient
    return CONSTRUCT_BFS;
}


template <class TIndex,class TObj>
branchNode<TIndex,TObj> *branchScheme<TIndex,TObj>::SelectActiveNode() throw()
{
    branchNode<TIndex,TObj> *retNode = firstActive;

    if (SearchState()!=CONSTRUCT_BFS && SearchState()!=EXHAUSTIVE_BFS)
    {
        firstActive = firstActive->succNode;
        nDFS++;

        #if defined(_LOGGING_)

        if (CT.logMeth>1 && CT.logGaps==0) LogEnd(LH,"  DEPTH");

        #endif
    }
    else
    {
        branchNode<TIndex,TObj> *thisNode = firstActive;
        branchNode<TIndex,TObj> *bestPredNode = NULL;
        bestBound = firstActive->Objective();
        nDFS = 0;

        while (thisNode)
        {
            branchNode<TIndex,TObj> *predNode = thisNode;
            thisNode = thisNode->succNode;

            if (thisNode && sign*(thisNode->Objective())<sign*bestBound)
            {
                bestPredNode = predNode;
                bestBound = thisNode->Objective();
            }
        }

        if (!bestPredNode) firstActive = firstActive->succNode;
        else
        {
            retNode = bestPredNode->succNode;
            bestPredNode->succNode = retNode->succNode;
        }

        #if defined(_LOGGING_)

        if (CT.logMeth>1 && CT.logGaps==0) LogEnd(LH,"  BEST");

        #endif
    }

    bestBound = savedObjective;
    branchNode<TIndex,TObj> *thisNode = firstActive;

    while (thisNode)
    {
        if (sign*(thisNode->Objective())<sign*bestBound)
            bestBound = thisNode->Objective();

        thisNode = thisNode->succNode;
    }

    nActive--;

    return retNode;
}


template <class TIndex,class TObj>
void branchScheme<TIndex,TObj>::StripQueue() throw()
{
    branchNode<TIndex,TObj> *thisNode = firstActive;

    while (thisNode)
    {
        branchNode<TIndex,TObj> *predNode = thisNode;
        thisNode = thisNode->succNode;

        if (thisNode &&
            sign*(thisNode->Objective())>sign*savedObjective+CT.epsilon-1
           )
        {
            predNode->succNode = thisNode->succNode;
            delete thisNode;
            thisNode = predNode->succNode;
            nActive--;
        }
    }
}


#ifndef DO_NOT_DOCUMENT_THIS

template class branchNode<TNode,TFloat>;
template class branchScheme<TNode,TFloat>;

#ifdef _BIG_ARCS_

template class branchNode<TArc,TFloat>;
template class branchScheme<TArc,TFloat>;

#endif // _BIG_ARCS_

#endif // DO_NOT_DOCUMENT_THIS
