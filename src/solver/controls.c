//-----------------------------------------------------------------------------
//   controls.c
//
//   Project:  EPA SWMM5
//   Version:  5.2
//   Date:     06/01/22  (Build 5.2.1)
//   Author:   L. Rossman
//
//   Rule-based controls functions.
//
//   Control rules have the format:
//     RULE name
//     IF <premise>
//     AND / OR <premise>
//     etc.
//     THEN <action>
//     AND  <action>
//     etc.
//     ELSE <action>
//     AND  <action>
//     etc.
//     PRIORITY <p>
//
//   <premise> consists of:
//      <variable> <relational operator> value / <variable>
//   where <variable> is <object type> <id name> <attribute>
//   E.g.: Node 123 Depth > 4.5
//         Node 456 Depth < Node 123 Depth
//
//   <action> consists of:
//      <variable> = setting
//   E.g.: Pump abc status = OFF
//         Weir xyz setting = 0.5
//
//  Update History
//  ==============
//  Build 5.1.008:
//  - Support added for r.h.s. variables in rule premises.
//  - Node volume added as a premise variable.
//  Build 5.1.009:
//  - Fixed problem with parsing a RHS premise variable.
//  Build 5.1.010:
//  - Support added for link TIMEOPEN & TIMECLOSED premises.
//  Build 5.1.011:
//  - Support added for DAYOFYEAR attribute.
//  - Modulated controls no longer included in reported control actions.
//  Build 5.2.0:
//  - Additional attributes added to condition clauses.
//  - Support added for named variables in condition clauses.
//  - Support added for math expressions in condition clauses.
//  Build 5.2.1:
//  - A refactoring bug from 5.2.0 causing duplicate actions to be added
//    to the list of control actions to take was fixed.
//-----------------------------------------------------------------------------
#define _CRT_SECURE_NO_DEPRECATE

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "headers.h"

//-----------------------------------------------------------------------------
//  Constants
//-----------------------------------------------------------------------------
enum RuleState    {r_RULE, r_IF, r_AND, r_OR, r_THEN, r_ELSE, r_PRIORITY,
                   r_VARIABLE, r_EXPRESSION, r_ERROR};
enum RuleObject   {r_GAGE, r_NODE, r_LINK, r_CONDUIT, r_PUMP, r_ORIFICE,
                   r_WEIR, r_OUTLET, r_SIMULATION};
enum RuleAttrib   {r_DEPTH, r_MAXDEPTH, r_HEAD, r_VOLUME, r_INFLOW,
                   r_FLOW, r_FULLFLOW, r_FULLDEPTH, r_STATUS, r_SETTING,
                   r_LENGTH, r_SLOPE, r_VELOCITY, r_TIMEOPEN, r_TIMECLOSED,
                   r_TIME, r_DATE, r_CLOCKTIME, r_DAYOFYEAR, r_DAY, r_MONTH};
enum RuleRelation {EQ, NE, LT, LE, GT, GE};
enum RuleSetting  {r_CURVE, r_TIMESERIES, r_PID, r_NUMERIC};

#define MAXVARNAME  32

static char* ObjectWords[] =
    {"GAGE", "NODE", "LINK", "CONDUIT", "PUMP", "ORIFICE", "WEIR", "OUTLET",
     "SIMULATION", NULL};
static char* AttribWords[] =
    {"DEPTH", "MAXDEPTH", "HEAD", "VOLUME", "INFLOW",
     "FLOW", "FULLFLOW", "FULLDEPTH", "STATUS", "SETTING",
     "LENGTH", "SLOPE", "VELOCITY", "TIMEOPEN", "TIMECLOSED",
     "TIME", "DATE", "CLOCKTIME", "DAYOFYEAR", "DAY", "MONTH", NULL}; 
static char* RelOpWords[] = {"=", "<>", "<", "<=", ">", ">=", NULL};
static char* StatusWords[]  = {"OFF", "ON", NULL};
static char* ConduitWords[] = {"CLOSED", "OPEN", NULL};
static char* SettingTypeWords[] = {"CURVE", "TIMESERIES", "PID", NULL};
static char* IntensityWord = "INTENSITY";

//-----------------------------------------------------------------------------                  
// Data Structures
//-----------------------------------------------------------------------------
// Rule Premise Variable
struct TVariable
{
   int      object;          // type of object
   int      index;           // index in object's array
   int      attribute;       // object's attribute
};

// Named Variable
struct TNamedVariable
{
    struct TVariable variable;           // a rule premise variable 
    char             name[MAXVARNAME+1]; // name used in math expression
};

// Rule Premise Function 
struct TExpression
{
    MathExpr*  expression;               // tokenized math expression
    char       name[MAXVARNAME+1];       // expression name
};

// Rule Premise Clause 
struct  TPremise
{
    int     type;                 // clause type (IF/AND/OR)
    int     exprIndex;            // expression index (-1 if N/A)
    struct  TVariable lhsVar;     // left hand side variable
    struct  TVariable rhsVar;     // right hand side variable 
    int     relation;             // relational operator (>, <, =, etc)
    double  value;                // right hand side value
    struct  TPremise *next;       // next premise clause of rule
};

// Rule Action Clause
struct  TAction              
{
   int     rule;             // index of rule that action belongs to
   int     link;             // index of link being controlled
   int     attribute;        // attribute of link being controlled
   int     curve;            // index of curve for modulated control
   int     tseries;          // index of time series for modulated control
   double  value;            // control setting for link attribute
   double  kp, ki, kd;       // coeffs. for PID modulated control
   double  e1, e2;           // PID set point error from previous time steps
   struct  TAction *next;    // next action clause of rule
};

// List of Control Actions
struct  TActionList          
{
   struct  TAction* action;
   struct  TActionList* next;
};

// Control Rule
struct  TRule
{
   char*    ID;                        // rule ID
   double   priority;                  // priority level
   struct   TPremise* firstPremise;    // pointer to first premise of rule
   struct   TPremise* lastPremise;     // pointer to last premise of rule
   struct   TAction*  thenActions;     // linked list of actions if true
   struct   TAction*  elseActions;     // linked list of actions if false
};

