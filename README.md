# EECS498-014-Materials
This is a modified HW1 code base for people interested in doing extra credit. I've removed all code in my rasterizer_impl.cpp other than function stubs. You will probably want to refernence the rasterizer.hpp for documentation on the new functions to implement as well as some new member variables that have been added for some of these tasks. Note that I have not build out infrastructure for shadow mapping or a rendering api.

If you are interested in doing shadow mapping, you will want to create a vector of shadow buffers for each light in rasterizier.hpp and have 1 function to generate this buffer for each light. Your shader function will then have to be udpated to include the shadow information.


1) MSAA: set the antialiasing property `MSAA`
 - Note: MSAA can only be visualized in the shader tasks since MSAA is an optimization to the number of times the shader is run
 - `task-shading-msaa.yaml` will display an example
2) deferred shading: set the task to `deferred-shading`
 - Note: You can also run MSAA with deferred shading
 - `task-deferred-shading*.yaml` tests will display examples of deferred shading
3) texture mapping: include a texture property with a value of the path to the texture image
 - Note: to visualize the mipmap create a directory "texture-mipmap" and set the task to task to `texture-test` and provide a texture parameter. (assuming you set the mipmap buffer's output types to "texture-mipmap")
 - Note: You may want to increase the decrease the ambient lighting since the ambient lighting is multiplied by the color of the texture (as is described in learn opengl).
 - Note: the texture will be applied to *all* surfaces
 - `task-texture-*` tests will display examples of texture.