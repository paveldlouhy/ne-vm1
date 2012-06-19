/*
 *  CVirtualMachine1-Interpret.cpp
 *  NE-VM1
 *
 *  Created by Pavel Dlouhy (LD48) on 4/21/12.
 *  Copyright 2012 @PavelDlouhy. All rights reserved.
 *
 */


#include "CVirtualMachine1.h"


#include <iostream>
#include <assert.h>
#include <algorithm>


using std::cout;
using std::endl;


void CVirtualMachine1::Crash()
{
	if (crashPos == te.end()) {
		cout << "CRASH! " << (unsigned int)std::distance(crashPos, execPos) << endl;
		crashPos = execPos;
		
	}
	else {
		cout << "crash!" << endl;
	}
}

void CVirtualMachine1::ExecTick()
{
	if (!primaryVirtualMachine) {
		cout << "svm: ExecTick" << endl;
	}
	int count = 0;
	while ((crashPos == te.end()) && (count++ < 10000)) {
		ProcessInstruction();
		if (m[SYS_SYSTEM_REQ] != 0) {
			if (m[SYS_SYSTEM_REQ] == 2) {
				if (strPos != te.end()) {
					m[SYS_SYSTEM_RESULT] = *strPos++;
				}
			}
			else if (m[SYS_SYSTEM_REQ] == 1) {
				std::swap(m[GFX_FRONTBUFFER_ADDR], m[GFX_BACKBUFFER_ADDR]);
				static int c = 0;
				c++;
				if (c >= 60) {
					c = 0;
					cout << "ExecTick-swapBuffers-count: " << count << endl;
				}
			}
			if (m[SYS_SYSTEM_REQ] == 1) {
				m[SYS_SYSTEM_REQ] = 0;
				return;
			}
			m[SYS_SYSTEM_REQ] = 0;
		}
	}	
}

bool CVirtualMachine1::IsWhiteSpace()
{
	if ((*execPos == ' ') || (*execPos == 0xd))
		return true;
	else
		return false;
}

bool CVirtualMachine1::IsLabelChar(int c)
{
	if ( ((c >= 'a') && (c <= 'z')) || ((c >= '0') && (c <= '9')) || (c == '_') || (c == '@') )
		return true;
	else
		return false;
}

bool CVirtualMachine1::IsNumberChar()
{
	if ( ((*execPos >= '0') && (*execPos <= '9')) || (*execPos == '-') )
		return true;
	else
		return false;
}

void CVirtualMachine1::SkipWhiteSpace()
{
	while (execPos != te.end()) {
		if (!IsWhiteSpace()) {
			return;
		}
		execPos++;
	}
}

void CVirtualMachine1::SkipLabel()
{
	while (execPos != te.end()) {
		if (*execPos == 0xd) {
			SkipWhiteSpace();
			return;
		}
		execPos++;
	}	
}

void CVirtualMachine1::SkipComment()
{
	while (execPos != te.end()) {
		if (*execPos == 0xd) {
			SkipWhiteSpace();
			return;
		}
		execPos++;
	}
}

void CVirtualMachine1::SkipRestOfLine()
{
	while (execPos != te.end()) {
		if (*execPos == 0xd) {
			SkipWhiteSpace();
			return;
		}
		execPos++;
	}
}

