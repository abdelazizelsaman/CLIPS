   /*******************************************************/
   /*      "C" Language Integrated Production System      */
   /*                                                     */
   /*             CLIPS Version 6.22  06/15/04            */
   /*                                                     */
   /*            DEFGLOBAL CONSTRUCTS-TO-C MODULE         */
   /*******************************************************/

/*************************************************************/
/* Purpose: Implements the constructs-to-c feature for the   */
/*    defglobal construct.                                   */
/*                                                           */
/* Principal Programmer(s):                                  */
/*      Gary D. Riley                                        */
/*                                                           */
/* Contributing Programmer(s):                               */
/*      Brian L. Dantes                                      */
/*                                                           */
/* Revision History:                                         */
/*                                                           */
/*************************************************************/

#define _GLOBLCMP_SOURCE_

#include "setup.h"

#if DEFGLOBAL_CONSTRUCT && CONSTRUCT_COMPILER && (! RUN_TIME)

#include <stdio.h>
#define _STDIO_INCLUDED_

#include "conscomp.h"
#include "globldef.h"
#include "envrnmnt.h"
#include "globlcmp.h"

/***************************************/
/* LOCAL INTERNAL FUNCTION DEFINITIONS */
/***************************************/

   static int                     ConstructToCode(void *,EXEC_STATUS,char *,char *,char *,int,FILE *,int,int);
   static void                    DefglobalToCode(void *,EXEC_STATUS,FILE *,struct defglobal *,
                                                 int,int,int);
   static void                    DefglobalModuleToCode(void *,EXEC_STATUS,FILE *,struct defmodule *,int,int,int);
   static void                    CloseDefglobalFiles(void *,EXEC_STATUS,FILE *,FILE *,int);
   static void                    BeforeDefglobalsToCode(void *,EXEC_STATUS);
   static void                    InitDefglobalsCode(void *,EXEC_STATUS,FILE *,int,int);

/***************************************************************/
/* DefglobalCompilerSetup: Initializes the defglobal construct */
/*    for use with the constructs-to-c command.                */
/***************************************************************/
globle void DefglobalCompilerSetup(
  void *theEnv,
  EXEC_STATUS)
  {
   DefglobalData(theEnv,execStatus)->DefglobalCodeItem = 
      AddCodeGeneratorItem(theEnv,execStatus,"defglobal",0,BeforeDefglobalsToCode,
                           InitDefglobalsCode,ConstructToCode,2);
  }

/**************************************************************/
/* BeforeDefglobalsToCode: Assigns each defglobal a unique ID */
/*   which will be used for pointer references when the data  */
/*   structures are written to a file as C code               */
/**************************************************************/
static void BeforeDefglobalsToCode(
  void *theEnv,
  EXEC_STATUS)
  {
   MarkConstructBsaveIDs(theEnv,execStatus,DefglobalData(theEnv,execStatus)->DefglobalModuleIndex);
  }

/*************************************************/
/* InitDefglobalsCode: Writes out initialization */
/*   code for defglobals for a run-time module.  */
/*************************************************/
#if WIN_BTC
#pragma argsused
#endif
static void InitDefglobalsCode(
  void *theEnv,
  EXEC_STATUS,
  FILE *initFP,
  int imageID,
  int maxIndices)
  {
#if MAC_MCW || WIN_MCW || MAC_XCD
#pragma unused(maxIndices)
#pragma unused(imageID)
#pragma unused(theEnv,execStatus)
#endif
   fprintf(initFP,"   ResetDefglobals(theEnv,execStatus);\n");
  }

