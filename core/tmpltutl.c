   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*             CLIPS Version 6.24  06/05/06            */
   /*                                                     */
   /*            DEFTEMPLATE UTILITIES MODULE             */
   /*******************************************************/

/*************************************************************/
/* Purpose: Provides utility routines for deftemplates.      */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*      Brian L. Dantes                                      */
/*                                                           */
/* Revision History:                                         */
/*      6.23: Added support for templates maintaining their  */
/*            own list of facts.                             */
/*                                                           */
/*      6.24: Renamed BOOLEAN macro type to intBool.         */
/*                                                           */
/*            Added additional arguments to                  */
/*            InvalidDeftemplateSlotMessage function.        */
/*                                                           */
/*            Added additional arguments to                  */
/*            PrintTemplateFact function.                    */
/*                                                           */
/*************************************************************/

#define  _TMPLTUTL_SOURCE_

#include "setup.h"

#if DEFTEMPLATE_CONSTRUCT

#include <stdio.h>

#define _STDIO_INCLUDED_

#include <string.h>

#include "extnfunc.h"
#include "memalloc.h"
#include "constrct.h"
#include "router.h"
#include "argacces.h"
#include "cstrnchk.h"
#include "envrnmnt.h"
#include "tmpltfun.h"
#include "tmpltpsr.h"
#include "modulutl.h"
#include "watch.h"
#include "sysdep.h"
#include "tmpltbsc.h"
#include "tmpltdef.h"

#include "tmpltutl.h"

/********************************************************/
/* InvalidDeftemplateSlotMessage: Generic error message */
/*   for use when a specified slot name isn't defined   */
/*   in its corresponding deftemplate.                  */
/********************************************************/
globle void InvalidDeftemplateSlotMessage(
  void *theEnv,
  EXEC_STATUS,
  char *slotName,
  char *deftemplateName,
  int printCR)
  {
   PrintErrorID(theEnv,execStatus,"TMPLTDEF",1,printCR);
   EnvPrintRouter(theEnv,execStatus,WERROR,"Invalid slot ");
   EnvPrintRouter(theEnv,execStatus,WERROR,slotName);
   EnvPrintRouter(theEnv,execStatus,WERROR," not defined in corresponding deftemplate ");
   EnvPrintRouter(theEnv,execStatus,WERROR,deftemplateName);
   EnvPrintRouter(theEnv,execStatus,WERROR,".\n");
  }

/**********************************************************/
/* SingleFieldSlotCardinalityError: Generic error message */
/*   used when an attempt is made to placed a multifield  */
/*   value into a single field slot.                      */
/**********************************************************/
globle void SingleFieldSlotCardinalityError(
  void *theEnv,
  EXEC_STATUS,
  char *slotName)
  {
   PrintErrorID(theEnv,execStatus,"TMPLTDEF",2,TRUE);
   EnvPrintRouter(theEnv,execStatus,WERROR,"The single field slot ");
   EnvPrintRouter(theEnv,execStatus,WERROR,slotName);
   EnvPrintRouter(theEnv,execStatus,WERROR," can only contain a single field value.\n");
  }

/**********************************************************************/
/* MultiIntoSingleFieldSlotError: Determines if a multifield value is */
/*   being placed into a single field slot of a deftemplate fact.     */
/**********************************************************************/
globle void MultiIntoSingleFieldSlotError(
  void *theEnv,
  EXEC_STATUS,
  struct templateSlot *theSlot,
  struct deftemplate *theDeftemplate)
  {
   PrintErrorID(theEnv,execStatus,"TMPLTFUN",2,TRUE);
   EnvPrintRouter(theEnv,execStatus,WERROR,"Attempted to assert a multifield value \n");
   EnvPrintRouter(theEnv,execStatus,WERROR,"into the single field slot ");
   if (theSlot != NULL) EnvPrintRouter(theEnv,execStatus,WERROR,theSlot->slotName->contents);
   else EnvPrintRouter(theEnv,execStatus,WERROR,"<<unknown>>");
   EnvPrintRouter(theEnv,execStatus,WERROR," of deftemplate ");
   if (theDeftemplate != NULL) EnvPrintRouter(theEnv,execStatus,WERROR,theDeftemplate->header.name->contents);
   else EnvPrintRouter(theEnv,execStatus,WERROR,"<<unknown>>");
   EnvPrintRouter(theEnv,execStatus,WERROR,".\n");

   SetEvaluationError(theEnv,execStatus,TRUE);
  }

