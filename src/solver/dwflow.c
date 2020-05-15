//-----------------------------------------------------------------------------
//   dwflow.c
//
//   Project:  EPA SWMM5
//   Version:  5.1
//   Date:     03/01/20   (Build 5.1.015)
//   Author:   L. Rossman (EPA)
//             M. Tryby (EPA)
//             R. Dickinson (CDM)
//
//   Solves the momentum equation for flow in a conduit under dynamic wave
//   flow routing.
//
//   Completely refactored for release 5.1.015.
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include "headers.h"
#include <math.h>

// Flow cross-section variables:
//   y = depth
//   h = head
//   a = area
//   r = hydraulic radius
//   w = top width
// Location notation:
//   1 = upstream
//   2 = downstream
//   Mid = midstream

// Intermediate conduit data
typedef struct
{
    double y1, y2;                // upstream/downstream flow depth (ft)
    double h1, h2;                // upstream/downstream hydraulic head (ft)
    double a1, a2;                // upstream/downstream flow area (ft2)
    double r1;                    // upstream hydraulic radius (ft)
    double yMid, aMid, rMid;      // mid-stream values of y, a, and r
    double yFull;                 // conduit full depth (ft)
    double yCrit;                 // critical flow depth (ft)
    double fasnh;                 // fraction between normal & critical depth
    double aWtd, rWtd;            // upstream weighted area & hyd. radius
    double flow;                  // estimated flow for current time step (cfs)
    double aOld;                  // computed area from previous time step (ft2)
    double velocity;              // flow velocity (ft/sec)
    double sigma;                 // inertial damping factor
    double length;                // effective conduit length (ft)
    int    isFull;                // TRUE if conduit flows full
    int    linkIndex;             // index of conduit's parent link
} ConduitData;

static const  double MAXVELOCITY =  50.;     // max. allowable velocity (ft/sec)

static void   initConduitData(TLink *link, ConduitData *cd);
static void   setFlowDepth(TNode *node, double hInvert, double yFull,
              double *h, double *y);

static void   findFlowClass(TLink *link, ConduitData *cd);
static int    getWetNegativeFlowClass(TLink *link, ConduitData *cd, double yOffset1);
static int    getWetPositiveFlowClass(TLink *link, ConduitData *cd, double yOffset2);
static int    getDryToWetFlowClass(TLink *link, ConduitData *cd, double yOffset1);
static int    getWetToDryFlowClass(TLink *link, ConduitData *cd, double yOffset2);

static void   computeSurfaceArea(TLink *link, ConduitData *cd);
static void   getSubCriticalArea(TLink *link, ConduitData *cd);
static void   getUpCriticalArea(TLink *link, ConduitData *cd);
static void   getDownCriticalArea(TLink *link, ConduitData *cd);
static void   getUpDryArea(TLink *link, ConduitData *cd);
static void   getDownDryArea(TLink *link, ConduitData *cd);

static void   computeFlowSectionGeometry(TLink *link, ConduitData *cd);
static int    conduitIsDryOrClosed(TLink *link, ConduitData *cd, double timeStep);
static void   applyInertialDamping(TLink *link, ConduitData *cd);
static double solveMomentumEqn(TLink *link, ConduitData *cd, double timeStep);
static double findLocalLosses(TLink *link, ConduitData *cd);

static double checkForCulvertInletControl(TLink *link, ConduitData *cd, double flow);
static double checkForNormalFlowControl(TLink *link, ConduitData *cd, double flow);
static int    hasSlopeBasedNormalFlow(ConduitData *cd, int hasOutfall);
static int    hasFroudeBasedNormalFlow(ConduitData *cd, int hasOutfall, double flow);
static double checkNormalFlowValue(TLink *link, ConduitData *cd, double flow);

static double applyUnderRelaxation(ConduitData *cd, double omega, double flow);
static double checkImposedFlowLimits(TLink *link, ConduitData *cd, double flow);
static void   saveFlowResult(TLink *link, ConduitData *cd, double flow);

static double getWidth(TXsect *xsect, double y);
static double getSlotWidth(TXsect *xsect, double y);
static double getArea(TXsect *xsect, double y, double wSlot);
static double getHydRad(TXsect *xsect, double y);

//=============================================================================

