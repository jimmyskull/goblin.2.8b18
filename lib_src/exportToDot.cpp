
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, March 2007
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file   exportToDot.cpp
/// \brief  #exportToDot class implementation

#include "exportToDot.h"


exportToDot::exportToDot(const abstractMixedGraph& GG,const char* expFileName)
    throw(ERFile) : canvasBuilder(GG), expFile(expFileName, ios::out)
{
    if (!expFile)
    {
        sprintf(CT.logBuffer,"Could not open export file %s, io_state %d",
            expFileName,expFile.rdstate());
        Error(ERR_FILE,"exportToDot",CT.logBuffer);
    }

    expFile.flags(expFile.flags() | ios::right);
    expFile.setf(ios::floatfield);
    expFile.precision(5);

    if (G.IsUndirected())
    {
        expFile << "graph G {" << endl;
    }
    else
    {
        expFile << "digraph G {" << endl;
    }
}


unsigned long exportToDot::Size() const throw()
{
    return
          sizeof(exportToDot)
        + managedObject::Allocated();
}


unsigned long exportToDot::Allocated() const throw()
{
    return 0;
}


void exportToDot::WriteArc(TArc a,TNode u,TNode v,TLineStyle lineStyle,int width,
    TArrowDir displayedArrows,TIndex colourIndex,int depth) throw()
{
    char* rgbColour = NULL;

    if (colourIndex==goblinController::OUTLINE_COLOUR)
    {
        rgbColour = "#000000";
    }
    else if (colourIndex==goblinController::PRED_COLOUR)
    {
        rgbColour = predColour;
    }
    else if (CFG.arcColours==3)
    {
        rgbColour = SmoothColour(colourIndex - goblinController::ZERO_COLOUR,
                                 maxEdgeColour,goblinController::MAX_COLOUR);
    }
    else
    {
        rgbColour = FixedEdgeColour(colourIndex - goblinController::ZERO_COLOUR);
    }

    expFile << "  v" << u;

    if (displayedArrows==ARROW_FORWARD)
    {
        expFile << " -> ";
    }
    else
    {
        expFile << " -- ";
    }

    expFile << "v" << v << " ";

    expFile << "[color = \"" << rgbColour << "\", label = \"";

    ComposeArcLabel(a,expFile);

    expFile << "\"";

    if (lineStyle==LINE_STYLE_DOT)
    {
        expFile << ", style = dotted";
    }
    else if (lineStyle==LINE_STYLE_DASH)
    {
        expFile << ", style = dashed";
    }
    else if (lineStyle==LINE_STYLE_DASH_DOT)
    {
        expFile << ", style = bold";
    }

    expFile << "];" << endl;
}


void exportToDot::DisplayNode(TNode v) throw()
{
    expFile << "  v" << v << " [";

    char* fillColour = "#ffffff";
    if (CFG.nodeColours==1 && (G.Dist(v)==InfFloat || G.Dist(v)==-InfFloat))
        fillColour = inftyColour;

    if (CFG.nodeColours==2) fillColour = FixedNodeColour(G.NodeColour(v));

    if (CFG.nodeColours==3 && G.Demand(v)>0) fillColour = FixedNodeColour(1);
    if (CFG.nodeColours==3 && G.Demand(v)<0) fillColour = FixedNodeColour(2);

    if (CFG.nodeColours==4)
        fillColour = SmoothColour(G.NodeColour(v),maxNodeColour,goblinController::NO_COLOUR);

    expFile << "style = filled, fillcolor = \"" << fillColour << "\", label = \"";

    if (strcmp(CFG.nodeFormatting,"")!=0 || CFG.nodeLabels!=0)
    {
        ComposeNodeLabel(v,expFile);
    }

    expFile << "\", shape = ";

    switch (CFG.nodeStyle)
    {
        case 0:
        {
            expFile << "point";
            break;
        }
        case 1:
        {
            expFile << "circle";
            break;
        }
        case 2:
        {
            expFile << "box";
            break;
        }
    }

    expFile << "];" << endl;
}


exportToDot::~exportToDot() throw()
{
    expFile << "}" << endl;
    expFile.close();
}
