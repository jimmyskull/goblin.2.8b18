
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, November 2002
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file libLayout.cpp
/// \brief Implementations of layout helper methods

#include "sparseRepresentation.h"
#include "staticStack.h"


TNode abstractMixedGraph::PortNode(TArc a) const throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (a>=2*m) NoSuchArc("PortNode",a);

    #endif

    TNode w = ArcLabelAnchor(a);

    if (w==NoNode) return NoNode;

    TNode x = ThreadSuccessor(w);

    if (x==NoNode) return NoNode;

    if (a%2==0) return x;

    do
    {
        w = x;
        x = ThreadSuccessor(w);
    }
    while (x!=NoNode);

    return w;
}


TNode abstractMixedGraph::GetBendNodes(TArc a,TNode* layoutNode,TNode length) const throw(ERRange)
{
    #if defined(_FAILSAVE_)

    if (a>=2*m) NoSuchArc("GetBendNodes",a);

    #endif

    layoutNode[0] = StartNode(a);
    TNode w = ArcLabelAnchor(a);
    TNode nLayoutNodes = 2;

    if (w==NoNode)
    {
        layoutNode[1] = EndNode(a);
        return nLayoutNodes;
    }

    TNode x = ThreadSuccessor(w);

    if (x==NoNode)
    {
        layoutNode[1] = EndNode(a);
        return nLayoutNodes;
    }

    // First pass: Count bend nodes
    do
    {
        x = ThreadSuccessor(x);
        ++nLayoutNodes;
    }
    while (x!=NoNode);

    x = ThreadSuccessor(w);

    // Second pass: Write list of nodes
    for (TNode i=1;i<nLayoutNodes-1 && i<length;++i)
    {
        if (a%2==0)
        {
            layoutNode[i] = x;
        }
        else if (nLayoutNodes-i-1<length)
        {
            layoutNode[nLayoutNodes-i-1] = x;
        }

        x = ThreadSuccessor(x);
    }

    if (nLayoutNodes<=length) layoutNode[nLayoutNodes-1] = EndNode(a);

    return nLayoutNodes;
}


TNode sparseRepresentation::InsertBendNode(TArc a,TNode x) throw(ERRange,ERRejected)
{
    #if defined(_FAILSAVE_)

    if (a>=2*mAct) NoSuchArc("InsertBendNode",a);

    if (x==EndNode(a))
        Error(ERR_REJECTED,"InsertBendNode","Cannot add a bend node after the end node");

    #endif

    TNode w = ArcLabelAnchor(a);

    if (w==NoNode) w = InsertArcLabelAnchor(a);

    if (!(a&1) && x==StartNode(a)) return InsertThreadSuccessor(w);

    TNode z = ThreadSuccessor(w);
    TNode y = w;

    while (z!=x && z!=NoNode)
    {
        y = z;
        z = ThreadSuccessor(z);
    }

    #if defined(_FAILSAVE_)

    if (x!=z && x!=StartNode(a))
        Error(ERR_REJECTED,"InsertBendNode","Invalid predecessor point");

    #endif

    if (a&1) return InsertThreadSuccessor(y);

    return InsertThreadSuccessor(z);
}


void abstractMixedGraph::Layout_StripEmbedding() throw(ERRejected)
{
    graphRepresentation* X = Representation();

    #if defined(_FAILSAVE_)

    if (!X) NoRepresentation("Layout_StripEmbedding");

    if (MetricType()!=METRIC_DISABLED && IsDense())
        Error(ERR_REJECTED,"Layout_StripEmbedding","Coordinates are fixed");

    #endif

    if (n+ni == 0) return;

    TFloat minX = C(0,0);
    TFloat minY = C(0,1);

    for (TNode v=0;v<n+ni;v++)
    {
        if (C(v,0)<minX) minX = C(v,0);
        if (C(v,1)<minY) minY = C(v,1);
    }

    if (MetricType()==METRIC_SPHERIC)
    {
        minX = floor(minX);
        minY = floor(minY);
    }

    for (TNode v=0;v<n+ni;v++)
    {
        X -> SetC(v,0,C(v,0)-minX);
        X -> SetC(v,1,C(v,1)-minY);
    }
}


