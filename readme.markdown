### Tests, examples and experiments  
Chris Pilkington  
Copyright (C) 2005-2010  
<http://chris.iluo.net/> 

If you find this project helpful, please consider making a donation. 

[<img alt="Make a donation via Pledgie" src="http://www.pledgie.com/campaigns/17973.png?skin_name=chrome" border="0" />][1]  

### License

This program is free software; you can redistribute it and/or  
modify it under the terms of the GNU General Public License  
as published by the Free Software Foundation; either version 2  
of the License, or (at your option) any later version. 

This program is distributed in the hope that it will be useful,  
but WITHOUT ANY WARRANTY; without even the implied warranty of  
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  
GNU General Public License for more details. 

You should have received a copy of the GNU General Public License  
along with this program; if not, write to the Free Software  
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.  

### Description  

A collection of tests and small utilities:
- c++11_test: Just a basic test for using implementing a type safe printf in C++11 using variadic template arguments  
- discombobulator: You give it a string and it creates a function to build a value from obfuscated data.  The idea being that you can use this function to store sensitive  
- data instead of just storing it as raw strings that can be viewed in a hex editor or with the "strings" utility.  
- openglmm_fadein: Fades into the scene from the desktop.  The way it does is by saving a screenshot image of the desktop, creating a full screen context that spans across all monitors, showing the stored image and then fading between that and the actual scene.  
- openglmm_font: Testing use of fonts.  
- openglmm_gears: Similar to the glxgears test application  
- openglmm_heightmap: Testing heightmap loading  
- openglmm_shaders: Testing various object shaders and post render effect shaders  
- permutations: Given a string this will print all the permutations of the characters in that string  
- size_test: Prints out the sizes of various types in the current architecture  
- source_cleaner: Applys various very simple fixes such as replacing tabs with spaces, removing leading and trailing spaces, in source code and text files in a folder  
- xdgmm: Test for using libxdgmm  

### Usage

Many of the tests have cmake projects which can be run like so:  
cd openglmm_gears  
cmake .  
make  
./openglmm_gears  

### Getting a copy of the project on Linux (Fedora 14 used)

Pull this project:  
yum install git  
OR  
sudo apt-get install git-core  
git clone git@github.com:pilkch/tests.git  
OR  
git clone https://github.com/pilkch/git@github.com:pilkch/tests.git  

### Building on Linux (Fedora 14 used)

Many of the tests require <https://github.com/pilkch/library/>. For tests that use libopenglmm or libxdgmm this will require extra dependencies such as <http://www.libsdl.org/>.  
It should be checked out at the same level as tests:  
source/  
- library/  
- tests/  

### Credit

Breathe and Spitfire are created by me, Christopher Pilkington.   
For the parts that are not mine, I try to keep an up to date list of any third party libraries that I use.   
I only use libraries that are license under either the GPL, LGPL or similar and am eternally grateful for the high quality ease of use and generosity of the open source community.  

Box2D  
Copyright (c) 2006-2007 Erin Catto  
All rights reserved  
<http://www.gphysics.com/> 

Open Dynamics Engine  
Copyright (C) 2001-2003 Russell L. Smith  
All rights reserved  
<http://www.q12.org/>  SDL - Simple DirectMedia Layer

  
Copyright (C) 1997-2006 Sam Lantinga  
Sam Lantinga  
<http://www.libsdl.org/>  FreeType Copyright 1996-2001, 2006 by

  
David Turner, Robert Wilhelm, and Werner Lemberg  
<http://www.freetype.org/> 

3DS File Loader  
genjix@gmail.com  
<http://sourceforge.net/projects/scene3ds>  
<http://www.gamedev.net/community/forums/topic.asp?topic_id=313126> 

MD5 RFC 1321 compliant MD5 implementation  
Copyright (C) 2001-2003 Christophe Devine 

Memory manager & tracking software  
Paul Nettle  
Copyright 2000, Fluid Studios  
All rights reserved  
<http://www.FluidStudios.com>

 [1]: http://www.pledgie.com/campaigns/17973

