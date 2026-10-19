#pragma once
#include "glad/glad.h"

#include <iostream>


/*
    Reflections need an image of the scene from somewhere other than the current camera
    (or of the current frame itself), and an FBO is how you get that image.
    
    Planar Reflection Techinque uses stencil buffer. No static & dynamic variant for FBO.
    
    Planar Render-to-Texture -> 
        Renders scene from the mirrored camera ->  2D Texture -> 
                Re-rendered whenever the camera or scene changes(Dynamic Variant). No static variant for FBO.
    
    Environment Mapping -> Scene from the probe, 6 times -> cube map ->
        static variant baked once at startup -> 
            re-rendered per frame, or spread across frames in dynamic variant.
    
    SSR -> the current frame's color, depth and normals -> several textures ->
        no static variant ->
        always per frame in dynamic variant.
        
    
    Planar Reflection with stencil buffer:
        It's done without FBO, mark the mirror's pixels in the stencil buffer,
        draw the reflected scene only where stencil is set, then draw the normal scene.
    
    Planar Reflection Render-to-Texture:
        This version uses an FBO, and it is more flexible in terms of adding a blur,
        distortion or rendering at reduced resolution.
        
    Static vs Dynamic Variants:
        SSR is always dynamic;
        
        Planar Reflection is baked once from viewport and static;
        
        Environment Mapping is the only technique with bake-once(static) vs render-every-frame(dynamic)
    
    A consistent definition across all three is therefore: 
        static = static scene, only the camera moves; 
        dynamic = the scene contains moving objects; 
    
     the comparison asks a clear question: 
         can this technique reflect moving objects, and at what cost?
         
         Planar: Yes, at full cost(?).
         SSR: Yes, but only objects visible on screen.
         Dynamic Cube Map: Yes, with cost depending on update frequency.
         Baked Cube Map: No.
    
*/

namespace Graphics
{

class Framebuffer
{
private:
    GLuint mId;
public:
    ~Framebuffer()
    {
        glDeleteFramebuffers(1, &mId);
    }
    
    Framebuffer(int pWidth, int pHeight)
    {
        glCreateFramebuffers(1, &mId);
    }
};

}