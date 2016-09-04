
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, August 2000
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file canvasBuilder.cpp
/// \brief #canvasBuilder class implementation

#include "canvasBuilder.h"


canvasBuilder::canvasBuilder(const abstractMixedGraph &GC)
    throw(ERRejected) :
    managedObject(GC.Context()),
    G(GC), CFG(GC.Context(),true), piG(G.GetPotentials())
{
    label = new char[255];

    sprintf(predColour,"#%06lX",CT.Colour(goblinController::PRED_COLOUR));
    sprintf(inftyColour,"#%06lX",CT.Colour(goblinController::INFTY_COLOUR));

    if (CFG.nodeColours==4)
    {
        maxNodeColour = 1;
        for (TNode v=0;v<G.N();v++)
        {
            TNode c = G.NodeColour(v);
            if (c>maxNodeColour && c!=NoNode) maxNodeColour = c;
        }
    }

    if (CFG.arcColours==3)
    {
        maxEdgeColour = 1;
        for (TArc a=0;a<G.M();a++)
        {
            TArc c = G.EdgeColour(2*a);
            if (c>maxEdgeColour && c!=NoArc) maxEdgeColour = c;
        }
    }

    portMode = PORTS_IMPLICIT;

    if (   CFG.layoutModel==LAYOUT_KANDINSKI
        || CFG.layoutModel==LAYOUT_VISIBILITY
//        || CFG.layoutModel==LAYOUT_ORTHO_BIG
       )
    {
        portMode = PORTS_EXPLICIT;
    }

    ComputeBoundingBox();

    CFG.logMeth = 0;

    LogEntry(LOG_MEM,"...Display object instanciated");
}


canvasBuilder::~canvasBuilder() throw()
{
    delete[] label;

    LogEntry(LOG_MEM,"...Display object disallocated");
}


unsigned long canvasBuilder::Allocated() const throw()
{
    return 255;
}


void canvasBuilder::ComputeBoundingBox() throw()
{
    minX =  InfFloat;
    minY =  InfFloat;
    maxX = -InfFloat;
    maxY = -InfFloat;

    for (TNode v=0;v<G.N()+G.NI();v++)
    {
        if (G.HiddenNode(v)) continue;

        TFloat thisX = G.C(v,0);
        TFloat thisY = G.C(v,1);

        if (thisX<minX) minX = thisX;
        if (thisY<minY) minY = thisY;
        if (thisX>maxX) maxX = thisX;
        if (thisY>maxY) maxY = thisY;
    }
}


double canvasBuilder::ScaleDisplayParameters(double zoomCalibration,double xSize, double ySize) throw()
{
    double shrinkingFactor = 1.0;

    if (maxX*CFG.xZoom>xSize) CFG.xZoom = int(xSize/maxX);
    if (maxY*CFG.yZoom>ySize) CFG.yZoom = int(ySize/maxY);

    CFG.xZoom = CFG.yZoom = ((CFG.xZoom>CFG.yZoom) ? CFG.yZoom : CFG.xZoom);

    CFG.xZoom = double(CFG.xZoom*zoomCalibration);
    CFG.yZoom = double(CFG.yZoom*zoomCalibration);
    CFG.xShift = int(CFG.xShift*zoomCalibration+0.5);
    CFG.yShift = int(CFG.yShift*zoomCalibration+0.5);

    nodeSize = CFG.bendSep/5.0 * CFG.nodeSize/100.0 * (CFG.xZoom+CFG.yZoom)/2.0;
    arrowSize = CFG.arrowSize/100.0 * (CFG.xZoom+CFG.yZoom)/2.0;

    if (CFG.bendSep/2.0<CFG.nodeSep/10.0)
    {
        arrowSize *= CFG.bendSep/2.0;
    }
    else
    {
        arrowSize *= CFG.nodeSep/10.0;
    }

    return shrinkingFactor;
}


long canvasBuilder::PortCX(TNode u,TNode v) throw()
{
    long dx = CanvasCX(v)-CanvasCX(u);
    long dy = CanvasCY(v)-CanvasCY(u);
    float norm = sqrt(double(dx*dx+dy*dy));

    if (fabs(norm)<10e-6)
    {
        return CanvasCX(v);
    }

    if (CFG.nodeStyle==0)
    {
        return long(CanvasCX(v)-dx*7/norm);
    }
    else
    {
        return long(CanvasCX(v)-dx*nodeSize/norm);
    }
}


