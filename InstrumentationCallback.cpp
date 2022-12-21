#include "InstrumentationCallback.h"

VECTORED_INSTRUMENTATION_CALLBACK InstrumentationCallback::g_Callbacks[ MAX_INSTRUMENTATION_CALLBACKS ];

CHAR g_CallbackListSpinlock = NULL;

NTSTATUS
InstrumentationCallback::Initialize(
	VOID
	)
{
	RtlZeroMemory( InstrumentationCallback::g_Callbacks, sizeof( InstrumentationCallback::g_Callbacks ) );

	INSTRUMENTATION_CALLBACK_INFORMATION CallbackInfo
	{
	};

	CallbackInfo.Callback = InstrumentationCallbackEntry;

	return NtSetInformationProcess( ( HANDLE )-1, ( PROCESS_INFORMATION_CLASS )40, &CallbackInfo, sizeof( CallbackInfo ) );
}

BOOLEAN
InstrumentationCallback::AddCallback( 
	IN VECTORED_INSTRUMENTATION_CALLBACK Callback 
	)
{
	//
	// Acquire spinlock to prevent race conditions in multi-threaded adds and removes
	//
	while ( _InterlockedExchange8( &g_CallbackListSpinlock, TRUE ) == TRUE ) {
		_mm_pause( );
	}

	BOOLEAN Result = FALSE;

	for ( UINT32 i = NULL; i < MAX_INSTRUMENTATION_CALLBACKS; i++ )
	{
		//
		// Search for the next available callback slot to occupy with our callback
		//
		if ( InstrumentationCallback::g_Callbacks[ i ] == NULL ) 
		{
			InstrumentationCallback::g_Callbacks[ i ] = Callback;
			Result = TRUE;

			break;
		}
	}

	//
	// Release the spinlock
	//
	g_CallbackListSpinlock = FALSE;

	return Result;
}

BOOLEAN
InstrumentationCallback::RemoveCallback( 
	IN LPVOID Callback 
	)
{
	//
	// Acquire spinlock to prevent race conditions in multi-threaded adds and removes
	//
	while ( _InterlockedExchange8( &g_CallbackListSpinlock, TRUE ) == TRUE ) {
		_mm_pause( );
	}

	BOOLEAN Result = FALSE;

	for ( UINT32 i = NULL; i < MAX_INSTRUMENTATION_CALLBACKS; i++ )
	{
		//
		// Search for the callback that matches the function pointer to invalidate the slot
		//
		if ( InstrumentationCallback::g_Callbacks[ i ] == Callback ) 
		{
			InstrumentationCallback::g_Callbacks[ i ] = NULL;
			Result = TRUE;

			break;
		}
	}

	//
	// Release the spinlock
	//
	g_CallbackListSpinlock = FALSE;

	return Result;
}

VOID
InstrumentationCallbackHandler(
	IN PCONTEXT PreviousContext 
	)
{
	//
	// TEB->InstrumentationCallbackDisabled member dedicated for the prevention of instrumentation callback recursion
	//
	CHAR* InstrumentationCallbackDisabled = ( CHAR* )( __readgsqword( 0x30 ) + 0x2EC );

	//
	// Prevent recursion inside our instrumentation callback
	//
	if ( _InterlockedExchange8( InstrumentationCallbackDisabled, TRUE ) == FALSE )
	{
		for ( UINT32 i = NULL; i < MAX_INSTRUMENTATION_CALLBACKS; i++ ) 
		{
			if ( InstrumentationCallback::g_Callbacks[ i ] != NULL ) {
				 InstrumentationCallback::g_Callbacks[ i ]( PreviousContext );
			}
		}

		//
		// Remove the recursion lock after we're done doing our things
		//
		*InstrumentationCallbackDisabled = FALSE;
	}

	//
	// Restore the current context to the corrected context after the instrumentation callback
	//
	RtlRestoreContext( PreviousContext, NULL );
}
