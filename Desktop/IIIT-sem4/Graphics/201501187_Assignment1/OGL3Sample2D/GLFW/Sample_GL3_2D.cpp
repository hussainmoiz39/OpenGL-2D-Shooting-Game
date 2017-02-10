#include<bits/stdc++.h>
#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define pb push_back
#define mp make_pair
#define PI 3.14159265


using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
    double x;
    double y;
    double vx;
    double vy;
    double length;
    double height;
    int isMoveable;
    double begx;
    double begy;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;
    vao->x = 0;
    vao->y = 0;
    vao->vx = 0;
    vao->vy= 0;
    vao->length = 0;
    vao->height = 0;
    vao->isMoveable = 0;
    vao->begx = 0;
    vao->begy = 0;
    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
bool triangle_rot_status = false;
bool rectangle_rot_status = false;


int game_flag=1;
int game_score=0;  //+3 for hitting black,+2 for collect^ red & green in right buckets,-1 for wrong collection of these colors
int game_score_length=1;
double scorepos[5] = {-3.5,-2,-0.5,1,2.5};
double Score[5] = { 0,-0.5,-1,-1.5,-2};


double pos[3][2] = { {-0.5,2.5},{3.3,-2},{3.3,1.5} }; //-2.5,0
double mrotation[3] = {180,30,135 }; 

double color[3][3] = {1,0,0,0,0.5,0,0,0,0};
	
double brick_speed = 0.005;
map<int,int> brickflag;

float timer[10];
map<int,float> begpos;
double gun_rot = 0;
double gun_y = 0;
bool gun_flag=0;

double rbag_x=0;
double gbag_x=0;
bool rbag_flag=0;
bool gbag_flag=0;

/*double x_change=0,y_change=0;
double zoom_camera=1;
double new_mouse_pos_x,new_mouse_pos_y;
int right_mouse_clicked=0;
*/

int Rbaghit=0;
int Rbagmiss=0;
int Gbaghit=0;
int Gbagmiss=0;
int Bbrick=0;


double t;
int Timer=0;
int game_timer=0;
double last_time=0;

double bullet_speed= 0.005;
int no_bullets=0;
map<int,VAO*> bullets;
map<int,double> angle;
map<int,int> bullet_flag;

void createBullet();

/*void mousescroll(GLFWwindow* window,double xoffset,double yoffset)
{
	if(yoffset==-1)
	{
		zoom_camera /= 1.1;
	}
	else if(yoffset==1)
	{
		zoom_camera *= 1.1;
	}

	if(zoom_camera<=1)
	{
		zoom_camera = 1;
	}
	if(zoom_camera>=4)
	{
		zoom_camera = 4;
	}

	if(x_change - 4.0f/zoom_camera < -4)
	{
		x_change = -4 + 4.0f/zoom_camera;
	}
	else if(x_change + 4.0f/zoom_camera > 400)
	{
		x_change =4 - 4.0f/zoom_camera;
	}

	if(y_change - 4.0f/zoom_camera < -4)
	{
		y_change = -4 + 4.0f/zoom_camera;
	}
	else if(y_change + 4.0f/zoom_camera > 4)
	{
		y_change = 4 - 4.0f/zoom_camera;
	}

	Matrices.projection = glm::ortho((double)(-4.0f/zoom_camera + x_change),(double(4.0f/zoom_camera + x_change),(double)(-3.0f/zoom_camera + y_change),(double)(4.0f/zoom_camera + y_change),0.1f,500.0f);
}
			
					
void check_pan()
{
        if(x_change - 4.0f/zoom_camera < -4)
        { 
             x_change= -4 + 4.0f/zoom_camera;
        }
	else if(x_change +  4.0f/zoom_camera > 4)
	{
	      x_change = 4 - 4.0f/zoom_camera ;
	}
	
	if(y_change - 4.0f/zoom_camera < -4)
	{
	      y_change = -4 + 4.0f/zoom_camera;
	}
	else if(y_change + 4.0f/zoom_camera > 4)
	{ 
		y_change = 4 - 4.0f/zoom_camera;
	}
}*/






/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_C:
                rectangle_rot_status = !rectangle_rot_status;
                break;
            case GLFW_KEY_P:
                triangle_rot_status = !triangle_rot_status;
                break;
            case GLFW_KEY_X:
                // do something ..
                break;
	    /*case GLFW_KEY_UP:
		mousescroll(window,0,+1);
		check_pan();
                break;
	    
	    case GLFW_KEY_DOWN:
		mousescroll(window,0,-1);
		check_pan();
	        break;
	    
	    case GLFW_KEY_RIGHT:
		x_change+=0.1;
		check_pan();
	        break;
	    
	    case GLFW_KEY_LEFT:
		x_change-=0.1;
		check_pan();
                break;*/
	 
	    case GLFW_KEY_A:
		gun_rot+=5;
		break;
	    case GLFW_KEY_D:
		gun_rot-=5;
		break;
	    case GLFW_KEY_S:
		gun_y=0.05;
		gun_flag=1;
		break;
	    case GLFW_KEY_F:
		gun_y=-0.05;
		gun_flag=1;
		break;
	    case GLFW_KEY_N:
		brick_speed+=0.005;
		break;
	    case GLFW_KEY_M:
		brick_speed-=0.005;
		if(brick_speed <= 0)//no -ve speed
		{
			brick_speed+=0.005;
		}
		break;
             
            case GLFW_KEY_SPACE:
                t = glfwGetTime();
		if((t-last_time)>0.7)
		{
			//cout<<"diff is: "<<(t-last_time)<<endl;
			//cout<<"t is "<<t<<" ltime is "<<last_time<<endl;
			createBullet();
			last_time = t;
			
		}
		break;


	    default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
       
	    if(glfwGetKey(window,GLFW_KEY_ESCAPE))
	     {
			          quit(window);
	      }
              if(  glfwGetKey(window,GLFW_KEY_LEFT) && glfwGetKey(window,GLFW_KEY_LEFT_CONTROL) )
	      {
	          rbag_x=-0.05;
		  rbag_flag=1;
	       }

	      if(glfwGetKey(window,GLFW_KEY_LEFT) && glfwGetKey(window,GLFW_KEY_LEFT_ALT) )
	      {
	      	  gbag_x = -0.05;
		  gbag_flag=1;
	      }


	      if(glfwGetKey(window,GLFW_KEY_RIGHT) && glfwGetKey(window,GLFW_KEY_LEFT_CONTROL) )
	      {
	          rbag_x=0.05;
		  rbag_flag=1;
	      }
              
	      if( glfwGetKey(window,GLFW_KEY_RIGHT) && glfwGetKey(window,GLFW_KEY_LEFT_ALT) )
	      {
		      gbag_x=0.05;
		      gbag_flag=1;
	      }
    }
}


