
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, August 2000
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file   exportToXFig.cpp
/// \brief  #exportToXFig class implementation

#include "exportToXFig.h"


exportToXFig::exportToXFig(const abstractMixedGraph& GG,const char* expFileName)
    throw(ERFile) : canvasBuilder(GG), expFile(expFileName, ios::out)
{
    if (!expFile)
    {
        sprintf(CT.logBuffer,"Could not open export file %s, io_state %d",
            expFileName,expFile.rdstate());
        Error(ERR_FILE,"exportToXFig",CT.logBuffer);
    }

    expFile.flags(expFile.flags() | ios::right);
    expFile.setf(ios::floatfield);
    expFile.precision(5);

    expFile << "#FIG 3.2" << endl;
    expFile << "#File generated by the GOBLIN library (Release "
        << CT.majorVersion << "." << CT.minorVersion
        << CT.patchLevel << ")" << endl;

    if (maxX<=maxY)
    {
        expFile << "Portrait" << endl;
    }
    else
    {
        expFile << "Landscape" << endl;
    }

    expFile << "Center" << endl;
    expFile << "Metric" << endl;
    expFile << "A4" << endl;
    expFile << "100.00" << endl;
    expFile << "Single" << endl;
    expFile << "-2" << endl;
    expFile << "1200 2" << endl;

    // Colours

    expFile << "0 32 " << predColour << endl;
    expFile << "0 33 " << inftyColour << endl;
    expFile << "0 34 " << FixedNodeColour(NoNode) << endl;

    TNode maxColourIndex =
        goblinController::MAX_COLOUR-goblinController::NO_COLOUR-1;

    if (CFG.nodeColours==4)
    {
        if (maxNodeColour<254)
        {
            for (TNode i=0;i<=maxNodeColour;i++)
            {
                expFile << "0 " << (35+i) << " "
                    << SmoothColour(i,maxNodeColour,goblinController::NO_COLOUR) << endl;
            }
        }
        else
        {
            for (TNode i=0;i<254;i++)
            {
                expFile << "0 " << (35+i) << " "
                    << SmoothColour(i,253,goblinController::NO_COLOUR) << endl;
            }
        }
    }
    else
    {
        for (TNode i=0;i<=maxColourIndex;i++)
        {
            expFile << "0 " << (35+i) << " " << FixedNodeColour(i) << endl;
        }
    }

    if (CFG.arcColours==3)
    {
        if (maxEdgeColour<254)
        {
            for (TNode i=0;i<=maxEdgeColour;i++)
            {
                expFile << "0 " << (35+254+i) << " "
                    << SmoothColour(i,maxEdgeColour,goblinController::MAX_COLOUR) << endl;
            }
        }
        else
        {
            for (TNode i=0;i<254;i++)
            {
                expFile << "0 " << (35+254+i) << " "
                    << SmoothColour(i,253,goblinController::MAX_COLOUR) << endl;
            }
        }
    }
    else
    {
        for (TNode i=0;i<=maxColourIndex;i++)
        {
            expFile << "0 " << (35+254+i) << " " << FixedEdgeColour(i) << endl;
        }
    }

    // Set magnification ratios
    if (maxX<=maxY)
    {
        // Portrait mode
        ScaleDisplayParameters(1.0,21000,30000);
    }
    else
    {
        // Landscape mode
        ScaleDisplayParameters(1.0,30000,21000);
    }


    // Bounding Box
    double margin = CFG.nodeSep;

    expFile << "2 1 0 0 0 7 150 0 -1 0.000 0 0 -1 0 0 5" << endl;
    expFile << "     "
        << int(CFG.xShift+(minX-margin)*CFG.xZoom) << " "
        << int(CFG.yShift+(minY-margin)*CFG.yZoom) << " "
        << int(CFG.xShift+(maxX+margin)*CFG.xZoom) << " "
        << int(CFG.yShift+(minY-margin)*CFG.yZoom) << " "
        << int(CFG.xShift+(maxX+margin)*CFG.xZoom) << " "
        << int(CFG.yShift+(maxY+CFG.legenda+margin)*CFG.yZoom) << " "
        << int(CFG.xShift+(minX-margin)*CFG.xZoom) << " "
        << int(CFG.yShift+(maxY+CFG.legenda+margin)*CFG.yZoom) << " "
        << int(CFG.xShift+(minX-margin)*CFG.xZoom) << " "
        << int(CFG.yShift+(minY-margin)*CFG.yZoom) << endl;

    // Merge the graph into a compound object

    expFile << "6 "
        << int(CFG.xShift+(minX-margin)*CFG.xZoom) << " "
        << int(CFG.yShift+(minY-margin)*CFG.yZoom) << " "
        << int(CFG.xShift+(maxX+margin)*CFG.xZoom) << " "
        << int(CFG.yShift+(maxY+margin)*CFG.yZoom)
        << endl;
}