TLayoutModel abstractMixedGraph::LayoutModel() const throw()
{
    if (!LayoutData()) return TLayoutModel(CT.layoutModel);

    return TLayoutModel(LayoutData()->GetValue<int>(TokLayoutModel,0,goblinDefaultContext.layoutModel));
}


void abstractMixedGraph::SetLayoutModel(TLayoutModel model) throw()
{
    if (!LayoutData()) return;

    if (model==goblinDefaultContext.layoutModel)
    {
        LayoutData() -> ReleaseAttribute(TokLayoutModel);
    }
    else
    {
        LayoutData() -> InitAttribute<int>(*this,TokLayoutModel,int(model));
    }
}


void abstractMixedGraph::Layout_ConvertModel(TLayoutModel model) throw()
{
    TLayoutModel currentModel = LayoutModel();

    if (model==LAYOUT_DEFAULT)
    {
        model = TLayoutModel(CT.layoutModel);
    }

    if (model==currentModel)
    {
        // Do nothing but reverting to the default parameters
        CT.SetDisplayParameters(model);
        return;
    }

    // In the new layout model, start with a fixed aspect ratio
    CT.yZoom = CT.xZoom = (CT.xZoom+CT.yZoom)/2;

    if (model==LAYOUT_STRAIGHT_2DIM)
    {
        // Clean up the drawing

        if (IsSparse() && Representation())
        {
            sparseRepresentation* X = static_cast<sparseRepresentation*>(Representation());

            for (TNode v=0;v<n;++v) X -> ReleaseShapeNodes(v);

            for (TArc a=0;a<m;++a) X -> ReleaseBendNodes(2*a);

            for (TDim i=2;i<Dim();++i) X -> ReleaseCoordinate(i);
        }
    }
    else if (   model==LAYOUT_ORTHO_BIG
             && (   currentModel==LAYOUT_ORTHO_SMALL
                 || currentModel==LAYOUT_VISIBILITY
                 || currentModel==LAYOUT_KANDINSKI
                )
            )
    {
        // Transforming into a more general layout model does not
        // require any manipulations of the drawing. If the previous
        // drawing was in the Kandinsky model, some grid lines may
        // be saved, but this is not implemented yet
    }
    else if (currentModel!=LAYOUT_STRAIGHT_2DIM)
    {
        // As a default rule, convert from one model to another
        // by using the straight line model as an intermediate step
        Layout_ConvertModel(LAYOUT_STRAIGHT_2DIM);
    }

    SetLayoutModel(model);
    CT.SetDisplayParameters(model);

    switch (model)
    {
        case LAYOUT_FREESTYLE_CURVES:
        case LAYOUT_FREESTYLE_POLYGONES:
        case LAYOUT_LAYERED:
        {
            if (IsSparse() && Representation())
            {
                static_cast<sparseRepresentation*>(Representation()) -> Layout_ArcAlignment();
            }

            break;
        }
        case LAYOUT_STRAIGHT_2DIM:
        case LAYOUT_ORTHO_BIG:
        case LAYOUT_ORTHO_SMALL:
        case LAYOUT_VISIBILITY:
        case LAYOUT_KANDINSKI:
        case LAYOUT_DEFAULT:
        case LAYOUT_NONE:
        {
            break;
        }
    }
}


void goblinController::SetDisplayParameters(TLayoutModel model) throw()
{
    switch (model)
    {
        case LAYOUT_STRAIGHT_2DIM:
        {
            nodeScalable = 0;
            arcStyle = 1;

            break;
        }
        case LAYOUT_FREESTYLE_CURVES:
        {
            nodeScalable = 0;
            arcStyle = 1;
            arrows &= ~1;

            break;
        }
        case LAYOUT_FREESTYLE_POLYGONES:
        {
            nodeScalable = 0;
            arcStyle = 0;

            break;
        }
        case LAYOUT_LAYERED:
        {
            nodeScalable = 0;
            arcStyle = 0;
            break;
        }
        case LAYOUT_ORTHO_BIG:
        case LAYOUT_ORTHO_SMALL:
        case LAYOUT_VISIBILITY:
        case LAYOUT_KANDINSKI:
        {
            nodeScalable = 1;
            arcStyle = 2;

            break;
        }
        case LAYOUT_DEFAULT:
        case LAYOUT_NONE:
        {
            break;
        }
    }

    if (nodeScalable)
    {
        nodeStyle = 2;
        nodeSep = bendSep*4;
        fineSep = bendSep;
        arrowSize = 100;
        nodeSize = 500;
    }
    else
    {
        if (nodeStyle>1) nodeStyle = 1;

        nodeSep = bendSep;
        fineSep = 0;
        nodeSize = 100;
        arrowSize = 100;
    }

    layoutModel = int(model);
}


