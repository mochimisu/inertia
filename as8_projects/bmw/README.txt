Author: Brandon Wang
Login: cs184-di
SID: 21127830

Description:
AS8 Textured Smooth Surfaces 

Tested on Apple Mac OS X 10.6 (Snow Leopard), g++ 4.2.1, OpenGL 2.1, GLSL 1.2, nVidia GeForce GTX280

Picture name format:
To distinguish between which effects are on in each screenshot, I used the following naming convention:
ambientlayer.png                   displays the ambient occlusion layer
animation.gif                      animation gif
chineseknot.trk                    my shape
defaultshape.png                   default shape in my renderer, can clearly see all effects
depthbuffer.png                    on-screen display of depth buffer
envmapping.png                     environment map example
envmappinghalf.png                 environment map example at half reflection
screenshot.png                     screenshot running on my system
thread1_bump.png                   bump map for my shapethread2.png                        texture for my shape

g.png                              My shape with Gourad only
gt.png                             Gourad + Textures
p.png                              Phong only
pt.png                             Phong + Textures
ptb.png                            Phong + Textures + Bump
ptbd.png                           Phong + Textures + Bump + Displacement
ptbds.png                          Phong + Textures + Bump + Displacement + Shadows
ptbdsf.png                         Phong + Textures + Bump + Displacement + Shadows + PCF
ptbdsfa.png                        Phong + Textures + Bump + Displacement + Shadows + PCF + Ambient Occlusion
ptbdsfag.png                       Phong + Textures + Bump + Displacement + Shadows + PCF + Ambient Occlusion + Ground

My source is in the folder framework/

  
Extra Credit:
Displacement Mapping (Implemented in GLSL)
I implemented displacement mapping using the vertex shader. This turned out to be simpler than modifying the geometry before sending it to the shaders, and will achieve a faster effect. However, this method of displacement is dependent on how finely sampled the scene is.

Shadow Mapping (Implemented in GLSL)
I implemented shadow mapping with much help of Fabien Sanglard's blog (http://www.fabiensanglard.net/). The shadows are achieved in a two-pass render phase. First, the scene is rendered from the light's point of view. This render is saved to a texture (depth buffer/shadow map), storing distance in Z as color. The second pass renders the scene from the actual camera's point of view, this time using the depth buffer in the fragment shader. The depth buffer is mapped to the current view, and the portions of the shape that the light can "see" are rendered in light. The other parts have their color values simply scaled by 1/2 to simulate a shadow. Shadow mapping proved to be difficult, especially keeping transformations consistent in both the depth buffer and camera view.

Screen Space Ambient Occlusion (Implemented in GLSL)
Using the depth buffer from my shadow implementation, I made a try at SSAO. I made a hack-y approximation of an ambient occlusion effect in the vertex shader by 16 random points in the depth buffer around the pixel to be rendered. The 16 random points are achieved using a vector offset within the 3d unit sphere (I chose a 3d unit sphere, but only utilize the 2d coordinates. The third coordinate is to ensure a maximum distance of 1 of its 2d space without making the distnace exactly 1). These 16 vectors are generated using a python script (randomspherical.py). The vectors are hardcoded to provide a consistent frame-to-frame picture in a single compilation of the fragment shader. The offset vectors are used in the depth buffer to determine if the given pixel should be occluded. An occluded pixel will be one with a nearby depth buffer pixel that is closer to the light. This effect achieves shadow similarly to my shadow map, by simply multiplying color.

Face Culling (OpenGL)
I used face culling primarily to prevent self-shadowing. I culled the front faces to provide an accurate shadow map for the back faces from the light. Extending this, culling back faces when rendering from the camera provides a speedup without sacrificing any visible differences. The given framework code actually renders b-splines backwards (front faces being back faces), and this issue is solved by reversing the order of the control points of the cross-section (courtesy of James Andrews), and reversing the computed normals.

Percentage Closer Filtering (PCF) (GLSL)
To achieve softer shadows, I used a PCF technique. This involves using a 8x8 kernel of pixels to sample around the actual depth buffer location. The final shadow result is the averaged total of the kernel's depth buffer values (of being in shadow or not).

Environment Mapping (GLSL)
I implemented environment mapping. Though it does output on my machine, my shaders give a warning. Therefore, I hardcoded it to be disabled by default. It can be turned on again by uncommenting line 217 in my fragment shader, and toggled with e.

On-Screen Console Output (OpenGL)
I used the on-screen console I wrote in AS1 to output toggles of various effects. This initially provided some issues with textures and coloring not previously encountered.

Movable Camera and Lights (OpenGL)
Using various keys, the user can move the camera and lights in the scene.

Rendering Depth Buffer (OpenGL)
The depth buffer can be shown on screen (with a toggle).

Ambient Layer Display (GLSL)
The ambient layer can be displayed instead of the actual render.

Chinese Knot (trk)
I created a Chinese Knot closed b-spline to render. This includes a bump map and a texture map.

Skybox (OpenGL)
I implemented a skybox by creating a large cube containing the object. It is not rendered in the first phase of shadow generation, so it does not cast a shadow.

Background (OpenGL)
I thought that my skybox was a bit too dizzy, so I implemented a static ground. It uses the same texture as my skybox. A custom texture can be used as the second argument to sweep

B-Spline Editor (OpenGL)
I utilized one of my AS7 EC, my track editor, to create the Chinese Knot b-spline. With some bugfixes related to adding points, it proved invaluable to designing such a shape. Without this tool, I would have not been able to (reasonably) make such a shape.


Issues
Moire effect/Self Shadowing Patterns
Because our shapes are so smooth, it is difficult removing the self-shadowing/Moire pattern effect (as compared to angled surfaces). I alleviated most of the effect by slightly displacing the depth buffer's z coordinate, allowing for (hopefully very insignificant) parts to be "assumed" in shadow. However, this solution breaks down at very slightly curved shapes, as can be seen in my screenshot (moire.png). This also effects ambient occlusion.

SSAO approximation
my SSAO effect is approximated by 16 random vectors around the current pixel to be rendered. This is a very large approximation, as I would ideally compare with all pixels in a given range of the given pixel to be rendered. However, this is unreasonable and would cause rendering times to increase to an unreasonable point (on most hardware). The 16 vectors are used for a quicker render time, but approximates the effect. The approximation is obvious on certain shapes and certain viewing angles.

Face Culling
Using face culling helps create the effect of shadows without self-shading. However, usage of face culling makes the depth buffer not a true depth buffer that would present the object's depth, not just its back faces. My SSAO implementation assumes a true depth buffer, and the effects of face culling can be seen when viewing the ambient layer, where it occludes only on back face, and appears to be able to see through objects.

PCF Performance
Using PCF gives nicer, soft shadows. However, the performance hit is massive. Using a 8x8 pixel kernel increases texture lookups per pixel by a factor of 64. 

Skybox Appearance
I wanted shadows to appear on my skybox. However, I could not turn off all my other effects. As a result, the skybox is unnecessarily bumped and displaced.
