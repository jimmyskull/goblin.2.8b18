
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, December 1998
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file   fileExport.cpp
/// \brief  #goblinExport class implementation

#include "fileExport.h"


goblinExport::goblinExport(const char* expFileName,goblinController &thisContext)
    throw(ERFile) : expFile(expFileName, ios::out), CT(thisContext)
{
    if (!expFile)
    {
        sprintf(CT.logBuffer,"Could not open export file %s, io_state %d",
            expFileName,expFile.rdstate());
        CT.Error(ERR_FILE,NoHandle,"goblinExport",CT.logBuffer);
    }

    expFile.flags(expFile.flags() | ios::right);
    expFile.setf(ios::scientific,ios::floatfield);
    expFile.precision(CT.externalPrecision-1);

    currentLevel = 0;
    currentType = 0;
}


void goblinExport::StartTuple(const char* header, char type) throw(ERRejected)
{
    #if defined(_FAILSAVE_)

    if (currentType != 0)
        CT.Error(ERR_REJECTED,NoHandle,"StartTuple","Illegal operation");

    #endif

    if (currentLevel>0) expFile << endl;

    currentLevel ++;
    currentPos = type;
    currentType = type;
    expFile.width(currentLevel);
    expFile << "(" << header;
}


void goblinExport::StartTuple(unsigned long k, char type) throw(ERRejected)
{
    #if defined(_FAILSAVE_)

    if (currentType != 0)
        CT.Error(ERR_REJECTED,NoHandle,"StartTuple","Illegal operation");

    #endif

    currentLevel ++;
    currentPos = type;   
    currentType = type;

    expFile << endl;
    expFile.width(currentLevel);
    expFile << "(" << k;
}


void goblinExport::EndTuple() throw(ERRejected)
{
    #if defined(_FAILSAVE_)

    if (currentLevel == 0)
        CT.Error(ERR_REJECTED,NoHandle,"EndTuple","Exceeding minimum depth");

    #endif

    if (currentType == 0)
    {
        expFile << endl;
        expFile.width(currentLevel);
    }

    expFile << ")";

    currentType = 0;
    currentLevel --;
}


static char itemBuffer[25];

void goblinExport::MakeIntItem(long int item, char length) throw()
{
    if (currentType!=1 && currentType==currentPos)
    {
        currentPos = 1;
        expFile << endl;
        expFile.width(length+currentLevel+1);
    }
    else
    {
        currentPos ++;
        expFile << " ";
        expFile.width(length);
    }

    expFile << item;
}


void goblinExport::MakeFloatItem(TFloat item, char length) throw()
{
    if (fabs(item)==InfFloat)
    {
        MakeNoItem(length);
        return;
    }

    if (currentType!=1 && currentType==currentPos)
    {
        currentPos = 1;
        expFile << endl;
        expFile.width(currentLevel+1);
        expFile << "";
    }
    else
    {
        currentPos ++;
        expFile << " ";
    }

    sprintf(itemBuffer,"%*.*f",length,CT.externalPrecision,double(item));
    expFile.width(length);
    expFile << itemBuffer;
}


void goblinExport::MakeStringItem(char* item, char length) throw()
{
    if (currentType!=1 && currentType==currentPos)
    {
        currentPos = 1;
        expFile << endl;
        expFile.width(currentLevel+1);
        expFile << "";
    }
    else
    {
        currentPos ++;
        expFile << " ";
    }

    expFile.width(length);
    expFile << "\"" << item << "\"";
}


template <typename T> void goblinExport::MakeItem(T item,char length) throw()
{
    MakeIntItem(item,length);
}


template <> void goblinExport::MakeItem(char* item,char length) throw()
{
    MakeStringItem(item,length);
}


template <> void goblinExport::MakeItem(double item,char length) throw()
{
    MakeFloatItem(item,length);
}


template <> void goblinExport::MakeItem(float item,char length) throw()
{
    MakeFloatItem(item,length);
}


