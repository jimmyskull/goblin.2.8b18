
//  This file forms part of the GOBLIN C++ Class Library.
//
//  Initially written by Christian Fremuth-Paeger, April 2007
//
//  Copying, compiling, distribution and modification
//  of this source code is permitted only in accordance
//  with the GOBLIN general licence information.

/// \file   libImport.cpp
/// \brief  A coolection of file import filters for graph objects

#include <stdio.h>
#include "sparseDigraph.h"
#include "sparseGraph.h"

goblinRootObject* goblinController::ImportFromFile(const char* filename, TFileFormat format)
    throw(ERParse)
{
    switch (format)
    {
        case FMT_DIMACS_MCF:
        {
            return Import_DimacsMCF(filename);
        }
        case FMT_DIMACS_EDGE:
        {
            return Import_DimacsEdge(filename);
        }
        default:
        {
        }
    }

    return NULL;
}

goblinRootObject* goblinController::Import_DimacsMCF(const char* filename)
    throw(ERParse)
{
    diGraph* G = NULL;
    graphRepresentation* GR = NULL;
    const unsigned LINE_BUFFER_LENGTH = 128;
    char lineBuffer[LINE_BUFFER_LENGTH];
    int nFound = 0;
    unsigned long n = 0;
    unsigned long m = 0;
    unsigned long u = 0;
    unsigned long v = 0;
    double demand = 0.0;
    double lcap = 0.0;
    double ucap = 0.0;
    double length = 0.0;

    FILE* inputFileDescriptor = fopen(filename,"r");

    while (fgets(lineBuffer,LINE_BUFFER_LENGTH,inputFileDescriptor))
    {
        if (n==0)
        {
            nFound = sscanf(lineBuffer,"p min %lu %lu",&n,&m);

            if (nFound==2)
            {
                if (n==0 || m==0)
                {
                    fclose(inputFileDescriptor);
                    Error(ERR_PARSE,NoHandle,"Import_DimacsMCF",
                        "Insufficient problem dimensions");
                }

                G = new diGraph(TNode(n),*this);
                GR = G->Representation();
                GR -> SetCDemand(0);
            }
        }
        else
        {
            nFound = sscanf(lineBuffer,"n %lu %lf",&v,&demand);

            if (nFound==2)
            {
                if (v>n || v<1)
                {
                    fclose(inputFileDescriptor);
                    delete G;
                    Error(ERR_PARSE,NoHandle,"Import_DimacsMCF",
                        "Node index exeeds problem dimension");
                }

                GR -> SetDemand(TNode(v-1),TCap(-demand));
            }
            else
            {
                nFound = sscanf(lineBuffer,"a %lu %lu %lf %lf %lf",&u,&v,&lcap,&ucap,&length);

                if (nFound==5)
                {
                    if (u>n || u<1 || v>n || v<1)
                    {
                        fclose(inputFileDescriptor);
                        delete G;
                        Error(ERR_PARSE,NoHandle,"Import_DimacsMCF",
                            "Node index exeeds problem dimension");
                    }

                    G -> InsertArc(TNode(u-1),TNode(v-1),TCap(ucap),TFloat(length),TCap(lcap));
                }
            }
        }
    }

    fclose(inputFileDescriptor);

    if (n==0)
    {
        Error(ERR_PARSE,NoHandle,"Import_DimacsMCF","Missing problem line");
    }

    return G;
}

goblinRootObject* goblinController::Import_DimacsEdge(const char* filename)
    throw(ERParse)
{
    graph* G = NULL;
    graphRepresentation* GR = NULL;
    const unsigned LINE_BUFFER_LENGTH = 128;
    char lineBuffer[LINE_BUFFER_LENGTH];
    int nFound = 0;
    unsigned long n = 0;
    unsigned long m = 0;
    unsigned long u = 0;
    unsigned long v = 0;
    double length = 0.0;

    FILE* inputFileDescriptor = fopen(filename,"r");

    while (fgets(lineBuffer,LINE_BUFFER_LENGTH,inputFileDescriptor))
    {
        if (n==0)
        {
            nFound = sscanf(lineBuffer,"p edge %lu %lu",&n,&m);

            if (nFound==2)
            {
                if (n==0 || m==0)
                {
                    fclose(inputFileDescriptor);
                    Error(ERR_PARSE,NoHandle,"Import_DimacsEdge",
                        "Insufficient problem dimensions");
                }

                G = new graph(TNode(n),*this);
                GR = G->Representation();
                GR -> SetCDemand(1);
                GR -> SetCUCap(1);
                GR -> SetCLCap(0);
                GR -> SetCLength(1);
                randGeometry = 0;
                randLength = 0;
            }
        }
        else
        {
            nFound = sscanf(lineBuffer,"e %lu %lu %lf",&u,&v,&length);

            if (nFound>=2)
            {
                if (u>n || u<1 || v>n || v<1)
                {
                    fclose(inputFileDescriptor);
                    delete G;
                    Error(ERR_PARSE,NoHandle,"Import_DimacsEdge",
                        "Node index exeeds problem dimension");
                }

                G -> InsertArc(TNode(u-1),TNode(v-1),TCap(1),TFloat(length));
            }
        }
    }

    fclose(inputFileDescriptor);

    if (n==0)
    {
        Error(ERR_PARSE,NoHandle,"Import_DimacsEdge","Missing problem line");
    }

    return G;
}
