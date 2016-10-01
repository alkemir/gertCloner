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
#include <string.h>
#include <signal.h>

#include "bcm_host.h"


void printHelp() {
	printf("Usage: ./gertCloner -displaySrc <value> -displayDst <value> -framePeriod <value>\n");
	printf("\tdisplaySrc: corresponds to the id of the source display (Default: 0)\n");
	printf("\tdisplayDst: corresponds to the id of the destination display (Default: 4)\n");
	printf("\tframePeriod: delay between frames in milliseconds (Default: 15)\n");
}

int parseFlags(int argc, char **argv, int *displaySrcID, int *displayDstID, int *framePeriod) {
	int ix = 1;
	for(; ix < argc; ix++) {
		if(argv[ix][0] != '-')
			continue; // Maybe we should return an error here.

		if(!strcmp(argv[ix]+1, "displaySrc")) {
			*displaySrcID = atoi(argv[++ix]);
			continue;
		}
		if(!strcmp(argv[ix]+1, "displayDst")) {
			*displayDstID = atoi(argv[++ix]);
			continue;
		}
		if(!strcmp(argv[ix]+1, "framePeriod")) {
			*framePeriod = atoi(argv[++ix]);
			continue;
		}

		if(!strcmp(argv[ix]+1, "help") || argv[ix][1] == 'h') {
			printHelp();
			return -1;
		}
		printf("Error: %s is not recognized as a valid flag string.\n", argv[ix]);
		printHelp();
		return -2;
	}

	if(displaySrcID == displayDstID) {
		printf("Displays can't have the same id:\n");
		printf("\tdisplaySrc: %i", *displaySrcID);
		printf("\tdisplayDst: %i", *displayDstID);
		return -3;
	}

	if(*framePeriod < 1) {
		printf("Frame period can't have a value lower than one. (current: %i)\n", *framePeriod);
		return -4;
	}

	return 0;
}

static int LOOPER = 1;
void loopHandler(int sig) {
	LOOPER = 0;
}

int main(int argc, char **argv) {
	int displaySrcID = 0;
	int displayDstID = 4;
	int framePeriod  = 15;
	if(parseFlags(argc, argv, &displaySrcID, &displayDstID, &framePeriod))
		return -1;

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

	printf("Cloning display[%i]... to display[%i]\n", displaySrcID , displayDstID);
	displaySrc = vc_dispmanx_display_open( displaySrcID );
	displayDst = vc_dispmanx_display_open( displayDstID );

	ret = vc_dispmanx_display_get_info( displaySrc, &infoSrc );
	assert( ret == 0 );
	ret = vc_dispmanx_display_get_info( displayDst, &infoDst );
	assert( ret == 0 );

	widthSrc = infoSrc.width;
	heightSrc = infoSrc.height;
	widthDst = infoDst.width;
	heightDst = infoDst.height;

	printf("display[%i] res: %dx%d\ndisplay[%i] res: %dx%d\n", displaySrcID, widthSrc, heightSrc, displayDstID, widthDst, heightDst);

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
	signal(SIGINT, loopHandler);
	while(LOOPER) {
		vc_dispmanx_snapshot( displaySrc, resource, 0 );
		usleep(framePeriod * 1000);
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