void  dwflow_findConduitFlow(int linkIndex, int trials, double omega, double timeStep)
//
//  Updates flow in a conduit link by solving finite difference form of combined
//  St. Venant continuity and momentum equations.
{
    double flow;        // new conduit flow estimate (per barrel) (cfs)
    TLink *link;        // conduit's parent link
    ConduitData cd;     // intermediate conduit data

    link = &Link[linkIndex];
    cd.linkIndex = linkIndex;

    initConduitData(link, &cd);
    findFlowClass(link, &cd);
    computeSurfaceArea(link, &cd);
    computeFlowSectionGeometry(link, &cd);
    if (conduitIsDryOrClosed(link, &cd, timeStep))
        return;
    applyInertialDamping(link, &cd);

    flow = solveMomentumEqn(link, &cd, timeStep);
    flow = checkForCulvertInletControl(link, &cd, flow);
    flow = checkForNormalFlowControl(link, &cd, flow);
    if (trials > 0)
        flow = applyUnderRelaxation(&cd, omega, flow);
    flow = checkImposedFlowLimits(link, &cd, flow);
    saveFlowResult(link, &cd, flow);
}

//=============================================================================

void initConduitData(TLink *link, ConduitData *cd)
{
    int    k;           // conduit index
    double hInvert;     // conduit's invert elevation (ft)
    TNode *node;        // node object

    k = link->subIndex;
    cd->flow = Conduit[k].q1;
    cd->yFull = link->xsect.yFull;
    cd->aOld = MAX(Conduit[k].a2, FUDGE);
    cd->length = Conduit[k].modLength;
    cd->fasnh = 1.0;
    cd->yCrit = cd->yFull;

    node = &Node[link->node1];
    hInvert = node->invertElev + link->offset1;
    setFlowDepth(node, hInvert, cd->yFull, &cd->h1, &cd->y1);
    node = &Node[link->node2];
    hInvert = node->invertElev + link->offset2;
    setFlowDepth(node, hInvert, cd->yFull, &cd->h2, &cd->y2);

    Conduit[k].evapLossRate = 0.0;
    Conduit[k].seepLossRate = 0.0;
}

//=============================================================================

void setFlowDepth(TNode *node, double hInvert, double yFull, double *h, double *y)
//
//  Set the head (h) and flow depth (y) of a conduit connected to a specified node.
{
    *h = node->newDepth + node->invertElev;
    *h = MAX(*h, hInvert);
    *y = *h - hInvert;
    *y = MAX(*y, FUDGE);
    if (SurchargeMethod != SLOT) *y = MIN(*y, yFull);
}

//=============================================================================

void findFlowClass(TLink *link, ConduitData *cd)
//
//  Find the type of flow a conduit is experiencing.
{
    int    n1 = link->node1,
           n2 = link->node2;
    double yOffset1, yOffset2;

    // --- get upstream & downstream conduit invert offsets
    yOffset1 = link->offset1;
    yOffset2 = link->offset2;
    if (Node[n1].type == OUTFALL)
        yOffset1 = MAX(0.0, (yOffset1 - Node[n1].newDepth));
    if (Node[n2].type == OUTFALL)
        yOffset2 = MAX(0.0, (yOffset2 - Node[n2].newDepth));

    // --- default class is SUBCRITICAL
    link->flowClass = SUBCRITICAL;
    cd->fasnh = 1.0;

    // -- conduit is full
    if (cd->y1 >= cd->yFull && cd->y2 >= cd->yFull)
        link->flowClass = SUBCRITICAL;

    // --- both ends of conduit are wet
    else if (cd->y1 > FUDGE && cd->y2 > FUDGE)
    {
        if (cd->flow < 0.0)
            link->flowClass = getWetNegativeFlowClass(link, cd, yOffset1);
        else
            link->flowClass = getWetPositiveFlowClass(link, cd, yOffset2);
    }

    // --- no flow at either end of conduit
    else if (cd->y1 <= FUDGE && cd->y2 <= FUDGE)
        link->flowClass = DRY;

    // --- downstream end of conduit is wet, upstream dry
    else if (cd->y2 > FUDGE)
        link->flowClass = getDryToWetFlowClass(link, cd, yOffset1);

    // --- upstream end of conduit is wet, downstream dry
    else
        link->flowClass = getWetToDryFlowClass(link, cd, yOffset2);
}

//=============================================================================

