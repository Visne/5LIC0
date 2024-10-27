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
1. The behaviour of virtualized CAN nodes may be modified under `motes/shared/sim-tuning.h` *(requires Reload of simulation to take effect)*
1. If you wish to make use of the price tag visualization, please update `CLUSTER_VIS_DIRECTORY` in `libraries/CAN-sim/shard/types.h` to your desired location, text representations of every 10th mote's CAN nodes will be generated here. Write privilege at the logging location is required for this. *(requires Reload of simulation to take effect)*
