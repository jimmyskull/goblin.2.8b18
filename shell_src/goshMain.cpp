
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, March 2001
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file   goshMain.cpp
/// \brief  Tcl shell command procedures for the GOBLIN library root object

#include "gosh.h"
#include "messenger.h"


goblinController *CT = NULL;
bool destroyThread = false;

static goblinMessenger *MSG = NULL;
static long unsigned goblinInterprCounter = 0;
static char* transscriptName = NULL;
static Tcl_Interp *masterInterp = NULL;
static char returnCode[255] = "";
static int errorCode = TCL_OK;


TThreadData goblinThreadData[MAX_NUM_THREADS];


void Goblin_InitThreadData() throw()
{
    for (unsigned i=0;i<MAX_NUM_THREADS;i++)
    {
        goblinThreadData[i].threadID = 0;
    }
}


unsigned Goblin_ReserveThreadIndex() throw()
{
    for (unsigned i=0;i<MAX_NUM_THREADS;i++)
    {
        if (goblinThreadData[i].threadID==0) return i;
    }

    return MAX_NUM_THREADS;
}


unsigned Goblin_MyThreadIndex() throw()
{
    pthread_t thisThreadID = pthread_self();

    for (unsigned i=0;i<MAX_NUM_THREADS;i++)
    {
        if (thisThreadID==goblinThreadData[i].threadID) return i;
    }

    return MAX_NUM_THREADS;
}


void Goblin_FreeThreadData() throw()
{
    pthread_t thisThreadID = pthread_self();

    for (unsigned i=0;i<MAX_NUM_THREADS;i++)
    {
        if (thisThreadID==goblinThreadData[i].threadID)
        {
            goblinThreadData[i].threadID = 0;
            return;
        }
    }

    return;
}


void Goblin_Config(goblinController& context) throw()
{
    char* configName = new char[strlen(getenv("HOME"))+9];
    sprintf(configName,"%s/.goshrc",getenv("HOME"));
    ifstream test(configName);

    if (test)
    {
        try
        {
            context.ReadConfiguration(configName);

            sprintf(CT->logBuffer,
                 "...configuration file \"%s\" read",configName);
            CT -> LogEntry(LOG_IO,NoHandle,CT->logBuffer);
        }
        catch (...) {}
    }

    delete[] configName;
}


void Goblin_TerminateHandler()
{
    CT -> logLevel = 0;

    sprintf(CT->logBuffer,"...Unhandled exception, caught by terminate handler");
    CT -> LogEntry(LOG_SHELL,NoHandle,CT->logBuffer);

    longjmp(goblinThreadData[Goblin_MyThreadIndex()].jumpBuffer,1);
}