int getWetNegativeFlowClass(TLink *link, ConduitData *cd, double yOffset1)
//
//  Determine flow class for a fully wetted conduit with reverse flow.
{
    double flow = fabs(cd->flow);
    int    flowClass = SUBCRITICAL;

    // --- upstream end at critical depth if flow depth is
    //     below conduit's critical depth and an upstream
    //     conduit offset exists
    if (yOffset1 > 0.0)
    {
        cd->yCrit = MIN(link_getYnorm(cd->linkIndex, flow),
                        link_getYcrit(cd->linkIndex, flow));
        if (cd->y1 < cd->yCrit) flowClass = UP_CRITICAL;
    }
    return flowClass;
}

//=============================================================================

int getWetPositiveFlowClass(TLink *link, ConduitData *cd, double yOffset2)
//
//  Determine flow class for a fully wetted conduit with positive flow.
{
    double flow = fabs(cd->flow);
    double yNorm, yCrit;
    double ycMin, ycMax;
    int    flowClass = SUBCRITICAL;

    // --- conduit has a downstream offset
    if (yOffset2 > 0.0)
    {
        yNorm = link_getYnorm(cd->linkIndex, flow);
        yCrit = link_getYcrit(cd->linkIndex, flow);
        ycMin = MIN(yNorm, yCrit);
        ycMax = MAX(yNorm, yCrit);

        // --- if downstream depth < smaller critical depth
        //     then flow class is Downstream Critical
        if (cd->y2 < ycMin) flowClass = DN_CRITICAL;

        // --- if downstream depth between critical & normal
        //     depth compute a weighting factor (fasnh) to
        //     apply to downstream surface area
        else if (cd->y2 < ycMax)
        {
            if (ycMax - ycMin < FUDGE)
                cd->fasnh = 0.0;
            else
                cd->fasnh = (ycMax - cd->y2) / (ycMax - ycMin);
        }
        cd->yCrit = ycMin;
    }
    return flowClass;
}

//=============================================================================

int getDryToWetFlowClass(TLink *link, ConduitData *cd, double yOffset1)
//
//  Determine flow class for a conduit that is dry at upstream end and
//  wet at downstream end.
{
    double flow = fabs(cd->flow);
    int flowClass = SUBCRITICAL;

    // --- flow classification is UP_DRY if downstream head <
    //     invert of upstream end of conduit
    if (cd->h2 < Node[link->node1].invertElev + link->offset1)
        flowClass = UP_DRY;

    // --- otherwise, the downstream head will be >= upstream
    //     conduit invert creating a flow reversal and upstream end
    //     should be at critical depth, providing that an upstream
    //     offset exists (otherwise subcritical condition is maintained)
    else if (yOffset1 > 0.0)
    {
        cd->yCrit = MIN(link_getYnorm(cd->linkIndex, flow),
                        link_getYcrit(cd->linkIndex, flow));
        flowClass = UP_CRITICAL;
    }
    return flowClass;
}

//=============================================================================

int getWetToDryFlowClass(TLink *link, ConduitData *cd, double yOffset2)
//
//  Determine flow class for a conduit that is wet at upstream end and
//  dry at downstream end.
{
    double flow = fabs(cd->flow);
    int flowClass = SUBCRITICAL;
    
    // --- flow classification is DN_DRY if upstream head <
    //     invert of downstream end of conduit
    if (cd->h1 < Node[link->node2].invertElev + link->offset2)
        flowClass = DN_DRY;

    // --- otherwise flow at downstream end should be at critical depth
    //     providing that a downstream offset exists (otherwise
    //     subcritical condition is maintained)
    else if (yOffset2 > 0.0)
    {
        cd->yCrit = MIN(link_getYnorm(cd->linkIndex, flow),
                        link_getYcrit(cd->linkIndex, flow));
        flowClass = DN_CRITICAL;
    }
    return flowClass;
}

//=============================================================================

void computeSurfaceArea(TLink *link, ConduitData *cd)
//
//  Compute surface area that conduit contributes to its end nodes.  
{
    switch (link->flowClass)
    {
    case SUBCRITICAL:
        getSubCriticalArea(link, cd);
        break;
    case UP_CRITICAL:
        getUpCriticalArea(link, cd);
        break;
    case DN_CRITICAL:
        getDownCriticalArea(link, cd);
        break;
    case UP_DRY:
        getUpDryArea(link, cd);
        break;
    case DN_DRY:
        getDownDryArea(link, cd);
        break;
    case DRY:
        link->surfArea1 = FUDGE * cd->length / 2.0;
        link->surfArea2 = link->surfArea1;
        break;
    }
}

