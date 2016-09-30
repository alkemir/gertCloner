/*
Copyright (c) 2012, Broadcom Europe Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>

#include "bcm_host.h"

int main(void) {
  DISPMANX_DISPLAY_HANDLE_T   displaySrc;
  DISPMANX_DISPLAY_HANDLE_T   displayDst;
  DISPMANX_MODEINFO_T         infoSrc;
  DISPMANX_MODEINFO_T         infoDst;
  DISPMANX_UPDATE_HANDLE_T    update;
  DISPMANX_RESOURCE_HANDLE_T  resource;
  DISPMANX_ELEMENT_HANDLE_T   element;

  VC_RECT_T       rectSrc;
  VC_RECT_T       rectDst;
  VC_IMAGE_TYPE_T imageType = VC_IMAGE_RGB565;

  uint32_t image_ptr;
  int ret;
  int widthSrc;
  int heightSrc;
  int widthDst;
  int heightDst;

  bcm_host_init();

  printf("Cloning display[%i]... to display[%i]\n", 0 , 4);
  displaySrc = vc_dispmanx_display_open( 0 ); // TODO: Get this from args
  displayDst = vc_dispmanx_display_open( 4 ); // and this

  ret = vc_dispmanx_display_get_info( displaySrc, &infoSrc );
  assert( ret == 0 );
  ret = vc_dispmanx_display_get_info( displayDst, &infoDst );
  assert( ret == 0 );

  widthSrc = infoSrc.width;
  heightSrc = infoSrc.height;
  widthDst = infoDst.width;
  heightDst = infoDst.height;

  printf("display[%i] res: %dx%d\ndisplay[%i] res: %dx%d\n", 0, widthSrc, heightSrc, 4, widthDst, heightDst); // params from args

  vc_dispmanx_rect_set( &rectSrc, 0, 0, widthDst << 16, heightDst << 16 ); // Not sure about this line
  vc_dispmanx_rect_set( &rectDst, 0, 0, widthDst, heightDst );

  printf("Creating resource\n");
  resource = vc_dispmanx_resource_create( imageType, widthDst, heightDst, &image_ptr );
  assert( resource );

  printf("Adding resource\n");
  update = vc_dispmanx_update_start( 0 );
  assert( update );

  element = vc_dispmanx_element_add( update, displayDst, 0, &rectDst, resource, &rectSrc,
                                    DISPMANX_PROTECTION_NONE, 0, NULL, 0 );

  ret = vc_dispmanx_update_submit_sync( update );
  assert( ret == 0 );

  printf( "Looping...\n" );
  while (1) { // TODO: Break on signal handler
      vc_dispmanx_snapshot( displaySrc, resource, 0 );
      usleep(25 * 1000); // Get this from args
  }
  printf("Quitting...\n");

  update = vc_dispmanx_update_start(0);
  assert( update );
  ret = vc_dispmanx_element_remove( update, element );
  assert( ret == 0 );
  ret = vc_dispmanx_update_submit_sync( update );
  assert( ret == 0 );
  ret = vc_dispmanx_resource_delete( resource );
  assert( ret == 0 );
  ret = vc_dispmanx_display_close( displaySrc );
  assert( ret == 0 );
  ret = vc_dispmanx_display_close( displayDst );
  assert( ret == 0 );

  return 0;
}
