/*
 * PROJECT:     Skiplist implementation for the ReactOS Project
 * LICENSE:     GNU LGPL v2.1 or any later version as published by the Free Software Foundation
 * PURPOSE:     Interfaces for the Skiplist
 * COPYRIGHT:   Copyright 2015 Colin Finck <colin@reactos.org>
 */

#ifndef _REACTOS_SKIPLIST_H
#define _REACTOS_SKIPLIST_H

// Define SKIPLIST_LEVELS to a value between 1 and 32 before including this header.
// This specifies the maximum number of levels you want your Skiplist to have.
// A value of X is recommended for handling up to 2^X elements.
#ifndef SKIPLIST_LEVELS
#error Please define SKIPLIST_LEVELS to a value between 1 and 32.
#endif

C_ASSERT(SKIPLIST_LEVELS >= 1);
C_ASSERT(SKIPLIST_LEVELS <= 32);

// Function pointer definitions
typedef PVOID (WINAPI *PSKIPLIST_ALLOCATE_ROUTINE)(DWORD);
typedef int (WINAPI *PSKIPLIST_COMPARE_ROUTINE)(PVOID, PVOID);
typedef void (WINAPI *PSKIPLIST_FREE_ROUTINE)(PVOID);

// Structure definitions
typedef struct _SKIPLIST_NODE
{
    DWORD Distance[SKIPLIST_LEVELS];
    struct _SKIPLIST_NODE* Next[SKIPLIST_LEVELS];
    PVOID Element;
}
SKIPLIST_NODE, *PSKIPLIST_NODE;

typedef struct _SKIPLIST
{
    SKIPLIST_NODE Head;
    CHAR MaximumLevel;
    DWORD NodeCount;
    PSKIPLIST_ALLOCATE_ROUTINE AllocateRoutine;
    PSKIPLIST_COMPARE_ROUTINE CompareRoutine;
    PSKIPLIST_FREE_ROUTINE FreeRoutine;
}
SKIPLIST, *PSKIPLIST;

// Function prototypes
void InitializeSkiplist(PSKIPLIST Skiplist, PSKIPLIST_ALLOCATE_ROUTINE AllocateRoutine, PSKIPLIST_COMPARE_ROUTINE CompareRoutine, PSKIPLIST_FREE_ROUTINE FreeRoutine);
BOOL InsertElementSkiplist(PSKIPLIST Skiplist, PVOID Element);
BOOL InsertTailElementSkiplist(PSKIPLIST Skiplist, PVOID Element);
PVOID DeleteElementSkiplist(PSKIPLIST Skiplist, PVOID Element);
PVOID LookupElementSkiplist(PSKIPLIST Skiplist, PVOID Element, PDWORD ElementIndex);

#endif