//=============================================================================

void getSubCriticalArea(TLink *link, ConduitData *cd)
//
//  Conduit surface area when neither end is dry or has critical flow.
{
    double w1, w2, wMid;
    TXsect *xsect = &link->xsect;

    cd->yMid = 0.5 * (cd->y1 + cd->y2);
    if (cd->yMid < FUDGE) cd->yMid = FUDGE;
    w1 = getWidth(xsect, cd->y1);
    w2 = getWidth(xsect, cd->y2);
    wMid = getWidth(xsect, cd->yMid);

    // --- assign each end the avg. area over its half of the conduit
    link->surfArea1 = (w1 + wMid) / 2. * cd->length / 2.;
    link->surfArea2 = (wMid + w2) / 2. * cd->length / 2. * (cd->fasnh);
}

//=============================================================================

void getUpCriticalArea(TLink *link, ConduitData *cd)
//
//  Conduit surface area when upstream end has critical flow.
{
    double w2, wMid;
    TXsect *xsect = &link->xsect;

    cd->y1 = MAX(cd->yCrit, FUDGE);
    cd->h1 = Node[link->node1].invertElev + link->offset1 + cd->y1;
    cd->yMid = 0.5 * (cd->y1 + cd->y2);
    if (cd->yMid < FUDGE) cd->yMid = FUDGE;
    w2 = getWidth(xsect, cd->y2);
    wMid = getWidth(xsect, cd->yMid);

    // --- assign downstream end avg. area over full length
    link->surfArea2 = (wMid + w2) / 2. * cd->length;
    link->surfArea1 = 0.0;
}

//=============================================================================

void getDownCriticalArea(TLink *link, ConduitData *cd)
//
//  Conduit surface area when downstream end has critical flow.
{
    double w1, wMid;
    TXsect *xsect = &link->xsect;

    cd->y2 = MAX(cd->yCrit, FUDGE);
    cd->h2 = Node[link->node2].invertElev + link->offset2 + cd->y2;
    w1 = getWidth(xsect, cd->y1);
    cd->yMid = 0.5 * (cd->y1 + cd->y2);
    if (cd->yMid < FUDGE) cd->yMid = FUDGE;
    wMid = getWidth(xsect, cd->yMid);

    // --- assign upstream end avg. surface area over full length
    link->surfArea1 = (w1 + wMid) / 2. * cd->length;
    link->surfArea2 = 0.0;
}

//=============================================================================

void getUpDryArea(TLink *link, ConduitData *cd)
//
//  Conduit surface area when upstream end is dry.
{
    double w1, w2, wMid;
    TXsect *xsect = &link->xsect;

    cd->y1 = FUDGE;
    cd->yMid = 0.5 * (cd->y1 + cd->y2);
    if (cd->yMid < FUDGE) cd->yMid = FUDGE;
    w1 = getWidth(xsect, cd->y1);
    w2 = getWidth(xsect, cd->y2);
    wMid = getWidth(xsect, cd->yMid);

    // --- assign avg. surface area of downstream half of conduit
    //     to the downstream node
    link->surfArea2 = (wMid + w2) / 2. * cd->length / 2.;

    // --- if there is no free-fall at upstream end, assign the
    //     upstream node the avg. surface area of the upstream half
    if (link->offset1 <= 0.0)
        link->surfArea1 = (w1 + wMid) / 2. * cd->length / 2.;
    else
        link->surfArea1 = 0.0;
}

//=============================================================================

void getDownDryArea(TLink *link, ConduitData *cd)
//
//  Conduit surface area when downstream end is dry.
{
    double w1, w2, wMid;
    TXsect *xsect = &link->xsect;

    cd->y2 = FUDGE;
    cd->yMid = 0.5 * (cd->y1 + cd->y2);
    if (cd->yMid < FUDGE) cd->yMid = FUDGE;
    w1 = getWidth(xsect, cd->y1);
    w2 = getWidth(xsect, cd->y2);
    wMid = getWidth(xsect, cd->yMid);

    // --- assign avg. surface area of upstream half of conduit
    //     to the upstream node
    link->surfArea1 = (wMid + w1) / 2. * cd->length / 2.;

    // --- if there is no free-fall at downstream end, assign the
    //     downstream node the avg. surface area of the downstream half
    if (link->offset2 <= 0.0)
        link->surfArea2 = (w2 + wMid) / 2. * cd->length / 2.;
    else
        link->surfArea2 = 0.0;
}

