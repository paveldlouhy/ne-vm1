//
//  VMOpenGLView.h
//  NE-VM1
//
//  Created by Pavel Dlouhy (LD48) on 4/21/12.
//  Copyright 2012 @PavelDlouhy. All rights reserved.
//

#import <Cocoa/Cocoa.h>


class CRealMachine;


@interface VMOpenGLView : NSOpenGLView {
	NSTimer *timer;
	CRealMachine *rm;
}

@end