//-----------------------------------------------------------------------------
//  Shared variables
//-----------------------------------------------------------------------------
struct   TRule*       Rules;           // array of control rules
struct   TActionList* ActionList;      // linked list of control actions
int      InputState;                   // state of rule interpreter
int      RuleCount;                    // total number of rules
double   ControlValue;                 // value of controller variable
double   SetPoint;                     // value of controller setpoint
DateTime CurrentDate;                  // current date in whole days 
DateTime CurrentTime;                  // current time of day (decimal)

int     VariableCount;
int     ExpressionCount;
int     CurrentVariable;
int     CurrentExpression;
struct  TNamedVariable* NamedVariable; // array of named variables
struct  TExpression* Expression;       // array of math expressions

//-----------------------------------------------------------------------------
//  External functions (declared in funcs.h)
//-----------------------------------------------------------------------------
//     controls_create
//     controls_delete
//     controls_init
//     controls_addToCount
//     controls_addVariable
//     controls_addExpression
//     controls_addRuleClause
//     controls_evaluate

//-----------------------------------------------------------------------------
//  Local functions
//-----------------------------------------------------------------------------
int    addPremise(int r, int type, char* Tok[], int nToks);
int    getPremiseVariable(char* tok[], int nToks, int* k, struct TVariable* v);
int    getPremiseValue(char* token, int attrib, double* value);
int    addAction(int r, char* Tok[], int nToks);

int    evaluatePremise(struct TPremise* p, double tStep);
double getVariableValue(struct TVariable v);
int    compareTimes(double lhsValue, int relation, double rhsValue,
       double halfStep);
int    compareValues(double lhsValue, int relation, double rhsValue);

void   updateActionList(struct TAction* a);
int    executeActionList(DateTime currentTime);
void   clearActionList(void);
void   deleteActionList(void);
void   deleteRules(void);

int    findExactMatch(char *s, char *keyword[]);
int    setActionSetting(char* tok[], int nToks, int* curve, int* tseries,
       int* attrib, double value[]);
void   updateActionValue(struct TAction* a, DateTime currentTime, double dt);
double getPIDSetting(struct TAction* a, double dt);

int    getVariableIndex(char* varName);
double getNamedVariableValue(int varIndex);
int    getExpressionIndex(char* exprName);
int    getGageAttrib(char* token);
double getRainValue(struct TVariable v);

//=============================================================================

void controls_init()
//
//  Input:   none
//  Output:  none
//  Purpose: initializes the control rule system.
//
{
    Rules = NULL;
    NamedVariable = NULL;
    Expression = NULL;
    RuleCount = 0;
    VariableCount = 0;
    ExpressionCount = 0;
}

//=============================================================================

void controls_addToCount(char* s)
//
//  Input:   s = either VARIABLE or EXPRESSION
//  Output:  none
//  Purpose: updates the number of named variables or math expressions used
//           by control rules.
//
{
    if (match(s, w_VARIABLE)) VariableCount++;
    else if (match(s, w_EXPRESSION)) ExpressionCount++;
}

//=============================================================================

int  controls_create(int n)
//
//  Input:   n = total number of control rules
//  Output:  returns error code
//  Purpose: creates an array of control rules.
//
{
    int r;
    ActionList = NULL;
    InputState = r_PRIORITY;
    RuleCount = n;
    if (RuleCount > 0)
    {
        Rules = (struct TRule *) calloc(RuleCount, sizeof(struct TRule));
        if (Rules == NULL) return ERR_MEMORY;
        for ( r=0; r<RuleCount; r++ )
        {
            Rules[r].ID = NULL;
            Rules[r].firstPremise = NULL;
            Rules[r].lastPremise = NULL;
            Rules[r].thenActions = NULL;
            Rules[r].elseActions = NULL;
            Rules[r].priority = 0.0;    
        }
   }

   CurrentVariable = -1;
   CurrentExpression = -1;
   if (VariableCount > 0)
   {
       NamedVariable = (struct TNamedVariable *) calloc(VariableCount,
           sizeof(struct TNamedVariable));
       if (NamedVariable == NULL) return ERR_MEMORY;
   }
   if (ExpressionCount > 0)
   {
       Expression = (struct TExpression *) calloc(ExpressionCount,
           sizeof(struct TExpression));
       if (Expression == NULL) return ERR_MEMORY;
   }
   return 0;
}

//=============================================================================

void controls_delete(void)
//
//  Input:   none
//  Output:  none
//  Purpose: deletes all control rules.
//
{
   int i;

   for (i = 0; i < ExpressionCount; i++)
   {
       mathexpr_delete(Expression[i].expression);
       Expression[i].expression = NULL;
   }
   FREE(Expression);
   FREE(NamedVariable);

   if ( RuleCount == 0 ) return;
   deleteActionList();
   deleteRules();
}

//=============================================================================

int  controls_addVariable(char* tok[], int nToks)
//
//  Input:   tok = an array of string tokens
//           n = the size of tok[]
//  Output:  returns error code
//  Purpose: adds a named variable to the control rule system from a
//           tokenized line of input with formats:
//    VARIABLE  name = Object  id  attribute
//    VARIABLE  name = SIMULATION attribute
//
{
    struct TVariable v1;
    int k, err;

    CurrentVariable++;
    if (nToks < 5) return ERR_ITEMS;
    if (findExactMatch(tok[1], AttribWords) >= 0)
        return error_setInpError(ERR_KEYWORD, tok[1]);
    if (!match(tok[2], "=")) return error_setInpError(ERR_KEYWORD, tok[2]);
    if (!match(tok[3], "SIMULATION") && nToks < 6) return ERR_ITEMS;
    k = 3;
    err = getPremiseVariable(tok, nToks, &k, &v1);
    if (err > 0) return err;
    k = CurrentVariable;
    NamedVariable[k].variable = v1;
    sstrncpy(NamedVariable[k].name, tok[1], MAXVARNAME);
    return 0;
}

