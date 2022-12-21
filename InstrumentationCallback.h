#ifndef __INSTRUMENTATIONCALLBACK_H__
#define __INSTRUMENTATIONCALLBACK_H__

#include <Windows.h>
#include <winternl.h>
#include <winnt.h>

#pragma comment( lib, "ntdll.lib" )

#define MAX_INSTRUMENTATION_CALLBACKS 64

typedef struct _INSTRUMENTATION_CALLBACK_INFORMATION
{
	ULONG Version;
	ULONG Reserved;
	PVOID Callback;
}INSTRUMENTATION_CALLBACK_INFORMATION, *PINSTRUMENTATION_CALLBACK_INFORMATION;

typedef VOID( WINAPI* VECTORED_INSTRUMENTATION_CALLBACK )(
	IN PCONTEXT PreviousContext
	);

namespace InstrumentationCallback
{
	extern VECTORED_INSTRUMENTATION_CALLBACK g_Callbacks[ MAX_INSTRUMENTATION_CALLBACKS ];

	/**
	 * @brief Initialize instrumentation callbacks for the current process
	 * 
	 * @return NONE
	*/
	NTSTATUS
	Initialize( 
		VOID 
		);

	/**
	 * @brief Add a vectored instrumentation callback to the global list of callbacks
	 * 
	 * @param [in] Callback: The callback to be added
	 * 
	 * @return TRUE if the function succeeds
	 * @return FALSE if the function fails
	*/
	BOOLEAN
	AddCallback( 
		IN VECTORED_INSTRUMENTATION_CALLBACK Callback 
		);

	/**
	 * @brief Remove a vectored instrumentation callback from the global list of callbacks
	 * 
	 * @param [in] Callback: The callback to be removed
	 * 
	 * @return TRUE if the function succeeds
	 * @return FALSE if the function fails
	*/
	BOOLEAN
	RemoveCallback( 
		IN LPVOID Callback 
		);
}

EXTERN_C
{
	NTSTATUS 
	NTSYSAPI 
	NtSetInformationProcess(
		IN HANDLE                    ProcessHandle,
		IN PROCESS_INFORMATION_CLASS ProcessInformationClass,
		IN LPVOID                    ProcessInformation,
		IN DWORD                     ProcessInformationSize
		);

	VOID
	InstrumentationCallbackEntry(
		VOID 
		);

	VOID
	RestoreCallbackContext( 
		IN PCONTEXT ContextRecord 
		);

	VOID
	InstrumentationCallbackHandler(
		IN PCONTEXT PreviousContext
		);
}

#endif
