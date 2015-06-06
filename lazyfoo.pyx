cdef class LShaderProgram(object):

    GLuint  m_program_id
    
    def load_program(self):
        pass
    
    def free_program(self):
        pass
    
    def bind(self):
        pass
        
    def unbind(self):
        pass
        
    def print_program_log(self, program):
        pass
        
    def print_shader_log(self, shader):
        pass
