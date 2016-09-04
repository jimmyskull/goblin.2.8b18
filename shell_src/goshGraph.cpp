
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, March 2001
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file   goshGraph.cpp
/// \brief  Tcl shell command procedures for the GOBLIN library graph objects

#include "gosh.h"


int Goblin_Mixed_Graph_Cmd (ClientData clientData,Tcl_Interp* interp,
    int argc,_CONST_QUAL_ char* argv[])
{
    mixedGraph* G = (mixedGraph*)clientData;

    if (setjmp(goblinThreadData[Goblin_MyThreadIndex()].jumpBuffer) != 0)
    {
        return Goblin_Propagate_Exception(interp);
    }
    else try
    {
        int ret = Goblin_Sparse_Cmd(G,interp,argc,argv);

        if (ret==GOSH_OK)    return TCL_OK;
        if (ret==GOSH_ERROR) return TCL_ERROR;

        return Goblin_Generic_Graph_Cmd(G,interp,argc,argv);
    }
    catch (...)
    {
        return Goblin_Propagate_Exception(interp);
    }
}


int Goblin_Delete_Mixed_Graph (ClientData clientData) throw()
{
    mixedGraph* thisObject = (mixedGraph*)clientData;
    goblinController* thisContext = &(thisObject->Context());

    if (destroyThread) return TCL_OK;

    if (thisContext==CT)
    {
        int savedTraceLevel = thisContext->traceLevel;
        thisContext->traceLevel = 0;
        delete thisObject;
        thisContext->traceLevel = savedTraceLevel;
    }
    else
    {
        thisContext->traceLevel = 0;
        delete thisObject;
        if (!thisContext->IsReferenced()) delete thisContext;
    }

    return TCL_OK;
}


int Goblin_Sparse_Graph_Cmd (ClientData clientData,Tcl_Interp* interp,
    int argc,_CONST_QUAL_ char* argv[])
{
    graph* G = (graph*)clientData;

    if (setjmp(goblinThreadData[Goblin_MyThreadIndex()].jumpBuffer) != 0)
    {
        return Goblin_Propagate_Exception(interp);
    }
    else try
    {
        int ret = Goblin_Sparse_Cmd(G,interp,argc,argv);

        if (ret==GOSH_OK)    return TCL_OK;
        if (ret==GOSH_ERROR) return TCL_ERROR;

        return Goblin_Undirected_Cmd(G,interp,argc,argv);
    }
    catch (...)
    {
        return Goblin_Propagate_Exception(interp);
    }
}


int Goblin_Delete_Sparse_Graph (ClientData clientData) throw()
{
    graph* thisObject = (graph*)clientData;
    goblinController* thisContext = &(thisObject->Context());

    if (destroyThread) return TCL_OK;

    if (thisContext==CT)
    {
        int savedTraceLevel = thisContext->traceLevel;
        thisContext->traceLevel = 0;
        delete thisObject;
        thisContext->traceLevel = savedTraceLevel;
    }
    else
    {
        thisContext->traceLevel = 0;
        delete thisObject;
        if (!thisContext->IsReferenced()) delete thisContext;
    }

    return TCL_OK;
}


int Goblin_Dense_Graph_Cmd (ClientData clientData,Tcl_Interp* interp,
    int argc,_CONST_QUAL_ char* argv[])
{
    denseGraph *G = (denseGraph *)clientData;

    if (setjmp(goblinThreadData[Goblin_MyThreadIndex()].jumpBuffer) != 0)
    {
        return Goblin_Propagate_Exception(interp);
    }
    else try
    {
        return Goblin_Undirected_Cmd(G,interp,argc,argv);
    }
    catch (...)
    {
        return Goblin_Propagate_Exception(interp);
    }
}


int Goblin_Delete_Dense_Graph (ClientData clientData) throw()
{
    denseGraph* thisObject = (denseGraph*)clientData;
    goblinController* thisContext = &(thisObject->Context());

    if (destroyThread) return TCL_OK;

    if (thisContext==CT)
    {
        int savedTraceLevel = thisContext->traceLevel;
        thisContext->traceLevel = 0;
        delete thisObject;
        thisContext->traceLevel = savedTraceLevel;
    }
    else
    {
        thisContext->traceLevel = 0;
        delete thisObject;
        if (!thisContext->IsReferenced()) delete thisContext;
    }

    return TCL_OK;
}


int Goblin_Sparse_Bigraph_Cmd (ClientData clientData,Tcl_Interp* interp,
    int argc,_CONST_QUAL_ char* argv[])
{
    biGraph *G = (biGraph *)clientData;

    if (setjmp(goblinThreadData[Goblin_MyThreadIndex()].jumpBuffer) != 0)
    {
        return Goblin_Propagate_Exception(interp);
    }
    else try
    {
        if (argc==4 && strcmp(argv[1],"node")==0 &&
            strcmp(argv[3],"swap")==0)
        {
            TNode u = atol(argv[2]);
            TNode v = G -> SwapNode(u);
            sprintf(interp->result,"%ld",v);
            return TCL_OK;
        }

        int ret = Goblin_Sparse_Cmd(G,interp,argc,argv);

        if (ret==GOSH_OK)    return TCL_OK;
        if (ret==GOSH_ERROR) return TCL_ERROR;

        return Goblin_Bipartite_Cmd(G,interp,argc,argv);
    }
    catch (...)
    {
        return Goblin_Propagate_Exception(interp);
    }
}


int Goblin_Delete_Sparse_Bigraph (ClientData clientData) throw()
{
    biGraph* thisObject = (biGraph*)clientData;
    goblinController* thisContext = &(thisObject->Context());

    if (destroyThread) return TCL_OK;

    if (thisContext==CT)
    {
        int savedTraceLevel = thisContext->traceLevel;
        thisContext->traceLevel = 0;
        delete thisObject;
        thisContext->traceLevel = savedTraceLevel;
    }
    else
    {
        thisContext->traceLevel = 0;
        delete thisObject;
        if (!thisContext->IsReferenced()) delete thisContext;
    }

    return TCL_OK;
}


int Goblin_Dense_Bigraph_Cmd (ClientData clientData,Tcl_Interp* interp,
    int argc,_CONST_QUAL_ char* argv[])
{
    denseBiGraph *G = (denseBiGraph *)clientData;

    if (setjmp(goblinThreadData[Goblin_MyThreadIndex()].jumpBuffer) != 0)
    {
        return Goblin_Propagate_Exception(interp);
    }
    else try
    {
        return Goblin_Bipartite_Cmd(G,interp,argc,argv);
    }
    catch (...)
    {
        return Goblin_Propagate_Exception(interp);
    }
}


int Goblin_Delete_Dense_Bigraph (ClientData clientData) throw()
{
    denseBiGraph* thisObject = (denseBiGraph*)clientData;
    goblinController* thisContext = &(thisObject->Context());

    if (destroyThread) return TCL_OK;

    if (thisContext==CT)
    {
        int savedTraceLevel = thisContext->traceLevel;
        thisContext->traceLevel = 0;
        delete thisObject;
        thisContext->traceLevel = savedTraceLevel;
    }
    else
    {
        thisContext->traceLevel = 0;
        delete thisObject;
        if (!thisContext->IsReferenced()) delete thisContext;
    }

    return TCL_OK;
}


int Goblin_Sparse_Digraph_Cmd (ClientData clientData,Tcl_Interp* interp,
    int argc,_CONST_QUAL_ char* argv[])
{
    diGraph *G = (diGraph *)clientData;

    if (setjmp(goblinThreadData[Goblin_MyThreadIndex()].jumpBuffer) != 0)
    {
        return Goblin_Propagate_Exception(interp);
    }
    else try
    {
        int ret = Goblin_Sparse_Cmd(G,interp,argc,argv);

        if (ret==GOSH_OK)    return TCL_OK;
        if (ret==GOSH_ERROR) return TCL_ERROR;

        return Goblin_Directed_Cmd(G,interp,argc,argv);
    }
    catch (...)
    {
        return Goblin_Propagate_Exception(interp);
    }
}


int Goblin_Delete_Sparse_Digraph (ClientData clientData) throw()
{
    diGraph* thisObject = (diGraph*)clientData;
    goblinController* thisContext = &(thisObject->Context());

    if (destroyThread) return TCL_OK;

    if (thisContext==CT)
    {
        int savedTraceLevel = thisContext->traceLevel;
        thisContext->traceLevel = 0;
        delete thisObject;
        thisContext->traceLevel = savedTraceLevel;
    }
    else
    {
        thisContext->traceLevel = 0;
        delete thisObject;
        if (!thisContext->IsReferenced()) delete thisContext;
    }

    return TCL_OK;
}


int Goblin_Dense_Digraph_Cmd (ClientData clientData,Tcl_Interp* interp,
    int argc,_CONST_QUAL_ char* argv[])
{
    denseDiGraph *G = (denseDiGraph *)clientData;

    if (setjmp(goblinThreadData[Goblin_MyThreadIndex()].jumpBuffer) != 0)
    {
        return Goblin_Propagate_Exception(interp);
    }
    else try
    {
        return Goblin_Directed_Cmd(G,interp,argc,argv);
    }
    catch (...)
    {
        return Goblin_Propagate_Exception(interp);
    }
}


int Goblin_Delete_Dense_Digraph (ClientData clientData) throw()
{
    denseDiGraph* thisObject = (denseDiGraph*)clientData;
    goblinController* thisContext = &(thisObject->Context());

    if (destroyThread) return TCL_OK;

    if (thisContext==CT)
    {
        int savedTraceLevel = thisContext->traceLevel;
        thisContext->traceLevel = 0;
        delete thisObject;
        thisContext->traceLevel = savedTraceLevel;
    }
    else
    {
        thisContext->traceLevel = 0;
        delete thisObject;
        if (!thisContext->IsReferenced()) delete thisContext;
    }

    return TCL_OK;
}


