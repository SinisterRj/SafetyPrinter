# SafetyPrinter
The objective of this project is to increase the safety of FDM (Fused Deposition Modeling) 3D printers in a similar way to what is done by the process industry.
WARNING: we are not responsible for damages generated, directly or indirectly through the use, even if correct or not, of the information contained herein.
In the process industry, we generally have a set of controllers responsible for controlling the process plant, a supervisory system where all information is made available in real time to the operating team and another set of controllers, called "safety controllers" , responsible for interrupting production and taking the industrial plant to a safe condition in case something gets out of control.
So, similarly speaking, considering our FDM 3D printer as the process to be controlled, we already have today:
- The main microcontroller responsible for controlling the printer individual elements, as its stepper motors, heaters and temperature sensors, usually configured with a firmware such as Marlin or Klipper;
- The supervision system, from which it is possible to configure, command and supervise the printer remotely. This is the role of higher level systems like Octoprint that runs on a dedicated controller (usually a Raspberry Pi);
Hence it is concluded that although we already have more than one microcontroller in the 3D printers, they perform complementary functions and, despite several efforts to improve its the safety, a single failure in a temperature sensor or the detachment of the heating cartridge can still start a fire.
This raises the need for an INDEPENDENT safety system that PREVENTS the accident by detecting by its own means that the main controller has lost control of the machine or MITIGATES the effects of an accident, reducing the damage caused by a failure.
Therefore, an FDM 3D printer Safety System should be:
**1) Independent**.
Independence is important to prevent failures (whether hardware or software related) from affecting the control and safety systems at the same time.
**2) Simple.**
Simplicity is the key to safety. Simpler systems are easier to understand and have fewer "hidden" errors, with a direct and known cause and effect relationship. All you don't want is a safety system that behaves strangely during an emergency.
**3) Effective.**
The action taken must be effective for the scenario that we want to avoid. There is no point to have beautiful engineering solutions that don't solve our problem or that may generate an even greater risk than the original.
This project intends to implement a Safety System for FDM 3d printers respecting the characteristics presented above. Although its implementation reduces the risk of having a 3d printer at home, it does not mean that you can abandon your printer. Even with the system installed there is still a risk of fire and you should never leave your printer unattended.
And remember, just like any other system, the Safety System for 3D printers is not flawsless, so it can even act spuriously and turn off your printer in the middle of a print without anything wrong. It's part of the game.