/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
		default:
			break;
	}
}


int mouse_clicked=0,mouse_release=0,mouse_rc=0,mouse_lc=0;
double mouse_x,mouse_y;
double mouse_x_old=-5500,mouse_y_old=-5500;//initialisation
int mouse_flag=-1;//0->Rbag,1->Gbag,2->cannon updown,3->cannon rotation


/*double zoom_camera=1;
double x_change = 0,y_change=0;
*/

void mouse_Click(int num)
{
	mouse_clicked=1;
	mouse_release=0;
	if(num==0)
	{
		mouse_rc=1;
	}
	if(num==1)
	{
		mouse_lc=1;
	}

}

void mouse_Release()
{
	mouse_release=1;
	mouse_clicked=0;
	


}



/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:    //0
            if (action == GLFW_RELEASE)
	    {
		    mouse_Release();
	    }
	    if (action == GLFW_PRESS)   //0
	    {
		    mouse_Click(0);
	    }
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:   //1
            if (action == GLFW_RELEASE) 
	    {
		    
                mouse_Release();
            }
	    if (action == GLFW_PRESS)    //1
	    {
		    //right_mouse_clicked=1;
		    mouse_Click(1);
	    }
            break;
        default:
            break;
    }
}











/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO *triangle, *rectangle, *square;
VAO *bucket,*gun,*Rbag,*Gbag,*box;
VAO *mirror[3], *brick[20];
VAO *bullet;
VAO *s,*c,*o,*r,*e;
VAO *score,*neg;



void createMinus()
{
	static const GLfloat vertex_buffer_data [] = {
		0,0,0,
		-0.5,0,0
	};
	static const GLfloat color_buffer_data [] = {
		1,0,0,
		1,0,0
	};
	neg  = create3DObject(GL_LINES, 2, vertex_buffer_data, color_buffer_data, GL_LINES);
}




void createScore(int i)
{
	if(i==0)
	{
		static const GLfloat vertex_buffer_data [] = {
			0,0.5,0,
			1,0.5,0,//1
			1,0.5,0,
			1,-0.5,0,//2
			0,-0.5,0,
			1,-0.5,0,//3
			0,0.5,0,
			0,-0.5,0 //4
		};
		static const GLfloat color_buffer_data [] = {
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0
		};
		score  = create3DObject(GL_LINES, 8, vertex_buffer_data, color_buffer_data, GL_LINES);

	}
	else if(i==1)
	{
		static const GLfloat vertex_buffer_data [] = {
			0,0.5,0,
			0,-0.5,0
		};
		static const GLfloat color_buffer_data [] = {
			1,0,0,
			1,0,0
		};
		score  = create3DObject(GL_LINES, 2, vertex_buffer_data, color_buffer_data, GL_LINES);
	}
	else if(i==2)
	{
		static const GLfloat vertex_buffer_data [] = {
			0,0.5,0,
			1,0.5,0,//1
			1,0.5,0,
			1,0,0,//2
			1,0,0,
			0,0,0,//3
			0,0,0,
			0,-0.5,0,//4
			0,-0.5,0,
			1,-0.5,0 //5
		};
		static const GLfloat color_buffer_data [] = {
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0
		};
		score  = create3DObject(GL_LINES, 10, vertex_buffer_data, color_buffer_data, GL_LINES);
	}
	else if(i==3)
	{
		static const GLfloat vertex_buffer_data [] = {
			0,0.5,0,
			1,0.5,0,//1
			1,0.5,0,
			1,-0.5,0,//2
			1,-0.5,0,
			0,-0.5,0,//3
			1,0,0,
			0.5,0,0 //4
		};
		static const GLfloat color_buffer_data [] = {
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0
		};
		score  = create3DObject(GL_LINES, 8, vertex_buffer_data, color_buffer_data, GL_LINES);
	}
	else if(i==4)
	{
		static const GLfloat vertex_buffer_data [] = {
			0,0.5,0,
			0,0,0, //1
			0,0,0,
			1,0,0,//2
			1,0.5,0,
			1,-0.5,0 //3
		};
		static const GLfloat color_buffer_data [] = {
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0
		};
		score  = create3DObject(GL_LINES, 6, vertex_buffer_data, color_buffer_data, GL_LINES);
	}
	else if(i==5)
	{
		static const GLfloat vertex_buffer_data [] = {
			0,0.5,0,
			1,0.5,0,//1
			0,0.5,0,
			0,0,0, //2
			0,0,0,
			1,0,0,//3
			1,0,0,
			1,-0.5,0,//4
			0,-0.5,0,
			1,-0.5,0 //5
		};
		static const GLfloat color_buffer_data [] = {
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0
		};
		score  = create3DObject(GL_LINES, 10, vertex_buffer_data, color_buffer_data, GL_LINES);
	}
	else if(i==6)
	{
		static const GLfloat vertex_buffer_data [] = {
			0,0.5,0,
			1,0.5,0,//1
			0,0.5,0,
			0,-0.5,0,//2
			0,0,0,
			1,0,0,//3
			1,0,0,
			1,-0.5,0,//4
			1,-0.5,0,
			0,-0.5,0//5
		};
		static const GLfloat color_buffer_data [] = {
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0
		};
		score  = create3DObject(GL_LINES, 10, vertex_buffer_data, color_buffer_data, GL_LINES);
	}
	else if(i==7)
	{
		static const GLfloat vertex_buffer_data [] = {
			0,0.5,0,
			1,0.5,0,//1
			1,0.5,0,
			1,-0.5,0//2
		};
		static const GLfloat color_buffer_data [] = {
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0
		};
		score  = create3DObject(GL_LINES, 4, vertex_buffer_data, color_buffer_data, GL_LINES);
	}
	else if(i==8)
	{
		static const GLfloat vertex_buffer_data [] = {
			0,0.5,0,
			0,-0.5,0,//1
			0,0.5,0,
			1,0.5,0, //2
			1,0.5,0,
			1,-0.5,0, //3
			0,0.1,0,
			1,0.1,0,//4
			0,-0.5,0,
			1,-0.5,0 //5
		};
		static const GLfloat color_buffer_data [] = {
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0
		};
		score  = create3DObject(GL_LINES, 10, vertex_buffer_data, color_buffer_data, GL_LINES);
	}
	else if(i==9)
	{
		static const GLfloat vertex_buffer_data [] = {
			0,0.5,0,
			1,0.5,0,//1
			1,0.5,0,
			1,-0.5,0,//2
			0,-0.5,0,
			1,-0.5,0,//3
			0,0.5,0,
			0,0,0,//4
			0,0,0,
			1,0,0//5
		};
		static const GLfloat color_buffer_data [] = {
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0,
			1,0,0
		};
		score  = create3DObject(GL_LINES, 10, vertex_buffer_data, color_buffer_data, GL_LINES);
	}
}