/**************************************************************/
/* CheckTemplateFact: Checks a fact to see if it violates any */
/*   deftemplate type, allowed-..., or range specifications.  */
/**************************************************************/
globle void CheckTemplateFact(
  void *theEnv,
  EXEC_STATUS,
  struct fact *theFact)
  {
   struct field *sublist;
   int i;
   struct deftemplate *theDeftemplate;
   struct templateSlot *slotPtr;
   DATA_OBJECT theData;
   char thePlace[20];
   int rv;

   if (! EnvGetDynamicConstraintChecking(theEnv,execStatus)) return;

   sublist = theFact->theProposition.theFields;

   /*========================================================*/
   /* If the deftemplate corresponding to the first field of */
   /* of the fact cannot be found, then the fact cannot be   */
   /* checked against the deftemplate format.                */
   /*========================================================*/

   theDeftemplate = theFact->whichDeftemplate;
   if (theDeftemplate == NULL) return;
   if (theDeftemplate->implied) return;

   /*=============================================*/
   /* Check each of the slots of the deftemplate. */
   /*=============================================*/

   i = 0;
   for (slotPtr = theDeftemplate->slotList;
        slotPtr != NULL;
        slotPtr = slotPtr->next)
     {
      /*================================================*/
      /* Store the slot value in the appropriate format */
      /* for a call to the constraint checking routine. */
      /*================================================*/

      if (slotPtr->multislot == FALSE)
        {
         theData.type = sublist[i].type;
         theData.value = sublist[i].value;
         i++;
        }
      else
        {
         theData.type = MULTIFIELD;
         theData.value = (void *) sublist[i].value;
         SetDOBegin(theData,1);
         SetDOEnd(theData,((struct multifield *) sublist[i].value)->multifieldLength);
         i++;
        }

      /*=============================================*/
      /* Call the constraint checking routine to see */
      /* if a constraint violation occurred.         */
      /*=============================================*/

      rv = ConstraintCheckDataObject(theEnv,execStatus,&theData,slotPtr->constraints);
      if (rv != NO_VIOLATION)
        {
         gensprintf(thePlace,"fact f-%-5lld ",theFact->factIndex);

         PrintErrorID(theEnv,execStatus,"CSTRNCHK",1,TRUE);
         EnvPrintRouter(theEnv,execStatus,WERROR,"Slot value ");
         PrintDataObject(theEnv,execStatus,WERROR,&theData);
         EnvPrintRouter(theEnv,execStatus,WERROR," ");
         ConstraintViolationErrorMessage(theEnv,execStatus,NULL,thePlace,FALSE,0,slotPtr->slotName,
                                         0,rv,slotPtr->constraints,TRUE);
         SetHaltExecution(theEnv,execStatus,TRUE);
         return;
        }
     }

   return;
  }

/***********************************************************************/
/* CheckRHSSlotTypes: Checks the validity of a change to a slot as the */
/*   result of an assert, modify, or duplicate command. This checking  */
/*   is performed statically (i.e. when the command is being parsed).  */
/***********************************************************************/
globle intBool CheckRHSSlotTypes(
  void *theEnv,
  EXEC_STATUS,
  struct expr *rhsSlots,
  struct templateSlot *slotPtr,
  char *thePlace)
  {
   int rv;
   char *theName;

   if (EnvGetStaticConstraintChecking(theEnv,execStatus) == FALSE) return(TRUE);
      rv = ConstraintCheckExpressionChain(theEnv,execStatus,rhsSlots,slotPtr->constraints);
      if (rv != NO_VIOLATION)
        {
         if (rv != CARDINALITY_VIOLATION) theName = "A literal slot value";
         else theName = "Literal slot values";
         ConstraintViolationErrorMessage(theEnv,execStatus,theName,thePlace,TRUE,0,
                                         slotPtr->slotName,0,rv,slotPtr->constraints,TRUE);
         return(0);
        }

   return(1);
  }

/*********************************************************/
/* GetNthSlot: Given a deftemplate and an integer index, */
/*   returns the nth slot of a deftemplate.              */
/*********************************************************/
globle struct templateSlot *GetNthSlot(
  struct deftemplate *theDeftemplate,
  int position)
  {
   struct templateSlot *slotPtr;
   int i = 0;

   slotPtr = theDeftemplate->slotList;
   while (slotPtr != NULL)
     {
      if (i == position) return(slotPtr);
      slotPtr = slotPtr->next;
      i++;
     }

   return(NULL);
  }

