//-----------------------------------------------------------------------------
//   toposort.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     03/20/14   (Build 5.1.001)
//   Author:   L. Rossman
//
//   Topological sorting of conveyance network links
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <stdlib.h>
#include "headers.h"

//-----------------------------------------------------------------------------
//  Constants
//-----------------------------------------------------------------------------
enum AdjListType {UNDIRECTED, DIRECTED};    // type of nodal adjacency list

//-----------------------------------------------------------------------------
//  Shared variables
//-----------------------------------------------------------------------------
static int* InDegree;                  // number of incoming links to each node
static int* StartPos;                  // start of a node's outlinks in AdjList
static int* AdjList;                   // list of outlink indexes for each node
static int* Stack;                     // array of nodes "reached" during sorting
static int  First;                     // position of first node in stack
static int  Last;                      // position of last node added to stack

static char* Examined;                 // TRUE if node included in spanning tree
static char* InTree;                   // state of each link in spanning tree:
                                       // 0 = unexamined,
                                       // 1 = in spanning tree,
                                       // 2 = chord of spanning tree
static int*  LoopLinks;                // list of links which forms a loop
static int   LoopLinksLast;            // number of links in a loop

//-----------------------------------------------------------------------------
//  External functions (declared in funcs.h)   
//-----------------------------------------------------------------------------
//  toposort_sortLinks (called by routing_open)

//-----------------------------------------------------------------------------
//  Local functions
//-----------------------------------------------------------------------------
static void createAdjList(int listType);
static void adjustAdjList(void);
static int  topoSort(int sortedLinks[]);
static void findCycles(void);
static void findSpanningTree(int startNode);
static void evalLoop(int startLink);
static int  traceLoop(int i1, int i2, int k);
static void checkDummyLinks(void);
//=============================================================================

void toposort_sortLinks(int sortedLinks[])
//
//  Input:   none
//  Output:  sortedLinks = array of link indexes in sorted order
//  Purpose: sorts links from upstream to downstream.
//
{
    int i, n = 0;

    // --- no need to sort links for Dyn. Wave routing
    for ( i=0; i<Nobjects[LINK]; i++) sortedLinks[i] = i;
    if ( RouteModel == DW )
    {

        // --- check for nodes with both incoming and outgoing
        //     dummy links (creates ambiguous ordering)
        checkDummyLinks();
        if ( ErrorCode ) return;

        // --- find number of outflow links for each node
        for ( i=0; i<Nobjects[NODE]; i++ ) Node[i].degree = 0;
        for ( i=0; i<Nobjects[LINK]; i++ )
        {
            // --- if upstream node is an outfall, then increment outflow
            //     count for downstream node, otherwise increment count
            //     for upstream node
            n = Link[i].node1;
            if ( Link[i].direction < 0 ) n = Link[i].node2;
            if ( Node[n].type == OUTFALL )
            {
                if ( Link[i].direction < 0 ) n = Link[i].node1;
                else n = Link[i].node2;
                Node[n].degree++;
            }
            else Node[n].degree++;
        }
        return;
    }

    // --- allocate arrays used for topo sorting
    if ( ErrorCode ) return;
    InDegree = (int *) calloc(Nobjects[NODE], sizeof(int));
    StartPos = (int *) calloc(Nobjects[NODE], sizeof(int));
    AdjList  = (int *) calloc(Nobjects[LINK], sizeof(int));
    Stack    = (int *) calloc(Nobjects[NODE], sizeof(int));
    if ( InDegree == NULL || StartPos == NULL ||
         AdjList == NULL || Stack == NULL )
    {
        report_writeErrorMsg(ERR_MEMORY, "");
    }
    else
    {
        // --- create a directed adjacency list of links leaving each node
        createAdjList(DIRECTED);

        // --- adjust adjacency list for DIVIDER nodes
        adjustAdjList();

        // --- find number of links entering each node
        for (i = 0; i < Nobjects[NODE]; i++) InDegree[i] = 0;
        for (i = 0; i < Nobjects[LINK]; i++) InDegree[ Link[i].node2 ]++;

        // --- topo sort the links
        n = topoSort(sortedLinks);
    }   

    // --- free allocated memory
    FREE(InDegree);
    FREE(StartPos);
    FREE(AdjList);
    FREE(Stack);

    // --- check that all links are included in SortedLinks
    if ( !ErrorCode &&  n != Nobjects[LINK] )
    {
        report_writeErrorMsg(ERR_LOOP, "");
        findCycles();
    }
}