int Goblin_Init(Tcl_Interp *interp)
{
    set_terminate(Goblin_TerminateHandler);

    if (goblinInterprCounter==0)
    {
        Goblin_InitThreadData();
        goblinThreadData[Goblin_ReserveThreadIndex()].threadID = pthread_self();

        transscriptName = new char[strlen(getenv("HOME"))+10];
        sprintf(transscriptName,"%s/gosh.log",getenv("HOME"));
        masterInterp = interp;
        CT = new goblinController(goblinDefaultContext);
        CT->logStream         = new ofstream(transscriptName,ios::app);
        CT->traceLevel        = 0;
        CT->traceEventHandler = &Goblin_TraceEventHandler;
        CT->logEventHandler   = &Goblin_LogEventHandler;
        CT->stopSignalHandler = &Goblin_StopSignalHandler;
        MSG = new goblinMessenger(100,*CT);

        Goblin_Config(goblinDefaultContext);
        Goblin_Config(*CT);
    }
    else CT -> LogEntry(LOG_SHELL,NoHandle,"Starting GOSH interpreter...");

    goblinInterprCounter++;

    Tcl_CreateCommand(interp,"goblin",Goblin_Cmd,(ClientData)NULL,
                      (Tcl_CmdDeleteProc *)Goblin_Delete);

    Tcl_SetVar(interp,"goblinMasterEvent","0",0);

    Tcl_LinkVar(interp,"goblinXZoom",(char*)&(CT->xZoom),TCL_LINK_DOUBLE);
    Tcl_LinkVar(interp,"goblinYZoom",(char*)&(CT->yZoom),TCL_LINK_DOUBLE);
    Tcl_LinkVar(interp,"goblinXShift",(char*)&(CT->xShift),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinYShift",(char*)&(CT->yShift),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinNodeSize",(char*)&(CT->nodeSize),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinNodeScalable",(char*)&(CT->nodeScalable),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinNodeSep",(char*)&(CT->nodeSep),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinBendSep",(char*)&(CT->bendSep),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinFineSep",(char*)&(CT->fineSep),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinArrowSize",(char*)&(CT->arrowSize),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinLegenda",(char*)&(CT->legenda),TCL_LINK_INT);

    Tcl_LinkVar(interp,"goblinArcLabels1",(char*)&(CT->arcLabels),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinArcLabels2",(char*)&(CT->arcLabels2),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinArcLabels3",(char*)&(CT->arcLabels3),TCL_LINK_INT);

    Tcl_LinkVar(interp,"goblinNodeFormatting",(char*)&(CT->nodeFormatting),
        TCL_LINK_STRING | TCL_LINK_READ_ONLY);
    Tcl_LinkVar(interp,"goblinArcFormatting",(char*)&(CT->arcFormatting),
        TCL_LINK_STRING | TCL_LINK_READ_ONLY);
    Tcl_LinkVar(interp,"goblinWallpaper",(char*)&(CT->wallpaper),
        TCL_LINK_STRING | TCL_LINK_READ_ONLY);

    Tcl_LinkVar(interp,"goblinNodeLabels",(char*)&(CT->nodeLabels),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinNodeColours",(char*)&(CT->nodeColours),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinSubgraph",(char*)&(CT->subgraph),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinPredecessors",(char*)&(CT->predecessors),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinArcStyle",(char*)&(CT->arcStyle),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinNodeStyle",(char*)&(CT->nodeStyle),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinArcColours",(char*)&(CT->arcColours),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinArrows",(char*)&(CT->arrows),TCL_LINK_INT);

    Tcl_LinkVar(interp,"goblinDisplayMode",(char*)&(CT->displayMode),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinTraceLevel",(char*)&(CT->traceLevel),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinTraceData",(char*)&(CT->traceData),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinTraceStep",(char*)&(CT->traceStep),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinThreshold",(char*)&(CT->threshold),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinTraceCounter",(char*)&(CT->fileCounter),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinCommLevel",(char*)&(CT->commLevel),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinBreakLevel",(char*)&(CT->breakLevel),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinLogLevel",(char*)&(CT->logLevel),TCL_LINK_INT);

    Tcl_LinkVar(interp,"goblinLogMethods",(char*)&(CT->logMeth),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinLogMemory",(char*)&(CT->logMem),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinLogManipulate",(char*)&(CT->logMan),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinLogIO",(char*)&(CT->logIO),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinLogResults",(char*)&(CT->logRes),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinLogWarnings",(char*)&(CT->logWarn),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinLogTimers",(char*)&(CT->logTimers),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinLogGaps",(char*)&(CT->logGaps),TCL_LINK_INT);

    Tcl_LinkVar(interp,"goblinMethFailSave",(char*)&(CT->methFailSave),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinMethDSU",(char*)&(CT->methDSU),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinMethPQ",(char*)&(CT->methPQ),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinMethModLength",(char*)&(CT->methModLength),TCL_LINK_INT);

    Tcl_LinkVar(interp,"goblinMethSPX",(char*)&(CT->methSPX),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinMethMST",(char*)&(CT->methMST),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinMethMXF",(char*)&(CT->methMXF),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinMethMCC",(char*)&(CT->methMCC),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinMethMCFST",(char*)&(CT->methMCFST),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinMethMCF",(char*)&(CT->methMCF),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinMethNWPricing",(char*)&(CT->methNWPricing),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinMethMaxBalFlow",(char*)&(CT->methMaxBalFlow),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinMethBNS",(char*)&(CT->methBNS),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinMethMinCBalFlow",(char*)&(CT->methMinCBalFlow),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinMethPrimalDual",(char*)&(CT->methPrimalDual),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinMethCandidates",(char*)&(CT->methCandidates),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinMethColour",(char*)&(CT->methColour),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinMethHeurTSP",(char*)&(CT->methHeurTSP),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinMethRelaxTSP1",(char*)&(CT->methRelaxTSP1),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinMethRelaxTSP2",(char*)&(CT->methRelaxTSP2),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinMethMaxCut",(char*)&(CT->methMaxCut),TCL_LINK_INT);

    Tcl_LinkVar(interp,"goblinMethSolve",(char*)&(CT->methSolve),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinMethLocal",(char*)&(CT->methLocal),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinMaxBBIterations",(char*)&(CT->maxBBIterations),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinMaxBBNodes",(char*)&(CT->maxBBNodes),TCL_LINK_INT);

    Tcl_LinkVar(interp,"goblinMethLP",(char*)&(CT->methLP),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinMethLPPricing",(char*)&(CT->methLPPricing),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinMethLPQTest",(char*)&(CT->methLPQTest),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinMethLPStart",(char*)&(CT->methLPStart),TCL_LINK_INT);

    Tcl_LinkVar(interp,"goblinMethFDP",(char*)&(CT->methFDP),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinMethPlanarity",(char*)&(CT->methPlanarity),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinMethOrthogonal",(char*)&(CT->methOrthogonal),TCL_LINK_INT);

    Tcl_LinkVar(interp,"goblinRandMin",(char*)&(CT->randMin),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinRandMax",(char*)&(CT->randMax),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinRandUCap",(char*)&(CT->randUCap),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinRandLCap",(char*)&(CT->randLCap),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinRandLength",(char*)&(CT->randLength),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinRandParallels",(char*)&(CT->randParallels),TCL_LINK_INT);
    Tcl_LinkVar(interp,"goblinRandGeometry",(char*)&(CT->randGeometry),TCL_LINK_INT);

    Tcl_LinkVar(interp,"goblinExtension",(char*)&(CT->objectExtension),
                TCL_LINK_STRING|TCL_LINK_READ_ONLY);

    Tcl_CreateCommand(interp,"exit",Goblin_Exit,(ClientData)NULL,
                      (Tcl_CmdDeleteProc *)NULL);

    char version[10];
    sprintf(version,"%d.%d",CT->majorVersion,CT->minorVersion);
    Tcl_PkgProvide(interp,"goblin",version);

    return TCL_OK;
}


// Propagate event in a thread to Tcl scripts by value changes of the
// variable goblinMasterEvent

static bool eventToggle = false;

void Goblin_MasterEventHandler()
{
    if (!eventToggle)
        Tcl_SetVar(masterInterp,"goblinMasterEvent","0",0);
    else
        Tcl_SetVar(masterInterp,"goblinMasterEvent","1",0);

    eventToggle = !eventToggle;
}


void Goblin_TraceEventHandler(char* fileName)
{
    if (   CT->commLevel<3
        || CT->logLevel>CT->breakLevel
        || !(CT->SolverRunning())
       )
    {
        return;
    }
    else
    {
        MSG -> TraceAppend(fileName);
        Goblin_MasterEventHandler();
        MSG -> TraceSemTake();
    }
}


void Goblin_LogEventHandler(msgType msg,TModule mod,THandle OH,char* logText)
{
    CT -> DefaultLogEventHandler(msg,mod,OH,logText);
    MSG -> MsgAppend(msg,mod,OH,logText);
    Goblin_MasterEventHandler();
}


bool Goblin_StopSignalHandler()
{
    return (MSG->SolverRunning() || CT->commLevel==0);
}


// Entry point for threaded script evaluation

void* Goblin_Thread(void* scriptName)
{
    while (Goblin_MyThreadIndex()>=MAX_NUM_THREADS) {};

    Tcl_Interp* interp2 = Tcl_CreateInterp();
    Goblin_Init(interp2);

    MSG -> SolverSignalStarted();
    sprintf(CT->logBuffer,"Evaluating \"%s\"...",(char*)scriptName);
    CT -> LogEntry(LOG_IO,NoHandle,CT->logBuffer);

    #if defined(_TIMERS_)

    CT -> globalTimer[TimerSolve] -> Enable();

    #endif

    if (Tcl_EvalFile(interp2,(char*)scriptName)==TCL_ERROR)
    {
        sprintf(CT->logBuffer,"...Thread exit status: %s",interp2->result);
        CT -> LogEntry(LOG_IO,NoHandle,CT->logBuffer);
    }

    #if defined(_TIMERS_)

    CT -> globalTimer[TimerSolve] -> Disable();

    #endif

    delete[] (char*)scriptName;
    MSG -> SolverSignalIdle();

    destroyThread = true;
    Tcl_DeleteInterp(interp2);
    destroyThread = false;

    #if defined(_TIMERS_)

    sprintf(CT->logBuffer,"Thread execution time: %.0f ms",
        CT->globalTimer[TimerSolve]->PrevTime());
    CT -> LogEntry(LOG_TIMERS,NoHandle,CT->logBuffer);

    #endif

    CT -> LogEntry(LOG_SHELL,NoHandle,"");

    Goblin_FreeThreadData();

    pthread_exit(NULL);

    return NULL;
}


// Tcl command evaluations start here

int Goblin_Cmd (ClientData clientData,Tcl_Interp* interp,int argc,
    _CONST_QUAL_ char* argv[])
{
    if (argc<2)
    {
        WrongNumberOfArguments(interp,argc,argv);
        return TCL_ERROR;
    }

    if (setjmp(goblinThreadData[Goblin_MyThreadIndex()].jumpBuffer) != 0)
    {
        return Goblin_Propagate_Exception(interp);
    }
    else try
    {
        if (strcmp(argv[1],"mixed")==0)
        {
            if (strcmp(argv[2],"graph")!=0)
            {
                sprintf(interp->result,"Unknown option: goblin mixed %s",argv[2]);
                return TCL_ERROR;
            }

            if (argc<4 || argc>5)
            {
                WrongNumberOfArguments(interp,argc,argv);
                return TCL_ERROR;
            }

            TNode n = 0;

            if (argc==5) n = TNode(atol(argv[4]));

            mixedGraph *G = new mixedGraph(n,*CT);

            Tcl_CreateCommand(interp,const_cast<char*>(argv[3]),
                Goblin_Mixed_Graph_Cmd,(ClientData)G,
                (Tcl_CmdDeleteProc *)Goblin_Delete_Mixed_Graph);

            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[1],"sparse")==0)
        {
            if (argc<4 || argc>6)
            {
                WrongNumberOfArguments(interp,argc,argv);
                return TCL_ERROR;
            }

            TNode n = 0;

            if (argc>=5) n = TNode(atol(argv[4]));

            TNode n2 = 0;

            if (argc==6) n2 = TNode(atol(argv[5]));

            if (strcmp(argv[2],"graph")==0)
            {
                graph *G = new graph(n,*CT);

                Tcl_CreateCommand(interp,const_cast<char*>(argv[3]),
                    Goblin_Sparse_Graph_Cmd,(ClientData)G,
                    (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Graph);

                interp->result = "";
                return TCL_OK;
            }

            if (strcmp(argv[2],"bigraph")==0)
            {
                biGraph *G = new biGraph(n,n2,*CT);

                Tcl_CreateCommand(interp,const_cast<char*>(argv[3]),
                    Goblin_Sparse_Bigraph_Cmd,(ClientData)G,
                    (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Bigraph);

                interp->result = "";
                return TCL_OK;
            }

            if (strcmp(argv[2],"digraph")==0)
            {
                diGraph *G = new diGraph(n,*CT);

                Tcl_CreateCommand(interp,const_cast<char*>(argv[3]),
                    Goblin_Sparse_Digraph_Cmd,(ClientData)G,
                    (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Digraph);

                interp->result = "";
                return TCL_OK;
            }

            sprintf(interp->result,"Unknown option: goblin sparse %s",argv[2]);
            return TCL_ERROR;
        }

        if (strcmp(argv[1],"dense")==0)
        {
            if (argc<4 || argc>6)
            {
                WrongNumberOfArguments(interp,argc,argv);
                return TCL_ERROR;
            }

            TNode n = 0;

            if (argc>=5) n = TNode(atol(argv[4]));

            TNode n2 = 0;

            if (argc==6) n2 = TNode(atol(argv[5]));

            if (strcmp(argv[2],"graph")==0)
            {
                denseGraph* G = new denseGraph(n,(TOption)0,*CT);

                Tcl_CreateCommand(interp,const_cast<char*>(argv[3]),
                    Goblin_Dense_Graph_Cmd,(ClientData)G,
                    (Tcl_CmdDeleteProc *)Goblin_Delete_Dense_Graph);

                interp->result = "";
                return TCL_OK;
            }

            if (strcmp(argv[2],"bigraph")==0)
            {
                denseBiGraph* G = new denseBiGraph(n,n2,(TOption)0,*CT);

                Tcl_CreateCommand(interp,const_cast<char*>(argv[3]),
                    Goblin_Dense_Bigraph_Cmd,(ClientData)G,
                    (Tcl_CmdDeleteProc *)Goblin_Delete_Dense_Bigraph);

                interp->result = "";
                return TCL_OK;
            }

            if (strcmp(argv[2],"digraph")==0)
            {
                denseDiGraph* G = new denseDiGraph(n,(TOption)0,*CT);

                Tcl_CreateCommand(interp,const_cast<char*>(argv[3]),
                    Goblin_Dense_Digraph_Cmd,(ClientData)G,
                    (Tcl_CmdDeleteProc *)Goblin_Delete_Dense_Digraph);

                interp->result = "";
                return TCL_OK;
            }

            sprintf(interp->result,"Unknown option: goblin dense %s",argv[2]);
            return TCL_ERROR;
        }


        if (strcmp(argv[1],"openGrid")==0)
        {
            TNode nRows = 4;
            TNode nColumns = 3;
            openGrid::TOptGrid shape = openGrid::GRID_SQUARE;

            int pos = CT->FindParam(argc,&argv[0],"-rows",2);

            if (pos>0 && pos<argc-1)
            {
                nRows = atol(argv[pos+1]);
            }

            pos = CT->FindParam(argc,&argv[0],"-columns",2);

            if (pos>0 && pos<argc-1)
            {
                nColumns = atol(argv[pos+1]);
            }

            pos = CT->FindParam(argc,&argv[0],"-triangular",2);

            if (pos>0 && pos<argc-1)
            {
                shape = openGrid::GRID_TRIANGULAR;
            }

            pos = CT->FindParam(argc,&argv[0],"-square",2);

            if (pos>0 && pos<argc-1)
            {
                shape = openGrid::GRID_SQUARE;
            }

            pos = CT->FindParam(argc,&argv[0],"-hexagonal",2);

            if (pos>0 && pos<argc-1)
            {
                shape = openGrid::GRID_HEXAGONAL;
            }

            pos = CT->FindParam(argc,&argv[0],"-triangular",2);

            if (pos>0 && pos<argc-1)
            {
                shape = openGrid::GRID_TRIANGULAR;
            }

            graph* G = new openGrid(nRows,nColumns,shape,*CT);

            Tcl_CreateCommand(interp,const_cast<char*>(argv[argc-1]),
                Goblin_Sparse_Graph_Cmd,(ClientData)G,
                (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Graph);

            interp->result = "";
            return TCL_OK;
        }


        if (strcmp(argv[1],"polarGrid")==0)
        {
            TNode nRows = 1;
            TNode nColumns = 3;
            TNode nPoles = 0;
            polarGrid::TOptPolar facetType = polarGrid::POLAR_DEFAULT;
            polarGrid::TOptPolar embed2D = polarGrid::POLAR_DEFAULT;

            int pos = CT->FindParam(argc,&argv[0],"-rows",2);

            if (pos>0 && pos<argc-1)
            {
                nRows = atol(argv[pos+1]);
            }

            pos = CT->FindParam(argc,&argv[0],"-columns",2);

            if (pos>0 && pos<argc-1)
            {
                nColumns = atol(argv[pos+1]);
            }

            pos = CT->FindParam(argc,&argv[0],"-poles",2);

            if (pos>0 && pos<argc-1)
            {
                nPoles = atol(argv[pos+1]);
            }

            pos = CT->FindParam(argc,&argv[0],"-triangular",2);

            if (pos>0 && pos<argc-1)
            {
                facetType = polarGrid::POLAR_TRIANGULAR;
            }

            pos = CT->FindParam(argc,&argv[0],"-square",2);

            if (pos>0 && pos<argc-1)
            {
                facetType = polarGrid::POLAR_SQUARE;
            }

            pos = CT->FindParam(argc,&argv[0],"-cone",2);

            if (pos>0 && pos<argc-1)
            {
                embed2D = polarGrid::POLAR_CONE;
            }

            pos = CT->FindParam(argc,&argv[0],"-hemisphere",2);

            if (pos>0 && pos<argc-1)
            {
                embed2D = polarGrid::POLAR_HEMISPHERE;
            }

            graph* G = new polarGrid(nRows,nColumns,nPoles,facetType,embed2D,*CT);

            Tcl_CreateCommand(interp,const_cast<char*>(argv[argc-1]),
                Goblin_Sparse_Graph_Cmd,(ClientData)G,
                (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Graph);

            interp->result = "";
            return TCL_OK;
        }


        if (strcmp(argv[1],"torusGrid")==0)
        {
            TNode girth = 9;
            TNode perimeter = 16;
            torusGrid::TOptTorus facetType = torusGrid::TORUS_DEFAULT;
            torusGrid::TOptTorus embed2D = torusGrid::TORUS_DEFAULT;

            int pos = CT->FindParam(argc,&argv[0],"-girth",2);

            if (pos>0 && pos<argc-1)
            {
                girth = atol(argv[pos+1]);
            }

            pos = CT->FindParam(argc,&argv[0],"-perimeter",2);

            if (pos>0 && pos<argc-1)
            {
                perimeter = atol(argv[pos+1]);
            }

            pos = CT->FindParam(argc,&argv[0],"-triangular",2);

            if (pos>0 && pos<argc-1)
            {
                facetType = torusGrid::TORUS_TRIANGULAR;
            }

            pos = CT->FindParam(argc,&argv[0],"-square",2);

            if (pos>0 && pos<argc-1)
            {
                facetType = torusGrid::TORUS_SQUARE;
            }

            pos = CT->FindParam(argc,&argv[0],"-hexagonal",2);

            if (pos>0 && pos<argc-1)
            {
                facetType = torusGrid::TORUS_HEXAGONAL;
            }

            pos = CT->FindParam(argc,&argv[0],"-plane",2);

            if (pos>0 && pos<argc-1)
            {
                embed2D = torusGrid::TORUS_HEMISPHERE;
            }

            graph* G = new torusGrid(girth,perimeter,facetType,embed2D,*CT);

            Tcl_CreateCommand(interp,const_cast<char*>(argv[argc-1]),
                Goblin_Sparse_Graph_Cmd,(ClientData)G,
                (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Graph);

            interp->result = "";
            return TCL_OK;
        }


        if (strcmp(argv[1],"regularTree")==0)
        {
            TNode n = NoNode;
            TNode depth = 3;
            TNode deg = 2;

            int pos = CT->FindParam(argc,&argv[0],"-nodes",2);

            if (pos>0 && pos<argc-1 && strcmp(argv[pos+1],"*")!=0)
            {
                n = atol(argv[pos+1]);
            }

            pos = CT->FindParam(argc,&argv[0],"-depth",2);

            if (pos>0 && pos<argc-1)
            {
                depth = atol(argv[pos+1]);
            }

            pos = CT->FindParam(argc,&argv[0],"-deg",2);

            if (pos>0 && pos<argc-1)
            {
                deg = atol(argv[pos+1]);
            }

            diGraph* G = new regularTree(depth,deg,n,*CT);

            Tcl_CreateCommand(interp,const_cast<char*>(argv[argc-1]),
                Goblin_Sparse_Digraph_Cmd,(ClientData)G,
                (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Digraph);

            interp->result = "";
            return TCL_OK;
        }


        if (strcmp(argv[1],"sierpinskiTriangle")==0)
        {
            TNode depth = 3;

            int pos = CT->FindParam(argc,&argv[0],"-depth",2);

            if (pos>0 && pos<argc-1)
            {
                depth = atol(argv[pos+1]);
            }

            graph* G = new sierpinskiTriangle(depth,*CT);

            Tcl_CreateCommand(interp,const_cast<char*>(argv[argc-1]),
                Goblin_Sparse_Graph_Cmd,(ClientData)G,
                (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Graph);

            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[1],"triangular")==0)
        {
            TNode cardinality = 4;

            int pos = CT->FindParam(argc,&argv[0],"-cardinality",2);

            if (pos>0 && pos<argc-1)
            {
                cardinality = atol(argv[pos+1]);
            }

            graph* G = new triangularGraph(cardinality,*CT);

            Tcl_CreateCommand(interp,const_cast<char*>(argv[argc-1]),
                Goblin_Sparse_Graph_Cmd,(ClientData)G,
                (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Graph);

            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[1],"ilp")==0 ||
            strcmp(argv[1],"lp")==0 ||
            strcmp(argv[1],"mip")==0)
        {
            if (goblinController::pLPModule==NULL)
            {
                interp->result = "No LP solver loaded";
                return TCL_ERROR;
            }

            if (argc!=5 && argc!=6)
            {
                WrongNumberOfArguments(interp,argc,argv);
                return TCL_ERROR;
            }

            TIndex k = TIndex(atol(argv[3]));
            TIndex l = TIndex(atol(argv[4]));
            TIndex nz = k*l/10+1;

            if (argc==6) nz = TIndex(atol(argv[5]));

            goblinILPModule* ModLP =
                reinterpret_cast<goblinILPModule*>(
                    const_cast<char*>(goblinController::pLPModule));
            goblinILPWrapper* XLP  =
                ModLP->NewInstance(k,l,nz,goblinILPWrapper::MINIMIZE,*CT);

            Tcl_CreateCommand(interp,const_cast<char*>(argv[2]),
                Goblin_Ilp_Cmd,(ClientData)XLP,
                (Tcl_CmdDeleteProc *)Goblin_Delete_Ilp);

            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[1],"read")==0)
        {
            if (argc<4)
            {
                WrongNumberOfArguments(interp,argc,argv);
                return TCL_ERROR;
            }

            goblinController* thisContext = CT;

            // Files contain only those configuration parameters which differ
            // from the default values. So it is required to restore these
            // default values before loading a file.
            if (CT->FindParam(argc,argv,"-spawn",2))
            {
                thisContext = new goblinController(goblinDefaultContext);
            }
            else
            {
                Goblin_Config(*thisContext);
            }


            int formatIndex = CT->FindParam(argc,argv,"-format",2)+1;

            if (formatIndex>1)
            {
                if (formatIndex>=argc-1)
                {
                    interp->result = "Missing format specification";
                    return TCL_ERROR;
                }
                else if (strcmp(argv[formatIndex],"dimacs_mcf")==0)
                {
                    goblinController::TFileFormat format = goblinController::FMT_DIMACS_MCF;

                    diGraph* G = dynamic_cast<diGraph*>(
                                    thisContext->ImportFromFile(argv[argc-1],format));

                    Tcl_CreateCommand(interp,const_cast<char*>(argv[argc-2]),
                        Goblin_Sparse_Digraph_Cmd,(ClientData)G,
                        (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Digraph);

                    interp->result = "";
                    return TCL_OK;
                }
                else if (strcmp(argv[formatIndex],"dimacs_edge")==0)
                {
                    goblinController::TFileFormat format = goblinController::FMT_DIMACS_EDGE;

                    graph* G = dynamic_cast<graph*>(
                                    thisContext->ImportFromFile(argv[argc-1],format));

                    Tcl_CreateCommand(interp,const_cast<char*>(argv[argc-2]),
                        Goblin_Sparse_Graph_Cmd,(ClientData)G,
                        (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Graph);

                    interp->result = "";
                    return TCL_OK;
                }

                sprintf(interp->result,"Unknown file format: %s",argv[formatIndex]);
                return TCL_ERROR;
            }

            goblinImport F(argv[argc-1],*thisContext);
            char* type = F.Scan();
            F.DontComplain();

            int retCode = TCL_OK;
            interp->result = "";

            if (strcmp(type,"mixed")==0)
            {
                mixedGraph* G = new mixedGraph(argv[argc-1],*thisContext);

                Tcl_CreateCommand(interp,const_cast<char*>(argv[argc-2]),
                    Goblin_Mixed_Graph_Cmd, (ClientData)G,
                    (Tcl_CmdDeleteProc *)Goblin_Delete_Mixed_Graph);
            }
            else if (strcmp(type,"graph")==0)
            {
                graph* G = new graph(argv[argc-1],*thisContext);

                Tcl_CreateCommand(interp,const_cast<char*>(argv[argc-2]),
                    Goblin_Sparse_Graph_Cmd,(ClientData)G,
                    (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Graph);
            }
            else if (strcmp(type,"dense_graph")==0)
            {
                denseGraph* G = new denseGraph(argv[argc-1],*thisContext);

                Tcl_CreateCommand(interp,const_cast<char*>(argv[argc-2]),
                    Goblin_Dense_Graph_Cmd,(ClientData)G,
                    (Tcl_CmdDeleteProc *)Goblin_Delete_Dense_Graph);
            }
            else if (strcmp(type,"bigraph")==0)
            {
                biGraph* G = new biGraph(argv[argc-1],*thisContext);

                Tcl_CreateCommand(interp,const_cast<char*>(argv[argc-2]),
                    Goblin_Sparse_Bigraph_Cmd,(ClientData)G,
                    (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Bigraph);
            }
            else if (strcmp(type,"dense_bigraph")==0)
            {
                denseBiGraph* G = new denseBiGraph(argv[argc-1],*thisContext);

                Tcl_CreateCommand(interp,const_cast<char*>(argv[argc-2]),
                    Goblin_Dense_Bigraph_Cmd,(ClientData)G,
                    (Tcl_CmdDeleteProc *)Goblin_Delete_Dense_Bigraph);
            }
            else if (strcmp(type,"digraph")==0)
            {
                diGraph* G = new diGraph(argv[argc-1],*thisContext);

                Tcl_CreateCommand(interp,const_cast<char*>(argv[argc-2]),
                    Goblin_Sparse_Digraph_Cmd,(ClientData)G,
                    (Tcl_CmdDeleteProc *)Goblin_Delete_Sparse_Digraph);
            }
            else if (strcmp(type,"dense_digraph")==0)
            {
                denseDiGraph* G = new denseDiGraph(argv[argc-1],*thisContext);

                Tcl_CreateCommand(interp,const_cast<char*>(argv[argc-2]),
                    Goblin_Dense_Digraph_Cmd,(ClientData)G,
                    (Tcl_CmdDeleteProc *)Goblin_Delete_Dense_Digraph);
            }
            else if (strcmp(type,"balanced_fnw")==0)
            {
                balancedFNW* G = new balancedFNW(argv[argc-1],*thisContext);

                Tcl_CreateCommand(interp,const_cast<char*>(argv[argc-2]),
                    Goblin_Balanced_FNW_Cmd,(ClientData)G,
                    (Tcl_CmdDeleteProc *)Goblin_Delete_Balanced_FNW);
            }
            else if (strcmp(type,"mixed_integer")==0)
            {
                if (goblinController::pLPModule==NULL)
                {
                    interp->result = "No LP solver loaded";
                    retCode = TCL_ERROR;
                }
                else
                {
                    goblinILPModule* ModLP = 
                        reinterpret_cast<goblinILPModule*>(
                            const_cast<char*>(goblinController::pLPModule));
                    goblinILPWrapper* XLP  =
                        ModLP->ReadInstance(argv[argc-1],*thisContext);

                    Tcl_CreateCommand(interp,const_cast<char*>(argv[argc-2]),
                        Goblin_Ilp_Cmd,(ClientData)XLP,
                        (Tcl_CmdDeleteProc *)Goblin_Delete_Ilp);
                }
            }
            else
            {
                sprintf(interp->result,"Not a GOBLIN object: %s",argv[argc-1]);
                retCode = TCL_ERROR;
            }

            return retCode;
        }

        if (strcmp(argv[1],"alias")==0)
        {
            if (argc!=4)
            {
                WrongNumberOfArguments(interp,argc,argv);
                return TCL_ERROR;
            }

            goblinRootObject *X = CT->ObjectPointer(THandle(atol(argv[3])));

            if (X->IsMixedILP())
            {
                goblinILPModule* ModLP =
                    reinterpret_cast<goblinILPModule*>(
                        const_cast<char*>(goblinController::pLPModule));

                goblinILPWrapper *tmpPtr = ModLP->Reinterprete(X->BackCast());

                Tcl_CreateCommand(interp,const_cast<char*>(argv[2]),
                    Goblin_Ilp_Cmd,tmpPtr,NULL);

                interp->result = "";
                return TCL_OK;
            }

            if (!X->IsGraphObject())
            {
                sprintf(interp->result,"Not a graph object ID: %s",argv[3]);
                return TCL_ERROR;
            }

            if (X->ClassID()==IDMixedGraph)
            {
                Tcl_CreateCommand(interp,const_cast<char*>(argv[2]),
                    Goblin_Mixed_Graph_Cmd,X->BackCast(),NULL);

                interp->result = "";
                return TCL_OK;
            }

            if (X->ClassID()==IDSparseGraph)
            {
                Tcl_CreateCommand(interp,const_cast<char*>(argv[2]),
                    Goblin_Sparse_Graph_Cmd,X->BackCast(),NULL);

                interp->result = "";
                return TCL_OK;
            }

            if (X->ClassID()==IDDenseGraph)
            {
                Tcl_CreateCommand(interp,const_cast<char*>(argv[2]),
                    Goblin_Dense_Graph_Cmd,X->BackCast(),NULL);

                interp->result = "";
                return TCL_OK;
            }

            if (X->ClassID()==IDSparseBiGraph)
            {
                Tcl_CreateCommand(interp,const_cast<char*>(argv[2]),
                    Goblin_Sparse_Bigraph_Cmd,X->BackCast(),NULL);

                interp->result = "";
                return TCL_OK;
            }

            if (X->ClassID()==IDDenseBiGraph)
            {
                Tcl_CreateCommand(interp,const_cast<char*>(argv[2]),
                    Goblin_Dense_Bigraph_Cmd,X->BackCast(),NULL);

                interp->result = "";
                return TCL_OK;
            }

            if (X->ClassID()==IDSparseDiGraph)
            {
                Tcl_CreateCommand(interp,const_cast<char*>(argv[2]),
                    Goblin_Sparse_Digraph_Cmd,X->BackCast(),NULL);

                interp->result = "";
                return TCL_OK;
            }

            if (X->ClassID()==IDDenseDiGraph)
            {
                Tcl_CreateCommand(interp,const_cast<char*>(argv[2]),
                    Goblin_Dense_Digraph_Cmd,X->BackCast(),NULL);

                interp->result = "";
                return TCL_OK;
            }

            if (X->ClassID()==IDBalancedFNW)
            {
                Tcl_CreateCommand(interp,const_cast<char*>(argv[2]),
                    Goblin_Balanced_FNW_Cmd,X->BackCast(),NULL);

                interp->result = "";
                return TCL_OK;
            }

            interp->result = "Unknown object type";
            return TCL_ERROR;
        }

        if (argc>5)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        if (strcmp(argv[1],"export")==0)
        {
            if (strcmp(argv[2],"settings")==0)
            {
                char* configName = new char[strlen(getenv("HOME"))+9];
                sprintf(configName,"%s/.goshrc",getenv("HOME"));

                try
                {
                    goblinExport* F = new goblinExport(configName,*CT);
                    F -> WriteConfiguration(*CT,goblinExport::CONF_FULL);
                    delete F;
                    sprintf(CT->logBuffer,
                         "...configuration file \"%s\" written",configName);
                    CT -> LogEntry(LOG_IO,NoHandle,CT->logBuffer);
                }
                catch (...) {}

                delete[] configName;

                return TCL_OK;
            }

            sprintf(interp->result,"Unknown option: goblin export %s",argv[2]);
            return TCL_ERROR;
        }

        if (strcmp(argv[1],"echo")==0)
        {
            static THandle LH = NoHandle;

            if (strcmp(argv[2],"-nonewline")==0)
            {
                if (LH!=NoHandle) CT->LogAppend(LH,(char*)argv[3]);
                else LH = CT->LogStart(MSG_ECHO,NoHandle,(char*)argv[3]);
            }
            else 
            {
                if (LH!=NoHandle)
                {
                    CT->LogEnd(LH,(char*)argv[2]);
                    LH = NoHandle;
                }
                else CT->LogEntry(MSG_ECHO,NoHandle,(char*)argv[2]);
            }

            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[1],"solver")==0)
        {
            if (strcmp(argv[2],"thread")==0)
            {
                if (argc!=4)
                {
                    WrongNumberOfArguments(interp,argc,argv);
                    return TCL_ERROR;
                }

                unsigned threadIndex = Goblin_ReserveThreadIndex();

                if (threadIndex>=MAX_NUM_THREADS)
                {
                    interp->result = "No more thread handle available";
                    return TCL_ERROR;
                }

                MSG -> SolverSignalPending();
                char* scriptName = new char[strlen(argv[3])+1];
                strcpy(scriptName,argv[3]);
                pthread_create(&goblinThreadData[threadIndex].threadID,
                                NULL,Goblin_Thread,(void*)scriptName);

                interp->result = "";
                return TCL_OK;
            }

            if (strcmp(argv[2],"return")==0)
            {
                if (argc!=4)
                {
                    WrongNumberOfArguments(interp,argc,argv);
                    return TCL_ERROR;
                }

                while (MSG->SolverIdle()) {};

                sprintf(returnCode,"%s",argv[3]);
                errorCode = TCL_OK;
                interp->result = "";
                return TCL_OK;
            }

            if (strcmp(argv[2],"throw")==0)
            {
                if (argc!=4)
                {
                    WrongNumberOfArguments(interp,argc,argv);
                    return TCL_ERROR;
                }

                while (MSG->SolverIdle()) {};

                sprintf(returnCode,"%s",argv[3]);
                errorCode = TCL_ERROR;
                interp->result = "";
                return TCL_OK;
            }

            if (argc!=3)
            {
                WrongNumberOfArguments(interp,argc,argv);
                return TCL_ERROR;
            }

            if (strcmp(argv[2],"stop")==0)
            {
                MSG -> SolverSignalStop();
                interp->result = "";
                return TCL_OK;
            }

            if (strcmp(argv[2],"idle")==0)
            {
                if (MSG->SolverIdle())
                    interp->result = "1";
                else interp->result = "0";

                return TCL_OK;
            }

            if (strcmp(argv[2],"running")==0)
            {
                if (MSG->SolverRunning())
                    interp->result = "1";
                else interp->result = "0";

                return TCL_OK;
            }

            if (strcmp(argv[2],"result")==0)
            {
                Tcl_SetResult(interp,returnCode,TCL_VOLATILE);
                return errorCode;
            }

            if (strcmp(argv[2],"progress")==0)
            {
                sprintf(interp->result,"%g",CT->ProgressCounter());
                return TCL_OK;
            }

            sprintf(interp->result,"Unknown option: goblin solver %s",argv[2]);
            return TCL_ERROR;
        }

        if (strcmp(argv[1],"messenger")==0)
        {
            if (argc!=3)
            {
                WrongNumberOfArguments(interp,argc,argv);
                return TCL_ERROR;
            }

            if (strcmp(argv[2],"restart")==0)
            {
                MSG -> Restart();
                interp->result = "";
                return TCL_OK;
            }

            if (strcmp(argv[2],"reset")==0)
            {
                MSG -> MsgReset();
                interp->result = "";
                return TCL_OK;
            }

            if (strcmp(argv[2],"eof")==0)
            {
                if (MSG->MsgEOF())
                    interp->result = "1";
                else interp->result = "0";

                return TCL_OK;
            }

            if (strcmp(argv[2],"void")==0)
            {
                if (MSG->MsgVoid())
                    interp->result = "1";
                else interp->result = "0";

                return TCL_OK;
            }

            if (strcmp(argv[2],"skip")==0)
            {
                try
                {
                    MSG -> MsgSkip();
                }
                catch (ERRejected)
                {
                    interp->result = "Message queue is empty";
                    return TCL_ERROR;
                }

                interp->result = "";
                return TCL_OK;
            }

            if (strcmp(argv[2],"text")==0)
            {
                char* ret = NULL;

                try
                {
                    ret = MSG->MsgText();
                }
                catch (ERRejected)
                {
                    interp->result = "Message queue is empty";
                    return TCL_ERROR;
                }

                Tcl_SetResult(interp,ret,TCL_VOLATILE);
                return TCL_OK;
            }

            if (strcmp(argv[2],"class")==0)
            {
                msgType ret = MSG_APPEND;

                try
                {
                    ret = MSG->MsgClass();
                }
                catch (ERRejected)
                {
                    interp->result = "Message queue is empty";
                    return TCL_ERROR;
                }

                sprintf(interp->result,"%d",ret);
                return TCL_OK;
            }

            if (strcmp(argv[2],"module")==0)
            {
                TModule ret = NoModule;

                try
                {
                    ret = MSG->MsgModule();
                }
                catch (ERRejected)
                {
                    interp->result = "Message queue is empty";
                    return TCL_ERROR;
                }

                sprintf(interp->result,"%ld",(unsigned long)ret);
                return TCL_OK;
            }

            if (strcmp(argv[2],"handle")==0)
            {
                THandle ret = NoHandle;

                try
                {
                    ret = MSG->MsgHandle();
                }
                catch (ERRejected)
                {
                    interp->result = "Message queue is empty";
                    return TCL_ERROR;
                }

                sprintf(interp->result,"%ld",(unsigned long)ret);
                return TCL_OK;
            }

            if (strcmp(argv[2],"level")==0)
            {
                int ret = 0;

                try
                {
                    ret = MSG->MsgLevel();
                }
                catch (ERRejected)
                {
                    interp->result = "Message queue is empty";
                    return TCL_ERROR;
                }

                sprintf(interp->result,"%d",ret);
                return TCL_OK;
            }

            if (strcmp(argv[2],"hidden")==0)
            {
                bool ret = false;

                try
                {
                    ret = MSG->MsgHidden();
                }
                catch (ERRejected)
                {
                    interp->result = "Message queue is empty";
                    return TCL_ERROR;
                }

                sprintf(interp->result,"%d",ret);
                return TCL_OK;
            }

            if (strcmp(argv[2],"filename")==0)
            {
                if (argc!=3)
                {
                    WrongNumberOfArguments(interp,argc,argv);
                    return TCL_ERROR;
                }

                char* ret = MSG->TraceFilename();

                if (ret==NULL)
                {
                    interp->result = "No more queued trace files";
                    return TCL_ERROR;
                }

                Tcl_SetResult(interp,ret,TCL_VOLATILE);
                return TCL_OK;
            }

            if (strcmp(argv[2],"blocked")==0)
            {
                if (MSG->TraceEvent())
                    interp->result = "1";
                else interp->result = "0";

                return TCL_OK;
            }

            if (strcmp(argv[2],"unblock")==0)
            {
                MSG -> TraceUnblock();
                interp->result = "";
                return TCL_OK;
            }

            sprintf(interp->result,"Unknown option: goblin messenger %s",argv[2]);
            return TCL_ERROR;
        }

        if (strcmp(argv[1],"configure")==0)
        {
            CT->Configure(argc,(const char**)argv);
            interp->result = "";
            return TCL_OK;
        }

        #if defined(_TIMERS_)

        if (strcmp(argv[1],"timer")==0)
        {
            if (argc<3)
            {
                WrongNumberOfArguments(interp,argc,argv);
                return TCL_ERROR;
            }

            if (strcmp(argv[2],"reset")==0)
            {
                CT -> ResetTimers();
                interp->result = "";
                return TCL_OK;
            }

            unsigned i = atol(argv[2]);

            if (i>=NoTimer)
            {
                sprintf(interp->result,"No such timer: %d",i);
                return TCL_ERROR;
            }

            if (strcmp(argv[3],"child")==0)
            {
                if (argc!=5)
                {
                    WrongNumberOfArguments(interp,argc,argv);
                    return TCL_ERROR;
                }

                unsigned j = atol(argv[4]);

                if (j>=NoTimer)
                {
                    sprintf(interp->result,"No such Timer: %d",j);
                    return TCL_ERROR;
                }

                sprintf(interp->result,"%g",
                    CT->globalTimer[i]->ChildTime(TTimer(j)));
                return TCL_OK;
            }

            if (argc!=4)
            {
                WrongNumberOfArguments(interp,argc,argv);
                return TCL_ERROR;
            }

            if (strcmp(argv[3],"reset")==0)
            {
                CT -> globalTimer[i] -> Reset();
                interp->result = "";
                return TCL_OK;
            }

            if (strcmp(argv[3],"enable")==0)
            {
                CT -> globalTimer[i] -> Enable();
                interp->result = "";
                return TCL_OK;
            }

            if (strcmp(argv[3],"disable")==0)
            {
                CT -> globalTimer[i] -> Disable();
                interp->result = "";
                return TCL_OK;
            }

            if (strcmp(argv[3],"label")==0)
            {
                sprintf(interp->result,"%s",
                    listOfTimers[i].timerName);
                return TCL_OK;
            }

            if (strcmp(argv[3],"acc")==0)
            {
                sprintf(interp->result,"%ld",
                    (unsigned long)CT->globalTimer[i]->AccTime());
                return TCL_OK;
            }

            if (strcmp(argv[3],"av")==0)
            {
                sprintf(interp->result,"%ld",
                    (unsigned long)CT->globalTimer[i]->AvTime());
                return TCL_OK;
            }

            if (strcmp(argv[3],"max")==0)
            {
                sprintf(interp->result,"%ld",
                    (unsigned long)CT->globalTimer[i]->MaxTime());
                return TCL_OK;
            }

            if (strcmp(argv[3],"min")==0)
            {
                sprintf(interp->result,"%ld",
                    (unsigned long)CT->globalTimer[i]->MinTime());
                return TCL_OK;
            }

            if (strcmp(argv[3],"prev")==0)
            {
                sprintf(interp->result,"%ld",
                    (unsigned long)CT->globalTimer[i]->PrevTime());
                return TCL_OK;
            }

            if (strcmp(argv[3],"lower")==0)
            {
                sprintf(interp->result,"%g",
                    (double)CT->globalTimer[i]->LowerBound());
                return TCL_OK;
            }

            if (strcmp(argv[3],"upper")==0)
            {
                sprintf(interp->result,"%g",
                    (double)CT->globalTimer[i]->UpperBound());
                return TCL_OK;
            }

            sprintf(interp->result,"Unknown option: goblin timer <index> %s",argv[3]);
            return TCL_ERROR;
        }

        #endif

        if (strcmp(argv[1],"module")==0)
        {
            if (argc!=4)
            {
                WrongNumberOfArguments(interp,argc,argv);
                return TCL_ERROR;
            }

            unsigned i = atol(argv[2]);

            if (i>=NoModule)
            {
                sprintf(interp->result,"No such module: %d",i);
                return TCL_ERROR;
            }

            if (strcmp(argv[3],"name")==0)
            {
                sprintf(interp->result,"%s",listOfModules[i].moduleName);
                return TCL_OK;
            }

            if (strcmp(argv[3],"timer")==0)
            {
                sprintf(interp->result,"%ld",
                    (unsigned long)listOfModules[i].moduleTimer);
                return TCL_OK;
            }

            if (strcmp(argv[3],"implementor1")==0)
            {
                sprintf(interp->result,"%ld",
                    (unsigned long)listOfModules[i].implementor1);
                return TCL_OK;
            }

            if (strcmp(argv[3],"implementor2")==0)
            {
                sprintf(interp->result,"%ld",
                    (unsigned long)listOfModules[i].implementor2);
                return TCL_OK;
            }

            if (strcmp(argv[3],"encoding_date")==0)
            {
                sprintf(interp->result,"%s",listOfModules[i].encodingDate);
                return TCL_OK;
            }

            if (strcmp(argv[3],"revision_date")==0)
            {
                sprintf(interp->result,"%s",listOfModules[i].revisionDate);
                return TCL_OK;
            }

            if (strcmp(argv[3],"original_reference")==0)
            {
                sprintf(interp->result,"%ld",
                    (unsigned long)listOfModules[i].originalReference);
                return TCL_OK;
            }

            if (strcmp(argv[3],"authors_reference")==0)
            {
                sprintf(interp->result,"%ld",
                    (unsigned long)listOfModules[i].authorsReference);
                return TCL_OK;
            }

            if (strcmp(argv[3],"text_book")==0)
            {
                sprintf(interp->result,"%ld",
                    (unsigned long)listOfModules[i].textBook);
                return TCL_OK;
            }

            sprintf(interp->result,"Unknown option: goblin module <index> %s",argv[3]);
            return TCL_ERROR;
        }

        if (strcmp(argv[1],"author")==0)
        {
            if (argc!=4)
            {
                WrongNumberOfArguments(interp,argc,argv);
                return TCL_ERROR;
            }

            unsigned i = atol(argv[2]);

            if (i>=NoAuthor)
            {
                sprintf(interp->result,"No such author: %d",i);
                return TCL_ERROR;
            }

            if (strcmp(argv[3],"name")==0)
            {
                sprintf(interp->result,"%s",listOfAuthors[i].name);
                return TCL_OK;
            }

            if (strcmp(argv[3],"affiliation")==0)
            {
                sprintf(interp->result,"%s",listOfAuthors[i].affiliation);
                return TCL_OK;
            }

            if (strcmp(argv[3],"e_mail")==0)
            {
                sprintf(interp->result,"%s",listOfAuthors[i].e_mail);
                return TCL_OK;
            }

            sprintf(interp->result,"Unknown option: goblin author <index> %s",argv[3]);
            return TCL_ERROR;
        }

        if (strcmp(argv[1],"reference")==0)
        {
            if (argc!=4)
            {
                WrongNumberOfArguments(interp,argc,argv);
                return TCL_ERROR;
            }

            unsigned i = atol(argv[2]);

            if (i>=NoReference)
            {
                sprintf(interp->result,"No such reference: %d",i);
                return TCL_ERROR;
            }

            if (strcmp(argv[3],"key")==0)
            {
                sprintf(interp->result,"%s",listOfReferences[i].refKey);
                return TCL_OK;
            }

            if (strcmp(argv[3],"authors")==0)
            {
                sprintf(interp->result,"%s",listOfReferences[i].authors);
                return TCL_OK;
            }

            if (strcmp(argv[3],"title")==0)
            {
                sprintf(interp->result,"%s",listOfReferences[i].title);
                return TCL_OK;
            }

            if (strcmp(argv[3],"type")==0)
            {
                sprintf(interp->result,"%s",listOfReferences[i].type);
                return TCL_OK;
            }

            if (strcmp(argv[3],"collection")==0)
            {
                sprintf(interp->result,"%s",listOfReferences[i].collection);
                return TCL_OK;
            }

            if (strcmp(argv[3],"editors")==0)
            {
                sprintf(interp->result,"%s",listOfReferences[i].editors);
                return TCL_OK;
            }

            if (strcmp(argv[3],"volume")==0)
            {
                sprintf(interp->result,"%ld",
                    (unsigned long)listOfReferences[i].volume);
                return TCL_OK;
            }

            if (strcmp(argv[3],"publisher")==0)
            {
                sprintf(interp->result,"%s",listOfReferences[i].publisher);
                return TCL_OK;
            }

            if (strcmp(argv[3],"year")==0)
            {
                sprintf(interp->result,"%ld",
                    (unsigned long)listOfReferences[i].year);
                return TCL_OK;
            }

            sprintf(interp->result,"Unknown option: goblin reference <index> %s",argv[3]);
            return TCL_ERROR;
        }

        if (strcmp(argv[1],"colour")==0)
        {
            if (argc!=3)
            {
                WrongNumberOfArguments(interp,argc,argv);
                return TCL_ERROR;
            }

            if (strcmp(argv[2],"predecessors")==0)
            {
                sprintf(interp->result,"#%06lX",
                    CT->Colour(goblinController::PRED_COLOUR));
                return TCL_OK;
            }

            if (strcmp(argv[2],"unreachable")==0)
            {
                sprintf(interp->result,"#%06lX",
                    CT->Colour(goblinController::INFTY_COLOUR));
                return TCL_OK;
            }

            if (strcmp(argv[2],"maximum")==0)
            {
                sprintf(interp->result,"#%06lX",
                    CT->Colour(goblinController::MAX_COLOUR));
                return TCL_OK;
            }

            if (strcmp(argv[2],"outline")==0)
            {
                sprintf(interp->result,"#%06lX",
                    CT->Colour(goblinController::OUTLINE_COLOUR));
                return TCL_OK;
            }

            if (strcmp(argv[2],"*")==0)
            {
                sprintf(interp->result,"#%06lX",
                    CT->Colour(goblinController::NO_COLOUR));
                return TCL_OK;
            }

            TNode c0 = atol(argv[2])+goblinController::ZERO_COLOUR;
            sprintf(interp->result,"#%06lX",CT->Colour(c0));
            return TCL_OK;
        }

        if (argc!=2)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        #if defined(_HEAP_MON_GLOBAL_)

        if (strcmp(argv[1],"size")==0)
        {
            sprintf(interp->result,"%ld",goblinHeapSize);
            return TCL_OK;
        }

        if (strcmp(argv[1],"maxsize")==0)
        {
            sprintf(interp->result,"%ld",goblinMaxSize);
            return TCL_OK;
        }

        if (strcmp(argv[1],"#allocs")==0)
        {
            sprintf(interp->result,"%ld",goblinNAllocs);
            return TCL_OK;
        }

        if (strcmp(argv[1],"#objects")==0)
        {
            sprintf(interp->result,"%ld",goblinNObjects);
            return TCL_OK;
        }

        if (strcmp(argv[1],"#fragments")==0)
        {
            sprintf(interp->result,"%ld",goblinNFragments);
            return TCL_OK;
        }

        #endif

        if (strcmp(argv[1],"#timers")==0)
        {
            sprintf(interp->result,"%d",NoTimer);
            return TCL_OK;
        }

        if (strcmp(argv[1],"#authors")==0)
        {
            sprintf(interp->result,"%d",NoAuthor);
            return TCL_OK;
        }

        if (strcmp(argv[1],"#modules")==0)
        {
            sprintf(interp->result,"%d",NoModule);
            return TCL_OK;
        }

        if (strcmp(argv[1],"#references")==0)
        {
            sprintf(interp->result,"%d",NoReference);
            return TCL_OK;
        }

        if (strcmp(argv[1],"restart")==0)
        {
            CT -> ResetCounters();
            CT -> ResetTimers();
            delete CT->logStream;
            CT->logStream = new ofstream(transscriptName);
            MSG -> Restart();

            interp->result = "";
            return TCL_OK;
        }
    }
    catch (...)
    {
        return Goblin_Propagate_Exception(interp);
    }

    sprintf(interp->result,"Unknown option: goblin %s",argv[1]);
    return TCL_ERROR;
}


int Goblin_Delete (ClientData clientData) throw()
{
    goblinInterprCounter--;

    if (goblinInterprCounter==0)
    {
        CT->logEventHandler = NULL;
        CT -> logStream -> flush();
        delete CT->logStream;
        CT->logStream = &clog;
        MSG -> Restart();
        delete MSG;
        delete CT;
        delete[] transscriptName;

    }
    else CT -> LogEntry(LOG_SHELL,NoHandle,"...GOSH interpreter halted");

    return TCL_OK;
}


int Goblin_Generic_Cmd (managedObject *X,Tcl_Interp* interp,int argc,
    _CONST_QUAL_ char* argv[]) throw(ERRejected,ERRange)
{
    if (argc<2)
    {
        interp->result = "Missing arguments";
        return TCL_ERROR;
    }

    if (strcmp(argv[1],"master")==0)
    {
        if (argc!=2)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        CT -> SetMaster(X->Handle());
        interp->result = "";
        return TCL_OK;
    }

    if (strcmp(argv[1],"handle")==0)
    {
        if (argc!=2)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        sprintf(interp->result,"%ld",X->Handle());
        return TCL_OK;
    }

    if (strcmp(argv[1],"trace")==0)
    {
        if (argc!=2)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        X -> Display();
        interp->result = "";
        return TCL_OK;
    }

    if (strcmp(argv[1],"is")==0)
    {
        if (strcmp(argv[2],"mip")==0        ||
            strcmp(argv[2],"graph")==0      ||
            strcmp(argv[2],"sparse")==0     ||
            strcmp(argv[2],"directed")==0   ||
            strcmp(argv[2],"undirected")==0 ||
            strcmp(argv[2],"bipartite")==0  ||
            strcmp(argv[2],"balanced")==0   ||
            strcmp(argv[2],"visible")==0
           )
        {
            interp->result = "0";
            return TCL_OK;
        }

        sprintf(interp->result,"Unknown option: %s is %s",argv[0],argv[2]);
        return TCL_ERROR;
    }

    if (strcmp(argv[1],"set")==0)
    {
        if (argc!=4)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        if (strcmp(argv[2],"name")==0)
        {
            X -> SetLabel((char*)argv[3]);
            interp->result = "";
            return TCL_OK;
        }

        sprintf(interp->result,"Unknown option: %s set %s",argv[0],argv[2]);
        return TCL_ERROR;
    }

    if (strcmp(argv[1],"export")==0)
    {
        if (argc<4)
        {
            WrongNumberOfArguments(interp,argc,argv);
            return TCL_ERROR;
        }

        if (strcmp(argv[2],"ascii")==0)
        {
            TOption opt = 0;

            if (argc>4) opt = atol(argv[4]);

            X -> ExportToAscii(argv[3],opt);
            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[2],"goblet")==0 || strcmp(argv[2],"tk")==0)
        {
            X -> ExportToTk(argv[3]);
            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[2],"xfig")==0)
        {
            X -> ExportToXFig(argv[3]);
            interp->result = "";
            return TCL_OK;
        }

        if (strcmp(argv[2],"dot")==0)
        {
            X -> ExportToDot(argv[3]);
            interp->result = "";
            return TCL_OK;
        }

        sprintf(interp->result,"Unknown export format: %s",argv[2]);
        return TCL_ERROR;
    }

    sprintf(interp->result,"Unknown option: %s",argv[1]);
    return TCL_ERROR;
}


int Goblin_Propagate_Exception (Tcl_Interp* interp) throw()
{
    if (CT->savedErrorMsgType != NO_MSG)
    {
        sprintf(interp->result,"%s - %s",
            CT->savedErrorMethodName,CT->savedErrorDescription);

        CT->savedErrorMsgType = NO_MSG;
    }
    else
    {
        CT->Error(MSG_WARN,NoHandle,"Goblin_Propagate_Exception",
            "An unknown exception has occured");
        sprintf(interp->result,"An unknown exception has occured");
    }

    CT -> logLevel = 0;
    return TCL_ERROR;
}


void WrongNumberOfArguments(Tcl_Interp* interp,int argc,_CONST_QUAL_ char* argv[]) throw()
{
    sprintf(interp->result,"Wrong number of arguments for command \"%s",argv[0]);

    for (int i=1;i<argc;++i)
    {
        sprintf(interp->result + strlen(interp->result)," %s",argv[i]);
    }

    sprintf(interp->result + strlen(interp->result),"\"");
}


int Goblin_Exit (ClientData clientData,Tcl_Interp* interp,int argc,
    _CONST_QUAL_ char* argv[]) throw()
{
    Tcl_DeleteCommand(interp,"goblin");

    Goblin_FreeThreadData();

    exit(0);

    return TCL_OK;
}


managedObject *Goblin_Read_Object(const char* fileName,
    goblinController &thisContext) throw()
{
    try
    {
        goblinImport F(fileName,thisContext);
        F.DontComplain();

        char* type = F.Scan();

        if (strcmp(type,"mixed")==0)
            return new mixedGraph(fileName,thisContext);
        if (strcmp(type,"graph")==0)
            return new graph(fileName,thisContext);
        if (strcmp(type,"dense_graph")==0)
            return new denseGraph(fileName,thisContext);
        if (strcmp(type,"bigraph")==0)
            return new biGraph(fileName,thisContext);
        if (strcmp(type,"dense_bigraph")==0)
            return new denseBiGraph(fileName,thisContext);
        if (strcmp(type,"digraph")==0)
            return new diGraph(fileName,thisContext);
        if (strcmp(type,"dense_digraph")==0)
            return new denseDiGraph(fileName,thisContext);
        if (strcmp(type,"balanced_fnw")==0)
            return new balancedFNW(fileName,thisContext);
        if (strcmp(type,"mixed_integer")==0)
        {
            goblinILPModule* ModLP =
                reinterpret_cast<goblinILPModule*>(
                    const_cast<char*>(goblinController::pLPModule));
            return ModLP->ReadInstance(fileName,thisContext);
        }

        return NULL;
    }
    catch (...) {}

    return NULL;
}
