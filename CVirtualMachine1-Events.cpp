/*
 *  CVirtualMachine1-Events.cpp
 *  NE-VM1
 *
 *  Created by Pavel Dlouhy (LD48) on 4/21/12.
 *  Copyright 2012 @PavelDlouhy. All rights reserved.
 *
 */


#include "CVirtualMachine1.h"


#include <iostream>
#include <assert.h>


using std::cout;
using std::endl;


void CVirtualMachine1::PushEvent(TVirtualEventValueType event, TVirtualEventValueType p1, TVirtualEventValueType p2)
{		
//	if (event != 1) { // exclude tick event
//		cout << "CVirtualMachine1::PushEvent: " << event << ", " << p1 << ", " << p2 << endl;
//	}
#ifdef USE_CUMULATED_TICK
	if (event == VEVNT_TICK) {
		cumulatedTick++;
		if (cumulatedTick >= 60*60) {//@#@#@
			events.push_back(VEVNT_CUMULATEDTICK);
			events.push_back(cumulatedTick);
			cumulatedTick = 0;
			cout << "PushEvent - events.size: " << (unsigned int)events.size() << endl;
		}
	}
	else
#endif // USE_CUMULATED_TICK
	{
#ifdef USE_CUMULATED_TICK
		if (cumulatedTick) {
			events.push_back(VEVNT_CUMULATEDTICK);
			events.push_back(cumulatedTick);
			cumulatedTick = 0;
			cout << "PushEvent - events.size: " << (unsigned int)events.size() << endl;
		}
#endif // USE_CUMULATED_TICK
		events.push_back(event);
		if (p1 != VIRTUAL_EVENT_NOT_USED)
			events.push_back(p1);
		if (p2 != VIRTUAL_EVENT_NOT_USED)
			events.push_back(p2);
		cout << "PushEvent - events.size: " << (unsigned int)events.size() << endl;
	}
}

