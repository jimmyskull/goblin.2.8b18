
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, May 2001
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file   exportToTk.cpp
/// \brief  #exportToTk class implementation

#include "exportToTk.h"


exportToTk::exportToTk(const abstractMixedGraph& GG,const char* expFileName)
    throw(ERFile) : canvasBuilder(GG), expFile(expFileName, ios::out)
{
    if (!expFile)
    {
        sprintf(CT.logBuffer,"Could not open export file %s, io_state %d",
            expFileName,expFile.rdstate());
        Error(ERR_FILE,"exportToTk",CT.logBuffer);
    }

    expFile.flags(expFile.flags() | ios::right);
    expFile.setf(ios::floatfield);
    expFile.precision(5);

    canvasName = "$goblinCanvas";
    smallFont = "-adobe-times-bold-r-normal--12-0-0-0-p-0-iso8859-1";
    largeFont = "-adobe-times-bold-r-normal--14-0-0-0-p-0-iso8859-1";

    ScaleDisplayParameters(0.073,10e+6,10e+6);

    double margin = CFG.nodeSep;

    expFile << "set goblinCanvasObjects {" << endl;
    expFile << "  {-1 " << ID_UPPER_LEFT << " line {"
        << int(CFG.xShift+(minX-margin)*CFG.xZoom) << " "
        << int(CFG.yShift+(minY-margin)*CFG.yZoom) << "} {} } \\" << endl;
    expFile << "  {-1 " << ID_LOWER_RIGHT << " line {"
        << int(CFG.xShift+(maxX+margin)*CFG.xZoom) << " "
        << int(CFG.yShift+(maxY+CFG.legenda+margin)*CFG.yZoom)
        << "} {} } \\" << endl;

    if (strcmp(CFG.wallpaper,"")!=0)
    {
//        expFile << "image create photo wallpaper -file \""
//            << CFG.wallpaper << "\"" << endl;
//        expFile << "$goblinCanvas create image 0 0 -image wallpaper"
//            << " -anchor nw" << endl;
    }
}


unsigned long exportToTk::Size() const throw()
{
    return
          sizeof(exportToTk)
        + managedObject::Allocated();
}


unsigned long exportToTk::Allocated() const throw()
{
    return 0;
}


void exportToTk::DisplayLegenda(long xm,long ym,long radius) throw()
{
    long xl = xm-radius;
    long xr = xm+radius;

    int textShift = int(arrowSize);
    if (!G.IsUndirected())
    {
        expFile << "  {-1 " << ID_GRAPH_EDGE << " line {"
            << xl << " " << ym << " " << xr-long(nodeSize) << " " << ym
            << "} {-width 2 -joinstyle bevel";

        if (CFG.arrows & 1)
        {
            expFile << "} } \\" << endl;
            WriteArrow(NoNode,xm+long(arrowSize),ym,1,0);
            textShift = long(2*arrowSize);
        }
        else
        {
            expFile << " -arrow last -arrowshape {"
                << long(arrowSize*2) << " " << long(arrowSize*3)
                << " " << long(arrowSize*1) << "}" << "} } \\" << endl;
        }
    }
    else
    {
        expFile << "  {-1 " << ID_GRAPH_EDGE << " line {"
            << xl << " " << ym << " " << xr-long(nodeSize) << " " << ym
            << "} {-width 2 -joinstyle bevel} } \\" << endl;
    }

    if (CFG.arcLabels || CFG.arcLabels2 || CFG.arcLabels3 ||
         strcmp(CFG.arcFormatting,"")!=0)
    {
        expFile << "  {-1 " << ID_EDGE_LABEL << " text {"
            << xm << " " << (ym-textShift) << "} {-text {";

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

        expFile << "} -anchor c -font " << smallFont << "} } \\" << endl;
    }

    switch (CFG.nodeStyle)
    {
        case 0:
        {
            WriteSmallNode(NoNode,xl,ym);
            WriteSmallNode(NoNode,xr,ym);
            WriteNodeLegenda(xl,ym+long(nodeSize),"u");
            WriteNodeLegenda(xr,ym+long(nodeSize),"v");

            return;
        }
        case 1:
        {
            WriteCircularNode(NoNode,xl,ym,"#ffffff");
            WriteCircularNode(NoNode,xr,ym,"#ffffff");
            WriteNodeLegenda(xl,ym,"u");
            WriteNodeLegenda(xr,ym,"v");

            return;
        }
        case 2:
        {
            WriteRectangularNode(NoNode,xl,ym,"#ffffff");
            WriteRectangularNode(NoNode,xr,ym,"#ffffff");
            WriteNodeLegenda(xl,ym,"u");
            WriteNodeLegenda(xr,ym,"v");

            return;
        }
    }
}