//=============================================================================

int  controls_addExpression(char* tok[], int nToks)
//
//  Input:   tok = an array of string tokens
//           n = number of tokens
//  Output:  returns error code
//  Purpose: adds a math expression to the control rule system from a
//           a tokenized line of input with format:
//    EXPRESSION  name = <math expression containing VARIABLE names>
//
{
    int i, k;
    char  s[MAXLINE + 1];
    MathExpr* expr;

    CurrentExpression++;
    if (nToks < 4) return ERR_ITEMS;
    k = CurrentExpression;
    Expression[k].expression = NULL;
    sstrncpy(Expression[k].name, tok[1], MAXVARNAME);
    sstrncpy(s, tok[3], MAXLINE);
    for (i = 4; i < nToks; i++)
    {
        sstrcat(s, " ", MAXLINE);
        sstrcat(s, tok[i], MAXLINE);
    }

    expr = mathexpr_create(s, getVariableIndex);
    if (expr == NULL)
        return error_setInpError(ERR_MATH_EXPR, "");

    Expression[k].expression = expr;
    return 0;
}

//=============================================================================

int  getVariableIndex(char* varName)
//
//  Input:   varName = string containing a variable name
//  Output:  returns the index of the named variable or -1 if not found
//  Purpose: finds the array index of a named variable.
//
{
    int i;
    for (i = 0; i < VariableCount; i++)
    {
        if (match(varName, NamedVariable[i].name)) return i;
    }
    return -1;
}

//=============================================================================

double getNamedVariableValue(int varIndex)
//
//  Input:   varIndex = index of a named variable
//  Output:  returns the current value of the variable
//  Purpose: finds the value of a named variable.
//
{
    return getVariableValue(NamedVariable[varIndex].variable);
}

//=============================================================================

int  getExpressionIndex(char* exprName)
//
//  Input:   exprName = string containing an expression name
//  Output:  returns the index of the expression or -1 if not found
//  Purpose: finds the array index of a math expression
//
{
    int i;
    for (i = 0; i < ExpressionCount; i++)
    {
        if (match(exprName, Expression[i].name)) return i;
    }
    return -1;
}

//=============================================================================

int  controls_addRuleClause(int r, int keyword, char* tok[], int nToks)
//
//  Input:   r = rule index
//           keyword = the clause's keyword code (IF, THEN, etc.)
//           tok = an array of string tokens that comprises the clause
//           nToks = number of tokens
//  Output:  returns an error  code
//  Purpose: addd a new clause to a control rule.
//
{
    switch (keyword)
    {
      case r_RULE:
        if ( Rules[r].ID == NULL )
            Rules[r].ID = project_findID(CONTROL, tok[1]);
        InputState = r_RULE;
        if ( nToks > 2 ) return ERR_RULE;
        return 0;

      case r_IF:
        if ( InputState != r_RULE ) return ERR_RULE;
        InputState = r_IF;
        return addPremise(r, r_AND, tok, nToks);

      case r_AND:
        if ( InputState == r_IF ) return addPremise(r, r_AND, tok, nToks);
        else if ( InputState == r_THEN || InputState == r_ELSE )
            return addAction(r, tok, nToks);
        else return ERR_RULE;

      case r_OR:
        if ( InputState != r_IF ) return ERR_RULE;
        return addPremise(r, r_OR, tok, nToks);

      case r_THEN:
        if ( InputState != r_IF ) return ERR_RULE;
        InputState = r_THEN;
        return addAction(r, tok, nToks);

      case r_ELSE:
        if ( InputState != r_THEN ) return ERR_RULE;
        InputState = r_ELSE;
        return addAction(r, tok, nToks);

      case r_PRIORITY:
        if ( InputState != r_THEN && InputState != r_ELSE ) return ERR_RULE;
        InputState = r_PRIORITY;
        if ( !getDouble(tok[1], &Rules[r].priority) ) return ERR_NUMBER;
        if ( nToks > 2 ) return ERR_RULE;
        return 0;
    }
    return 0;
}

//=============================================================================

int controls_evaluate(DateTime currentTime, DateTime elapsedTime, double tStep)
//
//  Input:   currentTime = current simulation date/time
//           elapsedTime = decimal days since start of simulation
//           tStep = simulation time step (days)
//  Output:  returns number of new actions taken
//  Purpose: evaluates all control rules at current time of the simulation.
//
{
    int    r;                          // control rule index
    int    result;                     // TRUE if rule premises satisfied
    struct TPremise* p;                // pointer to rule premise clause
    struct TAction*  a;                // pointer to rule action clause

    // --- save date and time to shared variables
    CurrentDate = floor(currentTime);
    CurrentTime = currentTime - floor(currentTime);
    ElapsedTime = elapsedTime;

    // --- evaluate each rule
    if ( RuleCount == 0 ) return 0;
    clearActionList();
    for (r=0; r<RuleCount; r++)
    {
        // --- evaluate rule's premises
        result = TRUE;
        p = Rules[r].firstPremise;
        while (p)
        {
            if ( p->type == r_OR )
            {
                if ( result == FALSE )
                    result = evaluatePremise(p, tStep);
            }
            else
            {
                if ( result == FALSE ) break;
                result = evaluatePremise(p, tStep);
            }
            p = p->next;
        }    

        // --- if premises true, add THEN clauses to action list
        //     else add ELSE clauses to action list
        if ( result == TRUE ) a = Rules[r].thenActions;
        else                  a = Rules[r].elseActions;
        while (a)
        {
            updateActionValue(a, currentTime, tStep);
            updateActionList(a);
            a = a->next;
        }
    }

    // --- execute actions on action list
    if ( ActionList ) return executeActionList(currentTime);
    else return 0;
}

//=============================================================================

