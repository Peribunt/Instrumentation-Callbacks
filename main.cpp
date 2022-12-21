#include "InstrumentationCallback.h"

VOID 
TestCallback( 
	IN PCONTEXT Context 
	)
{
	CHAR Buffer[ 0x200 ]{
	};

	sprintf_s( Buffer, "Hello from instrumentation callback! Syscall routine ret: %p\n", Context->Rip );

	OutputDebugStringA( Buffer );
}

UINT32
main( 
	VOID 
	)
{
	InstrumentationCallback::Initialize( );
	InstrumentationCallback::AddCallback( TestCallback );
}
