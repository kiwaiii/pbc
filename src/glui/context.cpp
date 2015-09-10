//
// nvGlutWidgets
//
//  Adaptor classes to integrate the nvWidgets UI library with the GLUT windowing
// toolkit. The adaptors convert native GLUT UI data to native nvWidgets data. All
// adaptor classes are implemented as in-line code in this header. The adaptor
// defaults to using the standard OpenGL painter implementation.
//
// Author: Ignacio Castano, Samuel Gateau, Evan Hart
// Email: sdkfeedback@nvidia.com
//
// Copyright (c) NVIDIA Corporation. All rights reserved.
////////////////////////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include <glui/context.hpp>
#include <glui/painter.hpp>
#include <cstdlib>
#include <glf/utils.hpp>
namespace glui
{
	//-------------------------------------------------------------------------
	GlfwContext::GlfwContext() :
	UIContext(*(new GLPainter())),
	m_modifiers(0)
	{

	}
	//-------------------------------------------------------------------------
	GlfwContext::~GlfwContext()
	{
		delete getPainter();
	}
	//-------------------------------------------------------------------------
}