//=============================================================================

void createAdjList(int listType)
//
//  Input:   lsitType = DIRECTED or UNDIRECTED
//  Output:  none
//  Purpose: creates listing of links incident on each node.
//
{
    int i, j, k;

    // --- determine degree of each node
    //     (for DIRECTED list only count link at its upstream node;
    //      for UNDIRECTED list count link at both end nodes)
    for (i = 0; i < Nobjects[NODE]; i++) Node[i].degree = 0;
    for (j = 0; j < Nobjects[LINK]; j++)
    {
        Node[ Link[j].node1 ].degree++;
        if ( listType == UNDIRECTED ) Node[ Link[j].node2 ].degree++;
    }

    // --- determine start position of each node in the adjacency list
    //     (the adjacency list, AdjList, is one long vector containing
    //      the individual node lists one after the other)
    StartPos[0] = 0;
    for (i = 0; i < Nobjects[NODE]-1; i++)
    {
        StartPos[i+1] = StartPos[i] + Node[i].degree;
        Node[i].degree = 0;
    }
    Node[Nobjects[NODE]-1].degree = 0;

    // --- traverse the list of links once more,
    //     adding each link's index to the proper 
    //     position in the adjacency list
    for (j = 0; j < Nobjects[LINK]; j++)
    {
        i = Link[j].node1;
        k = StartPos[i] + Node[i].degree;
        AdjList[k] = j;
        Node[i].degree++;
        if ( listType == UNDIRECTED )
        {
            i = Link[j].node2;
            k = StartPos[i] + Node[i].degree;
            AdjList[k] = j;
            Node[i].degree++;
        }
    }
}

//=============================================================================

void adjustAdjList()
//
//  Input:   none
//  Output:  none
//  Purpose: adjusts adjacency list for Divider nodes so that non-
//           diversion link appears before diversion link.
//
{
    int i, j, k, m;

    // --- check each node
    for (i=0; i<Nobjects[NODE]; i++)
    {
        // --- skip nodes that are not Dividers
        if ( Node[i].type != DIVIDER ) continue;
        if ( Node[i].degree != 2 ) continue;

        // --- switch position of outgoing links at the node if the
        //     diversion link appears first in the adjacency list
        k = Node[i].subIndex;
        m = StartPos[i];
        j = AdjList[m];
        if ( j == Divider[k].link )
        {
            AdjList[m] = AdjList[m+1];
            AdjList[m+1] = j;
        }
    }
}

//=============================================================================

int topoSort(int sortedLinks[])
//
//  Input:   none
//  Output:  sortedLinks = array of sorted link indexes,
//           returns number of links successfully sorted
//  Purpose: performs a stack-based topo sort of the drainage network's links.
//
{
    int i, j, k, n;
    int i1, i2, k1, k2;

    // --- initialize a stack which contains nodes with zero in-degree
    First = 0;
    Last = -1;
    for (i = 0; i < Nobjects[NODE]; i++)
    {
        if ( InDegree[i] == 0 )
        {
            Last++;
            Stack[Last] = i;
        }
    }

    // --- traverse the stack, adding each node's outgoing link indexes
    //     to the SortedLinks array in the order processed
    n = 0;
    while ( First <= Last )
    {
        // --- determine range of adjacency list indexes belonging to 
        //     first node remaining on the stack
        i1 = Stack[First];
        k1 = StartPos[i1];
        k2 = k1 + Node[i1].degree;

        // --- for each outgoing link from first node on stack
        for (k = k1; k < k2; k++)
        {
            // --- add link index to current position in SortedLinks
            j = AdjList[k];
            sortedLinks[n] = j;
            n++;

            // --- reduce in-degree of link's downstream node
            i2 = Link[j].node2;
            InDegree[i2]--;

            // --- add downstream node to stack if its in-degree is zero
            if ( InDegree[i2] == 0 )
            {
                Last++;
                Stack[Last] = i2;
            }  
        }
        First++;
    }
    return n;
}

