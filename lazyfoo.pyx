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
        
    def print_program_log(self, program):
        pass
        
    def print_shader_log(self, shader):
        pass
        
    def __dealloc__(self):
        self.free_program()