//=============================================================================

void  computeFlowSectionGeometry(TLink *link, ConduitData *cd)
//
//  Compute flow area and hydraulic radius for flow depths at each end
//  and midpoint of a conduit.
{
    double wSlot;       // Preissmann slot width (ft)
    TXsect *xsect = &link->xsect;

    wSlot = getSlotWidth(xsect, cd->y1);
    cd->a1 = getArea(xsect, cd->y1, wSlot); 
    cd->r1 = getHydRad(xsect, cd->y1);
    wSlot = getSlotWidth(xsect, cd->y2);
    cd->a2 = getArea(xsect, cd->y2, wSlot);
    
    cd->yMid = 0.5 * (cd->y1 + cd->y2);
    wSlot = getSlotWidth(xsect, cd->yMid);
    cd->aMid = getArea(xsect, cd->yMid, wSlot);
    cd->rMid = getHydRad(xsect, cd->yMid);

    // --- alternate approach not currently used, but might produce better
    //     Bernoulli energy balance for steady flows
    //aMid = (a1+a2)/2.0;
    //rMid = (r1+getHydRad(xsect,y2))/2.0;

    // --- check if conduit is flowing full
    cd->isFull = (cd->y1 >= cd->yFull &&
                  cd->y2 >= cd->yFull);
}

//=============================================================================

int conduitIsDryOrClosed(TLink *link, ConduitData *cd, double timeStep)
//
//  Set flow to 0 if conduit is dry or closed.
{
    int k = link->subIndex;

    if (link->flowClass == DRY ||
        link->flowClass == UP_DRY ||
        link->flowClass == DN_DRY ||
        link->setting == 0.0 ||
        cd->aMid <= FUDGE)
    {
        Conduit[k].a1 = 0.5 * (cd->a1 + cd->a2);
        Conduit[k].q1 = 0.0;;
        Conduit[k].q2 = 0.0;
        Conduit[k].fullState = 0;
        link->dqdh = GRAVITY * timeStep * cd->aMid / cd->length *
                     Conduit[k].barrels;
        link->froude = 0.0;
        link->newDepth = MIN(cd->yMid, cd->yFull);
        link->newVolume = Conduit[k].a1 * link_getLength(cd->linkIndex) *
                          Conduit[k].barrels;
        link->newFlow = 0.0;
        return TRUE;
    }
    return FALSE;
}

//=============================================================================

void  applyInertialDamping(TLink *link, ConduitData *cd)
//
//  Apply inertial damping factor to weight conduit's
//  average area and hydraulic radius with upstream values.
{
    double rho;
    double Fr;

    // --- compute velocity from last flow estimate
    cd->velocity = cd->flow / cd->aMid;
    if ( fabs(cd->velocity) > MAXVELOCITY )
        cd->velocity = MAXVELOCITY * SGN(cd->flow);

    // --- compute Froude No.
    Fr = link_getFroude(cd->linkIndex, cd->velocity, cd->yMid);
    if (link->flowClass == SUBCRITICAL && Fr > 1.0)
        link->flowClass = SUPCRITICAL;

    // --- find inertial damping factor (sigma)
    if ( Fr <= 0.5 )
        cd->sigma = 1.0;
    else if ( Fr >= 1.0 )
        cd->sigma = 0.0;
    else
        cd->sigma = 2.0 * (1.0 - Fr);
    link->froude = Fr;

    // --- get upstream-weighted area & hyd. radius based on damping factor
    //     (modified version of R. Dickinson's slope weighting)
    rho = 1.0;
    if ( !cd->isFull && cd->flow > 0.0 && cd->h1 >= cd->h2 )
        rho = cd->sigma;
    cd->aWtd = cd->a1 + (cd->aMid - cd->a1) * rho;
    cd->rWtd = cd->r1 + (cd->rMid - cd->r1) * rho;

    // --- determine how much inertial damping to apply
    if ( InertDamping == NO_DAMPING )
        cd->sigma = 1.0;
    else if ( InertDamping == FULL_DAMPING )
        cd->sigma = 0.0;

    // --- use full inertial damping if closed conduit is surcharged
    if (cd->isFull && !xsect_isOpen(link->xsect.type))
        cd->sigma = 0.0;
}

//=============================================================================