int Goblin_Balanced_FNW_Cmd (ClientData clientData,Tcl_Interp* interp,
    int argc,_CONST_QUAL_ char* argv[])
{
    balancedFNW *G = (balancedFNW *)clientData;

    if (setjmp(goblinThreadData[Goblin_MyThreadIndex()].jumpBuffer) != 0)
    {
        return Goblin_Propagate_Exception(interp);
    }
    else try
    {
        if (strcmp(argv[1],"maxbalflow")==0)
        {
            if (argc!=3)
            {
                WrongNumberOfArguments(interp,argc,argv);
                return TCL_ERROR;
            }

            TNode s = atol(argv[2]);
            G -> MaxBalFlow(s);

            try
            {
                sprintf(interp->result,"%f",G->FlowValue(s,s^1));
                return TCL_OK;
            }
            catch (ERCheck)
            {
                sprintf(interp->result,"Flow is corrupted");
                return TCL_ERROR;
            }
        }

        if (strcmp(argv[1],"mincbalflow")==0)
        {
            if (argc!=3)
            {
                WrongNumberOfArguments(interp,argc,argv);
                return TCL_ERROR;
            }

            TNode s = atol(argv[2]);
            TFloat ret = G -> MinCBalFlow(s);

            try
            {
                G -> FlowValue(s,s^1);
                sprintf(interp->result,"%f",ret);
                return TCL_OK;
            }
            catch (ERCheck)
            {
                sprintf(interp->result,"Flow is corrupted");
                return TCL_ERROR;
            }
        }

        int ret = Goblin_Sparse_Cmd(G,interp,argc,argv);

        if (ret==GOSH_OK)    return TCL_OK;
        if (ret==GOSH_ERROR) return TCL_ERROR;

        return Goblin_Directed_Cmd(G,interp,argc,argv);
    }
    catch (...)
    {
        return Goblin_Propagate_Exception(interp);
    }
}


int Goblin_Delete_Balanced_FNW (ClientData clientData) throw()
{
    balancedFNW* thisObject = (balancedFNW*)clientData;
    goblinController* thisContext = &(thisObject->Context());

    if (destroyThread) return TCL_OK;

    if (thisContext==CT)
    {
        int savedTraceLevel = thisContext->traceLevel;
        thisContext->traceLevel = 0;
        delete thisObject;
        thisContext->traceLevel = savedTraceLevel;
    }
    else
    {
        thisContext->traceLevel = 0;
        delete thisObject;
        if (!thisContext->IsReferenced()) delete thisContext;
    }

    return TCL_OK;
}


int Goblin_Sparse_Cmd (abstractMixedGraph *G,Tcl_Interp* interp,int argc,
    _CONST_QUAL_ char* argv[]) throw(ERRejected,ERRange)
{
    if (argc==3 && strcmp(argv[1],"incidences")==0)
    {
        if (strcmp(argv[2],"planarize")==0)
        {
            if (G->PlanarizeIncidenceOrder())
                interp->result = "1";
            else interp->result = "0";
            return GOSH_OK;
        }

        if (strcmp(argv[2],"randomize")==0)
        {
            G -> RandomizeIncidenceOrder();
            interp->result = "";
            return GOSH_OK;
        }

        if (strcmp(argv[2],"geometric")==0)
        {
            G -> IncidenceOrderFromDrawing();
            interp->result = "";
            return GOSH_OK;
        }

        if (strcmp(argv[2],"outerplanar")==0)
        {
            G -> GrowExteriorFace();
            interp->result = "";
            return GOSH_OK;
        }

        return GOSH_UNHANDLED;
    }

    if (strcmp(argv[1],"series-parallel")==0)
    {
        unsigned options = G->ESP_DIRECTED;
        TNode sourceNode = NoNode;
        TNode targetNode = NoNode;

        if (CT->FindParam(argc,argv,"-embedding",2)) options |= G->ESP_EMBEDDING;

        if (CT->FindParam(argc,argv,"-undirected",2) || G->IsUndirected())
        {
            options ^= G->ESP_DIRECTED;
        }

        if (CT->FindParam(argc,argv,"-orient",2)) options |= G->ESP_ORIENT;

        if (CT->FindParam(argc,argv,"-layout",2)) options |= G->ESP_VISIBILITY;

        if (CT->FindParam(argc,argv,"-minor",2)) options |= G->ESP_MINOR;

        int pos = CT->FindParam(argc,&argv[0],"-source",2);
        if (pos>0 && pos<argc-1 && strcmp(argv[pos+1],"*")!=0)
        {
            sourceNode = atol(argv[pos+1]);
        }

        pos = CT->FindParam(argc,&argv[0],"-target",2);
        if (pos>0 && pos<argc-1 && strcmp(argv[pos+1],"*")!=0)
        {
            targetNode = atol(argv[pos+1]);
        }

        if (G->EdgeSeriesParallelMethod(abstractMixedGraph::TOptSeriesParallel(options),
                    sourceNode,targetNode)
           )
        {
            interp->result = "";
            return GOSH_OK;
        }
        else
        {
            interp->result = "Graph is not edge series parallel";
            return GOSH_ERROR;
        }
    }

    if (argc==3 && strcmp(argv[1],"exterior")==0)
    {
        TArc a = atol(argv[2]);
        G -> MarkExteriorFace(a);
        interp->result = "";
        return GOSH_OK;
    }

    return GOSH_UNHANDLED;
}


int Goblin_Bipartite_Cmd (abstractBiGraph *G,Tcl_Interp* interp,int argc,
    _CONST_QUAL_ char* argv[]) throw(ERRejected,ERRange)
{
    if (strcmp(argv[1],"#outer")==0)
    {
        if (argc!=2)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        sprintf(interp->result,"%ld",G->N1());
        return TCL_OK;
    }

    if (strcmp(argv[1],"#inner")==0)
    {
        if (argc!=2)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        sprintf(interp->result,"%ld",G->N2());
        return TCL_OK;
    }

    return Goblin_Undirected_Cmd(G,interp,argc,argv);
}