/*******************************************************/
/* FindSlotPosition: Finds the position of a specified */
/*   slot in a deftemplate structure.                  */
/*******************************************************/
globle int FindSlotPosition(
  struct deftemplate *theDeftemplate,
  SYMBOL_HN *name)
  {
   struct templateSlot *slotPtr;
   int position;

   for (slotPtr = theDeftemplate->slotList, position = 1;
        slotPtr != NULL;
        slotPtr = slotPtr->next, position++)
     {
      if (slotPtr->slotName == name)
        { return(position); }
     }

   return(0);
  }

/*******************************************************************/
/* PrintTemplateFact: Prints a fact using the deftemplate format.  */
/*   Returns TRUE if the fact was printed using this format, */
/*   otherwise FALSE.                                        */
/*******************************************************************/
globle void PrintTemplateFact(
  void *theEnv,
  EXEC_STATUS,
  char *logicalName,
  struct fact *theFact,
  int seperateLines,
  int ignoreDefaults)
  {
   struct field *sublist;
   int i;
   struct deftemplate *theDeftemplate;
   struct templateSlot *slotPtr;
   DATA_OBJECT tempDO;
   int slotPrinted = FALSE;
   
   /*==============================*/
   /* Initialize some information. */
   /*==============================*/

   theDeftemplate = theFact->whichDeftemplate;
   sublist = theFact->theProposition.theFields;

   /*=============================================*/
   /* Print the relation name of the deftemplate. */
   /*=============================================*/

   EnvPrintRouter(theEnv,execStatus,logicalName,"(");
   EnvPrintRouter(theEnv,execStatus,logicalName,theDeftemplate->header.name->contents);

   /*===================================================*/
   /* Print each of the field slots of the deftemplate. */
   /*===================================================*/

   slotPtr = theDeftemplate->slotList;

   i = 0;
   while (slotPtr != NULL)
     {         
      /*=================================================*/
      /* If we're ignoring slots with their original     */
      /* default value, check to see if the fact's slot  */
      /* value differs from the deftemplate default.     */
      /*=================================================*/
      
      if (ignoreDefaults && (slotPtr->defaultDynamic == FALSE))
        {
         DeftemplateSlotDefault(theEnv,execStatus,theDeftemplate,slotPtr,&tempDO,TRUE);
         
         if (slotPtr->multislot == FALSE)
           {
            if ((GetType(tempDO) == sublist[i].type) &&
                (GetValue(tempDO) == sublist[i].value))
              {     
               i++;
               slotPtr = slotPtr->next;
               continue;
              }
           }
         else if (MultifieldsEqual((struct multifield*) GetValue(tempDO),
                                   (struct multifield *) sublist[i].value))
           {
            i++;
            slotPtr = slotPtr->next;
            continue;
           }
        }
        
      /*===========================================*/
      /* Print the opening parenthesis of the slot */
      /* and the slot name.                        */
      /*===========================================*/
     
      if (! slotPrinted) 
        { 
         slotPrinted = TRUE;
         EnvPrintRouter(theEnv,execStatus,logicalName," "); 
        }

      if (seperateLines)
        { EnvPrintRouter(theEnv,execStatus,logicalName,"\n   "); }

      EnvPrintRouter(theEnv,execStatus,logicalName,"(");
      EnvPrintRouter(theEnv,execStatus,logicalName,slotPtr->slotName->contents);

      /*======================================================*/
      /* Print the value of the slot for a single field slot. */
      /*======================================================*/

      if (slotPtr->multislot == FALSE)
        {
         EnvPrintRouter(theEnv,execStatus,logicalName," ");
         PrintAtom(theEnv,execStatus,logicalName,sublist[i].type,sublist[i].value);
        }

      /*==========================================================*/
      /* Else print the value of the slot for a multi field slot. */
      /*==========================================================*/

      else
        {
         struct multifield *theSegment;

         theSegment = (struct multifield *) sublist[i].value;
         if (theSegment->multifieldLength > 0)
           {
            EnvPrintRouter(theEnv,execStatus,logicalName," ");
            PrintMultifield(theEnv,execStatus,logicalName,(struct multifield *) sublist[i].value,
                            0,(long) theSegment->multifieldLength-1,FALSE);
           }
        }

      /*============================================*/
      /* Print the closing parenthesis of the slot. */
      /*============================================*/

      i++;
      EnvPrintRouter(theEnv,execStatus,logicalName,")");
      slotPtr = slotPtr->next;
      if (slotPtr != NULL) EnvPrintRouter(theEnv,execStatus,logicalName," ");
     }

   EnvPrintRouter(theEnv,execStatus,logicalName,")");
  }