double solveMomentumEqn(TLink *link, ConduitData *cd, double timeStep)
//
//  Solve St. Venant momentum equation for conduit flow over a time step.
{
    int    k = link->subIndex;
    int    linkIndex = cd->linkIndex;
    double flow;                         // new flow value (cfs)
    double dq1, dq2, dq3, dq4, dq5, dq6; // terms in momentum eqn.
    double denom;
    TXsect* xsect = &link->xsect;

    // --- 1. friction slope term
    if (xsect->type == FORCE_MAIN && cd->isFull)
        dq1 = timeStep *
              forcemain_getFricSlope(linkIndex, fabs(cd->velocity), cd->rMid);
    else
        dq1 = timeStep * Conduit[k].roughFactor / pow(cd->rWtd, 1.33333) *
              fabs(cd->velocity);

    // --- 2. energy slope term
    dq2 = timeStep * GRAVITY * cd->aWtd * (cd->h2 - cd->h1) / cd->length;

    // --- 3 & 4. inertial terms
    dq3 = 0.0;
    dq4 = 0.0;
    if (cd->sigma > 0.0)
    {
        dq3 = 2.0 * cd->velocity * (cd->aMid - cd->aOld) * cd->sigma;
        dq4 = timeStep * cd->velocity * cd->velocity *
              (cd->a2 - cd->a1) / cd->length * cd->sigma;
    }

    // --- 5. local losses term
    dq5 = 0.0;
    if (Conduit[k].hasLosses)
        dq5 = findLocalLosses(link, cd) / 2.0 / cd->length * timeStep;

    // --- 6. term for evap and seepage losses per unit length
    dq6 = link_getLossRate(linkIndex, cd->flow) * 2.5 * timeStep *
          cd->velocity / link_getLength(linkIndex);

    // --- combine terms to find new conduit flow
    denom = 1.0 + dq1 + dq5;
    flow = link->oldFlow / Conduit[k].barrels;
    flow = (flow - dq2 + dq3 + dq4 + dq6) / denom;

    // --- compute derivative of flow w.r.t. head
    link->dqdh = 1.0 / denom  * GRAVITY * timeStep * cd->aWtd / cd->length *
                 Conduit[k].barrels;
    return flow;
}

//=============================================================================

double findLocalLosses(TLink *link, ConduitData* cd)
//
//  Compute local losses term of a conduit's momentum equation.
{
    double losses = 0.0;
    double flow = fabs(cd->flow);

    if ( cd->a1 > FUDGE ) losses += link->cLossInlet  * (flow/cd->a1);
    if ( cd->a2 > FUDGE ) losses += link->cLossOutlet * (flow/cd->a2);
    if ( cd->aMid  > FUDGE ) losses += link->cLossAvg * (flow/cd->aMid);
    return losses;
}

//=============================================================================

double checkForCulvertInletControl(TLink *link, ConduitData *cd, double flow)
//
//  Check if conduit flow is subject to culvert inlet control.
{
    link->inletControl = FALSE;
    if (flow > 0.0 && link->xsect.culvertCode > 0 && !cd->isFull)
        flow = culvert_getInflow(cd->linkIndex, flow, cd->h1);
    return flow;
}

//=============================================================================

double checkForNormalFlowControl(TLink *link, ConduitData *cd, double flow)
//
//  Check if conduit flow should be reduced to normal Manning flow value.
{
    int    hasOutfall;

    link->normalFlow = FALSE;
    if (link->inletControl || cd->isFull)
        return flow;
    if (link->flowClass == SUBCRITICAL || link->flowClass == SUPCRITICAL)
    {
        hasOutfall = (Node[link->node1].type == OUTFALL ||
                      Node[link->node2].type == OUTFALL);
        if (hasSlopeBasedNormalFlow(cd, hasOutfall) ||
            hasFroudeBasedNormalFlow(cd, hasOutfall, flow))
            flow = checkNormalFlowValue(link, cd, flow);
    }
    return flow;
}

//=============================================================================

int hasSlopeBasedNormalFlow(ConduitData *cd, int hasOutfall)
//
//  Check if upstream flow depth is lower than downstream depth.
{
    if (NormalFlowLtd == SLOPE || NormalFlowLtd == BOTH || hasOutfall)
        return (cd->y1 < cd->y2);
    return FALSE;
}

//=============================================================================