void abstractMixedGraph::WriteLayout(goblinExport* F) const throw()
{
    if (!LayoutData()) return;

    LayoutData() -> WritePool(*this,*F,"layout");
}


void abstractMixedGraph::ReadLayoutData(goblinImport& F) throw(ERParse)
{
    LayoutData() -> ReadPool(*this,F);

    int* model = LayoutData()->GetArray<int>(TokLayoutModel);

    if (!model)
    {
        CT.layoutModel = goblinDefaultContext.layoutModel;
    }
    else if (model[0]<=int(LAYOUT_NONE) && model[0]>=0)
    {
        CT.layoutModel = model[0];
        CT.SetDisplayParameters(TLayoutModel(model[0]));
    }
    else
    {
        sprintf(CT.logBuffer,"Unknown layout model: %d",model[0]);
        Error(ERR_PARSE,"ReadLayout",CT.logBuffer);
    }


    TArc* pExteriorArc = LayoutData()->GetArray<TArc>(TokLayoutExteriorArc);

    if (pExteriorArc) MarkExteriorFace(*pExteriorArc);
}


void abstractMixedGraph::Layout_ScaleEmbedding(TFloat xMin,TFloat xMax,TFloat yMin,TFloat yMax)
    throw(ERRejected)
{
    graphRepresentation* X = Representation();

    #if defined(_FAILSAVE_)

    if (!X) NoRepresentation("Layout_ScaleEmbedding");

    if (MetricType()!=METRIC_DISABLED && IsDense())
        Error(ERR_REJECTED,"Layout_ScaleEmbedding","Coordinates are fixed");

    #endif

    TFloat xMinOld = CMin(0);
    TFloat xMaxOld = CMax(0);
    TFloat yMinOld = CMin(1);
    TFloat yMaxOld = CMax(1);

    for (TNode v=0;v<n+ni;v++)
    {
        if (fabs(xMaxOld-xMinOld) > CT.epsilon)
        {
            X -> SetC(v,0,xMin+(C(v,0)-xMinOld)*(xMax-xMin)/(xMaxOld-xMinOld));
        }
        else
        {
            X -> SetC(v,0,(xMax-xMin)/2);
        }

        if (fabs(yMaxOld-yMinOld) > CT.epsilon)
        {
            X -> SetC(v,1,yMin+(C(v,1)-yMinOld)*(yMax-yMin)/(yMaxOld-yMinOld));
        }
        else
        {
            X -> SetC(v,1,(yMax-yMin)/2);
        }
    }
}


void sparseRepresentation::Layout_ArcAlignment(TFloat spacing,bool drawLoops) throw()
{
    if (Dim()<2) return;

    LogEntry(LOG_METH,"Release bend nodes...");

    THandle H = G.Investigate();
    investigator &I = G.Investigator(H);
    staticStack<TArc,TFloat> S(2*mAct,CT);

    if (spacing<=0) spacing = CT.bendSep;

    for (TArc a=0;a<mAct;a++) ReleaseBendNodes(2*a);

    for (TNode u=0;u<nAct;u++)
    {
        for (TNode v=u;v<nAct;v++)
        {
            S.Init();

            while (I.Active(u))
            {
                TArc a = I.Read(u);
                if (EndNode(a)==v && (u!=v || (a%2))) S.Insert(a);
            }

            I.Reset(u);

            if (v!=u)
            {
                if (S.Cardinality()>1)
                {
                    TFloat dx = C(v,0)-C(u,0);
                    TFloat dy = C(v,1)-C(u,1);
                    TFloat norm = sqrt(dx*dx+dy*dy);
                    TFloat ox = dy/norm;
                    TFloat oy = -dx/norm;
                    TFloat cx = (C(v,0)+C(u,0))/2-spacing*(S.Cardinality()-1)/2*ox;
                    TFloat cy = (C(v,1)+C(u,1))/2-spacing*(S.Cardinality()-1)/2*oy;

                    while (!S.Empty())
                    {
                        TArc a = S.Delete();
                        TNode w = InsertArcLabelAnchor(a);
                        SetC(w,0,cx);
                        SetC(w,1,cy);
                        w = InsertThreadSuccessor(w);
                        SetC(w,0,cx);
                        SetC(w,1,cy);
                        cx += spacing*ox;
                        cy += spacing*oy;
                    }
                }
            }
            else if (drawLoops)
            {
                TFloat cx = C(v,0);
                TFloat cy = C(v,1)+spacing;

                while (!S.Empty())
                {
                    TArc a = S.Delete();
                    TNode w = InsertArcLabelAnchor(a);
                    SetC(w,0,cx);
                    SetC(w,1,cy);
                    w = InsertThreadSuccessor(w);
                    SetC(w,0,cx+spacing/4);
                    SetC(w,1,cy);
                    w = InsertThreadSuccessor(w);
                    SetC(w,0,cx-spacing/4);
                    SetC(w,1,cy);
                    cy += spacing;
                }
            }
        }
    }

    G.Close(H);
}