/***************************************************************************/
/* UpdateDeftemplateScope: Updates the scope flag of all the deftemplates. */
/***************************************************************************/
globle void UpdateDeftemplateScope(
  void *theEnv,
  EXEC_STATUS)
  {
   struct deftemplate *theDeftemplate;
   int moduleCount;
   struct defmodule *theModule;
   struct defmoduleItemHeader *theItem;

   /*==================================*/
   /* Loop through all of the modules. */
   /*==================================*/

   for (theModule = (struct defmodule *) EnvGetNextDefmodule(theEnv,execStatus,NULL);
        theModule != NULL;
        theModule = (struct defmodule *) EnvGetNextDefmodule(theEnv,execStatus,theModule))
     {
      /*======================================================*/
      /* Loop through each of the deftemplates in the module. */
      /*======================================================*/

      theItem = (struct defmoduleItemHeader *)
                GetModuleItem(theEnv,execStatus,theModule,DeftemplateData(theEnv,execStatus)->DeftemplateModuleIndex);

      for (theDeftemplate = (struct deftemplate *) theItem->firstItem;
           theDeftemplate != NULL ;
           theDeftemplate = (struct deftemplate *) EnvGetNextDeftemplate(theEnv,execStatus,theDeftemplate))
        {
         /*=======================================*/
         /* If the deftemplate can be seen by the */
         /* current module, then it is in scope.  */
         /*=======================================*/

         if (FindImportedConstruct(theEnv,execStatus,"deftemplate",theModule,
                                   ValueToString(theDeftemplate->header.name),
                                   &moduleCount,TRUE,NULL) != NULL)
           { theDeftemplate->inScope = TRUE; }
         else
           { theDeftemplate->inScope = FALSE; }
        }
     }
  }

/****************************************************************/
/* FindSlot: Finds a specified slot in a deftemplate structure. */
/****************************************************************/
globle struct templateSlot *FindSlot(
  struct deftemplate *theDeftemplate,
  SYMBOL_HN *name,
  short *whichOne)
  {
   struct templateSlot *slotPtr;

   *whichOne = 1;
   slotPtr = theDeftemplate->slotList;
   while (slotPtr != NULL)
     {
      if (slotPtr->slotName == name)
        { return(slotPtr); }
      (*whichOne)++;
      slotPtr = slotPtr->next;
     }

   *whichOne = -1;
   return(NULL);
  }

#if (! RUN_TIME) && (! BLOAD_ONLY)

/************************************************************/
/* CreateImpliedDeftemplate: Creates an implied deftemplate */
/*   and adds it to the list of deftemplates.               */
/************************************************************/
globle struct deftemplate *CreateImpliedDeftemplate(
  void *theEnv,
  EXEC_STATUS,
  SYMBOL_HN *deftemplateName,
  int setFlag)
  {
   struct deftemplate *newDeftemplate;

   newDeftemplate = get_struct(theEnv,execStatus,deftemplate);
   newDeftemplate->header.name = deftemplateName;
   newDeftemplate->header.ppForm = NULL;
   newDeftemplate->header.usrData = NULL;
   newDeftemplate->slotList = NULL;
   newDeftemplate->implied = setFlag;
   newDeftemplate->numberOfSlots = 0;
   newDeftemplate->inScope = 1;
   newDeftemplate->patternNetwork = NULL;
   newDeftemplate->factList = NULL;
   newDeftemplate->lastFact = NULL;
   newDeftemplate->busyCount = 0;
   newDeftemplate->watch = FALSE;
   newDeftemplate->header.next = NULL;

#if DEBUGGING_FUNCTIONS
   if (EnvGetWatchItem(theEnv,execStatus,"facts"))
     { EnvSetDeftemplateWatch(theEnv,execStatus,ON,(void *) newDeftemplate); }
#endif

   newDeftemplate->header.whichModule = (struct defmoduleItemHeader *)
                                        GetModuleItem(theEnv,execStatus,NULL,DeftemplateData(theEnv,execStatus)->DeftemplateModuleIndex);

   AddConstructToModule(&newDeftemplate->header);
   InstallDeftemplate(theEnv,execStatus,newDeftemplate);

   return(newDeftemplate);
  }

#endif

#endif /* DEFTEMPLATE_CONSTRUCT */

