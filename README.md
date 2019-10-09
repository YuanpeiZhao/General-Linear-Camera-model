# General-Linear-Camera-model
CS 535 Assignment 1 - This project implement a General Linear Camera (GLC) model with freeglut

## Object
This program defined hitable virtual class which derives two classes, sphere and triangleMesh. This two classes are defined in sphere.h and triangleMesh.h.

## Ray-object intersection
The sphere class and the triangleMesh class have different implementation of the ray-object intersection method which is written in hit() method. You can check them in the header file mentioned above. This part referred to these resources:
- [Ray tracing in a weekend, Peter Shirley](https://github.com/RayTracing/raytracing.github.io)
- IntersectTriangle demo in DirectX SDK

## GLC Definition
The GLC is defined in GLCcamera.h as a class. It is defined by 6 points in 2 planes. It also has functions to set the rotation matrix of the camera.

## GLC Rendering
The result for the GLC is saved as a .jpg image which is then read as a texture to render on the screen. You can check the LoadTexture() and WriteTexture() in main.cpp for more details.
## GUI
- You can press and hold the mouse left button and move the mouse to rotate the camera, just like in fps games. If you feel strange with the X-axis or Y-axis movement, you can also right click the mouse and choose “reverse X-axis” or “reverse Y-axis”
- You can right click the mouse to choose camera modes from “pinhole” “orthographic” and “XSlit” (XSlit is as defined in the GLC paper).
## Extra
-	Besides of using triangle meshes, this program generates sphere objects.
-	This program uses ray-casting to imitate the diffuse light, which can also generate shadows.
