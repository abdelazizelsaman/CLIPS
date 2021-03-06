   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*             CLIPS Version 6.24  06/05/06            */
   /*                                                     */
   /*          DEFRULE BASIC COMMANDS HEADER FILE         */
   /*******************************************************/

/*************************************************************/
/* Purpose: Implements core commands for the defrule         */
/*   construct such as clear, reset, save, undefrule,        */
/*   ppdefrule, list-defrules, and                           */
/*   get-defrule-list.                                       */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*      Brian L. Dantes                                      */
/*                                                           */
/* Revision History:                                         */
/*      6.23: Corrected compilation errors for files         */
/*            generated by constructs-to-c. DR0861           */
/*                                                           */
/*            Changed name of variable log to logName        */
/*            because of Unix compiler warnings of shadowed  */
/*            definitions.                                   */
/*                                                           */
/*      6.24: Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*************************************************************/

#define _RULEBSC_SOURCE_

#include "setup.h"

#if DEFRULE_CONSTRUCT

#include <stdio.h>
#define _STDIO_INCLUDED_

#include "argacces.h"
#include "constrct.h"
#include "envrnmnt.h"
#include "router.h"
#include "watch.h"
#include "extnfunc.h"
#include "ruledef.h"
#include "engine.h"
#include "drive.h"
#include "reteutil.h"
#if BLOAD || BLOAD_ONLY || BLOAD_AND_BSAVE
#include "rulebin.h"
#endif
#if CONSTRUCT_COMPILER && (! RUN_TIME)
#include "rulecmp.h"
#endif

#include "rulebsc.h"

/***************************************/
/* LOCAL INTERNAL FUNCTION DEFINITIONS */
/***************************************/

   static void                    ResetDefrules(void *,EXEC_STATUS);
   static void                    ResetDefrulesPrime(void *,EXEC_STATUS);
   static void                    SaveDefrules(void *,EXEC_STATUS,void *,char *);
#if (! RUN_TIME)
   static int                     ClearDefrulesReady(void *,EXEC_STATUS);
   static void                    ClearDefrules(void *,EXEC_STATUS);
#endif

/*************************************************************/
/* DefruleBasicCommands: Initializes basic defrule commands. */
/*************************************************************/
globle void DefruleBasicCommands(
  void *theEnv,
  EXEC_STATUS)
  {
   EnvAddResetFunction(theEnv,execStatus,"defrule",ResetDefrules,70);
   EnvAddResetFunction(theEnv,execStatus,"defrule",ResetDefrulesPrime,10);
   AddSaveFunction(theEnv,execStatus,"defrule",SaveDefrules,0);
#if (! RUN_TIME)
   AddClearReadyFunction(theEnv,execStatus,"defrule",ClearDefrulesReady,0);
   EnvAddClearFunction(theEnv,execStatus,"defrule",ClearDefrules,0);
#endif
   
#if DEBUGGING_FUNCTIONS
   AddWatchItem(theEnv,execStatus,"rules",0,&DefruleData(theEnv,execStatus)->WatchRules,70,DefruleWatchAccess,DefruleWatchPrint);
#endif

#if ! RUN_TIME
   EnvDefineFunction2(theEnv,execStatus,"get-defrule-list",'m',PTIEF GetDefruleListFunction,"GetDefruleListFunction","01w");
   EnvDefineFunction2(theEnv,execStatus,"undefrule",'v',PTIEF UndefruleCommand,"UndefruleCommand","11w");
   EnvDefineFunction2(theEnv,execStatus,"defrule-module",'w',PTIEF DefruleModuleFunction,"DefruleModuleFunction","11w");

#if DEBUGGING_FUNCTIONS
   EnvDefineFunction2(theEnv,execStatus,"rules",'v', PTIEF ListDefrulesCommand,"ListDefrulesCommand","01w");
   EnvDefineFunction2(theEnv,execStatus,"list-defrules",'v', PTIEF ListDefrulesCommand,"ListDefrulesCommand","01w");
   EnvDefineFunction2(theEnv,execStatus,"ppdefrule",'v',PTIEF PPDefruleCommand,"PPDefruleCommand","11w");
#endif

#if (BLOAD || BLOAD_ONLY || BLOAD_AND_BSAVE)
   DefruleBinarySetup(theEnv,execStatus);
#endif

#if CONSTRUCT_COMPILER && (! RUN_TIME)
   DefruleCompilerSetup(theEnv,execStatus);
#endif

#endif
  }

