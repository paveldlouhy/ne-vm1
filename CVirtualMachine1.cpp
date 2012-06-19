/*
 *  CVirtualMachine1.cpp
 *  NE-VM1
 *
 *  Created by Pavel Dlouhy (LD48) on 4/21/12.
 *  Copyright 2012 @PavelDlouhy. All rights reserved.
 *
 */


#include "CVirtualMachine1.h"


#include <iostream>


#ifdef VM_FINAL
#include "SaveData.h"
#endif // VM_FINAL


using std::cout;
using std::endl;


CVirtualMachine1::CVirtualMachine1(bool primary) : primaryVirtualMachine(primary)
{
	if (primaryVirtualMachine) {
		svm = new CVirtualMachine1(false);
		svm->Reset();
		svm->Load();
	}
}

CVirtualMachine1::~CVirtualMachine1()
{
	if (primaryVirtualMachine) {
		delete svm;
	}
}

void CVirtualMachine1::Reset()
{
	replayMode = false;
	if (!primaryVirtualMachine)
		replayMode = true;
	
	currentEventPos = 0;
#ifdef USE_CUMULATED_TICK
	cumulatedTick = 0;
	cumulatedTickProcessed = 0;
	cumulatedTickRead = 0;
#endif // USE_CUMULATED_TICK
	
	memset(m, 0, sizeof(m));
	m[SYS_MODE] = SYS_MODE_GFXEDIT;
	m[GFX_EDIT_ADDR] = 1024;
	m[GFX_SPRITES_ADDR] = 1024;
	m[GFX_SPRITE_W] = 8;
	m[GFX_FRONTBUFFER_ADDR] = 32768;
	m[GFX_BACKBUFFER_ADDR] = 33768;//valid for text mode 40 x 22.5 ;)
	if (primaryVirtualMachine) {
		m[SVM_RENDER_ENABLED] = 1;
	}
	else {
		m[SVM_RENDER_ENABLED] = 0;
	}
	
	m[GFX_VIEWX1] = 0;
	m[GFX_VIEWY1] = 0;
	m[GFX_VIEWX2] = VSCREEN_DX;
	m[GFX_VIEWY2] = VSCREEN_DY;
		//@#@ TODO:
	m[GFX_SVM_VIEWX1] = VSCREEN_DX / 3 * 2;
	m[GFX_SVM_VIEWY1] = VSCREEN_DY / 3 * 2;
	m[GFX_SVM_VIEWX2] = VSCREEN_DX;
	m[GFX_SVM_VIEWY2] = VSCREEN_DY;
	
	m[GFX_EDIT_SCALE] = 5;
	
	m[TXT_EDIT_VERTICALSTEP] = 8;
	
	m[SYS_SYSTEM_REQ] = 0;
	
	m[SYS_SYSTEM_RESULT] = 0;
	
	m[SVM_SPEED] = 1;
	
	m[SVM_KEYUP_STATE] = 0;
	m[SVM_KEYDOWN_STATE] = 0;
	m[SVM_KEYLEFT_STATE] = 0;
	m[SVM_KEYRIGHT_STATE] = 0;
	m[SVM_KEYSPACE_STATE] = 0;
	
	m[SYS_ENABLEEDITOR_REQ] = 0;
	
	//temporary
	//@#@#@
	//for (int i = 0; i < 64; i++) {
	//	m[m[GFX_SPRITES_ADDR] + 64 + i] = 1;
	//}
	
	mouseX = 0;
	mouseY = 0;
	
	editColor = 1;
	
	te.clear();
	tePos = te.begin();
	
	ExecReset();
}

void CVirtualMachine1::ExecReset()
{
	execPos = te.begin();
	crashPos = te.end();
	
	dataStack = std::stack<TVMMemValueType>();
	callStack = std::stack<TAddrValueType>();
	
	lastJumpLabel = te.end(); 
	lastJumpPos = te.end();
	
	cmpOp1 = 0;
	cmpOp2 = 0;
	
	strPos = te.end();
	
#ifdef USE_JUMP_CACHE
	jumpCache.clear();
#endif // USE_JUMP_CACHE
}

