//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <glf/window.hpp>
#include <glf/utils.hpp>
#include <GLFW/glfw3.h>

//------------------------------------------------------------------------------
// Declarations
//------------------------------------------------------------------------------
bool check();
bool begin();
bool end();
void display();
bool resize(int _w, int _h);

namespace glf
{
	//--------------------------------------------------------------------------
	static void error(int error, const char* description)
	{
		fputs(description, stderr);
	}
	//--------------------------------------------------------------------------
	static void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key >= 0 && key < 255)
			ctx::ui->Keyboard((unsigned char)key);

		if (action == GLFW_PRESS)
		{
			switch(key)
			{
			case GLFW_KEY_U:
				ctx::drawUI = !ctx::drawUI;
				break;
			case GLFW_KEY_H:
				ctx::drawHelpers = !ctx::drawHelpers;
				break;
			case GLFW_KEY_T:
				ctx::drawTimings = !ctx::drawTimings;
				break;
			case GLFW_KEY_W:
				ctx::drawWire = !ctx::drawWire;
				break;
			case 27:
				end();
				exit(0);
				break;
			}
		}
	}
	//--------------------------------------------------------------------------
	static void mouse(GLFWwindow* window, int button, int action, int mods)
	{
		// Get & convert mouse position from sub-pixel position to interger position
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		int x = (int)xpos;
		int y = (int)ypos;

		switch (action)
		{
			case GLFW_PRESS:
			{
				ctx::window.MouseOrigin = ctx::window.MouseCurrent = glm::ivec2(x, y);
				if (!ctx::ui->IsOnFocus())
				{
					switch (button)
					{
						case GLFW_MOUSE_BUTTON_LEFT:
						{
							ctx::ui->Mouse(x,y,glui::Mouse::LEFT,glui::Mouse::PRESS);
							ctx::window.MouseButtonFlags |= glf::MOUSE_BUTTON_LEFT;
							ctx::window.TranlationOrigin = ctx::window.TranlationCurrent;
							ctx::camera->MouseEvent(x,y, Mouse::LEFT, Mouse::PRESS);
						}
						break;
						case GLFW_MOUSE_BUTTON_MIDDLE:
						{
							ctx::window.MouseButtonFlags |= glf::MOUSE_BUTTON_MIDDLE;
							ctx::camera->MouseEvent(x,y, Mouse::MIDDLE, Mouse::PRESS);
						}
						break;
						case GLFW_MOUSE_BUTTON_RIGHT:
						{
							ctx::window.MouseButtonFlags |= glf::MOUSE_BUTTON_RIGHT;
							ctx::window.RotationOrigin = ctx::window.RotationCurrent;
							ctx::camera->MouseEvent(x,y, Mouse::RIGHT, Mouse::PRESS);
						}
						break;
						// Special case for handling Linux wheel implementation of FreeGLUT
						case 3:
						{
							ctx::camera->MouseEvent(x,y, Mouse::SCROLL_UP, Mouse::PRESS);
						}
						break;
						case 4:
						{
							ctx::camera->MouseEvent(x,y, Mouse::SCROLL_DOWN, Mouse::PRESS);
						}
						break;
					}
				}
			}
			break;
			case GLFW_RELEASE:
			{
				switch (button)
				{
					case GLFW_MOUSE_BUTTON_LEFT:
					{
						ctx::ui->Mouse(x,y,glui::Mouse::LEFT,glui::Mouse::RELEASE);
						ctx::window.TranlationOrigin += (ctx::window.MouseCurrent - ctx::window.MouseOrigin) / 10.f;
						ctx::window.MouseButtonFlags &= ~glf::MOUSE_BUTTON_LEFT;
						ctx::camera->MouseEvent(x,y, Mouse::LEFT, Mouse::RELEASE);
					}
					break;
					case GLFW_MOUSE_BUTTON_MIDDLE:
					{
						ctx::window.MouseButtonFlags &= ~glf::MOUSE_BUTTON_MIDDLE;
						ctx::camera->MouseEvent(x,y, Mouse::MIDDLE, Mouse::RELEASE);
					}
					break;
					case GLFW_MOUSE_BUTTON_RIGHT:
					{
						ctx::window.RotationOrigin += ctx::window.MouseCurrent - ctx::window.MouseOrigin;
						ctx::window.MouseButtonFlags &= ~glf::MOUSE_BUTTON_RIGHT;
						ctx::camera->MouseEvent(x,y, Mouse::RIGHT, Mouse::RELEASE);
					}
					break;
				}
			}
			break;
		}
	}
	//--------------------------------------------------------------------------
	static void reshape(GLFWwindow* window, int w, int h)
	{
		ctx::ui->Reshape(w,h);

		ctx::window.Size = glm::ivec2(w, h);
		glViewport(0,0,w,h);

		resize(w, h);
	}
	//--------------------------------------------------------------------------
	static void motion(GLFWwindow* window, double xpos, double ypos)
	{
		// Convert from sub-pixel position to integer position 
		int x = (int)xpos;
		int y = (int)ypos;

		ctx::ui->Move(x,y);

		ctx::window.MouseCurrent = glm::ivec2(x, y);
		ctx::window.TranlationCurrent = ctx::window.MouseButtonFlags & glf::MOUSE_BUTTON_LEFT ? ctx::window.TranlationOrigin + (ctx::window.MouseCurrent - ctx::window.MouseOrigin) / 10.f : ctx::window.TranlationOrigin;
		ctx::window.RotationCurrent = ctx::window.MouseButtonFlags & glf::MOUSE_BUTTON_RIGHT ? ctx::window.RotationOrigin + (ctx::window.MouseCurrent - ctx::window.MouseOrigin) : ctx::window.RotationOrigin;

		if(!ctx::ui->IsOnFocus())
			ctx::camera->MoveEvent(x,y);
	}
	//--------------------------------------------------------------------------
	static void wheel( int b, int dir, int x, int y)
	{
		if (dir > 0)
			ctx::camera->MouseEvent(x,y, Mouse::SCROLL_UP, Mouse::PRESS);
		else
			ctx::camera->MouseEvent(x,y, Mouse::SCROLL_DOWN, Mouse::PRESS);
	}
	//--------------------------------------------------------------------------
	void init()
	{
		#ifdef WIN32
		glewExperimental = GL_TRUE;
		glewInit();
		glGetError();
		#endif

		// Retrieve informations about uniform and atomic counter
		GLint value;
		glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS,&value);					Info("GL_MAX_VERTEX_UNIFORM_BLOCKS                  : %d",value);
		glGetIntegerv(GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS,&value);			Info("GL_MAX_TESS_CONTROL_UNIFORM_BLOCKS            : %d",value);
		glGetIntegerv(GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS,&value);		Info("GL_MAX_TESS_EVALUATION_UNIFORM_BLOCKS         : %d",value);
		glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_BLOCKS,&value);				Info("GL_MAX_GEOMETRY_UNIFORM_BLOCKS                : %d",value);
		glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS,&value);				Info("GL_MAX_FRAGMENT_UNIFORM_BLOCKS                : %d",value);
		glGetIntegerv(GL_MAX_COMBINED_UNIFORM_BLOCKS,&value);				Info("GL_MAX_COMBINED_UNIFORM_BLOCKS                : %d",value);
		glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS,&value);				Info("GL_MAX_UNIFORM_BUFFER_BINDINGS                : %d",value);

		glGetIntegerv(GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS,&value);			Info("GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS          : %d",value); 
		glGetIntegerv(GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS,&value);	Info("GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS    : %d",value); 
		glGetIntegerv(GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS,&value);Info("GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS : %d",value); 
		glGetIntegerv(GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS,&value);		Info("GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS        : %d",value); 
		glGetIntegerv(GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS,&value);		Info("GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS        : %d",value);
		glGetIntegerv(GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS,&value);		Info("GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS         : %d",value);
	}
	//--------------------------------------------------------------------------
	bool Run(	int argc, 
				char* argv[], 
				const glm::ivec2 & size, 
				int major, 
				int minor)
	{
		glfwSetErrorCallback(error);

		// GLFW initialization
		if (!glfwInit())
			exit(-1);

		glfwSetErrorCallback(error);

		// Configure the window
		glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
		glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
		glfwWindowHint(GLFW_DECORATED, GL_TRUE);
		glfwWindowHint(GLFW_FOCUSED, GL_TRUE);

		glfwWindowHint(GLFW_RED_BITS, 8);
		glfwWindowHint(GLFW_GREEN_BITS, 8);
		glfwWindowHint(GLFW_BLUE_BITS, 8);
		glfwWindowHint(GLFW_ALPHA_BITS, 8);
		glfwWindowHint(GLFW_DEPTH_BITS, 32);

		glfwWindowHint(GLFW_SAMPLES, 0); // (no msaa)
		glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);

		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		GLFWwindow* window = glfwCreateWindow(size.x, size.y, "PBC", nullptr, nullptr);
		if (!window)
		{
			glfwTerminate();
			glf::Error("Window creation failed");
			exit(-1);
		}
		glfwSetWindowPos(window, 64, 64);
		glfwMakeContextCurrent(window);
    	glfwSwapInterval(1);
    	glfwSetKeyCallback(window, keyboard);
		glfwSetWindowSizeCallback(window, reshape);
		glfwSetCursorPosCallback(window, motion);
		glfwSetMouseButtonCallback(window, mouse);

		glGetError();
		glf::init();
		ctx::ui = new glui::GlfwContext();
		ctx::ui->Initialize(size.x, size.y);

		// Render loop
		bool validRun = false;
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		if (begin())
		{
			validRun = true;
			while (!glfwWindowShouldClose(window))
			{
				display();
				glfwSwapBuffers(window);
				glGetError(); // 'glutSwapBuffers' generates an here with OpenGL 3 > core profile ... :/

				glfwPollEvents();
			}
			end();
		}

    	glfwDestroyWindow(window);

		delete ctx::ui;
		return validRun;

