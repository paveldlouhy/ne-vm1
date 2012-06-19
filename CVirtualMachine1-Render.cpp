/*
 *  CVirtualMachine1-Render.cpp
 *  NE-VM1
 *
 *  Created by Pavel Dlouhy (LD48) on 4/21/12.
 *  Copyright 2012 @PavelDlouhy. All rights reserved.
 *
 */


#include "CVirtualMachine1.h"


#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <gl/gl.h>
#else
#include <OpenGL/OpenGL.h>
#endif


#include <iostream>


using std::cout;
using std::endl;


void CVirtualMachine1::SetColor(float r, float g, float b)
{
	glColor3f(r, g, b);
}

void CVirtualMachine1::RenderQuad(int x, int y, int w, int h)
{
	float qx1 = viewX1 + x * pixDX;
	float qy1 = viewY1 - y * pixDY;
	float qx2 = qx1 + w * pixDX;
	float qy2 = qy1 - h * pixDY;

	glVertex2f(qx1, qy1);
	glVertex2f(qx2, qy1);
	glVertex2f(qx2, qy2);
	glVertex2f(qx1, qy2);
}

void CVirtualMachine1::RenderSprite(int x, int y, int idx, bool background) // "PutPixel" sprite function ;)
{
	int sx, sy;
	if (background) {
		if (idx == 2) // crash marker
			SetColor(0.2f, 0.0f, 0.0f);
		else
			SetColor(0.0f, 0.2f, 0.0f);
		RenderQuad(x, y, m[GFX_SPRITE_W], m[GFX_SPRITE_W]);	
	}
	if (idx == 2) // crash marker
		SetColor(1.0f, 0.0f, 0.0f);
	else
		SetColor(0.0f, 1.0f, 0.0f);
	int addr = m[GFX_SPRITES_ADDR] + idx * m[GFX_SPRITE_W] * m[GFX_SPRITE_W];
	for (sy = 0; sy < m[GFX_SPRITE_W]; sy++) {
		for (sx = 0; sx < m[GFX_SPRITE_W]; sx++) {
			if (m[addr++] != 0) {
				RenderQuad(x + sx, y + sy, 1, 1);
			}
		}
	}
}