long canvasBuilder::PortCY(TNode u,TNode v) throw()
{
    long dx = CanvasCX(v)-CanvasCX(u);
    long dy = CanvasCY(v)-CanvasCY(u);
    float norm = sqrt(double(dx*dx+dy*dy));

    if (fabs(norm)<10e-6)
    {
        return CanvasCY(v);
    }

    if (CFG.nodeStyle==0)
    {
        return long(CanvasCY(v)-dy*5/norm);
    }
    else
    {
        return long(CanvasCY(v)-dy*nodeSize/norm);
    }
}


void canvasBuilder::DisplayGraph() throw(ERRejected)
{
    #if defined(_FAILSAVE_)

    if (G.Dim()==0)
    {
        G.Error(ERR_REJECTED,"canvasBuilder","Missing geometric embedding");
    }

    #endif

    for (TArc a=0;a<G.M();a++)
    {
        TArc a2 = 2*a;

        if (G.HiddenArc(a2)) continue;

        if (G.Blocking(a2) && G.Blocking(a2+1)) continue;

        if (G.UCap(a2)==0) continue;

        if (G.StartNode(a2)==G.EndNode(a2) && G.ArcLabelAnchor(a2)==NoNode) continue;

        DisplayArc(a);
    }

    for (TNode v=0;v<G.N();v++)
    {
        if (G.HiddenNode(v)) continue;

        DisplayNode(v);
    }

    if (CFG.legenda>0)
    {
        long xm = long(CFG.xShift+(maxX/2)*CFG.xZoom);
        long ym = long(CFG.yShift+maxY*CFG.yZoom+CFG.legenda*CFG.yZoom);

        DisplayLegenda(xm,ym,long(10*nodeSize));
    }
}


void canvasBuilder::DisplayArc(TArc a) throw()
{
    TNode u = G.StartNode(2*a);
    TNode v = G.EndNode(2*a);

    bool drawPred = false;

    if (CFG.predecessors && CFG.subgraph!=0)
    {
        int displayedPredecessorArrows = ARROW_NONE;

        if (G.Pred(v)==2*a)   displayedPredecessorArrows |= ARROW_FORWARD;
        if (G.Pred(u)==2*a+1) displayedPredecessorArrows |= ARROW_BACKWARD;

        if (displayedPredecessorArrows!=ARROW_NONE)
        {
            drawPred = true;
            WriteLine(a,u,v,LINE_STYLE_SOLID,9,
                TArrowDir(displayedPredecessorArrows),
                goblinController::PRED_COLOUR,150);
        }
    }

    TArrowDir displayedArrows = ArrowDirections(a);
    TArrowDir displayedExteriorArrows = ARROW_NONE;
    TArrowDir displayedCenteredArrows = ARROW_NONE;

    if (CFG.arrows & 1)
    {
        if (displayedArrows!=ARROW_BOTH)
        {
            displayedCenteredArrows = displayedArrows;
        }
    }
    else
    {
        displayedExteriorArrows = displayedArrows;
    }

    bool drawSub = false;

    if ((CFG.subgraph && (CFG.subgraph!=1 || G.Sub(2*a)!=0)) ||
        (CFG.predecessors && !CFG.subgraph && (G.Pred(v)==2*a || G.Pred(u)==(2*a+1)))
       )
    {
        drawSub = true;

        int width = 2;
        if ((CFG.subgraph==2 || CFG.subgraph==3) && G.Sub(2*a)==0) width = 1;
        if (CFG.subgraph==3 && G.Sub(2*a)==G.UCap(2*a)) width = 3;

        TLineStyle lineStyle = LINE_STYLE_SOLID;
        if (CFG.subgraph==2 && floor(G.Sub(2*a))!=G.Sub(2*a))
        {
            lineStyle = LINE_STYLE_DOT;
        }
        else if (CFG.subgraph==5)
        {
            lineStyle = TLineStyle(int(G.EdgeColour(2*a))%4);

            if (G.EdgeColour(2*a)!=NoArc)
            {
                width = 1+int(G.EdgeColour(2*a))/4;
            }
        }

        TIndex colourIndex = goblinController::OUTLINE_COLOUR;

        if (CFG.arcColours==2)
        {
            colourIndex = goblinController::ZERO_COLOUR
                        + G.EdgeColour(2*a)%(goblinController::MAX_COLOUR -goblinController::ZERO_COLOUR);
        }
        else if (CFG.arcColours>0)
        {
            colourIndex = goblinController::ZERO_COLOUR + G.EdgeColour(2*a);
        }

        WriteArc(a,u,v,lineStyle,width,displayedExteriorArrows,colourIndex,100);
    }

    if (drawSub && displayedCenteredArrows!=ARROW_NONE)
    {
        TNode y = G.ArcLabelAnchor(2*a);
        if (y!=NoNode) y = G.ThreadSuccessor(y);
        TNode x = u;

        while (y!=NoNode)
        {
            if (portMode==PORTS_IMPLICIT || x!=u)
            {
                if (displayedCenteredArrows==ARROW_FORWARD) DisplayArrow(a,x,y);
                if (displayedCenteredArrows==ARROW_BACKWARD) DisplayArrow(a,y,x);
            }

            x = y;
            y = G.ThreadSuccessor(y);
        }

        if (x!=v && portMode==PORTS_IMPLICIT)
        {
            if (displayedCenteredArrows==ARROW_FORWARD) DisplayArrow(a,x,v);
            if (displayedCenteredArrows==ARROW_BACKWARD) DisplayArrow(a,v,x);
        }
    }

    if ((CFG.arcLabels || CFG.arcLabels2 || CFG.arcLabels3 ||
         strcmp(CFG.arcFormatting,"")) &&
        (drawPred || drawSub) && (u!=v || G.ArcLabelAnchor(2*a)!=NoNode))
    {
        DisplayArcLabel(a,u,v,displayedCenteredArrows);
    }
}


