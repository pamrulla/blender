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

#include "COM_LightningEffectOperation.h"
#include "BLI_math.h"
#include "DNA_node_types.h"
#include "vector"
#include "algorithm"
#include "random"

LightningEffectOperation::LightningEffectOperation() : NodeOperation()
{
	this->addInputSocket(COM_DT_VALUE);
	this->addInputSocket(COM_DT_VALUE);
	this->addInputSocket(COM_DT_VALUE);
	this->addInputSocket(COM_DT_VALUE);
	this->addInputSocket(COM_DT_VALUE);
	this->addOutputSocket(COM_DT_COLOR);
	this->x1 = NULL;
	this->y1 = NULL;
	this->x2 = NULL;
	this->y2 = NULL;
	this->m_inputMask = NULL;
	this->m_inputValue = NULL;
}

void LightningEffectOperation::initExecution()
{
	int w = this->m_rd->xsch * this->m_rd->size / 100.0f;
	int h = this->m_rd->ysch * this->m_rd->size / 100.0f;
	this->buf = (float **)calloc(w, sizeof(float *));
	int i, j;
	for (i = 0; i < w; i++) {
		this->buf[i] = (float *)calloc(h, sizeof(float));
	}

	this->x1 = this->getInputSocketReader(1);
	this->y1 = this->getInputSocketReader(2);
	this->x2 = this->getInputSocketReader(3);
	this->y2 = this->getInputSocketReader(4);
	this->m_inputMask = this->getInputSocketReader(0);
	
	Point start = { 0, 0 };
	Point end = { 0, 0 };
	
	float o[4];

	this->x1->readSampled(o, 512, 512, PixelSampler::COM_PS_NEAREST);
	start.x = o[0];
	
	this->y1->readSampled(o, 512, 512, PixelSampler::COM_PS_NEAREST);
	start.y = o[0];
	
	this->x2->readSampled(o, 512, 512, PixelSampler::COM_PS_NEAREST);
	end.x = o[0];
	
	this->y2->readSampled(o, 512, 512, PixelSampler::COM_PS_NEAREST);
	end.y = o[0];
	
	vector<Segment> segmentList;

	float iterations = this->m_data->iterations;
	int offsetAmount = this->m_data->offsetAmount * h / 2;

	float lengthScale = this->m_data->length < 0.01 ? 0.6f : this->m_data->length;
	float rr = this->m_data->rotation < 0.01 ? 0.6f : this->m_data->rotation;

	int seed = this->m_data->seed;
	//Main Trunk
	segmentList = this->CreateSegments(start, end, iterations, offsetAmount, seed);

	vector<Segment> branches = this->CreateBranches(segmentList, iterations, offsetAmount, seed);

	for (vector<Segment>::iterator i = branches.begin(); i != branches.end(); ++i)
	{
		segmentList.push_back(*i);
	}

	this->DrawLine(segmentList);
}

#pragma region Segment Generation

vector<Segment> LightningEffectOperation::CreateTwigs(vector<Segment> mainSegments, float iterations, int offsetAmount, int seed)
{
	int twigs = this->m_data->twigs;
	float lengthScale = this->m_data->length < 0.01 ? 0.1f : this->m_data->length;
	float rr = this->m_data->rotation < 0.01 ? 0.1f : this->m_data->rotation;
	vector<Segment> Twigs;

	if (mainSegments.size() == 0)
	{
		return Twigs;
	}

	float maxLength = offsetAmount * 0.75;
	lengthScale = maxLength * lengthScale;

	float maxRotation = 2.5;
	rr = maxRotation * rr;

	std::mt19937 rng(seed);
	//rng.seed(seed);

	while (twigs)
	{

		std::uniform_int_distribution<int> gen(0, mainSegments.size() - 1);
		int idx = gen(rng);


		Point start = mainSegments[idx].start;
		Point end = mainSegments[idx].end;

		Point direction = { end.x - start.x, end.y - start.y };
		std::uniform_int_distribution<int> genr(-rr, rr);
		float randomRotation = genr(rng);
		float rotated[2];
		float directionf[2] = { direction.x, direction.y };
		rotate_v2_v2fl(rotated, directionf, randomRotation);

		if (rotated[0] < 0) { rotated[0] = -1; }
		else { rotated[0] = 1; }
		if (rotated[1] < 0) { rotated[1] = -1; }
		else { rotated[1] = 1; }

		std::uniform_int_distribution<int> genl(maxLength / 2, maxLength);
		float len = genl(rng);

		Point splitEnd = { rotated[0] * len + end.x, rotated[1] * len + end.y };

		vector<Segment> tempSegments = this->CreateSegments(end, splitEnd, iterations, maxLength, seed);

		for (vector<Segment>::iterator i = tempSegments.begin(); i != tempSegments.end(); ++i)
		{
			Twigs.push_back(*i);
		}

		--twigs;
	}

	return Twigs;
}