void sparseRepresentation::Layout_SubdivideArcs(TFloat spacing) throw()
{
    if (Dim()<2) return;

    LogEntry(LOG_METH,"Subdivide arcs...");

    if (spacing<=0) spacing = CT.nodeSep;

    G.Layout_ConvertModel(LAYOUT_LAYERED);

    for (TArc a=0;a<mAct;a++)
    {
        TNode u = StartNode(2*a);
        TNode v = EndNode(2*a);

        if (v==u) continue;

        TFloat cu = C(u,1);
        TFloat cv = C(v,1);

        cu = ceil(cu/spacing-0.5)*spacing;
        cv = ceil(cv/spacing-0.5)*spacing;

        if (fabs(cv-cu)<spacing*1.5)
        {
            // Short arcs (which do not cross any layer) do not need bend nodes
            ReleaseBendNodes(2*a);
            continue;
        }

        // Check if the current routing of arc a is valid
        TFloat sign = 1-2*(cv<cu);
        TNode w = ArcLabelAnchor(2*a);
        TFloat cw = cu;
        bool needsRerouting = (w==NoNode);

        while (!needsRerouting && fabs(cv-cw)>=spacing*1.5)
        {
            w = ThreadSuccessor(w);
            cw += sign*spacing;

            if (w==NoNode || fabs(C(w,1)-cw)>0.5*spacing) needsRerouting = true;
        }

        if (!needsRerouting && ThreadSuccessor(w)!=NoNode) needsRerouting = true;

        if (!needsRerouting) continue;

        ReleaseBendNodes(2*a);

        TFloat cm = (C(v,0)+C(u,0))/2;
        w = InsertArcLabelAnchor(2*a);
        cw = cu+sign*spacing;
        SetC(w,0,cm+CT.fineSep);
        SetC(w,1,(C(v,1)+C(u,1))/2);
        w = InsertThreadSuccessor(w);
        SetC(w,0,C(v,0)*(cw-cu)/(cv-cu)+C(u,0)*(cv-cw)/(cv-cu));
        SetC(w,1,cw);

        while (fabs(cv-cw)>=spacing*1.5)
        {
            w = InsertThreadSuccessor(w);
            cw += sign*spacing;
            SetC(w,0,C(v,0)*(cw-cu)/(cv-cu)+C(u,0)*(cv-cw)/(cv-cu));
            SetC(w,1,cw);
        }
    }
}


void sparseRepresentation::Layout_AdoptArcRouting(abstractMixedGraph& G)
    throw()
{
    for (TArc a=0;a<mAct;a++)
    {
        TNode v = G.ArcLabelAnchor(2*a);

        if (v==NoNode) continue;

        TNode w = InsertArcLabelAnchor(2*a);

        SetC(w,0,G.C(v,0));
        SetC(w,1,G.C(v,1));

        v = G.ThreadSuccessor(v);

        while (v!=NoNode)
        {
            w = InsertThreadSuccessor(w);

            SetC(w,0,G.C(v,0));
            SetC(w,1,G.C(v,1));

            v = G.ThreadSuccessor(v);
        }
    }
}