void CVirtualMachine1::JumpToLabel()
{
#ifdef USE_JUMP_CACHE
	//std::map<int, int>::iterator cachedJumpPos; 
	std::map<std::list<TTextEditValueType>::iterator, std::list<TTextEditValueType>::iterator>::iterator cachedJumpPos;
	//cachedJumpPos = jumpCache.find(std::distance(te.begin(), execPos));
	cachedJumpPos = jumpCache.find(execPos);
	if (cachedJumpPos != jumpCache.end()) {
		//execPos = *cachedJumpPos;
		cout << "found in CACHE!" << endl;
		//execPos = te.begin();
		//std::advance(execPos, cachedJumpPos->second);
		execPos = cachedJumpPos->second;
	}
#else // !USE_JUMP_CACHE
	if (execPos == lastJumpLabel) {
		execPos = lastJumpPos;
		return;
	}
#endif // !USE_JUMP_CACHE
	else {
		std::list<TTextEditValueType>::iterator it = execPos;
		std::vector<TTextEditValueType> label;
		label.push_back(':');
		while (it != te.end()) {
			if (!IsLabelChar(*it)) {
				break;
			}
			label.push_back(*it);
			it++;
		}
		//'it' now points to the end of label
		it = std::search(te.begin(), te.end(), label.begin(), label.end());
		if (it == te.end()) {
			Crash();
		}
		else {
#ifdef USE_JUMP_CACHE
			//jumpCache[std::distance(te.begin(), execPos)] = std::distance(te.begin(), it);
			
#else // !USE_JUMP_CACHE
			lastJumpLabel = execPos;
			lastJumpPos = it;
#endif // !USE_JUMP_CACHE
			execPos = it;
		}
	}
}

//note: first char is expected to be '-' or digit!
int CVirtualMachine1::ProcessNumber()
{
	int num = 0;
	int sign = 1;
	if (*execPos == '-') {
		execPos++;
		sign = -1;
		if (!isdigit(*execPos)) {
			Crash();
			return 0;
		}
	}
	while (execPos != te.end()) {
		if (!isdigit(*execPos)) {
			return num * sign;
		}
		num = num * 10 + (*execPos - '0');
		execPos++;
	}
	Crash();
	return 0;
}

//note: return: addr
//note: caller is responsible to test return value
//note: zaro is error
int CVirtualMachine1::ProcessOperand1()
{
	if (!IsNumberChar()) {
		if (*execPos == '[') {
			execPos++;
			int addr = ProcessOperand2();
			if (*execPos == ']') {
				execPos++;
				SkipWhiteSpace();
				return addr;
			}
			else
				Crash();
		}
	}
	else
		Crash();
	return 0;
}

//note: return: value
int CVirtualMachine1::ProcessOperand2()
{
	if (IsNumberChar()) {
		int val = ProcessNumber();
		SkipWhiteSpace();
		return val;
	}
	else if (*execPos == '[') {
		execPos++;
		int addr = ProcessOperand2();
		if (addr == 0) {
			Crash();
			return 0;
		}
		if (*execPos == ']') {
			execPos++;
			SkipWhiteSpace();
			return m[addr % VM_MEMORY_SIZE];
		}
		else
			Crash();
	}
	else
		Crash();
	return 0;
}