/*****************************************************/
/* ResetDefrules: Defrule reset routine for use with */
/*   the reset command. Sets the current entity time */
/*   tag (used by the conflict resolution strategies */
/*   for recency) to zero. The focus stack is also   */
/*   cleared.                                        */
/*****************************************************/
static void ResetDefrules(
  void *theEnv,
  EXEC_STATUS)
  {
   struct defmodule *theModule;
   struct joinLink *theLink;
   struct partialMatch *notParent;
  
   DefruleData(theEnv,execStatus)->CurrentEntityTimeTag = 1L;
   EnvClearFocusStack(theEnv,execStatus);
   theModule = (struct defmodule *) EnvFindDefmodule(theEnv,execStatus,"MAIN");
   EnvFocus(theEnv,execStatus,(void *) theModule);
   
   for (theLink = DefruleData(theEnv,execStatus)->RightPrimeJoins;
        theLink != NULL;
        theLink = theLink->next)
     { PosEntryRetractAlpha(theEnv,execStatus,theLink->join->rightMemory->beta[0]); }

   for (theLink = DefruleData(theEnv,execStatus)->LeftPrimeJoins;
        theLink != NULL;
        theLink = theLink->next)
     { 
      if ((theLink->join->patternIsNegated || theLink->join->joinFromTheRight) && 
          (! theLink->join->patternIsExists))
        {
         notParent = theLink->join->leftMemory->beta[0];
         
         if (notParent->marker)
           { RemoveBlockedLink(notParent); }
           
         /*==========================================================*/
         /* Prevent any retractions from generating partial matches. */
         /*==========================================================*/
           
         notParent->marker = notParent;
         
         if (notParent->children != NULL)
           { PosEntryRetractBeta(theEnv,execStatus,notParent,notParent->children); }
           /*
         if (notParent->dependents != NULL) 
           { RemoveLogicalSupport(theEnv,execStatus,notParent); } */
        }
     }
  }

/*****************************************************/
/* ResetDefrulesPrime:                      */
/*****************************************************/
static void ResetDefrulesPrime(
  void *theEnv,
  EXEC_STATUS)
  {
   struct joinLink *theLink;
   struct partialMatch *notParent;
      
   for (theLink = DefruleData(theEnv,execStatus)->RightPrimeJoins;
        theLink != NULL;
        theLink = theLink->next)
     { NetworkAssert(theEnv,execStatus,theLink->join->rightMemory->beta[0],theLink->join); }

   for (theLink = DefruleData(theEnv,execStatus)->LeftPrimeJoins;
        theLink != NULL;
        theLink = theLink->next)
     { 
      if ((theLink->join->patternIsNegated || theLink->join->joinFromTheRight) && 
          (! theLink->join->patternIsExists))
        {
         notParent = theLink->join->leftMemory->beta[0];

         if (theLink->join->secondaryNetworkTest != NULL)
           {
            if (EvaluateSecondaryNetworkTest(theEnv,execStatus,notParent,theLink->join) == FALSE)
              { continue; }
           }

         notParent->marker = NULL;

         EPMDrive(theEnv,execStatus,notParent,theLink->join);
        }
     }

  }
#if (! RUN_TIME)

/******************************************************************/
/* ClearDefrulesReady: Indicates whether defrules can be cleared. */
/******************************************************************/
static int ClearDefrulesReady(
  void *theEnv,
  EXEC_STATUS)
  {
   if (EngineData(theEnv,execStatus)->ExecutingRule != NULL) return(FALSE);
   
   EnvClearFocusStack(theEnv,execStatus);
   if (EnvGetCurrentModule(theEnv,execStatus) == NULL) return(FALSE);

   DefruleData(theEnv,execStatus)->CurrentEntityTimeTag = 1L;

   return(TRUE);
  }

/***************************************************************/
/* ClearDefrules: Pushes the MAIN module as the current focus. */
/***************************************************************/
static void ClearDefrules(
  void *theEnv,
  EXEC_STATUS)
  {
   struct defmodule *theModule;

   theModule = (struct defmodule *) EnvFindDefmodule(theEnv,execStatus,"MAIN");
   EnvFocus(theEnv,execStatus,(void *) theModule);
  }
#endif

