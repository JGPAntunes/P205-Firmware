# P205-Firmware
This repository was imported from GITLAB manually, so no commits or branches are available.

At the start of development, we defined the firmware hierarchy according to the expectation that the board would consist of 2 microcontrollers. 

This mandatory so that the firmware implementation would remain consistent and would avoid redefinition situations. 

This hierarchy, as the project progressed, was adjusted as other features were added and optimization was considered.

The final hierarchy is available in the "images" folder.

All the code was developed in SEGGER Embedded Studio using JLink to flash our custom boards.

# Disclaimer! To accelerate development, a partner company provided a few libraries manly for the cloud communications (since we're also using their cloud),
and since those libraries have proprietary content, the respective folders are empty.