void CVirtualMachine1::ProcessInstruction()
{
	if (*execPos == 'm') {
		execPos++;
		if (*execPos == 'o') {
			execPos++;
			SkipWhiteSpace();
			int addr = ProcessOperand1();
			if (addr == 0)
			{
				Crash();
				return;
			}
			SkipWhiteSpace();
			int val = ProcessOperand2();
			if (!primaryVirtualMachine) {
				cout << "svm: mo [" << addr << "] " << val << endl;
			}
			m[addr % VM_MEMORY_SIZE] = val;
			return;
		}
		else if (*execPos == 'u') {
			execPos++;
			SkipWhiteSpace();
			int addr = ProcessOperand1();
			if (addr == 0)
			{
				Crash();
				return;
			}
			SkipWhiteSpace();
			int val = ProcessOperand2();
			//cout << "mu [" << addr << "] " << val << " ;before: " << m[addr % VM_MEMORY_SIZE] << endl;
			m[addr % VM_MEMORY_SIZE] *= val;
			return;
		}
		else
			Crash();
	}
	if (*execPos == 'a') {
		execPos++;
		if (*execPos == 'd') {
			execPos++;
			SkipWhiteSpace();
			int addr = ProcessOperand1();
			if (addr == 0)
			{
				Crash();
				return;
			}
			SkipWhiteSpace();
			int val = ProcessOperand2();
			//cout << "ad [" << addr << "] " << val << " ;before: " << m[addr % VM_MEMORY_SIZE] << endl;
			m[addr % VM_MEMORY_SIZE] += val;
			return;
		}
		else
			Crash();
	}
	else if (*execPos == 'j') {
		execPos++;
		if (*execPos == 'n')
		{
			execPos++;
			SkipWhiteSpace();
			if (cmpOp1 != cmpOp2) {
				JumpToLabel();
				return;
			}
		}
		else if (*execPos == ' ')
		{
			SkipWhiteSpace();
			JumpToLabel();
			return;
		}
		else if (*execPos == 'e')
		{
			execPos++;
			SkipWhiteSpace();
			if (cmpOp1 == cmpOp2) {
				JumpToLabel();
				return;
			}
		}
		else if (*execPos == 'g')
		{
			execPos++;
			SkipWhiteSpace();
			if (cmpOp1 > cmpOp2) {
				JumpToLabel();
				return;
			}
		}
		else if (*execPos == 'l')
		{
			execPos++;
			SkipWhiteSpace();
			if (cmpOp1 < cmpOp2) {
				JumpToLabel();
				return;
			}
		}
		else
			Crash();
		//not jump - skip label
		SkipLabel();
		return;
	}
	else if (*execPos == 'p') {
		execPos++;
		if (*execPos == 'u') {
			execPos++;
			SkipWhiteSpace();
			int addr = ProcessOperand1();
			if (addr == 0)
			{
				Crash();
				return;
			}
			dataStack.push(addr);
			dataStack.push(m[addr]);
			SkipWhiteSpace();
			return;
		}
		else
			Crash();
	}
	else if (*execPos == 'r') {
		execPos++;
		if (*execPos == 'e') {
			execPos++;
			//automatic pop (up to marker)
			TVMMemValueType addr;
			TVMMemValueType val;
			if (dataStack.empty()) {
				Crash();
				return;
			}
			do {
				val = dataStack.top();
				dataStack.pop();
				addr = dataStack.top();
				dataStack.pop();
				if (addr != -1) {
					m[addr % VM_MEMORY_SIZE] = val;
				}
			} while ((addr != -1) && (val != -1));
			//
			TAddrValueType retPos = callStack.top();
			callStack.pop();
			execPos = te.begin();
			std::advance(execPos, retPos);
			SkipWhiteSpace();
			SkipLabel();
			return;
		}
		else
			Crash();
	}
	else if (*execPos == 'c') {
		execPos++;
		if (*execPos == 'a') {
			execPos++;
			callStack.push((unsigned int)std::distance(te.begin(),execPos));
			dataStack.push(-1);//marker
			dataStack.push(-1);
			SkipWhiteSpace();
			//if (*execPos == 0xd) {
			//	Crash();
			//	return;
			//}
			JumpToLabel();
			return;
		}
		else if (*execPos == 'm') {
			execPos++;
			SkipWhiteSpace();
			int addr = ProcessOperand1();
			if (addr == 0)
			{
				Crash();
				return;
			}
			SkipWhiteSpace();
			int val = ProcessOperand2();
			cmpOp1 = m[addr % VM_MEMORY_SIZE];
			cmpOp2 = val;
			//cout << "cm " << cmpOp1 << " " << cmpOp2 << endl;
			return;
		}
		else
			Crash();
	}
	else if (*execPos == ';') {
		SkipComment();
		//cout << "EP: " << *execPos << endl;
	}
	else if (*execPos == ':') {
		SkipLabel();
		//cout << "EP: " << *execPos << endl;
	}
	else if (*execPos == '$') {
		execPos++;
		SkipWhiteSpace();
		strPos = execPos;
		SkipRestOfLine();
	}
	else {
		cout << "*execPos: " << *execPos << endl;
		Crash();
	}
}