void CVirtualMachine1::Render(int x1, int y1, int x2, int y2)
{
	//DON'T FORGET: top left corner is -1.0f, 1.0f
	
	//precalculate
	
	if (x1 != -1) {
		viewX1 = -1.0f + ((float)x1) / (VSCREEN_DX / 2);
		viewY1 = 1.0f - ((float)y1) / (VSCREEN_DY / 2);
		viewX2 = -1.0f + ((float)x2) / (VSCREEN_DX / 2);
		viewY2 = 1.0f - ((float)y2) / (VSCREEN_DY / 2);
	}
	else {
		viewX1 = -1.0f + ((float)m[GFX_VIEWX1]) / (VSCREEN_DX / 2);
		viewY1 = 1.0f - ((float)m[GFX_VIEWY1]) / (VSCREEN_DY / 2);
		viewX2 = -1.0f + ((float)m[GFX_VIEWX2]) / (VSCREEN_DX / 2);
		viewY2 = 1.0f - ((float)m[GFX_VIEWY2]) / (VSCREEN_DY / 2);
	}
	pixDX = (viewX2 - viewX1) / VSCREEN_DX;
	pixDY = (viewY1 - viewY2) / VSCREEN_DY;
	
	if (!primaryVirtualMachine)
	{
		SetColor(0.0f, 0.0f, 0.0f);
		RenderQuad(0, 0, VSCREEN_DX, VSCREEN_DY);
	}
	
	if (m[SYS_MODE] == SYS_MODE_GFXEDIT) {
		int x, y;
		SetColor(0.0f, 0.8f, 0.0f);
		for (y = 0; y < VSCREEN_DY; y++) {
			for (x = 0; x < VSCREEN_DX; x++) {
				if (m[VCoor2Addr(x, y) % VM_MEMORY_SIZE])
					RenderQuad(x, y, 1, 1);
			}
		}
		
		for (x = 0; x <= VSCREEN_DX / m[GFX_EDIT_SCALE]; x++) {
			if (x % m[GFX_SPRITE_W] == 0)
				SetColor(0.0f, 1.0f, 0.0f);
			else
				SetColor(0.0f, 0.7f, 0.0f);
			RenderQuad(x * m[GFX_EDIT_SCALE], 0, 1, VSCREEN_DY);
		}
		
		for (y = 0; y <= VSCREEN_DY / m[GFX_EDIT_SCALE]; y++) {
			if (y % m[GFX_SPRITE_W] == 0)
				SetColor(0.5f, 1.0f, 0.5f);
			else
				SetColor(0.0f, 0.7f, 0.0f);
			RenderQuad(0, y * m[GFX_EDIT_SCALE], VSCREEN_DX, 1);
		}
		
		//editPixel on cursor
		SetColor(0.5f, 1.0f, 0.5f);
		RenderQuad(mouseX / m[GFX_EDIT_SCALE] * m[GFX_EDIT_SCALE], mouseY / m[GFX_EDIT_SCALE] * m[GFX_EDIT_SCALE],
				   m[GFX_EDIT_SCALE], m[GFX_EDIT_SCALE]);
		
		//cross
		SetColor(1.0f, 1.0f, 1.0f);
		RenderQuad(mouseX - 2, mouseY, 5, 1); 
		RenderQuad(mouseX, mouseY - 2, 1, 5); 
	}
	
	//textedit -> textmode
	if (m[SYS_MODE] == SYS_MODE_TXTEDIT)
	{
		std::list<TTextEditValueType>::iterator it = tePos;
		// handle big text (more pages)
		int count = VSCREEN_DY / m[GFX_SPRITE_W] / 4 * 3;
		while ((it != te.begin()) && (count)) {
			it--;
			if (*it == 0xd) {
				count--;
			}

		}
		if (it != te.begin()) {
			it++;
		}
		
		int bufferAddr = m[GFX_FRONTBUFFER_ADDR];
		int pos = 0;
		int maxPos = (VSCREEN_DX / m[GFX_SPRITE_W]) * (VSCREEN_DY / m[GFX_SPRITE_W]);
		while ((pos < maxPos)) {
			if (it == tePos) {
				m[bufferAddr + pos++] = 1;
				if (pos >= maxPos)
					break;
			}
			if ((crashPos != te.end()) && (it == crashPos)) {
				m[bufferAddr + pos++] = 2;
				if (pos >= maxPos)
					break;
			}
			if (it == te.end()) {
				while (pos < maxPos) {
					m[bufferAddr + pos++] = 0;
				}
				break;
			}
			int c = *it++;
			//fprintf(stderr, "c: 0x%x\n", c);
			if (c == 0xd) {
				do {
					m[bufferAddr + pos++] = 0;
				} while (pos % (VSCREEN_DX / m[GFX_SPRITE_W]) != 0);
				continue;
			}
			m[bufferAddr + pos++] = c;
		}
	}
	
	//render text mode
	if ((m[SYS_MODE] == SYS_MODE_TXTEDIT) || (m[SYS_MODE] == SYS_MODE_EXEC)) {
		int addr = m[GFX_FRONTBUFFER_ADDR];
		int x, y;
		for (y = 0; y < VSCREEN_DY / m[GFX_SPRITE_W]; y++) {
			for (x = 0; x < VSCREEN_DX / m[GFX_SPRITE_W]; x++) {
				if (m[addr] != 0) {
					RenderSprite(x * m[GFX_SPRITE_W], y * m[GFX_SPRITE_W], m[addr], (m[SYS_MODE] == SYS_MODE_TXTEDIT));
				}
				addr++;
			}
		}
		if ((m[SYS_MODE] == SYS_MODE_EXEC) && (crashPos != te.end())) {
			RenderSprite(0, 0, 2, true);
		}
	}
	
	if (primaryVirtualMachine && (m[SVM_RENDER_ENABLED]))
		svm->Render(m[GFX_SVM_VIEWX1], m[GFX_SVM_VIEWY1], m[GFX_SVM_VIEWX2], m[GFX_SVM_VIEWY2]);
	
	if((primaryVirtualMachine) && (currentEventPos < (events.size() / 100 * 99)))
	{
		int rX = VSCREEN_DX / 20;
		int rY = VSCREEN_DY / 20;
		unsigned int div = (unsigned int)(events.size() / 100 * 99);
		if (div == 0) div = 1;
		SetColor(0.5f, 0.5f, 0.5f);
		RenderQuad(rX * 1, rY * 8 , rX * 18, rY * 4);
		SetColor(1.0f, 1.0f, 1.0f);
		RenderQuad(rX * 2, rY * 9 , (rX * 16) * currentEventPos / div, rY * 2);
	}
}

