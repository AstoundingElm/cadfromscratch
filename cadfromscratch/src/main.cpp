#include <stdio.h>
#include <stdlib.h>
#include "defines.h"
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <math.h>

typedef GLXContext (*glXCreateContextAttribsARBProc)
(Display*, GLXFBConfig, GLXContext, bool, const int *);

const char *vertexShaderSource = "#version 330 core\n"
"in vec3 aPos;\n"
"uniform vec3 u1;\n"
"uniform vec3 u2;\n"
"uniform vec3 u3;\n"
"uniform float scaling;\n"
"uniform vec3 lookAt;\n"
"void main()\n"
"{\n"
"	gl_Position = vec4((u1.x*(aPos.x-lookAt.x)+u1.y*(aPos.y-lookAt.y)+u1.z*(aPos.z-lookAt.z))*scaling,(u2.x*(aPos.x-lookAt.x)+u2.y*(aPos.y-lookAt.y)+u2.z*(aPos.z-lookAt.z))*scaling,0,1.0);\n"
"}\0";

const char *fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);\n"
"}\0";


PINLINE void get_normal(f32 p1[3], f32 p2[3], f32 p3[3], f32 normal[3]){
    f32 v1[3]={p2[0]-p1[0],p2[1]-p1[1],p2[2]-p1[2]};
    f32 v2[3]={p3[0]-p1[0],p3[1]-p1[1],p3[2]-p1[2]};
    normal[0]=v1[1]*v2[2]-v1[2]*v2[1];
    normal[1]=v1[2]*v2[0]-v1[0]*v2[2];
    normal[2]=v1[0]*v2[1]-v1[1]*v2[0];
    return;
};

PINLINE void normalize_vector(float vec[3]){
    
    float mag=pow(vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2],.5);
	vec[0]=vec[0]/mag;
	vec[1]=vec[1]/mag;
	vec[2]=vec[2]/mag;
	return;
    
};

