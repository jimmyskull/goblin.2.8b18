
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, January 2002
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file   treeView.cpp
/// \brief  #goblinTreeView class implementation

#include "treeView.h"

goblinTreeView::goblinTreeView(TNode n,goblinController &thisContext) throw() :
    managedObject(thisContext),
    diGraph(n,thisContext)
{
    LogEntry(LOG_MEM,"Generating tree view...");
    InitNodeColours();
}


goblinTreeView::~goblinTreeView() throw()
{
    LogEntry(LOG_MEM,"...tree view disallocated");
}


void goblinTreeView::ConfigDisplay(goblinController &CFG) const throw()
{
    CFG.predecessors = 0;
    CFG.nodeLabels = 2;
    CFG.arcLabels = 0;
    CFG.arcLabels2 = 0;
    CFG.nodeColours = 2;
    CFG.subgraph = 2;
    CFG.legenda = 0;
}


bool goblinTreeView::HiddenNode(TNode v) const throw(ERRange)
{
    return NodeColour(v)==NoNode;
}