void canvasBuilder::WriteArc(TArc a,TNode u,TNode v,TLineStyle lineStyle,int width,
    TArrowDir displayedArrows,TIndex colourIndex,int depth) throw()
{
    if (lineStyle==LINE_STYLE_SOLID)
    {
        WriteLine(a,u,v,lineStyle,width,displayedArrows,colourIndex,depth);
    }
    else
    {
        if (CFG.arcColours>0)
        {
            WriteLine(a,u,v,LINE_STYLE_SOLID,width,displayedArrows,colourIndex,depth+1);
        }

        WriteLine(a,u,v,lineStyle,width,displayedArrows,goblinController::OUTLINE_COLOUR,depth);
    }
}


canvasBuilder::TArrowDir canvasBuilder::ArrowDirections(TArc a) throw()
{
   int arrowDisplayMode = (CFG.arrows & ~1)>>2;

    switch (arrowDisplayMode)
    {
        case 1:
        {
            return ARROW_NONE;
        }
        case 2:
        {
            return ARROW_FORWARD;
        }
        case 3:
        {
            if (G.Sub(2*a)>0)
            {
                return ARROW_FORWARD;
            }
            else if (G.Sub(2*a)<0)
            {
                return ARROW_BACKWARD;
            }
            else
            {
                return ARROW_NONE;
            }
        }
        default:
        case 0:
        {
            return TArrowDir(G.Orientation(2*a));
        }
    }
}


void canvasBuilder::DisplayArrow(TArc a,TNode u,TNode v) throw()
{
    double dx = CFG.xZoom*(CX(v)-CX(u));
    double dy = CFG.yZoom*(CY(v)-CY(u));
    double norm = sqrt(dx*dx+dy*dy);

    if (fabs(norm)<CFG.bendSep*0.5) return;

    dx = dx/norm;
    dy = dy/norm;

    WriteArrow(a,
        long((CanvasCX(u)+CanvasCX(v))/2+arrowSize*dx),
        long((CanvasCY(u)+CanvasCY(v))/2+arrowSize*dy),
        dx,dy);
}


void canvasBuilder::DisplayArcLabel(TArc a,TNode u,TNode v,TArrowDir displayedCenteredArrows) throw()
{
    long xm = 0;
    long ym = 0;
    TNode p = G.ArcLabelAnchor(2*a);

    if (p==NoNode)
    {
        double dx = CFG.xZoom*(CX(v)-CX(u));
        double dy = CFG.yZoom*(CY(v)-CY(u));
        double norm = sqrt(dx*dx+dy*dy);

        if (fabs(norm)<CFG.bendSep*0.5) return;

        double textShift = arrowSize;

        if (displayedCenteredArrows!=ARROW_NONE)
        {
            textShift = 2*arrowSize;
        }

        dx = dx/norm;
        dy = dy/norm;
        xm = long((CanvasCX(u)+CanvasCX(v))/2+textShift*dy);
        ym = long((CanvasCY(u)+CanvasCY(v))/2-textShift*dx);
    }
    else
    {
        xm = CanvasCX(p);
        ym = CanvasCY(p);
    }

    WriteArcLabel(a,xm,ym);
}


