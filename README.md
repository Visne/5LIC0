# Combining self-service technology and electronic shelf labeling using a networked embedded system
To run the project, follow these steps:

### Install Contiki-NG
These instructions will install Contiki-NG to `~/contiki-ng` on a commit that is known to work with our project. If you install Contiki-NG elsewhere, `Makefile.path` needs to be updated so that our build system can find it!
1. `cd ~`
1. `git clone https://github.com/contiki-ng/contiki-ng`
1. `cd ~/contiki-ng`
1. `git checkout f638934358e`
1. `git submodule update --init --recursive`

### Run Cooja
1. `cd ~/contiki-ng/tools/cooja`
1. `./gradlew run`

### Run the simulation
1. File > Open simulation > Browse...
1. Select `Small Store.csc` from this repository
1. Press Start/Pause to start the simulation
