//------------------------------------------------------------------------------
// <copyright file="BodyBasics.h" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#pragma once

#include "kinect.h"
#include <glm/glm.hpp>

class KinectData
{
public:
	bool curTrackingState;
	Joint joints[JointType_Count];

	KinectData();
	~KinectData();

	HRESULT Init();

	void Update();

private:
	// Current Kinect
	IKinectSensor*          m_pKinectSensor;
	ICoordinateMapper*      m_pCoordinateMapper;

	// Body reader
	IBodyFrameReader*       m_pBodyFrameReader;
	
	void ProcessData(int nBodyCount, IBody** ppBodies);
};
