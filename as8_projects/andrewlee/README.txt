Author: Andrew Lee
Login: cs184-ei
SID: 20293290

Description:
- Compiled on Windows 7 Professional, using Visual Studio 2010 Professional

Usage:
- 'T': Toggle texture (default on)
- 'B': Toggle bump mapping (default on)
- 'P': Toggle Phong/Gouraud (default Phong)
- 'D': Toggle displacement/parallax mapping (default displacement)
- 'E': Toggle environment mapping (default off)

Submission Files:
- images/dragonfly_gouraud.png:
  Just Gouraud shading
- images/dragonfly_phong.png:
  Just Phong shading
- images/dragonfly_phongtexture.png:
  Phong shading and texture
- images/dragonfly_phongtexturebump.png:
  Phong shading, texture, and bump mapping (plus displacement mapping)
- images/simple_gouraud.png:
  Gouraud shading on the dragonfly has little difference from Phong shading
  because of the high resolution of the model. Here, a simpler, lower
  resolution model is used to illustrate the difference better.
- images/simple_phong.png:
  Phong shading on the simpler model.
My .GIF files are super large, so I'll provide the links instead
- https://inst.eecs.berkeley.edu/~cs184-ei/AS8/media/animation.gif:
  Animation of all features on (except environment mapping)
- https://inst.eecs.berkeley.edu/~cs184-ei/AS8/media/metal.gif:
  Animation showcasing environment mapping (bump and displacement mapping off)
  
Extra Credit:
- Parallax Mapping:
  I implemented parallax mapping, which basically morphs the texture such that
  a feeling of depth is created, but the profile of the sweep doesn't change.
- Displacement mapping:
  I implemented displacement mapping, which changes the actual geometry of the
  sweep.
- Environment Mapping:
  I implemented enviroment mapping to give the sweep a convincing reflective/
  metallic look. Also added a skybox to go with it.