//		glutInitWindowSize(Size.x, Size.y);
//		glutInitWindowPosition(64, 64);
//		glutInit(&argc, argv);
//		glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);// | GLUT_MULTISAMPLE);

//		int WindowHandle = glutCreateWindow(argv[0]);
//		glewExperimental = GL_TRUE; 
//		glewInit();
//		glutDestroyWindow(WindowHandle);

//		glutInitContextVersion(Major, Minor);
//		if(glf::Version(Major, Minor) >= 410)
		{
//			glutInitContextProfile(GLUT_CORE_PROFILE);
			//glutInitContextProfile(GLUT_CORE_PROFILE | GLUT_DEBUG);
			//glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE | GLUT_DEBUG);
			//glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);
		}

//		glutCreateWindow(argv[0]);
//		glGetError();
//		glf::init();
//		ctx::ui = new glui::GlutContext();
//		ctx::ui->Initialize(size.x, size.y);
/*
		// 
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		if(begin())
		{
			glutDisplayFunc(	display); 
			glutReshapeFunc(	glf::reshape);
			glutMouseFunc(		glf::mouse);
			glutMotionFunc(		glf::motion);
			glutPassiveMotionFunc( glf::passivemotion);
			glutKeyboardFunc(	glf::keyboard);
			glutIdleFunc(		glf::idle);
			glutCloseFunc(		glf::close);
			glutMouseWheelFunc( glf::wheel );
			glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
			glutMainLoop();

			return true;
		}

		delete ctx::ui;
		return false;*/
	}

}//namespace glf
