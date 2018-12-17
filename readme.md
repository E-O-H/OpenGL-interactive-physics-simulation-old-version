# Assignment 4: Final Project
Introduction<br>
The project is a physics simulation that has Newtonian gravity between all pairs of objects, as well as elastic collision for spheres (only one-on-one collision is currently supported; if an object collides with more than two other objects at the exact same frame, the calculation would not be correct; you should also not put two objects at the same place; if the model is not a sphere, the collision calculation uses the smallest bounding sphere).<br>
User can navigate the scene with a FPS-style control with keyboard and mouse. User can shoot new objects into the scene by clicking the left mouse button (a preview of the object is displayed at the bottom-right corner of the screen, referred to as the object in “hand”). The default shooting speed is zero (i.e. put a static object at the bottom-right corner of the screen). User can change the shooting speed with the mouse wheel. A cone will appear at the bottom-right corner of the screen to indicate the shooting direction and speed. The size and density can also be changed interactively. Object in hand is given a random rotation speed that roughly follows a logarithmic distribution; if you find it annoying you can press the middle mouse button to stop the rotation, or press a number key again (for example 4 for the earth model) to re-randomize the rotation. If the object in hand is too large and blocks the view, you can press F1 to change it to wireframe mode.<br>
There are also premade object formation examples that can be dynamically loaded and added to the scene (by pressing a number key in 6~0 and F5~F8; it is recommended to press “`” to clear the scene first; 6, F5 and F6 are the most recommended examples; you can also write your own example files and put them in the “data/examples” folder.<br>
For more features and controls see the key bindings section below.<br>
The program should be pretty stable, but if you ever encounter a case where you cannot add new objects, it is likely due to there are objects in the scene that has infinite properties (putting two objects at the exact same place would cause this to happen); in this case simply press “`” (the first key on the number row) to delete all objects in the simulation to reset the scene. Also, please avoid putting too many objects in the scene. Since this is a simulation that has gravity between every pair of objects (instead of a single gravity like the usual physics simulation in video games), the complexity is O(n2).<br><br>
Key bindings<br>
“<esc>”: exit program<br>
“`”: Delete all objects in the simulation and reset the scene<br>
“w”: move forward<br>
“s”: move backward<br>
“a”: strafe left<br>
“d”: strafe right<br>
“space”: ascend<br>
“left-alt”: descend<br>
“=“ or “numpad+“: increase moving speed<br>
“-” or “numpad-”: decrease moving speed<br>
“left-mouse-button”: shoot a new object<br>
“mouse-wheel-scroll”: change the shooting speed (speed change is linear in low speed, and exponential in high speed).<br>
“middle-mouse-button”: stop the randomized rotation; also reset launch speed to zero<br>
“r”: make object in hand (or selected object) larger. This will increase the mass while preserving the density.<br>
“f”: make object in hand (or selected object) smaller. This will decrease the mass while preserving the density.<br>
“t”: make object in hand (or selected object) denser. This will increase the mass while preserving the size.<br>
“g”: make object in hand (or selected object) less dense. This will decrease the mass while preserving the size.<br>
“<F1>”: change the object in hand (or the currently selected object in scene) to wireframe mode<br>
“<F2>”: change the object in hand (or the currently selected object in scene) to flat mode<br>
“<F3>”: change the object in hand (or the currently selected object in scene) to smooth mode<br>
“<F4>”: change the object in hand (or the currently selected object in scene) to normal-vector display mode<br>
“1”: change the current object in hand to a unit cube<br>
“2”: change the current object in hand to a bumpy cube<br>
“3”: change the current object in hand to a bunny<br>
“4”: change the current object in hand to the earth (default and recommended model)<br>
“5”: change the current object in hand to a fancy skeletal sphere (note this object is actually much larger than it seems and is very massive by default despite the skeletal look; it is intended to function as a “star core”; putting other objects close to it is not recommended)<br>
“e”: enter select mode and select next object (cycle); used for editing objects in the scene<br>
“q”: enter select mode and select previous object (cycle)<br>
“backspace” or “z”: cancel selection (editing mode changes back to the object in “hand”)<br>
“\”: toggle blink for selected object<br>
“<tab>”: toggle wireframe display in any mode for object in hand (or selected object)<br>
“<enter>”: toggle perspective/orthographic projection<br>
“[”: increase perspective FOV / orthographic width<br>
“]”: decrease perspective FOV / orthographic width<br>
“h”, “j”, “k”, “l”, “u”, “I”: translate selected object in world space<br>
“n”, “m”, “,”, “.”, “;”, “/”: rotate selected object in world space<br>