void CVirtualMachine1::Save()
{
	FILE *f = fopen("ne-vm1.sav", "wb");
	if (f) {
		std::vector<TVirtualEventValueType>::iterator it = events.begin();
		unsigned int count = (unsigned int)events.size();
		fwrite(&count, sizeof(count), 1, f);
		while (it != events.end()) {
			TVirtualEventValueType val = *it++;
			fwrite(&val, sizeof(TVirtualEventValueType), 1, f);
		}
		fclose(f);
	}
	f = fopen("../../SaveData.h", "wt");
	if (f) {
		std::vector<TVirtualEventValueType>::iterator it = events.begin();
		int count = 0;
		fprintf(f, "\n\n#ifdef VM_FINAL\n");
		fprintf(f, "int saveData[] = {\n\t");
		while (it != events.end()) {
			fprintf(f, "%d,", *it++);
			count++;
			if (count == 32) {
				count = 0;
				fprintf(f, "\n\t");
			}
		}
		fprintf(f, "\n};\n");
		fprintf(f, "#endif // VM_FINAL\n");
		fclose(f);
	}
}

void CVirtualMachine1::Load()
{
	FILE *f = fopen("ne-vm1.sav", "rb");
	if (f) {
		events.clear();
		unsigned int count;
		fread(&count, sizeof(count), 1, f);
		while (count--) {
			TVirtualEventValueType val;
			fread(&val, sizeof(TVirtualEventValueType), 1, f);
			events.push_back(val);
		}
		fclose(f);
	}
#ifdef VM_FINAL
	else {
		events.clear();
		int count = sizeof(saveData) / sizeof(int);
		//cout << "saveData count: " << count << endl;
		int pos = 0;
		while (count--) {
			events.push_back(saveData[pos++]);
		}
	}
#endif // VM_FINAL		
}

void CVirtualMachine1::Tick()
{
	//temp @#@#@ -divide it later
	//cout << "events.size: " << events.size() << endl;
	
#ifdef USE_CUMULATED_TICK
	if (currentEventPos == events.size()) {
		if (cumulatedTick) {
			cumulatedTickProcessed++;
			if (m[SYS_MODE] == SYS_MODE_EXEC) {
				ExecTick();
			}
		}
	}
	else
#endif // USE_CUMULATED_TICK
	{
		int count = 0;
		while (currentEventPos < events.size()) {
			count++;
			if (count > 2000) {
				break;
			}
#ifdef USE_CUMULATED_TICK
			if (cumulatedTickRead) {
				cumulatedTickRead--;
				if (m[SYS_MODE] == SYS_MODE_EXEC) {
					ExecTick();
				}
			}
#endif // USE_CUMULATED_TICK
			std::vector<TVirtualEventValueType>::iterator it1 = events.begin() + currentEventPos;
			std::vector<TVirtualEventValueType>::iterator it2 = it1;
			ProcessEvent(it2);
			currentEventPos += (unsigned int)std::distance(it1, it2);
			if (!primaryVirtualMachine) {
				break;
			}
		}
	}
	
	if (primaryVirtualMachine) {
		int count = m[SVM_SPEED];
		while (count--) {
			svm->Tick();
		}		
	}
}

bool CVirtualMachine1::IsGFXEditMode()
{
	return (m[SYS_MODE] == SYS_MODE_GFXEDIT);
}

bool CVirtualMachine1::IsTXTEditMode()
{
	return (m[SYS_MODE] == SYS_MODE_TXTEDIT);
}

bool CVirtualMachine1::IsExecMode()
{
	return (m[SYS_MODE] == SYS_MODE_EXEC);
}

bool CVirtualMachine1::IsEnableEditorReq()
{
	return (m[SYS_ENABLEEDITOR_REQ] ? true : false);
}

void CVirtualMachine1::SetVMMem(int addr, int val)
{
	m[addr % VM_MEMORY_SIZE] = val;
}

int CVirtualMachine1::VCoor2Addr(int vX, int vY)
{
	int editX = vX / m[GFX_EDIT_SCALE];
	int editY = vY / m[GFX_EDIT_SCALE];
	int spritesPerX = VSCREEN_DX / m[GFX_EDIT_SCALE] / m[GFX_SPRITE_W];
	int spriteIdx = editY / m[GFX_SPRITE_W] * spritesPerX + editX / m[GFX_SPRITE_W];
	int inSpriteX = editX % m[GFX_SPRITE_W];
	int inSpriteY = editY % m[GFX_SPRITE_W];
	int addr = m[GFX_EDIT_ADDR] + spriteIdx * m[GFX_SPRITE_W] * m[GFX_SPRITE_W] + inSpriteY * m[GFX_SPRITE_W] + inSpriteX;
	return addr;
}