void canvasBuilder::ComposeArcLabel(TArc a,ofstream& expFile) throw()
{
    if (strcmp(CFG.arcFormatting,"")==0)
    {
        bool first = true;

        if (CFG.arcLabels)
        {
            expFile << ArcLabel(a,CFG.arcLabels);
            first = false;
        }

        if (CFG.arcLabels2)
        {
            if (!first) expFile << "/";

            expFile << ArcLabel(a,CFG.arcLabels2);
            first = false;
        }

        if (CFG.arcLabels3)
        {
            if (!first) expFile << "/";

            expFile << ArcLabel(a,CFG.arcLabels3);
        }
    }
    else 
    {
        for (unsigned i=0;i<strlen(CFG.arcFormatting);i++)
        {
            if (CFG.arcFormatting[i]=='#' && i<strlen(CFG.arcFormatting)-1)
            {
                expFile << ArcLabel(a,int(CFG.arcFormatting[i+1]-'0'));
                i++;
                continue;
            }

            if (CFG.arcFormatting[i]!='%' || i==strlen(CFG.arcFormatting)-1)
            {
                expFile << char(CFG.arcFormatting[i]);
                continue;
            }

            switch (CFG.arcFormatting[i+1])
            {
                case '1':
                {
                    expFile << ArcLabel(a,CFG.arcLabels);
                    i++;
                    break;
                }
                case '2':
                {
                    expFile << ArcLabel(a,CFG.arcLabels2);
                    i++;
                    break;
                }
                case '3':
                {
                    expFile << ArcLabel(a,CFG.arcLabels3);
                    i++;
                    break;
                }
                default:
                {
                    expFile << char(CFG.arcFormatting[i]);
                }
            }
        }
    }
}


char* canvasBuilder::ArcLabel(TArc a,int option) throw(ERRejected)
{
    switch (option)
    {
        case 0:
        {
            strcpy(label,"");
            break;
        }
        case 1:
        {
            sprintf(label,"%ld",a);
            break;
        }
        case 2:
        {
            if (G.UCap(2*a)==InfCap) return "*";
            sprintf(label,"%g",G.UCap(2*a));
            break;
        }
        case 3:
        {
            sprintf(label,"%g",fabs(G.Sub(2*a)));
            break;
        }
        case 4:
        {
            if (G.Length(2*a)==InfFloat || G.Length(2*a)==-InfFloat) return "*";
            sprintf(label,"%g",G.Length(2*a));
            break;
        }
        case 5:
        {
            sprintf(label,"%g",G.RedLength(piG,2*a));
            break;
        }
        case 6:
        {
            sprintf(label,"%g",G.LCap(2*a));
            break;
        }
        case 7:
        {
            sprintf(label,"%ld",a+1);
            break;
        }
        case 8:
        {
            if (G.EdgeColour(2*a)==NoArc) sprintf(label,"*");
            else sprintf(label,"%ld",G.EdgeColour(2*a));
            break;
        }
        default:
        {
            UnknownOption("ArcLabel",option);
        }
    }

    return label;
}


void canvasBuilder::ComposeNodeLabel(TNode v,ofstream& expFile) throw()
{
    if (strcmp(CFG.nodeFormatting,"")==0)
    {
        expFile << NodeLabel(v,CFG.nodeLabels);
    }
    else
    {
        for (unsigned i=0;i<strlen(CFG.nodeFormatting);i++)
        {
            if (CFG.nodeFormatting[i]=='#' && i<strlen(CFG.nodeFormatting)-1)
            {
                expFile << NodeLabel(v,int(CFG.nodeFormatting[i+1]-'0'));
                i++;
                continue;
            }

            if (CFG.nodeFormatting[i]!='%' || i==strlen(CFG.nodeFormatting)-1)
            {
                expFile << char(CFG.nodeFormatting[i]);
                continue;
            }

            switch (CFG.nodeFormatting[i+1])
            {
                case '1':
                {
                    expFile << NodeLabel(v,CFG.nodeLabels);
                    i++;
                    break;
                }
                default:
                {
                    expFile << char(CFG.nodeFormatting[i]);
                }
            }
        }
    }
}


