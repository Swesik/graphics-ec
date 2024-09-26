I have implemented the following tasks:
You should be able to run the code using the following command:
``` g++ *.cpp -o rasterizer -fcolor-diagnostics -std=c++20 && ./rasterizer sample-tests/<test-name>.yaml ```
1) MSAA: set the antialiasing property `MSAA`
 - Note: MSAA can only be visualized in the shader tasks since MSAA is an optimization to the number of times the shader is run
 - `task-shading-msaa.yaml` will display an example
2) deferred shading: set the task to `deferred-shading`
 - Note: You can also run MSAA with deferred shading
 - `task-deferred-shading*.yaml` tests will display examples of deferred shading
3) texture mapping: include a texture property with a value of the path to the texture image
 - Note: to visualize the mipmap create a directory "texture-mipmap" and set the task to task to `texture-test` and provide a texture parameter.
 - Note: You may want to increase the decrease the ambient lighting since the ambient lighting is multiplied by the color of the texture (as is described in learn opengl).
 - Note: the texture will be applied to *all* surfaces
 - `task-texture-*` tests will display examples of texture.