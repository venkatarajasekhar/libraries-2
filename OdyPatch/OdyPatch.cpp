//OdyPatch.cpp  Read and save patch.  No easter egg at present.  There's > 16 functions, to allow for the extra envelope and filter ones.
//Copyright (C) 2015  Paul Soulsby info@soulsbysynths.com
//
//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program.  If not, see <http://www.gnu.org/licenses/>.


#include "OdyPatch.h"
extern void writeMemory(const void* data, void* startAddr, size_t size);
extern void readMemory(void* data, const void* startAddr, size_t size);
// default constructor
OdyPatch::OdyPatch(OdyPatchBase* base)
{
	if(base!=NULL)
	{
		base_ = base;
	}
} //OdyPatch

// default destructor
OdyPatch::~OdyPatch()
{
	if(base_!=NULL)
	{
		delete base_;
	}
} //~OdyPatch

void OdyPatch::setFunctionValue(unsigned char func, unsigned char newValue)
{
	unsigned char i = func>>1;
	funcValue_c_[i] = compressFourBit(funcValue_c_[i],newValue,func%2);
	if(base_!=NULL)
	{
		base_->patchValueChanged(func,newValue);
	}
}

unsigned char OdyPatch::getFunctionValue(unsigned char func)
{
	unsigned char i = func>>1;
	return uncompressFourBit(funcValue_c_[i],func%2);
}
void OdyPatch::setOptionValue(unsigned char func, bool newValue)
{
	unsigned char i = func>>3;
	unsigned char b = func - (8*i);
	bitWrite(optionValue_c_[i],b,newValue);
	if(base_!=NULL)
	{
		base_->patchOptionChanged(func,newValue);
	}
}
bool OdyPatch::getOptionValue(unsigned char func)
{
	unsigned char i = func>>3;
	unsigned char b = func - (8*i);
	return (bool)bitRead(optionValue_c_[i],b);
}
void OdyPatch::setCtrlValue(unsigned char ctrl, unsigned char newValue)
{
	ctrlValue_[ctrl] = newValue;
	if(base_!=NULL)
	{
		base_->patchCtrlChanged(ctrl,ctrlValue_[ctrl]);
	}
}
unsigned char OdyPatch::getCtrlValue(unsigned char ctrl)
{
	return ctrlValue_[ctrl];
}
void OdyPatch::writePatch(unsigned char patchNum)
{
	unsigned int startAddr = (unsigned int)PATCH_SIZE * patchNum;
	unsigned int addr = 0;
	unsigned char i;
	unsigned char data[PATCH_SIZE];
	for(i=0;i<10;++i)
	{
		data[addr] = funcValue_c_[i];
		addr++;
	}
	for(i=0;i<6;++i)
	{
		data[addr] = ctrlValue_[i];
		addr++;
	}
	for(i=0;i<2;++i)   //no need to read/write last 3 func opts
	{
		data[addr] = optionValue_c_[i];
		addr++;
	}
	writeMemory((const void*)data, (void*)startAddr, sizeof(data));
}
void OdyPatch::readPatch(unsigned char patchNum)
{
	unsigned int startAddr = (unsigned int)PATCH_SIZE * patchNum;
	unsigned int addr = 0;
	unsigned char i,j;
	unsigned char data[PATCH_SIZE];
	readMemory((void*)data,(const void*)startAddr, sizeof(data));
	for(i=0;i<10;++i)
	{
		setFunctionValue(i<<1,uncompressFourBit(data[addr],false));
		setFunctionValue((i<<1)+1,uncompressFourBit(data[addr],true));
		addr++;
	}
	for(i=0;i<6;++i)
	{
		setCtrlValue(i,data[addr]);
		addr++;
	}
	for(i=0;i<2;++i)  //no need to read/write last 3 func opts
	{
		for(j=0;j<8;++j)
		{
			setOptionValue((i*8)+j,(bool)bitRead(data[addr],j));
		}
		addr++;
	}
}