PINLINE void draw(int w, int h, f32 lookAt[3], f32 lookFrom[3], int num_nodes, i32 num_triangles, f32 nodes[][3], i32 triangles[][3]){
    
    // view frame axes
    float u3[3]={lookAt[0]-lookFrom[0],lookAt[1]-lookFrom[1],lookAt[2]-lookFrom[2]};
    float u1[3]={-u3[1],u3[0],0};
    float u2[3]={-u3[2]*u3[0],-u3[2]*u3[1],u3[0]*u3[0]+u3[1]*u3[1]};
    
    normalize_vector(u1);
    normalize_vector(u2);
    normalize_vector(u3);
    
    Display * disp = 0;
    Window win = 0;
    disp = XOpenDisplay(0);
    win = XCreateSimpleWindow(disp, DefaultRootWindow(disp), 10, 10, w, h, 0, 0, 0);
    static int attribs[] = {GLX_RENDER_TYPE, GLX_RGBA_BIT, GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT, GLX_DOUBLEBUFFER, true, GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1, GLX_BLUE_SIZE, 1,None};
    int num_fbc = 0;
    GLXFBConfig * fbc = glXChooseFBConfig(disp, DefaultScreen(disp), attribs, &num_fbc);
    
    glXCreateContextAttribsARBProc glxCreateContextAttribsARB = 0;
    
    glxCreateContextAttribsARB = (glXCreateContextAttribsARBProc) glXGetProcAddress((const GLubyte *)"glXCreateContextAttribsARB");
    
    static i32 context[] = {GLX_CONTEXT_MAJOR_VERSION_ARB, 4, GLX_CONTEXT_MINOR_VERSION_ARB, 2, None};
    
    GLXContext ctx = glxCreateContextAttribsARB(disp, fbc[0], NULL, true, context);
    XMapWindow(disp, win);
    XStoreName(disp, win, "View");
    glXMakeCurrent(disp, win, ctx);
    
    u32 vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertex_shader);
    
    
    
    // compile shaders
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    char infoLog[512];
    int success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED %s\n",infoLog);
    }
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    float vertices[3*num_nodes];
    unsigned int indices[3*num_triangles];
    for (int i=0; i<num_nodes; i++) {
        vertices[3*i]=nodes[i][0];
        vertices[3*i+1]=nodes[i][1];
        vertices[3*i+2]=nodes[i][2];
    }
    for (int i=0; i<num_triangles; i++) {
        indices[3*i]=triangles[i][0];
        indices[3*i+1]=triangles[i][1];
        indices[3*i+2]=triangles[i][2];
    }
    
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    GLint u1_loc = glGetUniformLocation(shaderProgram,"u1");
    GLint u2_loc = glGetUniformLocation(shaderProgram,"u2");
    GLint u3_loc = glGetUniformLocation(shaderProgram,"u3");
    GLint scaling_loc = glGetUniformLocation(shaderProgram,"scaling");
    GLint lookAt_loc = glGetUniformLocation(shaderProgram,"lookAt");
    
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    
    glUseProgram(shaderProgram); // out of loop or....?
    
    XEvent event;
	XSelectInput(disp,win,ButtonPressMask|ButtonReleaseMask|Button1MotionMask|Button3MotionMask);
    
	bool left_clicking=0;
	int x_start, y_start;
	float theta_u1, theta_u2;
	float u1_new[3], u2_new[3], u3_new[3], u1_pend[3], u2_pend[3], u3_pend[3], mag;
	float scaling=0.5;
    
	u1_new[0]=u1[0];u1_new[1]=u1[1];u1_new[2]=u1[2];
	u2_new[0]=u2[0];u2_new[1]=u2[1];u2_new[2]=u2[2];
	u3_new[0]=u3[0];u3_new[1]=u3[1];u3_new[2]=u3[2];
    
	while(1){
		glClearColor(0.0f,0.0f,0.0f,1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
        
		glUniform3f(u1_loc,u1_new[0],u1_new[1],u1_new[2]);
		glUniform3f(u2_loc,u2_new[0],u2_new[1],u2_new[2]);
		glUniform3f(u3_loc,u3_new[0],u3_new[1],u3_new[2]);
		glUniform1f(scaling_loc,scaling);
		glUniform3f(lookAt_loc,lookAt[0],lookAt[1],lookAt[2]);
        
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 3*num_triangles, GL_UNSIGNED_INT, 0);
		glXSwapBuffers(disp,win);
        
		// handle Xwin inputs
		XNextEvent(disp,&event);
		if ((event.xbutton.button==Button1) && (event.xbutton.type==ButtonPress)){
			left_clicking=1;
			x_start=event.xbutton.x;
			y_start=event.xbutton.y;
		}
		if ((event.xbutton.button==Button1) && (event.xbutton.type==ButtonRelease)){
			left_clicking=0;
			u1[0]=u1_new[0];u1[1]=u1_new[1];u1[2]=u1_new[2];			
			u2[0]=u2_new[0];u2[1]=u2_new[1];u2[2]=u2_new[2];			
			u3[0]=u3_new[0];u3[1]=u3_new[1];u3[2]=u3_new[2];			
			x_start=event.xbutton.x;
			y_start=event.xbutton.y;
		}
		if ((event.xbutton.button==Button4) && (event.xbutton.type==ButtonPress)){
			scaling=scaling*.90f;
		}
		if ((event.xbutton.button==Button5) && (event.xbutton.type==ButtonPress)){
			scaling=scaling/.90f;
		}
		if (left_clicking && (event.xmotion.type==MotionNotify)){
			theta_u1=(event.xmotion.y-y_start)/((float) h)*15.0f/3.14159f;
			theta_u2=(event.xmotion.x-x_start)/((float) w)*15.0f/3.14159f;
			
			// rotation theta_u2 about u2
			mag=u2[0]*u3[0]+u2[1]*u3[1]+u2[2]*u3[2];
			u3_new[0]=u3[0]*cos(theta_u2)+(u2[1]*u3[2]-u2[2]*u3[1])*sin(theta_u2)+u2[0]*mag*(1-cos(theta_u2));
			u3_new[1]=u3[1]*cos(theta_u2)+(u2[2]*u3[0]-u2[0]*u3[2])*sin(theta_u2)+u2[1]*mag*(1-cos(theta_u2));
			u3_new[2]=u3[2]*cos(theta_u2)+(u2[0]*u3[1]-u2[1]*u3[0])*sin(theta_u2)+u2[2]*mag*(1-cos(theta_u2));
            
			mag=u2[0]*u1[0]+u2[1]*u1[1]+u2[2]*u1[2];
			u1_new[0]=u1[0]*cos(theta_u2)+(u2[1]*u1[2]-u2[2]*u1[1])*sin(theta_u2)+u2[0]*mag*(1-cos(theta_u2));
			u1_new[1]=u1[1]*cos(theta_u2)+(u2[2]*u1[0]-u2[0]*u1[2])*sin(theta_u2)+u2[1]*mag*(1-cos(theta_u2));
			u1_new[2]=u1[2]*cos(theta_u2)+(u2[0]*u1[1]-u2[1]*u1[0])*sin(theta_u2)+u2[2]*mag*(1-cos(theta_u2));
            
			// normalize
			normalize_vector(u1_new);
			normalize_vector(u3_new);
            
			u1_pend[0]=u1_new[0];u1_pend[1]=u1_new[1];u1_pend[2]=u1_new[2];
			u2_pend[0]=u2[0];u2_pend[1]=u2[1];u2_pend[2]=u2[2];
			u3_pend[0]=u3_new[0];u3_pend[1]=u3_new[1];u3_pend[2]=u3_new[2];
			
			// rotation theta_u1 about u1
			mag=u1_pend[0]*u3_pend[0]+u1_pend[1]*u3_pend[1]+u1_pend[2]*u3_pend[2];
			u3_new[0]=u3_pend[0]*cos(theta_u1)+(u1_pend[1]*u3_pend[2]-u1_pend[2]*u3_pend[1])*sin(theta_u1)+u1_pend[0]*mag*(1-cos(theta_u1));
			u3_new[1]=u3_pend[1]*cos(theta_u1)+(u1_pend[2]*u3_pend[0]-u1_pend[0]*u3_pend[2])*sin(theta_u1)+u1_pend[1]*mag*(1-cos(theta_u1));
			u3_new[2]=u3_pend[2]*cos(theta_u1)+(u1_pend[0]*u3_pend[1]-u1_pend[1]*u3_pend[0])*sin(theta_u1)+u1_pend[2]*mag*(1-cos(theta_u1));
            
			mag=u1_pend[0]*u2_pend[0]+u1_pend[1]*u2_pend[1]+u1_pend[2]*u2_pend[2];
			u2_new[0]=u2_pend[0]*cos(theta_u1)+(u1_pend[1]*u2_pend[2]-u1_pend[2]*u2_pend[1])*sin(theta_u1)+u1_pend[0]*mag*(1-cos(theta_u1));
			u2_new[1]=u2_pend[1]*cos(theta_u1)+(u1_pend[2]*u2_pend[0]-u1_pend[0]*u2_pend[2])*sin(theta_u1)+u1_pend[1]*mag*(1-cos(theta_u1));
			u2_new[2]=u2_pend[2]*cos(theta_u1)+(u1_pend[0]*u2_pend[1]-u1_pend[1]*u2_pend[0])*sin(theta_u1)+u1_pend[2]*mag*(1-cos(theta_u1));
			
			// normalize
			normalize_vector(u2_new);
			normalize_vector(u3_new);
		}
	}
	return;
};

