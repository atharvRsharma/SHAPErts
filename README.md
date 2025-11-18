 # <div align="center"> SHAPErts </div>

A simple Realtime Strategy game, with model loading, placing and camera controls<br />

The single biggest project I have built in my life(~3400 loc), a true from scratch project, where almost everything used is handwritten(aside from dependenices of course)<br />

From the custom obj model loader to basic meshes itself, everything was handcrafted(with a lot of pain due to learning blender from scratch as well)

This project also happens to be my very first exposure to the ECS(Entity Component System) pipeline, was so much fun to learn the new and unique design patterns, wild how people come up with them.<br />
Finally learned project structure, still don't have the strongest grasp ver it(all code definitions are in the header itself, if <100 loc), but definitely cleaner than previous projects  
sds

# A lot of drawbacks obviously as I would consider this my very first true game(or atleast a half decent skeleton of one), such as:<br />
•	Cannot remove building with right mosue click(i know it is a very basic feature, will be implemented in the very near future)<br />
•	No instanced rendering(a smidge too advance for me, would love to implement it, however it runs decently for how simple the game is, definitely could optimise it further)<br />
•	No explicit log manager, everythings logged to the command line which can be a bit tedious to debug<br />
•	No animations to signal if an actor has been destroyed, they are simply just not rendered to screen and taken out of the registry.  
•	The general actor behaviour is unbelievably buggy, sometimes the enemies will vibrate if targetting same building, sometimes turrets will just stare at their counterparts being pillaged, might need to rework the whole
combat system but for now, it will be untouched, maybe when more building types are added, I will retouch their behaviour, the jank simply remains for now

# General game controls:<br />
•	Hold down mouse wheel to pan to different positions.<br />
•	Right click drag to rotate about current position normal.<br />
•	Left mouse click for general interactivity.<br />
•	Mouse scroll for zooming in and out.<br />
•	Esc key for main menu<br />

# Special commands(cheat codes):<br />
•	Free cam mode: press arrow up, up, down, down, left, right, left, right for a free cam mode to freely move about the scene, reenter the same buffer to snap back to last stable orbit camera postion. When in free cam mode, general wasd controls apply and cursor callback is disabled.
<br />•	Toggle fullscreen: press left alt+enter to toggle between fullscreen and restored window mode.
