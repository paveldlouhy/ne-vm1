/*
 *  CVirtualMachine1.h
 *  NE-VM1
 *
 *  Created by Pavel Dlouhy (LD48) on 4/21/12.
 *  Copyright 2012 @PavelDlouhy. All rights reserved.
 *
 */


#include <vector>
#include <list>
#include <stack>
#include <map>


#define VM_FINAL //@#@#@ set/force exec mode?


#define USE_CUMULATED_TICK
//#define USE_JUMP_CACHE

#define VM_MEMORY_SIZE			65536 // must be power of two


#define VSCREEN_DX				320
#define VSCREEN_DY				180

//Virtual Events - 'V' stands for virtual 
#define VEVNT_TICK					1
#define VEVNT_MOUSEDOWN				2	
#define VEVNT_MOUSEMOVED			3	//p1 p2		// x and y already in VSCREEN coordinates
#define VEVNT_MOUSEDRAGGED			4	//p1 p2		// x and y already in VSCREEN coordinates
#define VEVNT_KEYDOWN				5	//p1		// @#@ filtered?
#define VEVNT_KEYDOWNREPEATE		6	//p1		// @#@ filtered?
#define VEVNT_KEYUP					7	//p1		// @#@ filtered?
#define VEVNT_SETTXTEDITMODE		8
#define VEVNT_SETGFXEDITMODE		9
#define VEVNT_UPKEY_DOWN			10
#define VEVNT_DOWNKEY_DOWN			11
#define VEVNT_LEFTKEY_DOWN			12
#define VEVNT_RIGHTKEY_DOWN			13
#define VEVNT_UPKEY_UP				14
#define VEVNT_DOWNKEY_UP			15
#define VEVNT_LEFTKEY_UP			16
#define VEVNT_RIGHTKEY_UP			17
#define VEVNT_CUMULATEDTICK			18	//p1
#define VEVNT_SETEXECMODE			19
#define VEVNT_TXTEDIT_SCROLL_UP		20
#define VEVNT_TXTEDIT_SCROLL_DOWN	21
#define VEVNT_SPACEKEY_DOWN			22
#define VEVNT_SPACEKEY_UP			23


//sys modes
#define SYS_MODE_TXTEDIT			1
#define SYS_MODE_GFXEDIT			2
#define SYS_MODE_EXEC				3

//Registers
#define SYS_MODE					512
#define GFX_EDIT_ADDR				513
#define GFX_SPRITES_ADDR			514
#define GFX_SPRITE_W				515
#define GFX_FRONTBUFFER_ADDR		516
#define GFX_BACKBUFFER_ADDR			517
#define SVM_RENDER_ENABLED			518
#define GFX_VIEWX1					519	// -1.0f mapped to 0, 1.0f mapped to VSCREEN_DX
#define GFX_VIEWY1					520	// -1.0f mapped to 0, 1.0f mapped to VSCREEN_DY
#define GFX_VIEWX2					521	// -1.0f mapped to 0, 1.0f mapped to VSCREEN_DX
#define GFX_VIEWY2					522	// -1.0f mapped to 0, 1.0f mapped to VSCREEN_DY
#define GFX_SVM_VIEWX1				523	// -1.0f mapped to 0, 1.0f mapped to VSCREEN_DX
#define GFX_SVM_VIEWY1				524	// -1.0f mapped to 0, 1.0f mapped to VSCREEN_DY
#define GFX_SVM_VIEWX2				525	// -1.0f mapped to 0, 1.0f mapped to VSCREEN_DX
#define GFX_SVM_VIEWY2				526	// -1.0f mapped to 0, 1.0f mapped to VSCREEN_DY
#define GFX_EDIT_SCALE				527
#define TXT_EDIT_VERTICALSTEP		528
#define SYS_SYSTEM_REQ				529 // 1-flipandSleep, 2-getchar,
#define SYS_SYSTEM_RESULT			530
#define SVM_SPEED					531
#define SVM_KEYUP_STATE				532
#define SVM_KEYDOWN_STATE			533
#define SVM_KEYLEFT_STATE			534
#define SVM_KEYRIGHT_STATE			535
#define SVM_KEYSPACE_STATE			536
#define SYS_ENABLEEDITOR_REQ		999



#define VIRTUAL_EVENT_NOT_USED		0xffff


typedef unsigned short TVirtualEventValueType;
typedef int TVMMemValueType;
typedef unsigned short TTextEditValueType;
typedef unsigned int TAddrValueType;


class CVirtualMachine1 {
public:
	CVirtualMachine1(bool primary);
	~CVirtualMachine1();
	
	void Reset();
	void ExecReset();
	void Save();
	void Load();
	void Tick();
	bool IsGFXEditMode();
	bool IsTXTEditMode();
	bool IsExecMode();
	bool IsEnableEditorReq();
	
	// common
	
	void SetVMMem(int addr, int val); // to protect real memory
	int VCoor2Addr(int vX, int vY);
	
	// -Events
	
	void PushEvent(TVirtualEventValueType event, TVirtualEventValueType p1 = VIRTUAL_EVENT_NOT_USED,
				   TVirtualEventValueType p2 = VIRTUAL_EVENT_NOT_USED);
	void ProcessEvent(std::vector<TVirtualEventValueType>::iterator &it);
	
	// -Render
	
	void SetColor(float r, float g, float b);
	void RenderQuad(int x, int y, int w, int h);
	void RenderSprite(int x, int y, int idx, bool background);
	void Render(int x1 = -1, int y1 = -1, int x2 = -1, int y2 = -1);
	
	// -Interpret
	void Crash();
	void ExecTick();
	bool IsWhiteSpace();
	bool IsLabelChar(int c);
	bool IsNumberChar();
	void SkipWhiteSpace();
	void SkipLabel();
	void SkipComment();
	void SkipRestOfLine();
	void JumpToLabel();
	int ProcessNumber();
	int ProcessOperand1();
	int ProcessOperand2();
	void ProcessInstruction();
	
private:
	bool primaryVirtualMachine;
	CVirtualMachine1 *svm; // secondary (replay for now)
	
	TVMMemValueType m[VM_MEMORY_SIZE];
	
	// render - precomputed values
	float viewX1;
	float viewY1;
	float viewX2;
	float viewY2;
	float pixDX;
	float pixDY;
	
	// events
	std::vector<TVirtualEventValueType> events;
	int currentEventPos;
#ifdef USE_CUMULATED_TICK
	int cumulatedTick;
	int cumulatedTickProcessed;
	int cumulatedTickRead;
#endif // USE_CUMULATED_TICK
	
	// states
	bool replayMode;
	
	// gfx edit
	int mouseX;
	int mouseY;
	bool editColor;
	
	// txt edit
	std::list<TTextEditValueType> te; // TextEdit
	std::list<TTextEditValueType>::iterator tePos;
	
	// exec
	std::list<TTextEditValueType>::iterator execPos;
	std::list<TTextEditValueType>::iterator crashPos;
	std::stack<TVMMemValueType> dataStack;
	std::stack<TAddrValueType> callStack;
	std::list<TTextEditValueType>::iterator lastJumpLabel;
	std::list<TTextEditValueType>::iterator lastJumpPos;
	TVMMemValueType cmpOp1;
	TVMMemValueType cmpOp2;
	std::list<TTextEditValueType>::iterator strPos;
#ifdef USE_JUMP_CACHE
	//std::map<int, int> jumpCache;
	std::map<std::list<TTextEditValueType>::iterator, std::list<TTextEditValueType>::iterator> jumpCache;
#endif // USE_JUMP_CACHE
};