void abstractMixedGraph::Layout_Circular(int spacing) throw(ERRejected)
{
    #if defined(_FAILSAVE_)

    if (MetricType()!=METRIC_DISABLED && IsDense())
        Error(ERR_REJECTED,"Layout_Circular","Coordinates are fixed");

    #endif


    // If an outerplanar representation is available, compute a respective layout

    if (!IsDense() && m<=2*n-3 && Layout_Outerplanar(spacing)) return;

    if (GetPredecessors())
    {
        Layout_CircularByPredecessors(spacing);
    }
    else
    {
        Layout_CircularByColours(spacing);
    }
}


void abstractMixedGraph::Layout_CircularByPredecessors(int spacing)
    throw(ERRejected)
{
    #if defined(_FAILSAVE_)

    if (MetricType()!=METRIC_DISABLED && IsDense())
        Error(ERR_REJECTED,"Layout_CircularByPredecessors",
            "Coordinates are fixed");

    #endif

    TArc* pred = GetPredecessors();

    if (pred)
    {
        // If a Hamiltonian cycle is available, display this

        TNode* index = new TNode[n];
        bool* pending = new bool[n];
        for (TNode v=0;v<n;v++) pending[v] = true;
        TNode thisPos = 0;

        for (TNode u=0;u<n;u++)
        {
            TNode v = u;

            while (pred[v]!=NoArc && pending[v])
            {
                index[thisPos++] = v;
                pending[v] = false;
                v = StartNode(pred[v]);
            }

            if (pending[v])
            {
                index[thisPos++] = v;
                pending[v] = false;
            }
        }

        delete[] pending;

        Layout_AssignCircularCoordinates(spacing,index);

        delete[] index;
    }
    else
    {
        // Display nodes in the order of indices

        Layout_AssignCircularCoordinates(spacing);
    }

    Layout_ConvertModel(LAYOUT_FREESTYLE_CURVES);
}


void abstractMixedGraph::Layout_CircularByColours(int spacing)
    throw(ERRejected)
{
    #if defined(_FAILSAVE_)

    if (MetricType()!=METRIC_DISABLED && IsDense())
        Error(ERR_REJECTED,"Layout_CircularByColours",
            "Coordinates are fixed");

    #endif

    TNode* nodeColour = GetNodeColours();

    if (nodeColour)
    {
        // Display the order or clusters available by the
        // node colour labels. If no colours are present,
        // display the nodes in arbitrary order.

        goblinQueue<TNode,TFloat> *Q = NULL;

        if (nHeap!=NULL)
        {
            Q = nHeap;
            Q -> Init();
        }
        else Q = NewNodeHeap();

        for (TNode u=0;u<n;u++) Q->Insert(u,n*nodeColour[u]+u);

        TNode* index = new TNode[n];

        for (TNode u=0;u<n;u++) index[u] = Q->Delete();

        if (nHeap==NULL) delete Q;

        Layout_AssignCircularCoordinates(spacing,index);

        delete[] index;
    }
    else
    {
        // Display nodes in the order of indices

        Layout_AssignCircularCoordinates(spacing);
    }

    Layout_ConvertModel(LAYOUT_FREESTYLE_CURVES);
}


bool abstractMixedGraph::Layout_Outerplanar(int spacing) throw(ERRejected)
{
    #if defined(_FAILSAVE_)

    if (IsDense())
        Error(ERR_REJECTED,"Layout_Outerplanar","Not an outerplanar graph");

    #endif

    TNode* thread = new TNode[n];
    for (TNode v=0;v<n;v++) thread[v] = NoNode;

    TNode tail = NoNode;
    TNode l = 0;

    for (TNode u=0;u<n;u++)
    {
        if (thread[u]!=NoNode || u==tail) continue;

        if (tail!=NoNode)
        {
            thread[tail] = u;
            l++;
        }

        tail = u;

        if (First(u)==NoArc) continue;

        TArc a = Right(First(u),u);
        TArc k = 0;

        while ((a^1)!=First(u))
        {
            TNode w = EndNode(a);

            if (thread[w]==NoNode && tail!=w)
            {
                thread[tail] = w;
                tail = w;
                l++;
            }

            a = Right(a^1,w);
            k++;

            if (k>2*m)
            {
                // Not a planar representation, leave the outer loop
                u = n;
                break;
            }
        }
    }

    if (l==n-1)
    {
        if (spacing>0)
        {
            CT.nodeSep = spacing;
            CT.bendSep = spacing;
            CT.fineSep = 0;
        }

        TFloat rad = CT.nodeSep*n/2/PI;

        TNode v = 0;
        TNode k = 0;

        while (v!=NoNode)
        {
            SetC(v,0,rad*(1+cos(k*2*PI/n)));
            SetC(v,1,rad*(1+sin(k*2*PI/n)));
            v = thread[v];
            k++;
        }

        if (CT.methLocal==LOCAL_OPTIMIZE) Layout_ForceDirected(FDP_RESTRICTED);

        Layout_ConvertModel(LAYOUT_FREESTYLE_CURVES);
    }

    delete[] thread;

    return (l==n-1);
}