int Goblin_Undirected_Cmd (abstractGraph *G,Tcl_Interp* interp,int argc,
    _CONST_QUAL_ char* argv[]) throw(ERRejected,ERRange)
{
    if (argc<2)
    {
        interp->result = "Missing arguments";
        return TCL_ERROR;
    }

    if (strcmp(argv[1],"subgraph")==0)
    {
        if (argc!=3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        graph *H = new graph(*G,
            managedObject::OPT_SUB|managedObject::OPT_PARALLELS);

        Tcl_CreateCommand(interp,const_cast<char*>(argv[2]),
            Goblin_Sparse_Graph_Cmd,(ClientData)H,
            (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Graph);

        interp->result = "";
        return TCL_OK;
    }

    if (strcmp(argv[1],"metricGraph")==0)
    {
        if (argc!=3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        denseGraph *H = new metricGraph(*G);

        Tcl_CreateCommand(interp,const_cast<char*>(argv[2]),
            Goblin_Dense_Graph_Cmd,(ClientData)H,
            (Tcl_CmdDeleteProc *)Goblin_Delete_Dense_Graph);

        interp->result = "";
        return TCL_OK;
    }

    if (strcmp(argv[1],"tiling")==0)
    {
        if (argc!=5)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        TNode x = atol(argv[3]);
        TNode y = atol(argv[4]);
        graph *H = new tiling(*G,(TOption)0,x,y);

        Tcl_CreateCommand(interp,const_cast<char*>(argv[2]),
            Goblin_Sparse_Graph_Cmd,(ClientData)H,
            (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Graph);

        interp->result = "";
        return TCL_OK;
    }

    if (strcmp(argv[1],"maxmatch")==0)
    {
        if (argc!=2)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        TFloat ret = G -> MaximumMatching();

        sprintf(interp->result,"%f",ret);
        return TCL_OK;
    }

    if (strcmp(argv[1],"mincmatch")==0)
    {
        if (argc!=2)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        if (G -> MinCMatching())
        {
            sprintf(interp->result,"%f",G->Weight());
            return TCL_OK;
        }
        else
        {
            interp->result = "No such structure exists";
            return TCL_ERROR;
        }
    }

    if (strcmp(argv[1],"edgecover")==0)
    {
        if (argc!=2)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        TFloat ret = G -> MinCEdgeCover();

        sprintf(interp->result,"%f",ret);
        return TCL_OK;
    }


    if (strcmp(argv[1],"tjoin")==0)
    {
        if (argc!=2)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        try
        {
            G -> MinCTJoin(demandNodes(*G));
            sprintf(interp->result,"%f",G->Weight());
            return TCL_OK;
        }
        catch (ERRejected)
        {
            interp->result = "No such structure exists";
            return TCL_ERROR;
        }
    }

    return Goblin_Generic_Graph_Cmd(G,interp,argc,argv);
}


int Goblin_Directed_Cmd (abstractDiGraph *G,Tcl_Interp* interp,int argc,
    _CONST_QUAL_ char* argv[]) throw(ERRejected,ERRange)
{
    if (argc<2)
    {
        interp->result = "Missing arguments";
        return TCL_ERROR;
    }

    if (strcmp(argv[1],"subgraph")==0)
    {
        if (argc!=3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        diGraph *H = new diGraph(*G,
            managedObject::OPT_SUB|managedObject::OPT_PARALLELS);

        Tcl_CreateCommand(interp,const_cast<char*>(argv[2]),
            Goblin_Sparse_Digraph_Cmd,(ClientData)H,
            (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Digraph);

        interp->result = "";
        return TCL_OK;
    }

    if (strcmp(argv[1],"transitive")==0)
    {
        if (argc!=3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        diGraph *H = new transitiveClosure(*G,managedObject::OPT_SUB);

        Tcl_CreateCommand(interp,const_cast<char*>(argv[2]),
            Goblin_Sparse_Digraph_Cmd,(ClientData)H,
            (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Digraph);

        interp->result = "";
        return TCL_OK;
    }

    if (strcmp(argv[1],"intransitive")==0)
    {
        if (argc!=3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        diGraph *H = new intransitiveReduction(*G,managedObject::OPT_SUB);

        Tcl_CreateCommand(interp,const_cast<char*>(argv[2]),
            Goblin_Sparse_Digraph_Cmd,(ClientData)H,
            (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Digraph);

        interp->result = "";
        return TCL_OK;
    }

    if (strcmp(argv[1],"lpflow")==0)
    {
        if (argc!=3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        goblinILPWrapper* XLP = static_cast<goblinILPWrapper*>(G->BFlowToLP());

        Tcl_CreateCommand(interp,const_cast<char*>(argv[2]),
            Goblin_Ilp_Cmd,(ClientData)XLP,
            (Tcl_CmdDeleteProc *)Goblin_Delete_Ilp);

        interp->result = "";
        return TCL_OK;
    }

    if (strcmp(argv[1],"splitgraph")==0)
    {
        if (argc!=5)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        TNode s = atol(argv[3]);
        TNode t = atol(argv[4]);
        balancedFNW *H = new splitGraph(*G,s,t);

        Tcl_CreateCommand(interp,const_cast<char*>(argv[2]),
            Goblin_Balanced_FNW_Cmd,(ClientData)H,
            (Tcl_CmdDeleteProc *)Goblin_Delete_Balanced_FNW);

        interp->result = "";
        return TCL_OK;
    }

    if (strcmp(argv[1],"topsort")==0)
    {
        if (argc!=2)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        TNode v = G->TopSort();

        if (v==NoNode)
        {
            interp->result = "*";
            return TCL_OK;
        }

        sprintf(interp->result,"%ld",v);
        return TCL_OK;
    }

    if (strcmp(argv[1],"critical")==0)
    {
        if (argc!=2)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        TNode v = G->CriticalPath();

        if (v==NoNode)
        {
            interp->result = "Graph is not a DAG";
            return TCL_ERROR;
        }

        sprintf(interp->result,"%ld",v);
        return TCL_OK;
    }

    if (strcmp(argv[1],"treepacking")==0)
    {
        if (argc!=3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        TNode v = NoNode;
        if (argc==3) v = atol(argv[2]);

        TCap ret = G->TreePacking(v);
        sprintf(interp->result,"%f",ret);
        return TCL_OK;
    }

    return Goblin_Generic_Graph_Cmd(G,interp,argc,argv);
}


int Goblin_Generic_Graph_Cmd (abstractMixedGraph *G,Tcl_Interp* interp,int argc,
    _CONST_QUAL_ char* argv[]) throw(ERRejected,ERRange)
{
    if (argc<2)
    {
        interp->result = "Missing arguments";
        return TCL_ERROR;
    }

    if (strcmp(argv[1],"linegraph")==0)
    {
        if (argc>4)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        if (CT->FindParam(argc,argv,"-planar",2))
        {
            graph *H = new planarLineGraph(*G);

            Tcl_CreateCommand(interp,const_cast<char*>(argv[argc-1]),
                Goblin_Sparse_Graph_Cmd,(ClientData)H,
                (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Graph);
        }
        else if (CT->FindParam(argc,argv,"-directed",2))
        {
            mixedGraph *H = new lineGraph(*G,lineGraph::LG_DIRECTED);

            Tcl_CreateCommand(interp,const_cast<char*>(argv[argc-1]),
                Goblin_Mixed_Graph_Cmd,(ClientData)H,
                (Tcl_CmdDeleteProc *)Goblin_Delete_Mixed_Graph);
        }
        else
        {
            mixedGraph *H = new lineGraph(*G,lineGraph::LG_UNDIRECTED);

            Tcl_CreateCommand(interp,const_cast<char*>(argv[argc-1]),
                Goblin_Mixed_Graph_Cmd,(ClientData)H,
                (Tcl_CmdDeleteProc *)Goblin_Delete_Mixed_Graph);
        }

        interp->result = "";
        return TCL_OK;
    }

    if (strcmp(argv[1],"dualgraph")==0)
    {
        if (argc>4)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        if (argc==3 || strcmp(argv[2],"-directed")!=0)
        {
            graph *H = new dualGraph(*G,(TOption)0);

            Tcl_CreateCommand(interp,const_cast<char*>(argv[argc-1]),
                Goblin_Sparse_Graph_Cmd,(ClientData)H,
                (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Graph);
        }
        else
        {
            diGraph *H = new directedDual(*G,(TOption)0);

            Tcl_CreateCommand(interp,const_cast<char*>(argv[argc-1]),
                Goblin_Sparse_Digraph_Cmd,(ClientData)H,
                (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Digraph);
        }

        interp->result = "";
        return TCL_OK;
    }

    if (strcmp(argv[1],"spread")==0)
    {
        if (argc!=3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        graph *H = new spreadOutRegular(*G,(TOption)0);

        Tcl_CreateCommand(interp,const_cast<char*>(argv[2]),
            Goblin_Sparse_Graph_Cmd,(ClientData)H,
            (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Graph);

        interp->result = "";
        return TCL_OK;
    }

    if (strcmp(argv[1],"truncate")==0)
    {
        if (argc!=3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        graph *H = new vertexTruncation(*G,(TOption)0);

        Tcl_CreateCommand(interp,const_cast<char*>(argv[2]),
            Goblin_Sparse_Graph_Cmd,(ClientData)H,
            (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Graph);

        interp->result = "";
        return TCL_OK;
    }

    if (strcmp(argv[1],"separate")==0)
    {
        if (argc!=3 && argc!=4)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        graph* H = NULL;

        if (argc==4 && strcmp(argv[2],"-turnLeft")==0)
        {
            H = new facetSeparation(*G,facetSeparation::ROT_LEFT);
        }
        else if (argc==4 && strcmp(argv[2],"-turnRight")==0)
        {
            H = new facetSeparation(*G,facetSeparation::ROT_RIGHT);
        }
        else
        {
            H = new facetSeparation(*G,facetSeparation::ROT_NONE);
        }

        Tcl_CreateCommand(interp,const_cast<char*>(argv[argc-1]),
            Goblin_Sparse_Graph_Cmd,(ClientData)H,
            (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Graph);

        interp->result = "";
        return TCL_OK;
    }

    if (strcmp(argv[1],"complement")==0)
    {
        if (argc!=3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        graph *H = new complementaryGraph(*G,(TOption)0);

        Tcl_CreateCommand(interp,const_cast<char*>(argv[2]),
            Goblin_Sparse_Graph_Cmd,(ClientData)H,
            (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Graph);

        interp->result = "";
        return TCL_OK;
    }

    if (strcmp(argv[1],"underlying")==0)
    {
        if (argc!=3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        graph *H = new graph(*G);

        Tcl_CreateCommand(interp,const_cast<char*>(argv[2]),
            Goblin_Sparse_Graph_Cmd,(ClientData)H,
            (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Graph);

        interp->result = "";
        return TCL_OK;
    }

    if (strcmp(argv[1],"induced")==0)
    {
        if (strcmp(argv[2],"subgraph")==0)
        {
            if (argc!=5)
            {
                WrongNumberOfArguments(interp,argc,argv);
                return TCL_ERROR;
            }

            TNode c = atol(argv[4]);
            mixedGraph *H =
                new inducedSubgraph(*G,colouredNodes(*G,c),TOption(0));

            Tcl_CreateCommand(interp,const_cast<char*>(argv[3]),
                Goblin_Mixed_Graph_Cmd,(ClientData)H,
                (Tcl_CmdDeleteProc *)Goblin_Delete_Mixed_Graph);

            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[2],"colour")==0)
        {
            if (argc!=5)
            {
                WrongNumberOfArguments(interp,argc,argv);
                return TCL_ERROR;
            }

            TNode c = atol(argv[4]);
            mixedGraph *H =
                new inducedSubgraph(*G,fullIndex<TNode>(G->N(),G->Context()),
                                        colouredArcs(*G,c),TOption(0));

            Tcl_CreateCommand(interp,const_cast<char*>(argv[3]),
                Goblin_Mixed_Graph_Cmd,(ClientData)H,
                (Tcl_CmdDeleteProc *)Goblin_Delete_Mixed_Graph);

            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[2],"orientation")==0)
        {
            if (argc!=4)
            {
                WrongNumberOfArguments(interp,argc,argv);
                return TCL_ERROR;
            }

            diGraph *H = new inducedOrientation(*G,managedObject::OPT_PARALLELS);

            Tcl_CreateCommand(interp,const_cast<char*>(argv[3]),
                Goblin_Sparse_Digraph_Cmd,(ClientData)H,
                (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Digraph);

            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[2],"bigraph")==0)
        {
            if (argc!=6)
            {
                WrongNumberOfArguments(interp,argc,argv);
                return TCL_ERROR;
            }

            TNode c1 = atol(argv[4]);
            TNode c2 = atol(argv[5]);
            biGraph *H = new inducedBigraph(*G,colouredNodes(*G,c1),
                colouredNodes(*G,c2),TOption(0));

            Tcl_CreateCommand(interp,const_cast<char*>(argv[3]),
                Goblin_Sparse_Bigraph_Cmd,(ClientData)H,
                (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Bigraph);

            interp->result = "";
            return TCL_OK;
        }
    }

    if (strcmp(argv[1],"contraction")==0)
    {
        if (argc!=3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        mixedGraph *H = new colourContraction(*G,TOption(0));

        Tcl_CreateCommand(interp,const_cast<char*>(argv[2]),
            Goblin_Mixed_Graph_Cmd,(ClientData)H,
            (Tcl_CmdDeleteProc *)Goblin_Delete_Mixed_Graph);

        interp->result = "";
        return TCL_OK;
    }

    if (strcmp(argv[1],"nodesplitting")==0)
    {
        if (argc!=3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        diGraph *H = new nodeSplitting(*G);

        Tcl_CreateCommand(interp,const_cast<char*>(argv[2]),
            Goblin_Sparse_Digraph_Cmd,(ClientData)H,
            (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Digraph);

        interp->result = "";
        return TCL_OK;
    }

    if (strcmp(argv[1],"orientation")==0)
    {
        if (argc!=3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        diGraph *H = new diGraph(*G,managedObject::OPT_PARALLELS);

        Tcl_CreateCommand(interp,const_cast<char*>(argv[2]),
            Goblin_Sparse_Digraph_Cmd,(ClientData)H,
            (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Digraph);

        interp->result = "";
        return TCL_OK;
    }

    if (strcmp(argv[1],"subdivision")==0)
    {
        if (argc!=3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        mixedGraph *H = new explicitSubdivision(*G);

        Tcl_CreateCommand(interp,const_cast<char*>(argv[2]),
            Goblin_Mixed_Graph_Cmd,(ClientData)H,
            (Tcl_CmdDeleteProc *)Goblin_Delete_Mixed_Graph);

        interp->result = "";
        return TCL_OK;
    }

    if (strcmp(argv[1],"distances")==0)
    {
        if (argc!=3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        denseDiGraph *H = new distanceGraph(*G);

        Tcl_CreateCommand(interp,const_cast<char*>(argv[2]),
            Goblin_Dense_Digraph_Cmd,(ClientData)H,
            (Tcl_CmdDeleteProc *)Goblin_Delete_Dense_Digraph);

        interp->result = "";
        return TCL_OK;
    }

    if (strcmp(argv[1],"ilpstable")==0)
    {
        if (argc!=3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        goblinILPWrapper* XLP =
            static_cast<goblinILPWrapper*>(G->StableSetToMIP());

        Tcl_CreateCommand(interp,const_cast<char*>(argv[2]),
            Goblin_Ilp_Cmd,(ClientData)XLP,
            (Tcl_CmdDeleteProc *)Goblin_Delete_Ilp);

        interp->result = "";
        return TCL_OK;
    }

    if (strcmp(argv[1],"merge")==0)
    {
        if (argc!=3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        goblinRootObject *X =
            (G->Context()).ObjectPointer(THandle(atol(argv[2])));

        if (!X->IsGraphObject())
        {
            sprintf(interp->result,"Not a graph object ID: %s",argv[3]);
            return TCL_ERROR;
        }

        abstractMixedGraph* Y = NULL;

        if (X->ClassID()==IDMixedGraph)
        {
            mixedGraph* Z = reinterpret_cast<mixedGraph*>(X->BackCast());
            Y = Z;
        }

        if (X->ClassID()==IDSparseGraph)
        {
            graph* Z = reinterpret_cast<graph*>(X->BackCast());
            Y = Z;
        }

        if (X->ClassID()==IDSparseDiGraph)
        {
            diGraph* Z = reinterpret_cast<diGraph*>(X->BackCast());
            Y = Z;
        }

        if (X->ClassID()==IDSparseBiGraph)
        {
            biGraph* Z = reinterpret_cast<biGraph*>(X->BackCast());
            Y = Z;
        }

        if (Y==NULL)
        {
            interp->result = "Unhandled object type";
            return TCL_ERROR;
        }
        else
        {
            G -> AddGraph(*Y);

            interp->result = "";
            return TCL_OK;
        }
    }

    if (strcmp(argv[1],"layout")==0)
    {
        if (argc<3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        if (strcmp(argv[2],"scale")==0)
        {
            if (argc!=7)
            {
                WrongNumberOfArguments(interp,argc,argv);
                return TCL_ERROR;
            }

            TFloat xMin = atof(argv[3]);
            TFloat xMax = atof(argv[4]);
            TFloat yMin = atof(argv[5]);
            TFloat yMax = atof(argv[6]);
            G -> Layout_ScaleEmbedding(xMin,xMax,yMin,yMax);

            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[2],"strip")==0)
        {
            G -> Layout_StripEmbedding();
            interp->result = "";
            return TCL_OK;
        }

        int pos = CT->FindParam(argc,&argv[0],"-spacing",3);
        int spacing = 0;

        if (pos>0)
        {
            if (pos<argc-1) spacing = atol(argv[pos+1]);
            else
            {
                interp->result = "Missing value for parameter \"-spacing\"";
                return TCL_ERROR;
            }
        }

        if (strcmp(argv[2],"align")==0)
        {
            if (!G->IsSparse())
            {
                interp->result = "Operation applies to sparse graphs only";
                return TCL_ERROR;
            }

            static_cast<sparseRepresentation*>(G->Representation()) -> Layout_ArcAlignment(spacing);

            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[2],"tree")==0)
        {
            int dx = spacing;
            int dy = spacing;

            pos = CT->FindParam(argc,&argv[0],"-dx",3);

            if (pos>0)
            {
                if (pos<argc-1) dx = atol(argv[pos+1]);
                else
                {
                    interp->result = "Missing value for parameter \"-dx\"";
                    return TCL_ERROR;
                }
            }

            pos = CT->FindParam(argc,&argv[0],"-dy",3);

            if (pos>0)
            {
                if (pos<argc-1) dy = atol(argv[pos+1]);
                else
                {
                    interp->result = "Missing value for parameter \"-dy\"";
                    return TCL_ERROR;
                }
            }

            abstractMixedGraph::TOptAlign mode =
                abstractMixedGraph::ALIGN_OPTIMIZE;

            if (CT->FindParam(argc,argv,"-left",3))
            {
                mode = abstractMixedGraph::ALIGN_LEFT;
            }
            else if (CT->FindParam(argc,argv,"-right",3))
            {
                mode = abstractMixedGraph::ALIGN_RIGHT;
            }
            else if (CT->FindParam(argc,argv,"-fdp",3))
            {
                mode = abstractMixedGraph::ALIGN_FDP;
            }
            else if (CT->FindParam(argc,argv,"-center",3))
            {
                mode = abstractMixedGraph::ALIGN_CENTER;
            }

            try
            {
                G -> Layout_PredecessorTree(mode,dx,dy);
            }
            catch (ERRejected) {}

            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[2],"fdp")==0)
        {
            bool restricted = false;

            if (CT->FindParam(argc,argv,"-preserve",3)) restricted = true;

            if (CT->FindParam(argc,argv,"-layered",3))
            {
                if (restricted)
                {
                    G -> Layout_ForceDirected(
                        abstractMixedGraph::FDP_LAYERED_RESTR,int(spacing));
                }
                else
                {
                    G -> Layout_ForceDirected(
                        abstractMixedGraph::FDP_LAYERED,int(spacing));
                }
            }
            else if (restricted)
            {
                G -> Layout_ForceDirected(
                        abstractMixedGraph::FDP_RESTRICTED,int(spacing));
            }
            else if (CT->FindParam(argc,argv,"-unrestricted",3))
            {
                G -> Layout_ForceDirected(
                        abstractMixedGraph::FDP_GEM,int(spacing));
            }
            else
            {
                G -> Layout_ForceDirected(
                        abstractMixedGraph::FDP_DEFAULT,int(spacing));
            }

            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[2],"layered")==0)
        {
            int dx = spacing;
            int dy = spacing;

            pos = CT->FindParam(argc,&argv[0],"-dx",3);

            if (pos>0)
            {
                if (pos<argc-1) dx = atol(argv[pos+1]);
                else dx = CT->nodeSep;
            }

            pos = CT->FindParam(argc,&argv[0],"-dy",3);

            if (pos>0)
            {
                if (pos<argc-1) dy = atol(argv[pos+1]);
                else dy = CT->nodeSep;
            }

            int method = 0;

            pos = CT->FindParam(argc,&argv[0],"-colours",3);
            if (pos>0) method |= G->LAYERED_COLOURS;

            pos = CT->FindParam(argc,&argv[0],"-span",3);
            if (pos>0) method |= G->LAYERED_EDGE_SPAN;

            pos = CT->FindParam(argc,&argv[0],"-vertical",3);
            if (pos>0) method |= G->LAYERED_VERTICAL;

            pos = CT->FindParam(argc,&argv[0],"-sweep",3);
            if (pos>0) method |= G->LAYERED_SWEEP;

            pos = CT->FindParam(argc,&argv[0],"-align",3);
            if (pos>0) method |= G->LAYERED_ALIGN;

            pos = CT->FindParam(argc,&argv[0],"-fdp",3);
            if (pos>0) method |= G->LAYERED_FDP;

            pos = CT->FindParam(argc,&argv[0],"-horizontal",3);
            if (pos>0) method |= G->LAYERED_HORIZONTAL;

            if (method==0)
            {
                G -> Layout_Layered(G->LAYERED_DEFAULT,dx,dy);
            }
            else
            {
                G -> Layout_Layered(method,dx,dy);
            }

            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[2],"circular")==0)
        {
            if (CT->FindParam(argc,argv,"-predecessors",3))
            {
                 G -> Layout_CircularByPredecessors(spacing);
            }
            else if (CT->FindParam(argc,argv,"-colours",3))
            {
                 G -> Layout_CircularByColours(spacing);
            }
            else if (CT->FindParam(argc,argv,"-outerplanar",3))
            {
                if (!(G->Layout_Outerplanar(spacing)))
                {
                    interp->result = "No outerplanar embedding given";
                    return TCL_ERROR;
                }
            }
            else
            {
                G -> Layout_Circular(spacing);
            }

            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[2],"equilateral")==0)
        {
            G -> Layout_Equilateral(spacing);

            interp->result = "";
            return TCL_OK;
        }

        pos = CT->FindParam(argc,&argv[0],"-grid",3);
        int grid = int(spacing);

        if (pos>0)
        {
            if (pos<argc-1) grid = atol(argv[pos+1]);
            else
            {
                interp->result = "Missing value for parameter \"-grid\"";
                return TCL_ERROR;
            }
        }

        if (strcmp(argv[2],"plane")==0)
        {
            TArc aBasis = NoArc;

            pos = CT->FindParam(argc,&argv[0],"-basis",3);

            if (pos>0)
            {
                if (pos<argc-1) aBasis = atol(argv[pos+1]);
                else
                {
                    interp->result = "Missing value for parameter \"-basis\"";
                    return TCL_ERROR;
                }
            }

            if (CT->FindParam(argc,argv,"-convex",3))
            {
                G -> Layout_ConvexDrawing(aBasis,grid);
            }
            else
            {
                G -> Layout_StraightLineDrawing(aBasis,grid);
            }

            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[2],"orthogonal")==0)
        {
            if (CT->FindParam(argc,argv,"-tree",3))
            {
                TNode rootNode = G->Root();

                pos = CT->FindParam(argc,&argv[0],"-root",3);

                if (pos>0)
                {
                    if (pos<argc-1) rootNode = atol(argv[pos+1]);
                    else
                    {
                        interp->result = "Missing value for parameter \"-root\"";
                        return TCL_ERROR;
                    }
                }

                if (CT->FindParam(argc,argv,"-binary",3))
                {
                    G -> Layout_HorizontalVerticalTree(rootNode,grid);
                }
                else
                {
                    G -> Layout_KandinskyTree(rootNode,grid);
                }
            }
            else if (CT->FindParam(argc,argv,"-small",3))
            {
                G -> Layout_OrthogonalDeg4(
                        abstractMixedGraph::ORTHO_4PLANAR,grid);
            }
            else if (CT->FindParam(argc,argv,"-staircase",3))
            {
                G -> Layout_StaircaseTriconnected(NoArc,grid);
            }
            else if (CT->FindParam(argc,argv,"-planar",3))
            {
                G -> Layout_StaircaseDrawing(NoArc,grid);
            }
            else
            {
                G -> Layout_Kandinsky(abstractMixedGraph::ORTHO_DEFAULT,grid);
            }

            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[2],"visibility")==0)
        {
            if (CT->FindParam(argc,argv,"-giotto",3))
            {
                G -> Layout_VisibilityRepresentation(
                    abstractMixedGraph::ORTHO_VISIBILITY_GIOTTO,grid);
            }
            else if (CT->FindParam(argc,argv,"-raw",3))
            {
                G -> Layout_VisibilityRepresentation(
                    abstractMixedGraph::ORTHO_VISIBILITY_RAW,grid);
            }
            else if (CT->FindParam(argc,argv,"-series-parallel",3))
            {
                abstractMixedGraph::TOptSeriesParallel options =
                    abstractMixedGraph::TOptSeriesParallel(
                        G->ESP_VISIBILITY|G->ESP_DIRECTED);
                G -> EdgeSeriesParallelMethod(options);
            }
            else
            {
                G -> Layout_VisibilityRepresentation(
                    abstractMixedGraph::ORTHO_VISIBILITY_TRIM,grid);
            }

            interp->result = "";
            return TCL_OK;
        }

        sprintf(interp->result,"Unknown option: %s layout %s",argv[0],argv[2]);
        return TCL_ERROR;
    }

    if (strcmp(argv[1],"extract")==0)
    {
        if (argc<3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        if (strcmp(argv[2],"tree")==0)
        {
            if (argc!=4)
            {
                WrongNumberOfArguments(interp,argc,argv);
                return TCL_ERROR;
            }

            try
            {
                TNode r = atol(argv[3]);
                G -> ExtractTree(r);
                interp->result = "1";
            }
            catch (ERCheck)
            {
                interp->result = "Invalid input data";
                return TCL_ERROR;
            }

            return TCL_OK;
        }

        if (strcmp(argv[2],"path")==0)
        {
            if (argc!=5)
            {
                WrongNumberOfArguments(interp,argc,argv);
                return TCL_ERROR;
            }

            try
            {
                TNode s = atol(argv[3]);
                TNode t = atol(argv[3]);
                G -> ExtractPath(s,t);
                interp->result = "1";
            }
            catch (ERCheck)
            {
                interp->result = "Invalid input data";
                return TCL_ERROR;
            }

            return TCL_OK;
        }

        if (argc!=3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        if (strcmp(argv[2],"cycles")==0)
        {
            try
            {
                sprintf(interp->result,"%ld",G -> ExtractCycles());
            }
            catch (ERCheck)
            {
                interp->result = "Invalid input data";
                return TCL_ERROR;
            }

            return TCL_OK;
        }

        try
        {
            interp->result = "1";

            if (strcmp(argv[2],"matching")==0)
            {
                G -> Extract1Matching();
                return TCL_OK;
            }

            if (strcmp(argv[2],"edgecover")==0)
            {
                G -> ExtractEdgeCover();
                return TCL_OK;
            }

            if (strcmp(argv[2],"trees")==0)
            {
                G -> ExtractTrees();
                return TCL_OK;
            }

            if (strcmp(argv[2],"cut")==0)
            {
                G -> ExtractCut();
                return TCL_OK;
            }

            if (strcmp(argv[2],"bipartition")==0)
            {
                G -> ExtractBipartition();
                return TCL_OK;
            }

            if (strcmp(argv[2],"colours")==0)
            {
                G -> ExtractColours();
                return TCL_OK;
            }


            interp->result = "1";
        }
        catch (ERCheck)
        {
            interp->result = "Invalid input data";
            return TCL_ERROR;
        }
        catch (ERRejected)
        {
            interp->result = "Invalid input data";
            return TCL_ERROR;
        }

        sprintf(interp->result,"Unknown option: %s extract %s",argv[0],argv[2]);
        return TCL_ERROR;
    }

    if (strcmp(argv[1],"delete")==0)
    {
        if (argc==2)
        {
            Tcl_DeleteCommand(interp,argv[0]);
            interp->result = "";
            return TCL_OK;
        }

        if (argc==3)
        {
            if (strcmp(argv[2],"subgraph")==0)
            {
                G -> InitSubgraph();
                interp->result = "";
                return TCL_OK;
            }

            if (strcmp(argv[2],"labels")==0)
            {
                G -> ReleaseLabels();
                interp->result = "";
                return TCL_OK;
            }

            if (strcmp(argv[2],"predecessors")==0)
            {
                G -> ReleasePredecessors();
                interp->result = "";
                return TCL_OK;
            }

            if (strcmp(argv[2],"colours")==0)
            {
                G -> ReleaseNodeColours();
                interp->result = "";
                return TCL_OK;
            }

            if (strcmp(argv[2],"edgecolours")==0)
            {
                G -> ReleaseEdgeColours();
                interp->result = "";
                return TCL_OK;
            }

            if (strcmp(argv[2],"potentials")==0)
            {
                G -> ReleasePotentials();
                interp->result = "";
                return TCL_OK;
            }

            if (strcmp(argv[2],"partition")==0)
            {
                G -> ReleasePartition();
                interp->result = "";
                return TCL_OK;
            }

            sprintf(interp->result,"Unknown option: %s delete %s",argv[0],argv[2]);
            return TCL_ERROR;
        }

        WrongNumberOfArguments(interp,argc,argv);
        return TCL_ERROR;
    }

    if (strcmp(argv[1],"#nodes")==0)
    {
        if (argc!=2)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        sprintf(interp->result,"%ld",G->N());
        return TCL_OK;
    }

    if (strcmp(argv[1],"#artificial")==0)
    {
        if (argc!=2)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        sprintf(interp->result,"%ld",G->NI());
        return TCL_OK;
    }

    if (strcmp(argv[1],"#arcs")==0)
    {
        if (argc!=2)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        sprintf(interp->result,"%ld",G->M());
        return TCL_OK;
    }

    if (strcmp(argv[1],"source")==0)
    {
        if (argc!=2)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        if (G->Source()==NoNode) interp->result = "*";
        else sprintf(interp->result,"%ld",G->Source());

        return TCL_OK;
    }

    if (strcmp(argv[1],"target")==0)
    {
        if (argc!=2)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        if (G->Target()==NoNode) interp->result = "*";
        else sprintf(interp->result,"%ld",G->Target());

        return TCL_OK;
    }

    if (strcmp(argv[1],"root")==0)
    {
        if (argc!=2)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        if (G->Root()==NoNode) interp->result = "*";
        else sprintf(interp->result,"%ld",G->Root());

        return TCL_OK;
    }

    if (strcmp(argv[1],"cardinality")==0)
    {
        if (argc!=2)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        sprintf(interp->result,"%f",G->Cardinality());
        return TCL_OK;
    }

    if (strcmp(argv[1],"weight")==0)
    {
        if (argc!=2)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        sprintf(interp->result,"%f",G->Weight());
        return TCL_OK;
    }

    if (strcmp(argv[1],"metric")==0)
    {
        if (argc!=2)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        sprintf(interp->result,"%d",G->MetricType());
        return TCL_OK;
    }

    if (strcmp(argv[1],"length")==0)
    {
        if (argc!=2)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        sprintf(interp->result,"%f",G->Length());
        return TCL_OK;
    }

    if (strcmp(argv[1],"generate")==0)
    {
        graphRepresentation* GR = G->Representation();

        if (strcmp(argv[2],"length")==0 && argc==3)
        {
            for (TArc a=0;a<G->M();a++)
            {
                GR -> SetLength(2*a,TFloat(G->Context().SignedRand()));
            }

            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[2],"ucap")==0 && argc==3)
        {
            for (TArc a=0;a<G->M();a++)
            {
                GR -> SetUCap(2*a,G->LCap(2*a)+1+TCap(G->Context().UnsignedRand()));
            }

            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[2],"lcap")==0 && argc==3)
        {
            for (TArc a=0;a<G->M();a++)
            {
                TCap oldCap = G->LCap(2*a);
                TCap newCap = TCap(TCap(G->Context().UnsignedRand()));

                if (newCap>oldCap)
                {
                    GR -> SetUCap(2*a,G->UCap(2*a)+newCap-oldCap);
                    GR -> SetLCap(2*a,newCap);
                }
                else
                {
                    GR -> SetLCap(2*a,newCap);
                    GR -> SetUCap(2*a,G->UCap(2*a)+newCap-oldCap);
                }
            }

            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[2],"geometry")==0 && argc==3)
        {
            TDim dNew = 2;
            TDim dOld = GR->Dim();

            if (dNew>dOld) dOld = dNew;

            for (TDim i=0;i<dOld;++i)
            {
                if (i<dNew)
                {
                    for (TNode v=0;v<G->N();v++)
                    {
                        GR -> SetC(v,i,TFloat(G->Context().SignedRand()));
                    }
                }
                else
                {
                    GR -> ReleaseCoordinate(i);
                }
            }

            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[2],"parallels")==0)
        {
            if (!G->IsSparse())
            {
                interp->result = "Operation applies to sparse graphs only";
                return TCL_ERROR;
            }

            static_cast<sparseRepresentation*>(G->Representation()) -> ExplicitParallels();
            interp->result = "";
            return TCL_OK;
        }

        if (argc!=4)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        if (strcmp(argv[2],"arcs")==0)
        {
            G -> RandomArcs(TArc(atol(argv[3])));
            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[2],"eulerian")==0)
        {
            G -> RandEulerian(TArc(atol(argv[3])));
            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[2],"regular")==0)
        {
            G -> RandRegular(TNode(atol(argv[3])));
            interp->result = "";
            return TCL_OK;
        }

        sprintf(interp->result,"Unknown option: %s generate %s",argv[0],argv[2]);
        return TCL_ERROR;
    }

    if (strcmp(argv[1],"write")==0)
    {
        if (argc!=3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        G -> Write(argv[2]);
        interp->result = "";
        return TCL_OK;
    }

    if (strcmp(argv[1],"spath")==0)
    {
        TNode rootNode = NoNode;
        TNode targetNode = NoNode;

        int pos = CT->FindParam(argc,&argv[0],"-source",2);

        if (pos>0 && pos<argc-1 && strcmp(argv[pos+1],"*")!=0)
        {
            rootNode = atol(argv[pos+1]);
        }
        else
        {
            interp->result = "Missing root node";
            return TCL_ERROR;
        }

        pos = CT->FindParam(argc,&argv[0],"-target",2);

        if (pos>0 && pos<argc-1 && strcmp(argv[pos+1],"*")!=0)
        {
            targetNode = atol(argv[pos+1]);
        }

        try
        {
            if (CT->FindParam(argc,&argv[0],"-residual",2)>0)
            {
                G -> ShortestPath(abstractMixedGraph::SPX_DEFAULT,
                                  abstractMixedGraph::SPX_RED_RES,
                                  rootNode,targetNode);

                // Export the distance labels to the node potentials
                // When calling the shortest path solver again, the
                // distance labels refer to the reduced length labels
                // but the predecessors define a shortest path tree for
                // the original edge lengths
                G -> RawPotentials();
                G -> UpdatePotentials(InfFloat);
            }
            else
            {
                G -> ShortestPath(rootNode,targetNode);
            }

            if (targetNode!=NoNode && G->Dist(targetNode)<InfFloat)
            {
                sprintf(interp->result,"%f",G->Dist(targetNode));
                return TCL_OK;
            }
            else
            {
                interp->result = "*";
                return TCL_OK;
            }
        }
        catch (...) {}

        interp->result = "Unable to assign distance labels";
        return TCL_ERROR;
    }

    if (strcmp(argv[1],"connectivity")==0)
    {
        bool edgeConnnectivity = false;
        bool strongConnnectivity = false;
        TNode sourceNode = NoNode;
        TNode targetNode = NoNode;
        TCap retCap = InfCap;

        int pos = CT->FindParam(argc,&argv[0],"-edge",2);

        if (pos>0)
        {
            edgeConnnectivity = true;
        }

        pos = CT->FindParam(argc,&argv[0],"-strong",2);

        if (pos>0)
        {
            strongConnnectivity = true;
        }

        pos = CT->FindParam(argc,&argv[0],"-source",2);

        if (pos>0 && pos<argc-1 && strcmp(argv[pos+1],"*")!=0)
        {
            sourceNode = atol(argv[pos+1]);
        }

        pos = CT->FindParam(argc,&argv[0],"-target",2);

        if (pos>0 && pos<argc-1 && strcmp(argv[pos+1],"*")!=0)
        {
            targetNode = atol(argv[pos+1]);
        }

        if (edgeConnnectivity)
        {
            if (strongConnnectivity)
            {
                retCap = G->StrongEdgeConnectivity(sourceNode,targetNode);
            }
            else
            {
                retCap = G->EdgeConnectivity(sourceNode,targetNode);
            }
        }
        else
        {
            pos = CT->FindParam(argc,&argv[0],"-node",2);
            abstractMixedGraph::TOptNodeSplitting mode =
                (pos>0) ? G->MCC_UNIT_CAPACITIES : G->MCC_MAP_DEMANDS;

            if (strongConnnectivity)
            {
                retCap = G->StrongNodeConnectivity(sourceNode,targetNode,mode);
            }
            else
            {
                retCap = G->NodeConnectivity(sourceNode,targetNode,mode);
            }
        }

        sprintf(interp->result,"%f",retCap);
        return TCL_OK;
    }

    if (strcmp(argv[1],"components")==0)
    {
        bool strongConnnectivity = false;
        TCap kappa = 1;

        int pos = CT->FindParam(argc,&argv[0],"-kappa",2);
        if (pos>0 && pos<argc-1)
        {
            kappa = atol(argv[pos+1]);
        }

        pos = CT->FindParam(argc,&argv[0],"-strong",2);
        if (pos>0)
        {
            strongConnnectivity = true;
        }

        if (strongConnnectivity)
        {
            sprintf(interp->result,"%d",G->StronglyEdgeConnected(kappa));
        }
        else
        {
            sprintf(interp->result,"%d",G->EdgeConnected(kappa));
        }

        return TCL_OK;
    }

    if (strcmp(argv[1],"bipolar")==0)
    {
        TNode sourceNode = NoNode;
        TNode targetNode = NoNode;
        TArc rootArc = NoArc;
        bool feasible = false;

        int pos = CT->FindParam(argc,&argv[0],"-rootArc",2);
        if (pos>0 && pos<argc-1 && strcmp(argv[pos+1],"*")!=0)
        {
            rootArc = atol(argv[pos+1]);
        }
        else
        {
            pos = CT->FindParam(argc,&argv[0],"-source",2);
            if (pos>0 && pos<argc-1 && strcmp(argv[pos+1],"*")!=0)
            {
                sourceNode = atol(argv[pos+1]);
            }

            pos = CT->FindParam(argc,&argv[0],"-target",2);
            if (pos>0 && pos<argc-1 && strcmp(argv[pos+1],"*")!=0)
            {
                targetNode = atol(argv[pos+1]);
            }
        }

        if (CT->FindParam(argc,&argv[0],"-decompose",2)>0)
        {
            feasible = G->STNumbering(rootArc,sourceNode,targetNode);
        }
        else
        {
            feasible = G->ImplicitSTOrientation(sourceNode,targetNode);
        }

        if (feasible)
        {
            interp->result = "";
            return TCL_OK;
        }
        else
        {
            interp->result = "Graph is not 2-connected";
            return TCL_ERROR;
        }
    }

    if (strcmp(argv[1],"colouring")==0)
    {
        if (argc>3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        TNode chi = 0;

        if (argc==3) chi = G->NodeColouring(atol(argv[2]));
        else chi = G->NodeColouring();

        if (chi>0)
        {
            sprintf(interp->result,"%ld",chi);
            return TCL_OK;
        }
        else
        {
            interp->result = "No such structure exists";
            return TCL_ERROR;
        }
    }

    if (strcmp(argv[1],"edgecolouring")==0)
    {
        if (argc>3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        TNode chi = 0;

        if (argc==3) chi = G->EdgeColouring(atol(argv[2]));
        else chi = G->EdgeColouring();

        if (chi>0)
        {
            sprintf(interp->result,"%ld",chi);
            return TCL_OK;
        }
        else
        {
            interp->result = "No such structure exists";
            return TCL_ERROR;
        }
    }

    if (strcmp(argv[1],"cliques")==0)
    {
        if (argc>3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        TNode chi = 0;

        if (argc==3) chi = G->CliqueCover(atol(argv[2]));
        else chi = G->CliqueCover();

        if (chi>0)
        {
            sprintf(interp->result,"%ld",chi);
            return TCL_OK;
        }
        else
        {
            interp->result = "No such structure exists";
            return TCL_ERROR;
        }
    }

    if (strcmp(argv[1],"stable")==0)
    {
        if (argc!=2)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        sprintf(interp->result,"%ld",G->StableSet());
        return TCL_OK;
    }

    if (strcmp(argv[1],"clique")==0)
    {
        if (argc!=2)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        sprintf(interp->result,"%ld",G->Clique());
        return TCL_OK;
    }

    if (strcmp(argv[1],"vertexcover")==0)
    {
        if (argc!=2)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        sprintf(interp->result,"%ld",G->VertexCover());
        return TCL_OK;
    }

    if (strcmp(argv[1],"eulerian")==0)
    {
        if (argc!=2)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        sprintf(interp->result,"%d",G->EulerCycle());
        return TCL_OK;
    }

    if (strcmp(argv[1],"feedback")==0)
    {
        if (argc!=2)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        sprintf(interp->result,"%f",G->FeedbackArcSet());
        return TCL_OK;
    }

    if (strcmp(argv[1],"tsp")==0)
    {
        if (argc>3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        TNode r = NoNode;

        if (argc==3 && strcmp(argv[2],"*")!=0) r=atol(argv[2]);

        TFloat ret = G->TSP(r);

        if (ret!=InfFloat)
        {
            sprintf(interp->result,"%f",ret);
            return TCL_OK;
        }
        else
        {
            interp->result = "Graph is non-Hamiltonian";
            return TCL_ERROR;
        }
    }

    if (strcmp(argv[1],"mintree")==0)
    {
        abstractMixedGraph::TOptMST characteristic = G->MST_PLAIN;
        TNode rootNode = NoNode;

        if (CT->FindParam(argc,&argv[0],"-max",2))
        {
            characteristic = G->MST_MAX;
        }

        if (CT->FindParam(argc,&argv[0],"-cycle",2))
        {
            characteristic = abstractMixedGraph::TOptMST(
                                characteristic | G->MST_ONE_CYCLE);
        }

        int pos = CT->FindParam(argc,&argv[0],"-root",2);

        if (pos>0 && pos<argc-1 && strcmp(argv[pos+1],"*")!=0)
        {
            rootNode = atol(argv[pos+1]);
        }

        TFloat ret = G->MinTree(G->MST_DEFAULT,characteristic,rootNode);

        if (ret!=InfFloat)
        {
            sprintf(interp->result,"%f",ret);
            return TCL_OK;
        }
        else
        {
            interp->result = "Graph is disconnected";
            return TCL_ERROR;
        }
    }

    if (strcmp(argv[1],"steiner")==0)
    {
        if (argc!=3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        TNode v = atol(argv[2]);

        if (strcmp(argv[2],"*")==0) v = NoNode;

        TFloat ret = G->SteinerTree(demandNodes(*G),v);

        if (ret!=InfFloat)
        {
            sprintf(interp->result,"%f",ret);
            return TCL_OK;
        }
        else
        {
            interp->result = "Terminal nodes are disconnected";
            return TCL_ERROR;
        }
    }

    if (strcmp(argv[1],"maxflow")==0)
    {
        if (argc!=4)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        TNode s = atol(argv[2]);
        TNode t = atol(argv[3]);
        TFloat flowValue = G->MaxFlow(s,t);

        try
        {
            sprintf(interp->result,"%f",flowValue);
            return TCL_OK;
        }
        catch (ERCheck)
        {
            sprintf(interp->result,"Flow is corrupted");
            return TCL_ERROR;
        }
    }

    if (strcmp(argv[1],"mincflow")==0)
    {
        if (argc!=4)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        TNode s = atol(argv[2]);
        TNode t = atol(argv[3]);
        TFloat ret = G -> MinCostSTFlow(s,t);

        try
        {
            sprintf(interp->result,"%f",ret);
            return TCL_OK;
        }
        catch (ERCheck)
        {
            interp->result = "Flow is corrupted";
            return TCL_ERROR;
        }
    }

    if (strcmp(argv[1],"minccirc")==0)
    {
        if (argc!=2)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        try
        {
            TFloat ret = G->MinCostBFlow();
            sprintf(interp->result,"%f",ret);
            return TCL_OK;
        }
        catch (ERRejected)
        {
            interp->result = "No such structure exists";
            return TCL_ERROR;
        }
    }

    if (strcmp(argv[1],"circulation")==0)
    {
        if (argc!=2)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        if (G->AdmissibleBFlow())
        {
            interp->result = "";
            return TCL_OK;
        }
        else
        {
            interp->result = "No such structure exists";
            return TCL_ERROR;
        }
    }

    if (strcmp(argv[1],"postman")==0)
    {
        bool adjustUCap = false;

        if (CT->FindParam(argc,&argv[0],"-adjust",2))
        {
            adjustUCap = true;
        }

        G -> ChinesePostman(adjustUCap);
        sprintf(interp->result,"%f",G->Weight());
        return TCL_OK;
    }

    if (strcmp(argv[1],"maxcut")==0)
    {
        if (argc!=2 && argc!=4)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        TNode s = NoNode;
        TNode t = NoNode;

        if (argc==4)
        {
            if (strcmp(argv[2],"*")==0) s = NoNode;
            else s = atol(argv[2]);

            if (strcmp(argv[3],"*")==0) t = NoNode;
            else t = atol(argv[3]);
        }

        sprintf(interp->result,"%f",G->MaxCut(s,t));

        G -> InitSubgraph();

        for (TArc a=0;a<G->M();a++)
        {
            if (G->NodeColour(G->StartNode(2*a))!=G->NodeColour(G->EndNode(2*a)) &&
                (G->Orientation(2*a)==0 || G->NodeColour(G->StartNode(2*a))==0))
            {
                G -> SetSub(2*a,G->UCap(2*a));
            }
            else
            {
                G -> SetSub(2*a,G->LCap(2*a));
            }
        }

        return TCL_OK;
    }

    if (strcmp(argv[1],"max")==0)
    {
        if (argc!=3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        if (strcmp(argv[2],"ucap")==0)
        {
            if (G->MaxUCap()!=InfCap)
            {
                if (G->MaxUCap()!=(long int)(G->MaxUCap()))
                    sprintf(interp->result,"%f",G->MaxUCap());
                else  sprintf(interp->result,"%ld",(long int)G->MaxUCap());
            }
            else interp->result = "*";

            return TCL_OK;
        }

        if (strcmp(argv[2],"lcap")==0)
        {
            if (G->MaxLCap()!=(long int)(G->MaxLCap()))
                sprintf(interp->result,"%f",G->MaxLCap());
            else  sprintf(interp->result,"%ld",(long int)G->MaxLCap());

            return TCL_OK;
        }

        if (strcmp(argv[2],"length")==0)
        {
            if (G->MaxLength()!=InfFloat)
            {
                if (G->MaxLength()!=(long int)(G->MaxLength()))
                    sprintf(interp->result,"%f",G->MaxLength());
                else  sprintf(interp->result,"%ld",(long int)G->MaxLength());
            }
            else interp->result = "*";

            return TCL_OK;
        }

        if (strcmp(argv[2],"demand")==0)
        {
            if (G->MaxDemand()!=InfCap)
            {
                if (G->MaxDemand()!=(long int)(G->MaxDemand()))
                    sprintf(interp->result,"%f",G->MaxDemand());
                else  sprintf(interp->result,"%ld",(long int)G->MaxDemand());
            }
            else interp->result = "*";

            return TCL_OK;
        }

        if (strcmp(argv[2],"cx")==0)
        {
            if (G->CMax(0)!=(long int)(G->CMax(0)))
                sprintf(interp->result,"%f",G->CMax(0));
            else  sprintf(interp->result,"%ld",(long int)G->CMax(0));

            return TCL_OK;
        }

        if (strcmp(argv[2],"cy")==0)
        {
            if (G->CMax(1)!=(long int)(G->CMax(1)))
                sprintf(interp->result,"%f",G->CMax(1));
            else  sprintf(interp->result,"%ld",(long int)G->CMax(1));

            return TCL_OK;
        }

        sprintf(interp->result,"Unknown option: %s max %s",argv[0],argv[2]);
        return TCL_ERROR;
    }

    if (strcmp(argv[1],"min")==0)
    {
        if (argc!=3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        if (strcmp(argv[2],"cx")==0)
        {
            if (G->CMin(0)!=(long int)(G->CMin(0)))
                sprintf(interp->result,"%f",G->CMin(0));
            else  sprintf(interp->result,"%ld",(long int)G->CMin(0));

            return TCL_OK;
        }

        if (strcmp(argv[2],"cy")==0)
        {
            if (G->CMin(1)!=(long int)(G->CMin(1)))
                sprintf(interp->result,"%f",G->CMin(1));
            else  sprintf(interp->result,"%ld",(long int)G->CMin(1));

            return TCL_OK;
        }

        sprintf(interp->result,"Unknown option: %s max %s",argv[0],argv[2]);
        return TCL_ERROR;
    }

    if (strcmp(argv[1],"constant")==0)
    {
        if (argc!=3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        if (strcmp(argv[2],"ucap")==0)
        {
            sprintf(interp->result,"%d",G->CUCap());
            return TCL_OK;
        }

        if (strcmp(argv[2],"lcap")==0)
        {
            sprintf(interp->result,"%d",G->CLCap());
            return TCL_OK;
        }

        if (strcmp(argv[2],"length")==0)
        {
            sprintf(interp->result,"%d",G->CLength());
            return TCL_OK;
        }

        if (strcmp(argv[2],"demand")==0)
        {
            sprintf(interp->result,"%d",G->CDemand());
            return TCL_OK;
        }

        sprintf(interp->result,"Unknown option: %s constant %s",argv[0],argv[2]);
        return TCL_ERROR;
    }

    if (strcmp(argv[1],"set")==0)
    {
        if (argc!=4)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        if (strcmp(argv[2],"ucap")==0)
        {
            TCap uu = InfCap;

            if (strcmp(argv[3],"*")!=0) uu = TCap(atof(argv[3]));

            G->Representation() -> SetCUCap(uu);
            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[2],"lcap")==0)
        {
            TCap ll = TCap(atof(argv[3]));
            G->Representation() -> SetCLCap(ll);
            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[2],"length")==0)
        {
            TFloat ll = InfFloat;

            if (strcmp(argv[3],"*")!=0) ll = TFloat(atof(argv[3]));

            G->Representation() -> SetCLength(ll);
            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[2],"demand")==0)
        {
            TCap bb = TCap(atof(argv[3]));
            G->Representation() -> SetCDemand(bb);
            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[2],"source")==0)
        {
            TNode v = NoNode;

            if (strcmp(argv[3],"*")!=0) v = TNode(atol(argv[3]));

            G -> SetSourceNode(v);
            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[2],"target")==0)
        {
            TNode v = NoNode;

            if (strcmp(argv[3],"*")!=0) v = TNode(atol(argv[3]));

            G -> SetTargetNode(v);
            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[2],"root")==0)
        {
            TNode v = NoNode;

            if (strcmp(argv[3],"*")!=0) v = TNode(atol(argv[3]));

            G -> SetRootNode(v);
            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[2],"metric")==0)
        {
            abstractMixedGraph::TMetricType metricType =
                abstractMixedGraph::TMetricType(atol(argv[3]));

            G->Representation() -> SetMetricType(metricType);
            interp->result = "";
            return TCL_OK;
        }
   }

    if (strcmp(argv[1],"is")==0)
    {
        if (strcmp(argv[2],"graph")==0)
        {
            interp->result = "1";
            return TCL_OK;
        }

        if (strcmp(argv[2],"sparse")==0)
        {
            if (G->IsSparse()) interp->result = "1";
            else interp->result = "0";

            return TCL_OK;
        }

        if (strcmp(argv[2],"directed")==0)
        {
            if (G->IsDirected()) interp->result = "1";
            else interp->result = "0";

            return TCL_OK;
        }

        if (strcmp(argv[2],"undirected")==0)
        {
            if (G->IsUndirected()) interp->result = "1";
            else interp->result = "0";

            return TCL_OK;
        }

        if (strcmp(argv[2],"bipartite")==0)
        {
            if (G->IsBipartite()) interp->result = "1";
            else interp->result = "0";

            return TCL_OK;
        }

        if (strcmp(argv[2],"balanced")==0)
        {
            if (G->IsBalanced()) interp->result = "1";
            else interp->result = "0";

            return TCL_OK;
        }

        if (strcmp(argv[2],"planar")==0)
        {
            if (G->IsPlanar()) interp->result = "1";
            else interp->result = "0";

            return TCL_OK;
        }

        if (strcmp(argv[2],"visible")==0)
        {
            if (G->Dim()>0 && G->CMax(0)>-100000 && G->CMax(1)>-100000)
            {
                interp->result = "1";
            }
            else
            {
                interp->result = "0";
            }

            return TCL_OK;
        }

        // Do not complain, message 'is' is interpreted again
    }

    if (strcmp(argv[1],"adjacency")==0)
    {
        if (argc!=4)
        {
            interp->result = "Missing end nodes";
            return TCL_ERROR;
        }

        TNode u = (TArc)atol(argv[2]);
        TNode v = (TArc)atol(argv[3]);
        TArc a = G->Adjacency(u,v);

        if (a==NoArc) interp->result = "*";
        else sprintf(interp->result,"%ld",a);

        return TCL_OK;
    }

    if (strcmp(argv[1],"node")==0)
        return Goblin_Node_Cmd(G,interp,argc,argv);

    if (strcmp(argv[1],"arc")==0)
        return Goblin_Arc_Cmd(G,interp,argc,argv);

    return Goblin_Generic_Cmd(G,interp,argc,argv);
}


int Goblin_Node_Cmd (abstractMixedGraph *G,Tcl_Interp* interp,int argc,
    _CONST_QUAL_ char* argv[]) throw(ERRejected,ERRange)
{
    if (argc<3)
    {
        WrongNumberOfArguments(interp,argc,argv);
        return TCL_ERROR;
    }

    if (strcmp(argv[2],"insert")==0)
    {
        if (argc!=3)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        sprintf(interp->result,"%ld",G->InsertNode());
        return TCL_OK;
    }

    if (argc<4)
    {
        WrongNumberOfArguments(interp,argc,argv);
        return TCL_ERROR;
    }

    TNode v = atol(argv[2]);

    if (strcmp(argv[3],"delete")==0)
    {
        G->DeleteNode(v);
        interp->result = "";
        return TCL_OK;
    }

    if (strcmp(argv[3],"demand")==0)
    {
        sprintf(interp->result,"%g",G->Demand(v));
        return TCL_OK;
    }

    if (strcmp(argv[3],"first")==0)
    {
        TArc a = G->First(v);
        if (a==NoArc) interp->result = "*";
        else sprintf(interp->result,"%ld",a);
        return TCL_OK;
    }

    if (strcmp(argv[3],"cx")==0)
    {
        sprintf(interp->result,"%f",G->C(v,0));
        return TCL_OK;
    }

    if (strcmp(argv[3],"cy")==0)
    {
        sprintf(interp->result,"%f",G->C(v,1));
        return TCL_OK;
    }

    if (strcmp(argv[3],"hidden")==0)
    {
        sprintf(interp->result,"%d",G->HiddenNode(v));
        return TCL_OK;
    }

    if (strcmp(argv[3],"colour")==0)
    {
        if (G->NodeColour(v)!=NoNode)
            sprintf(interp->result,"%ld",G->NodeColour(v));
        else interp->result = "*";

        return TCL_OK;
    }

    if (strcmp(argv[3],"degree")==0)
    {
        sprintf(interp->result,"%g",G->Deg(v));
        return TCL_OK;
    }

    if (strcmp(argv[3],"distance")==0)
    {
        if (G->Dist(v)!=InfFloat)
            sprintf(interp->result,"%g",G->Dist(v));
        else interp->result = "*";

        return TCL_OK;
    }

    if (strcmp(argv[3],"potential")==0)
    {
        sprintf(interp->result,"%g",G->Pi(v));
        return TCL_OK;
    }

    if (strcmp(argv[3],"predecessor")==0)
    {
        TArc a = G->Pred(v);

        if (a!=NoArc)
        {
            sprintf(interp->result,"%ld",a);
        }
        else interp->result = "*";

        return TCL_OK;
    }

    if (strcmp(argv[3],"first")==0)
    {
        sprintf(interp->result,"%ld",G->First(v));
        return TCL_OK;
    }

    if (strcmp(argv[3],"thread")==0)
    {
        TNode w = G->ThreadSuccessor(v);

        if (w!=NoNode)
        {
            sprintf(interp->result,"%ld",w);
        }
        else interp->result = "*";

        return TCL_OK;
    }

    if (strcmp(argv[3],"set")==0)
    {
        if (strcmp(argv[4],"thread")==0)
        {
            if (argc<7 || !(argc&1))
            {
                WrongNumberOfArguments(interp,argc,argv);
                return TCL_ERROR;
            }

            if (!G->IsSparse())
            {
                interp->result = "Operation applies to sparse graphs only";
                return TCL_ERROR;
            }

            sparseRepresentation* GR =
                static_cast<sparseRepresentation*>(G->Representation());


            for (int i=5;i<argc;i+=2)
            {
                TFloat xx = TFloat(atof(argv[i]));
                TFloat yy = TFloat(atof(argv[i+1]));

                TNode w = GR->InsertThreadSuccessor(v);
                GR -> SetC(w,0,xx);
                GR -> SetC(w,1,yy);

                v = w;
            }

            interp->result = "";
            return TCL_OK;
        }

        if (argc!=6)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        if (strcmp(argv[4],"demand")==0)
        {
            TCap bb = TCap(atof(argv[5]));
            G->Representation() -> SetDemand(v,bb);
            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[4],"cx")==0)
        {
            TFloat xx = TFloat(atof(argv[5]));
            G->Representation() -> SetC(v,0,xx);
            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[4],"cy")==0)
        {
            TFloat yy = TFloat(atof(argv[5]));
            G->Representation() -> SetC(v,1,yy);
            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[4],"first")==0)
        {
            if (!G->IsSparse())
            {
                interp->result = "Operation applies to sparse graphs only";
                return TCL_ERROR;
            }

            TArc a = TArc(atol(argv[5]));
            static_cast<sparseRepresentation*>(G->Representation()) -> SetFirst(v,a);
            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[4],"distance")==0)
        {
            TFloat dd = InfFloat;

            if (strcmp(argv[5],"*")!=0) dd = TFloat(atof(argv[5]));

            G -> SetDist(v,dd);
            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[4],"potential")==0)
        {
            TFloat pp = TFloat(atof(argv[5]));
            G -> SetPotential(v,pp);
            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[4],"colour")==0)
        {
            TNode cc = NoNode;

            if (strcmp(argv[5],"*")!=0) cc =  TNode(atoi(argv[5]));

            G -> SetNodeColour(v,cc);
            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[4],"predecessor")==0)
        {
            TArc aa = NoArc;

            if (strcmp(argv[5],"*")!=0) aa = TArc(atoi(argv[5]));

            G -> SetPred(v,aa);
            interp->result = "";
            return TCL_OK;
        }

        sprintf(interp->result,"Unknown option: %s node %s set %s",argv[0],argv[2],argv[4]);
        return TCL_ERROR;
    }

    sprintf(interp->result,"Unknown option: %s node %s",argv[0],argv[2]);
    return TCL_ERROR;
}


int Goblin_Arc_Cmd (abstractMixedGraph *G,Tcl_Interp* interp,int argc,
    _CONST_QUAL_ char* argv[]) throw(ERRejected,ERRange)
{
    if (argc<4)
    {
        WrongNumberOfArguments(interp,argc,argv);
        return TCL_ERROR;
    }

    if (strcmp(argv[2],"insert")==0)
    {
        if (argc!=5)
        {
            interp->result = "Missing end nodes";
            return TCL_ERROR;
        }

        TNode u = TArc(atol(argv[3]));
        TNode v = TArc(atol(argv[4]));

        sprintf(interp->result,"%ld",G->InsertArc(u,v));
        return TCL_OK;
    }

    TArc a = (TArc)atol(argv[2]);

    if (strcmp(argv[3],"delete")==0)
    {
        if (!G->IsSparse())
        {
            interp->result = "Operation applies to sparse graphs only";
            return TCL_ERROR;
        }

        static_cast<sparseRepresentation*>(G->Representation()) -> DeleteArc(a);
        interp->result = "";
        return TCL_OK;
    }

    if (strcmp(argv[3],"contract")==0)
    {
        if (!G->IsSparse())
        {
            interp->result = "Operation applies to sparse graphs only";
            return TCL_ERROR;
        }

        static_cast<sparseRepresentation*>(G->Representation()) -> ContractArc(a);
        interp->result = "";
        return TCL_OK;
    }

    if (strcmp(argv[3],"straight")==0)
    {
        if (!G->IsSparse())
        {
            interp->result = "Operation applies to sparse graphs only";
            return TCL_ERROR;
        }

        static_cast<sparseRepresentation*>(G->Representation()) -> ReleaseBendNodes(a);
        interp->result = "";
        return TCL_OK;
    }

    if (strcmp(argv[3],"hidden")==0)
    {
        sprintf(interp->result,"%d",G->HiddenArc(a));
        return TCL_OK;
    }

    if (strcmp(argv[3],"ucap")==0)
    {
        if (G->UCap(a)==InfCap) sprintf(interp->result,"*");
        else sprintf(interp->result,"%g",G->UCap(a));

        return TCL_OK;
    }

    if (strcmp(argv[3],"lcap")==0)
    {
        sprintf(interp->result,"%g",G->LCap(a));
        return TCL_OK;
    }

    if (strcmp(argv[3],"length")==0)
    {
        sprintf(interp->result,"%g",G->Length(a));
        return TCL_OK;
    }

    if (strcmp(argv[3],"subgraph")==0)
    {
        sprintf(interp->result,"%g",G->Sub(a));
        return TCL_OK;
    }

    if (strcmp(argv[3],"colour")==0)
    {
        if (G->EdgeColour(a)!=NoArc)
            sprintf(interp->result,"%ld",G->EdgeColour(a));
        else interp->result = "*";

        return TCL_OK;
    }

    if (strcmp(argv[3],"orientation")==0)
    {
        sprintf(interp->result,"%d",G->Orientation(a));
        return TCL_OK;
    }

    if (strcmp(argv[3],"head")==0)
    {
        sprintf(interp->result,"%ld",G->EndNode(a));
        return TCL_OK;
    }

    if (strcmp(argv[3],"tail")==0)
    {
        sprintf(interp->result,"%ld",G->StartNode(a));
        return TCL_OK;
    }

    if (strcmp(argv[3],"right")==0)
    {
        sprintf(interp->result,"%ld",G->Right(a,G->StartNode(a)));
        return TCL_OK;
    }

    if (strcmp(argv[3],"align")==0)
    {
        TNode x = G->ArcLabelAnchor(a);

        if (x!=NoNode)
        {
            sprintf(interp->result,"%ld",x);
        }
        else
        {
            interp->result = "*";
        }

        return TCL_OK;
    }

    if (strcmp(argv[3],"flip")==0)
    {
        if (!G->IsSparse())
        {
            interp->result = "Operation applies to sparse graphs only";
            return TCL_ERROR;
        }

        static_cast<sparseRepresentation*>(G->Representation()) -> FlipArc(a);
        interp->result = "";
        return TCL_OK;
    }

    if (strcmp(argv[3],"set")==0)
    {
        if (strcmp(argv[4],"align")==0)
        {
            if (!G->IsSparse())
            {
                interp->result = "Operation applies to sparse graphs only";
                return TCL_ERROR;
            }

            sparseRepresentation* GR =
                static_cast<sparseRepresentation*>(G->Representation());

            TFloat xx = TFloat(atof(argv[5]));
            TFloat yy = TFloat(atof(argv[6]));

            TNode v = G->ArcLabelAnchor(a);

            if (v==NoNode) v = GR->InsertArcLabelAnchor(a);

            GR -> SetC(v,0,xx);
            GR -> SetC(v,1,yy);

            sprintf(interp->result,"%ld",v);
            return TCL_OK;
        }

        if (argc!=6)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        if (strcmp(argv[4],"ucap")==0)
        {
            TCap uu = InfCap;

            if (strcmp(argv[5],"*")!=0) uu = TCap(atof(argv[5]));

            G->Representation() -> SetUCap(a,uu);
            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[4],"lcap")==0)
        {
            TCap ll = TCap(atof(argv[5]));
            G->Representation() -> SetLCap(a,ll);
            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[4],"length")==0)
        {
            TFloat ll = InfFloat;

            if (strcmp(argv[5],"*")!=0) ll = TFloat(atof(argv[5]));

            G->Representation() -> SetLength(a,ll);
            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[4],"right")==0)
        {
            if (!G->IsSparse())
            {
                interp->result = "Operation applies to sparse graphs only";
                return TCL_ERROR;
            }

            TArc a2 = TArc(atol(argv[5]));
            static_cast<sparseRepresentation*>(G->Representation()) -> SetRight(a,a2);
            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[4],"colour")==0)
        {
            TArc cc = NoArc;

            if (strcmp(argv[5],"*")!=0) cc =  TArc(atoi(argv[5]));

            G -> SetEdgeColour(a,cc);
            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[4],"orientation")==0)
        {
            char oo = char(atol(argv[5]));
            G->Representation() -> SetOrientation(a,oo);
            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[4],"subgraph")==0)
        {
            TFloat multiplicity = (TFloat)atof(argv[5]);
            G->SetSub(a,multiplicity);
            interp->result = "";
            return TCL_OK;
        }

        sprintf(interp->result,"Unknown option: %s arc %s set %s",argv[0],argv[2],argv[4]);
        return TCL_ERROR;
    }

    sprintf(interp->result,"Unknown option: %s arc %s",argv[0],argv[2]);
    return TCL_ERROR;
}
