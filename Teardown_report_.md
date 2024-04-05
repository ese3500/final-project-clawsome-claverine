[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-24ddc0f5d75046c5622901739e7c5dd533143b0c8e959d652212380cedb1ea36.svg)](https://classroom.github.com/a/2TmiRqwI)
# final-project-skeleton

    * Team Name: Clawsome Claverine
    * Team Members: August Fu & Darian Mettler
    * Github Repository URL: https://github.com/ese3500/final-project-clawsome-claverine
    * Github Pages Website URL: (not present yet)
    * Description of hardware: ATmega328PB, Windows on Arm 11 (Surface X Pro)

## Final Teardown report

### Work done
- We labeled all the calbes
- We found out how the cables are supposed to work
- All the wires to the top part was cut

### Motor control
- 3 motors
- Each motor has two wires (ground and hight)
- Those are DC motors, 4.2V (backward -4.2V, forward 4.2V)

### Sensors
- Boundary detection sensors
    - The right/left motors have two sensors to signal the boundary hit
    - The back/forth motors have two sensors to signal the boundary hit
    - They work like common buttons wit a pull up resistor attached
- Sucsess Sensor
    - LED always lighting towards the sensor with light in a specific frequency
    - Light sensor which detets if the incomming light from the matching frequency is too low and then it goes down.

### Actuators
- Buzzer
- Coin detection mechanism


## Github Repo Submission Resources

You can remove this section if you don't need these references.

* [ESE5160 Example Repo Submission](https://github.com/ese5160/example-repository-submission)
* [Markdown Guide: Basic Syntax](https://www.markdownguide.org/basic-syntax/)
* [Adobe free video to gif converter](https://www.adobe.com/express/feature/video/convert/video-to-gif)
* [Curated list of example READMEs](https://github.com/matiassingers/awesome-readme)
* [VS Code](https://code.visualstudio.com/) is heavily recommended to develop code and handle Git commits
  * Code formatting and extension recommendation files come with this repository.
  * Ctrl+Shift+V will render the README.md (maybe not the images though)