void CVirtualMachine1::ProcessEvent(std::vector<TVirtualEventValueType>::iterator &it)
{
	TVirtualEventValueType event = *it++;
	switch (event) {
		case VEVNT_TICK:
			if (m[SYS_MODE] == SYS_MODE_EXEC) {
				ExecTick();
			}
			break;
#ifdef USE_CUMULATED_TICK
		case VEVNT_CUMULATEDTICK:
//			if (!primaryVirtualMachine) {
//				cout << "svm: CT add" << endl;
//			}
			cumulatedTickRead = *it++ - cumulatedTickProcessed;
			cumulatedTickProcessed = 0;
			assert(cumulatedTickRead >= 0);
			break;
#endif // USE_CUMULATED_TICK
		case VEVNT_MOUSEDOWN:
			if (m[SYS_MODE] == SYS_MODE_GFXEDIT) {
				//cout << "MouseDown" << endl;
				int addr = VCoor2Addr(mouseX, mouseY);
				editColor = m[addr % VM_MEMORY_SIZE] ? 0 : 1;
				SetVMMem(addr, editColor);
			}
			else {
				assert(0);
			}
			break;
		case VEVNT_MOUSEMOVED:
			if (m[SYS_MODE] == SYS_MODE_GFXEDIT) {
				mouseX = *it++;
				mouseY = *it++;
				//cout << "MouseMoved" << endl;
			}
			else {
				assert(0);
			}
			break;
		case VEVNT_MOUSEDRAGGED:
			if (m[SYS_MODE] == SYS_MODE_GFXEDIT) {
				mouseX = *it++;
				mouseY = *it++;
				//cout << "MouseDragged" << endl;
				int addr = VCoor2Addr(mouseX, mouseY);
				SetVMMem(addr, editColor);
			}
			else {
				assert(0);
			}
			break;
		case VEVNT_KEYDOWN:
		{
			int c = *it++;
			if (m[SYS_MODE] == SYS_MODE_TXTEDIT)
			{
#ifndef VM_FINAL
				if (! (((c >= 'a') && (c <= 'z')) || ((c >= '0') && (c <= '9')) || (c == ' ') || (c == '$')
					   || (c == '-') || (c == ':') || (c == ';') || (c == '@')
					   || (c == '[') || (c == ']') || (c == '_')
					|| (c == 0xd) || (c == 127) ) ) // 127 - backspace
				{
					assert(0);
				}
				else
#endif // VM_FINAL
				{
					if (c == 127) {
						if (tePos != te.begin()) {
							tePos--;
							tePos = te.erase(tePos);
						}
					}
					else
						te.insert(tePos, c);
				}

				//if (c == ) {
					
				//}
			}
			else {
				assert(0);
			}
			break;
		}
/*		case VEVNT_KEYDOWNREPEATE:
		{
			int c = *it++;
			if (m[SYS_MODE] == SYS_MODE_TXTEDIT)
			{
				
			}
			else {
				assert(0);
			}
			break;
		}*/
		case VEVNT_KEYUP:
		{
			//int c = 
			it++;//@#@
			if ((m[SYS_MODE] == SYS_MODE_TXTEDIT) || (m[SYS_MODE] == SYS_MODE_EXEC))
			{
				
			}
			else {
				assert(0);
			}
			break;
		}
		case VEVNT_SETTXTEDITMODE:
			m[SYS_MODE] = SYS_MODE_TXTEDIT;
			break;
		case VEVNT_SETGFXEDITMODE:
			m[SYS_MODE] = SYS_MODE_GFXEDIT;
			break;
		case VEVNT_SETEXECMODE:
			m[SYS_MODE] = SYS_MODE_EXEC;
			ExecReset();
			break;
		case VEVNT_UPKEY_DOWN:
			if (m[SYS_MODE] == SYS_MODE_EXEC) {
				//cout << "SVM_KEYUP_STATE = 1" << endl;
				m[SVM_KEYUP_STATE] = 1;
			}
			else if (m[SYS_MODE] == SYS_MODE_TXTEDIT)
			{
				while (tePos != te.begin()) {
					tePos--;
					if (*tePos == 0xd) {
						break;
					}
				}			
			}
			else if (m[SYS_MODE] == SYS_MODE_GFXEDIT) {
				m[GFX_EDIT_ADDR] -= VSCREEN_DX / m[GFX_EDIT_SCALE] /*/ m[GFX_SPRITE_W] * m[GFX_SPRITE_W]*/ * m[GFX_SPRITE_W];
				if (m[GFX_EDIT_ADDR] < 1024) {
					m[GFX_EDIT_ADDR] = 1024;
				}
			}
			else {
				assert(0);
			}
			break;
		case VEVNT_DOWNKEY_DOWN:
			if (m[SYS_MODE] == SYS_MODE_EXEC) {
				//cout << "SVM_KEYDOWN_STATE = 1" << endl;
				m[SVM_KEYDOWN_STATE] = 1;
			}
			else if (m[SYS_MODE] == SYS_MODE_TXTEDIT)
			{
				while (tePos != te.end()) {
					tePos++;
					if (*tePos == 0xd) {
						break;
					}
				}
			}
			else if (m[SYS_MODE] == SYS_MODE_GFXEDIT) {
				m[GFX_EDIT_ADDR] += VSCREEN_DX / m[GFX_EDIT_SCALE] /*/ m[GFX_SPRITE_W] * m[GFX_SPRITE_W]*/ * m[GFX_SPRITE_W];
			}
			else {
				assert(0);
			}
			break;
		case VEVNT_LEFTKEY_DOWN:
			if (m[SYS_MODE] == SYS_MODE_EXEC) {
				//cout << "SVM_KEYLEFT_STATE = 1" << endl;
				m[SVM_KEYLEFT_STATE] = 1;
			}
			else if (m[SYS_MODE] == SYS_MODE_TXTEDIT)
			{
				if (m[SYS_MODE] == SYS_MODE_TXTEDIT) {
					if (tePos != te.begin()) {
						tePos--;
					}
				}
			}
			else {
				assert(0);
			}
			break;
		case VEVNT_RIGHTKEY_DOWN:
			if (m[SYS_MODE] == SYS_MODE_EXEC) {
				//cout << "SVM_KEYRIGHT_STATE = 1" << endl;
				m[SVM_KEYRIGHT_STATE] = 1;
			}
			else if (m[SYS_MODE] == SYS_MODE_TXTEDIT)
			{
				if (m[SYS_MODE] == SYS_MODE_TXTEDIT) {
					if (tePos != te.end()) {
						tePos++;
					}
				}
			}
			else {
				assert(0);
			}
			break;
		case VEVNT_SPACEKEY_DOWN:
			if (m[SYS_MODE] == SYS_MODE_EXEC) {
				//cout << "SVM_KEYSPACE_STATE = 1" << endl;
				m[SVM_KEYSPACE_STATE] = 1;
			}
			else {
				assert(0);
			}
			break;
		case VEVNT_UPKEY_UP:
			if (m[SYS_MODE] == SYS_MODE_EXEC)
			{
				//cout << "SVM_KEYUP_STATE = 0" << endl;
				m[SVM_KEYUP_STATE] = 0;
			}
			else {
				assert(0);
			}
			break;
		case VEVNT_DOWNKEY_UP:
			if (m[SYS_MODE] == SYS_MODE_EXEC)
			{
				//cout << "SVM_KEYDOWN_STATE = 0" << endl;
				m[SVM_KEYDOWN_STATE] = 0;
			}
			else {
				assert(0);
			}
			break;
		case VEVNT_LEFTKEY_UP:
			if (m[SYS_MODE] == SYS_MODE_EXEC)
			{
				//cout << "SVM_KEYLEFT_STATE = 0" << endl;
				m[SVM_KEYLEFT_STATE] = 0;
			}
			else {
				assert(0);
			}
			break;
		case VEVNT_RIGHTKEY_UP:
			if (m[SYS_MODE] == SYS_MODE_EXEC)
			{
				//cout << "SVM_KEYRIGHT_STATE = 0" << endl;
				m[SVM_KEYRIGHT_STATE] = 0;
			}
			else {
				assert(0);
			}
			break;
		case VEVNT_SPACEKEY_UP:
			if (m[SYS_MODE] == SYS_MODE_EXEC)
			{
				//cout << "SVM_KEYSPACE_STATE = 0" << endl;
				m[SVM_KEYSPACE_STATE] = 0;
			}
			else {
				assert(0);
			}
			break;
		case VEVNT_TXTEDIT_SCROLL_UP:
		{
			int count = m[TXT_EDIT_VERTICALSTEP];
			while ((tePos != te.begin()) && (count)) {
				tePos--;
				if (*tePos == 0xd) {
					count--;
				}
			}
			break;
		}
		case VEVNT_TXTEDIT_SCROLL_DOWN:
		{
			int count = m[TXT_EDIT_VERTICALSTEP];
			while ((tePos != te.end()) && (count)) {
				tePos++;
				if (*tePos == 0xd) {
					count--;
				}
			}
			break;
		}
		default:
			cout << "CVirtualMachine::PushEvent - UNKNOWN event: " << event << endl;
			break;
	}
}