int hasFroudeBasedNormalFlow(ConduitData *cd, int hasOutfall, double flow)
//
//  Check if Froude number at upstream end of conduit is >= 1.
{
    if ( (NormalFlowLtd == FROUDE || NormalFlowLtd == BOTH) && !hasOutfall)
    {
        if (link_getFroude(cd->linkIndex, flow / cd->a1, cd->y1) >= 1.0)
            return TRUE;
    }
    return FALSE;
}

//=============================================================================

double checkNormalFlowValue(TLink *link, ConduitData *cd, double flow)
//
//  Return smaller of normal Manning flow and current dynamic wave flow.
{
    int    k = link->subIndex;
    double normalFlow = Conduit[k].beta * cd->a1 * pow(cd->r1, 2./3.);

    if ( normalFlow < flow )
    {
        link->normalFlow = TRUE;
        flow = normalFlow;
    }
    return flow;
}

//=============================================================================

double applyUnderRelaxation(ConduitData *cd, double omega, double flow)
//
// Weight current flow estimate with previous estimate.
{
    flow = (1.0 - omega) * cd->flow + omega * flow;

    // --- flow can't switch sign without first being close to 0
    if (flow * cd->flow < 0.0) flow = 0.001 * SGN(flow);
    return flow;
}

//=============================================================================

double checkImposedFlowLimits(TLink *link, ConduitData *cd, double flow)
//
//  Perform additional checks that limit a conduit's flow.
{
    int n1 = link->node1;
    int n2 = link->node2;

    // --- check if user-supplied flow limit applies
    if (link->qLimit > 0.0)
    {
        if (fabs(flow) > link->qLimit) flow = SGN(flow) * link->qLimit;
    }

    // --- check for reverse flow with closed flap gate
    if (link_setFlapGate(cd->linkIndex, n1, n2, flow)) flow = 0.0;

    // --- do not allow flow out of a dry node
    //     (as suggested by R. Dickinson)
    if (flow > FUDGE && Node[n1].newDepth <= FUDGE)
        flow = FUDGE;
    if (flow < -FUDGE && Node[n2].newDepth <= FUDGE)
        flow = -FUDGE;
    return flow;
}

//=============================================================================

void saveFlowResult(TLink *link, ConduitData *cd, double flow)
{
    int k = link->subIndex;
    double aFull = link->xsect.aFull;
    double aAvg = (cd->a1 + cd->a2) / 2.0;

    Conduit[k].a1 = cd->aMid;
    Conduit[k].q1 = flow;
    Conduit[k].q2 = flow;
    Conduit[k].fullState = link_getFullState(cd->a1, cd->a2, aFull);
    link->newDepth = MIN(cd->yMid, cd->yFull);
    link->newVolume = aAvg * link_getLength(cd->linkIndex) * Conduit[k].barrels;
    link->newFlow = flow * Conduit[k].barrels;
}

//=============================================================================

double getSlotWidth(TXsect* xsect, double y)
//
//  Compute width of Preissmann slot atop a conduit at surcharged depth y.
{
    double yNorm = y / xsect->yFull;

    // --- check if slot is needed
    if (SurchargeMethod != SLOT || xsect_isOpen(xsect->type) ||
        yNorm < CrownCutoff)
        return 0.0;

    // --- for depth > 1.78 * pipe depth, slot width = 1% of max. width
    if (yNorm > 1.78) return xsect->wMax * 0.01;

    // --- otherwise use the Sjoberg formula
    return xsect->wMax * exp(-pow(yNorm, 2.4)) * 0.5423;
}

//=============================================================================

double getWidth(TXsect* xsect, double y)
//
//  Compute top width of conduit cross section (xsect) at flow depth y.
{
    double wSlot = getSlotWidth(xsect, y);
    if (wSlot > 0.0) return wSlot;
    if (y / xsect->yFull >= CrownCutoff && !xsect_isOpen(xsect->type))
        y = CrownCutoff * xsect->yFull;
    return xsect_getWofY(xsect, y);
}

//=============================================================================

double getArea(TXsect* xsect, double y, double wSlot)
//
//  Compute area of conduit cross-section (xsect) at flow depth y.
{
    if ( y >= xsect->yFull )
        return xsect->aFull + (y - xsect->yFull) * wSlot;
    return xsect_getAofY(xsect, y);
}

//=============================================================================

double getHydRad(TXsect* xsect, double y)
//
//  Compute hydraulic radius of conduit cross-section (xsect) at flow depth y.
{
    if (y >= xsect->yFull) return xsect->rFull;
    return xsect_getRofY(xsect, y);
}
