Auto-Aiming Turret V1: January 11 2023 - December 14 2023

This is a turret that tracks the closes face around. It can rotate around 360 degrees and beyond without tangling itself up due to the gear.

It isn't too responsive, as the computer is a little weak, and the stepper motors cannot produce a fast enough max speed without creating a blurry video the computer can't read.

Used LBP cascades for facial detection.

The turret works on a pseudo-proportional controller, where it tries to aim the camera so that it is centered at the face it is aiming at. The farther it is from the face, the larger the voltage applied to the motor. It is pseudo- proportional control because K_p changes based on the distance.

Next version, I plan to use a brushless DC motor with PID, since they are more powerful and efficient than stepper motors. Stepper motors were used because they do not require positional feedback. Also plan to train my own face detection.
