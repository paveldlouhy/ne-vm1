/*
 *  CRealMachine.h
 *  NE-VM1
 *
 *  Created by Pavel Dlouhy (LD48) on 4/21/12.
 *  Copyright 2012 @PavelDlouhy. All rights reserved.
 *
 */


#define RSCREEN_DX			(320*3)
#define RSCREEN_DY			(180*3)


//Real Events - 'R' stands for real 
#define REVNT_TICK					1
#define REVNT_MOUSEDOWN				2
#define REVNT_MOUSEMOVED			3	// y switched here (y0 is on the top now)
#define REVNT_MOUSEDRAGGED			4	// y switched here (y0 is on the top now)
#define REVNT_KEYDOWN				5
#define REVNT_KEYDOWNREPEATE		6
#define REVNT_KEYUP					7
#define REVNT_CMDKEYDOWN			8
#define REVNT_CMDKEYUP				9


typedef unsigned short TRealEventValueType;


class CVirtualMachine1;


class CRealMachine {
public:
	CRealMachine();
	~CRealMachine();
	
//	void Tick();
	void PushEvent(TRealEventValueType event, TRealEventValueType p1 = 0, TRealEventValueType p2 = 0);
	void Render();
private:
	CVirtualMachine1 *vm; // primary
	
	// real machine state
	bool editorEnabled;
	
	// cmd key
	bool cmdKeyIsDown;
};
