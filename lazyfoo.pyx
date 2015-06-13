cdef class LShaderProgram(object):

    cdef GLuint  m_program_id:
    
    def __cinit__(self):
        self.m_program_id = NULL
    
    cdef void free_program(LShaderProgram self):
        glDeleteProgram(self.m_program_id)
    
    def load_program(self):
        pass
    
    def bind(self):
        ### use shader ###
        glUseProgram(self.m_program_id)
        
        ### check for error ###
        cdef GLenum error = glGetError()
        
        if error != GL_NO_ERROR:
            print "Error binding shader! {!s}\n".format(gluErrorString(error))
            self.print_program_log(self.m_program_id)
            return False

        return True
        
    def unbind(self):
        glUseProgram(NULL)
        
    def print_program_log(self, GLuint program):

        if glIsProgram(program):
            ### program log length ###
            cdef int info_log_length = 0
            cdef int max_length = info_log_length
            
            ### get info string length ###
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &max_length)
            
            ### allocate string ###
            cdef char *info_log = char[max_length]
            
            ### get info log ###
            glGetProgramInfoLog(program, max_length, &info_log_length, info_log)
            
            if info_log_length > 0:
                print "{!s}\n".format(info_log)
        else:
            print "name {!d} is not a program\n".format(program)
            
        
    def print_shader_log(self, GLuint shader):
        if glIsShader(shader):
            ### shader log length ###
            cdef int info_log_length = 0
            cdef max_length = info_log_length

            ### get info string length ###
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &max_length)

            ### allocate string ###
            cdef char *info_log = char[max_length]

            ### get info log ###
            glGetShaderInfoLog(shader, max_length, &info_log_length, info_log)
            
            if info_log_length > 0:
                print "{!s}\n".format(info_log)
        else:
            print "Name {!d} is not a shader\n".format(shader)
        
    def __dealloc__(self):
        self.free_program()


cdef class LPlainPolygonProgram2D(LShaderProgram):

    def load_program(self):
        ### success flag ###
        GLint program_success = GL_TRUE
        
        ### generate program ###
        self.m_program_id = glCreateProgram()
        
        ### create vertex shader ###
        GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER)
        
        ### get vertex source ###
        const GLchar *vertex_shader_source[] = """
                                                void main()
                                                {
                                                    gl_Position = gl_Vertex;
                                                }
                                                """

        ### set vertex source ###
        glShaderSource(vertex_shader, 1, vertex_shader_source, NULL)
        
        ### compile vertex source ###
        glCompileShader(vertex_shader)
        
        ### check vertex shader for errors ###
        GLint v_shader_compiled = GL_FALSE
        glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &v_shader_compiled)
        
        if v_shader_compiled != GL_TRUE:
            print "Unable to compile vertex shader {!d}!\n".format(vertex_shader)
            self.print_shader_log(vertex_shader)
            return False
            
        ### attach vertex shader to program ###
        glAttachShader(self.m_program_id, vertex_shader)
        
        ### create fragment shader ###
        GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER)
        
        ### get fragment source ###
        const GLchar *fragment_shader_source[] = """
                                                 void main{}
                                                 {
                                                    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
                                                 }
                                                 """

        ### set fragment source ###
        glShaderSource(fragment_shader, 1, fragment_shader_source, NULL)
        
        ### compile fragment source ###
        glCompileShader(fragment_shader)
        
        ### check fragment shader for errors ###
        GLint f_shader_compiled = GL_FALSE
        glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &f_shader_compiled)
        
        if f_shader_compiled != GL_TRUE:
            print "Unable to compile fragment shader {!d}!\n".format(fragment_shader)
            self.print_shader_log(fragment_shader)
            return False
            
        ### attach fragment shader to program ###
        glAttachShader(self.m_program_id, fragment_shader)
        
        ### link program ###
        glLinkProgram(self.m_program_id)
        
        ### check for errors ###
        glGetProgramiv(self.m_program_id, GL_LINK_STATUS, &program_success)
        
        if program_success != GL_TRUE:
            print "Error lining program {!d}!\n".format(self.m_program_id)
            self.program_log(self.m_program_id)
            return False
            
        return True


cdef bool init_GL():
    ### initialize GLEW? ###
    cdef GLenum glew_error = glewInit()
    
    if glew_error != GLEW_OK:
        print "Error initializing GLEW! %s\n".format(glewGetErrorString(glew_error))
        return False
    elif !GLEW_VERSION_2_1: # make sure OpenGL 3.1 is supported
        print "OpenGL 2.1. not supported!\n"
        return False

    ### set the viewport ###
    glViewport(0.f, 0.f, SCREEN_WIDTH, SCREEN_HEIGHT)
    
    ### initialize projection matrix ###
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    glOrtho(0.0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0, 1.0, -1.0)
    
    ### initialize modelview matrix ###
    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()
    
    ### initialize modelview matrix ###
    glMatrixMode(GL_MODELVIEW)
    glLoadIdentity()
    
    ### initialize clear color ###
    glClearColor(0.f, 0.f, 0.f, 1.f)
    
    ### enable texturing ###
    glEnable(GL_TEXTURE_2D)
    
    ###start blending ###
    glEnable(GL_BLEND)
    glDisable(GL_DEPTH_TEST)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
    
    ### check for error ###
    cdef GLenum error glGetError()
    
    if error != GL_NO_ERROR:
        print "Error initializing OpenGL! %s\n".format(gluErrorString(error))
        return False
        
    ### initialize DevIL and DevILU ###
    iLInit()
    iluInit()
    ilClearColor(255, 255, 255, 000)
    
    ### check for error ###
    cdef ILenum il_error = ilGetError()
    
    if il_error != IL_NO_ERROR:
        print "Error initializing DevIL! %s\n".format(iluErrorString(il_error))
        return False
    
    return True


cdef bool load_GP(LShaderProgram polygon):
    ### load basic shader program ###
    if !polygon.load_program():
        print "Unable to load basic shader!\n"
        return False
    
    polygon.bind()
    return True


cdef void render():
    ### clear color buffer ###
    glClear(GL_COLOR_BUFFER_BIT)
    
    ### reset transformations ###
    glLoadIdentity()
    
    ### solid red quad in the center ###
    glTranslatef(SCREEN_WIDTH/2.f, SCREEN_HEIGHT/2.f, 0.f)

    glBegin(GL_QUADS)
    glColor3f(0.f, 1.f, 1.f)
    glVertex2f(-50.f, -50.f);
    glVertex2f(50.f, -50.f);
    glVertex2f(50.f, 50.f);
    glVertex2f(-50.f, 50.f);
    glEnd()
    
    ### update screen ###
    glutSwapBuffers()
    

if __name__ == "__main__":
    if !init_GL():
        print "Unable to initialize graphics library!\n"
        return 1
    elif !load_GP():
        print "Unable to load shader programs!\n"
        return 1
    elif !load_media():
        print "Unable to load media!\n"
        return 2