/***********************************************************/
/* ConstructToCode: Produces defglobal code for a run-time */
/*   module created using the constructs-to-c function.    */
/***********************************************************/
static int ConstructToCode(
  void *theEnv,
  EXEC_STATUS,
  char *fileName,
  char *pathName,
  char *fileNameBuffer,
  int fileID,
  FILE *headerFP,
  int imageID,
  int maxIndices)
  {
   int fileCount = 1;
   struct defmodule *theModule;
   struct defglobal *theDefglobal;
   int moduleCount = 0, moduleArrayCount = 0, moduleArrayVersion = 1;
   int defglobalArrayCount = 0, defglobalArrayVersion = 1;
   FILE *moduleFile = NULL, *defglobalFile = NULL;

   /*================================================*/
   /* Include the appropriate defglobal header file. */
   /*================================================*/

   fprintf(headerFP,"#include \"globldef.h\"\n");

   /*===================================================================*/
   /* Loop through all the modules and all the defglobals writing their */
   /*  C code representation to the file as they are traversed.         */
   /*===================================================================*/

   for (theModule = (struct defmodule *) EnvGetNextDefmodule(theEnv,execStatus,NULL);
        theModule != NULL;
        theModule = (struct defmodule *) EnvGetNextDefmodule(theEnv,execStatus,theModule))
     {
      EnvSetCurrentModule(theEnv,execStatus,(void *) theModule);

      moduleFile = OpenFileIfNeeded(theEnv,execStatus,moduleFile,fileName,pathName,fileNameBuffer,fileID,imageID,&fileCount,
                                    moduleArrayVersion,headerFP,
                                    "struct defglobalModule",ModulePrefix(DefglobalData(theEnv,execStatus)->DefglobalCodeItem),
                                    FALSE,NULL);

      if (moduleFile == NULL)
        {
         CloseDefglobalFiles(theEnv,execStatus,moduleFile,defglobalFile,maxIndices);
         return(0);
        }

      DefglobalModuleToCode(theEnv,execStatus,moduleFile,theModule,imageID,maxIndices,moduleCount);
      moduleFile = CloseFileIfNeeded(theEnv,execStatus,moduleFile,&moduleArrayCount,&moduleArrayVersion,
                                     maxIndices,NULL,NULL);

      for (theDefglobal = (struct defglobal *) EnvGetNextDefglobal(theEnv,execStatus,NULL);
           theDefglobal != NULL;
           theDefglobal = (struct defglobal *) EnvGetNextDefglobal(theEnv,execStatus,theDefglobal))
        {
         defglobalFile = OpenFileIfNeeded(theEnv,execStatus,defglobalFile,fileName,pathName,fileNameBuffer,fileID,imageID,&fileCount,
                                         defglobalArrayVersion,headerFP,
                                         "struct defglobal",ConstructPrefix(DefglobalData(theEnv,execStatus)->DefglobalCodeItem),
                                         FALSE,NULL);
         if (defglobalFile == NULL)
           {
            CloseDefglobalFiles(theEnv,execStatus,moduleFile,defglobalFile,maxIndices);
            return(0);
           }

         DefglobalToCode(theEnv,execStatus,defglobalFile,theDefglobal,imageID,maxIndices,moduleCount);
         defglobalArrayCount++;
         defglobalFile = CloseFileIfNeeded(theEnv,execStatus,defglobalFile,&defglobalArrayCount,
                                           &defglobalArrayVersion,maxIndices,NULL,NULL);
        }

      moduleCount++;
      moduleArrayCount++;
     }

   CloseDefglobalFiles(theEnv,execStatus,moduleFile,defglobalFile,maxIndices);

   return(1);
  }

/**********************************************************/
/* CloseDefglobalFiles: Closes all of the C files created */
/*   for defglobals. Called when an error occurs or when  */
/*   the defglobals have all been written to the files.   */
/**********************************************************/
static void CloseDefglobalFiles(
  void *theEnv,
  EXEC_STATUS,
  FILE *moduleFile,
  FILE *defglobalFile,
  int maxIndices)
  {
   int count = maxIndices;
   int arrayVersion = 0;

   if (defglobalFile != NULL)
     {
      count = maxIndices;
      CloseFileIfNeeded(theEnv,execStatus,defglobalFile,&count,&arrayVersion,maxIndices,NULL,NULL);
     }

   if (moduleFile != NULL)
     {
      count = maxIndices;
      CloseFileIfNeeded(theEnv,execStatus,moduleFile,&count,&arrayVersion,maxIndices,NULL,NULL);
     }
  }

