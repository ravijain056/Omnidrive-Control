.. Three wheel omnidrive documentation master file, created by
   Aniket on Thu Jun 20 13:31:01 2016.
Three Wheel Omnidrive
=====================
A three wheel omnidrive consists of three omniwheels mounted on a chasis and giving the wheels a correct set of RPM can make the robot move in all directions. Normally, the wheels are mounted near the periphery of the chasis and also the angle between the wheel/motor mountings is kept equilavent i.e. 120°

The mobility equations
----------------------
There are two main classical models of representing any omnidrive mobile body,
  1. Unicycle model
  2. Omnidrive model

Unicycle model
^^^^^^^^^^^^^^
Unicycle model has the control over the following parameters of the robot to make it move : 
   1. Linear velocity of robot chasis
   2. Angular velocity of robot chasis
This is model has a more physical sense and comparability to our daily life. Say for example, you want your robot to go at 10m/s and then turn 90°. But while controlling a robot using motors and wheels you need to convert this to RPM commands for each motor, which is the Omnidrive model.

Omnidrive model
^^^^^^^^^^^^^^^
Omnidrive model for a three wheel drive chasis has the following paramters of the robot to make it move :
   1. RPM of wheel 1
   2. RPM of wheel 2
   3. RPM of wheel 3

Mathematical model
^^^^^^^^^^^^^^^^^^
The mathematical model derived from kinematics of the three wheel robot without consideration of the friction or other external factors is as shown below,

   :math:`RPM _a = V _x * 0.5 - V _y * 0.866 + w * R`

   :math:`RPM _b = V _x * 0.5 + V _y * 0.866 + w * R`

   :math:`RPM _c = V _x + w * R`

   where RPM :subscript:`a`, RPM :subscript:`b`, RPM :subscript:`c` are RPMs of wheels A,B,C

         V :subscript:`x` is the component of velocity of robot in x direction

         V :subscript:`y` is the component of velocity of robot in y direction

         w is Angular velocity of robot

         R is Radius of robot chasis

You can check the for the derivation by referring to the following :download:`this pdf</download/n-wheel-drive-paper.pdf>`

This is the model we have used in this version of the project.

Hardware setup
--------------
