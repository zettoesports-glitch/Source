#include "../include/GL46Capabilities.h"
#if defined(_WIN32)
#include <GL/glew.h>
#include <GL/gl.h>
namespace reverse_gl46 {
GL46Capabilities QueryGL46Capabilities(){
    GL46Capabilities c{};
    // INFERRED implementation using exactly the capabilities named by strings/imports in Main.exe.
    c.gl33Bridge = GLEW_VERSION_3_3 != 0;
    c.gl46Bridge = GLEW_VERSION_4_6 != 0;
    c.shaderObjects = GLEW_VERSION_2_0 != 0;
    c.bufferObjects = GLEW_VERSION_1_5 != 0;
    c.vertexArrayObjects = GLEW_VERSION_3_0 != 0;
    c.textureStorage = GLEW_ARB_texture_storage != 0;
    c.timerQuery = GLEW_ARB_timer_query != 0;
    c.debugOutput = GLEW_KHR_debug != 0 || GLEW_ARB_debug_output != 0;
    c.directStateAccess = GLEW_VERSION_4_5 != 0 || GLEW_ARB_direct_state_access != 0;
    c.bufferStorage = GLEW_ARB_buffer_storage != 0;
    c.uniformBuffer = GLEW_ARB_uniform_buffer_object != 0;
    c.shaderStorageBuffer = GLEW_ARB_shader_storage_buffer_object != 0;
    c.samplerObjects = GLEW_ARB_sampler_objects != 0;
    c.syncObjects = GLEW_ARB_sync != 0;
    c.instancedDrawing = GLEW_ARB_draw_instanced != 0;
    c.multiDrawIndirect = GLEW_ARB_multi_draw_indirect != 0;
    c.computeShader = GLEW_ARB_compute_shader != 0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE,&c.maxTextureSize);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,&c.maxTextureUnits);
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS,&c.maxVertexAttribs);
#ifdef GL_MAX_UNIFORM_BUFFER_BINDINGS
    glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS,&c.maxUniformBufferBindings);
#endif
#ifdef GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS
    if(c.shaderStorageBuffer) glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS,&c.maxShaderStorageBufferBindings);
#endif
    return c;
}
}
#endif