void createS()
{
	static const GLfloat vertex_buffer_data [] = {
		0,0,0,
		0,0.5,0,//1
		0,0.5,0,
		1,0.5,0, //2
		1,0.5,0,
		1,0.4,0,//3
		0,0,0,
		1,0,0,//4
		1,0,0,
		1,-0.5,0,//5
		1,-0.5,0,
		0,-0.5,0,//6
		0,-0.5,0,
		0,-0.4,0 //7
	};
	static const GLfloat color_buffer_data [] = {
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3

	};
	s  = create3DObject(GL_LINES, 14, vertex_buffer_data, color_buffer_data, GL_LINES);
        s->x = -3.5;
}

void createC()
{
	static const GLfloat vertex_buffer_data [] = {
		0,0.5,0,
		1,0.5,0,//1
		//1,0.5,0,
		//1,0.4,0,//2
		0,0.5,0,
		0,-0.5,0,//3
		0,-0.5,0,
		1,-0.5,0,//4
		//1,-0.5,0,
		//1,-0.4,0 //5
	};
	static const GLfloat color_buffer_data [] = {
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		//0.18,0.18,0.3,
		//0.18,0.18,0.3,
		//0.18,0.18,0.3,
		//0.18,0.18,0.3
	};
        c  = create3DObject(GL_LINES, 6, vertex_buffer_data, color_buffer_data, GL_LINES);
	c->x = -2;
}

void createO()
{
	static const GLfloat vertex_buffer_data [] = {
		0,0.5,0,
		1,0.5,0,//1
		0,0.5,0,
		0,-0.5,0,//2
		0,-0.5,0,
		1,-0.5,0,//3
		1,0.5,0,
		1,-0.5,0
	};
	static const GLfloat color_buffer_data [] = {
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3
	};

        o  = create3DObject(GL_LINES, 10, vertex_buffer_data, color_buffer_data, GL_LINES);
	o->x = -0.5;
}

void createR()
{
	static const GLfloat vertex_buffer_data [] = {
		0,0.5,0,
		0,-0.5,0,//1
		0,0.5,0,
		1,0.5,0,//2
		1,0.5,0,
		1,0,0,//3
		0,0,0,
		1,0,0,//4
		0,0,0,
		1,-0.5,0//5
	};
	static const GLfloat color_buffer_data [] = {
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3
	};
        r  = create3DObject(GL_LINES, 10, vertex_buffer_data, color_buffer_data, GL_LINES);
	r->x = 1;
}


void createE()
{
	static const GLfloat vertex_buffer_data [] = {
		0,0.5,0,
		0,-0.5,0,//1
		0,0.5,0,
		1,0.5,0,//2
		0,0,0,
		0.5,0,0,//3
		0,-0.5,0,
		1,-0.5,0,//4
		//1,0.5,0,
		//1,0.4,0,//5
		//1,-0.5,0,
		//1,-0.4,0//6

	};
	static const GLfloat color_buffer_data [] = {
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		/*0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3,
		0.18,0.18,0.3*/

	};
	e  = create3DObject(GL_LINES, 8, vertex_buffer_data, color_buffer_data, GL_LINES);
	e->x = 2.5;
}















// Creates the triangle object used in this sample code
void createBrick (double x,double y,int i,int col)
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
  static const GLfloat vertex_buffer_data [] = {
    -0.1, 0.1,0, // vertex 0
     0.1,  0.1,0, // vertex 1
    -0.1,-0.1,0, // vertex 2

    0.1,0.1,0,
    0.1,-0.1,0,
    -0.1,-0.1,0
  };

  if(col==0)
  {
  	static const GLfloat color_buffer_data [] = {
		1,0,0,
		1,0,0,
		1,0,0,

		1,0,0,
		1,0,0,
		1,0,0
	};


	brick[i] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  	brick[i]->length = 0.2;
  	brick[i]->height = 0.2;
  	brick[i]->x = x ;
  	brick[i]->y = y ;
	brickflag[i]=1;
}
  else if(col==1)
  {
	  static const GLfloat color_buffer_data [] = {
		  0,0.5,0,
		  0,0.5,0,
		  0,0.5,0,

		  0,0.5,0,
		  0,0.5,0,
		  0,0.5,0
	  };

		 brick[i] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  		brick[i]->length = 0.2;
  		brick[i]->height = 0.2;
  		brick[i]->x = x ;
  		brick[i]->y = y ;
 }
  else if(col==2) 
  {
	  static const GLfloat color_buffer_data [] = {
		  0,0,0,
		  0,0,0,
		  0,0,0,

		  0,0,0,
		  0,0,0,
		  0,0,0
	  };

	 brick[i] = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  	 brick[i]->length = 0.2;
  	 brick[i]->height = 0.2;
  	 brick[i]->x = x ;
  	 brick[i]->y = y ;
	 

    }

 
}