vector<Segment> LightningEffectOperation::CreateBranches(vector<Segment> mainSegments, float iterations, int offsetAmount, int seed)
{
	int branches = this->m_data->branches;
	float lengthScale = this->m_data->length < 0.01 ? 0.3f : this->m_data->length;
	float rr = this->m_data->rotation < 0.01 ? 0.3f : this->m_data->rotation;
	vector<Segment> Branches;
	vector<Segment> Twigs;

	if (mainSegments.size() == 0)
	{
		return Branches;
	}

	//float maxLength = offsetAmount * 100 / 100;
	//lengthScale = maxLength * lengthScale;
	
	float maxRotation = 5;
	rr = maxRotation * rr;

	std::mt19937 rng(seed);
	//rng.seed(seed);

	while (branches)
	{
		
		std::uniform_int_distribution<int> gen(0, mainSegments.size() - 1);
		int idx = gen(rng);
		

		Point start = mainSegments[idx].start;
		Point end = mainSegments[mainSegments.size() - 1].end;

		Point direction = { end.x - start.x, end.y - start.y };
		std::uniform_int_distribution<int> genr(-rr, rr);
		float randomRotation = genr(rng);
		float rotated[2];
		float directionf[2] = { direction.x, direction.y };
		rotate_v2_v2fl(rotated, directionf, randomRotation);
		
		if (rotated[0] < 0) { rotated[0] = -1; }
		else { rotated[0] = 1; }
		if (rotated[1] < 0) { rotated[1] = -1; }
		else { rotated[1] = 1; }

		float maxLength = len_v2v2(new float[2]{ (float)start.x, (float)start.y }, new float[2]{ (float)end.x, (float)end.y });
		maxLength = maxLength * lengthScale;
		std::uniform_int_distribution<int> genl(maxLength * 0.75, maxLength);
		float len = genl(rng);

		Point splitEnd = { rotated[0] * len + start.x, rotated[1] * len + start.y };
		
		vector<Segment> tempSegments = this->CreateSegments(start, splitEnd, iterations, offsetAmount/2, seed);
		
		for (vector<Segment>::iterator i = tempSegments.begin(); i != tempSegments.end(); ++i)
		{
			i->value = 0.3;
			Branches.push_back(*i);
		}

		vector<Segment> twigs = this->CreateTwigs(tempSegments, iterations, offsetAmount/4, seed);

		for (vector<Segment>::iterator i = twigs.begin(); i != twigs.end(); ++i)
		{
			i->value = 0.1;
			Twigs.push_back(*i);
		}

		--branches;
	}

	for (vector<Segment>::iterator i = Twigs.begin(); i != Twigs.end(); ++i)
	{
		Branches.push_back(*i);
	}

	return Branches;
}

