gl = require("ffi")

gl.cdef[[


    typedef void             GLvoid;
    typedef char             GLchar;
    typedef unsigned int     GLenum;
    typedef unsigned char    GLboolean;
    typedef unsigned int     GLbitfield;
    typedef int8_t   		 GLbyte;
    typedef short            GLshort;
    typedef int              GLint;
    typedef int              GLsizei;
    typedef uint8_t          GLubyte;
    typedef unsigned short   GLushort;
    typedef unsigned int     GLuint;
    typedef float		     GLfloat;
    typedef float		     GLclampf;
    typedef int32_t          GLfixed;

    typedef signed long int	 GLintptr;
    typedef signed long int	 GLsizeiptr;

    void glClear (GLbitfield mask);
    void glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
	void glViewport (GLint x, GLint y, GLsizei width, GLsizei height);

]]

gl.COLOR_BUFFER_BIT = 0x00004000