void createBullet ()
{
	static const GLfloat vertex_buffer_data [] = {
		-0.2,0.05,0,
		0.2,0.05,0,
		-0.2,-0.05,0,

		0.2,0.05,0,
		0.2,-0.05,0,
		-0.2,-0.05,0
	};
	static const GLfloat color_buffer_data [] = {
		0.30,0.18,0.30,
		0.30,0.18,0.30,
		0.30,0.18,0.30,//1
		0.30,0.18,0.30,
		0.30,0.18,0.30,
		0.30,0.18,0.30,//2
	
	};

        bullet = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
	bullets[no_bullets]=bullet;
	bullet_flag[no_bullets]=1;
	angle[no_bullets]=gun_rot;
	no_bullets++;
}


// Creates the rectangle object used in this sample code
void createRectangle ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -4,3,0, // vertex 1
    4,3,0, // vertex 2
    -4,-3,0, // vertex 3

    4,3,0, // vertex 3
    4,-3,0, // vertex 4
    -4,-3,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,0.5,0, // color 1
    1,0.5,0, // color 2
    1,0.5,0, // color 3

    1,0.5,0, // color 3
    1,0.5,0, // color 4
    1,0.5,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createSquare ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -1,-1,0, // vertex 1
    1,-1,0, // vertex 2
    1, 1,0, // vertex 3

    1, 1,0, // vertex 3
    -1, 1,0, // vertex 4
    -1,-1,0  // vertex 1
  };

  static const GLfloat color_buffer_data [] = {
    1,0,0, // color 1
    0,0,1, // color 2
    0,1,0, // color 3

    0,1,0, // color 3
    0,0,0, // color 4
    1,0,0  // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  square = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createBucket (double k)
{
	static const GLfloat vertex_buffer_data [] = {
		-4,0.1,0,
		4,0.1,0,
		-4,-0.1,0,

		4,0.1,0,
		4,-0.1,0,
		-4,-0.1,0
	};
	static const GLfloat colour_buffer_data [] = {
		1.0,0.5,0,
		1.0,0.5,0,
		1.0,0.5,0,

		1.0,0.5,0,
		1.0,0.5,0,
		1.0,0.5,0
	};

	bucket = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, colour_buffer_data,GL_FILL);
        bucket->x = 0;
	bucket->y = k;
	bucket->vx = 0;
	bucket->vy = 0;
	bucket->length = 8;
	bucket->height = 0.2;
	bucket->isMoveable = 0;
}



void createGun(double x, double y,double vy)
{
	//cout<<"hello moiz"<<endl;
	static const GLfloat vertex_buffer_data [] = {
		-0.2,0.1,0,
		-0.1,0.2,0,
		0.1,0.2,0, //v1

		-0.2,0.1,0,
		0.1,0.2,0,
		0.2,0.1,0, //v2

		-0.2,0.1,0,
		-0.2,-0.1,0,
		0.2,-0.1,0, //v3

		-0.2,0.1,0,
		0.2,0.1,0,
		0.2,-0.1,0,//v4

		-0.2,-0.1,0,
		-0.1,-0.2,0,
		0.2,-0.1,0, //v5

		-0.1,-0.2,0,
		0.1,-0.2,0,
		0.2,-0.1,0,//v6

		0.2,0.05,0,
		0.6,0.05,0,
		0.2,-0.05,0,//v7

		0.2,-0.05,0,
		0.6,0.05,0,
		0.6,-0.05,0
	};

	static const GLfloat color_buffer_data [] = {
		0.64,0.16,0.16,
		0.64,0.16,0.16,
		0.64,0.16,0.16,//v1

		0.64,0.16,0.16,
		0.64,0.16,0.16,
		0.64,0.16,0.16,//v2

		0.64,0.16,0.16,
		0.64,0.16,0.16,
		0.64,0.16,0.16,//v3

		0.64,0.16,0.16,
		0.64,0.16,0.16,
		0.64,0.16,0.16,//v4

		0.64,0.16,0.16,
		0.64,0.16,0.16,
		0.64,0.16,0.16,//v5

		0.64,0.16,0.16,
		0.64,0.16,0.16,
		0.64,0.16,0.16,//v6

		0,0,0,
		0,0,0,
		0,0,0,//v7

		0,0,0,
		0,0,0,
		0,0,0 //v8
	};
        
	gun = create3DObject(GL_TRIANGLES, 24, vertex_buffer_data, color_buffer_data, GL_FILL);
	gun->x = x;
	gun->y = y;
	gun->vx = 0;
	gun->vy = vy;
	gun->length = 0.8;
	gun->height = 0.4;
	gun->isMoveable = 1;

}