void abstractMixedGraph::Layout_AssignCircularCoordinates(int spacing,
    TNode* index) throw(ERRejected)
{
    if (spacing>0)
    {
        CT.nodeSep = spacing;
        CT.bendSep = spacing;
        CT.fineSep = 0;
    }

    TFloat rad = CT.nodeSep*n/2/PI;

    for (TNode u=0;u<n;u++)
    {
        TNode v = u;

        if (index) v = index[u];

        SetC(v,0,rad*(1+cos(u*2*PI/n)));
        SetC(v,1,rad*(1+sin(u*2*PI/n)));
    }
}


void abstractMixedGraph::Layout_Equilateral(int spacing) throw(ERRejected)
{
    graphRepresentation* X = Representation();

    #if defined(_FAILSAVE_)

    if (!X) NoRepresentation("Layout_Equilateral");

    if (ExtractEmbedding(PLANEXT_DEFAULT)==NoNode)
    {
        Error(ERR_REJECTED,"Layout_Equilateral","Graph is not embedded");
    }

    #endif

    TArc exteriorArc = ExteriorArc();
    TNode fExterior = face[exteriorArc];

    #if defined(_FAILSAVE_)

    if (face[exteriorArc^1]==fExterior)
        Error(ERR_REJECTED,"Layout_Equilateral","Graph must be 2-connected");

    #endif

    if (spacing<=0) spacing = CT.nodeSep;

    bool* placed = new bool[n];
    for (TNode v=0;v<n;v++) placed[v] = false;

    TArc aBasis = exteriorArc^1;
    X -> SetC(StartNode(aBasis),0,0);
    X -> SetC(StartNode(aBasis),1,0);
    X -> SetC(EndNode(aBasis),0,spacing);
    X -> SetC(EndNode(aBasis),1,0);

    staticStack<TArc,TFloat> Q(2*m,CT);
    Q.Insert(aBasis);

    while (!Q.Empty())
    {
        aBasis = Q.Delete();

        TNode u = StartNode(aBasis);
        TNode v = EndNode(aBasis);

        if (u==v)
        {
            delete[] placed;
            Error(ERR_REJECTED,"Layout_Equilateral","Graph contains loops");
        }

        // Scan the face of aBase the first time: Determine the degree
        // of this face and memorize all adjacent faces
        TArc a = Right(aBasis^1,EndNode(aBasis));
        TNode degree = 1;

        while (a!=aBasis)
        {
            if (face[a^1]!=fExterior) Q.Insert(a^1);

            a = Right(a^1,EndNode(a));
            degree++;
        }

        TFloat radMax = spacing*0.5/sin(PI/degree);
        TFloat radMin = radMax*cos(PI/degree);
        TFloat ox = C(v,1)-C(u,1);
        TFloat oy = C(u,0)-C(v,0);
        TFloat norm = sqrt(ox*ox+oy*oy);
        TFloat xCenter = (C(u,0)+C(v,0))/2+ox/norm*radMin;
        TFloat yCenter = (C(u,1)+C(v,1))/2+oy/norm*radMin;
        TFloat alpha0 = atan2(C(u,0)-xCenter,C(u,1)-yCenter);

        // Scan this face the second time and assign coordinates
        a = Right(aBasis^1,v);
        TNode w = EndNode(a);
        TNode k = 2;

        while (w!=u)
        {
            X -> SetC(w,0,xCenter+radMax*sin(alpha0 + k*2*PI/degree));
            X -> SetC(w,1,yCenter+radMax*cos(alpha0 + k*2*PI/degree));

            a = Right(a^1,w);
            w = EndNode(a);
            k++;
        }
    }

    Layout_ConvertModel(LAYOUT_FREESTYLE_CURVES);

    delete[] placed;
}