int  addPremise(int r, int type, char* tok[], int nToks)
//
//  Input:   r = control rule index
//           type = type of premise (IF, AND, OR)
//           tok = array of string tokens containing premise statement
//           nToks = number of string tokens
//  Output:  returns an error code
//  Purpose: adds a new premise to a control rule.
//
{
    int    relation, n, err = 0;
    double value = MISSING;
    struct TPremise* p;
    struct TVariable v1;
    struct TVariable v2;
    int    obj, exprIndex, varIndex = -1;

    // --- initialize LHS variable v1
    if (nToks < 4) return ERR_ITEMS;
    v1.attribute = -1;
    v1.object = -1;
    v1.index = -1;
    n = 1;

    // --- check if 2nd token is a math expression
    exprIndex = getExpressionIndex(tok[1]);

    // --- if not then check if it's a named variable
    if (exprIndex < 0)
    {
        varIndex = getVariableIndex(tok[n]);
        if (varIndex >= 0)
        {
            v1 = NamedVariable[varIndex].variable;
        }

        // otherwise parse object|index|attribute tokens
        else
        {
            err = getPremiseVariable(tok, nToks, &n, &v1);
            if ( err > 0 ) return err;
        }
    }

    // --- get relational operator
    n++;
    if ( n >= nToks ) return error_setInpError(ERR_ITEMS, "");
    relation = findExactMatch(tok[n], RelOpWords);
    if ( relation < 0 ) return error_setInpError(ERR_KEYWORD, tok[n]);

    // --- initialize RHS variable v2
    v2.attribute = -1;
    v2.object = -1;
    v2.index = -1;
    n++;
    if (n >= nToks) return error_setInpError(ERR_ITEMS, "");

    // --- check for named RHS variable
    varIndex = getVariableIndex(tok[n]);
    if (varIndex >= 0)
    {
        v2 = NamedVariable[varIndex].variable;
    }

    // --- check for object|index|attribute variable
    else
    {
        obj = findmatch(tok[n], ObjectWords);
        if (obj >= 0)
        {
            err = getPremiseVariable(tok, nToks, &n, &v2);
            if ( err > 0 ) return ERR_RULE;
            if (exprIndex < 0 && v1.attribute != v2.attribute)
                report_writeWarningMsg(WARN11, Rules[r].ID);
        }

        // --- check for a single RHS value
        else
        {
            err = getPremiseValue(tok[n], v1.attribute, &value);
            if ( err > 0 ) return err;
        }
    }

    // --- make sure another clause is not on same line
    n++;
    if ( n < nToks && findmatch(tok[n], RuleKeyWords) >= 0 ) return ERR_RULE;

    // --- create the premise object
    p = (struct TPremise *) malloc(sizeof(struct TPremise));
    if ( !p ) return ERR_MEMORY;
    p->type      = type;
    p->exprIndex = exprIndex;
    p->lhsVar    = v1;
    p->rhsVar    = v2;
    p->relation  = relation;
    p->value     = value;
    p->next      = NULL;
    if ( Rules[r].firstPremise == NULL )
    {
        Rules[r].firstPremise = p;
    }
    else
    {
        Rules[r].lastPremise->next = p;
    }
    Rules[r].lastPremise = p;
    return 0;
}

//=============================================================================

int getPremiseVariable(char* tok[], int nToks, int* k, struct TVariable* v)
//
//  Input:   tok = array of string tokens
//           nToks = number of tokens
//           k = index of current token
//  Output:  returns an error code; updates k to new current token and
//           places identity of specified variable in v
//  Purpose: parses a variable (e.g., Node 123 Depth) used in a control rule.
//
{
    int    n = *k;
    int    object = -1;
    int    index = -1;
    int    obj, attrib;

    // --- get object type
    obj = findmatch(tok[n], ObjectWords);
    if ( obj < 0 ) return error_setInpError(ERR_KEYWORD, tok[n]);

    // --- get object index from its name
    n++;
    if (n >= nToks) return error_setInpError(ERR_ITEMS, "");
    switch (obj)
    {
      case r_GAGE:
        index = project_findObject(GAGE, tok[n]);
        if (index < 0) return error_setInpError(ERR_NAME, tok[n]);
        object = r_GAGE;
        break;

      case r_NODE:
        index = project_findObject(NODE, tok[n]);
        if ( index < 0 ) return error_setInpError(ERR_NAME, tok[n]);
        object = r_NODE;
        break;

      case r_LINK:
      case r_CONDUIT:
      case r_PUMP:
      case r_ORIFICE:
      case r_WEIR:
      case r_OUTLET:
        index = project_findObject(LINK, tok[n]);
        if ( index < 0 ) return error_setInpError(ERR_NAME, tok[n]);
        object = r_LINK;
        break;
      default: n--;
    }
    n++;
    if (n >= nToks) return error_setInpError(ERR_ITEMS, "");

    // --- get attribute index from its name
    if (object == r_GAGE)
        attrib = getGageAttrib(tok[n]);
    else
        attrib = findmatch(tok[n], AttribWords);
    if ( attrib < 0 ) return error_setInpError(ERR_KEYWORD, tok[n]);

    // --- check that attribute belongs to object type
    if (obj == r_GAGE)
    {

    }

    else if ( obj == r_NODE ) switch (attrib)
    {
      case r_DEPTH:
      case r_MAXDEPTH:
      case r_HEAD:
      case r_VOLUME:
      case r_INFLOW: break;
      default: return error_setInpError(ERR_KEYWORD, tok[n]);
    }

    // --- check for link TIMEOPEN & TIMECLOSED attributes
    else if ( object == r_LINK && index >= 0 &&
            ( (attrib == r_TIMEOPEN || attrib == r_TIMECLOSED)
            ))
    {
    // nothing to do here
    }

    else if ( obj == r_LINK || obj == r_CONDUIT ) switch (attrib)
    {
      case r_STATUS:
      case r_DEPTH:
      case r_FULLFLOW:
      case r_FULLDEPTH:
      case r_FLOW:
      case r_LENGTH:
      case r_SLOPE:
      case r_VELOCITY: break;
      default: return error_setInpError(ERR_KEYWORD, tok[n]);
    }
    else if ( obj == r_PUMP ) switch (attrib)
    {
      case r_FLOW:
      case r_SETTING:
      case r_STATUS: break;
      default: return error_setInpError(ERR_KEYWORD, tok[n]);
    }
    else if ( obj == r_ORIFICE || obj == r_WEIR ||
              obj == r_OUTLET ) switch (attrib)
    {
      case r_FLOW:
      case r_SETTING: break;
      default: return error_setInpError(ERR_KEYWORD, tok[n]);
    }
    else switch (attrib)
    {
      case r_TIME:
      case r_DATE:
      case r_CLOCKTIME:
      case r_DAY:
      case r_MONTH:
      case r_DAYOFYEAR: break;
      default: return error_setInpError(ERR_KEYWORD, tok[n]);
    }

    // --- populate variable structure
    v->object    = object;
    v->index     = index;
    v->attribute = attrib;
    *k = n;
    return 0;
}