void createRbag(double x,double y,double vx,double vy)
{
	static const GLfloat vertex_buffer_data [] = {
		-0.3,0,0,
		-0.25,0.01,0,
		-0.25,-0.01,0, //1
			
		-0.25,0.01,0,
		0.25,0.01,0,
		-0.25,-0.01,0, //2

		-0.25,-0.01,0,
		0.25,0.01,0,
		0.25,-0.01,0, //3

		0.25,0.01,0,
		0.3,0,0,
		0.25,-0.01,0, //4

		-0.3,0,0,
		-0.25,-0.01,0,
		-0.3,-0.8,0, //5

		-0.25,-0.01,0,
		-0.3,-0.8,0,
		0.25,-0.01,0, //6

		-0.3,-0.8,0,
		0.25,-0.01,0,
		0.3,-0.8,0, //7

		0.3,0,0,
		0.25,-0.01,0,
		0.3,-0.8,0 //8
	};

	static const GLfloat color_buffer_data [] = {
		1,1,1,
		1,1,1,
		1,1,1,//1

		1,1,1,
		1,1,1,
		1,1,1,//2

		1,1,1,
		1,1,1,
		1,1,1,//3

		1,1,1,
		1,1,1,
		1,1,1,//4

		1,0,0,
		1,0,0,
		1,0,0,//5

		1,0,0,
		1,0,0,
		1,0,0,//6

		1,0,0,
		1,0,0,
		1,0,0,//7

		1,0,0,
		1,0,0,
		1,0,0//8
	};

	
	Rbag = create3DObject(GL_TRIANGLES, 24, vertex_buffer_data, color_buffer_data, GL_FILL);
	Rbag->x = x;
	Rbag->y = y;
	Rbag->vx = vx;
	Rbag->vy = vy;
	//Rbag->length = length;
	//Rbag->height = height;
	Rbag->isMoveable = 1;

}


void createGbag(double x,double y,double vx,double vy)
{
	static const GLfloat vertex_buffer_data [] = {
		-0.3,0,0,
		-0.25,0.01,0,
		-0.25,-0.01,0, //1
			
		-0.25,0.01,0,
		0.25,0.01,0,
		-0.25,-0.01,0, //2

		-0.25,-0.01,0,
		0.25,0.01,0,
		0.25,-0.01,0, //3

		0.25,0.01,0,
		0.3,0,0,
		0.25,-0.01,0, //4

		-0.3,0,0,
		-0.25,-0.01,0,
		-0.3,-0.8,0, //5

		-0.25,-0.01,0,
		-0.3,-0.8,0,
		0.25,-0.01,0, //6

		-0.3,-0.8,0,
		0.25,-0.01,0,
		0.3,-0.8,0, //7

		0.3,0,0,
		0.25,-0.01,0,
		0.3,-0.8,0 //8
	};

	static const GLfloat color_buffer_data [] = {
		1,1,1,
		1,1,1,
		1,1,1,//1

		1,1,1,
		1,1,1,
		1,1,1,//2

		1,1,1,
		1,1,1,
		1,1,1,//3

		1,1,1,
		1,1,1,
		1,1,1,//4

		0,0.5,0,
		0,0.5,0,
		0,0.5,0,//5

		0,0.5,0,
		0,0.5,0,
		0,0.5,0,//6

		0,0.5,0,
		0,0.5,0,
		0,0.5,0,//7

		0,0.5,0,
		0,0.5,0,
		0,0.5,0//8
	};

	
	Gbag = create3DObject(GL_TRIANGLES, 24, vertex_buffer_data, color_buffer_data, GL_FILL);
	Gbag->x = x;
	Gbag->y = y;
	Gbag->vx = vx;
	Gbag->vy = vy;
	//Gbag->length = length;
	//Gbag->height = height;
	Gbag->isMoveable = 1;
}

void createMirror(int i)
	
{

	 static const GLfloat vertex_buffer_data [] = {
		-0.4,0.05,0,
		0.4,0.05,0,
		-0.4,-0.01,0,

		0.4,0.05,0,
		0.4,-0.01,0,
		-0.4,-0.01,0,

		-0.4,-0.01,0,
		0.4,-0.01,0,
		-0.4,-0.05,0,

		0.4,-0.01,0,
		0.4,-0.05,0,
		-0.4,-0.05,0

	 };

	static const GLfloat color_buffer_data [] = {
		1,1,1,
		1,1,1,
		1,1,1,
		1,1,1,
		1,1,1,
		1,1,1,
		0.59,0.41,0.31,
		0.59,0.41,0.31,
		0.59,0.41,0.31,
		0.59,0.41,0.31,
		0.59,0.41,0.31,
		0.59,0.41,0.31

		
	};

	mirror[i] = create3DObject(GL_TRIANGLES, 12, vertex_buffer_data, color_buffer_data, GL_FILL);
	mirror[i]->x = pos[i][0];
	mirror[i]->y = pos[i][1];
	mirror[i]->length = 0.8;
	mirror[i]->isMoveable = 0;
}






