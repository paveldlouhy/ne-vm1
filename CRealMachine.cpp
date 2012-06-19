/*
 *  CRealMachine.cpp
 *  NE-VM1
 *
 *  Created by Pavel Dlouhy (LD48) on 4/21/12.
 *  Copyright 2012 @PavelDlouhy. All rights reserved.
 *
 */


#include "CRealMachine.h"
#include "CVirtualMachine1.h"


#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>
#else
#include <OpenGL/OpenGL.h>
#endif


#include <iostream>


#define RVSCREEN_COEFX		(RSCREEN_DX/VSCREEN_DX)	// RV stands for Real and Virtual
#define RVSCREEN_COEFY		(RSCREEN_DY/VSCREEN_DY)

using std::cout;
using std::endl;


CRealMachine::CRealMachine()
{
	vm = new CVirtualMachine1(true);
	vm->Reset();
	vm->Load();
	
#ifdef VM_FINAL
	editorEnabled = false;
#else
	editorEnabled = true;
#endif // !VM_FINAL
	cmdKeyIsDown = false;
}

CRealMachine::~CRealMachine()
{
	delete vm;
}

//void CRealMachine::Tick()
//{
//	//@#@ Do I need this?
//}

void CRealMachine::PushEvent(TRealEventValueType event, TRealEventValueType p1, TRealEventValueType p2)
{
//	if (event != 1) { // exclude tick event
		cout << "CRealMachine::PushEvent: " << event << ", " << p1 << ", " << p2 << endl;
//	}
	if(vm->IsEnableEditorReq())
	{
		editorEnabled = true;
	}
	switch (event) {
		case REVNT_TICK:
			vm->PushEvent(VEVNT_TICK);
			vm->Tick();
			break;
		case REVNT_MOUSEDOWN:
			if (vm->IsGFXEditMode() && editorEnabled) {
				vm->PushEvent(VEVNT_MOUSEDOWN);
			}
			break;
		case REVNT_MOUSEMOVED:
			if (vm->IsGFXEditMode() && editorEnabled) {
				vm->PushEvent(VEVNT_MOUSEMOVED, p1 / RVSCREEN_COEFX, (RSCREEN_DY - p2) / RVSCREEN_COEFX);
			}
			break;
		case REVNT_MOUSEDRAGGED:
			if (vm->IsGFXEditMode() && editorEnabled) {
				vm->PushEvent(VEVNT_MOUSEDRAGGED, p1 / RVSCREEN_COEFX, (RSCREEN_DY - p2) / RVSCREEN_COEFX);
			}
			break;
		case REVNT_KEYDOWN:
			if (vm->IsExecMode()) {
				if (p1 == 63232) { // up
					vm->PushEvent(VEVNT_UPKEY_DOWN);
					break;
				}
				if (p1 == 63233) { // down
					vm->PushEvent(VEVNT_DOWNKEY_DOWN);
					break;
				}
				if (p1 == 63234) { // left
					vm->PushEvent(VEVNT_LEFTKEY_DOWN);
					break;
				}
				if (p1 == 63235) { // right
					vm->PushEvent(VEVNT_RIGHTKEY_DOWN);
					break;
				}
				if (p1 == ' ') { // space
					vm->PushEvent(VEVNT_SPACEKEY_DOWN);
					break;
				}
			}
			//break; << THIS IS INTENTIONAL!
		case REVNT_KEYDOWNREPEATE:
			//cout << "R-KeyDown: " << p1 << endl;
			if (!cmdKeyIsDown) {
				if (vm->IsGFXEditMode() && editorEnabled) {
					if (p1 == 63232) { // up
						vm->PushEvent(VEVNT_UPKEY_DOWN);
						break;
					}
					if (p1 == 63233) { // down
						vm->PushEvent(VEVNT_DOWNKEY_DOWN);
						break;
					}
				}
				if ((vm->IsTXTEditMode() && editorEnabled)) {
					if (p1 == 63232) { // up
						vm->PushEvent(VEVNT_UPKEY_DOWN);
						break;
					}
					if (p1 == 63233) { // down
						vm->PushEvent(VEVNT_DOWNKEY_DOWN);
						break;
					}
					if (p1 == 63234) { // left
						vm->PushEvent(VEVNT_LEFTKEY_DOWN);
						break;
					}
					if (p1 == 63235) { // right
						vm->PushEvent(VEVNT_RIGHTKEY_DOWN);
						break;
					}
					//note: this condition is here to allow only known characters (in VM1)
					//note: it is for future compatibility of save data
					//note: by future I mean for example tomorrow ;)
					if ( ((p1 >= 'a') && (p1 <= 'z')) || ((p1 >= '0') && (p1 <= '9')) || (p1 == ' ') || (p1 == '$')
						|| (p1 == '-') || (p1 == ':') || (p1 == ';') || (p1 == '@')
						|| (p1 == '[') || (p1 == ']') || (p1 == '_')
						|| (p1 == 0xd) || (p1 == 127) ) // 127 - backspace
					{
						vm->PushEvent(VEVNT_KEYDOWN, p1);
					}
				}
			}
			if (editorEnabled) {
				if (cmdKeyIsDown) {
					if (p1 == '1') {
						vm->PushEvent(VEVNT_SETTXTEDITMODE);
					}
					if (p1 == '2') {
						vm->PushEvent(VEVNT_SETGFXEDITMODE);
					}
					if (p1 == '5') {
						vm->PushEvent(VEVNT_SETEXECMODE);
					}
					if (p1 == 's') {
						vm->Save();
					}
					if (vm->IsTXTEditMode()) {
						if (p1 == 63232) { // up
							vm->PushEvent(VEVNT_TXTEDIT_SCROLL_UP);
							break;
						}
						if (p1 == 63233) { // down
							vm->PushEvent(VEVNT_TXTEDIT_SCROLL_DOWN);
							break;
						}
					}
				}
			}
			break;
/*		case REVNT_KEYDOWNREPEATE:
			if (vm->IsTXTEditMode()) {
				vm->PushEvent(VEVNT_KEYDOWNREPEATE, p1);
			}
			break;*/
		case REVNT_KEYUP:
			//if ((vm->IsTXTEditMode() && editorEnabled) || vm->IsExecMode()) { // @#@ use later
			if (vm->IsExecMode()) { //@#@ useful later
				if (p1 == 63232) { // up
					vm->PushEvent(VEVNT_UPKEY_UP);
					break;
				}
				if (p1 == 63233) { // down
					vm->PushEvent(VEVNT_DOWNKEY_UP);
					break;
				}
				if (p1 == 63234) { // left
					vm->PushEvent(VEVNT_LEFTKEY_UP);
					break;
				}
				if (p1 == 63235) { // right
					vm->PushEvent(VEVNT_RIGHTKEY_UP);
					break;
				}
				if (p1 == ' ') { // space
					vm->PushEvent(VEVNT_SPACEKEY_UP);
					break;
				}
			}
			//}
			break;
		case REVNT_CMDKEYDOWN:
			if (editorEnabled) {
				if (!cmdKeyIsDown)
				{
					//cout << "R cmdKeyDown" << endl;
					cmdKeyIsDown = true;
				}
			}
			break;
		case REVNT_CMDKEYUP:
			if (editorEnabled) {
				if (cmdKeyIsDown)
				{
					//cout << "R cmdKeyUp" << endl;
					cmdKeyIsDown = false;
				}
			}
			break;
		default:
			cout << "CRealMachine::PushEvent - UNKNOWN event: " << event << endl;
			break;
	}
}

void CRealMachine::Render()
{
	glClear(GL_COLOR_BUFFER_BIT);
	
	glBegin(GL_QUADS);
	
	vm->Render();
	
	glEnd();
	
	glFlush();
}