//=============================================================================

void  findCycles()
//
//  Input:   none
//  Output:  none
//  Purpose: finds all cycles in the drainage network (i.e., closed loops that
//           start and end at the same node).
//
{
    int i;

    // --- allocate arrays
    AdjList  = (int *) calloc(2*Nobjects[LINK], sizeof(int));
    StartPos = (int *) calloc(Nobjects[NODE], sizeof(int));
    Stack    = (int *) calloc(Nobjects[NODE], sizeof(int));
    Examined = (char *) calloc(Nobjects[NODE], sizeof(char));
    InTree   = (char *) calloc(Nobjects[LINK], sizeof(char));
    LoopLinks = (int *) calloc(Nobjects[LINK], sizeof(int));
    if ( StartPos && AdjList && Stack && Examined && InTree && LoopLinks )
    {
        // --- create an undirected adjacency list for the nodes
        createAdjList(UNDIRECTED);

        // --- set to empty the list of nodes examined and the list
        //     of links in the spanning tree
        for ( i=0; i<Nobjects[NODE]; i++) Examined[i] = 0;
        for ( i=0; i<Nobjects[LINK]; i++) InTree[i] = 0;

        // --- find a spanning tree for each unexamined node
        //     (cycles are identified as tree is constructed)
        for ( i=0; i<Nobjects[NODE]; i++)
        {
            if ( Examined[i] ) continue;
            Last = -1;
            findSpanningTree(i);
        }
    }
    FREE(StartPos);
    FREE(AdjList);
    FREE(Stack);
    FREE(Examined);
    FREE(InTree);
    FREE(LoopLinks);
}

//=============================================================================

void  findSpanningTree(int startNode)
//
//  Input:   i = index of starting node of tree
//  Output:  none
//  Purpose: finds continuation of network's spanning tree of links.
//
{
    int nextNode, j, k, m;

    // --- examine each link connected to node i
    for ( m = StartPos[startNode];
          m < StartPos[startNode]+Node[startNode].degree; m++ )
    {
        // --- find which node (j) connects link k from start node
        k = AdjList[m];
        if ( Link[k].node1 == startNode ) j = Link[k].node2;
        else j = Link[k].node1;

        // --- if link k is not in the tree
        if ( InTree[k] == 0 )
        {
            // --- if connecting node already examined,
            //     then link k forms a loop; mark it as a chord
            //     and check if loop forms a cycle
            if ( Examined[j] )
            {
                InTree[k] = 2;
                evalLoop(k);
            }

            // --- otherwise mark connected node as being examined,
            //     add it to the node stack, and mark the connecting
            //     link as being in the spanning tree
            else
            {
                Examined[j] = 1;
                Last++;
                Stack[Last] = j;
                InTree[k] = 1;
            }
        }
    }

    // --- continue to grow the spanning tree from
    //     the last node added to the stack
    if ( Last >= 0 )
    {
        nextNode = Stack[Last];
        Last--;
        findSpanningTree(nextNode);
    }
}

//=============================================================================