void goblinExport::MakeNoItem(char length) throw()
{
    if ((currentType != 1)&&(currentType==currentPos))
    {
        currentPos = 1;
        expFile << endl;
        expFile.width(length+currentLevel+1);
    }
    else
    {
        currentPos ++;
        expFile << " ";
        expFile.width(length);
    }

    expFile << "*";
}


template <typename T>
void goblinExport::WriteAttribute(T* array,const char* attributeLabel,size_t size,T undefined) throw()
{
    if (size == 1)
    {
        StartTuple(attributeLabel,1);

        if  (array[0]==undefined)
        {
            MakeNoItem(0);
        }
        else
        {
            MakeItem<T>(array[0],0);
        }
    }
    else
    {
        StartTuple(attributeLabel,10);

        char length = 1;

        for (size_t i=0;i<size;i++)
        {
            char thisLength = CT.template ExternalLength<T>(array[i]);

            if (array[i]!=undefined && thisLength>length) length = thisLength;
        }

        for (size_t i=0;i<size;i++)
        {
            if (array[i]!=undefined)
            {
                MakeItem<T>(array[i],length);
            }
            else
            {
                MakeNoItem(length);
            }
        }
    }

    EndTuple();
}

template void goblinExport::WriteAttribute(double* array,const char* attributeLabel,size_t size,double undefined) throw();

template void goblinExport::WriteAttribute(float* array,const char* attributeLabel,size_t size,float undefined) throw();

template
void goblinExport::WriteAttribute(unsigned long* array,const char* attributeLabel,size_t size,unsigned long undefined) throw();

template void goblinExport::WriteAttribute(int* array,const char* attributeLabel,size_t size,int undefined) throw();

template void goblinExport::WriteAttribute(bool* array,const char* attributeLabel,size_t size,bool undefined) throw();

template void goblinExport::WriteAttribute(char* array,const char* attributeLabel,size_t size,char undefined) throw();

template void goblinExport::WriteAttribute(char** array,const char* attributeLabel,size_t size,char* undefined) throw();


void goblinExport::WriteConfiguration(const goblinRootObject *traceObject) throw()
{
    goblinController CT3(CT);
    traceObject->ConfigDisplay(CT3);
    WriteConfiguration(CT3);
}


