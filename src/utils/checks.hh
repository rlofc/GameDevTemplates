#ifndef GDT_CHECKS_HEADER_INCLUDED
#define GDT_CHECKS_HEADER_INCLUDED
#include "utils/logger.hh"

#define GL_CHECK(x)                                                     \
    x;                                                                  \
    {                                                                   \
        GLenum glError = glGetError();                                  \
        if (glError != GL_NO_ERROR) {                                   \
            LOG_ERROR << "glGetError() = " << glError << " at line "    \
                      << __LINE__;                                      \
            exit(1);                                                    \
        }                                                               \
    }
#endif // GDT_CHECKS_HEADER_INCLUDED