/**************************************/
/* SaveDefrules: Defrule save routine */
/*   for use with the save command.   */
/**************************************/
static void SaveDefrules(
  void *theEnv,
  EXEC_STATUS,
  void *theModule,
  char *logicalName)
  {
   SaveConstruct(theEnv,execStatus,theModule,logicalName,DefruleData(theEnv,execStatus)->DefruleConstruct); 
  }

/******************************************/
/* UndefruleCommand: H/L access routine   */
/*   for the undefrule command.           */
/******************************************/
globle void UndefruleCommand(
  void *theEnv,
  EXEC_STATUS)
  { 
   UndefconstructCommand(theEnv,execStatus,"undefrule",DefruleData(theEnv,execStatus)->DefruleConstruct); 
  }

/**********************************/
/* EnvUndefrule: C access routine */
/*   for the undefrule command.   */
/**********************************/
globle intBool EnvUndefrule(
  void *theEnv,
  EXEC_STATUS,
  void *theDefrule)
  {
   return(Undefconstruct(theEnv,execStatus,theDefrule,DefruleData(theEnv,execStatus)->DefruleConstruct)); 
  }

/************************************************/
/* GetDefruleListFunction: H/L access routine   */
/*   for the get-defrule-list function.         */
/************************************************/
globle void GetDefruleListFunction(
  void *theEnv,
  EXEC_STATUS,
  DATA_OBJECT_PTR returnValue)
  {
   GetConstructListFunction(theEnv,execStatus,"get-defrule-list",returnValue,DefruleData(theEnv,execStatus)->DefruleConstruct); 
  }

/****************************************/
/* EnvGetDefruleList: C access routine  */
/*   for the get-defrule-list function. */
/****************************************/
globle void EnvGetDefruleList(
  void *theEnv,
  EXEC_STATUS,
  DATA_OBJECT_PTR returnValue,
  void *theModule)
  {
   GetConstructList(theEnv,execStatus,returnValue,DefruleData(theEnv,execStatus)->DefruleConstruct,(struct defmodule *) theModule);
  }

/*********************************************/
/* DefruleModuleFunction: H/L access routine */
/*   for the defrule-module function.        */
/*********************************************/
globle void *DefruleModuleFunction(
  void *theEnv,
  EXEC_STATUS)
  {
   return(GetConstructModuleCommand(theEnv,execStatus,"defrule-module",DefruleData(theEnv,execStatus)->DefruleConstruct)); 
  }

#if DEBUGGING_FUNCTIONS

/******************************************/
/* PPDefruleCommand: H/L access routine   */
/*   for the ppdefrule command.           */
/******************************************/
globle void PPDefruleCommand(
  void *theEnv,
  EXEC_STATUS)
  {
   PPConstructCommand(theEnv,execStatus,"ppdefrule",DefruleData(theEnv,execStatus)->DefruleConstruct);
  }

/***********************************/
/* PPDefrule: C access routine for */
/*   the ppdefrule command.        */
/***********************************/
globle int PPDefrule(
  void *theEnv,
  EXEC_STATUS,
  char *defruleName,
  char *logicalName)
  {
   return(PPConstruct(theEnv,execStatus,defruleName,logicalName,DefruleData(theEnv,execStatus)->DefruleConstruct));
  }

/*********************************************/
/* ListDefrulesCommand: H/L access routine   */
/*   for the list-defrules command.          */
/*********************************************/
globle void ListDefrulesCommand(
  void *theEnv,
  EXEC_STATUS)
  {
   ListConstructCommand(theEnv,execStatus,"list-defrules",DefruleData(theEnv,execStatus)->DefruleConstruct); 
  }

/*************************************/
/* EnvListDefrules: C access routine */
/*   for the list-defrules command.  */
/*************************************/
globle void EnvListDefrules(
  void *theEnv,
  EXEC_STATUS,
  char *logicalName,
  void *theModule)
  {
   ListConstruct(theEnv,execStatus,DefruleData(theEnv,execStatus)->DefruleConstruct,logicalName,(struct defmodule *) theModule); 
  }

/*******************************************************/
/* EnvGetDefruleWatchActivations: C access routine for */
/*   retrieving the current watch value of a defrule's */
/*   activations.                                      */
/*******************************************************/
#if WIN_BTC
#pragma argsused
#endif
globle unsigned EnvGetDefruleWatchActivations(
  void *theEnv,
  EXEC_STATUS,
  void *rulePtr)
  {
   struct defrule *thePtr;
#if MAC_MCW || WIN_MCW || MAC_XCD
#pragma unused(theEnv,execStatus)
#endif

   for (thePtr = (struct defrule *) rulePtr;
        thePtr != NULL;
        thePtr = thePtr->disjunct)
     { if (thePtr->watchActivation) return(TRUE); }

   return(FALSE);
  }