//=============================================================================

int  getGageAttrib(char* token)
//
//  Input:   token = a string token
//  Output:  returns an attribute code or -1 if an error occurred
//  Purpose: determines the atrribute code for a rain gage variable.
//  Note:    a valid token is INTENSITY for current rainfall intensity
//           (attribute code = 0) or nHR_PRECIP for total rain depth
//           over past n hours (attribute code = n).
//
{
    int attrib;

    // --- check if token is currrent rainfall intensity
    if (match(token, IntensityWord))
        return 0;

    // --- token is past rain depth - read number of past hours
    attrib = atoi(token);

    // --- check that number of hours is in allowable range
    if (attrib < 1 || attrib > MAXPASTRAIN)
        return -1;
    return attrib;
}

//=============================================================================

int getPremiseValue(char* token, int attrib, double* value)
//
//  Input:   token = a string token
//           attrib = index of a node/link attribute
//  Output:  value = attribute value;
//           returns an error code;
//  Purpose: parses the numerical value of a particular node/link attribute
//           in the premise clause of a control rule.
//
{
    char   strDate[25]; 
    switch (attrib)
    {
      case r_STATUS:
        *value = findmatch(token, StatusWords);
        if ( *value < 0.0 ) *value = findmatch(token, ConduitWords);
        if ( *value < 0.0 ) return error_setInpError(ERR_KEYWORD, token);
        break;

      case r_TIME:
      case r_CLOCKTIME:
      case r_TIMEOPEN:
      case r_TIMECLOSED:
        if ( !datetime_strToTime(token, value) )
            return error_setInpError(ERR_DATETIME, token);
        break;

      case r_DATE:
        if ( !datetime_strToDate(token, value) )
            return error_setInpError(ERR_DATETIME, token);
        break;

      case r_DAY:
        if ( !getDouble(token, value) ) 
            return error_setInpError(ERR_NUMBER, token);
        if ( *value < 1.0 || *value > 7.0 )
             return error_setInpError(ERR_DATETIME, token);
        break;

      case r_MONTH:
        if ( !getDouble(token, value) )
            return error_setInpError(ERR_NUMBER, token);
        if ( *value < 1.0 || *value > 12.0 )
             return error_setInpError(ERR_DATETIME, token);
        break;

      case r_DAYOFYEAR:
        sstrncpy(strDate, token, 6);
        sstrcat(strDate, "/1947", 25);
        if ( datetime_strToDate(strDate, value) )
        {
            *value = datetime_dayOfYear(*value);
        }
        else if ( !getDouble(token, value) || *value < 1 || *value > 365 )
            return error_setInpError(ERR_DATETIME, token);
        break;
       
      default: if ( !getDouble(token, value) )
          return error_setInpError(ERR_NUMBER, token);
    }
    return 0;
}

//=============================================================================