char* canvasBuilder::NodeLabel(TNode v,int option) throw(ERRejected)
{
    switch (option)
    {
        case 0:
        {
            strcpy(label,"");
            break;
        }
        case 1:
        {
            sprintf(label,"%ld",v);
            break;
        }
        case 2:
        {
            if (G.Dist(v)==InfFloat || G.Dist(v)==-InfFloat) return "*";
            sprintf(label,"%g",G.Dist(v));
            break;
        }
        case 3:
        {
            if (G.Pi(v)==InfFloat || G.Pi(v)==-InfFloat) return "*";
            sprintf(label,"%g",G.Pi(v));
            break;
        }
        case 4:
        {
            if (G.NodeColour(v)==NoNode) return "*";
            sprintf(label,"%ld",G.NodeColour(v));
            break;
        }
        case 5:
        {
            sprintf(label,"%g",G.Demand(v));
            break;
        }
        case 6:
        {
            sprintf(label,"%ld",v+1);
            break;
        }
        default:
        {
            UnknownOption("NodeLabel",option);
        }
    }

    return label;
}


char* canvasBuilder::ArcLegenda(int option) throw(ERRejected)
{
    switch (option)
    {
        case 0:
        {
            strcpy(label,"");
            break;
        }
        case 1:
        case 7:
        {
            sprintf(label,"a");
            break;
        }
        case 2:
        {
            sprintf(label,"ucap");
            break;
        }
        case 3:
        {
            sprintf(label,"x");
            break;
        }
        case 4:
        {
            sprintf(label,"length");
            break;
        }
        case 5:
        {
            sprintf(label,"redlength");
            break;
        }
        case 6:
        {
            sprintf(label,"lcap");
            break;
        }
        case 8:
        {
            sprintf(label,"colour");
            break;
        }
        default:
        {
            UnknownOption("ArcLegenda",option);
        }
    }

    return label;
}


char* canvasBuilder::NodeLegenda(char* index,int option) throw(ERRejected)
{
    switch (option)
    {
        case 1:
        case 6:
        {
            sprintf(label,"%s",index);
            break;
        }
        case 2:
        {
            sprintf(label,"d(%s)",index);
            break;
        }
        case 3:
        {
            sprintf(label,"pi(%s)",index);
            break;
        }
        case 4:
        {
            sprintf(label,"colour(%s)",index);
            break;
        }
        case 5:
        {
            sprintf(label,"B(%s)",index);
            break;
        }
        default:
        {
            UnknownOption("NodeLegenda",option);
        }
    }

    return label;
}


char* canvasBuilder::FixedNodeColour(TIndex c) throw()
{
    if (c==NoNode)
    {
        sprintf(nodeColour,"#%06lX",CT.Colour(goblinController::NO_COLOUR));
    }
    else
    {
        TNode c0 = goblinController::ZERO_COLOUR + c;

        sprintf(nodeColour,"#%06lX",CT.Colour(c0));
    }

    return nodeColour;
}


char* canvasBuilder::FixedEdgeColour(TIndex c) throw()
{
    if (c==NoArc)
    {
        // Other than for node colours, NO_COLOUR does not apply since this is
        // displayed white. Use some kind of grey here
        sprintf(nodeColour,"#%06lX",CT.Colour(goblinController::MAX_COLOUR));
    }
    else
    {
        TNode c0 = goblinController::ZERO_COLOUR + c;

        sprintf(nodeColour,"#%06lX",CT.Colour(c0));
    }

    return nodeColour;
}


char* canvasBuilder::SmoothColour(TIndex c,TIndex maxColour,goblinController::TColourDummy undefColour)
    throw()
{
    if (c>maxColour)
    {
        sprintf(nodeColour,"#%06lX",CT.Colour(undefColour));
        return nodeColour;
    }

    double ratio = double(c)/double(maxColour+1);

    long unsigned rSat = 100;
    long unsigned bSat = 100;
    long unsigned gSat = 100;

    if (ratio<0.333)
    {
        rSat = (long unsigned)(ceil(255*3*(0.333-ratio)));
        gSat = (long unsigned)(ceil(255*3*ratio));
    }
    else if (ratio<0.667)
    {
        gSat = (long unsigned)(ceil(254*3*(0.667-ratio)));
        bSat = (long unsigned)(ceil(254*3*(ratio-0.333)));
    }
    else
    {
        bSat = (long unsigned)(ceil(255*3*(1-ratio)));
        rSat = (long unsigned)(ceil(255*3*(ratio-0.667)));
    }

    sprintf(nodeColour,"#%02lX%02lX%02lX",rSat,gSat,bSat);

    return nodeColour;
}
