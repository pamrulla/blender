/*
* Copyright 2011, Blender Foundation.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software Foundation,
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
* Contributor:
*		Jeroen Bakker
*		Monique Dewanchand
*/

#ifndef _COM_LightningEffectOperation_h
#define _COM_LightningEffectOperation_h
#include "COM_NodeOperation.h"

typedef struct Point {
	int x;
	int y;
}Point;

typedef struct Segment {
	Point start;
	Point end;
	float value;
}Segment;

class LightningEffectOperation : public NodeOperation {
private:
	/**
	* Cached reference to the inputProgram
	*/
	SocketReader *m_inputMask;
	SocketReader *m_inputValue;

	SocketReader *x1;
	SocketReader *y1;
	SocketReader *x2;
	SocketReader *y2;
	const RenderData *m_rd;

	int m_maskType;
	float **buf;

	NodeLightningEffect *m_data;

	void DrawLine(vector<Segment> segmentList);
public:
	LightningEffectOperation();

	/**
	* the inner loop of this program
	*/
	void executePixelSampled(float output[4], float x, float y, PixelSampler sampler);

	/**
	* Initialize the execution
	*/
	void initExecution();

	vector<Segment> CreateTwigs(vector<Segment> mainSegments, float iterations, int offsetAmount, int seed);

	vector<Segment> CreateBranches(vector<Segment> mainSegments, float iterations, int offsetAmount, int seed);

	vector<Segment> CreateSegments(Point start, Point end, float iterations, int offsetAmount, int seed);

	
	/**
	* Deinitialize the execution
	*/
	void deinitExecution();

	void setData(NodeLightningEffect *data) { this->m_data = data; }

	void setMaskType(int maskType) { this->m_maskType = maskType; }

	void setRenderData(const RenderData *rd) { this->m_rd = rd; }
};
#endif
