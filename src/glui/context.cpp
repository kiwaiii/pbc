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
	void GlfwContext::Mouse(int _x, int _y, int _button, int _state)
	{
// Is this function usfull???
		glui::Mouse::Button b;
		switch(_button)
		{
//			case GLUT_LEFT_BUTTON	: b = glui::Mouse::LEFT; break;
//			case GLUT_MIDDLE_BUTTON	: b = glui::Mouse::MIDDLE; break;
//			case GLUT_RIGHT_BUTTON	: b = glui::Mouse::RIGHT;	break;
			default 				: return;
		}

//		glui::Mouse::State s = _state==GLUT_DOWN?glui::Mouse::PRESS:glui::Mouse::RELEASE;
//		UIContext::Mouse(b,s,m_modifiers,_x,_y);
	}
}