float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw (GLFWwindow* window)
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

  // Load identity to model matrix
  Matrices.model = glm::mat4(1.0f);

  /* Render your scene */

  glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef
  glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
  Matrices.model *= triangleTransform; 
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  //draw3DObject(triangle);

  // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
  // glPopMatrix ();
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateRectangle = glm::translate (glm::vec3(2, 0, 0));        // glTranslatef
  glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangle * rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  //draw3DObject(rectangle);


  glm::mat4 translateSquare = glm::translate (glm::vec3(0, 0, 0));        // glTranslatef

  Matrices.model *= (translateSquare);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  //draw3DObject(square);

   //Timer++;
 
   game_timer = glfwGetTime();
   //cout<<"timer: "<<game_timer<<endl;
   if( game_flag==0 || (game_timer >=90)||game_flag==-1)
   {
	   

	   createRectangle();
	   Matrices.model = glm::mat4(1.0f);
	   glm::mat4 translateRectangle = glm::translate (glm::vec3(0, 0, 0));        // glTranslatef
	   Matrices.model *= (translateRectangle);
	   MVP = VP * Matrices.model;
	   glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
           draw3DObject(rectangle);	
	   
	   
	   createS();
	   Matrices.model = glm::mat4(1.0f);
	   glm::mat4 translateS = glm::translate (glm::vec3(s->x, 1, 0));        // glTranslatef
           Matrices.model *= (translateS);
           MVP = VP * Matrices.model;
	   glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
           draw3DObject(s);	

	   createC();
	   Matrices.model = glm::mat4(1.0f);
	   glm::mat4 translateC= glm::translate (glm::vec3(c->x, 1, 0));        // glTranslatef
           Matrices.model *= (translateC);
           MVP = VP * Matrices.model;
           glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
           draw3DObject(c);	

	   createO();
	   Matrices.model = glm::mat4(1.0f);
	   glm::mat4 translateO= glm::translate (glm::vec3(o->x, 1, 0));        // glTranslatef
	   Matrices.model *= (translateO);
           MVP = VP * Matrices.model;
	   glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
           draw3DObject(o);	
 
           createR();
	   Matrices.model = glm::mat4(1.0f);
           glm::mat4 translateR= glm::translate (glm::vec3(r->x, 1, 0));        // glTranslatef
	   Matrices.model *= (translateR);
           MVP = VP * Matrices.model;
           glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
           draw3DObject(r);	

           createE();
	   Matrices.model = glm::mat4(1.0f);
           glm::mat4 translateE= glm::translate (glm::vec3(e->x, 1, 0));        // glTranslatef
           Matrices.model *= (translateE);
	   MVP = VP * Matrices.model;
           glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
           draw3DObject(e);	

           int num=game_score_length;
	   int sc=game_score;
	  
	   if(game_flag==0)
	   {
	   	cout<<"Red bag's hits    (+2) : "<<Rbaghit<<endl;
	   	cout<<"Red bag's misses  (-1) : "<<Rbagmiss<<endl;
	   	cout<<"Green bag's hits  (+2) : "<<Gbaghit<<endl;
	   	cout<<"Green bag's mises (-1) : "<<Gbagmiss<<endl;
	   	cout<<"Black bricks hit  (+3) : "<<Bbrick<<endl;
		
		game_flag=-1;
	   }
	   int it=0;
	   if(sc<0)
	   {
		   sc*=-1;
	   }
	   while(num!=0)
	   {
		   int k = sc%10;

		   createScore(k);
		   Matrices.model = glm::mat4(1.0f);
		   glm::mat4 translateScore= glm::translate (glm::vec3(Score[it], -1, 0));        // glTranslatef
	           Matrices.model *= (translateScore);
		   MVP = VP * Matrices.model;
		   glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		   draw3DObject(score);	
	           
		   sc = sc/10;
		   num--;
		   it++;
	   }

	   if(game_score < 0)
	   {
		   createMinus();
		   Matrices.model = glm::mat4(1.0f);
		   glm::mat4 translateMinus= glm::translate (glm::vec3(Score[it], -1, 0));        // glTranslatef
		   Matrices.model *= (translateMinus);
                   MVP = VP * Matrices.model;
		   glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
                   draw3DObject(neg);
	   }

   }

 //addedByme-MouseControls
   if(game_flag==1){


   if(mouse_clicked==1)
   {
	   //double mouse_x_cur,mouse_y_cur;
	   glfwGetCursorPos(window,&mouse_x,&mouse_y);
	   double x,y;
	   x= (mouse_x/75) - 4;         //scale the window
	   y= -(((mouse_y)/75) - 4);
	   mouse_x = x;
	   mouse_y = y;
	   //cout<<"x: "<<x<<" y: "<<y<<endl;
      
	   if( fabs(x-gun->x)<0.2 && (y>(gun->y - 0.2)) && (y<(gun->y + 0.2)) ) //inside gun and for moving it
	   {
		   if(mouse_flag==-1)
		   {
			   mouse_flag=2;
		   }
		   if(mouse_flag!=2)
		   {
			   mouse_x_old = -5500;
			   mouse_y_old = -5500;
			   mouse_flag = 2;
		   }

		   if(mouse_x_old == -5500 && mouse_y_old == -5500 )
		   {
			   mouse_x_old = mouse_x;
			   mouse_y_old = mouse_y;
		   }


		   gun->y = gun->y + (mouse_y - mouse_y_old);
		   if( (gun->y > 3.8) || (gun->y < -3.8) )  //going out of window
		   {
			   gun->y -=(mouse_y - mouse_y_old);
		   }
		   //cout<<mouse_y<<"   "<<mouse_y_old<<endl;
		   mouse_x_old = mouse_x;
		   mouse_y_old = mouse_y;
	   }




	   else if( (fabs(x - Rbag->x) <= 0.5) && (y<-3) && (y>=-3.8) )//inside Rbag
	   {
		   if(mouse_flag==-1)//begining
		   {
			   mouse_flag=0;
		   }
		   if(mouse_flag!=0)
		   {
			   mouse_x_old = -5500;
			   mouse_y_old = -5500;
			   mouse_flag=0;
		   }
		   
		  
		   
		   if(mouse_x_old == -5500 && mouse_x_old == -5500)
		   {
			   mouse_x_old = mouse_x;
			   mouse_y_old = mouse_y;
		   }

		   Rbag->x = Rbag->x + (mouse_x - mouse_x_old);
		   if( (Rbag->x > 3.7) || (Rbag->x < -3.7) ) //going out of window
		   {
			   Rbag->x -= (mouse_x - mouse_x_old);
		   }
		   //cout<<(mouse_x - mouse_x_old)<<endl;
		   mouse_x_old = mouse_x;
		   mouse_y_old = mouse_y;

	   }
	   else if( (fabs(x - Gbag->x) <= 0.5) && (y<-3) && (y>=-3.8 ) ) //inside Gbag
	   {
		   if(mouse_flag==-1)//begining
		   {
			   mouse_flag=1;
		   }
		   if(mouse_flag!=1)//discontinued from prev. object
		   {
			   mouse_x_old = -5500;
			   mouse_y_old = -5500;
			   mouse_flag=1;
		   }
		  
		   if(mouse_x_old == -5500 && mouse_x_old == -5500)
		   {
			   mouse_x_old = mouse_x;
			   mouse_y_old = mouse_y;
		   }
   		   Gbag->x = Gbag->x + (mouse_x - mouse_x_old);
		   if( (Gbag->x > 3.7) || (Gbag->x < -3.7) ) //going out of window
		   {
			   Gbag->x -= (mouse_x - mouse_x_old);
		   }
		   //cout<<mouse_x<<"  "<<mouse_x_old<<endl;
                   mouse_x_old = mouse_x;
                   mouse_y_old = mouse_y;
	   }

	   else                //rotating gun
	   {
		   if(mouse_flag==-1)
		   {
			   mouse_flag=3;
		   }
		   if(mouse_flag!=3)
		   {
			   mouse_x_old = -5500;
			   mouse_y_old = -5500;
			   mouse_flag = 3;
		   }
		   if(mouse_x_old == -5500 && mouse_y_old == -5500)
		   {
			   mouse_x_old = mouse_x;
			   mouse_y_old = mouse_y;
		   }
		   double angle = ((atan( (y-(gun->y))/(x-(gun->x))))*180)/M_PI;
		   //cout<<(angle*180)/M_PI<<endl;
		   gun_rot=angle;
	   }



   }
	   
   

  //added by me-Bucket
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateBucket = glm::translate (glm::vec3(0, -3.9, 0));        // glTranslatef
  Matrices.model *= (translateBucket);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(bucket);


  
   //addedbyme- Gun
    if(gun_flag==1)//keyboard input
       {
  	    gun->y+=gun_y;
	    if( (gun->y > 4)||(gun->y < -4) ) //going out of window
	    {
		    gun->y-=gun_y;
	    }
	    gun_flag=0;
        }
        Matrices.model = glm::mat4(1.0f);
        glm::mat4 translategun = glm::translate (glm::vec3(gun->x, gun->y, 0));//-3.8,0.5,0
  	glm::mat4 rotategun = glm::rotate((float)(gun_rot*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  	Matrices.model *= (translategun*rotategun);
  	MVP = VP * Matrices.model;
  	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  	draw3DObject(gun);

  
  //added by me-Rbag
  if(rbag_flag==1)//rbag called from keyboard
  {
	  Rbag->x+=rbag_x;
	  if( (Rbag->x < -4) || (Rbag->x > 4 ) )//going out of window
	  {
		  Rbag->x-=rbag_x;
	  }
	  rbag_flag=0;
  }
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateRbag = glm::translate (glm::vec3(Rbag->x, Rbag->y, 0));
  Matrices.model *= (translateRbag);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(Rbag);
  
  
  
  //added by me-Gbag
  if(gbag_flag==1)//gbag called from keyboard
  {
	  Gbag->x+=gbag_x;
	  if( (Gbag->x < -4) || (Gbag->x > 4 ) )//going out of window
	  {
		  Gbag->x-=gbag_x;
	  }
	  gbag_flag=0;
  }
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateGbag = glm::translate (glm::vec3(Gbag->x, Gbag->y, 0));
  Matrices.model *= (translateGbag);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(Gbag);
  
  
  //added by me - Mirror
  for(int i=0;i<3;i++)
   {
	   Matrices.model = glm::mat4(1.0f);
	   glm::mat4 translateMirror = glm::translate (glm::vec3(pos[i][0], pos[i][1], 0));
           
           glm::mat4 rotateMirror = glm::rotate((float)(mrotation[i]*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	   Matrices.model *= (translateMirror * rotateMirror);
           MVP = VP * Matrices.model;
           glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
           draw3DObject(mirror[i]);
   }

   

 
   //added by me - Bricks
   for(int i=0;i<10;i++)
   {
	   if(brick[i]->y < -3.7)
	   {
		   brick[i]->y = begpos[i];
		   
	   }
	   else
	   {
		   double a = fabs(brick[i]->x - Rbag->x);
		   double b = fabs(brick[i]->x - Gbag->x);

		   double c = 0.4;//0.3+0.1 d1+d2
		  
		
		   if(brick[i]->y <= (Rbag->y + 0.1) && (brick[i]->y > (Rbag->y - 0.1) ) )
		   {
			   if(a < c)
			   {
				   brick[i]->y = begpos[i];
				   if(i%3 == 2)
				   {
					   game_flag = 0;
					   //cout<<"black in red"<<endl;
				   }
				   else if(i%3 == 0)
				   {
					   game_score+=2;
					   Rbaghit++;
				   }
				   else
				   {
					   game_score-=1;
					   Rbagmiss++;
				   }

				  // cout<<game_score<<endl;
				   
			   }
		   }
		   if( (brick[i]->y <= (Gbag->y + 0.1)) && (brick[i]->y > (Gbag->y - 0.1)))
		   {
			   if(b<c)
			   {
				   brick[i]->y = begpos[i];
				   if(i%3 == 2)
				   {
					   game_flag = 0;
					   //cout<<"black in green"<<endl;
				   }
				   else if(i%3 == 1)
				   {
					   game_score+=2;
					   Gbaghit++;
				   }
				   else
				   {
					   game_score-=1;
					   Gbagmiss++;
				   }

				   //cout<<game_score<<endl;
				   
			   }
		   }
	   }

   }
   //gun_score+=sum;
   for(int i=0;i<10;i++)
  {
	 
	  {
		brick[i]->y = brick[i]->y - brick_speed;  
		Matrices.model = glm::mat4(1.0f);
		glm::mat4 translateBrick = glm::translate (glm::vec3(brick[i]->x, brick[i]->y, 0));	
		Matrices.model *= (translateBrick);
		MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(brick[i]);
	  }
  }

   
  //added by me -Bullet
   for(int i=0;i<no_bullets;i++)
   {
        if(bullet_flag[i]==1)//just created @cannon
   	{
		bullets[i]->x = gun->x + 0.8*cos((gun_rot*M_PI)/180);//gun->x is -ve
		bullets[i]->y=  gun->y + 0.8*sin((gun_rot*M_PI)/180);
		angle[i]=gun_rot;
		//last_time=Timer;
		
	
        	Matrices.model = glm::mat4(1.0f);
	
        	glm::mat4 translateBullet = glm::translate (glm::vec3(bullets[i]->x,bullets[i]->y,0));	
        	glm::mat4 rotateBullet = glm::rotate( (float)((gun_rot*M_PI)/180.0f), glm::vec3(0,0,1) ); // rotate about vector (-1,1,1)
	
		Matrices.model *= (translateBullet * rotateBullet);
        	MVP = VP * Matrices.model;
		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
        	draw3DObject(bullets[i]);
		bullet_flag[i]=0;
   	}
   	
	else if( bullet_flag[i]==0)
   	{
		float a = (0.05)*(cos((angle[i]*PI)/180));
		float b = (0.05)*(sin((angle[i]*PI)/180));	
                //cout<<"original "<<a<<" "<<b<<endl;
		bullets[i]->x=bullets[i]->x + a;
		bullets[i]->y=bullets[i]->y + b;
	        //cout<<"final "<<bullets[i]->x<<"  "<<bullets[i]->y<<endl;
        	Matrices.model = glm::mat4(1.0f);
		glm::mat4 translateBullet = glm::translate (glm::vec3(bullets[i]->x, bullets[i]->y, 0));	
        	
        	glm::mat4 rotateBullet = glm::rotate( (float)((angle[i]*M_PI)/180.0f), glm::vec3(0,0,1) ); // rotate about vector (-1,1,1)
		Matrices.model *= (translateBullet*rotateBullet);
        	MVP = VP * Matrices.model;
        	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		draw3DObject(bullets[i]);
   	}
   }
	
   //check collision -: bullet & bricks
   for(int i=0;i<no_bullets;i++)
   {
	   if(bullets[i]->x > 4 || bullets[i]->y < -4 || bullets[i]->x < -4 || bullets[i]->y > 4 )
	   {
		   continue;
	   }
	   else
	   {
	        double c =0.2;float d = 0.2;//d1+d1   
	   	for(int j=0;j<10;j++)
	   	{
			double a = fabs(brick[j]->x - bullets[i]->x);
			double b = fabs(brick[j]->y - bullets[i]->y);
			if( a<0.2 && b<0.2 )
			{
				//cout<<"collided "<<j<<endl;
                                if((j%3)==2)
				{
					game_score+=3;
					Bbrick++;
					//cout<<"j%3 is "<<j%3<<endl;
					//cout<<"score is "<<game_score<<endl;
					if(game_score>=10)
					{
						game_score_length=2;
					}
					if(game_score>=100)
					{
						game_score_length=3;
					}
				}
				brick[j]->y = begpos[j];
				bullets[i]->x = 4;
				bullets[i]->y = 4;
				break;
			}
		}
	   }
   }



//check collision -: bullet & mirrors
   for(int i=0;i<no_bullets;i++)
   {
	   if(bullets[i]->x > 4 || bullets[i]->y < -4 || bullets[i]->x < -4 || bullets[i]->y > 4 )
	   {
		   continue;
	   }
	   else
	   {
	        double c =0.6;double d = 0.11;//d1+d1   
	   	for(int j=0;j<3;j++)
	   	{
			double a = fabs(mirror[j]->x - bullets[i]->x);
			double b = fabs(mirror[j]->y - bullets[i]->y);
			//cout<<"a:0.6 "<<a<<" b:0.1 "<<b<<endl;
			if( a<0.6 && b<0.08 )
			{
				//cout<<"collided ";
				
      				float ang = 2*mrotation[j] - angle[i];
				angle[i]=ang;
				Matrices.model = glm::mat4(1.0f);
	   			glm::mat4 rotateBullet = glm::rotate((float)(ang*M_PI/180.0f), glm::vec3(0,0,1));
	   			Matrices.model *= (rotateBullet);
           			MVP = VP * Matrices.model;
           			glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
           			draw3DObject(bullets[i]);
				break;
			}
		}
	   }
   }




// Increment angles
  float increments = 1;

  //camera_rotation_angle++; // Simulating camera rotation
  triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
  rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;

  }
   
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	// Create the models
	//createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	//createRectangle ();

  	//createSquare();
        createBucket(-3.9);
	createGun(-3.8, 0.5,0);
	createRbag(-2,-3,0,0);
	createGbag(2,-3,0,0);
	for(int i=0;i<3;i++)
	{
		createMirror(i);
	}

	
	createBrick(-2.7,5,0, 0);   //5
	createBrick(-2,4.2,1, 1);   //4.2
	createBrick(-1.5,6,2, 2);   //6 
	createBrick(-1.75,6.5,3, 0); //6.5
	createBrick(-3.1,3.9,4, 1);  //3.9
	createBrick(1.5,7.5,5, 2);   //7.5
	createBrick(2.5,5,6, 0);      //5
	createBrick(0.7,6,7, 1);      //6
	createBrick(2.65,6.5,8, 2);     //6.5
	createBrick(1,4,9, 0);       //4
	
	
	begpos[0]=5;
	begpos[1]=4.2;
	begpos[2]=6;
	begpos[3]=6.5;
	begpos[4]=3.9;
	begpos[5]=7.5;
	begpos[6]=5;
	begpos[7]=6;
	begpos[8]=6.5;
	begpos[9]=4;
	
	




  /* A Vertex Buffer Object (VBO) is a memory buffer in the high speed memory of your video card designed to hold information about vertices.
  A Vertex Array Object (VAO) is an object which contains one or more Vertex Buffer Objects and is designed to store the information for a complete rendered object. 
  In our example this is a diamond consisting of four vertices as well as a color for each vertex.
  */
	
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	
	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (0.74f, 0.84f, 0.84f, 0.0f); // R, G, B, A cyan-(0,1,1)
	glClearDepth (1.0f);//1.0 original

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 600;
	int height = 600;

	for(int i =0;i<10;i++)
	{
		timer[i]=-1;
	}

    GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
        draw(window);

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;
        }
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}