void exportToTk::WriteNodeLegenda(long x,long y,char* index) throw()
{
    if (strcmp(CFG.nodeFormatting,"")==0 && CFG.nodeLabels==0) return;

    expFile << "  {-1 " << ID_NODE_LABEL << " text {"
        << x << " " << y << "} {-text {";

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

    expFile << "} -anchor c -font " << largeFont << "} } \\" << endl;
}


void exportToTk::WriteLine(TArc a,TNode u,TNode v,TLineStyle lineStyle,int width,
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

    TNode w = G.ArcLabelAnchor(2*a);

    if (w!=NoNode) w = G.ThreadSuccessor(w);

    expFile << "  {" << a << " " << ID_GRAPH_EDGE << " line {";

    if (w==NoNode)
    {
        expFile << PortCX(v,u) << " " << PortCY(v,u) << " "
            << PortCX(u,v) << " " << PortCY(u,v) << " ";
    }
    else
    {
        if (portMode==PORTS_IMPLICIT)
        {
            expFile << PortCX(w,u) << " " << PortCY(w,u) << " ";
        }

        TNode x = w;
        while (x!=NoNode)
        {
            expFile << CanvasCX(x) << " " << CanvasCY(x) << " ";
            w = x;
            x = G.ThreadSuccessor(x);
        }

        if (portMode==PORTS_IMPLICIT)
        {
            expFile << PortCX(w,v) << " " << PortCY(w,v) << " ";
        }
    }

    expFile << "} {"; 

    if (CFG.arcStyle==1) expFile << " -smooth on";

    if (lineStyle==LINE_STYLE_DOT)
    {
        expFile << " -dash .";
    }
    else if (lineStyle==LINE_STYLE_DASH)
    {
        expFile << " -dash -";
    }
    else if (lineStyle==LINE_STYLE_DASH_DOT)
    {
        expFile << " -dash -.";
    }

    if (displayedArrows!=ARROW_NONE)
    {
        if (displayedArrows==ARROW_FORWARD)
        {
            expFile << " -arrow last";
        }
        else if (displayedArrows==ARROW_BACKWARD)
        {
            expFile << " -arrow first";
        }
        else
        {
            expFile << " -arrow both";
        }

        expFile << " -arrowshape {"
                << long(arrowSize*2) << " " << long(arrowSize*3)
                << " " << long(arrowSize*1) << "}";
    }

    expFile << " -joinstyle bevel -width " << width
        << " -fill " << rgbColour << "} } \\" << endl;
}


void exportToTk::WriteArrow(TArc a,long xtop,long ytop,double dx,double dy) throw()
{
    double ox = dy;
    double oy = -dx;

    expFile << "  {" << a << " " << ID_ARROW << " polygon {"
        << long(xtop) << " "
        << long(ytop) << " "
        << long(xtop-(2*dx+ox)*arrowSize) << " "
        << long(ytop-(2*dy+oy)*arrowSize) << " "
        << long(xtop-dx*arrowSize) << " "
        << long(ytop-dy*arrowSize) << " "
        << long(xtop-(2*dx-ox)*arrowSize) << " "
        << long(ytop-(2*dy-oy)*arrowSize) << " "
        << long(xtop) << " "
        << long(ytop)
        << "} {-outline #000000 -fill #000000} } \\" << endl;
}


void exportToTk::WriteArcLabel(TArc a,long xm,long ym) throw()
{
    expFile << "  {" << a << " " << ID_EDGE_LABEL << " text {"
        << xm << " " << ym << "} {-text {";

    ComposeArcLabel(a,expFile);

    expFile << "} -anchor c -font " << smallFont << "} } \\" << endl;
}


