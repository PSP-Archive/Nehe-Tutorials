Hi !

In this archive you'll find :

- this file :D

- Nehe11, Nehe12, Nehe16, Nehe17, Nehe19, and Nehe20: they are adaptation of cygwin version of Nehe tutorials modified in order to run on a PSP. 
You will notice in each folder a working Makefile, "psp-setup.c" needed to compile pspgl programs and "copy.sh" that permits to copy directly your program to your PSP without living cygwin, just type "./copy.sh". 
All parts I've modified are maked by "@@@".
IMPORTANT NOTE: 
To get those tutorials working you'll need the modified version of PSPGL I made (it handles display lists and resolve some bugs). You'll find it there:
http://edorul.free.fr/psp/pspgl_modified.rar

As you saw Nehe13, Nehe14, Nehe15, and Nehe18 are missing:
- Nehe13, Nehe14, Nehe15 : because they need a buit'in font system.
- Nehe18: because it needs too many functions that are not coded in pspgl (gluQuadricNormals, gluQuadricTexture, gluCylinder, gluDisk, gluSphere...)


Known bugs:
- in Nehe12: in this program default light have a strange result everything seems dark unless you rotate the cubes enough. In this case there is no color applied on various cubes (lighning bug ??? or color material bug ??? or both ???)


I hope this archive will be usefull to someone.

Edorul.


history :
v0.1 (07.18.2006): first release

