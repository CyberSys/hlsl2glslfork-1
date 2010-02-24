//
//Copyright (C) 2002-2005  3Dlabs Inc. Ltd.
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions
//are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    Neither the name of 3Dlabs Inc. Ltd. nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//POSSIBILITY OF SUCH DAMAGE.
//

//
//Copyright (C) 2005-2006  ATI Research, Inc.
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions
//are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    Neither the name of ATI Research, Inc. nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//POSSIBILITY OF SUCH DAMAGE.
//

#include "InitializeDll.h"
#include "../hlslang/Include/InitializeGlobals.h"
#include "../hlslang/Include/InitializeParseContext.h"

#include "../include/hlsl2glsl.h"

OS_TLSIndex ThreadInitializeIndex = OS_INVALID_TLS_INDEX;

bool InitProcess()
{
   if (ThreadInitializeIndex != OS_INVALID_TLS_INDEX)
   {
      //
      // Function is re-entrant.
      //
      return true;
   }

   ThreadInitializeIndex = OS_AllocTLSIndex();

   if (ThreadInitializeIndex == OS_INVALID_TLS_INDEX)
   {
      assert(0 && "InitProcess(): Failed to allocate TLS area for init flag");
      return false;
   }


   if (!InitializePoolIndex())
   {
      assert(0 && "InitProcess(): Failed to initalize global pool");
      return false;
   }

   if (!InitializeParseContextIndex())
   {
      assert(0 && "InitProcess(): Failed to initalize parse context");
      return false;
   }

   InitThread();
   return true;
}


bool InitThread()
{
   //
   // This function is re-entrant
   //
   if (ThreadInitializeIndex == OS_INVALID_TLS_INDEX)
   {
      assert(0 && "InitThread(): Process hasn't been initalised.");
      return false;
   }

   if (OS_GetTLSValue(ThreadInitializeIndex) != 0)
      return true;

   InitializeGlobalPools();

   if (!InitializeGlobalParseContext())
      return false;

   if (!OS_SetTLSValue(ThreadInitializeIndex, (void *)1))
   {
      assert(0 && "InitThread(): Unable to set init flag.");
      return false;
   }

   return true;
}


bool DetachThread()
{
   bool success = true;

   if (ThreadInitializeIndex == OS_INVALID_TLS_INDEX)
      return true;

   //
   // Function is re-entrant and this thread may not have been initalised.
   //
   if (OS_GetTLSValue(ThreadInitializeIndex) != 0)
   {
      if (!OS_SetTLSValue(ThreadInitializeIndex, (void *)0))
      {
         assert(0 && "DetachThread(): Unable to clear init flag.");
         success = false;
      }

      FreeGlobalPools();

      if (!FreeParseContext())
         success = false;
   }

   return success;
}

bool DetachProcess()
{
   bool success = true;

   if (ThreadInitializeIndex == OS_INVALID_TLS_INDEX)
      return true;

   Hlsl2Glsl_Finalize();

   success = DetachThread();

   FreePoolIndex();

   if (!FreeParseContextIndex())
      success = false;

   OS_FreeTLSIndex(ThreadInitializeIndex);
   ThreadInitializeIndex = OS_INVALID_TLS_INDEX;

   return success;
}

