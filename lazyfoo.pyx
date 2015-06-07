cdef class LShaderProgram(object):

    property GLuint  m_program_id:
        def __get__(self):
            return self.m_program_id
    
    def __cinit__(self):
        self.m_program_id = NULL
    
    def load_program(self):
        pass
    
    def free_program(self):
        glDeleteProgram(self.m_program_id)
    
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