/***********************************************************/
/* DefglobalModuleToCode: Writes the C code representation */
/*   of a single defglobal module to the specified file.   */
/***********************************************************/
#if WIN_BTC
#pragma argsused
#endif
static void DefglobalModuleToCode(
  void *theEnv,
  EXEC_STATUS,
  FILE *theFile,
  struct defmodule *theModule,
  int imageID,
  int maxIndices,
  int moduleCount)
  {
#if MAC_MCW || WIN_MCW || MAC_XCD
#pragma unused(moduleCount)
#endif

   fprintf(theFile,"{");

   ConstructModuleToCode(theEnv,execStatus,theFile,theModule,imageID,maxIndices,
                                  DefglobalData(theEnv,execStatus)->DefglobalModuleIndex,ConstructPrefix(DefglobalData(theEnv,execStatus)->DefglobalCodeItem));

   fprintf(theFile,"}");
  }

/**********************************************************/
/* DefglobalToCode: Writes the C code representation of a */
/*   single defglobal construct to the specified file.    */
/**********************************************************/
static void DefglobalToCode(
  void *theEnv,
  EXEC_STATUS,
  FILE *theFile,
  struct defglobal *theDefglobal,
  int imageID,
  int maxIndices,
  int moduleCount)
  {
   /*==================*/
   /* Defglobal Header */
   /*==================*/

   fprintf(theFile,"{");

   ConstructHeaderToCode(theEnv,execStatus,theFile,&theDefglobal->header,imageID,maxIndices,
                         moduleCount,ModulePrefix(DefglobalData(theEnv,execStatus)->DefglobalCodeItem),
                         ConstructPrefix(DefglobalData(theEnv,execStatus)->DefglobalCodeItem));

   fprintf(theFile,",");

   /*============================================*/
   /* Watch Flag, In Scope Flag, and Busy Count. */
   /*============================================*/

   fprintf(theFile,"0,0,%ld,",theDefglobal->busyCount);

   /*================*/
   /* Current Value. */
   /*================*/

   fprintf(theFile,"{NULL,RVOID}");

   /*=====================*/
   /* Initial Expression. */
   /*=====================*/

   fprintf(theFile,",");
   PrintHashedExpressionReference(theEnv,execStatus,theFile,theDefglobal->initial,imageID,maxIndices);

   fprintf(theFile,"}");
  }

/***************************************************************/
/* DefglobalCModuleReference: Writes the C code representation */
/*   of a reference to a defglobal module data structure.      */
/***************************************************************/
globle void DefglobalCModuleReference(
  void *theEnv,
  EXEC_STATUS,
  FILE *theFile,
  int count,
  int imageID,
  int maxIndices)
  {
   fprintf(theFile,"MIHS &%s%d_%d[%d]",
                      ModulePrefix(DefglobalData(theEnv,execStatus)->DefglobalCodeItem),
                      imageID,
                      (count / maxIndices) + 1,
                      (count % maxIndices));
  }

/******************************************************************/
/* DefglobalCConstructReference: Writes the C code representation */
/*   of a reference to a defglobal data structure.                */
/******************************************************************/
globle void DefglobalCConstructReference(
  void *theEnv,
  EXEC_STATUS,
  FILE *theFile,
  void *vTheGlobal,
  int imageID,
  int maxIndices)
  {
   struct defglobal *theGlobal = (struct defglobal *) vTheGlobal;

   if (theGlobal == NULL)
     { fprintf(theFile,"NULL"); }
   else
     {
      fprintf(theFile,"&%s%d_%ld[%ld]",ConstructPrefix(DefglobalData(theEnv,execStatus)->DefglobalCodeItem),
                      imageID,
                      (theGlobal->header.bsaveID / maxIndices) + 1,
                      theGlobal->header.bsaveID % maxIndices);
     }

  }

#endif /* DEFGLOBAL_CONSTRUCT && CONSTRUCT_COMPILER && (! RUN_TIME) */


