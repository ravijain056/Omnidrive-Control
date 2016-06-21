Theory behind the project
=========================

The mobility equations
----------------------
There are two main classical models of representing any omnidrive mobile body,
  1. Unicycle model
  2. Omnidrive model

.. _unicycle:

Unicycle model
^^^^^^^^^^^^^^
Unicycle model has the control over the following parameters of the robot to make it move : 
   1. Linear velocity of robot chassis
   2. Angular velocity of robot chassis
This model has a more physical sense and comparability to our daily life. Say for example, you want your robot to go at 10m/s and then turn 90°. But while controlling a robot using motors and wheels you need to convert this to RPM commands for each motor, which is the Omnidrive model.

.. _omnidrive:

Omnidrive model
^^^^^^^^^^^^^^^
Omnidrive model for a three wheel drive chassis has the following paramters of the robot to make it move :
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

         R is Radius of robot chassis

You can check the for the derivation by referring to :download:`this pdf</download/n-wheel-drive-paper.pdf>`

This is the model we have used in this version of the project.

System description
------------------

The following represents the system schematics of this version.

.. image:: ../images/threewheelschematic.png

The following section will give a brief idea of each blocks role,

  * **Upper Control System** :    
    
    * **Upper controller** : The Upper controller calculates the :ref:`unicycle` parameters of the robot, converts it to :ref:`omnidrive` and sends RPM in the form of packets to the lower controller to execute the desired RPM so that the robot reaches the desired unicycle State and hence the desired physical state. It takes the feedback from the physical orientation of the robot as well as its physical location and applies PID controller to calculate the unicycle parameters.
    
    * **Angular position feedback** : This is an IMU module used to calculate the magentometer readings to find the relative angular position of the IMU and hence the robot from the magnetic axis. 

    * **Cartesian position feedback** : This is a feedback system to find out the distance traversed by the bot in X and Y direction and hence keep a trace of the path followed by the robot. In this version, we have used **two dummy wheels arranged orthogonally** with :ref:`Encoder` on the chassis. So this would give distances travelled in X and Y direction.
 
  * **Lower Control System** :

    * **Lower controller** : It contains the logic to drive the motor at the commanded rpm using a P controller. Irrespective of change in load on the motor the motor should maintain the desired RPM in the least possible time which is seen by the lower controller. It also calculates the wheel rpm from the :ref:`Encoder` for feedback rpm. The Upper and Lower conrtol system communicate with the help of UART.

    * **Motor actuators/Motors/Wheels** : Sending PWM signals to :ref:`motor-drivers` of different duty cycles can vary motor RPM and hence wheel RPM.

    * **RPM feedback** : Rotary shaft encoders are used for RPM feedback.
