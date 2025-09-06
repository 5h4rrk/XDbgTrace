#pragma once
#include <Windows.h>
#include <iostream>
#include "utils/utils.h"
#include <WDBGEXTS.H>

#define PS_PROTECTED_SIGNER_MASK 0xFF
#define PS_PROTECTED_AUDIT_MASK 0x08
#define PS_PROTECTED_TYPE_MASK 0x07

/**
 * The PS_PROTECTED_TYPE enumeration defines the types of protection that can be applied to a process.
 */
typedef enum _PS_PROTECTED_TYPE
{
    PsProtectedTypeNone,           // No protection.
    PsProtectedTypeProtectedLight, // Light protection.
    PsProtectedTypeProtected,      // Full protection.
    PsProtectedTypeMax
} PS_PROTECTED_TYPE;

/**
 * The PS_PROTECTED_SIGNER enumeration defines the types of signers that can be associated with a protected process.
 */
typedef enum _PS_PROTECTED_SIGNER
{
    PsProtectedSignerNone,         // No signer.
    PsProtectedSignerAuthenticode, // Authenticode signer.
    PsProtectedSignerCodeGen,      // Code generation signer.
    PsProtectedSignerAntimalware,  // Antimalware signer.
    PsProtectedSignerLsa,          // Local Security Authority signer.
    PsProtectedSignerWindows,      // Windows signer.
    PsProtectedSignerWinTcb,       // Windows Trusted Computing Base signer.
    PsProtectedSignerWinSystem,    // Windows system signer.
    PsProtectedSignerApp,          // Application signer.
    PsProtectedSignerMax
} PS_PROTECTED_SIGNER;

/**
 * The PS_PROTECTION structure is used to define the protection level of a process.
 */
typedef struct _PS_PROTECTION
{
    union
    {
        UCHAR Level;
        struct
        {
            UCHAR Type : 3;
            UCHAR Audit : 1;
            UCHAR Signer : 4;
        };
    };
} PS_PROTECTION, *PPS_PROTECTION;

BOOL __stdcall isPPL(u64 Eprocess)
{

    PS_PROTECTION Protection;
    GetFieldValue(Eprocess, "nt!_EPROCESS", "Protection", Protection);

    if (Protection.Level == 0x0)
    {
        return FALSE;
    }
    return TRUE;
}