vector<Segment> LightningEffectOperation::CreateSegments(Point start, Point end, float iterations, int offsetAmount, int seed)
{
	vector<Segment> segmentList;
	Segment seg = { start, end, 1 };

	segmentList.push_back(seg);
	std::mt19937 rng(seed);
	//rng.seed(seed);

	while (iterations > 0)
	{
		vector<Segment> tempSegments;

		for (vector<Segment>::iterator i = segmentList.begin(); i != segmentList.end(); i++)
		{
			Point midPoint = { (i->start.x + i->end.x) / 2, (i->start.y + i->end.y) / 2 };

			if (midPoint.x == i->start.x || midPoint.y == i->start.y)
			{
				tempSegments.push_back(*i);
				continue;
			}

			int randomOffset = 0;
			std::uniform_int_distribution<int> gen(-offsetAmount, offsetAmount);
			randomOffset = gen(rng);

			int tx = i->end.x - i->start.x;
			int ty = i->end.y - i->start.y;
			float len = sqrt(tx*tx + ty*ty);
			float nx = tx / len;
			float ny = ty / len;
			float px = ny;
			float py = -nx;
			midPoint.x += px * randomOffset;
			midPoint.y += py * randomOffset;

			Segment seg1 = { i->start, midPoint, i->value };
			Segment seg2 = { midPoint, i->end, i->value };
			tempSegments.push_back(seg1);
			tempSegments.push_back(seg2);
		}
		segmentList.clear();
		segmentList = tempSegments;
		offsetAmount /= 2;
		iterations -= 0.05;
	}

	return segmentList;
}

void LightningEffectOperation::DrawLine(vector<Segment> segmentList)
{
	int w = this->m_rd->xsch * this->m_rd->size / 100.0f;
	int h = this->m_rd->ysch * this->m_rd->size / 100.0f;
	
	//Line pixels
	for (vector<Segment>::iterator i = segmentList.begin(); i != segmentList.end(); i++)
	{
		int Dx = i->end.x - i->start.x;
		int Dy = i->end.y - i->start.y;

		//# Increments
		int Sx = 0;
		if (Dx < 0)
		{
			Sx = -1;
		}
		else if (Dx > 0)
		{
			Sx = 1;
		}
		int Sy = 0;
		if (Dy < 0)
		{
			Sy = -1;
		}
		else if (Dy > 0)
		{
			Sy = 1;
		}
		//# Segment length
		Dx = abs(Dx);
		Dy = abs(Dy);
		int D = max(Dx, Dy);

		//# Initial remainder
		double R = D / 2;

		int X = i->start.x;
		int Y = i->start.y;
		if (Dx > Dy)
		{
			//# Main loop
			for (int I = 0; I<D && X < w && Y < h; I++)
			{
				if (X >= 0 && Y >= 0)
				{
					if (this->buf[X][Y] < i->value)
					{
						this->buf[X][Y] = i->value;
					}
				}
				//# Update (X, Y) and R
				X += Sx; R += Dy; //# Lateral move
				if (R >= Dx)
				{
					Y += Sy;
					R -= Dx; //# Diagonal move
				}
			}
		}
		else
		{
			//# Main loop
			for (int I = 0; I<D && X < w && Y < h; I++)
			{
				if (X >= 0 && Y >= 0)
				{
					if (this->buf[X][Y] < i->value)
					{
						this->buf[X][Y] = i->value;
					}
				}
				//# Update (X, Y) and R
				Y += Sy;
				R += Dx; //# Lateral move
				if (R >= Dy)
				{
					X += Sx;
					R -= Dy; //# Diagonal move
				}
			}
		}
	}
}

#pragma endregion

void LightningEffectOperation::executePixelSampled(float output[4], float x, float y, PixelSampler sampler)
{
	output[0] = this->buf[(int)x][(int)y];
	output[1] = this->buf[(int)x][(int)y];
	output[2] = this->buf[(int)x][(int)y];
	output[3] = this->buf[(int)x][(int)y];
}

void LightningEffectOperation::deinitExecution()
{
	this->x1 = NULL;
	this->y1 = NULL;
	this->x2 = NULL;
	this->y2 = NULL; 
	this->m_inputMask = NULL;
	this->m_inputValue = NULL;
}