int  addAction(int r, char* tok[], int nToks)
//
//  Input:   r = control rule index
//           tok = array of string tokens containing action statement
//           nToks = number of string tokens
//  Output:  returns an error code
//  Purpose: adds a new action to a control rule.
//
{
    int    obj, link, attrib;
    int    curve = -1, tseries = -1;
    int    n;
    int    err;
    double values[] = {1.0, 0.0, 0.0};

    struct TAction* a;

    // --- check for proper number of tokens
    if ( nToks < 6 ) return error_setInpError(ERR_ITEMS, "");

    // --- check for valid object type
    obj = findmatch(tok[1], ObjectWords);
    if ( obj != r_LINK && obj != r_CONDUIT && obj != r_PUMP && 
         obj != r_ORIFICE && obj != r_WEIR && obj != r_OUTLET )
        return error_setInpError(ERR_KEYWORD, tok[1]);

    // --- check that object name exists and is of correct type
    link = project_findObject(LINK, tok[2]);
    if ( link < 0 ) return error_setInpError(ERR_NAME, tok[2]);
    switch (obj)
    {
      case r_CONDUIT:
        if ( Link[link].type != CONDUIT )
            return error_setInpError(ERR_NAME, tok[2]);
        break;
      case r_PUMP:
        if ( Link[link].type != PUMP )
            return error_setInpError(ERR_NAME, tok[2]);
        break;
      case r_ORIFICE:
        if ( Link[link].type != ORIFICE )
            return error_setInpError(ERR_NAME, tok[2]);
        break;
      case r_WEIR:
        if ( Link[link].type != WEIR )
            return error_setInpError(ERR_NAME, tok[2]);
        break;
      case r_OUTLET:
        if ( Link[link].type != OUTLET )
            return error_setInpError(ERR_NAME, tok[2]);
        break;
    }

    // --- check for valid attribute name
    attrib = findmatch(tok[3], AttribWords);
    if ( attrib < 0 ) return error_setInpError(ERR_KEYWORD, tok[3]);

    // --- get control action setting
    if ( obj == r_CONDUIT )
    {
        if ( attrib == r_STATUS )
        {
            values[0] = findmatch(tok[5], ConduitWords);
            if ( values[0] < 0.0 )
                return error_setInpError(ERR_KEYWORD, tok[5]);
        }
        else return error_setInpError(ERR_KEYWORD, tok[3]);
    }

    else if ( obj == r_PUMP )
    {
        if ( attrib == r_STATUS )
        {
            values[0] = findmatch(tok[5], StatusWords);
            if ( values[0] < 0.0 )
                return error_setInpError(ERR_KEYWORD, tok[5]);
        }
        else if ( attrib == r_SETTING )
        {
            err = setActionSetting(tok, nToks, &curve, &tseries,
                                   &attrib, values);
            if ( err > 0 ) return err;
        }
        else return error_setInpError(ERR_KEYWORD, tok[3]);
    }

    else if ( obj == r_ORIFICE || obj == r_WEIR || obj == r_OUTLET )
    {
        if ( attrib == r_SETTING )
        {
           err = setActionSetting(tok, nToks, &curve, &tseries,
                                  &attrib, values);
           if ( err > 0 ) return err;
           if (  attrib == r_SETTING
           && (values[0] < 0.0 || values[0] > 1.0) ) 
               return error_setInpError(ERR_NUMBER, tok[5]);
        }
        else return error_setInpError(ERR_KEYWORD, tok[3]);
    }
    else return error_setInpError(ERR_KEYWORD, tok[1]);

    // --- check if another clause is on same line
    n = 6;
    if ( curve >= 0 || tseries >= 0 ) n = 7;
    if ( attrib == r_PID ) n = 9;
    if ( n < nToks && findmatch(tok[n], RuleKeyWords) >= 0 ) return ERR_RULE;

    // --- create the action object
    a = (struct TAction *) malloc(sizeof(struct TAction));
    if ( !a ) return ERR_MEMORY;
    a->rule      = r;
    a->link      = link;
    a->attribute = attrib;
    a->curve     = curve;
    a->tseries   = tseries;
    a->value     = values[0];
    if ( attrib == r_PID )
    {
        a->kp = values[0];
        a->ki = values[1];
        a->kd = values[2];
        a->e1 = 0.0;
        a->e2 = 0.0;
    }
    if ( InputState == r_THEN )
    {
        a->next = Rules[r].thenActions;
        Rules[r].thenActions = a;
    }
    else
    {
        a->next = Rules[r].elseActions;
        Rules[r].elseActions = a;
    }
    return 0;
}

//=============================================================================

int  setActionSetting(char* tok[], int nToks, int* curve, int* tseries,
                      int* attrib, double values[])
//
//  Input:   tok = array of string tokens containing action statement
//           nToks = number of string tokens
//  Output:  curve = index of controller curve
//           tseries = index of controller time series
//           attrib = r_PID if PID controller used
//           values = values of control settings
//           returns an error code
//  Purpose: identifies how control actions settings are determined.
//
{
    int k, m;

    // --- see if control action is determined by a Curve or Time Series
    if (nToks < 6) return error_setInpError(ERR_ITEMS, "");
    k = findmatch(tok[5], SettingTypeWords);
    if ( k >= 0 && nToks < 7 ) return error_setInpError(ERR_ITEMS, "");
    switch (k)
    {

    // --- control determined by a curve - find curve index
    case r_CURVE:
        m = project_findObject(CURVE, tok[6]);
        if ( m < 0 ) return error_setInpError(ERR_NAME, tok[6]);
        *curve = m;
        break;

    // --- control determined by a time series - find time series index
    case r_TIMESERIES:
        m = project_findObject(TSERIES, tok[6]);
        if ( m < 0 ) return error_setInpError(ERR_NAME, tok[6]);
        *tseries = m;
        Tseries[m].refersTo = CONTROL;
        break;

    // --- control determined by PID controller 
    case r_PID:
        if (nToks < 9) return error_setInpError(ERR_ITEMS, "");
        for (m=6; m<=8; m++)
        {
            if ( !getDouble(tok[m], &values[m-6]) )
                return error_setInpError(ERR_NUMBER, tok[m]);
        }
        *attrib = r_PID;
        break;

    // --- direct numerical control is used
    default:
        if ( !getDouble(tok[5], &values[0]) )
            return error_setInpError(ERR_NUMBER, tok[5]);
    }
    return 0;
}

//=============================================================================

void  updateActionValue(struct TAction* a, DateTime currentTime, double dt)
//
//  Input:   a = an action object
//           currentTime = current simulation date/time (days)
//           dt = time step (days)
//  Output:  none
//  Purpose: updates value of actions found from Curves or Time Series.
//
{
    if ( a->curve >= 0 )
    {
        a->value = table_lookup(&Curve[a->curve], ControlValue);
    }
    else if ( a->tseries >= 0 )
    {
        a->value = table_tseriesLookup(&Tseries[a->tseries], currentTime, TRUE);
    }
    else if ( a->attribute == r_PID )
    {
        a->value = getPIDSetting(a, dt);
    }
}

//=============================================================================

double getPIDSetting(struct TAction* a, double dt)
//
//  Input:   a = an action object
//           dt = current time step (days)
//  Output:  returns a new link setting 
//  Purpose: computes a new setting for a link subject to a PID controller.
//
//  Note:    a->kp = gain coefficient,
//           a->ki = integral time (minutes)
//           a->k2 = derivative time (minutes)
//           a->e1 = error from previous time step
//           a->e2 = error from two time steps ago
{
    double e0, setting;
    double p, i, d, update;
    double tolerance = 0.0001;

    // --- convert time step from days to minutes
    dt *= 1440.0;

    // --- determine relative error in achieving controller set point
    e0 = SetPoint - ControlValue;
    if ( fabs(e0) > TINY )
    {
        if ( SetPoint != 0.0 ) e0 = e0/SetPoint;
        else                   e0 = e0/ControlValue;
    }

    // --- reset previous errors to 0 if controller gets stuck
    if (fabs(e0 - a->e1) < tolerance)
    {
        a->e2 = 0.0;
        a->e1 = 0.0;
    }

    // --- use the recursive form of the PID controller equation to
    //     determine the new setting for the controlled link
    p = (e0 - a->e1);
    if ( a->ki == 0.0 ) i = 0.0;
    else i = e0 * dt / a->ki;
    d = a->kd * (e0 - 2.0*a->e1 + a->e2) / dt;
    update = a->kp * (p + i + d);
    if ( fabs(update) < tolerance ) update = 0.0;
    setting = Link[a->link].targetSetting + update;

    // --- update previous errors
    a->e2 = a->e1;
    a->e1 = e0;

    // --- check that new setting lies within feasible limits
    if ( setting < 0.0 ) setting = 0.0;
    if (Link[a->link].type != PUMP && setting > 1.0 ) setting = 1.0;
    return setting;
}