/***********************************************/
/* EnvGetDefruleWatchFirings: C access routine */
/*   for retrieving the current watch value of */
/*   a defrule's firings.                      */
/***********************************************/
#if WIN_BTC
#pragma argsused
#endif
globle unsigned EnvGetDefruleWatchFirings(
  void *theEnv,
  EXEC_STATUS,
  void *rulePtr)
  {
   struct defrule *thePtr;
#if MAC_MCW || WIN_MCW || MAC_XCD
#pragma unused(theEnv,execStatus)
#endif

   for (thePtr = (struct defrule *) rulePtr;
        thePtr != NULL;
        thePtr = thePtr->disjunct)
     { if (thePtr->watchFiring) return(TRUE); }

   return(FALSE);
  }

/***************************************************/
/* EnvSetDefruleWatchActivations: C access routine */
/*   for setting the current watch value of a      */
/*   defrule's activations.                        */
/***************************************************/
#if WIN_BTC
#pragma argsused
#endif
globle void EnvSetDefruleWatchActivations(
  void *theEnv,
  EXEC_STATUS,
  unsigned newState,
  void *rulePtr)
  {
   struct defrule *thePtr;
#if MAC_MCW || WIN_MCW || MAC_XCD
#pragma unused(theEnv,execStatus)
#endif

   for (thePtr = (struct defrule *) rulePtr;
        thePtr != NULL;
        thePtr = thePtr->disjunct)
     { thePtr->watchActivation = newState; }
  }

/****************************************************/
/* EnvSetDefruleWatchFirings: C access routine for  */
/*   setting the current watch value of a defrule's */ 
/*   firings.                                       */
/****************************************************/
#if WIN_BTC
#pragma argsused
#endif
globle void EnvSetDefruleWatchFirings(
  void *theEnv,
  EXEC_STATUS,
  unsigned newState,
  void *rulePtr)
  {
   struct defrule *thePtr;
#if MAC_MCW || WIN_MCW || MAC_XCD
#pragma unused(theEnv,execStatus)
#endif

   for (thePtr = (struct defrule *) rulePtr;
        thePtr != NULL;
        thePtr = thePtr->disjunct)
     { thePtr->watchFiring = newState; }
  }

/*******************************************************************/
/* DefruleWatchAccess: Access function for setting the watch flags */
/*   associated with rules (activations and rule firings).         */
/*******************************************************************/
globle unsigned DefruleWatchAccess(
  void *theEnv,
  EXEC_STATUS,
  int code,
  unsigned newState,
  struct expr *argExprs)
  {
   if (code)
     return(ConstructSetWatchAccess(theEnv,execStatus,DefruleData(theEnv,execStatus)->DefruleConstruct,newState,argExprs,
                                    EnvGetDefruleWatchActivations,EnvSetDefruleWatchActivations));
   else
     return(ConstructSetWatchAccess(theEnv,execStatus,DefruleData(theEnv,execStatus)->DefruleConstruct,newState,argExprs,
                                    EnvGetDefruleWatchFirings,EnvSetDefruleWatchFirings));
  }

/*****************************************************************/
/* DefruleWatchPrint: Access routine for printing which defrules */
/*   have their watch flag set via the list-watch-items command. */
/*****************************************************************/
globle unsigned DefruleWatchPrint(
  void *theEnv,
  EXEC_STATUS,
  char *logName,
  int code,
  struct expr *argExprs)
  {   
   if (code)
     return(ConstructPrintWatchAccess(theEnv,execStatus,DefruleData(theEnv,execStatus)->DefruleConstruct,logName,argExprs,
                                      EnvGetDefruleWatchActivations,EnvSetDefruleWatchActivations));
   else
     return(ConstructPrintWatchAccess(theEnv,execStatus,DefruleData(theEnv,execStatus)->DefruleConstruct,logName,argExprs,
                                      EnvGetDefruleWatchActivations,EnvSetDefruleWatchActivations));
  }

#endif /* DEBUGGING_FUNCTIONS */

#endif /* DEFTEMPLATE_CONSTRUCT */


