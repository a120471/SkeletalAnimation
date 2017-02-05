//------------------------------------------------------------------------------
// <copyright file="BodyBasics.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "kinectDataClass.h"
#include "utils.h"
#include <iostream>

KinectData::KinectData()
	: m_pKinectSensor(NULL)
	, m_pCoordinateMapper(NULL)
	, m_pBodyFrameReader(NULL)
{
	curTrackingState = false;
}

KinectData::~KinectData()
{
	//done with body frame reader
	SafeRelease(m_pBodyFrameReader);

	//done with coordinate mapper
	SafeRelease(m_pCoordinateMapper);

	//close the kinect senser
	if (m_pKinectSensor)
		m_pKinectSensor->Close();
	SafeRelease(m_pKinectSensor);
}

HRESULT KinectData::Init()
{
	 HRESULT hr = GetDefaultKinectSensor(&m_pKinectSensor);

	if (FAILED(hr))
		return hr;

	if (m_pKinectSensor)
	{
		if (SUCCEEDED(hr))
			hr = m_pKinectSensor->get_CoordinateMapper(&m_pCoordinateMapper);

		//Initialize the kinect and get the body reader
		IBodyFrameSource* pBodyFrameSource = NULL;

		hr = m_pKinectSensor->Open();

		if (SUCCEEDED(hr))
			hr = m_pKinectSensor->get_BodyFrameSource(&pBodyFrameSource);
		if (SUCCEEDED(hr))
			hr = pBodyFrameSource->OpenReader(&m_pBodyFrameReader);
		SafeRelease(pBodyFrameSource);
	}
	if (!m_pKinectSensor || FAILED(hr))
	{
		std::cerr << "No ready Kinect found!\n";
		return E_FAIL;
	}

	return hr;
}

void KinectData::Update()
{
	if (!m_pBodyFrameReader)
		return;

	IBodyFrame* pBodyFrame = NULL;

	HRESULT hr = m_pBodyFrameReader->AcquireLatestFrame(&pBodyFrame);

	if (SUCCEEDED(hr))
	{
		IBody* ppBodies[BODY_COUNT] = { 0 };

		if (SUCCEEDED(hr))
			hr = pBodyFrame->GetAndRefreshBodyData(_countof(ppBodies), ppBodies);

		if (SUCCEEDED(hr))
			ProcessData(BODY_COUNT, ppBodies);

		for (int i = 0; i < _countof(ppBodies); ++i)
			SafeRelease(ppBodies[i]);
	}
	SafeRelease(pBodyFrame);
}

void KinectData::ProcessData(int nBodyCount, IBody** ppBodies)
{
	curTrackingState = false;
	HRESULT hr;
	if (m_pCoordinateMapper)
	{
		for (int i = 0; i < nBodyCount; ++i)
		{
			IBody* pBody = ppBodies[i];
			if (pBody)
			{
				BOOLEAN bTracked = false;
				hr = pBody->get_IsTracked(&bTracked);

				if (SUCCEEDED(hr) && bTracked)
				{
					//HandState leftHandState = HandState_Unknown;
					//HandState rightHandState = HandState_Unknown;

					//pBody->get_HandLeftState(&leftHandState);
					//pBody->get_HandRightState(&rightHandState);

					hr = pBody->GetJoints(_countof(joints), joints);
					for (int j = 0; j < _countof(joints); j++)
					{
						joints[j].Position.X *= 3.0f;
						joints[j].Position.Y *= 3.0f;
						joints[j].Position.Z *= 3.0f;
					}
					if (SUCCEEDED(hr))
						curTrackingState = true;
				}
			}
		}
	}
}