//=============================================================================

void updateActionList(struct TAction* a)
//
//  Input:   a = an action object
//  Output:  none
//  Purpose: adds a new action to the list of actions to be taken.
//
{
    struct TActionList* listItem;
    struct TAction* a1;
    double priority = Rules[a->rule].priority;

    // --- check if link referred to in action is already listed
    listItem = ActionList;
    while ( listItem )
    {
        a1 = listItem->action;
        if ( !a1 ) break;
        if ( a1->link == a->link )
        {
            // --- replace old action if new action has higher priority
            if ( priority > Rules[a1->rule].priority ) listItem->action = a;
            return;
        }
        listItem = listItem->next;
    }

    // --- action not listed so add it to ActionList                           //5.2.1
    if ( !listItem )
    {
        listItem = (struct TActionList *) malloc(sizeof(struct TActionList));
        listItem->next = ActionList;
        ActionList = listItem;
    }
    listItem->action = a;
}

//=============================================================================

int executeActionList(DateTime currentTime)
//
//  Input:   currentTime = current date/time of the simulation
//  Output:  returns number of new actions taken
//  Purpose: executes all actions required by fired control rules.
//
{
    struct TActionList* listItem;
    struct TActionList* nextItem;
    struct TAction* a1;
    int count = 0;

    listItem = ActionList;
    while ( listItem )
    {
        a1 = listItem->action;
        if ( !a1 ) break;
        if ( a1->link >= 0 )
        {
            if ( Link[a1->link].targetSetting != a1->value )
            {
                Link[a1->link].targetSetting = a1->value;
                if ( RptFlags.controls && a1->curve < 0 
                     && a1->tseries < 0 && a1->attribute != r_PID )
                    report_writeControlAction(currentTime, Link[a1->link].ID,
                                              a1->value, Rules[a1->rule].ID);
                count++;
            }
        }
        nextItem = listItem->next;
        listItem = nextItem;
    }
    return count;
}

//=============================================================================

int evaluatePremise(struct TPremise* p, double tStep)
//
//  Input:   p = a control rule premise condition
//           tStep = current time step (days)
//  Output:  returns TRUE if the condition is true or FALSE otherwise
//  Purpose: evaluates the truth of a control rule premise condition.
//
{
    double lhsValue, rhsValue;
    int    result = FALSE;

    // --- check if left hand side (lhs) of premise is an expression
    if (p->exprIndex >= 0)
        lhsValue = mathexpr_eval(Expression[p->exprIndex].expression,
            getNamedVariableValue);

    // --- otherwise get value of the lhs variable
    else
        lhsValue = getVariableValue(p->lhsVar);

    // --- if right hand side (rhs) of premise is a variable then get its value
    if ( p->value == MISSING ) rhsValue = getVariableValue(p->rhsVar);
    else                       rhsValue = p->value;
    if ( lhsValue == MISSING || rhsValue == MISSING ) return FALSE;

    // --- compare the lhs of the premise to the rhs
    switch (p->lhsVar.attribute)
    {
    case r_TIME:
    case r_CLOCKTIME:
        return compareTimes(lhsValue, p->relation, rhsValue, tStep/2.0); 
    case r_TIMEOPEN:
    case r_TIMECLOSED:
        result = compareTimes(lhsValue, p->relation, rhsValue, tStep/2.0);
        ControlValue = lhsValue * 24.0;  // convert time from days to hours
        return result;
    default:
        return compareValues(lhsValue, p->relation, rhsValue);
    }
}

//=============================================================================

double getVariableValue(struct TVariable v)
{
    int i = -1;    // a node index
    int j = -1;    // a link index

    if (v.object == r_GAGE)
        return getRainValue(v);
    if (v.object == r_NODE) i = v.index;
    if (v.object == r_LINK) j = v.index;

    switch ( v.attribute )
    {
      case r_TIME:
        return ElapsedTime;
        
      case r_DATE:
        return CurrentDate;

      case r_CLOCKTIME:
        return CurrentTime;

      case r_DAY:
        return datetime_dayOfWeek(CurrentDate);

      case r_MONTH:
        return datetime_monthOfYear(CurrentDate);

      case r_DAYOFYEAR:
        return datetime_dayOfYear(CurrentDate);

      case r_STATUS:
        if ( j < 0 ||
            (Link[j].type != CONDUIT && Link[j].type != PUMP) ) return MISSING;
        else return Link[j].setting;
        
      case r_SETTING:
        if ( j < 0 || (Link[j].type != PUMP &&
                       Link[j].type != ORIFICE &&
                       Link[j].type != WEIR) )
            return MISSING;
        else return Link[j].setting;

      case r_FLOW:
        if ( j < 0 ) return MISSING;
        else return Link[j].direction*Link[j].newFlow*UCF(FLOW);

      case r_FULLFLOW:
      case r_FULLDEPTH:
      case r_VELOCITY:
      case r_LENGTH:
      case r_SLOPE:
        if ( j < 0 ) return MISSING;
        else if (Link[j].type != CONDUIT) return MISSING;
        switch (v.attribute)
        {
          case r_FULLFLOW:  return Link[j].qFull * UCF(FLOW);
          case r_FULLDEPTH: return Link[j].xsect.yFull * UCF(LENGTH);
          case r_VELOCITY:
            return link_getVelocity(j, Link[j].newFlow, Link[j].newDepth)
                * UCF(LENGTH);
          case r_LENGTH: return Conduit[Link[j].subIndex].length * UCF(LENGTH);
          case r_SLOPE:  return Conduit[Link[j].subIndex].slope;
          default: return MISSING;
        }
      case r_DEPTH:
        if ( j >= 0 ) return Link[j].newDepth*UCF(LENGTH);
        else if ( i >= 0 )
            return Node[i].newDepth*UCF(LENGTH);
        else return MISSING;

      case r_MAXDEPTH:
          if (i >= 0) return Node[i].fullDepth*UCF(LENGTH);
          else return MISSING;

      case r_HEAD:
        if ( i < 0 ) return MISSING;
        return (Node[i].newDepth + Node[i].invertElev) * UCF(LENGTH);

      case r_VOLUME:
        if ( i < 0 ) return MISSING;
        return (Node[i].newVolume * UCF(VOLUME));

      case r_INFLOW:
        if ( i < 0 ) return MISSING;
        else return Node[i].newLatFlow*UCF(FLOW);

      case r_TIMEOPEN:
          if ( j < 0 ) return MISSING;
          if ( Link[j].setting <= 0.0 ) return MISSING;
          return CurrentDate + CurrentTime - Link[j].timeLastSet;

      case r_TIMECLOSED:
          if ( j < 0 ) return MISSING;
          if ( Link[j].setting > 0.0 ) return MISSING;
          return CurrentDate + CurrentTime - Link[j].timeLastSet;

      default: return MISSING;
    }
}