void exportToTk::DisplayNode(TNode v) throw()
{
    char* fillColour = "#ffffff";
    if (CFG.nodeColours==1 && (G.Dist(v)==InfFloat || G.Dist(v)==-InfFloat))
        fillColour = inftyColour;

    if (CFG.nodeColours==2) fillColour = FixedNodeColour(G.NodeColour(v));

    if (CFG.nodeColours==3 && G.Demand(v)>0) fillColour = FixedNodeColour(1);
    if (CFG.nodeColours==3 && G.Demand(v)<0) fillColour = FixedNodeColour(2);

    if (CFG.nodeColours==4)
        fillColour = SmoothColour(G.NodeColour(v),maxNodeColour,goblinController::NO_COLOUR);

    long x = CanvasCX(v);
    long y = CanvasCY(v);

    switch (CFG.nodeStyle)
    {
        case 0:
        {
            WriteSmallNode(v,x,y);
            TNode p = G.ThreadSuccessor(v);

            if (p==NoNode)
            {
                 WriteNodeLabel(v,x+long(nodeSize),y+long(nodeSize));
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


void exportToTk::WriteSmallNode(TNode v,long x,long y) throw()
{
    int radius = 5;

    if (v==NoNode) expFile << "  {-1";
    else expFile << "  {" << v;

    expFile << " " << ID_GRAPH_NODE << " oval {"
        << x-radius << " " << y-radius << " " << x+radius << " " << y+radius
        << "} {-outline #000000 -fill #000000 -width 1} } \\" << endl;
}


void exportToTk::WriteCircularNode(TNode v,long x,long y,char* fillColour) throw()
{
    double radius = nodeSize;

    if (v!=NoNode && G.ThreadSuccessor(v)!=NoNode)
    {
        TNode w = G.ThreadSuccessor(v);
        radius += sqrt(G.C(w,0)*G.C(w,0)+G.C(w,1)*G.C(w,1))*(CFG.xZoom+CFG.yZoom)/2;
    }

    if (v==NoNode) expFile << "  {-1";
    else expFile << "  {" << v;

    expFile << " " << ID_GRAPH_NODE << " oval {"
        << RoundToLong(x-radius) << " " << RoundToLong(y-radius) << " "
        << RoundToLong(x+radius) << " " << RoundToLong(y+radius)
        << "} {-outline #000000 -fill " << fillColour << "} } \\" << endl;
}


void exportToTk::WriteRectangularNode(TNode v,long x,long y,char* fillColour) throw()
{
    double width  = nodeSize;
    double height = nodeSize;

    if (v!=NoNode && G.ThreadSuccessor(v)!=NoNode)
    {
        TNode w = G.ThreadSuccessor(v);

        width  += G.C(w,0)*CFG.xZoom;
        height += G.C(w,1)*CFG.yZoom;
    }

    if (v==NoNode) expFile << "  {-1";
    else expFile << "  {" << v;

    expFile << " " << ID_GRAPH_NODE << " rectangle {"
        << RoundToLong(x-width) << " " << RoundToLong(y-height) << " "
        << RoundToLong(x+width) << " " << RoundToLong(y+height)
        << "} {-outline #000000 -fill " << fillColour << "} } \\" << endl;
}


void exportToTk::WriteNodeLabel(TNode v,long x,long y) throw()
{
    if (strcmp(CFG.nodeFormatting,"")==0 && CFG.nodeLabels==0) return;

    expFile << "  {" << v << " " << ID_NODE_LABEL << " text {"
        << x << " " << y << "} {-text {";

    ComposeNodeLabel(v,expFile);

    expFile << "}" << " -anchor c -font " << largeFont << "} } \\" << endl;
}


void exportToTk::DisplayArtificialNode(TNode v) throw()
{
    long x = CanvasCX(v);
    long y = CanvasCY(v);
    int radius = 4;

    expFile << "  {" << v << " " << ID_BEND_NODE << " rectangle {"
        << x-radius << " " << y-radius << " " << x+radius << " " << y+radius
        << "} {-outline #000000 -fill #000000} } \\" << endl;
}


exportToTk::~exportToTk() throw()
{
    for (TNode i=G.N();i<G.N()+G.NI();i++) DisplayArtificialNode(i);

    expFile << "}" << endl;
    expFile.close();
}