void goblinExport::WriteConfiguration(const goblinController &CT1,TConfig tp) throw()
{
    goblinController CT2(goblinDefaultContext);
    CT2.SetDisplayParameters(TLayoutModel(CT1.layoutModel));

    StartTuple("configure",2);

    if (CT1.sourceNode!=CT2.sourceNode || tp==CONF_DIFF)
        expFile << endl << "   -source             " << CT1.sourceNode;

    if (CT1.targetNode!=CT2.targetNode || tp==CONF_DIFF)
        expFile << endl << "   -target             " << CT1.targetNode;

    if (CT1.rootNode!=CT2.rootNode || tp==CONF_DIFF)
        expFile << endl << "   -root               " << CT1.rootNode;

    if (CT1.xShift!=CT2.xShift || tp==CONF_DIFF)
        expFile << endl << "   -xShift             " << CT1.xShift;

    if (CT1.yShift!=CT2.yShift || tp==CONF_DIFF)
        expFile << endl << "   -yShift             " << CT1.yShift;

    if (CT1.xZoom!=CT2.xZoom || tp==CONF_DIFF)
        expFile << endl << "   -xZoom              " << CT1.xZoom;

    if (CT1.yZoom!=CT2.yZoom || tp==CONF_DIFF)
        expFile << endl << "   -yZoom              " << CT1.yZoom;

    if (CT1.nodeSize!=CT2.nodeSize || tp==CONF_FULL)
        expFile << endl << "   -nodeSize           " << CT1.nodeSize;

    if (CT1.nodeScalable!=CT2.nodeScalable || tp==CONF_FULL)
        expFile << endl << "   -nodeScalable       " << CT1.nodeScalable;

    if (CT1.nodeSep!=CT2.nodeSep || tp==CONF_FULL)
        expFile << endl << "   -nodeSep            " << CT1.nodeSep;

    if (CT1.bendSep!=CT2.bendSep || tp==CONF_FULL)
        expFile << endl << "   -bendSep            " << CT1.bendSep;

    if (CT1.fineSep!=CT2.fineSep || tp==CONF_FULL)
        expFile << endl << "   -fineSep            " << CT1.fineSep;

    if (CT1.arrowSize!=CT2.arrowSize || tp==CONF_FULL)
        expFile << endl << "   -arrowSize          " << CT1.arrowSize;

    if (CT1.legenda!=CT2.legenda || tp==CONF_DIFF)
        expFile << endl << "   -legenda            " << CT1.legenda;


    if (CT1.arcLabels!=CT2.arcLabels || tp==CONF_FULL)
        expFile << endl << "   -arcLabels          " << CT1.arcLabels;

    if (CT1.arcLabels2!=CT2.arcLabels2 || tp==CONF_FULL)
        expFile << endl << "   -arcLabels2         " << CT1.arcLabels2;

    if (CT1.arcLabels3!=CT2.arcLabels3 || tp==CONF_FULL)
        expFile << endl << "   -arcLabels3         " << CT1.arcLabels3;

    if (CT1.nodeLabels!=CT2.nodeLabels || tp==CONF_FULL)
        expFile << endl << "   -nodeLabels         " << CT1.nodeLabels;

    if (CT1.nodeColours!=CT2.nodeColours || tp==CONF_FULL)
        expFile << endl << "   -nodeColours        " << CT1.nodeColours;

    if (CT1.subgraph!=CT2.subgraph || tp==CONF_FULL)
        expFile << endl << "   -subgraph           " << CT1.subgraph;

    if (CT1.predecessors!=CT2.predecessors || tp==CONF_FULL)
        expFile << endl << "   -predecessors       " << CT1.predecessors;

    if (CT1.arcStyle!=CT2.arcStyle || tp==CONF_FULL)
        expFile << endl << "   -arcStyle           " << CT1.arcStyle;

    if (CT1.nodeStyle!=CT2.nodeStyle || tp==CONF_FULL)
        expFile << endl << "   -nodeStyle          " << CT1.nodeStyle;

    if (CT1.arrows!=CT2.arrows || tp==CONF_FULL)
        expFile << endl << "   -arrows             " << CT1.arrows;

    if (CT1.arcColours!=CT2.arcColours || tp==CONF_FULL)
        expFile << endl << "   -arcColours         " << CT1.arcColours;

    if (strcmp(CT1.nodeFormatting,CT2.nodeFormatting)!=0 || tp==CONF_FULL)
    {
        expFile << endl << "   -nodeFormatting     "
            << "\"" << CT1.nodeFormatting << "\"";
    }

    if (strcmp(CT1.arcFormatting,CT2.arcFormatting)!=0 || tp==CONF_FULL)
    {
        expFile << endl << "   -arcFormatting      "
            << "\"" << CT1.arcFormatting << "\"";
    }

    if (strcmp(CT1.wallpaper,CT2.wallpaper)!=0 || tp==CONF_FULL)
    {
        expFile << endl << "   -wallpaper          "
            << "\"" << CT1.wallpaper << "\"";
    }


    if (CT1.displayMode!=CT2.displayMode || tp==CONF_FULL)
        expFile << endl << "   -displayMode        " << CT1.displayMode;

    if (CT1.traceLevel!=CT2.traceLevel || tp==CONF_FULL)
        expFile << endl << "   -traceLevel         " << CT1.traceLevel;

    if (CT1.traceData!=CT2.traceData || tp==CONF_FULL)
        expFile << endl << "   -traceData          " << CT1.traceData;

    if (CT1.traceStep!=CT2.traceStep || tp==CONF_FULL)
        expFile << endl << "   -traceStep          " << CT1.traceStep;

    if (CT1.threshold!=CT2.threshold || tp==CONF_FULL)
        expFile << endl << "   -threshold          " << CT1.threshold;


    if (CT1.logMeth!=CT2.logMeth || tp==CONF_FULL)
        expFile << endl << "   -logMeth            " << CT1.logMeth;

    if (CT1.logMem!=CT2.logMem || tp==CONF_FULL)
        expFile << endl << "   -logMem             " << CT1.logMem;

    if (CT1.logMan!=CT2.logMan || tp==CONF_FULL)
        expFile << endl << "   -logMan             " << CT1.logMan;

    if (CT1.logIO!=CT2.logIO || tp==CONF_FULL)
        expFile << endl << "   -logIO              " << CT1.logIO;

    if (CT1.logRes!=CT2.logRes || tp==CONF_FULL)
        expFile << endl << "   -logRes             " << CT1.logRes;

    if (CT1.logWarn!=CT2.logWarn || tp==CONF_FULL)
        expFile << endl << "   -logWarn            " << CT1.logWarn;

    if (CT1.logTimers!=CT2.logTimers || tp==CONF_FULL)
        expFile << endl << "   -logTimers          " << CT1.logTimers;

    if (CT1.logGaps!=CT2.logGaps || tp==CONF_FULL)
        expFile << endl << "   -logGaps            " << CT1.logGaps;


    if (CT1.methFailSave!=CT2.methFailSave || tp==CONF_FULL)
        expFile << endl << "   -methFailSave       " << CT1.methFailSave;

    if (CT1.methDSU!=CT2.methDSU || tp==CONF_FULL)
        expFile << endl << "   -methDSU            " << CT1.methDSU;

    if (CT1.methPQ!=CT2.methPQ || tp==CONF_FULL)
        expFile << endl << "   -methPQ             " << CT1.methPQ;

    if (CT1.methModLength!=CT2.methModLength || tp==CONF_FULL)
        expFile << endl << "   -methModLength      " << CT1.methModLength;


    if (CT1.methSPX!=CT2.methSPX || tp==CONF_FULL)
        expFile << endl << "   -methSPX            " << CT1.methSPX;

    if (CT1.methMST!=CT2.methMST || tp==CONF_FULL)
        expFile << endl << "   -methMST            " << CT1.methMST;

    if (CT1.methMXF!=CT2.methMXF || tp==CONF_FULL)
        expFile << endl << "   -methMXF            " << CT1.methMXF;

    if (CT1.methMCFST!=CT2.methMCFST || tp==CONF_FULL)
        expFile << endl << "   -methMCFST          " << CT1.methMCFST;

    if (CT1.methMCF!=CT2.methMCF || tp==CONF_FULL)
        expFile << endl << "   -methMCF            " << CT1.methMCF;

    if (CT1.methNWPricing!=CT2.methNWPricing || tp==CONF_FULL)
        expFile << endl << "   -methNWPricing      " << CT1.methNWPricing;

    if (CT1.methMCC!=CT2.methMCC || tp==CONF_FULL)
        expFile << endl << "   -methMCC            " << CT1.methMCC;

    if (CT1.methMaxBalFlow!=CT2.methMaxBalFlow || tp==CONF_FULL)
        expFile << endl << "   -methMaxBalFlow     " << CT1.methMaxBalFlow;

    if (CT1.methBNS!=CT2.methBNS || tp==CONF_FULL)
        expFile << endl << "   -methBNS            " << CT1.methBNS;

    if (CT1.methMinCBalFlow!=CT2.methMinCBalFlow || tp==CONF_FULL)
        expFile << endl << "   -methMinCBalFlow    " << CT1.methMinCBalFlow;

    if (CT1.methPrimalDual!=CT2.methPrimalDual || tp==CONF_FULL)
        expFile << endl << "   -methPrimalDual     " << CT1.methPrimalDual;

    if (CT1.methCandidates!=CT2.methCandidates || tp==CONF_FULL)
        expFile << endl << "   -methCandidates     " << CT1.methCandidates;

    if (CT1.methColour!=CT2.methColour || tp==CONF_FULL)
        expFile << endl << "   -methColour         " << CT1.methColour;

    if (CT1.methHeurTSP!=CT2.methHeurTSP || tp==CONF_FULL)
        expFile << endl << "   -methHeurTSP        " << CT1.methHeurTSP;

    if (CT1.methRelaxTSP1!=CT2.methRelaxTSP1 || tp==CONF_FULL)
        expFile << endl << "   -methRelaxTSP1      " << CT1.methRelaxTSP1;

    if (CT1.methRelaxTSP2!=CT2.methRelaxTSP2 || tp==CONF_FULL)
        expFile << endl << "   -methRelaxTSP2      " << CT1.methRelaxTSP2;

    if (CT1.methMaxCut!=CT2.methMaxCut || tp==CONF_FULL)
        expFile << endl << "   -methMaxCut         " << CT1.methMaxCut;


    if (CT1.methLP!=CT2.methLP || tp==CONF_FULL)
        expFile << endl << "   -methLP             " << CT1.methLP;

    if (CT1.methLPPricing!=CT2.methLPPricing || tp==CONF_FULL)
        expFile << endl << "   -methLPPricing      " << CT1.methLPPricing;

    if (CT1.methLPQTest!=CT2.methLPQTest || tp==CONF_FULL)
        expFile << endl << "   -methLPQTest        " << CT1.methLPQTest;

    if (CT1.methLPStart!=CT2.methLPStart || tp==CONF_FULL)
        expFile << endl << "   -methLPStart        " << CT1.methLPStart;


    if (CT1.methSolve!=CT2.methSolve || tp==CONF_FULL)
        expFile << endl << "   -methSolve          " << CT1.methSolve;

    if (CT1.methLocal!=CT2.methLocal || tp==CONF_FULL)
        expFile << endl << "   -methLocal          " << CT1.methLocal;

    if (CT1.maxBBIterations!=CT2.maxBBIterations || tp==CONF_FULL)
        expFile << endl << "   -maxBBIterations    " << CT1.maxBBIterations;

    if (CT1.maxBBNodes!=CT2.maxBBNodes || tp==CONF_FULL)
        expFile << endl << "   -maxBBNodes         " << CT1.maxBBNodes;


    if (CT1.methFDP!=CT2.methFDP || tp==CONF_FULL)
        expFile << endl << "   -methFDP            " << CT1.methFDP;

    if (CT1.methPlanarity!=CT2.methPlanarity || tp==CONF_FULL)
        expFile << endl << "   -methPlanarity     " << CT1.methPlanarity;

    if (CT1.methOrthogonal!=CT2.methOrthogonal || tp==CONF_FULL)
        expFile << endl << "   -methOrthogonal     " << CT1.methOrthogonal;


    if (CT1.randMin!=CT2.randMin || tp==CONF_FULL)
        expFile << endl << "   -randMin            " << CT1.randMin;

    if (CT1.randMax!=CT2.randMax || tp==CONF_FULL)
        expFile << endl << "   -randMax            " << CT1.randMax;

    if (CT1.randUCap!=CT2.randUCap || tp==CONF_FULL)
        expFile << endl << "   -randUCap           " << CT1.randUCap;

    if (CT1.randLCap!=CT2.randLCap || tp==CONF_FULL)
        expFile << endl << "   -randLCap           " << CT1.randLCap;

    if (CT1.randLength!=CT2.randLength || tp==CONF_FULL)
        expFile << endl << "   -randLength         " << CT1.randLength;

    if (CT1.randParallels!=CT2.randParallels || tp==CONF_FULL)
        expFile << endl << "   -randParallels      " << CT1.randParallels;

    if (CT1.randGeometry!=CT2.randGeometry || tp==CONF_FULL)
        expFile << endl << "   -randGeometry       " << CT1.randGeometry;

    EndTuple();
}


goblinExport::~goblinExport() throw()
{
    expFile << endl;
    expFile.close();

    if (currentLevel > 0)
        CT.Error(ERR_INTERNAL,NoHandle,"goblinExport","Some lists are open");
}