//=============================================================================

double getRainValue(struct TVariable v)
//
//  Input:   v = a rule premise variable for a rain gage
//  Output:  returns current or past rainfall amount
//  Purpose: retrieves either the current rainfall intensity or the past
//           rainfall total for a rain gage.
//
{
    if (v.index < 0) return MISSING;
    else if (Gage[v.index].isUsed == FALSE) return 0.0;
    else if (v.attribute == 0)
        return Gage[v.index].rainfall;
    else return gage_getPastRain(v.index, v.attribute);
}

//=============================================================================

int compareTimes(double lhsValue, int relation, double rhsValue, double halfStep)
//
//  Input:   lhsValue = date/time value on left hand side of relation
//           relation = relational operator code (see RuleRelation enumeration)
//           rhsValue = date/time value on right hand side of relation 
//           halfStep = 1/2 the current time step (days)
//  Output:  returns TRUE if time relation is satisfied
//  Purpose: evaluates the truth of a relation between two date/times.
//
{
    if ( relation == EQ )
    {
        if ( lhsValue >= rhsValue - halfStep
        &&   lhsValue < rhsValue + halfStep ) return TRUE;
        return FALSE;
    }
    else if ( relation == NE )
    {
        if ( lhsValue < rhsValue - halfStep
        ||   lhsValue >= rhsValue + halfStep ) return TRUE;
        return FALSE;
    }
    else return compareValues(lhsValue, relation, rhsValue);
}

//=============================================================================

int compareValues(double lhsValue, int relation, double rhsValue)
//  Input:   lhsValue = value on left hand side of relation
//           relation = relational operator code (see RuleRelation enumeration)
//           rhsValue = value on right hand side of relation 
//  Output:  returns TRUE if relation is satisfied
//  Purpose: evaluates the truth of a relation between two values.
{
    SetPoint = rhsValue;
    ControlValue = lhsValue;
    switch (relation)
    {
      case EQ: if ( lhsValue == rhsValue ) return TRUE; break;
      case NE: if ( lhsValue != rhsValue ) return TRUE; break;
      case LT: if ( lhsValue <  rhsValue ) return TRUE; break;
      case LE: if ( lhsValue <= rhsValue ) return TRUE; break;
      case GT: if ( lhsValue >  rhsValue ) return TRUE; break;
      case GE: if ( lhsValue >= rhsValue ) return TRUE; break;
    }
    return FALSE;
}

//=============================================================================

void clearActionList(void)
//
//  Input:   none
//  Output:  none
//  Purpose: clears the list of actions to be executed.
//
{
    struct TActionList* listItem;
    listItem = ActionList;
    while ( listItem )
    {
        listItem->action = NULL;
        listItem = listItem->next;
    }
}

//=============================================================================

void  deleteActionList(void)
//
//  Input:   none
//  Output:  none
//  Purpose: frees the memory used to hold the list of actions to be executed.
//
{
    struct TActionList* listItem;
    struct TActionList* nextItem;
    listItem = ActionList;
    while ( listItem )
    {
        nextItem = listItem->next;
        free(listItem);
        listItem = nextItem;
    }
    ActionList = NULL;
}

//=============================================================================

void  deleteRules(void)
//
//  Input:   none
//  Output:  none
//  Purpose: frees the memory used for all of the control rules.
//
{
   struct TPremise* p;
   struct TPremise* pnext;
   struct TAction*  a;
   struct TAction*  anext;
   int r;
   for (r=0; r<RuleCount; r++)
   {
      p = Rules[r].firstPremise;
      while ( p )
      {
         pnext = p->next;
         free(p);
         p = pnext;
      }
      a = Rules[r].thenActions;
      while (a )
      {
         anext = a->next;
         free(a);
         a = anext;
      }
      a = Rules[r].elseActions;
      while (a )
      {
         anext = a->next;
         free(a);
         a = anext;
      }
   }
   FREE(Rules);
   RuleCount = 0;
}

//=============================================================================

int  findExactMatch(char *s, char *keyword[])
//
//  Input:   s = character string
//           keyword = array of keyword strings
//  Output:  returns index of keyword which matches s or -1 if no match found  
//  Purpose: finds exact match between string and array of keyword strings.
//
{
   int i = 0;
   while (keyword[i] != NULL)
   {
      if ( strcomp(s, keyword[i]) ) return(i);
      i++;
   }
   return(-1);
}

//=============================================================================