void evalLoop(int startLink)
//
//  Input:   startLink = index of starting link of a loop
//  Output:  none
//  Purpose: checks if loop starting with a given link forms a closed cycle.
//
{
    int i;                             // loop list index
    int i1, i2;                        // start & end node indexes
    int j;                             // index of link in loop
    int kount;                         // items per line counter
    int isCycle;                       // TRUE if loop forms a cycle

    // --- make startLink the first link in the loop
    LoopLinksLast = 0;
    LoopLinks[0] = startLink;

    // --- trace a path on the spanning tree that starts at the
    //     tail node of startLink and ends at its head node
    i1 = Link[startLink].node1;
    i2 = Link[startLink].node2;
    if ( !traceLoop(i1, i2, startLink) ) return;

    // --- check if all links on the path are oriented head-to-tail
    isCycle = TRUE;
    j = LoopLinks[0];
    i2 = Link[j].node2;
    for (i=1; i<=LoopLinksLast; i++)
    {
        j = LoopLinks[i];
        i1 = Link[j].node1;
        if ( i1 == i2 ) i2 = Link[j].node2;
        else
        {
            isCycle = FALSE;
            break;
        }
    }

    // --- print cycle to report file
    if ( isCycle )
    {
        kount = 0;
        for (i = 0; i <= LoopLinksLast; i++)
        {
            if ( kount % 5 == 0 ) fprintf(Frpt.file, "\n");
            kount++;
            fprintf(Frpt.file, "  %s", Link[LoopLinks[i]].ID);
            if ( i < LoopLinksLast ) fprintf(Frpt.file, "  -->");
        }
    }
}

//=============================================================================

int traceLoop(int i1, int i2, int k1)
//
//  Input:   i1 = index of current node reached while tracing a loop
//           i2 = index of final node on the loop
//           k1 = index of link which extends loop to node i1
//  Output:  returns TRUE if loop can be extended through node i1
//  Purpose: tries to extend closed loop through current node.
//
{
    int j, k, m;

    // --- if current node equals final node then return with loop found
    if ( i1 == i2 ) return TRUE;

    // --- examine each link connected to current node
    for (m = StartPos[i1]; m < StartPos[i1] + Node[i1].degree; m++)
    {
        // --- ignore link if it comes from predecessor node or if
        //     it is not on the spanning tree
        k = AdjList[m];
        if ( k == k1 || InTree[k] != 1 ) continue;

        // --- identify other node that link is connected to
        if ( Link[k].node1 == i1 ) j = Link[k].node2;
        else                       j = Link[k].node1;

        // --- try to continue tracing the loop from this node;
        //     if successful, then add link to loop and return
        if ( traceLoop(j, i2, k) )
        {
            LoopLinksLast++;
            LoopLinks[LoopLinksLast] = k;
            return TRUE;
        }
    }

    // --- return false if loop cannot be continued from current node
    return FALSE;
}

//=============================================================================

void checkDummyLinks()
//
//  Input:   none
//  Output:  none
//  Purpose: checks for nodes that have both incoming and outgoing dummy links.
//
{
    int   i, j;
    int* marked;

    // --- create an array that marks nodes
    //     (calloc initializes the array to 0)
    marked = (int *) calloc(Nobjects[NODE], sizeof(int));
    if ( marked == NULL )
    {
        report_writeErrorMsg(ERR_MEMORY, "");
        return;
    }

    // --- mark nodes that whose incoming links are all
    //     either dummy links or ideal pumps
    for ( i = 0; i < Nobjects[LINK]; i++ )
    {
        j = Link[i].node2;
        if ( Link[i].direction < 0 ) j = Link[i].node1;
        if ( (Link[i].type == CONDUIT && Link[i].xsect.type == DUMMY) ||
             (Link[i].type == PUMP &&
              Pump[Link[i].subIndex].type == IDEAL_PUMP) )
        {
            if ( marked[j] == 0 ) marked[j] = 1;
        }
        else marked[j] = -1;
    }

    // --- find marked nodes with outgoing dummy links or ideal pumps
    for ( i = 0; i < Nobjects[LINK]; i++ )
    {
        if ( (Link[i].type == CONDUIT && Link[i].xsect.type == DUMMY) ||
             (Link[i].type == PUMP && 
              Pump[Link[i].subIndex].type == IDEAL_PUMP) )
        {
            j = Link[i].node1;
            if ( marked[j] > 0 )
            {
                report_writeErrorMsg(ERR_DUMMY_LINK, Node[j].ID);
            }
        }
    }
    FREE(marked);
}

//=============================================================================