PINLINE void write_stl_ascii(i32 num_nodes, i32 num_triangles, f32 nodes[][3], i32 triangles[][3], const char **  filename){
    
    FILE * f = fopen(*filename, "w");
    fprintf(f, "solid \n");
    for(i32 i = 0; i < num_triangles; i++){
        f32 normal[3];
        get_normal(nodes[triangles[i][0]], nodes[triangles[i][1]], nodes[triangles[i][2]], normal );
        fprintf(f, "facet normal %f %f %f\n", normal[0], normal[1], normal[2]);
        fprintf(f, "\touter loop\n");
        for(i32 j = 0; j < 3; j++){
            
            fprintf(f, "\t\tvertex %f %f %f\n", nodes[triangles[i][j]][0],nodes[triangles[i][j]][1],nodes[triangles[i][j]][2]);
        }
        
        fprintf(f, "\tendloop\n");
    }
    fprintf(f, "endsolid \n");
    
    return;
    
};


PINLINE void write_stl_binary(i32 num_nodes, i32 num_triangles, f32 nodes[][3], i32 triangles[][3], const char **  filename){
    
    FILE * f = fopen(*filename, "w");
    
    char header[80] = {0};
    char attribute_byte_count[2] = {0};
    
    
    fwrite(header, 1, 80, f);
    fwrite(&num_triangles, 4, 1, f);
    
    for(i32 i = 0; i < num_triangles; i++){
        f32 normal[3];
        get_normal(nodes[triangles[i][0]], nodes[triangles[i][1]], nodes[triangles[i][2]], normal );
        fwrite(normal, 4, 3, f);
        
        for(i32 j = 0; j < 3; j++){
            fwrite(nodes[triangles[i][j]], 4, 3, f);
            
        }
        
        fwrite(attribute_byte_count, 1, 2, f);
    }
    
    
    return;
    
};

void make_clip(f32 t, f32 w, f32 l, f32 r, f32 g, i32 n){};

i32 main(){
    /*f32 nodes[9][3]={{0,0,0},{1,0,0},{1,1,0},{0,1,0},{0,0,1},{1,0,1},{1,1,1},{0,1,1},{.5,.5,1.5}};
    i32 triangles[14][3]={{0,1,5},{0,5,4},{1,2,6},{1,6,5},{2,3,7},{2,7,6},{3,0,4},{3,4,7},{4,5,8},{5,6,8},{6,7,8},{7,4,8},{0,3,2},{0,2,1}};
    
    const char * filename = "house.stl";
    write_stl_binary(9, 14, nodes, triangles, &filename);
    f32 look_at[3] ={.5, 0, .5};
    f32 look_from[3] = {.5, -5, .5};
    draw(800, 600, look_at, look_from, 9, 14, nodes, triangles);
    */
    make_clip(0.5f, 0.1f, 1.0f, 0.1f,0.025f,10);
    return 0;
    
};