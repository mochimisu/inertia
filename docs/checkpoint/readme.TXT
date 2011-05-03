AS12
Intermediate progress report. http://brandonwang.net/inertia/?page_id=159


GROUP
Brandon Wang (cs184-di)
Andrew Lee (cs184-ei)
Chris Tandiono (cs184-ew)


TECHNICAL CHALLENGES
All of our methods are outlined in in the blog.

ALLOW THE USER TO TURN THE SHIP, MAINTAINING A CONSISTENT VELOCITY DIRECTION – 80%
Brandon has implemented a system in which velocity and acceleration vectors are maintained in world space, coplanar to the tangent plane, but independent from the tangent vector direction. There currently exist some fine tuning to do of the friction/air drag coefficients, camera position, velocity, and to make the function consistent in time across multiple machines.

PROCEDURALLY GENERATE THE SURROUNDING ENVIRONMENT – 75%
Andrew has created randomly generated cityscape using greebling, but they are stretched gray cubes that we hope to texture later. The difficult part of procedural track generation has been completed – the algorithm has successfully produced very interesting tracks, and now Andrew is fine tuning the control points and combining it with Brandon’s renderer.

IMPLEMENT ADVANCED RENDERER FUNCTIONS – 95%
Brandon has implemented variance shadow maps and light scattering in the classical rendering pipeline. After the scene and gameplay comes together, Brandon will have to tweak the settings and resolutions of the renderer effects to achieve an acceptable framerate on our target machine.

TEXTURE MAPPING AND MODELS – 60%
Brandon created a ship object in Blender last week, and explored making a simple HUD using alpha blending. Chris has been working to texture the ship, as the cylindrical object was in AS8, fixing the normals of faces for backface culling, exploring different textures for the track, and finding a method to export the texture in such a way that the mesh can import it.

Pictures and movies can be seen on the online version of AS12:
http://brandonwang.net/inertia/?page_id=159

