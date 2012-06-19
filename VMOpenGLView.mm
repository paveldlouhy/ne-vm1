//
//  VMOpenGLView.m
//  NE-VM1
//
//  Created by Pavel Dlouhy (LD48) on 4/21/12.
//  Copyright 2012 @PavelDlouhy. All rights reserved.
//


#import "VMOpenGLView.h"


#include "CRealMachine.h"


@implementation VMOpenGLView

+ (NSOpenGLPixelFormat *) customPixelFormat
{
	NSOpenGLPixelFormatAttribute attribs[] = {
		NSOpenGLPFAWindow,
		NSOpenGLPFADoubleBuffer,
		(NSOpenGLPixelFormatAttribute)nil
	};
	
	return [[[NSOpenGLPixelFormat alloc] initWithAttributes:attribs] autorelease];
}

- (id) initWithFrame:(NSRect)frame {
	NSOpenGLPixelFormat *pf = [VMOpenGLView customPixelFormat];
	
    self = [super initWithFrame:frame pixelFormat:pf];
    if (self) {
        rm = new CRealMachine();
    }
    return self;
}

- (void) drawRect:(NSRect)dirtyRect {
    if (rm) {
		rm->Render();
	}
	
	[[self openGLContext] flushBuffer];
}

- (void)timerFireMethod:(NSTimer*)theTimer
{
	//NSLog(@"timerFireMethod\n");
	if (rm)
		rm->PushEvent(REVNT_TICK);
	[self drawRect:[self bounds]];
}

- (void) awakeFromNib
{
	timer = [NSTimer timerWithTimeInterval:(1.0f/60.0f) target:self selector:@selector(timerFireMethod:) userInfo:nil repeats:YES];
	[[NSRunLoop currentRunLoop] addTimer:timer forMode:NSDefaultRunLoopMode];
	
	[[self window] setAcceptsMouseMovedEvents:YES];
}

- (BOOL) acceptsFirstResponder
{
	return YES;
}

- (void)mouseMoved:(NSEvent *)event
{
	NSPoint p = [self convertPoint:[event locationInWindow] fromView:nil]; 
	//NSLog(@"mouseMoved: [%d,%d]\n", (int)p.x, (int)p.y );
	if ((rm) && (p.x >= 0) && (p.x < RSCREEN_DX) && (p.y >= 0) && (p.y < RSCREEN_DY))
		rm->PushEvent(REVNT_MOUSEMOVED, (int)p.x, (int)p.y);
}

- (void)mouseDragged:(NSEvent *)event
{
	NSPoint p = [self convertPoint:[event locationInWindow] fromView:nil]; 
	//NSLog(@"mouseDragged: [%d,%d]\n", (int)p.x, RSCREEN_DY - (int)p.y);
	if ((rm) && (p.x >= 0) && (p.x < RSCREEN_DX) && (p.y >= 0) && (p.y < RSCREEN_DY))
		rm->PushEvent(REVNT_MOUSEDRAGGED, (int)p.x, (int)p.y);
}

- (void)mouseDown:(NSEvent *)event
{
	//NSLog(@"mouseDown\n");
	if (rm)
		rm->PushEvent(REVNT_MOUSEDOWN);
}

- (void)keyDown:(NSEvent *)event
{
	int c = [[event characters] characterAtIndex:0];
	if ([event modifierFlags] & NSCommandKeyMask) {
		//NSLog(@"cmdKeyDown\n");
		if (rm)
			rm->PushEvent(REVNT_CMDKEYDOWN);
	} else {
		//NSLog(@"cmdKeyUp\n");
		if (rm)
			rm->PushEvent(REVNT_CMDKEYUP);
	}
	if ([event isARepeat]) {
		//NSLog(@"keyDown(repeat): 0x%x\n", c);
		if (rm)
			rm->PushEvent(REVNT_KEYDOWNREPEATE, c);
	}
	else {
		//NSLog(@"keyDown: 0x%x\n", c);
		if (rm)
			rm->PushEvent(REVNT_KEYDOWN, c);
	}
}

- (void)keyUp:(NSEvent *)event
{
	int c = [[event characters] characterAtIndex:0];
	//NSLog(@"keyUp: 0x%x\n", c);
	if (rm)
		rm->PushEvent(REVNT_KEYUP, c);
}

@end