unsigned long exportToXFig::Size() const throw()
{
    return
          sizeof(exportToXFig)
        + managedObject::Allocated();
}


unsigned long exportToXFig::Allocated() const throw()
{
    return 0;
}


void exportToXFig::DisplayLegenda(long xm,long ym,long radius) throw()
{
    int xl = xm-radius;
    int xr = xm+radius;
    int fillColour = 7;
    int forwardArrow = !(CFG.arrows & 1) && G.Blocking(1);

    // Complete the graph compound object

    expFile << "-6" << endl;


    // Merge the legenda into a nested compound object

    expFile << "6 "
        << xl-int(nodeSize) << " " << ym-int(nodeSize) << " "
        << xr+int(nodeSize) << " " << ym+int(nodeSize) << " "
        << endl;

    expFile << "2 1 0 2 0 7 100 0 -1 0.000 0 0 -1 " << forwardArrow
        << " 0 2" << endl;
    if (forwardArrow) expFile << "2 1 3.00 120.00 150.00" << endl;
    expFile << "     "
        << xl+int(nodeSize) << " " << ym << " "
        << xr-int(nodeSize) << " " << ym << " " << endl;

    int textShift = int(arrowSize);
    if (!G.IsUndirected() && (CFG.arrows & 1))
    {
        WriteArrow(NoArc,xm+int(arrowSize),ym,1,0);
        textShift = int(2*arrowSize);
    }

    if (CFG.arcLabels || CFG.arcLabels2 || CFG.arcLabels3 ||
         strcmp(CFG.arcFormatting,"")!=0)
    {
        int radius = int(nodeSize);
        expFile << "4 1 0 30 0 2 12.000 0.000 0 "
            << 2*radius << " " << 2*radius << " "
            << xm << " ";

        expFile << ym+75-textShift << " ";

        if (strcmp(CFG.arcFormatting,"")==0)
        {
            bool first = true;

            if (CFG.arcLabels)
            {
                expFile << ArcLegenda(CFG.arcLabels);
                first = false;
            }

            if (CFG.arcLabels2)
            {
                if (!first) expFile << "/";

                expFile << ArcLegenda(CFG.arcLabels2);
                first = false;
            }

            if (CFG.arcLabels3)
            {
                if (!first) expFile << "/";

                expFile << ArcLegenda(CFG.arcLabels3);
            }
        }
        else 
        {
            for (unsigned i=0;i<strlen(CFG.arcFormatting);i++)
            {
                if (CFG.arcFormatting[i]=='#' && i<strlen(CFG.arcFormatting)-1)
                {
                    expFile << ArcLegenda(int(CFG.arcFormatting[i+1]-'0'));
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
                        expFile << ArcLegenda(CFG.arcLabels);
                        i++;
                        break;
                    }
                    case '2':
                    {
                        expFile << ArcLegenda(CFG.arcLabels2);
                        i++;
                        break;
                    }
                    case '3':
                    {
                        expFile << ArcLegenda(CFG.arcLabels3);
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

        expFile << "\\001" << endl;
    }

    switch (CFG.nodeStyle)
    {
         case 0:
         {
             WriteSmallNode(NoNode,xl,ym,fillColour);
             WriteSmallNode(NoNode,xr,ym,fillColour);
             WriteNodeLegenda(xl,ym+int(nodeSize),"u");
             WriteNodeLegenda(xr,ym+int(nodeSize),"v");

             return;
         }
         case 1:
         {
             WriteCircularNode(NoNode,xl,ym,fillColour);
             WriteCircularNode(NoNode,xr,ym,fillColour);
             WriteNodeLegenda(xl,ym,"u");
             WriteNodeLegenda(xr,ym,"v");

             return;
         }
         case 2:
         {
             WriteRectangularNode(NoNode,xl,ym,fillColour);
             WriteRectangularNode(NoNode,xr,ym,fillColour);
             WriteNodeLegenda(xl,ym,"u");
             WriteNodeLegenda(xr,ym,"v");

             return;
         }
    }
}


void exportToXFig::WriteNodeLegenda(int x,int y,char* index) throw()
{
    if (strcmp(CFG.nodeFormatting,"")==0 && CFG.nodeLabels==0) return;

    int radius = 100;
    expFile << "4 1 0 30 0 2 14.000 0.000 0 "
        << 2*radius << " " << 2*radius << " "
        << x << " " << y+75 << " ";

    if (strcmp(CFG.nodeFormatting,"")==0)
    {
        expFile << NodeLegenda(index,CFG.nodeLabels);
    }
    else 
    {
        for (unsigned i=0;i<strlen(CFG.nodeFormatting);i++)
        {
            if (CFG.nodeFormatting[i]=='#' && i<strlen(CFG.nodeFormatting)-1)
            {
                expFile << NodeLegenda(index,int(CFG.nodeFormatting[i+1]-'0'));
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
                    expFile << NodeLegenda(index,CFG.nodeLabels);
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

    expFile << "\\001" << endl;
}


void exportToXFig::WriteLine(TArc a,TNode u,TNode v,TLineStyle lineStyle,int thickness,
    TArrowDir displayedArrows,TIndex colourIndex,int depth) throw()
{
    int figColour = 0;

    if (colourIndex==goblinController::OUTLINE_COLOUR)
    {
        figColour = 0;
    }
    else if (colourIndex==goblinController::PRED_COLOUR)
    {
        figColour = 32;
    }
    else
    {
        TIndex edgeColour = colourIndex - goblinController::ZERO_COLOUR;

        if (   (CFG.arcColours==3 && edgeColour>253)
            || (CFG.arcColours==1 && colourIndex>=goblinController::MAX_COLOUR)
           )
        {
            figColour = 0;
        }
        else
        {
            figColour = edgeColour+35+254;
        }
    }

    float gap = 0.000;
    if (lineStyle!=0) gap = 10.000;

    int forwardArrow = (displayedArrows & ARROW_FORWARD);
    int backwardArrow = (displayedArrows & ARROW_BACKWARD);

    TNode w = G.ArcLabelAnchor(2*a);
    if (w!=NoNode) w = G.ThreadSuccessor(w);
    TNode x = w;

    int nPoints = (portMode==PORTS_IMPLICIT) ? 2 : 0;
    while (x!=NoNode)
    {
        nPoints++;
        x = G.ThreadSuccessor(x);
    }

    if (CFG.arcStyle==1)    // Spline
    {
        expFile << "3 4 " << int(lineStyle) << " " << thickness << " "
            << figColour << " 7 " << depth << " 0 -1 "
            << gap << " 0 " << forwardArrow << " " << backwardArrow << " "
            << nPoints << endl;
    }
    else                // Polygon
    {
        expFile << "2 1 " << int(lineStyle) << " " << thickness << " "
            << figColour << " 7 " << depth << " 0 -1 "
            << gap << " 0 0 -1 " << forwardArrow << " " << backwardArrow << " "
            << nPoints << endl;
    }

    if (forwardArrow)
    {
        expFile << "2 1 " << thickness << " "
            <<       int(arrowSize*1.8/sqrt(double(thickness)))
            << " " << int(arrowSize*1.8/sqrt(double(thickness))) << endl;
    }

    if (backwardArrow)
    {
        expFile << "2 1 " << thickness << " "
            <<       int(arrowSize*1.8/sqrt(double(thickness)))
            << " " << int(arrowSize*1.8/sqrt(double(thickness))) << endl;
    }

    expFile << "     ";

    if (w==NoNode)
        expFile << PortCX(v,u) << " " << PortCY(v,u) << " "
            << PortCX(u,v) << " " << PortCY(u,v) << " ";
    else
    {
        if (portMode==PORTS_IMPLICIT)
        {
            expFile << PortCX(w,u) << " " << PortCY(w,u) << " ";
        }

        x = w;
        TNode y = w;
        while (x!=NoNode)
        {
            expFile << CanvasCX(x) << " " << CanvasCY(x) << " ";
            y = x;
            x = G.ThreadSuccessor(x);
        }

        if (portMode==PORTS_IMPLICIT)
        {
            expFile << PortCX(y,v) << " " << PortCY(y,v) << " ";
        }
    }

    expFile << endl;

    if (CFG.arcStyle==1)    // Spline
    {
        expFile << "     0.000";
        x = w;

        while (x!=NoNode)
        {
            expFile << " 1.000";
            x = G.ThreadSuccessor(x);
        }

        expFile << " 0.000" << endl;
    }
}


void exportToXFig::WriteArrow(TArc,long xtop,long ytop,double dx,double dy) throw()
{
    double ox = dy;
    double oy = -dx;

    expFile << "2 3 0 1 0 0 100 0 20 0.000 1 0 -1 0 0 5" << endl;
    expFile << "     "
        << int(xtop) << " "
        << int(ytop) << " "
        << int(xtop-(1.8*dx+0.7*ox)*arrowSize) << " "
        << int(ytop-(1.8*dy+0.7*oy)*arrowSize) << " "
        << int(xtop-1.4*dx*arrowSize) << " "
        << int(ytop-1.4*dy*arrowSize) << " "
        << int(xtop-(1.8*dx-0.7*ox)*arrowSize) << " "
        << int(ytop-(1.8*dy-0.7*oy)*arrowSize) << " "
        << int(xtop) << " "
        << int(ytop) << endl;
}


void exportToXFig::WriteArcLabel(TArc a,long xm,long ym) throw()
{
    int radius = int(nodeSize);

    expFile << "4 1 0 90 0 2 12.000 0.000 0 "
        << 2*radius << " " << 2*radius << " "
        << int(xm) << " " << int(ym+75) << " ";

    ComposeArcLabel(a,expFile);

    expFile << "\\001" << endl;
}


void exportToXFig::DisplayNode(TNode v) throw()
{
    int fillColour = 7;
    if (CFG.nodeColours==1)
    {
        if (G.Dist(v)==InfFloat || G.Dist(v)==-InfFloat) fillColour = 33;
    }

    if (CFG.nodeColours==2)
    {
        TNode maxColourIndex =
            goblinController::MAX_COLOUR-goblinController::NO_COLOUR-1;

        fillColour = 35+maxColourIndex;
        if (G.NodeColour(v)<=maxColourIndex) fillColour = 35+G.NodeColour(v);
    }

    if (CFG.nodeColours==3)
    {
        if (G.Demand(v)>0) fillColour = 36;
        if (G.Demand(v)<0) fillColour = 37;
    }

    if (CFG.nodeColours==4)
    {
        fillColour = G.NodeColour(v);
        if (fillColour>253) fillColour = 34;
        else fillColour += 35;
    }

    int x = CanvasCX(v);
    int y = CanvasCY(v);

    switch (CFG.nodeStyle)
    {
        case 0:
        {
            WriteSmallNode(v,x,y,fillColour);
            TNode p = G.ThreadSuccessor(v);

            if (p==NoNode)
            {
                WriteNodeLabel(v,x+int(nodeSize),y+int(nodeSize));
            }
            else
            {
                WriteNodeLabel(v,CanvasCX(p),CanvasCY(p));
            }

            return;
         }
         case 1:
         {
             WriteCircularNode(v,x,y,fillColour);
             WriteNodeLabel(v,x,y);

             return;
         }
         case 2:
         {
             WriteRectangularNode(v,x,y,fillColour);
             WriteNodeLabel(v,x,y);

             return;
         }
    }
}


void exportToXFig::WriteSmallNode(TNode v,int x,int y,int fillColour) throw()
{
    int radius = 60;

    expFile << "1 3 0 4 0 0 50 0 20 0.000 1 0.0000 "
        << x << " " << y << " "
        << radius << " " << radius << " "
        << x-radius << " " << y << " "
        << x+radius  << " " << y << endl;
}


void exportToXFig::WriteCircularNode(TNode v,int x,int y,int fillColour) throw()
{
    double radius = nodeSize;

    if (v!=NoNode && G.ThreadSuccessor(v)!=NoNode)
    {
        TNode w = G.ThreadSuccessor(v);
        radius += sqrt(G.C(w,0)*G.C(w,0)+G.C(w,1)*G.C(w,1))*(CFG.xZoom+CFG.yZoom)/2;
    }

    expFile << "1 3 0 1 0 " << fillColour
        << " 50 0 20 0.000 1 0.0000 "
        << x << " " << y << " "
        << RoundToInt(radius) << " " << RoundToInt(radius) << " "
        << RoundToInt(x-radius) << " " << y << " "
        << RoundToInt(x+radius) << " " << y << endl;
}


void exportToXFig::WriteRectangularNode(TNode v,int x,int y,int fillColour) throw()
{
    double width  = nodeSize;
    double height = nodeSize;

    if (v!=NoNode && G.ThreadSuccessor(v)!=NoNode)
    {
        TNode w = G.ThreadSuccessor(v);

        width  += G.C(w,0)*CFG.xZoom;
        height += G.C(w,1)*CFG.yZoom;
    }

    expFile << "2 2 0 1 0 " << fillColour
        << " 50 0 20 0.000 0 0 0 0 0 5 " << endl
        << RoundToInt(x-width) << " " << RoundToInt(y-height) << " "
        << RoundToInt(x-width) << " " << RoundToInt(y+height) << " "
        << RoundToInt(x+width) << " " << RoundToInt(y+height) << " "
        << RoundToInt(x+width) << " " << RoundToInt(y-height) << " "
        << RoundToInt(x-width) << " " << RoundToInt(y-height) << " " << endl;
}


void exportToXFig::WriteNodeLabel(TNode v,int x,int y) throw()
{
    if (strcmp(CFG.nodeFormatting,"")==0 && CFG.nodeLabels==0) return;

    int radius = 100;
    expFile << "4 1 0 30 0 2 14.000 0.000 0 "
        << 2*radius << " " << 2*radius << " "
        << x << " " << y+75 << " ";

    ComposeNodeLabel(v,expFile);

    expFile << "\\001" << endl;
}


exportToXFig::~exportToXFig() throw()
{
    expFile << endl;
    expFile.close();
}